//------------------------------------------------------------------------------------------------------------------------------------------
// Manages the low level details of interacting with Vulkan for the new Vulkan renderer, as well as the classic renderer output via Vulkan.
// Sets up key Vulkan objects and provides various Vulkan utility code.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Vulkan.h"

#if PSYDOOM_VULKAN_RENDERER

#include "CmdBufferRecorder.h"
#include "CmdBufferWaitCond.h"
#include "FatalErrors.h"
#include "Framebuffer.h"
#include "Gpu.h"
#include "LogicalDevice.h"
#include "MutableTexture.h"
#include "PhysicalDeviceSelection.h"
#include "PsxVm.h"
#include "Semaphore.h"
#include "Video.h"
#include "VkFuncs.h"
#include "Vulkan/VDrawRenderPass.h"
#include "Vulkan/VPresentRenderPass.h"
#include "VulkanInstance.h"
#include "WindowSurface.h"

BEGIN_NAMESPACE(Vulkan)

bool                            gbDidBeginFrame;                // Whether a frame was begun
bool                            gbUsePsxRenderer = true;        // Use the classic PSX renderer? In this case just blit the PSX framebuffer to the screen.
vgl::VkFuncs                    gVkFuncs;                       // Cached pointers to all Vulkan API functions
vgl::VulkanInstance             gVulkanInstance(gVkFuncs);      // Vulkan API instance object
vgl::WindowSurface              gWindowSurface;                 // Window surface to draw on
const vgl::PhysicalDevice*      gpPhysicalDevice;               // Physical device chosen for rendering
vgl::LogicalDevice              gDevice(gVkFuncs);              // The logical device used for Vulkan

static VDrawRenderPass          gDrawRenderPass;                // Render pass used for drawing
static VPresentRenderPass       gPresentRenderPass;             // Render pass used for presentation
static vgl::Semaphore           gFramebufferReadySignal;        // Semaphore signalled when the framebuffer is ready
static vgl::Semaphore           gRenderDoneSignal;              // Semaphore signalled when rendering is done
static vgl::CmdBufferRecorder   gCmdBufferRec(gVkFuncs);        // Command buffer recorder helper object

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

