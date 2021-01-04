#pragma once

#include "Macros.h"

#include "vulkan/vulkan.h"
#include <vector>

BEGIN_NAMESPACE(vgl)

class PhysicalDevice;
class WindowSurface;

//------------------------------------------------------------------------------------------------------------------------------------------
// Gathers and provides access to capabilities of a given Vulkan physical device with respect to a given surface
//------------------------------------------------------------------------------------------------------------------------------------------
class DeviceSurfaceCaps {
public:
    DeviceSurfaceCaps() noexcept;
    ~DeviceSurfaceCaps() noexcept;

    bool query(const PhysicalDevice& device, const WindowSurface& windowSurface) noexcept;
    void clear() noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline const std::vector<uint32_t>& getPresentCapableQueueFamilies() const noexcept { return mPresentCapableQueueFamilies; }
    inline const VkSurfaceCapabilitiesKHR& getVkSurfaceCapabilities() const noexcept { return mVkSurfaceCapabilities; }
    inline const std::vector<VkSurfaceFormatKHR>& getVkSurfaceFormats() const noexcept { return mVkSurfaceFormats; }
    inline const std::vector<VkPresentModeKHR>& getVkPresentModes() const noexcept { return mVkPresentModes; }

    VkFormat getSupportedSurfaceFormat(const VkFormat* const pFormats, const uint32_t numFormats) const noexcept;
    bool isZeroSizedMaxImageExtent() const noexcept;

    // Querying the width and height of the min and max image extent
    inline uint32_t getMinImageExtentWidth() const noexcept { return mVkSurfaceCapabilities.minImageExtent.width; }
    inline uint32_t getMinImageExtentHeight() const noexcept { return mVkSurfaceCapabilities.minImageExtent.height; }
    inline uint32_t getMaxImageExtentWidth() const noexcept { return mVkSurfaceCapabilities.maxImageExtent.width; }
    inline uint32_t getMaxImageExtentHeight() const noexcept { return mVkSurfaceCapabilities.maxImageExtent.height; }

private:

    bool                                mbIsValid;                      // True if this capabilities data structure was successfully queried
    std::vector<uint32_t>               mPresentCapableQueueFamilies;   // Which queue families on the device are capable of presenting to the surface
    VkSurfaceCapabilitiesKHR            mVkSurfaceCapabilities;         // Capabilities for the device with the surface
    std::vector<VkSurfaceFormatKHR>     mVkSurfaceFormats;              // Supported surface formats
    std::vector<VkPresentModeKHR>       mVkPresentModes;                // Supported present modes
};

END_NAMESPACE(vgl)
