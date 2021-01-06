#include "ScreenFramebufferMgr.h"

#include "DeviceSurfaceCaps.h"
#include "Finally.h"
#include "Framebuffer.h"
#include "LogicalDevice.h"
#include "PhysicalDevice.h"
#include "RenderTexture.h"
#include "RetirementMgr.h"
#include "Swapchain.h"

BEGIN_NAMESPACE(vgl)

// What color surface formats are supported, with the most preferential being first
static constexpr VkFormat SUPPORTED_COLOR_SURFACE_FORMATS[] = {
    VK_FORMAT_B8G8R8A8_UNORM,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_FORMAT_A8B8G8R8_UNORM_PACK32,
    VK_FORMAT_A2R10G10B10_UNORM_PACK32,
    VK_FORMAT_A2B10G10R10_UNORM_PACK32
};

// What depth stencil formats are supported, with the most preferential being first
static constexpr VkFormat SUPPORTED_DEPTH_STENCIL_FORMATS[] = {
    VK_FORMAT_D32_SFLOAT,
    VK_FORMAT_D32_SFLOAT_S8_UINT,
    VK_FORMAT_D24_UNORM_S8_UINT,
    VK_FORMAT_X8_D24_UNORM_PACK32,
    VK_FORMAT_D16_UNORM,
    VK_FORMAT_D16_UNORM_S8_UINT,
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized framebuffer manager
//------------------------------------------------------------------------------------------------------------------------------------------
ScreenFramebufferMgr::ScreenFramebufferMgr() noexcept
    : mbIsValid(false)
    , mColorFormat(VK_FORMAT_UNDEFINED)
    , mDepthStencilFormat(VK_FORMAT_UNDEFINED)
    , mCurrentFramebufferIdx(INVALID_FRAMEBUFFER_IDX)
    , mpDevice(nullptr)
    , mDeviceSurfaceCaps()
    , mSwapchain()
    , mDepthStencilBuffer()
    , mDrawFramebuffers()
    , mPresentFramebuffers()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the framebuffer manager
//------------------------------------------------------------------------------------------------------------------------------------------
ScreenFramebufferMgr::~ScreenFramebufferMgr() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up and initializes the framebuffer manager and the swapchain for the main framebuffers
//------------------------------------------------------------------------------------------------------------------------------------------
bool ScreenFramebufferMgr::init(LogicalDevice& device) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.getPhysicalDevice());
    ASSERT(device.getWindowSurface());

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true);
        }
    });

    // On creation the screen framebuffer simply queries the device surface caps and decides on a framebuffer format
    // for the application. The actual creation of the framebuffer is handled on-demand by the renderer managing the
    // graphics device and framebuffers:
    mpDevice = &device;

    if (!mDeviceSurfaceCaps.query(*device.getPhysicalDevice(), *device.getWindowSurface())) {
        ASSERT_FAIL("Failed to query device surface caps while creating screen framebuffer!");
        return false;
    }

    if (!chooseSurfaceAndDepthStencilFormats()) {
        ASSERT_FAIL("Failed to find a suitable surface and/or depth stencil buffer format for the main screen framebuffer!");
        return false;
    }
    
    // All went well if we got to here!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the framebuffer manager and releases it's resources
//------------------------------------------------------------------------------------------------------------------------------------------
void ScreenFramebufferMgr::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we have to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    mbIsValid = false;

    // Note that this will also wait until the device is idle before proceeding!
    destroyFramebuffers();

    // Cleanup everything else
    mDeviceSurfaceCaps.clear();
    mpDevice = nullptr;
    mCurrentFramebufferIdx = INVALID_FRAMEBUFFER_IDX;
    mDepthStencilFormat = VK_FORMAT_UNDEFINED;
    mColorFormat = VK_FORMAT_UNDEFINED;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Can be called when something happens to invalidate the framebuffers used for presentation to the screen.