//------------------------------------------------------------------------------------------------------------------------------------------
// Frame begin logic for the classic PSX renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void beginFrame_Psx() noexcept {
    ASSERT(gpCurCmdBuffer);

    // Transition the swapchain framebuffer to transfer destination optimal in preparation for blitting
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
    const VkImage framebufferImage = gDevice.getScreenFramebufferMgr().getCurrentDrawFramebuffer().getAttachmentImages()[0];

    {
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgBarrier.image = framebufferImage;
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
        vgl::MutableTexture& fbTexture = gPsxFramebufferTextures[ringbufferIdx];

        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imgBarrier.image = fbTexture.getVkImage();
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

    // Clear the screen to opaque black
    {
        VkClearColorValue clearColor = {};
        clearColor.float32[3] = 1.0f;

        VkImageSubresourceRange imageResRange = {};
        imageResRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageResRange.levelCount = 1;
        imageResRange.layerCount = 1;

        gCmdBufferRec.clearColorImage(framebufferImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageResRange);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frame end logic for the classic PSX renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void endFrame_Psx() noexcept {
    ASSERT(gpCurCmdBuffer);

    // Copy the PlayStation 1 framebuffer to the current framebuffer texture, row by row
    const uint32_t ringbufferIdx = gDevice.getRingbufferMgr().getBufferIndex();
    vgl::MutableTexture& fbTexture = gPsxFramebufferTextures[ringbufferIdx];

    {
        Gpu::Core& gpu = PsxVm::gGpu;
        const uint16_t* pSrcRowPixels = gpu.pRam + (gpu.displayAreaX + (uintptr_t) gpu.displayAreaY * gpu.ramPixelW);
        uint16_t* pDstRowPixels = (uint16_t*) fbTexture.getBytes();

        for (uint32_t y = 0; y < Video::ORIG_DRAW_RES_Y; ++y) {
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

    // Blit the PSX framebuffer to the swapchain framebuffer
    vgl::ScreenFramebufferMgr& framebufferMgr = gDevice.getScreenFramebufferMgr();
    const VkImage framebufferImage = framebufferMgr.getCurrentDrawFramebuffer().getAttachmentImages()[0];

    {
        VkImageBlit blitRegion = {};
        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.srcOffsets[1].x = Video::ORIG_DRAW_RES_X;
        blitRegion.srcOffsets[1].y = Video::ORIG_DRAW_RES_Y;
        blitRegion.srcOffsets[1].z = 1;
        // TODO: scale and position the image appropriately
        blitRegion.dstOffsets[1].x = framebufferMgr.getFramebuffersWidth();
        blitRegion.dstOffsets[1].y = framebufferMgr.getFramebuffersHeight();
        blitRegion.dstOffsets[1].z = 1;

        gCmdBufferRec.blitImage(
            fbTexture.getVkImage(),
            VK_IMAGE_LAYOUT_GENERAL,
            framebufferImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blitRegion,
            VK_FILTER_NEAREST
        );
    }

    // Transition the framebuffer to presentation optimal
    {
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imgBarrier.image = framebufferImage;
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
    // TODO...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frame end logic for the new Vulkan renderer.
// Must be called only when there is a valid framebuffer and hence command buffer.
//------------------------------------------------------------------------------------------------------------------------------------------
static void endFrame_VkRenderer() noexcept {
    ASSERT(gpCurCmdBuffer);
    // TODO...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes Vulkan for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    // Initialize the Vulkan API, the window surface, and choose a device to use
    if (!gVulkanInstance.init(Video::gpSdlWindow)) {
        FatalErrors::raise("Failed to initialize a Vulkan API instance!");
        return;
    }

    if (!gWindowSurface.init(Video::gpSdlWindow, gVulkanInstance)) {
        FatalErrors::raise("Failed to initialize a Vulkan window surface!");
        return;
    }

    gpPhysicalDevice = vgl::PhysicalDeviceSelection::selectBestDevice(gVulkanInstance.getPhysicalDevices(), gWindowSurface);

    if (!gpPhysicalDevice) {
        FatalErrors::raise("Failed to find a suitable rendering device for use with Vulkan!");
        return;
    }

    // Initialize the logical Vulkan device is used actual operations
    if (!gDevice.init(*gpPhysicalDevice, &gWindowSurface)) {
        FatalErrors::raise("Failed to initialize a Vulkan logical device!");
        return;
    }

    // Initialize the render passes
    vgl::ScreenFramebufferMgr& framebufferMgr = gDevice.getScreenFramebufferMgr();
    const VkFormat colorFormat = framebufferMgr.getColorFormat();
    const VkFormat depthStencilFormat = framebufferMgr.getDepthStencilFormat();

    gDrawRenderPass.init(gDevice, colorFormat, depthStencilFormat);
    gPresentRenderPass.init(gDevice, colorFormat);

    // Initialize various semaphores
    gFramebufferReadySignal.init(gDevice);
    gRenderDoneSignal.init(gDevice);

    // Init the command buffers
    for (vgl::CmdBuffer& cmdBuffer : gCmdBuffers) {
        cmdBuffer.init(gDevice.getCmdPool(), VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    }

    // Initialize the framebuffer textures used to old the old PSX renderer framebuffer before it is blit to the Vulkan framebuffer
    for (vgl::MutableTexture& fbTexture : gPsxFramebufferTextures) {
        fbTexture.initAs2dTexture(gDevice, VK_FORMAT_A1R5G5B5_UNORM_PACK16, Video::ORIG_DRAW_RES_X, Video::ORIG_DRAW_RES_Y);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tears down Vulkan for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
void destroy() noexcept {
    // Finish up the current frame if there is any
    if (gbDidBeginFrame) {
        endFrame();
    }

    // Wait for the device to finish before proceeding
    if (gDevice.isValid()) {
        gDevice.waitUntilDeviceIdle();
    }

    // Tear everything down and make sure to destroy immediate where we have the option
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

END_NAMESPACE(Vulkan)

#endif  // #if PSYDOOM_VULKAN_RENDERER
