#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "BaseRenderPass.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// The single Vulkan render pass used by PsyDoom.
// If MSAA is disabled it consists of only a single drawing subpass.
// If MSAA is enabled an additional subpass is added for resolving the MSAA framebuffer to a single sample.
//------------------------------------------------------------------------------------------------------------------------------------------
class VRenderPass : public vgl::BaseRenderPass {
public:
    VRenderPass() noexcept;
    ~VRenderPass() noexcept;

    bool init(
        vgl::LogicalDevice& device,
        const VkFormat colorFormat,
        const VkFormat depthStencilFormat,
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
