//------------------------------------------------------------------------------------------------------------------------------------------
// Vulkan renderer, core manager/module.
// Manages high level render paths and details for the frame lifecycle with Vulkan.
// Also creates several key objects, such as the swapchain, the primary command buffer and so on.
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
#include "VCrossfader.h"
#include "VDrawing.h"
#include "VkFuncs.h"
#include "VPipelines.h"
#include "VRenderPath_Crossfade.h"
#include "VRenderPath_Main.h"
#include "VRenderPath_Psx.h"
#include "VulkanInstance.h"
#include "WindowSurface.h"

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

// Cached pointers to all Vulkan API functions
vgl::VkFuncs gVkFuncs;

// Render paths used
VRenderPath_Psx         gRenderPath_Psx;
VRenderPath_Main        gRenderPath_Main;
VRenderPath_Crossfade   gRenderPath_Crossfade;

// A collection of all the render paths
IVRendererPath* const gAllRenderPaths[] = {
    &gRenderPath_Psx,
    &gRenderPath_Main,
    &gRenderPath_Crossfade,
};

// Coord system info: the width and height of the window/swap-chain surface we present to
uint32_t    gPresentSurfaceW;
uint32_t    gPresentSurfaceH;

// Coord system info: the width and height to use for framebuffers created - esentially the render/draw resolution
uint32_t    gFramebufferW;
uint32_t    gFramebufferH;

// Coord system info: where the original PSX GPU framebuffer/coord-system maps to on the draw/render framebuffer
int32_t     gPsxCoordsFbX;
int32_t     gPsxCoordsFbY;
uint32_t    gPsxCoordsFbW;
uint32_t    gPsxCoordsFbH;

// Coord system conversion: scalars to convert from normalized device coords to the original PSX framebuffer coords (256x240).
// Also where the where the original PSX framebuffer coords start in NDC space.
//
// Note: these are updated only at the start of each frame and ONLY if there is a valid framebuffer.
// They should not be used outside of rendering and should only be used if the framebuffer is valid.
float gNdcToPsxScaleX, gNdcToPsxScaleY;
float gPsxNdcOffsetX,  gPsxNdcOffsetY;

static bool                         gbDidBeginFrame;                // Whether a frame was begun
static vgl::VulkanInstance          gVulkanInstance(gVkFuncs);      // Vulkan API instance object
static vgl::WindowSurface           gWindowSurface;                 // Window surface to draw on
static const vgl::PhysicalDevice*   gpPhysicalDevice;               // Physical device chosen for rendering
static VkFormat                     gPresentSurfaceFormat;          // What color format the surface we are presenting to should be in
static VkColorSpaceKHR              gPresentSurfaceColorspace;      // What colorspace the surface we are presenting to should use
static vgl::LogicalDevice           gDevice(gVkFuncs);              // The logical device used for Vulkan
static uint32_t                     gDrawSampleCount;               // The number of samples to use when drawing (if > 1 then MSAA is active)

// The swapchain we present to
vgl::Swapchain gSwapchain;

// Command buffer recorder helper object.
// This begins recording at the start of every frame.
vgl::CmdBufferRecorder gCmdBufferRec(gVkFuncs);

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

// A mirrored copy of PSX VRAM (minus framebuffers) so we can access in the new Vulkan renderer.
// Any texture uploads to PSX VRAM will get passed along from LIBGPU and eventually find their way in here.
static vgl::Texture gPsxVramTexture;

// The current and next frame render paths to use: these should always be valid
static IVRendererPath* gpCurRenderPath;
static IVRendererPath* gpNextRenderPath;

