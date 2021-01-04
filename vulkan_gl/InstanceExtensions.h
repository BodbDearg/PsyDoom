#pragma once

#include "Macros.h"

#include <vector>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

struct VkFuncs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Provides details for the Vulkan instance level extensions that are available.
// These are extensions which are not specific to any particular device.
//------------------------------------------------------------------------------------------------------------------------------------------
class InstanceExtensions {
public:
    InstanceExtensions() noexcept;

    void init(const VkFuncs& vkFuncs) noexcept;
    bool hasExtension(const char* const pName) const noexcept;
    const VkExtensionProperties* getExtensionByName(const char* const pName) const noexcept;

    // Get all instance extensions
    inline const std::vector<VkExtensionProperties>& getAllExtensions() const noexcept { return mExtProps; }

private:
    // Properties for all the instance level extensions supported by the Vulkan implementation.
    // The list is sorted by the extension id.
    std::vector<VkExtensionProperties> mExtProps;
};

END_NAMESPACE(vgl)
