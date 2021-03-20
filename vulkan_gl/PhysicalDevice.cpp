#include "PhysicalDevice.h"

#include "Asserts.h"
#include "VkFuncs.h"

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper that does most of the work for finding a supported format matching a given condition
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static VkFormat findFirstMatchingFormat(
    const VkFuncs& vkFuncs,
    const VkPhysicalDevice vkPhysicalDevice,
    const VkFormat* const pFormats,
    const size_t numFormats,
    const VkFormatFeatureFlags requiredFeatureFlags,
    const T& areFormatPropsOk
) noexcept {
    ASSERT(pFormats || (numFormats == 0));

    for (size_t i = 0; i < numFormats; ++i) {
        const VkFormat format = pFormats[i];
        VkFormatProperties formatProps = {};
        vkFuncs.vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice, format, &formatProps);

        if (areFormatPropsOk(formatProps))
            return format;
    }

    return VK_FORMAT_UNDEFINED;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates this representation of the physical device using the device handle
//------------------------------------------------------------------------------------------------------------------------------------------
PhysicalDevice::PhysicalDevice(const VkFuncs& vkFuncs, VulkanInstance& vulkanInstance, const VkPhysicalDevice vkPhysicalDevice) noexcept
    : mVkFuncs(vkFuncs)
    , mVulkanInstance(vulkanInstance)
    , mVkPhysicalDevice(vkPhysicalDevice)
    , mProps()
    , mFeatures()
    , mExtensions()
    , mMemProps()
    , mDeviceMem(0)
    , mNonDeviceMem(0)
    , mQueueFamilyProps()
    , mGraphicsQueueFamilyIndexes()
    , mComputeQueueFamilyIndexes()
    , mTransferQueueFamilyIndexes()
{
    ASSERT(vkPhysicalDevice);

    // Query device properties and features
    mVkFuncs.vkGetPhysicalDeviceProperties(vkPhysicalDevice, &mProps);
    mVkFuncs.vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &mFeatures);
    mVkFuncs.vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &mMemProps);

    // Sum up the amount of available device and non device memory
    for (uint32_t i = 0; i < mMemProps.memoryHeapCount; ++i) {
        const VkMemoryHeap& heap = mMemProps.memoryHeaps[i];

        if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
            mDeviceMem += heap.size;
        } else {
            mNonDeviceMem += heap.size;
        }
    }

    // Get the queue family properties
    {
        // Query how many queue families there are
        uint32_t count = 0;
        mVkFuncs.vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, nullptr);

        // Then get all of the queue family properties
        mQueueFamilyProps.resize(count);
        mVkFuncs.vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &count, mQueueFamilyProps.data());
    }

    // See what queue families support which operations
    {
        uint32_t queueFamilyIndex = 0;
        mGraphicsQueueFamilyIndexes.reserve(mQueueFamilyProps.size());
        mComputeQueueFamilyIndexes.reserve(mQueueFamilyProps.size());
        mTransferQueueFamilyIndexes.reserve(mQueueFamilyProps.size());

        for (const VkQueueFamilyProperties& queueFamilyProps : mQueueFamilyProps) {
            // Ignore if there are no queues in this family
            if (queueFamilyProps.queueCount > 0) {
                // Nab the flags for the queue and see what operations are allowed
                VkQueueFlags flags = queueFamilyProps.queueFlags;

                // See if the queue supports graphics operations
                if (flags & VK_QUEUE_GRAPHICS_BIT) {
                    mGraphicsQueueFamilyIndexes.push_back(queueFamilyIndex);
                }

                // See if the queue supports compute operations
                if (flags & VK_QUEUE_COMPUTE_BIT) {
                    mComputeQueueFamilyIndexes.push_back(queueFamilyIndex);
                }

                // See if the queue supports transfer operations.
                // Note that any queue that supports GFX or compute operations also implicitly supports transfer operations too:
                if (flags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT)) {
                    mTransferQueueFamilyIndexes.push_back(queueFamilyIndex);
                }
            }

            ++queueFamilyIndex;
        }
    }

    // Query device extensions
    mExtensions.init(vkFuncs, vkPhysicalDevice);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to locate a suitable memory type among the available memory types in the device.
