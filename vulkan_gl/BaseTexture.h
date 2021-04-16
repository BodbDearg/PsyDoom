#pragma once

#include "DeviceMemAlloc.h"

BEGIN_NAMESPACE(vgl)

class LogicalDevice;
enum class AlphaMode : uint8_t;
enum class DeviceMemAllocMode : uint8_t;

//------------------------------------------------------------------------------------------------------------------------------------------
// Base class for texture type resources.
// Texture type resources include ordinary draw textures, render targets and depth buffers.
//------------------------------------------------------------------------------------------------------------------------------------------
class BaseTexture {
public:
    inline bool isValid() const noexcept { return mbIsValid; }
    inline VkFormat getFormat() const noexcept { return mFormat; }
    inline uint32_t getWidth() const noexcept { return mWidth; }
    inline uint32_t getHeight() const noexcept { return mHeight; }
    inline uint32_t getDepth() const noexcept { return mDepth; }
    inline uint32_t getNumLayers() const noexcept { return mNumLayers; }
    inline uint32_t getNumMipLevels() const noexcept { return mNumMipLevels; }
    inline uint32_t getNumSamples() const noexcept { return mNumSamples; }
    inline bool isCubemap() const noexcept { return mbIsCubemap; }
    inline AlphaMode getAlphaMode() const noexcept { return mAlphaMode; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkImage getVkImage() const noexcept { return mVkImage; }
    inline VkImageView getVkImageView() const noexcept { return mVkImageView; }
    inline uint64_t getSizeInBytes() const noexcept { return mSizeInBytes; }
    inline uint64_t getAllocSizeInBytes() const noexcept { return mDeviceMemAlloc.size; }

protected:
    BaseTexture() noexcept;
    BaseTexture(BaseTexture&& other) noexcept;
    ~BaseTexture() noexcept;

    void destroy() noexcept;

    bool initInternal(
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
    ) noexcept;

    bool                mbIsValid;
    VkFormat            mFormat;            // Texture format for the texture surface
    uint32_t            mWidth;
    uint32_t            mHeight;
    uint32_t            mDepth;
    uint32_t            mNumLayers;
    uint32_t            mNumMipLevels;
    uint32_t            mNumSamples;
    bool                mbIsCubemap;
    AlphaMode           mAlphaMode;
    LogicalDevice*      mpDevice;
    VkImage             mVkImage;
    VkImageView         mVkImageView;
    uint64_t            mSizeInBytes;       // How many bytes are required to store the image, according to Vulkan
    DeviceMemAlloc      mDeviceMemAlloc;    // Device memory buffer allocation for the image memory

private:
    // Copy and move assign disallowed
    BaseTexture(const BaseTexture& other) = delete;
    BaseTexture operator = (const BaseTexture& other) = delete;
    BaseTexture operator = (BaseTexture&& other) = delete;

    // Retirement manager has intrusive access (for modifying retirement state)
    friend class RetirementMgr;

    bool createVkImage(
        const VkImageType vkImageType,
        const VkFormat vkImageFormat,
        const VkImageTiling vkImageTilingMode,
        const VkImageUsageFlags vkImageUsageFlags,
        const VkImageLayout vkInitialImageLayout
    ) noexcept;

    bool createImageMemBuffer(const DeviceMemAllocMode deviceMemAllocMode) noexcept;

    bool createImageView(
        const VkImageViewType vkImageViewType,
        const VkFormat vkImageFormat,
        const VkImageAspectFlags vkImageAspectFlags
    ) noexcept;
};

END_NAMESPACE(vgl)
