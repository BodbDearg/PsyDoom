#pragma once

#include "DeviceExtensions.h"

BEGIN_NAMESPACE(vgl)

class VulkanInstance;
struct VkFuncs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan physical device, from which a Vulkan logical device handle can be created.
// Contains info on the device and it's capabilities.
//------------------------------------------------------------------------------------------------------------------------------------------
class PhysicalDevice {
public:
    PhysicalDevice(const VkFuncs& vkFuncs, VulkanInstance& vulkanInstance, const VkPhysicalDevice vkPhysicalDevice) noexcept;

    // Get various device properties
    inline const VkFuncs& getVkFuncs() const noexcept { return mVkFuncs; }
    inline VulkanInstance& getVulkanInstance() const noexcept { return mVulkanInstance; }
    inline VkPhysicalDevice getVkPhysicalDevice() const noexcept { return mVkPhysicalDevice; }
    inline const VkPhysicalDeviceProperties& getProps() const noexcept { return mProps; }
    inline const VkPhysicalDeviceFeatures& getFeatures() const noexcept { return mFeatures; }
    inline const DeviceExtensions& getExtensions() const noexcept { return mExtensions; }
    inline const VkPhysicalDeviceMemoryProperties& getMemProps() const noexcept { return mMemProps; }
    inline uint64_t getDeviceMem() const noexcept { return mDeviceMem; }
    inline uint64_t getNonDeviceMem() const noexcept { return mNonDeviceMem; }
    inline const std::vector<VkQueueFamilyProperties>& getQueueFamilyProps() const noexcept { return mQueueFamilyProps; }
    inline const std::vector<uint32_t> getGraphicsQueueFamilyIndexes() const noexcept { return mGraphicsQueueFamilyIndexes; }
    inline const std::vector<uint32_t> getComputeQueueFamilyIndexes() const noexcept { return mComputeQueueFamilyIndexes; }
    inline const std::vector<uint32_t> getTransferQueueFamilyIndexes() const noexcept { return mTransferQueueFamilyIndexes; }

    // Gives the name of the device as a null terminated C-String
    inline const char* getName() const noexcept { return mProps.deviceName; }
    
    uint32_t findSuitableMemTypeIndex(
        const uint32_t allowedMemTypesIndexMask,
        const VkMemoryPropertyFlags requiredMemProps = 0
    ) const noexcept;

    VkFormat findFirstSupportedLinearTilingFormat(
        const VkFormat* const pFormats,
        const size_t numFormats,
        const VkFormatFeatureFlags requiredFeatureFlags
    ) const noexcept;

    VkFormat findFirstSupportedOptimalTilingFormat(
        const VkFormat* const pFormats,
        const size_t numFormats,
        const VkFormatFeatureFlags requiredFeatureFlags
    ) const noexcept;

    VkFormat findFirstSupportedBufferFormat(
        const VkFormat* const pFormats,
        const size_t numFormats,
        const VkFormatFeatureFlags requiredFeatureFlags
    ) const noexcept;

    VkFormat findFirstSupportedTextureFormat(
        const VkFormat* const pFormats,
        const size_t numFormats,
        const VkImageTiling imageTiling,
        const VkFormatFeatureFlags requiredFeatureFlags
    ) const noexcept;

    VkFormat findFirstSupportedRenderTextureFormat(
        const VkFormat* const pFormats,
        const size_t numFormats,
        const VkImageTiling imageTiling,
        const VkFormatFeatureFlags requiredFeatureFlags
    ) const noexcept;

    VkFormat findFirstSupportedDepthStencilBufferFormat(
        const VkFormat* const pFormats,
        const size_t numFormats,
        const VkImageTiling imageTiling,
        const VkFormatFeatureFlags requiredFeatureFlags
    ) const noexcept;

    // Get the minimum required alignment for uniform buffers
    inline size_t getMinUniformBufferAlignment() const noexcept {
        return mProps.limits.minUniformBufferOffsetAlignment;
    }

    // Get the maximum level of anisotropic filtering supported
    inline float getMaxAnisotropy() const noexcept {
        return mProps.limits.maxSamplerAnisotropy;
    }

private:

    const VkFuncs&                          mVkFuncs;                       // Pointers to Vulkan API functions
    VulkanInstance&                         mVulkanInstance;                // The Vulkan instane the physical device is associated with
    const VkPhysicalDevice                  mVkPhysicalDevice;              // Handle to the Vulkan physical device
    VkPhysicalDeviceProperties              mProps;                         // Properties for the physical device
    VkPhysicalDeviceFeatures                mFeatures;                      // Features for the physical device
    DeviceExtensions                        mExtensions;                    // What extensions are supported by the physical device
    VkPhysicalDeviceMemoryProperties        mMemProps;                      // Description of available memory for the physical device
    uint64_t                                mDeviceMem;                     // Number of bytes available for on-device memory
    uint64_t                                mNonDeviceMem;                  // Number of bytes available for general system memory
    std::vector<VkQueueFamilyProperties>    mQueueFamilyProps;              // Properties for each queue family
    std::vector<uint32_t>                   mGraphicsQueueFamilyIndexes;    // Which queue families support graphics operations
    std::vector<uint32_t>                   mComputeQueueFamilyIndexes;     // Which queue families support compute operations
    std::vector<uint32_t>                   mTransferQueueFamilyIndexes;    // Which queue families support transfer operations
};

END_NAMESPACE(vgl)