//------------------------------------------------------------------------------------------------------------------------------------------
// Decides on the color format used for the window/presentation surface as well as the colorspace
//------------------------------------------------------------------------------------------------------------------------------------------
static void decidePresentSurfaceFormat() noexcept {
    // Firstly query the device surface capabilities
    vgl::DeviceSurfaceCaps surfaceCaps;

    if (!surfaceCaps.query(*gpPhysicalDevice, gWindowSurface))
        FatalErrors::raise("Failed to query device surface capabilities!");

    // Try to choose a valid surface format from what is available.
    // Note hardcoding the colorspace to deliberately to SRGB - not considering HDR10 etc. for this project.
    gPresentSurfaceFormat = surfaceCaps.getSupportedSurfaceFormat(ALLOWED_COLOR_SURFACE_FORMATS, C_ARRAY_SIZE(ALLOWED_COLOR_SURFACE_FORMATS));
    gPresentSurfaceColorspace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    if (gPresentSurfaceFormat == VK_FORMAT_UNDEFINED)
        FatalErrors::raise("Failed to find a suitable surface format to present with!");
}

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
// Update the resolution to render at and also helper values which aid converting between coordinate systems.
// Updates everything based on the current present surface size.
//------------------------------------------------------------------------------------------------------------------------------------------
static void updateCoordSysInfo() noexcept {
    // Save the size of the present surface
    if (gSwapchain.isValid()) {
        gPresentSurfaceW = gSwapchain.getSwapExtentWidth();
        gPresentSurfaceH = gSwapchain.getSwapExtentHeight();
    } else {
        gPresentSurfaceW = 0;
        gPresentSurfaceH = 0;
    }

    // Save the size of the framebuffer
    const bool bHaveValidPresentSurface = ((gPresentSurfaceW > 0) && (gPresentSurfaceH > 0));

    if (bHaveValidPresentSurface) {
        // TODO: support a render scale setting here...
        gFramebufferW = gPresentSurfaceW;
        gFramebufferH = gPresentSurfaceH;
    } else {
        gFramebufferW = 0;
        gFramebufferH = 0;
    }

    // Get the rectangular region of the framebuffer that the original PlayStation framebuffer/coord-system would map to.
    // This info is also used by the new Vulkan renderer to setup coordinate systems and projection matrices.
    if (bHaveValidPresentSurface) {
        Video::getClassicFramebufferWindowRect(
            gFramebufferW,
            gFramebufferH,
            gPsxCoordsFbX,
            gPsxCoordsFbY,
            gPsxCoordsFbW,
            gPsxCoordsFbH
        );
    } else {
        gPsxCoordsFbX = 0;
        gPsxCoordsFbY = 0;
        gPsxCoordsFbW = 0;
        gPsxCoordsFbH = 0;
    }

    // Compute scalings and offsets to help convert from normalized device coords to original PSX framebuffer (256x240) coords.
    // This is useful for things like sky rendering, which work in NDC space.
    //
    // Notes:
    //  (1) If there is space leftover at the sides (widescreen) then the original PSX framebuffer is centered in NDC space.
    //  (2) I don't allow vertical stretching of the original UI assets, hence the 'Y' axis conversions here are fixed.
    //      The game viewport will be letterboxed (rather than extend) if the view is too long on the vertical axis.
    //      Because of all this, it is assumed the PSX framebuffer uses all of NDC space vertically.
    //
    if (bHaveValidPresentSurface) {
        const float blitWPercent = (float) gPsxCoordsFbW / (float) gPresentSurfaceW;
        gNdcToPsxScaleX = (0.5f / blitWPercent) * Video::ORIG_DRAW_RES_X;
        gNdcToPsxScaleY = 0.5f * Video::ORIG_DRAW_RES_Y;

        const float blitStartXPercent = (float) gPsxCoordsFbX / (float) gPresentSurfaceW;
        gPsxNdcOffsetX = blitStartXPercent * 2.0f;
        gPsxNdcOffsetY = 0;
    } else {
        gNdcToPsxScaleX = 0.0f;
        gNdcToPsxScaleY = 0.0f;
        gPsxNdcOffsetX = 0.0f;
        gPsxNdcOffsetY = 0.0f;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recreates the swapchain and new Vulkan renderer framebuffers if required and returns 'false' if that is not possible to do currently.
// Recreation might fail validly if the window is currently zero sized for example.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool ensureValidSwapchainAndFramebuffers() noexcept {
    // Sanity checks
    ASSERT(gpCurRenderPath);

    // No swapchain or invalid swapchain? If that is the case then try to create or re-create...
    if ((!gSwapchain.isValid()) || gSwapchain.needsRecreate()) {
        gDevice.waitUntilDeviceIdle();
        gSwapchain.destroy();

        if (!gSwapchain.init(gDevice, gPresentSurfaceFormat, gPresentSurfaceColorspace))
            return false;
    }

    // Do we need to update coord system info?
    const uint32_t presentSurfaceW = gSwapchain.getSwapExtentWidth();
    const uint32_t presentSurfaceH = gSwapchain.getSwapExtentHeight();

    if ((presentSurfaceW != gPresentSurfaceW) || (presentSurfaceH != gPresentSurfaceH)) {
        updateCoordSysInfo();
    }

    // Ensure the current render path has valid framebuffers
    ASSERT(gFramebufferW > 0);
    ASSERT(gFramebufferH > 0);
    return gpCurRenderPath->ensureValidFramebuffers(gFramebufferW, gFramebufferH);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes Vulkan for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    // Coord sys info is initially invalid
    updateCoordSysInfo();

    // Initialize the Vulkan API, the window surface, and choose a device to use
    if (!gVulkanInstance.init(Video::gpSdlWindow))
        FatalErrors::raise("Failed to initialize a Vulkan API instance!");

    if (!gWindowSurface.init(Video::gpSdlWindow, gVulkanInstance))
        FatalErrors::raise("Failed to initialize a Vulkan window surface!");

    gpPhysicalDevice = vgl::PhysicalDeviceSelection::selectBestDevice(gVulkanInstance.getPhysicalDevices(), gWindowSurface);

    if (!gpPhysicalDevice)
        FatalErrors::raise("Failed to find a suitable rendering device for use with Vulkan!");

    // Initialize the logical Vulkan device used for commands and operations and then decide draw sample count and window surface format
    if (!gDevice.init(*gpPhysicalDevice, &gWindowSurface))
        FatalErrors::raise("Failed to initialize a Vulkan logical device!");

    decidePresentSurfaceFormat();
    decideDrawSampleCount();

    // Initialize all pipeline components: must be done BEFORE creating render paths, as they rely on some components
    VPipelines::initPipelineComponents(gDevice, gDrawSampleCount);

    // Initialize all render paths
    const VkFormat drawColorFormat = (Config::gbUseVulkan32BitShading) ? COLOR_32_FORMAT : COLOR_16_FORMAT;

    gRenderPath_Psx.init(gDevice, COLOR_16_FORMAT);
    gRenderPath_Main.init(gDevice, gDrawSampleCount, drawColorFormat, COLOR_32_FORMAT);
    gRenderPath_Crossfade.init(gDevice, gSwapchain, gPresentSurfaceFormat, gRenderPath_Main);

    // Create all of the pipelines needed, these use the previously created pipeline components
    VPipelines::initPipelines(gRenderPath_Main, gRenderPath_Crossfade, gDrawSampleCount);

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

    // Initialize the draw command submission module and crossfader
    VDrawing::init(gDevice, gPsxVramTexture);
    VCrossfader::init(gDevice);

    // Set the initial render path and make it active.
    // TODO: remember renderer preference here.
    gpCurRenderPath = &gRenderPath_Main;
    gpNextRenderPath = &gRenderPath_Main;

    // Try to init the swapchain and framebuffers
    ensureValidSwapchainAndFramebuffers();
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
    VCrossfader::destroy();
    VDrawing::shutdown();

    gpNextRenderPath = nullptr;
    gpCurRenderPath = nullptr;
    gPsxVramTexture.destroy(true);

    for (vgl::CmdBuffer& cmdBuffer : gCmdBuffers) {
        cmdBuffer.destroy(true);
    }

    for (vgl::Semaphore& semaphore : gRenderDoneSemaphores) {
        semaphore.destroy();
    }

    for (vgl::Semaphore& semaphore : gSwapImageReadySemaphores) {
        semaphore.destroy();
    }

    gRenderPath_Crossfade.destroy();
    gRenderPath_Main.destroy();
    gRenderPath_Psx.destroy();

    VPipelines::shutdown();
    gSwapchain.destroy();
    gDevice.destroy();
    gpPhysicalDevice = nullptr;
    gWindowSurface.destroy();
    gVulkanInstance.destroy();
    gVkFuncs = {};

    // Clear all coord sys info
    updateCoordSysInfo();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform tasks that need to be done on frame startup.
// Returns 'true' if a frame was successfully started and a draw can proceed, 'false' if no drawing should take place.
// Alternatively 'isRendering()' can be queried at any time to tell if drawing can take place.
//------------------------------------------------------------------------------------------------------------------------------------------
bool beginFrame() noexcept {
    // Must have ended the frame or not started one previously
    ASSERT(!gbDidBeginFrame);
    gbDidBeginFrame = true;

    // Do a render path switch if requested
    gpCurRenderPath = gpNextRenderPath;

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

    // Render path specific frame start
    gpCurRenderPath->beginFrame(gSwapchain, gCmdBufferRec);
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a render path is active for this frame and whether we are actually going to do some drawing.
// This will be the case if we have a valid swapchain and render path framebuffers and have successfully started a frame.
//------------------------------------------------------------------------------------------------------------------------------------------
bool isRendering() noexcept {
    return gCmdBufferRec.isRecording();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// End the current frame and present to the screen
//------------------------------------------------------------------------------------------------------------------------------------------
void endFrame() noexcept {
    // Must have begun the frame
    ASSERT(gbDidBeginFrame);
    gbDidBeginFrame = false;

    // Render path specific frame end, if we are rendering; note: may cause transfer tasks to be kicked off...
    if (isRendering()) {
        gpCurRenderPath->endFrame(gSwapchain, gCmdBufferRec);
    }

    // Begin executing any pending transfers
    vgl::TransferMgr& transferMgr = gDevice.getTransferMgr();
    transferMgr.executePreFrameTransferTask();

    // If we do not have a framebuffer (and hence no command buffer) then simply wait until transfers have finished (device idle) and exit
    if (!gCmdBufferRec.isRecording()) {
        gDevice.waitUntilDeviceIdle();
        return;
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
// Gets the render path which is currently active for this frame
//------------------------------------------------------------------------------------------------------------------------------------------
IVRendererPath& getActiveRenderPath() noexcept {
    ASSERT(gpCurRenderPath);
    return *gpCurRenderPath;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the render path which will be active in the next frame
//------------------------------------------------------------------------------------------------------------------------------------------
IVRendererPath& getNextRenderPath() noexcept {
    ASSERT(gpNextRenderPath);
    return *gpNextRenderPath;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the render path which will be active in the next frame
//------------------------------------------------------------------------------------------------------------------------------------------
void setNextRenderPath(IVRendererPath& renderPath) noexcept {
    gpNextRenderPath = &renderPath;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: tells if the classic PSX render path is the current render path being used
//------------------------------------------------------------------------------------------------------------------------------------------
bool isUsingPsxRenderPath() noexcept {
    return (gpCurRenderPath == &gRenderPath_Psx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helpers to switch between the PSX and main Vulkan render paths.
// The switch happens at the beginning of the next frame.
//------------------------------------------------------------------------------------------------------------------------------------------
void switchToPsxRenderPath() noexcept {
    setNextRenderPath(gRenderPath_Psx);
}

void switchToMainVulkanRenderPath() noexcept {
    setNextRenderPath(gRenderPath_Main);
}

END_NAMESPACE(VRenderer)

#endif  // #if PSYDOOM_VULKAN_RENDERER
