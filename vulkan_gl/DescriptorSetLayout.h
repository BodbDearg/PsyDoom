#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan descriptor set layout.
// A descriptor set layout describes the type of resources accessed by shaders.
//------------------------------------------------------------------------------------------------------------------------------------------
class DescriptorSetLayout {
public:
    DescriptorSetLayout() noexcept;
    DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
    ~DescriptorSetLayout() noexcept;

    bool init(
        LogicalDevice& device,
        const VkDescriptorSetLayoutBinding* const pLayoutBindings,
        const uint32_t numLayoutBindings
    ) noexcept;

    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkDescriptorSetLayout getVkLayout() const noexcept { return mVkLayout; }

private:
    // Copy and move assign disallowed
    DescriptorSetLayout(const DescriptorSetLayout& other) = delete;
    DescriptorSetLayout& operator = (const DescriptorSetLayout& other) = delete;
    DescriptorSetLayout& operator = (DescriptorSetLayout&& other) = delete;
    
    bool                    mbIsValid;      // True if the layout has been validly initialized/created
    LogicalDevice*          mpDevice;       // Device the layout was created with
    VkDescriptorSetLayout   mVkLayout;      // The Vulkan descriptor set layout object
};

END_NAMESPACE(vgl)
