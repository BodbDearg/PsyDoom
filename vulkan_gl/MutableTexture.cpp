#include "MutableTexture.h"

#include "LogicalDevice.h"
#include "RawBuffer.h"
#include "RetirementMgr.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized mutable texture
//------------------------------------------------------------------------------------------------------------------------------------------
MutableTexture::MutableTexture() noexcept
    : BaseTexture()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a mutable texture to this object
//------------------------------------------------------------------------------------------------------------------------------------------
MutableTexture::MutableTexture(MutableTexture&& other) noexcept
    : BaseTexture(std::move(other))
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically destroys the mutable texture
//------------------------------------------------------------------------------------------------------------------------------------------
MutableTexture::~MutableTexture() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a 1d texture.
// Note: does not populate/initialize texture data, just allocates memory, creates the Vulkan image object and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
bool MutableTexture::initAs1dTexture(
    LogicalDevice& device,
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t numLayers,
    const uint32_t numMipLevels,
    const AlphaMode alphaMode
) noexcept {
    return initInternal(
        device,
        VK_IMAGE_TYPE_1D,
        VK_IMAGE_VIEW_TYPE_MAX_ENUM,                                        // Don't create an image view! Mutable textures can't be sampled.
        VK_IMAGE_TILING_LINEAR,                                             // Straight image storage - no fancy stuff
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,  // Mutable textures are only for transferring to/from and storage
        VK_IMAGE_LAYOUT_PREINITIALIZED,                                     // Any mem contents from here on in must be preserved
        DeviceMemAllocMode::REQUIRE_HOST_VISIBLE,                           // Must be in RAM
        textureFormat,
        width,
        1,              // Height
        1,              // Depth
        numLayers,
        numMipLevels,
        1,              // Num samples
        false,          // Is cubemap?
        alphaMode
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a 2d texture.
// Note: does not populate/initialize texture data, just allocates memory, creates the Vulkan image object and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
bool MutableTexture::initAs2dTexture(
    LogicalDevice& device,
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t height,
    const uint32_t numLayers,
    const uint32_t numMipLevels,
    const AlphaMode alphaMode
) noexcept {
    return initInternal(
        device,
        VK_IMAGE_TYPE_2D,
        VK_IMAGE_VIEW_TYPE_MAX_ENUM,                                        // Don't create an image view! Mutable textures can't be sampled.
        VK_IMAGE_TILING_LINEAR,                                             // Straight image storage - no fancy stuff
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,  // Mutable textures are only for transferring to/from and storage
        VK_IMAGE_LAYOUT_PREINITIALIZED,                                     // Any mem contents from here on in must be preserved
        DeviceMemAllocMode::REQUIRE_HOST_VISIBLE,                           // Must be in RAM
        textureFormat,
        width,
        height,
        1,              // Depth
        numLayers,
        numMipLevels,
        1,              // Num samples
        false,          // Is cubemap?
        alphaMode
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a 3d texture.
// Note: does not populate/initialize texture data, just allocates memory, creates the Vulkan image object and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
bool MutableTexture::initAs3dTexture(
    LogicalDevice& device,
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t height,
    const uint32_t depth,
    const uint32_t numMipLevels,
    const AlphaMode alphaMode
) noexcept {
    return initInternal(
        device,
        VK_IMAGE_TYPE_3D,
        VK_IMAGE_VIEW_TYPE_MAX_ENUM,                                        // Don't create an image view! Mutable textures can't be sampled.
        VK_IMAGE_TILING_LINEAR,                                             // Straight image storage - no fancy stuff
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,  // Mutable textures are only for transferring to/from and storage
        VK_IMAGE_LAYOUT_PREINITIALIZED,                                     // Any mem contents from here on in must be preserved
        DeviceMemAllocMode::REQUIRE_HOST_VISIBLE,                           // Must be in RAM
        textureFormat,
        width,
        height,
        depth,
        1,              // Num layers
        numMipLevels,
        1,              // Num samples
        false,          // Is cubemap?
        alphaMode
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Create a cube texture.
// Note: does not populate/initialize texture data, just allocates memory, creates the Vulkan image object and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
bool MutableTexture::initAsCubeTexture(
    LogicalDevice& device,
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t height,
    const uint32_t numLayers,
    const uint32_t numMipLevels,
    const AlphaMode alphaMode
) noexcept {
    return initInternal(
        device,
        VK_IMAGE_TYPE_2D,
        VK_IMAGE_VIEW_TYPE_MAX_ENUM,                                        // Don't create an image view! Mutable textures can't be sampled.
        VK_IMAGE_TILING_LINEAR,                                             // Straight image storage - no fancy stuff
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,  // Mutable textures are only for transferring to/from and storage
        VK_IMAGE_LAYOUT_PREINITIALIZED,                                     // Any mem contents from here on in must be preserved
        DeviceMemAllocMode::REQUIRE_HOST_VISIBLE,                           // Must be in RAM
        textureFormat,
        width,
        height,
        1,              // Depth
        numLayers,
        numMipLevels,
        1,              // Num samples
        true,           // Is cubemap?
        alphaMode
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the mutable texture and releases its resources
//------------------------------------------------------------------------------------------------------------------------------------------
void MutableTexture::destroy(const bool bImmediately, const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;

    // Preconditions
    ASSERT_LOG((!mpDevice) || mpDevice->getVkDevice(), "Parent device must still be valid if defined!");

    // Gradual 'retirement' logic if specified and possible
    if ((!bImmediately) && mbIsValid) {
        mpDevice->getRetirementMgr().retire(*this);
        return;
    }

    // Cleanup logic: this class has no real logic of it's own so defers to base
    BaseTexture::destroy();
}

END_NAMESPACE(vgl)