// Returns the index of the memory type found or UINT32_MAX if no suitable memory type was found.
// If there are multiple suitable memory types then the first suitable memory type found is returned.
//
// @param   allowedMemTypesIndexMask    A bit mask which specifies which memory type indexes are allowed.
//                                      Each bit represents an index in the memory type array, with bit 0
//                                      corresponding to index 0, bit 1 corresponding to index 1 and so on.
// @param   requiredMemProps    An optional mask of memory type property flags which are required.
//                              Can be used to ask for a more specific type of memory, such as
//                              device local, host visible etc.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t PhysicalDevice::findSuitableMemTypeIndex(
    const uint32_t allowedMemTypesIndexMask,
    const VkMemoryPropertyFlags requiredMemProps
) const noexcept {
    // Run through all memory types and see which ones are allowed to be used according to the index
    // mask passed into the function. Each bit represents whether that memory index is usable or not:
    for (uint32_t curMemTypeIndex = 0; curMemTypeIndex < mMemProps.memoryTypeCount; ++curMemTypeIndex) {
        const uint32_t curMemTypeIndexMask = 1 << curMemTypeIndex;

        if ((allowedMemTypesIndexMask & curMemTypeIndexMask) != 0) {
            // Okay this memory type index is in the list of indexes allowed. If we don't require any particular memory
            // type properties ('0' passed) then we can just return this memory type now, without checking any properties:
            if (requiredMemProps == 0)
                return curMemTypeIndex;

            // Otherwise only return the memory type if it has the properties we are seeking:
            const VkMemoryPropertyFlags curMemTypeProps = mMemProps.memoryTypes[curMemTypeIndex].propertyFlags;

            if ((curMemTypeProps & requiredMemProps) == requiredMemProps)
                return curMemTypeIndex;
        }
    }

    return UINT32_MAX;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Search for the first supported format for linear tiling, optimal tiling and buffer usage
//------------------------------------------------------------------------------------------------------------------------------------------
VkFormat PhysicalDevice::findFirstSupportedLinearTilingFormat(
    const VkFormat* const pFormats,
    const size_t numFormats,
    const VkFormatFeatureFlags requiredFeatureFlags
) const noexcept {
    return findFirstMatchingFormat(
        mVkFuncs,
        mVkPhysicalDevice,
        pFormats,
        numFormats,
        requiredFeatureFlags,
        [&](const VkFormatProperties& props) noexcept -> bool {
            return ((props.linearTilingFeatures & requiredFeatureFlags) == requiredFeatureFlags);
        }
    );
}

VkFormat PhysicalDevice::findFirstSupportedOptimalTilingFormat(
    const VkFormat* const pFormats,
    const size_t numFormats,
    const VkFormatFeatureFlags requiredFeatureFlags
) const noexcept {
    return findFirstMatchingFormat(
        mVkFuncs,
        mVkPhysicalDevice,
        pFormats,
        numFormats,
        requiredFeatureFlags,
        [&](const VkFormatProperties& props) noexcept -> bool {
            return ((props.optimalTilingFeatures & requiredFeatureFlags) == requiredFeatureFlags);
        }
    );
}

VkFormat PhysicalDevice::findFirstSupportedBufferFormat(
    const VkFormat* const pFormats,
    const size_t numFormats,
    const VkFormatFeatureFlags requiredFeatureFlags
) const noexcept {
    return findFirstMatchingFormat(
        mVkFuncs,
        mVkPhysicalDevice,
        pFormats,
        numFormats,
        requiredFeatureFlags,
        [&](const VkFormatProperties& props) noexcept -> bool {
            return ((props.bufferFeatures & requiredFeatureFlags) == requiredFeatureFlags);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Search for the first supported format for textures, render textures and depth stencil buffers.
// These just automatically add flags.
//------------------------------------------------------------------------------------------------------------------------------------------
VkFormat PhysicalDevice::findFirstSupportedTextureFormat(
    const VkFormat* const pFormats,
    const size_t numFormats,
    const VkImageTiling imageTiling,
    const VkFormatFeatureFlags requiredFeatureFlags
) const noexcept {
    constexpr VkFormatFeatureFlags BASE_FEATURE_FLAGS = (
        VK_FORMAT_FEATURE_TRANSFER_DST_BIT |
        VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
    );

    if (imageTiling == VK_IMAGE_TILING_OPTIMAL) {
        return findFirstSupportedOptimalTilingFormat(pFormats, numFormats, BASE_FEATURE_FLAGS | requiredFeatureFlags);
    } else if (imageTiling == VK_IMAGE_TILING_LINEAR) {
        return findFirstSupportedLinearTilingFormat(pFormats, numFormats, BASE_FEATURE_FLAGS | requiredFeatureFlags);
    }

    return VK_FORMAT_UNDEFINED;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Search for the first supported device format in a list of formats for 'RenderTexture' objects that are a color attachment.
// Optionally, additional required format feature flags can be supplied.
// On failure to find a supported format, 'VK_FORMAT_UNDEFINED' is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
VkFormat PhysicalDevice::findFirstSupportedRenderTextureFormat(
    const VkFormat* const pFormats,
    const size_t numFormats,
    const VkImageTiling imageTiling,
    const VkFormatFeatureFlags requiredFeatureFlags
) const noexcept {
    constexpr VkFormatFeatureFlags BASE_FEATURE_FLAGS = VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;

    if (imageTiling == VK_IMAGE_TILING_OPTIMAL) {
        return findFirstSupportedOptimalTilingFormat(pFormats, numFormats, BASE_FEATURE_FLAGS | requiredFeatureFlags);
    } else if (imageTiling == VK_IMAGE_TILING_LINEAR) {
        return findFirstSupportedLinearTilingFormat(pFormats, numFormats, BASE_FEATURE_FLAGS | requiredFeatureFlags);
    }

    return VK_FORMAT_UNDEFINED;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Search for the first supported device format in a list of formats for 'RenderTexture' objects that are a depth/stencil attachment.
// Optionally, additional required format feature flags can be supplied.
// On failure to find a supported format, 'VK_FORMAT_UNDEFINED' is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
VkFormat PhysicalDevice::findFirstSupportedDepthStencilBufferFormat(
    const VkFormat* const pFormats,
    const size_t numFormats,
    const VkImageTiling imageTiling,
    const VkFormatFeatureFlags requiredFeatureFlags
) const noexcept {
    constexpr VkFormatFeatureFlags BASE_FEATURE_FLAGS = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    if (imageTiling == VK_IMAGE_TILING_OPTIMAL) {
        return findFirstSupportedOptimalTilingFormat(pFormats, numFormats, BASE_FEATURE_FLAGS | requiredFeatureFlags);
    } else if (imageTiling == VK_IMAGE_TILING_LINEAR) {
        return findFirstSupportedLinearTilingFormat(pFormats, numFormats, BASE_FEATURE_FLAGS | requiredFeatureFlags);
    }

    return VK_FORMAT_UNDEFINED;
}

END_NAMESPACE(vgl)
