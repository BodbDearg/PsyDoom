//------------------------------------------------------------------------------------------------------------------------------------------
// Vulkan renderer, core functionality.
// Manages the low level details of interacting with Vulkan for the new native Vulkan render path.
// Also handles blitting the PSX framebuffer to the screen if we are using the classic PSX renderer and outputting via Vulkan.
// Sets up key Vulkan objects and provides various Vulkan utility code.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "VRenderer.h"

#if PSYDOOM_VULKAN_RENDERER

#include "CmdBufferRecorder.h"
#include "CmdBufferWaitCond.h"
#include "FatalErrors.h"
#include "Framebuffer.h"
#include "Gpu.h"
#include "LogicalDevice.h"
#include "MutableTexture.h"
#include "PcPsx/Config.h"
#include "PcPsx/PsxVm.h"
#include "PcPsx/Video.h"
#include "PhysicalDevice.h"
#include "PhysicalDeviceSelection.h"
#include "RenderTexture.h"
#include "Semaphore.h"
#include "Swapchain.h"
#include "Texture.h"
#include "VDrawing.h"
#include "VkFuncs.h"
#include "VMsaaResolver.h"
#include "VPipelines.h"
#include "VRenderPass.h"
#include "VulkanInstance.h"
#include "WindowSurface.h"

// TODO: remove temp/hack renderer toggle and implement properly
#include "PcPsx/Input.h"
#include <SDL.h>

BEGIN_NAMESPACE(VRenderer)

// What color surface formats are supported for presentation, with the most preferential being first.
// B8G8R8A8 should be supported everwhere and apparently is the most performant, but just in case offer some alternatives...
static constexpr VkFormat ALLOWED_COLOR_SURFACE_FORMATS[] = {
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_A8B8G8R8_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    VK_FORMAT_A2B10G10R10_UNORM_PACK32
};

// What formats we use for 16 and 32-bit color.
// Note that 'VK_FORMAT_A1R5G5B5_UNORM_PACK16' is not required to be supported by Vulkan as a framebuffer attachment, but it's pretty ubiquitous.
static constexpr VkFormat COLOR_16_FORMAT = VK_FORMAT_A1R5G5B5_UNORM_PACK16;
static constexpr VkFormat COLOR_32_FORMAT = VK_FORMAT_B8G8R8A8_UNORM;

// Use the classic PSX renderer? If this is enabled then just blit the PSX framebuffer to the screen.
// TODO: add ways to toggle PSX renderer.
bool gbUsePsxRenderer = false;

// Cached pointers to all Vulkan API functions
vgl::VkFuncs gVkFuncs;

// Coordinate system conversion: scalars to convert from normalized device coords to the original PSX framebuffer coords (256x240).
// Also where the where the original PSX framebuffer coords start in NDC space.
//
// Note: these are updated only at the start of each frame and ONLY if there is a valid framebuffer.
// They should not be used outside of rendering and should only be used if the framebuffer is valid.
float gNdcToPsxScaleX, gNdcToPsxScaleY;
float gPsxNdcOffsetX, gPsxNdcOffsetY;

static bool                         gbDidBeginFrame;                // Whether a frame was begun
static vgl::VulkanInstance          gVulkanInstance(gVkFuncs);      // Vulkan API instance object
static vgl::WindowSurface           gWindowSurface;                 // Window surface to draw on
static const vgl::PhysicalDevice*   gpPhysicalDevice;               // Physical device chosen for rendering
static vgl::LogicalDevice           gDevice(gVkFuncs);              // The logical device used for Vulkan
static uint32_t                     gDrawSampleCount;               // The number of samples to use when drawing (if > 1 then MSAA is active)
static vgl::Swapchain               gSwapchain;                     // The swapchain we present to
static VRenderPass                  gRenderPass;                    // The single render pass used for drawing for the new native Vulkan renderer
static vgl::CmdBufferRecorder       gCmdBufferRec(gVkFuncs);        // Command buffer recorder helper object

