#include "VideoSurface_Vulkan.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"

#include <cstring>

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to create the surface with the specified dimensions.
// If creation fails then the failure is silent and usage of the surface results in NO-OPs.
//------------------------------------------------------------------------------------------------------------------------------------------
VideoSurface_Vulkan::VideoSurface_Vulkan(vgl::LogicalDevice& device, const uint32_t width, const uint32_t height) noexcept
    : mTexture()
    , mWidth(width)
    , mHeight(height)
    , mbInVkGeneralImgLayout(false)
{
    ASSERT((width > 0) && (height > 0));

    if (!mTexture.initAs2dTexture(device, VK_FORMAT_A8B8G8R8_UNORM_PACK32, width, height)) {
        ASSERT_FAIL("VideoSurface_Vulkan::VideoSurface_Vulkan: initializing the texture failed!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleans up surface resources
//------------------------------------------------------------------------------------------------------------------------------------------
VideoSurface_Vulkan::~VideoSurface_Vulkan() noexcept = default;

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the dimensions of the surface
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t VideoSurface_Vulkan::getWidth() const noexcept { return mWidth; }
uint32_t VideoSurface_Vulkan::getHeight() const noexcept { return mHeight; }

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the pixels for the surface.
// Ignores the call if the surface was not successfully initialized.
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoSurface_Vulkan::setPixels(const uint32_t* const pSrcPixels) noexcept {
    ASSERT(pSrcPixels);

    // Only if the texture was initialized validly
    if (mTexture.isValid()) {
        std::memcpy(mTexture.getBytes(), pSrcPixels, mWidth * mHeight * sizeof(uint32_t));
    }
}

END_NAMESPACE(Video)

#endif  // #if PSYDOOM_VULKAN_RENDERER
