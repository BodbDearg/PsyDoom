#if PSYDOOM_VULKAN_RENDERER

#include "VideoBackend_Vulkan.h"

#include "Asserts.h"
#include "CmdBufferRecorder.h"
#include "LogicalDevice.h"
#include "PhysicalDeviceSelection.h"
#include "Swapchain.h"
#include "VideoSurface_Vulkan.h"
#include "VkFuncs.h"
#include "Vulkan/VRenderer.h"
#include "Vulkan/VRenderPath_Blit.h"
#include "Vulkan/VRenderPath_Main.h"
#include "VulkanInstance.h"
#include "WindowSurface.h"

#include <algorithm>
#include <SDL.h>
#include <SDL_vulkan.h>

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks whether the Vulkan video backend is supported
//------------------------------------------------------------------------------------------------------------------------------------------
bool VideoBackend_Vulkan::isBackendSupported() noexcept {
    // Must be able to load the Vulkan library or have it embedded in the application
    if (SDL_Vulkan_LoadLibrary(nullptr) != 0)
        return false;

    // Must be able to retrieve Vulkan functions
    if (!SDL_Vulkan_GetVkGetInstanceProcAddr())
        return false;

    // Create a temporary (headless) API instance and make sure we can pick a suitable device from it.
    // It's possible that a Vulkan driver might be installed on the system but no valid devices are present.
    //
    // Note: have to use the headless checks because at this point we don't have an SDL window created.
    // This function is used to DECIDE what type of SDL Window to create, so we can't check surface output capabilities yet...
    vgl::VkFuncs vkFuncs = {};
    vgl::VulkanInstance vulkanInstance(vkFuncs);

    if (!vulkanInstance.init(nullptr))
        return false;

    const std::vector<vgl::PhysicalDevice>& physicalDevices = vulkanInstance.getPhysicalDevices();
    return vgl::PhysicalDeviceSelection::selectBestHeadlessDevice(physicalDevices, VRenderer::isHeadlessPhysicalDeviceSuitable);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the backend with stuff for the Vulkan and classic renderers uninitialized
//------------------------------------------------------------------------------------------------------------------------------------------
VideoBackend_Vulkan::VideoBackend_Vulkan() noexcept 
    : mpSdlWindow(nullptr)
    , mpDispExtSurfOldRenderPath(nullptr)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ensures everything is cleaned up
//------------------------------------------------------------------------------------------------------------------------------------------
VideoBackend_Vulkan::~VideoBackend_Vulkan() noexcept {
    destroyRenderers();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the window create flags required for an SDL video backend window
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t VideoBackend_Vulkan::getSdlWindowCreateFlags() noexcept {
    // Want a Vulkan window only
    return SDL_WINDOW_VULKAN;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the SDL renderer used by this backend
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_Vulkan::initRenderers(SDL_Window* const pSdlWindow) noexcept {
    // Save the SDL window for later use
    ASSERT(pSdlWindow);
    mpSdlWindow = pSdlWindow;

    // Initialize the game's core Vulkan renderer module and begin a frame
    VRenderer::init();
    VRenderer::beginFrame();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleans up and destroys the SDL renderer used by this video backend
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_Vulkan::destroyRenderers() noexcept {
    VRenderer::destroy();
    mpSdlWindow = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays the output from the Vulkan based renderer (classic, or new) to the screen
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_Vulkan::displayFramebuffer() noexcept {
    VRenderer::endFrame();
    VRenderer::beginFrame();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs setup prior to displaying external surfaces
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_Vulkan::beginExternalSurfaceDisplay() noexcept {
    // Finish up the current frame that was automatically started and remember the render path used.
    // Frames will be submitted manually via 'displayExternalSurface()' from here on in:
    mpDispExtSurfOldRenderPath = &VRenderer::getActiveRenderPath();
    VRenderer::endFrame();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs cleanup once we are done displaying external surfaces
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_Vulkan::endExternalSurfaceDisplay() noexcept {
    // Go back to automatically starting frames and restore the previous render path.
    // No longer submitting frames manually via 'displayExternalSurface()'.
    if (mpDispExtSurfOldRenderPath) {
        VRenderer::setNextRenderPath(*mpDispExtSurfOldRenderPath);
        mpDispExtSurfOldRenderPath = nullptr;
    }

    VRenderer::beginFrame();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays the specified surface to the screen
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_Vulkan::displayExternalSurface(
    IVideoSurface& surface,
    const int32_t displayX,
    const int32_t displayY,
    const uint32_t displayW,
    const uint32_t displayH,
    const bool bUseFiltering
) noexcept {
    // Must be a Vulkan video surface!
    ASSERT(dynamic_cast<VideoSurface_Vulkan*>(&surface));
    VideoSurface_Vulkan& vulkanSurface = static_cast<VideoSurface_Vulkan&>(surface);

    // Note: it's assumed that no frame is currently active, hence no 'endFrame()' call.
    // This will always be the case if 'beginExternalSurfaceDisplay()' is called prior to calling this function for a series of frames.
    VRenderer::setNextRenderPath(VRenderer::gRenderPath_Blit);
    const bool bCanDraw = VRenderer::beginFrame();

    // Get the size of the area we can blit to and clip the desintation rectangle to that area
    const vgl::Texture& srcTexture = vulkanSurface.getTexture();
    const int32_t screenW = VRenderer::gSwapchain.getSwapExtentWidth();
    const int32_t screenH = VRenderer::gSwapchain.getSwapExtentHeight();

    int32_t blitSrcStartX = 0;
    int32_t blitSrcEndX = srcTexture.getWidth();
    int32_t blitSrcStartY = 0;
    int32_t blitSrcEndY = srcTexture.getHeight();

    int32_t blitDstStartX = displayX;
    int32_t blitDstEndX = blitDstStartX + (int32_t) displayW;
    int32_t blitDstStartY = displayY;
    int32_t blitDstEndY = blitDstStartY + (int32_t) displayH;

    int32_t blitDstW = blitDstEndX - blitDstStartX;
    int32_t blitDstH = blitDstEndY - blitDstStartY;

    if ((blitDstStartX < 0) && (blitDstW > 0)) {
        // Offscreen to the left - clip:
        const float offscreenPercent = (float) -blitDstStartX / (float) blitDstW;
        blitSrcStartX += (int32_t)((float)(blitSrcEndX - blitSrcStartX) * offscreenPercent);
        blitDstStartX = 0;
        blitDstW = blitDstEndX - blitDstStartX;
    }

    if ((blitDstEndX > screenW) && (blitDstW > 0)) {
        // Offscreen to the right - clip:
        const float offscreenPercent = (float)(blitDstEndX - screenW) / (float) blitDstW;
        blitSrcEndX -= (int32_t)((float)(blitSrcEndX - blitSrcStartX) * offscreenPercent);
        blitDstEndX = screenW;
        blitDstW = blitDstEndX - blitDstStartX;
    }

    if ((blitDstStartY < 0) && (blitDstH > 0)) {
        // Offscreen to the top - clip:
        const float offscreenPercent = (float) -blitDstStartY / (float) blitDstH;
        blitSrcStartY += (int32_t)((float)(blitSrcEndY - blitSrcStartY) * offscreenPercent);
        blitDstStartY = 0;
        blitDstH = blitDstEndY - blitDstStartY;
    }

    if ((blitDstEndY > screenH) && (blitDstH > 0)) {
        // Offscreen to the bottom - clip:
        const float offscreenPercent = (float)(blitDstEndY - screenH) / (float) blitDstH;
        blitSrcEndY -= (int32_t)((float)(blitSrcEndY - blitSrcStartY) * offscreenPercent);
        blitDstEndY = screenH;
        blitDstH = blitDstEndY - blitDstStartY;
    }

    const int32_t blitSrcW = blitSrcEndX - blitSrcStartX;
    const int32_t blitSrcH = blitSrcEndY - blitSrcStartY;

    // Only bother doing commands if we're going to present, the blit area is valid, drawing is allowed and if the surface is valid.
    // This avoids errors on MacOS/Metal also, where we try to blit to an incompatible destination window size.
    const bool bCanBlit = (
        bCanDraw &&
        (blitSrcW > 0) &&
        (blitSrcH > 0) &&
        (blitDstW > 0) &&
        (blitDstH > 0) &&
        (!VRenderer::willSkipNextFramePresent()) &&
        srcTexture.isValid()
    );

    if (bCanBlit) {
        // Get the swap chain image to blit to
        const uint32_t curSwapchainImgIdx = VRenderer::gSwapchain.getAcquiredImageIdx();
        const VkImage vkSwapchainImg = VRenderer::gSwapchain.getVkImages()[curSwapchainImgIdx];

        // Transition the blit texture to the 'transfer src' layout (from 'shader read only') in preparation for blitting (if not already done so)
        vgl::CmdBufferRecorder& cmdRec = VRenderer::gCmdBufferRec;

        if (!vulkanSurface.isReadyForBlit()) {
            VkImageMemoryBarrier imgBarrier = {};
            imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;            // Wait for any texture uploads to finish
            imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            imgBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            imgBarrier.image = srcTexture.getVkImage();
            imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imgBarrier.subresourceRange.levelCount = 1;
            imgBarrier.subresourceRange.layerCount = 1;

            cmdRec.addPipelineBarrier(
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                nullptr,
                1,
                &imgBarrier
            );

            // Don't do this transition again!
            vulkanSurface.setReadyForBlit();
        }

        // Schedule the blit
        {
            VkImageBlit blitRegion = {};
            blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blitRegion.srcSubresource.layerCount = 1;
            blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blitRegion.dstSubresource.layerCount = 1;
            blitRegion.srcOffsets[0].x = blitSrcStartX;
            blitRegion.srcOffsets[0].y = blitSrcStartY;
            blitRegion.srcOffsets[1].x = blitSrcEndX;
            blitRegion.srcOffsets[1].y = blitSrcEndY;
            blitRegion.srcOffsets[1].z = 1;
            blitRegion.dstOffsets[0].x = blitDstStartX;
            blitRegion.dstOffsets[0].y = blitDstStartY;
            blitRegion.dstOffsets[1].x = blitDstEndX;
            blitRegion.dstOffsets[1].y = blitDstEndY;
            blitRegion.dstOffsets[1].z = 1;

            cmdRec.blitImage(
                srcTexture.getVkImage(),
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                vkSwapchainImg,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &blitRegion,
                (bUseFiltering) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST
            );
        }
    }

    // Finish up and wait for all rendering to complete so the surface can be used freely again without worrying about synchronization.
    // This is of course suboptimal, but this routine is used for things that don't need much performance.
    VRenderer::endFrame();
    VRenderer::gDevice.waitUntilDeviceIdle();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gives the size of the swapchain/window in pixels
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_Vulkan::getScreenSizeInPixels(uint32_t& width, uint32_t& height) noexcept {
    width = VRenderer::gSwapchain.getSwapExtentWidth();
    height = VRenderer::gSwapchain.getSwapExtentHeight();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates and returns a Vulkan format video surface.
// Fails if the device has not been initialized successfully.
//------------------------------------------------------------------------------------------------------------------------------------------
std::unique_ptr<IVideoSurface> VideoBackend_Vulkan::createSurface(const uint32_t width, const uint32_t height) noexcept {
    return (VRenderer::gDevice.isValid()) ?
        std::make_unique<VideoSurface_Vulkan>(VRenderer::gDevice, width, height) :
        std::unique_ptr<IVideoSurface>();
}

END_NAMESPACE(Video)

#endif  // #if PSYDOOM_VULKAN_RENDERER