// Color attachments and framebuffers for the new native Vulkan renderer - one per ringbuffer slot.
// Note that the framebuffer might contain an additional MSAA resolve attachment (owned by the MSAA resolver) if that feature is active.
static vgl::RenderTexture   gFbColorAttachments[vgl::Defines::RINGBUFFER_SIZE];
static vgl::Framebuffer     gFramebuffers[vgl::Defines::RINGBUFFER_SIZE];

// Semaphores signalled when the acquired swapchain image is ready.
// One per ringbuffer slot, so we don't disturb a frame that is still being processed.
static vgl::Semaphore gSwapImageReadySemaphores[vgl::Defines::RINGBUFFER_SIZE];

// Semaphore signalled when rendering is done.
// One per ringbuffer slot, so we don't disturb a frame that is still being processed.
static vgl::Semaphore gRenderDoneSemaphores[vgl::Defines::RINGBUFFER_SIZE];

// Primary command buffers used for drawing operations in each frame.
// One for each ringbuffer slot, so we can record a new buffer while a previous frame's buffer is still executing.
static vgl::CmdBuffer gCmdBuffers[vgl::Defines::RINGBUFFER_SIZE];

// Which command buffer we are using for this frame.
// This will be null if we didn't acquire a framebuffer for this frame (zero sized window):
static vgl::CmdBuffer* gpCurCmdBuffer;

// PSX renderer framebuffers, as copied from the PSX GPU.
// These are blitted onto the current framebuffer image.
// One for each ringbuffer slot, so we can update while a previous frame's image is still blitting to the screen.
static vgl::MutableTexture gPsxFramebufferTextures[vgl::Defines::RINGBUFFER_SIZE];

// A mirrored copy of PSX VRAM (minus framebuffers) so we can access in the new Vulkan renderer.
// Any texture uploads to PSX VRAM will get passed along from LIBGPU and eventually find their way in here.
static vgl::Texture gPsxVramTexture;

