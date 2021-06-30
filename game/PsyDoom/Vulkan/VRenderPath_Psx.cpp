#include "VRenderPath_Psx.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"
#include "CmdBufferRecorder.h"
#include "Gpu.h"
#include "LogicalDevice.h"
#include "PsyDoom/PsxVm.h"
#include "PsyDoom/Video.h"
#include "Swapchain.h"
#include "VRenderer.h"

#include <algorithm>

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the render path to a default uninitialized state
//------------------------------------------------------------------------------------------------------------------------------------------
VRenderPath_Psx::VRenderPath_Psx() noexcept 
    : mbIsValid(false)
    , mpDevice(nullptr)
    , mPsxFramebufferTextures{}
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the render path if not already destroyed
//------------------------------------------------------------------------------------------------------------------------------------------
VRenderPath_Psx::~VRenderPath_Psx() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the render path - this must always succeed
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Psx::init(vgl::LogicalDevice& device, const VkFormat psxFramebufferFormat) noexcept {
    // Sanity checks and remembering the device used
    ASSERT_LOG(!mbIsValid, "Can't initialize twice!");
    ASSERT(device.isValid());
    mpDevice = &device;

    // Only two framebuffer formats are supported
    ASSERT((psxFramebufferFormat == VK_FORMAT_A1R5G5B5_UNORM_PACK16) || (psxFramebufferFormat == VK_FORMAT_B8G8R8A8_UNORM));

    // Initialize the PSX framebuffer textures used to hold the old PSX renderer framebuffer before it is blit to the Vulkan framebuffer
    for (vgl::MutableTexture& psxFbTex : mPsxFramebufferTextures) {
        if (!psxFbTex.initAs2dTexture(device, psxFramebufferFormat, Video::ORIG_DRAW_RES_X, Video::ORIG_DRAW_RES_Y))
            FatalErrors::raise("Failed to create a Vulkan texture for the classic PSX renderer's framebuffer!");
    }

    // Now initialized
    mbIsValid = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tears down the render path
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Psx::destroy() noexcept {
    if (!mbIsValid)
        return;

    mbIsValid = false;

    for (vgl::MutableTexture& tex : mPsxFramebufferTextures) {
        tex.destroy(true);
    }

    mpDevice = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// This doesn't need to do anything for the PSX render path.
// The textures used to hold the original PSX framebuffers are created once on initialization, and their size will never change.
// Technically these are not framebuffers, as we don't draw to them normally.
//------------------------------------------------------------------------------------------------------------------------------------------
bool VRenderPath_Psx::ensureValidFramebuffers([[maybe_unused]] const uint32_t fbWidth, [[maybe_unused]] const uint32_t fbHeight) noexcept {
    ASSERT(mbIsValid);
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begins the frame for the render path
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Psx::beginFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Sanity checks
    ASSERT(mbIsValid);
    ASSERT(swapchain.isValid());
    ASSERT(swapchain.getAcquiredImageIdx() < swapchain.getLength());

    // Transition the PSX framebuffer to general in preparation for writing
    vgl::LogicalDevice& device = *mpDevice;
    const uint32_t ringbufferIdx = device.getRingbufferMgr().getBufferIndex();

    {
        vgl::MutableTexture& psxFbTexture = mPsxFramebufferTextures[ringbufferIdx];

        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imgBarrier.image = psxFbTexture.getVkImage();
        imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgBarrier.subresourceRange.levelCount = 1;
        imgBarrier.subresourceRange.layerCount = 1;

        cmdRec.addPipelineBarrier(
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0,
            nullptr,
            1,
            &imgBarrier
        );
    }

    // Transition the swapchain image to transfer destination optimal in preparation for blitting
    const uint32_t swapchainIdx = swapchain.getAcquiredImageIdx();
    ASSERT(swapchainIdx < swapchain.getLength());

    const VkImage swapchainImage = swapchain.getVkImages()[swapchainIdx];

    {
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgBarrier.image = swapchainImage;
        imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgBarrier.subresourceRange.levelCount = 1;
        imgBarrier.subresourceRange.layerCount = 1;

        cmdRec.addPipelineBarrier(
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
        VkClearColorValue clearColor = {};
        clearColor.float32[3] = 1.0f;

        VkImageSubresourceRange imageResRange = {};
        imageResRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageResRange.levelCount = 1;
        imageResRange.layerCount = 1;

        cmdRec.clearColorImage(swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &imageResRange);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ends the frame for the render path
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Psx::endFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept {
    // Sanity checks
    ASSERT(mbIsValid);
    ASSERT(swapchain.isValid());
    ASSERT(swapchain.getAcquiredImageIdx() < swapchain.getLength());

    // Copy the PlayStation 1 framebuffer to the current framebuffer texture
    vgl::LogicalDevice& device = *mpDevice;
    const uint32_t ringbufferIdx = device.getRingbufferMgr().getBufferIndex();
    vgl::MutableTexture& psxFbTexture = mPsxFramebufferTextures[ringbufferIdx];

    {
        Gpu::Core& gpu = PsxVm::gGpu;
        const uint16_t* pSrcPixels = gpu.pRam + (gpu.displayAreaX + (uintptr_t) gpu.displayAreaY * gpu.ramPixelW);

        if (psxFbTexture.getFormat() == VK_FORMAT_A1R5G5B5_UNORM_PACK16) {
            copyPsxFramebufferToFbTexture_A1R5G5B5(pSrcPixels, (uint16_t*) psxFbTexture.getBytes());
        } else {
            ASSERT(psxFbTexture.getFormat() == VK_FORMAT_B8G8R8A8_UNORM);
            copyPsxFramebufferToFbTexture_B8G8R8A8(pSrcPixels, (uint32_t*) psxFbTexture.getBytes());
        }
    }

    // Get the area of the window to blit the PSX framebuffer to
    const uint32_t screenWidth = swapchain.getSwapExtentWidth();
    const uint32_t screenHeight = swapchain.getSwapExtentHeight();

    float blitDstX = {};
    float blitDstY = {};
    float blitDstW = {};
    float blitDstH = {};
    Video::getClassicFramebufferWindowRect((float) screenWidth, (float) screenHeight, blitDstX, blitDstY, blitDstW, blitDstH);

    // Only bother doing further commands if we're going to present.
    // This avoids errors on MacOS/Metal also, where we try to blit to an incompatible destination window size.
    if (VRenderer::willSkipNextFramePresent())
        return;

    // Blit the PSX framebuffer to the swapchain image
    ASSERT((Video::gTopOverscan >= 0) && (Video::gTopOverscan < Video::ORIG_DRAW_RES_Y / 2));
    ASSERT((Video::gBotOverscan >= 0) && (Video::gBotOverscan < Video::ORIG_DRAW_RES_Y / 2));

    const uint32_t swapchainIdx = swapchain.getAcquiredImageIdx();
    const VkImage swapchainImage = swapchain.getVkImages()[swapchainIdx];

    if ((blitDstW > 0) && (blitDstH > 0)) {
        VkImageBlit blitRegion = {};
        blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.srcSubresource.layerCount = 1;
        blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blitRegion.dstSubresource.layerCount = 1;
        blitRegion.srcOffsets[0].y = Video::gTopOverscan;
        blitRegion.srcOffsets[1].x = Video::ORIG_DRAW_RES_X;
        blitRegion.srcOffsets[1].y = Video::ORIG_DRAW_RES_Y - Video::gBotOverscan;
        blitRegion.srcOffsets[1].z = 1;
        blitRegion.dstOffsets[0].x = std::clamp<int32_t>((int32_t) blitDstX, 0, screenWidth);
        blitRegion.dstOffsets[0].y = std::clamp<int32_t>((int32_t) blitDstY, 0, screenHeight);
        blitRegion.dstOffsets[1].x = std::clamp<int32_t>((int32_t)(blitDstX + blitDstW), 0, screenWidth);
        blitRegion.dstOffsets[1].y = std::clamp<int32_t>((int32_t)(blitDstY + blitDstH), 0, screenHeight);
        blitRegion.dstOffsets[1].z = 1;

        cmdRec.blitImage(
            psxFbTexture.getVkImage(),
            VK_IMAGE_LAYOUT_GENERAL,
            swapchainImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &blitRegion,
            VK_FILTER_NEAREST
        );
    }

    // Transition the swapchain image back to presentation optimal in preparation for presentation
    {
        VkImageMemoryBarrier imgBarrier = {};
        imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imgBarrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
        imgBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imgBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imgBarrier.image = swapchainImage;
        imgBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imgBarrier.subresourceRange.levelCount = 1;
        imgBarrier.subresourceRange.layerCount = 1;

        cmdRec.addPipelineBarrier(
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
// Copies the PSX framebuffer to a Vulkan texture containing the same framebuffer.
// This overload is for an A1R5G5B5 format destination framebuffer.
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Psx::copyPsxFramebufferToFbTexture_A1R5G5B5(const uint16_t* const pSrcPixels, uint16_t* pDstPixels) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;
    const uint16_t ramPixelW = gpu.ramPixelW;

    const uint16_t* pSrcRowPixels = pSrcPixels;
    uint16_t* pDstRowPixels = pDstPixels;

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

        pSrcRowPixels += ramPixelW;
        pDstRowPixels += Video::ORIG_DRAW_RES_X;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copies the PSX framebuffer to a Vulkan texture containing the same framebuffer.
// This overload is for an B8G8R8A8 format destination framebuffer.
//------------------------------------------------------------------------------------------------------------------------------------------
void VRenderPath_Psx::copyPsxFramebufferToFbTexture_B8G8R8A8(const uint16_t* const pSrcPixels, uint32_t* pDstPixels) noexcept {
    Gpu::Core& gpu = PsxVm::gGpu;
    const uint16_t ramPixelW = gpu.ramPixelW;

    const uint16_t* pSrcRowPixels = pSrcPixels;
    uint32_t* pDstRowPixels = pDstPixels;

    for (uint32_t y = 0; y < Video::ORIG_DRAW_RES_Y; ++y) {
        // Note: don't bother doing multiple pixels at a time - compiler is smart and already optimizes this to use SIMD
        for (uint32_t x = 0; x < Video::ORIG_DRAW_RES_X; ++x) {
            const uint32_t srcPixel = pSrcRowPixels[x];
            const uint32_t srcR = ((srcPixel >>  0) & 0x1F) << 3;
            const uint32_t srcG = ((srcPixel >>  5) & 0x1F) << 3;
            const uint32_t srcB = ((srcPixel >> 10) & 0x1F) << 3;

            const uint32_t dstPixel = (srcB << 0) | (srcG << 8) | (srcR << 16) | 0xFF000000u;
            pDstRowPixels[x] = dstPixel;
        }

        pSrcRowPixels += ramPixelW;
        pDstRowPixels += Video::ORIG_DRAW_RES_X;
    }
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
