#pragma once

#include "Macros.h"

#include <vector>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

struct VkFuncs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Provides details on which Vulkan extensions are available for the given physical device.
// Device level extensions are specific to each device, as opposed to instance extensions which extend the Vulkan API.
//------------------------------------------------------------------------------------------------------------------------------------------
class DeviceExtensions {
public:
    DeviceExtensions() noexcept;

    void init(const VkFuncs& vkFuncs, const VkPhysicalDevice vkPhysicalDevice) noexcept;
    bool hasExtension(const char* const pName) const noexcept;
    const VkExtensionProperties* getExtensionByName(const char* const pName) const noexcept;

    // Get all available extensions
    inline const std::vector<VkExtensionProperties>& getAllExtensions() const noexcept { return mExtProps; }

private:
    // Properties for all the device level extensions supported by the Vulkan implementation.
    // The list is sorted by the extension id.
    std::vector<VkExtensionProperties>  mExtProps;
};

END_NAMESPACE(vgl)
