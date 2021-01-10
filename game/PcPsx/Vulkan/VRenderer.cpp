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
#include "Semaphore.h"
#include "Texture.h"
#include "VDrawing.h"
#include "VDrawRenderPass.h"
#include "VkFuncs.h"
#include "VPipelines.h"
#include "VPresentRenderPass.h"
#include "VulkanInstance.h"
#include "WindowSurface.h"

BEGIN_NAMESPACE(VRenderer)

bool gbUsePsxRenderer = true;  // Use the classic PSX renderer? In this case just blit the PSX framebuffer to the screen.

static bool                         gbDidBeginFrame;                // Whether a frame was begun
static vgl::VkFuncs                 gVkFuncs;                       // Cached pointers to all Vulkan API functions
static vgl::VulkanInstance          gVulkanInstance(gVkFuncs);      // Vulkan API instance object
static vgl::WindowSurface           gWindowSurface;                 // Window surface to draw on
static const vgl::PhysicalDevice*   gpPhysicalDevice;               // Physical device chosen for rendering
static vgl::LogicalDevice           gDevice(gVkFuncs);              // The logical device used for Vulkan
static VDrawRenderPass              gDrawRenderPass;                // Render pass used for drawing
static VPresentRenderPass           gPresentRenderPass;             // Render pass used for presentation
static vgl::Semaphore               gFramebufferReadySignal;        // Semaphore signalled when the framebuffer is ready
static vgl::Semaphore               gRenderDoneSignal;              // Semaphore signalled when rendering is done
static vgl::CmdBufferRecorder       gCmdBufferRec(gVkFuncs);        // Command buffer recorder helper object

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
// Frame begin logic for the classic PSX renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void beginFrame_Psx() noexcept {
    ASSERT(gpCurCmdBuffer);

    // Transition the swapchain framebuffer to transfer destination optimal in preparation for blitting
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
    const VkImage fbImage = gDevice.getScreenFramebufferMgr().getCurrentDrawFramebuffer().getAttachmentImages()[0];

    {
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgBarrier.image = fbImage;
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

    // Transition the PSX framebuffer to general in preparation for writing
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

    // Clear the swapchain framebuffer to opaque black
    {
        VkClearColorValue clearColor = {};
        clearColor.float32[3] = 1.0f;

        VkImageSubresourceRange imageResRange = {};
        imageResRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageResRange.levelCount = 1;
        imageResRange.layerCount = 1;

        gCmdBufferRec.clearColorImage(fbImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageResRange);
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
    vgl::ScreenFramebufferMgr& framebufferMgr = gDevice.getScreenFramebufferMgr();
    const uint32_t fbWidth = (int32_t) framebufferMgr.getFramebuffersWidth();
    const uint32_t fbHeight = (int32_t) framebufferMgr.getFramebuffersHeight();

    int32_t blitDstX = {};
    int32_t blitDstY = {};
    uint32_t blitDstW = {};
    uint32_t blitDstH = {};
    Video::getClassicFramebufferWindowRect(fbWidth, fbHeight, blitDstX, blitDstY, blitDstW, blitDstH);

    // Blit the PSX framebuffer to the swapchain framebuffer
    const VkImage fbImage = framebufferMgr.getCurrentDrawFramebuffer().getAttachmentImages()[0];

    {
        VkImageBlit blitRegion = {};
        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.srcOffsets[1].x = Video::ORIG_DRAW_RES_X;
        blitRegion.srcOffsets[1].y = Video::ORIG_DRAW_RES_Y;
        blitRegion.srcOffsets[1].z = 1;
        blitRegion.dstOffsets[0].x = std::clamp<int32_t>(blitDstX, 0, fbWidth);
        blitRegion.dstOffsets[0].y = std::clamp<int32_t>(blitDstY, 0, fbHeight);
        blitRegion.dstOffsets[1].x = std::clamp<int32_t>(blitDstX + blitDstW, 0, fbWidth);
        blitRegion.dstOffsets[1].y = std::clamp<int32_t>(blitDstY + blitDstH, 0, fbHeight);
        blitRegion.dstOffsets[1].z = 1;

        gCmdBufferRec.blitImage(
            psxFbTexture.getVkImage(),
            VK_IMAGE_LAYOUT_GENERAL,
            fbImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blitRegion,
            VK_FILTER_NEAREST
        );
    }

    // Transition the swapchain framebuffer to presentation optimal
    {
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imgBarrier.image = fbImage;
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
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frame begin logic for the new Vulkan renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void beginFrame_VkRenderer() noexcept {
    ASSERT(gpCurCmdBuffer);

    // Begin the draw render pass
    const vgl::Framebuffer& framebuffer = gDevice.getScreenFramebufferMgr().getCurrentDrawFramebuffer();
    const VkClearValue framebufferClearValues[2] = {};

    gCmdBufferRec.beginRenderPass(
        gDrawRenderPass,
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

    // Initialize the render passes for the new renderer
    vgl::ScreenFramebufferMgr& framebufferMgr = gDevice.getScreenFramebufferMgr();
    const VkFormat colorFormat = framebufferMgr.getColorFormat();
    const VkFormat depthStencilFormat = framebufferMgr.getDepthStencilFormat();

    if (!gDrawRenderPass.init(gDevice, colorFormat, depthStencilFormat))
        FatalErrors::raise("Failed to create the Vulkan 'draw' render pass!");

    if (!gPresentRenderPass.init(gDevice, colorFormat))
        FatalErrors::raise("Failed to create the Vulkan 'present' render pass!");

    // Initialize various semaphores
    if (!gFramebufferReadySignal.init(gDevice))
        FatalErrors::raise("Failed to create the Vulkan 'framebuffer ready' semaphore!");

    if (!gRenderDoneSignal.init(gDevice))
        FatalErrors::raise("Failed to create the Vulkan 'render done' semaphore!");

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
    VPipelines::init(gDevice, gDrawRenderPass);
    VDrawing::init(gDevice);
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

    gRenderDoneSignal.destroy();
    gFramebufferReadySignal.destroy();
    gPresentRenderPass.destroy();
    gDrawRenderPass.destroy();
    gDevice.destroy();
    gpPhysicalDevice = nullptr;
    gWindowSurface.destroy();
    gVulkanInstance.destroy();
    gVkFuncs = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform tasks that need to be done on frame startup
//------------------------------------------------------------------------------------------------------------------------------------------
void beginFrame() noexcept {
    // Must have ended the frame or not started one previously
    ASSERT(!gbDidBeginFrame);
    gbDidBeginFrame = true;

    // Recreate the framebuffer if required and bail if we do not have a valid framebuffer
    vgl::ScreenFramebufferMgr& framebufferMgr = gDevice.getScreenFramebufferMgr();

    if (framebufferMgr.framebuffersNeedRecreate()) {
        if (!framebufferMgr.recreateFramebuffers(gDrawRenderPass, gPresentRenderPass))
            return;
    }

    // Acquire a framebuffer and bail if that failed
    if (!framebufferMgr.acquireFramebuffer(gFramebufferReadySignal))
        return;

    // Begin recording the command buffer for this frame
    const uint8_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
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

    // Renderer specific initialization
    if (gbUsePsxRenderer) {
        beginFrame_Psx();
    } else {
        beginFrame_VkRenderer();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// End the current frame and present to the screen
//------------------------------------------------------------------------------------------------------------------------------------------
void endFrame() noexcept {
    // Must have begun the frame
    ASSERT(gbDidBeginFrame);
    gbDidBeginFrame = false;

    // Begin executing any pending transfers
    vgl::TransferMgr& transferMgr = gDevice.getTransferMgr();
    transferMgr.executePreFrameTransferTask();

    // If we do not have a framebuffer (and hence no command buffer) then simply wait until transfers have finished (device idle) and exit
    if (!gpCurCmdBuffer) {
        gDevice.waitUntilDeviceIdle();
        return;
    }

    // Renderer specific finishing up
    if (gbUsePsxRenderer) {
        endFrame_Psx();
    } else {
        endFrame_VkRenderer();
    }

    // End command recording and submit the command buffer to the device.
    // Wait for the current framebuffer to be acquired before executing this command buffer.
    // Signal the current ringbuffer slot fence when drawing is done.
    gCmdBufferRec.endCmdBuffer();
    vgl::RingbufferMgr& ringbufferMgr = gDevice.getRingbufferMgr();
    
    {
        vgl::CmdBufferWaitCond cmdsWaitCond = {};
        cmdsWaitCond.pSemaphore = &gFramebufferReadySignal;
        cmdsWaitCond.blockedStageFlags = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

        vgl::Fence& ringbufferSlotFence = ringbufferMgr.getCurrentBufferFence();
        gDevice.submitCmdBuffer(*gpCurCmdBuffer, { cmdsWaitCond }, &gRenderDoneSignal, &ringbufferSlotFence);   // Note: signal render done semaphore and the ringbuffer slot fence when finished
    }

    // Present the current framebuffer if we have it and only do present when rendering is done
    vgl::ScreenFramebufferMgr& framebufferMgr = gDevice.getScreenFramebufferMgr();

    if (framebufferMgr.hasValidFramebuffers()) {
        framebufferMgr.presentCurrentFramebuffer(gRenderDoneSignal);
    }

    // Move onto the next ringbuffer index and clear the command buffer used: will get it again once we begin a frame
    ringbufferMgr.acquireNextBuffer();
    gpCurCmdBuffer = nullptr;
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

END_NAMESPACE(VRenderer)

#endif  // #if PSYDOOM_VULKAN_RENDERER
