#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan pipeline layout for a graphics or compute pipeline. 
// Pipeline layouts specify what descriptor set layouts are active for a given pipeline.
// They also specify which push constants are used by which shader stages.
//------------------------------------------------------------------------------------------------------------------------------------------
class PipelineLayout {
public:
    PipelineLayout() noexcept;
    PipelineLayout(PipelineLayout&& other) noexcept;
    ~PipelineLayout() noexcept;

    bool init(
        LogicalDevice& device,
        VkDescriptorSetLayout* const pDescriptorSetLayouts,
        const uint32_t numDescriptorSetLayouts,
        VkPushConstantRange* const pPushConstantRanges,
        const uint32_t numPushConstantRanges
    ) noexcept;

    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkPipelineLayout getVkPipelineLayout() const noexcept { return mVkPipelineLayout; }

private:
    // Copy and move assign are disallowed
    PipelineLayout(const PipelineLayout& other) = delete;
    PipelineLayout& operator = (const PipelineLayout& other) = delete;
    PipelineLayout& operator = (PipelineLayout&& other) = delete;

    bool                mbIsValid;          // True if the layout has been validly initialized/created
    LogicalDevice*      mpDevice;           // The device that the pipeline layout was created with
    VkPipelineLayout    mVkPipelineLayout;  // The Vulkan pipeline layout object
};

END_NAMESPACE(vgl)
