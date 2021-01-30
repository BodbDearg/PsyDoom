#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "BaseRenderPass.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// The single Vulkan render pass used by PsyDoom.
// Consists of multiple subpasses, the amount of which depends on graphic settings.
//------------------------------------------------------------------------------------------------------------------------------------------
class VRenderPass : public vgl::BaseRenderPass {
public:
    VRenderPass() noexcept;
    ~VRenderPass() noexcept;

    bool init(
        vgl::LogicalDevice& device,
        const VkFormat colorFormat,
        const VkFormat depthFormat,
        const VkFormat colorMsaaResolveFormat,
        const uint32_t sampleCount
    ) noexcept;

    void destroy(const bool bForceIfInvalid = false) noexcept;

private:
    // Copy and move disallowed
    VRenderPass(const VRenderPass& other) = delete;
    VRenderPass(VRenderPass&& other) = delete;
    VRenderPass& operator = (const VRenderPass& other) = delete;
    VRenderPass& operator = (VRenderPass&& other) = delete;
};

#endif  // #if PSYDOOM_VULKAN_RENDERER
