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
#include "PcPsx/PsxVm.h"
#include "PcPsx/Video.h"
#include "PhysicalDeviceSelection.h"
#include "RenderTexture.h"
#include "Semaphore.h"
#include "Swapchain.h"
#include "Texture.h"
#include "VDrawing.h"
#include "VDrawRenderPass.h"
#include "VkFuncs.h"
#include "VPipelines.h"
#include "VulkanInstance.h"
#include "WindowSurface.h"

// TODO: REMOVE THIS TEMP CODE
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

// TODO: add ways to toggle this
bool gbUsePsxRenderer = false;      // Use the classic PSX renderer? In this case just blit the PSX framebuffer to the screen.

static bool                         gbDidBeginFrame;                // Whether a frame was begun
static vgl::VkFuncs                 gVkFuncs;                       // Cached pointers to all Vulkan API functions
static vgl::VulkanInstance          gVulkanInstance(gVkFuncs);      // Vulkan API instance object
static vgl::WindowSurface           gWindowSurface;                 // Window surface to draw on
static const vgl::PhysicalDevice*   gpPhysicalDevice;               // Physical device chosen for rendering
static vgl::LogicalDevice           gDevice(gVkFuncs);              // The logical device used for Vulkan
static vgl::Swapchain               gSwapchain;                     // The swapchain we present to
static VDrawRenderPass              gRenderPass;                    // The render pass used for drawing for the new native Vulkan renderer
static vgl::CmdBufferRecorder       gCmdBufferRec(gVkFuncs);        // Command buffer recorder helper object

// Color attachments, depth attachments and framebuffers for the new native Vulkan renderer - one per ringbuffer slot.
// The color attachment is blitted to the swapchain image in preparation for final presentation.
static vgl::RenderTexture    gFbColorAttachments[vgl::Defines::RINGBUFFER_SIZE];
static vgl::RenderTexture    gFbDepthAttachments[vgl::Defines::RINGBUFFER_SIZE];
static vgl::Framebuffer      gFramebuffers[vgl::Defines::RINGBUFFER_SIZE];

// Semaphores signalled when the acquired swapchain image is ready.
// One per ringbuffer slot, so we don't disturb a frame that is still being processed.
static vgl::Semaphore gSwapImageReadySemaphores[vgl::Defines::RINGBUFFER_SIZE];

// Semaphore signalled when rendering is done.
// One per ringbuffer slot, so we don't disturb a frame that is still being processed.
static vgl::Semaphore gRenderDoneSemaphores[vgl::Defines::RINGBUFFER_SIZE];

