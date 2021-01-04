#pragma once

#include "Macros.h"

#include <vector>
#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

struct VkFuncs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Class containing details on the available API layers for Vulkan, including any validation layers.
// 
// These are all instance level layers since device specific layers have been deprecated.
// Instance level layers are now applied to all device calls.
//------------------------------------------------------------------------------------------------------------------------------------------
class ApiLayers {
public:
    ApiLayers() noexcept;

    void init(const VkFuncs& vkFuncs) noexcept;
    bool hasLayer(const char* const pName) const noexcept;
    const VkLayerProperties* getLayerByName(const char* const pName) const noexcept;

    // Get all API layers
    inline const std::vector<VkLayerProperties>& getAllLayers() const noexcept { return mLayerProps; }

private:
    // Properties for all the API layers supported by Vulkan on this machine
    std::vector<VkLayerProperties> mLayerProps;
};

END_NAMESPACE(vgl)
