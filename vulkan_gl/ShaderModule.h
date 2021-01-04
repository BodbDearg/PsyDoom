#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

BEGIN_NAMESPACE(vgl)

class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a Vulkan shader module.
// A shader module is basically a dumb wrapper around some compiled SPIR-V code.
//------------------------------------------------------------------------------------------------------------------------------------------
class ShaderModule {
public:
    ShaderModule() noexcept;
    ShaderModule(ShaderModule&& other) noexcept;
    ~ShaderModule() noexcept;

    bool init(
        LogicalDevice& device,
        const VkShaderStageFlagBits stageFlags,
        const uint32_t* const pCode,
        const size_t codeSize
    ) noexcept;

    void destroy(const bool bImmediately = false, const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline VkShaderModule getVkShaderModule() const noexcept { return mVkShaderModule; }
    inline VkShaderStageFlagBits getVkShaderStageFlagBits() const noexcept { return mVkShaderStageFlagBits; }

private:
    // Copy and move assign are disallowed
    ShaderModule(const ShaderModule& other) = delete;
    ShaderModule& operator = (const ShaderModule& other) = delete;
    ShaderModule& operator = (ShaderModule&& other) = delete;

    bool                    mbIsValid;
    LogicalDevice*          mpDevice;
    VkShaderStageFlagBits   mVkShaderStageFlagBits;
    VkShaderModule          mVkShaderModule;
};

END_NAMESPACE(vgl)