// Command buffers used for drawing operations in each frame.
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
// Destroys all framebuffers immediately
//------------------------------------------------------------------------------------------------------------------------------------------
static void destroyFramebuffers() noexcept {
    for (uint32_t i = 0; i < vgl::Defines::RINGBUFFER_SIZE; ++i) {
        gFramebuffers[i].destroy(true);
        gFbColorAttachments[i].destroy(true);
        gFbDepthAttachments[i].destroy(true);
    }
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

    if (bNeedNewFramebuffers) {
        if (!bDidWaitForDeviceIdle) {
            gDevice.waitUntilDeviceIdle();
            bDidWaitForDeviceIdle = true;
        }

        destroyFramebuffers();

        for (uint32_t i = 0; i < vgl::Defines::RINGBUFFER_SIZE; ++i) {
            if (!gFbColorAttachments[i].initAsRenderTexture(gDevice, VK_FORMAT_A1R5G5B5_UNORM_PACK16, wantedFbWidth, wantedFbHeight)) {
                destroyFramebuffers();
                return false;
            }

            if (!gFbDepthAttachments[i].initAsDepthStencilBuffer(gDevice, VK_FORMAT_D32_SFLOAT, wantedFbWidth, wantedFbHeight)) {
                destroyFramebuffers();
                return false;
            }

            if (!gFramebuffers[i].init(gRenderPass, { &gFbColorAttachments[i], &gFbDepthAttachments[i] })) {
                destroyFramebuffers();
                return false;
            }
        }
    }

    // All good if we got to here!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frame begin logic for the classic PSX renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void beginFrame_Psx() noexcept {
    ASSERT(gpCurCmdBuffer);
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
    ASSERT(gpCurCmdBuffer);

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
    ASSERT(gpCurCmdBuffer);

    // Begin the render pass
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
    const vgl::Framebuffer& framebuffer = gFramebuffers[ringbufferIdx];

    VkClearValue framebufferClearValues[2] = {};
    framebufferClearValues[1].depthStencil.depth = MAX_DEPTH;

    gCmdBufferRec.beginRenderPass(
        gRenderPass,
        framebuffer,
        VK_SUBPASS_CONTENTS_INLINE,
        0,
        0,
        framebuffer.getWidth(),
        framebuffer.getHeight(),
        framebufferClearValues,
        2
    );

    // Begin a frame for the drawing module
    VDrawing::beginFrame(gDevice, gCmdBufferRec);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frame end logic for the new Vulkan renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void endFrame_VkRenderer() noexcept {
    ASSERT(gpCurCmdBuffer);
    
    // End the drawing module frame and finish up the draw render pass
    VDrawing::endFrame();
    gCmdBufferRec.endRenderPass();

    // Blit the framebuffer color attachment to the swapchain image
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
    const vgl::Framebuffer& framebuffer = gFramebuffers[ringbufferIdx];

    const uint32_t swapchainIdx = gSwapchain.getAcquiredImageIdx();
    const VkImage swapchainImage = gSwapchain.getVkImages()[swapchainIdx];

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
            framebuffer.getAttachmentImages()[0],
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            swapchainImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blitRegion,
            VK_FILTER_NEAREST
        );
    }
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

    // Initialize the logical Vulkan device is used actual operations
    if (!gDevice.init(*gpPhysicalDevice, &gWindowSurface))
        FatalErrors::raise("Failed to initialize a Vulkan logical device!");

    // Initialize the draw render pass for the new renderer
    if (!gRenderPass.init(gDevice, VK_FORMAT_A1R5G5B5_UNORM_PACK16, VK_FORMAT_D32_SFLOAT))
        FatalErrors::raise("Failed to create the Vulkan draw render pass!");

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
        if (!psxFbTex.initAs2dTexture(gDevice, VK_FORMAT_A1R5G5B5_UNORM_PACK16, Video::ORIG_DRAW_RES_X, Video::ORIG_DRAW_RES_Y))
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

    // Initialize other Vulkan Renderer modules
    VPipelines::init(gDevice, gRenderPass);
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
    VPipelines::shutdown();
    gPsxVramTexture.destroy(true);

    for (vgl::MutableTexture& fbTexture : gPsxFramebufferTextures) {
        fbTexture.destroy(true);
    }

    gpCurCmdBuffer = nullptr;

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
    gpCurCmdBuffer = &gCmdBuffers[ringbufferIdx];
    gCmdBufferRec.beginPrimaryCmdBuffer(*gpCurCmdBuffer, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

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
    return gpCurCmdBuffer;
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
    if (gpCurCmdBuffer) {
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
    if (!gpCurCmdBuffer) {
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
            *gpCurCmdBuffer,
            { cmdsWaitCond },
            &gRenderDoneSemaphores[ringbufferIdx],  // Note: signal render done semaphore and the ringbuffer slot fence when finished
            &ringbufferSlotFence
        );
    }

    // Present the current swapchain image once all the commands have finished
    gSwapchain.presentAcquiredImage(gRenderDoneSemaphores[ringbufferIdx]);

    // Move onto the next ringbuffer index and clear the command buffer used: will get it again once we begin a frame
    ringbufferMgr.acquireNextBuffer();
    gpCurCmdBuffer = nullptr;

    // TODO: REMOVE THIS TEMP CODE
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