// Examples of such events would be the screen or window resolution changing, but also MSAA setting changes.
//------------------------------------------------------------------------------------------------------------------------------------------
void ScreenFramebufferMgr::invalidateFramebuffers() noexcept {
    ASSERT(mbIsValid);
    ASSERT_LOG(mCurrentFramebufferIdx == INVALID_FRAMEBUFFER_IDX, "This is invalid while a frame is being rendered!");

    if (mSwapchain.isValid()) {
        mSwapchain.setNeedsRecreate();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the screen frame buffers have been invalidated or are not created
//------------------------------------------------------------------------------------------------------------------------------------------
bool ScreenFramebufferMgr::framebuffersNeedRecreate() noexcept {
    ASSERT(mbIsValid);
    return ((!mSwapchain.isValid()) || mSwapchain.needsRecreate());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create the screen frame buffers: should be called ONLY at the start or end of a frame, and not during rendering operations.
//
// Note that creation of the screen framebuffers can legitimately fail if the window is resized to zero, when minimized for instance.
// In these cases the app should wait until the surface being drawn to is non zero sized before attempting to render again.
//------------------------------------------------------------------------------------------------------------------------------------------
bool ScreenFramebufferMgr::createFramebuffers(BaseRenderPass& mainDrawRenderPass, BaseRenderPass& presentRenderPass) noexcept {
    // Sanity checks
    ASSERT(mbIsValid);
    ASSERT_LOG(framebuffersNeedRecreate(), "Should only be called when the framebuffers need recreation!");

    // Create the swap chain
    LogicalDevice& device = *mpDevice;
    const VkSurfaceFormatKHR surfaceFormat = { mColorFormat, VK_COLORSPACE_SRGB_NONLINEAR_KHR };    // Note: only support SRGB colorspace for now!

    if (!mSwapchain.init(device, surfaceFormat))
        return false;

    // Create the depth stencil buffer
    const uint32_t resolutionW = mSwapchain.getSwapExtentWidth();
    const uint32_t resolutionH = mSwapchain.getSwapExtentHeight();

    if (!mDepthStencilBuffer.initAsDepthStencilBuffer(device, mDepthStencilFormat, resolutionW, resolutionH))
        return false;

    // Create the drawing and presentation framebuffers for each image view.
    // Note: present framebuffer is just the draw framebuffer without a depth attachment!
    const uint32_t swapchainSize = mSwapchain.getLength();
    mDrawFramebuffers.resize(swapchainSize);
    mPresentFramebuffers.resize(swapchainSize);

    for (uint32_t swapchainIndex = 0; swapchainIndex < swapchainSize; ++swapchainIndex) {
        if (!mDrawFramebuffers[swapchainIndex].init(mainDrawRenderPass, mSwapchain, swapchainIndex, { &mDepthStencilBuffer }))
            return false;

        if (!mPresentFramebuffers[swapchainIndex].init(presentRenderPass, mSwapchain, swapchainIndex, {}))
            return false;
    }
    
    // If we got to here then all is well!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys then creates the framebuffers.
// See 'createFramebuffers' for more notes on framebuffer creation.
//------------------------------------------------------------------------------------------------------------------------------------------
bool ScreenFramebufferMgr::recreateFramebuffers(BaseRenderPass& mainDrawRenderPass, BaseRenderPass& presentRenderPass) noexcept {
    // Preconditions
    ASSERT(mbIsValid);

    // Cleanup the old screen framebuffers first (if they exist)
    destroyFramebuffers();

    // Query the surface caps as they may have changed due to window resizing, monitor changes etc.
    //
    // Note that while this might *IN THEORY* affect the surface formats that are supported (if the caps have changed) it is HIGHLY HIGHLY
    // unlikely to happen in practice. Deliberately choosing not to handle this case here in order to simplify and avoid the complexity of
    // recreating render passes and so forth...
    //
    // The only thing I can think of that might cause a change in supported formats is switching to 16-bit color in older Windows (pre 8/10).
    // Worst comes to the worst the user can restart the app and if it becomes an actual problem later can revisit...
    // Maybe it would be an issue if HDR support was added later (switching from HDR to non HDR monitor for example).
    //
    const PhysicalDevice& physicalDevice = *mpDevice->getPhysicalDevice();
    const WindowSurface& windowSurface = *mpDevice->getWindowSurface();

    if (!mDeviceSurfaceCaps.query(physicalDevice, windowSurface))
        return false;

    // If that succeeds then create the swap chain, but only if we have a non zero sized window surface to present to.
    // If the window surface is zero sized then we'll have to try again later:
    if (hasZeroSizedWindowSurface())
        return false;

    return createFramebuffers(mainDrawRenderPass, presentRenderPass);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the framebuffer
//------------------------------------------------------------------------------------------------------------------------------------------
void ScreenFramebufferMgr::destroyFramebuffers() noexcept {
    // Wait for all render operations etc. to finish
    if (mpDevice) {
        mpDevice->waitUntilDeviceIdle();
    }

    // Destroy everything and do it immediately
    for (Framebuffer& framebuffer : mPresentFramebuffers) {
        framebuffer.destroy(true);
    }

    for (Framebuffer& framebuffer : mDrawFramebuffers) {
        framebuffer.destroy(true);
    }

    mPresentFramebuffers.clear();
    mDrawFramebuffers.clear();
    mDepthStencilBuffer.destroy(true);
    mSwapchain.destroy();

    // Note that since we destroyed all framebuffers and finished all rendering ops we can
    // immediately free any resources which have been placed in the retirement queues:
    if (mpDevice) {
        mpDevice->getRetirementMgr().freeRetiredResourcesForAllRingbufferSlots();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the window surface being presented to by the device is currently zero sized
//------------------------------------------------------------------------------------------------------------------------------------------
bool ScreenFramebufferMgr::hasZeroSizedWindowSurface() noexcept {
    ASSERT(mbIsValid);
    return mDeviceSurfaceCaps.isZeroSizedMaxImageExtent();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check if framebuffers have been acquired for rendering and presentation
//------------------------------------------------------------------------------------------------------------------------------------------
bool ScreenFramebufferMgr::hasValidFramebuffers() const noexcept {
    ASSERT(mbIsValid);
    return (mCurrentFramebufferIdx != INVALID_FRAMEBUFFER_IDX);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the currently acquired draw framebuffer.
// Note that framebuffers *MUST* be acquired before calling this.
//------------------------------------------------------------------------------------------------------------------------------------------
const Framebuffer& ScreenFramebufferMgr::getCurrentDrawFramebuffer() const noexcept {
    ASSERT(mbIsValid);
    ASSERT(hasValidFramebuffers());
    return mDrawFramebuffers[mCurrentFramebufferIdx];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the currently acquired present framebuffer.
// Note that framebuffers *MUST* be acquired before calling this.
//------------------------------------------------------------------------------------------------------------------------------------------
const Framebuffer& ScreenFramebufferMgr::getCurrentPresentFramebuffer() const noexcept {
    ASSERT(mbIsValid);
    ASSERT(hasValidFramebuffers());
    return mPresentFramebuffers[mCurrentFramebufferIdx];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the width of the all the framebuffers; returns 0 if there is no current framebuffer
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t ScreenFramebufferMgr::getFramebuffersWidth() const noexcept {
    // Note: draw framebuffers array will be the same size as present, so use that size:
    return (!mDrawFramebuffers.empty()) ? mDrawFramebuffers[0].getWidth() : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the height of the all the framebuffers; returns 0 if there is no current framebuffer
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t ScreenFramebufferMgr::getFramebuffersHeight() const noexcept {
    // Note: draw framebuffers array will be the same size as present, so use that size:
    return (!mDrawFramebuffers.empty()) ? mDrawFramebuffers[0].getHeight() : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Acquires a framebuffer for the next render. Signals the given semaphore when the framebuffer is ready, which can be used to
// synchronize with the execution of the first submitted command buffer.
//
// Notes:
// (1) This function must ONLY be called if there are valid framebuffers!
// (2) Any previously acquired images must have been presented already also.
// (3) Operation can possibly fail, check the return result!
//------------------------------------------------------------------------------------------------------------------------------------------
bool ScreenFramebufferMgr::acquireFramebuffer(Semaphore& framebufferReadySemaphore) noexcept {
    // Preconditions
    ASSERT(mbIsValid);
    ASSERT_LOG(!framebuffersNeedRecreate(), "Framebuffers must not need recreation before calling this!");
    ASSERT_LOG(mCurrentFramebufferIdx == INVALID_FRAMEBUFFER_IDX, "Must not already have acquired a framebuffer for this frame!");

    // Attempt to acquire the image and return 'true' if successful
    const uint32_t swapImageIdx = mSwapchain.acquireImage(framebufferReadySemaphore);

    if (swapImageIdx >= mSwapchain.getLength())
        return false;

    mCurrentFramebufferIdx = swapImageIdx;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Presents the current framebuffer that has been acquired for rendering. Only performs the presentation once the given semaphore has
// been signalled and the given semaphore is expected to be signalled only once all rendering operations have finished.
// Note that the operation can fail and 'false' will be returned if that is the case.
//------------------------------------------------------------------------------------------------------------------------------------------
bool ScreenFramebufferMgr::presentCurrentFramebuffer(Semaphore& renderFinishedSemaphore) noexcept {
    ASSERT(mbIsValid);
    ASSERT_LOG(mCurrentFramebufferIdx != INVALID_FRAMEBUFFER_IDX, "Must have acquired a framebuffer for rendering first!");

    const bool success = mSwapchain.presentImage(mCurrentFramebufferIdx, renderFinishedSemaphore);
    mCurrentFramebufferIdx = INVALID_FRAMEBUFFER_IDX;
    return success;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Choose what formats to use for the main color surface and the depth stencil buffer
//------------------------------------------------------------------------------------------------------------------------------------------
bool ScreenFramebufferMgr::chooseSurfaceAndDepthStencilFormats() noexcept {
    // Sanity checks
    ASSERT(mDeviceSurfaceCaps.isValid());

    // Choose an SRGB surface format.
    // Note we are hardcoding to SRGB at the moment, will need to revisit later if HDR10 is required:   
    mColorFormat = mDeviceSurfaceCaps.getSupportedSurfaceFormat(
        SUPPORTED_COLOR_SURFACE_FORMATS,
        C_ARRAY_SIZE(SUPPORTED_COLOR_SURFACE_FORMATS)
    );

    if (mColorFormat == VK_FORMAT_UNDEFINED) {
        ASSERT_FAIL("Failed to find an appropriate surface format for the device swap chain!");
        return false;
    }

    // Choose depth stencil format
    const PhysicalDevice& physicalDevice = *mpDevice->getPhysicalDevice();
    mDepthStencilFormat = physicalDevice.findFirstSupportedDepthStencilBufferFormat(
        SUPPORTED_DEPTH_STENCIL_FORMATS,
        C_ARRAY_SIZE(SUPPORTED_DEPTH_STENCIL_FORMATS)
    );

    if (mDepthStencilFormat == VK_FORMAT_UNDEFINED) {
        ASSERT_FAIL("Unable to find a suitable/supported depth stencil format to use!");
        return false;
    }

    // Success if we have reached here
    return true;
}

END_NAMESPACE(vgl)