//------------------------------------------------------------------------------------------------------------------------------------------
// Decides the multisample anti-aliasing sample count based on user preferences and hardware capabilities
//------------------------------------------------------------------------------------------------------------------------------------------
static void decideDrawSampleCount() noexcept {
    // Must do at least 1 sample
    const int32_t wantedNumSamples = std::max(Config::gAAMultisamples, 1);

    // Vulkan specifies up to 64x MSAA currently...
    const VkPhysicalDeviceLimits& deviceLimits = gpPhysicalDevice->getProps().limits;
    const VkSampleCountFlags deviceMsaaModes = deviceLimits.framebufferColorSampleCounts;

    if ((wantedNumSamples >= 64) && (deviceMsaaModes & VK_SAMPLE_COUNT_64_BIT)) {
        gDrawSampleCount = 64;
    } else if ((wantedNumSamples >= 32) && (deviceMsaaModes & VK_SAMPLE_COUNT_32_BIT)) {
        gDrawSampleCount = 32;
    } else if ((wantedNumSamples >= 16) && (deviceMsaaModes & VK_SAMPLE_COUNT_16_BIT)) {
        gDrawSampleCount = 16;
    } else if ((wantedNumSamples >= 8) && (deviceMsaaModes & VK_SAMPLE_COUNT_8_BIT)) {
        gDrawSampleCount = 8;
    } else if ((wantedNumSamples >= 4) && (deviceMsaaModes & VK_SAMPLE_COUNT_4_BIT)) {
        gDrawSampleCount = 4;
    } else if ((wantedNumSamples >= 2) && (deviceMsaaModes & VK_SAMPLE_COUNT_2_BIT)) {
        gDrawSampleCount = 2;
    } else {
        gDrawSampleCount = 1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys all framebuffers immediately
//------------------------------------------------------------------------------------------------------------------------------------------
static void destroyFramebuffers() noexcept {
    for (uint32_t i = 0; i < vgl::Defines::RINGBUFFER_SIZE; ++i) {
        gFramebuffers[i].destroy(true);
        gFbColorAttachments[i].destroy(true);
    }

    VMsaaResolver::destroyResolveColorAttachments();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recreates the swapchain and new Vulkan renderer framebuffers if required and returns 'false' if that is not possible to do currently.
// Recreation might fail if the window is currently zero sized for example.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool ensureValidSwapchainAndFramebuffers() noexcept {
    // No swapchain or invalid swapchain?
    bool bDidWaitForDeviceIdle = false;

    if ((!gSwapchain.isValid()) || gSwapchain.needsRecreate()) {
        gDevice.waitUntilDeviceIdle();
        bDidWaitForDeviceIdle = true;
        gSwapchain.destroy();

        if (!gSwapchain.init(gDevice, ALLOWED_COLOR_SURFACE_FORMATS, C_ARRAY_SIZE(ALLOWED_COLOR_SURFACE_FORMATS)))
            return false;
    }

    // No framebuffers or framebuffer size incorrect?
    const uint32_t wantedFbWidth = gSwapchain.getSwapExtentWidth();
    const uint32_t wantedFbHeight = gSwapchain.getSwapExtentHeight();

    const bool bNeedNewFramebuffers = (
        (!gFramebuffers[0].isValid()) ||
        (gFramebuffers[0].getWidth() != wantedFbWidth) ||
        (gFramebuffers[0].getHeight() != wantedFbHeight)
    );

    if (!bNeedNewFramebuffers)
        return true;

    // Need new framebuffers: wait for the device to become idle firstly so that old framebuffers are unused
    if (!bDidWaitForDeviceIdle) {
        gDevice.waitUntilDeviceIdle();
        bDidWaitForDeviceIdle = true;
    }

    // Destroy the old framebuffers and attachments, then create the new ones
    destroyFramebuffers();
    const bool bDoingMsaa = (gDrawSampleCount > 1);

    if (bDoingMsaa) {
        if (!VMsaaResolver::createResolveColorAttachments(gDevice, COLOR_32_FORMAT, wantedFbWidth, wantedFbHeight)) {
            destroyFramebuffers();
            return false;
        }
    }

    for (uint32_t i = 0; i < vgl::Defines::RINGBUFFER_SIZE; ++i) {
        // Color attachment can either be used as a transfer source (for blits, no MSAA) or an input attachment for MSAA resolve
        const VkImageUsageFlags colorAttachUsage = (
            ((bDoingMsaa) ? VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT : VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
        );

        if (!gFbColorAttachments[i].initAsRenderTexture(gDevice, COLOR_16_FORMAT, colorAttachUsage, wantedFbWidth, wantedFbHeight, gDrawSampleCount)) {
            destroyFramebuffers();
            return false;
        }

        std::vector<const vgl::BaseTexture*> fbAttachments;
        fbAttachments.reserve(3);
        fbAttachments.push_back(&gFbColorAttachments[i]);

        if (bDoingMsaa) {
            fbAttachments.push_back(&VMsaaResolver::gResolveColorAttachments[i]);
        }

        if (!gFramebuffers[i].init(gRenderPass, fbAttachments)) {
            destroyFramebuffers();
            return false;
        }
    }

    // Need to set the input attachments for the MSAA resolver after creating the framebuffer attachments
    if (bDoingMsaa) {
        VMsaaResolver::setMsaaResolveInputAttachments(gFbColorAttachments);
    }

    // All good if we got to here!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frame begin logic for the classic PSX renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void beginFrame_Psx() noexcept {
    ASSERT(gSwapchain.getAcquiredImageIdx() < gSwapchain.getLength());

    // Transition the PSX framebuffer to general in preparation for writing
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();

    {
        vgl::MutableTexture& psxFbTexture = gPsxFramebufferTextures[ringbufferIdx];

        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imgBarrier.image = psxFbTexture.getVkImage();
        imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgBarrier.subresourceRange.levelCount = 1;
        imgBarrier.subresourceRange.layerCount = 1;

        gCmdBufferRec.addPipelineBarrier(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            nullptr,
            1,
            &imgBarrier
        );
    }

    // Clear the swapchain image to opaque black.
    // This is needed for the classic PSX renderer as not all parts of the image might be filled.
    {
        const uint32_t swapchainIdx = gSwapchain.getAcquiredImageIdx();
        const VkImage swapchainImage = gSwapchain.getVkImages()[swapchainIdx];

        VkClearColorValue clearColor = {};
        clearColor.float32[3] = 1.0f;

        VkImageSubresourceRange imageResRange = {};
        imageResRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageResRange.levelCount = 1;
        imageResRange.layerCount = 1;

        gCmdBufferRec.clearColorImage(swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageResRange);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frame end logic for the classic PSX renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void endFrame_Psx() noexcept {
    // Copy the PlayStation 1 framebuffer to the current framebuffer texture
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
    vgl::MutableTexture& psxFbTexture = gPsxFramebufferTextures[ringbufferIdx];

    {
        Gpu::Core& gpu = PsxVm::gGpu;
        const uint16_t* pSrcRowPixels = gpu.pRam + (gpu.displayAreaX + (uintptr_t) gpu.displayAreaY * gpu.ramPixelW);
        uint16_t* pDstRowPixels = (uint16_t*) psxFbTexture.getBytes();

        for (uint32_t y = 0; y < Video::ORIG_DRAW_RES_Y; ++y) {
            // Note: don't bother doing multiple pixels at a time - compiler is smart and already optimizes this to use SIMD
            for (uint32_t x = 0; x < Video::ORIG_DRAW_RES_X; ++x) {
                const uint16_t srcPixel = pSrcRowPixels[x];
                const uint16_t srcR = (srcPixel >>  0) & 0x1F;
                const uint16_t srcG = (srcPixel >>  5) & 0x1F;
                const uint16_t srcB = (srcPixel >> 10) & 0x1F;

                const uint16_t dstPixel = (srcR << 10) | (srcG << 5) | (srcB << 0) | 0x8000;
                pDstRowPixels[x] = dstPixel;
            }

            pSrcRowPixels += gpu.ramPixelW;
            pDstRowPixels += Video::ORIG_DRAW_RES_X;
        }
    }

    // Get the area of the window to blit the PSX framebuffer to
    const uint32_t screenWidth = gSwapchain.getSwapExtentWidth();
    const uint32_t screenHeight = gSwapchain.getSwapExtentHeight();

    int32_t blitDstX = {};
    int32_t blitDstY = {};
    uint32_t blitDstW = {};
    uint32_t blitDstH = {};
    Video::getClassicFramebufferWindowRect(screenWidth, screenHeight, blitDstX, blitDstY, blitDstW, blitDstH);

    // Blit the PSX framebuffer to the swapchain image
    const uint32_t swapchainIdx = gSwapchain.getAcquiredImageIdx();
    const VkImage swapchainImage = gSwapchain.getVkImages()[swapchainIdx];

    {
        VkImageBlit blitRegion = {};
        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.srcOffsets[1].x = Video::ORIG_DRAW_RES_X;
        blitRegion.srcOffsets[1].y = Video::ORIG_DRAW_RES_Y;
        blitRegion.srcOffsets[1].z = 1;
        blitRegion.dstOffsets[0].x = std::clamp<int32_t>(blitDstX, 0, screenWidth);
        blitRegion.dstOffsets[0].y = std::clamp<int32_t>(blitDstY, 0, screenHeight);
        blitRegion.dstOffsets[1].x = std::clamp<int32_t>(blitDstX + blitDstW, 0, screenWidth);
        blitRegion.dstOffsets[1].y = std::clamp<int32_t>(blitDstY + blitDstH, 0, screenHeight);
        blitRegion.dstOffsets[1].z = 1;

        gCmdBufferRec.blitImage(
            psxFbTexture.getVkImage(),
            VK_IMAGE_LAYOUT_GENERAL,
            swapchainImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blitRegion,
            VK_FILTER_NEAREST
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frame begin logic for the new Vulkan renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void beginFrame_VkRenderer() noexcept {
    // Begin the render pass and clear all attachments
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
    vgl::Framebuffer& framebuffer = gFramebuffers[ringbufferIdx];

    VkClearValue framebufferClearValues[1] = {};

    gCmdBufferRec.beginRenderPass(
        gRenderPass,
        framebuffer,
        VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS,
        0,
        0,
        framebuffer.getWidth(),
        framebuffer.getHeight(),
        framebufferClearValues,
        2
    );

    // Begin a frame for the drawing module
    VDrawing::beginFrame(ringbufferIdx, gRenderPass, framebuffer);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frame end logic for the new Vulkan renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void endFrame_VkRenderer() noexcept {
    // Finish up drawing for the 'occluder plane' and regular 'draw' subpasses
    VDrawing::endFrame(gCmdBufferRec);

    // Do an MSAA resolve subpass if MSAA is enabled
    if (gDrawSampleCount > 1) {
        gCmdBufferRec.nextSubpass(VK_SUBPASS_CONTENTS_INLINE);
        VMsaaResolver::doMsaaResolve(gDevice, gCmdBufferRec);
    }

    // Done with the render pass now
    gCmdBufferRec.endRenderPass();
    
    // Blit the drawing color attachment (or MSAA resolve target, if MSAA is active) to the swapchain image
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
    const vgl::Framebuffer& framebuffer = gFramebuffers[ringbufferIdx];

    const VkImage blitSrcImage = (gDrawSampleCount > 1) ? 
        VMsaaResolver::gResolveColorAttachments[ringbufferIdx].getVkImage() :   // Blit from the MSAA resolve color buffer
        framebuffer.getAttachmentImages()[0];                                   // No MSAA: blit directly from the color buffer that was drawn to

    const uint32_t swapchainIdx = gSwapchain.getAcquiredImageIdx();
    const VkImage blitDstImage = gSwapchain.getVkImages()[swapchainIdx];

    {
        VkImageBlit blitRegion = {};
        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.srcOffsets[1].x = framebuffer.getWidth();
        blitRegion.srcOffsets[1].y = framebuffer.getHeight();
        blitRegion.srcOffsets[1].z = 1;
        blitRegion.dstOffsets[1].x = gSwapchain.getSwapExtentWidth();
        blitRegion.dstOffsets[1].y = gSwapchain.getSwapExtentHeight();
        blitRegion.dstOffsets[1].z = 1;

        gCmdBufferRec.blitImage(
            blitSrcImage,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            blitDstImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blitRegion,
            VK_FILTER_NEAREST
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update the values which help convert from normalized device coordinates to the original PSX framebuffer coordinate system (256x240).
//------------------------------------------------------------------------------------------------------------------------------------------
void static updateCoordSysConversions() noexcept {
    // Get the area of the window that the original PSX framebuffer would be blitted to for the classic renderer
    const uint32_t screenWidth = gSwapchain.getSwapExtentWidth();
    const uint32_t screenHeight = gSwapchain.getSwapExtentHeight();

    int32_t blitDstX = {};
    int32_t blitDstY = {};
    uint32_t blitDstW = {};
    uint32_t blitDstH = {};
    Video::getClassicFramebufferWindowRect(screenWidth, screenHeight, blitDstX, blitDstY, blitDstW, blitDstH);

    // Compute all the scalings and offsets from this.
    // If there is space leftover at the sides or top and bottom then the original PSX framebuffer would be centered.
    const float blitWPercent = (float) blitDstW / (float) screenWidth;
    const float blitHPercent = (float) blitDstH / (float) screenHeight;
    gNdcToPsxScaleX = (0.5f / blitWPercent) * Video::ORIG_DISP_RES_X;
    gNdcToPsxScaleY = (0.5f / blitHPercent) * Video::ORIG_DISP_RES_Y;

    const float blitStartXPercent = (float) blitDstX / screenWidth;
    const float blitStartYPercent = (float) blitDstY / screenHeight;
    gPsxNdcOffsetX = blitStartXPercent * 2.0f;
    gPsxNdcOffsetY = blitStartYPercent * 2.0f;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes Vulkan for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    // Initialize the Vulkan API, the window surface, and choose a device to use
    if (!gVulkanInstance.init(Video::gpSdlWindow))
        FatalErrors::raise("Failed to initialize a Vulkan API instance!");

    if (!gWindowSurface.init(Video::gpSdlWindow, gVulkanInstance))
        FatalErrors::raise("Failed to initialize a Vulkan window surface!");

    gpPhysicalDevice = vgl::PhysicalDeviceSelection::selectBestDevice(gVulkanInstance.getPhysicalDevices(), gWindowSurface);

    if (!gpPhysicalDevice)
        FatalErrors::raise("Failed to find a suitable rendering device for use with Vulkan!");

    // Initialize the logical Vulkan device used for commands and operations and then decide draw sample count
    if (!gDevice.init(*gpPhysicalDevice, &gWindowSurface))
        FatalErrors::raise("Failed to initialize a Vulkan logical device!");

    decideDrawSampleCount();

    // Initialize the draw render pass for the new renderer
    if (!gRenderPass.init(gDevice, COLOR_16_FORMAT, COLOR_32_FORMAT, gDrawSampleCount))
        FatalErrors::raise("Failed to create the Vulkan draw render pass!");

    // Create pipelines
    VPipelines::init(gDevice, gRenderPass, gDrawSampleCount);

    // Init the MSAA resolver (if using MSAA)
    if (gDrawSampleCount > 1) {
        VMsaaResolver::init(gDevice);
    }

    // Try to init the swapchain and framebuffers
    ensureValidSwapchainAndFramebuffers();

    // Initialize various semaphores
    for (vgl::Semaphore& semaphore : gSwapImageReadySemaphores) {
        if (!semaphore.init(gDevice))
            FatalErrors::raise("Failed to create a Vulkan 'swapchain image ready' semaphore!");
    }

    for (vgl::Semaphore& semaphore : gRenderDoneSemaphores) {
        if (!semaphore.init(gDevice))
            FatalErrors::raise("Failed to create the Vulkan 'render done' semaphore!");
    }

    // Init the command buffers
    for (vgl::CmdBuffer& cmdBuffer : gCmdBuffers) {
        if (!cmdBuffer.init(gDevice.getCmdPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY))
            FatalErrors::raise("Failed to create a Vulkan command buffer required for rendering!");
    }

    // Initialize the PSX framebuffer textures used to hold the old PSX renderer framebuffer before it is blit to the Vulkan framebuffer
    for (vgl::MutableTexture& psxFbTex : gPsxFramebufferTextures) {
        if (!psxFbTex.initAs2dTexture(gDevice, COLOR_16_FORMAT, Video::ORIG_DRAW_RES_X, Video::ORIG_DRAW_RES_Y))
            FatalErrors::raise("Failed to create a Vulkan texture for the classic PSX renderer's framebuffer!");
    }

    // Initialize the texture representing PSX VRAM and clear it all to black.
    // Note: use the R16 UINT format as we will have to unpack in the shader depending on the PSX texture format being used.
    Gpu::Core& psxGpu = PsxVm::gGpu;

    if (!gPsxVramTexture.initAs2dTexture(gDevice, VK_FORMAT_R16_UINT, psxGpu.ramPixelW, psxGpu.ramPixelH))
        FatalErrors::raise("Failed to create a Vulkan texture for PSX VRAM!");

    {
        std::byte* const pBytes = gPsxVramTexture.lock();
        std::memset(pBytes, 0, gPsxVramTexture.getLockedSizeInBytes());
        gPsxVramTexture.unlock();
    }

    // Initialize the main draw command submission module
    VDrawing::init(gDevice, gPsxVramTexture);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tears down Vulkan for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
void destroy() noexcept {
    // Finish up the current frame if we begun it
    if (gbDidBeginFrame) {
        endFrame();
    }

    // Wait for the device to finish before proceeding
    if (gDevice.isValid()) {
        gDevice.waitUntilDeviceIdle();
    }

    // Tear everything down and make sure to destroy immediate where we have the option
    VDrawing::shutdown();
    gPsxVramTexture.destroy(true);

    for (vgl::MutableTexture& fbTexture : gPsxFramebufferTextures) {
        fbTexture.destroy(true);
    }

    for (vgl::CmdBuffer& cmdBuffer : gCmdBuffers) {
        cmdBuffer.destroy(true);
    }

    for (vgl::Semaphore& semaphore : gRenderDoneSemaphores) {
        semaphore.destroy();
    }

    for (vgl::Semaphore& semaphore : gSwapImageReadySemaphores) {
        semaphore.destroy();
    }

    destroyFramebuffers();
    VMsaaResolver::destroy();
    VPipelines::shutdown();
    gRenderPass.destroy();
    gSwapchain.destroy();
    gDevice.destroy();
    gpPhysicalDevice = nullptr;
    gWindowSurface.destroy();
    gVulkanInstance.destroy();
    gVkFuncs = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform tasks that need to be done on frame startup.
// Returns 'true' if a frame was successfully started and a draw can proceed, 'false' if no drawing should take place (invalid framebuffer).
//------------------------------------------------------------------------------------------------------------------------------------------
bool beginFrame() noexcept {
    // Must have ended the frame or not started one previously
    ASSERT(!gbDidBeginFrame);
    gbDidBeginFrame = true;

    // Recreate the swapchain and framebuffers if required and bail if that operation failed
    if (!ensureValidSwapchainAndFramebuffers())
        return false;

    // Acquire a swapchain image and bail if that failed
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
    const uint32_t swapchainIdx = gSwapchain.acquireImage(gSwapImageReadySemaphores[ringbufferIdx]);

    if (swapchainIdx == vgl::Swapchain::INVALID_IMAGE_IDX)
        return false;

    // Begin recording the command buffer for this frame
    gCmdBufferRec.beginPrimaryCmdBuffer(gCmdBuffers[ringbufferIdx], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    // The first command is to wait for any transfers to finish
    {
        VkMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

        gCmdBufferRec.addPipelineBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            1,
            &barrier,
            0,
            nullptr
        );
    }

    // Transition the swapchain image to transfer destination optimal in preparation for blitting.
    // This is needed by both the classic PSX renderer and the new Vulkan renderer.
    {
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgBarrier.image = gSwapchain.getVkImages()[swapchainIdx];
        imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgBarrier.subresourceRange.levelCount = 1;
        imgBarrier.subresourceRange.layerCount = 1;

        gCmdBufferRec.addPipelineBarrier(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            nullptr,
            1,
            &imgBarrier
        );
    }

    // Update coordinate system conversions for drawing code that needs it
    updateCoordSysConversions();

    // Renderer specific initialization
    if (gbUsePsxRenderer) {
        beginFrame_Psx();
    } else {
        beginFrame_VkRenderer();
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if draw commands can currently be submitted.
// This will be the case if we have valid framebuffers and successfully started a frame.
//------------------------------------------------------------------------------------------------------------------------------------------
bool canSubmitDrawCmds() noexcept {
    return gCmdBufferRec.isRecording();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// End the current frame and present to the screen
//------------------------------------------------------------------------------------------------------------------------------------------
void endFrame() noexcept {
    // Must have begun the frame
    ASSERT(gbDidBeginFrame);
    gbDidBeginFrame = false;

    // Renderer specific finishing up, if we are drawing.
    // May cause transfer tasks to be kicked off.
    if (gCmdBufferRec.isRecording()) {
        if (gbUsePsxRenderer) {
            endFrame_Psx();
        } else {
            endFrame_VkRenderer();
        }
    }

    // Begin executing any pending transfers
    vgl::TransferMgr& transferMgr = gDevice.getTransferMgr();
    transferMgr.executePreFrameTransferTask();

    // If we do not have a framebuffer (and hence no command buffer) then simply wait until transfers have finished (device idle) and exit
    if (!gCmdBufferRec.isRecording()) {
        gDevice.waitUntilDeviceIdle();
        return;
    }

    // Transition the swapchain image back to presentation optimal in preparation for presentation
    const uint32_t swapchainIdx = gSwapchain.getAcquiredImageIdx();

    {
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imgBarrier.image = gSwapchain.getVkImages()[swapchainIdx];
        imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgBarrier.subresourceRange.levelCount = 1;
        imgBarrier.subresourceRange.layerCount = 1;

        gCmdBufferRec.addPipelineBarrier(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            nullptr,
            1,
            &imgBarrier
        );
    }

    // End command recording and submit the command buffer to the device.
    // Wait for the current swapchain image to be acquired before executing this command buffer.
    // Signal the current ringbuffer slot fence when drawing is done.
    gCmdBufferRec.endCmdBuffer();
    
    vgl::RingbufferMgr& ringbufferMgr = gDevice.getRingbufferMgr();
    const uint32_t ringbufferIdx = ringbufferMgr.getBufferIndex();
    
    {
        vgl::CmdBufferWaitCond cmdsWaitCond = {};
        cmdsWaitCond.pSemaphore = &gSwapImageReadySemaphores[ringbufferIdx];
        cmdsWaitCond.blockedStageFlags = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

        vgl::Fence& ringbufferSlotFence = ringbufferMgr.getCurrentBufferFence();
        gDevice.submitCmdBuffer(
            gCmdBuffers[ringbufferIdx],
            { cmdsWaitCond },
            &gRenderDoneSemaphores[ringbufferIdx],  // Note: signal render done semaphore and the ringbuffer slot fence when finished
            &ringbufferSlotFence
        );
    }

    // Present the current swapchain image once all the commands have finished
    gSwapchain.presentAcquiredImage(gRenderDoneSemaphores[ringbufferIdx]);

    // Move onto the next ringbuffer index and clear the command buffer used: will get it again once we begin a frame
    ringbufferMgr.acquireNextBuffer();

    // TODO: remove temp/hack renderer toggle and implement properly
    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_F9)) {
        gbUsePsxRenderer = false;
    }
    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_F10)) {
        gbUsePsxRenderer = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copies a rectangular area of pixels (of at least 1x1 pixels) from the PSX GPU's VRAM to the Vulkan texture that mirrors it.
// This makes updates to PSX VRAM visible to the new native Vulkan renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
void pushPsxVramUpdates(const uint16_t rectLx, const uint16_t rectRx, const uint16_t rectTy, const uint16_t rectBy) noexcept {
    // Sanity check the rectangle bounds.
    // Note that the rect IS allowed to wraparound to the other side of VRAM, but the coordinates must remain within range.
    // Wraparound is allowed because PSX VRAM writes wrap and LIBGPU supports that.
    Gpu::Core& psxGpu = PsxVm::gGpu;
    const uint32_t vramW = psxGpu.ramPixelW;
    const uint32_t vramH = psxGpu.ramPixelH;

    ASSERT(rectLx < vramW);
    ASSERT(rectRx < vramW);
    ASSERT(rectTy < vramH);
    ASSERT(rectBy < vramH);

    // Does the update rect wrap horizontally?
    // If so then split up into two non-wrapping rectangles.
    if (rectRx < rectLx) {
        pushPsxVramUpdates(0, rectRx, rectTy, rectBy);
        pushPsxVramUpdates(rectLx, (uint16_t)(vramW - 1), rectTy, rectBy);
        return;
    }

    // Does the update rect wrap vertically?
    // If so then split up into two non-wrapping rectangles.
    if (rectBy < rectTy) {
        pushPsxVramUpdates(rectLx, rectRx, 0, rectBy);
        pushPsxVramUpdates(rectLx, rectRx, rectTy, (uint16_t)(vramH - 1));
        return;
    }

    // This is the size of the area to be updated and the number of bytes in each row
    const uint32_t copyRectW = (uint32_t) rectRx + 1 - rectLx;
    const uint32_t copyRectH = (uint32_t) rectBy + 1 - rectTy;
    const uint32_t copyRowSize = copyRectW * sizeof(uint16_t);

    // Lock the required region of the texture and copy in the updates, row by row
    uint16_t* pDstBytes = (uint16_t*) gPsxVramTexture.lock(rectLx, rectTy, 0, 0, copyRectW, copyRectH, 1, 1);
    const uint16_t* pSrcBytes = psxGpu.pRam + rectLx + ((uintptr_t) rectTy * vramW);

    for (uint32_t row = 0; row < copyRectH; ++row) {
        std::memcpy(pDstBytes, pSrcBytes, copyRowSize);
        pDstBytes += copyRectW;
        pSrcBytes += vramW;
    }

    // Unlock the texture to begin uploading the updates
    gPsxVramTexture.unlock();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the width and height of the framebuffer used for the native Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t getVkRendererFbWidth() noexcept {
    return (gFramebuffers[0].isValid()) ? gFramebuffers[0].getWidth() : 0;
}

uint32_t getVkRendererFbHeight() noexcept {
    return (gFramebuffers[0].isValid()) ? gFramebuffers[0].getHeight() : 0;
}

END_NAMESPACE(VRenderer)

#endif  // #if PSYDOOM_VULKAN_RENDERER
