#include "BaseTexture.h"

#include "AlphaMode.h"
#include "DeviceMemMgr.h"
#include "Finally.h"
#include "LogicalDevice.h"
#include "TextureUtils.h"
#include "VkFormatUtils.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates an uninitialized base texture
//------------------------------------------------------------------------------------------------------------------------------------------
BaseTexture::BaseTexture() noexcept
    : mbIsValid(false)
    , mFormat(VK_FORMAT_UNDEFINED)
    , mWidth(0)
    , mHeight(0)
    , mDepth(0)
    , mNumLayers(0)
    , mNumMipLevels(0)
    , mNumSamples(0)
    , mbIsCubemap(false)
    , mAlphaMode(AlphaMode::UNSPECIFIED)
    , mpDevice(nullptr)
    , mVkImage(VK_NULL_HANDLE)
    , mVkImageView(VK_NULL_HANDLE)
    , mSizeInBytes(0)
    , mDeviceMemAlloc()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move constructor: relocate a texture to this object
//------------------------------------------------------------------------------------------------------------------------------------------
BaseTexture::BaseTexture(BaseTexture&& other) noexcept
    : mbIsValid(other.mbIsValid)
    , mFormat(other.mFormat)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mDepth(other.mDepth)
    , mNumLayers(other.mNumLayers)
    , mNumMipLevels(other.mNumMipLevels)
    , mNumSamples(other.mNumSamples)
    , mbIsCubemap(other.mbIsCubemap)
    , mAlphaMode(other.mAlphaMode)
    , mpDevice(other.mpDevice)
    , mVkImage(other.mVkImage)
    , mVkImageView(other.mVkImageView)
    , mSizeInBytes(other.mSizeInBytes)
    , mDeviceMemAlloc(other.mDeviceMemAlloc)
{
    other.mbIsValid = false;
    other.mFormat = VK_FORMAT_UNDEFINED;
    other.mWidth = 0;
    other.mHeight = 0;
    other.mDepth = 0;
    other.mNumLayers = 0;
    other.mNumMipLevels = 0;
    other.mNumSamples = 0;
    other.mbIsCubemap = false;
    other.mAlphaMode = AlphaMode::UNSPECIFIED;
    other.mpDevice = nullptr;
    other.mVkImage = VK_NULL_HANDLE;
    other.mVkImageView = VK_NULL_HANDLE;
    other.mSizeInBytes = 0;
    other.mDeviceMemAlloc = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destructor: does nothing intentionally
//------------------------------------------------------------------------------------------------------------------------------------------
BaseTexture::~BaseTexture() noexcept {
    // Note: not calling 'destroy()' here because it must be done the derived class destructor instead.
    // This is so the derived class version of 'destroy()' can be called, so the object is fully cleaned up.
    ASSERT_LOG((!mbIsValid), "Derived classes must call destroy() in their destructor!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs common destruction logic for the base texture class and frees up the texture resource
//------------------------------------------------------------------------------------------------------------------------------------------
void BaseTexture::destroy() noexcept {
    ASSERT_LOG(((!mpDevice) || mpDevice->getVkDevice()), "Parent device must still be valid if defined!");
    mbIsValid = false;

    // Image cleanup
    if (mVkImageView) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyImageView(mpDevice->getVkDevice(), mVkImageView, nullptr);
        mVkImageView = VK_NULL_HANDLE;
    }

    if (mVkImage) {
        ASSERT(mpDevice && mpDevice->getVkDevice());
        const VkFuncs& vkFuncs = mpDevice->getVkFuncs();
        vkFuncs.vkDestroyImage(mpDevice->getVkDevice(), mVkImage, nullptr);
        mVkImage = VK_NULL_HANDLE;
    }

    // Free up the device memory allocation for the image if there
    if (mDeviceMemAlloc.size > 0) {
        ASSERT(mpDevice);
        DeviceMemMgr& deviceMemMgr = mpDevice->getDeviceMemMgr();
        deviceMemMgr.dealloc(mDeviceMemAlloc);
    }

    // Cleanup other stuff
    mSizeInBytes = 0;
    mpDevice = nullptr;
    mAlphaMode = AlphaMode::UNSPECIFIED;
    mbIsCubemap = false;
    mNumSamples = 0;
    mNumMipLevels = 0;
    mNumLayers = 0;
    mDepth = 0;
    mHeight = 0;
    mWidth = 0;
    mFormat = VK_FORMAT_UNDEFINED;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal init implementation: allocates memory for the texture, creates image views and so on.
//------------------------------------------------------------------------------------------------------------------------------------------
bool BaseTexture::initInternal(
    LogicalDevice& device,
    const VkImageType vkImageType,
    const VkImageViewType vkImageViewType,
    const VkImageTiling vkImageTilingMode,
    const VkImageUsageFlags vkImageUsageFlags,
    const VkImageLayout vkInitialImageLayout,
    const DeviceMemAllocMode deviceMemAllocMode,
    const VkFormat textureFormat,
    const uint32_t width,
    const uint32_t height,
    const uint32_t depth,
    const uint32_t numLayers,
    const uint32_t numMipLevels,
    const uint32_t numSamples,
    const bool bIsCubemap,
    const AlphaMode alphaMode
) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(device.getVkDevice());
    ASSERT(width >= 1);
    ASSERT(height >= 1);
    ASSERT(depth >= 1);
    ASSERT(numLayers >= 1);
    ASSERT(numMipLevels >= 1);
    ASSERT(numSamples >= 1);
    ASSERT_LOG((depth == 1) || (numLayers == 1), "Can't specify array layers for a 3D texture!");
    
    // Verify valid alpha mode
    ASSERT(
        (alphaMode == AlphaMode::UNSPECIFIED) ||
        (alphaMode == AlphaMode::PREMULTIPLIED) ||
        (alphaMode == AlphaMode::STRAIGHT)
    );

    // Verify there are no mipmaps or the mip chain is complete
    ASSERT(
        (numMipLevels == 1) ||
        (TextureUtils::getNumMipLevels(width, height, depth) == numMipLevels)
    );

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy();
        }
    });

    // Save for future reference
    mFormat = textureFormat;
    mWidth = width;
    mHeight = height;
    mDepth = depth;
    mNumLayers = numLayers;
    mNumMipLevels = numMipLevels;
    mNumSamples = numSamples;
    mbIsCubemap = bIsCubemap;
    mAlphaMode = alphaMode;
    mpDevice = &device;
    
    // Get the image aspect flags for the texture format
    const VkImageAspectFlags vkImageAspectFlags = VkFormatUtils::getVkImageAspectFlags(textureFormat);

    // Create the following:
    //
    //  (1) The Vulkan image object
    //  (2) The memory allocation for the image
    //  (3) The image view for the image
    //
    if (!createVkImage(vkImageType, textureFormat, vkImageTilingMode, vkImageUsageFlags, vkInitialImageLayout))
        return false;

    if (!createImageMemBuffer(deviceMemAllocMode))
        return false;

    // Note: image view creation is optional depending on the texture type
    const bool bIsValidImageViewType = (
        (vkImageViewType == VK_IMAGE_VIEW_TYPE_1D) ||
        (vkImageViewType == VK_IMAGE_VIEW_TYPE_2D) ||
        (vkImageViewType == VK_IMAGE_VIEW_TYPE_3D) ||
        (vkImageViewType == VK_IMAGE_VIEW_TYPE_CUBE) ||
        (vkImageViewType == VK_IMAGE_VIEW_TYPE_1D_ARRAY) ||
        (vkImageViewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY) ||
        (vkImageViewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
    );

    if (bIsValidImageViewType) {
        if (!createImageView(vkImageViewType, textureFormat, vkImageAspectFlags))
            return false;
    }
    
    // Success if we get to here!
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the Vulkan image object for the texture
//------------------------------------------------------------------------------------------------------------------------------------------
bool BaseTexture::createVkImage(
    const VkImageType vkImageType,
    const VkFormat vkImageFormat,
    const VkImageTiling vkImageTilingMode,
    const VkImageUsageFlags vkImageUsageFlags,
    const VkImageLayout vkInitialImageLayout
) noexcept {
    // Preconditions: shouldn't call if the image already created!
    ASSERT(mVkImage == VK_NULL_HANDLE);

    // Populate the create info struct for the image and create it
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = vkImageType;
    imageInfo.format = vkImageFormat;
    imageInfo.extent.width = mWidth;
    imageInfo.extent.height = mHeight;
    imageInfo.extent.depth = mDepth;
    imageInfo.mipLevels = mNumMipLevels;
    imageInfo.arrayLayers = TextureUtils::getNumTexImages(mNumLayers, mbIsCubemap);     // Note: Vulkan counts cube faces as layers so count is multiplied by 6 for cubemaps
    imageInfo.samples = static_cast<VkSampleCountFlagBits>(mNumSamples);
    imageInfo.tiling = vkImageTilingMode;
    imageInfo.usage = vkImageUsageFlags;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = vkInitialImageLayout;

    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();

    if (vkFuncs.vkCreateImage(mpDevice->getVkDevice(), &imageInfo, nullptr, &mVkImage) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a Vulkan image object!");
        return false;
    }

    return true;    // Succeeded!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the memory buffer for the texture and binds it to the created image
//------------------------------------------------------------------------------------------------------------------------------------------
bool BaseTexture::createImageMemBuffer(const DeviceMemAllocMode deviceMemAllocMode) noexcept {
    // Preconditions
    ASSERT(mpDevice && mpDevice->getVkDevice());
    ASSERT_LOG(mVkImage, "Must create the Vulkan image first!");

    // Nab the memory related requirements of the image, including size, required memory types and alignment
    const VkDevice vkDevice = mpDevice->getVkDevice();
    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();

    VkMemoryRequirements memRequirements = {};
    vkFuncs.vkGetImageMemoryRequirements(vkDevice, mVkImage, &memRequirements);
    mSizeInBytes = memRequirements.size;
    
    // Get the device memory allocator and make sure it can support our alignment requirements
    DeviceMemMgr& deviceMemMgr = mpDevice->getDeviceMemMgr();

    if (deviceMemMgr.getMinAlignment() < memRequirements.alignment) {
        ASSERT_FAIL("Unable to satisfy alignment requirements for the image!");
        return false;
    }

    // Try to do a device memory alloc and if that fails (size <= 0) bail out
    deviceMemMgr.alloc(memRequirements.size, memRequirements.memoryTypeBits, deviceMemAllocMode, mDeviceMemAlloc);

    if (mDeviceMemAlloc.size <= 0)
        return false;

    // Okay, now that we have the buffer bind it to the image
    if (vkFuncs.vkBindImageMemory(vkDevice, mVkImage, mDeviceMemAlloc.vkDeviceMemory, mDeviceMemAlloc.offset) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to associate the Vulkan image with it's allocated memory!");
        return false;
    }

    return true;    // Succeeded!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the Vulkan image view for the texture
//------------------------------------------------------------------------------------------------------------------------------------------
bool BaseTexture::createImageView(
    const VkImageViewType vkImageViewType,
    const VkFormat vkImageFormat,
    const VkImageAspectFlags vkImageAspectFlags
) noexcept {
    // Preconditions: shouldn't call if the image view is already created!
    ASSERT(mVkImageView == VK_NULL_HANDLE);

    // Populate the create info struct for the view and create it
    VkImageViewCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = mVkImage;
    createInfo.viewType = vkImageViewType;
    createInfo.format = vkImageFormat;
    createInfo.components = {
        VK_COMPONENT_SWIZZLE_IDENTITY,      // R
        VK_COMPONENT_SWIZZLE_IDENTITY,      // G
        VK_COMPONENT_SWIZZLE_IDENTITY,      // B
        VK_COMPONENT_SWIZZLE_IDENTITY       // A
    };
    createInfo.subresourceRange.aspectMask = vkImageAspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = mNumMipLevels;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = TextureUtils::getNumTexImages(mNumLayers, mbIsCubemap);     // Note: Vulkan counts cube faces as layers so count is multiplied by 6 for cubemaps

    const VkFuncs& vkFuncs = mpDevice->getVkFuncs();

    if (vkFuncs.vkCreateImageView(mpDevice->getVkDevice(), &createInfo, nullptr, &mVkImageView) != VK_SUCCESS) {
        ASSERT_FAIL("Failed to create a Vulkan image view!");
        return false;
    }

    return true;    // Succeeded!
}

END_NAMESPACE(vgl)
