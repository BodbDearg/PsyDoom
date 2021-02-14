#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "BaseRenderPass.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// The Vulkan render pass used by PsyDoom for pretty much all drawing, except for a few corner cases like screen crossfades.
// Consists of multiple subpasses, the amount of which depends on graphic settings.
//------------------------------------------------------------------------------------------------------------------------------------------
class VMainRenderPass : public vgl::BaseRenderPass {
public:
    VMainRenderPass() noexcept;
    ~VMainRenderPass() noexcept;

    bool init(
        vgl::LogicalDevice& device,
        const VkFormat colorFormat,
        const VkFormat colorMsaaResolveFormat,
        const uint32_t sampleCount
    ) noexcept;

    void destroy(const bool bForceIfInvalid = false) noexcept;

private:
    // Copy and move disallowed
    VMainRenderPass(const VMainRenderPass& other) = delete;
    VMainRenderPass(VMainRenderPass&& other) = delete;
    VMainRenderPass& operator = (const VMainRenderPass& other) = delete;
    VMainRenderPass& operator = (VMainRenderPass&& other) = delete;
};

#endif  // #if PSYDOOM_VULKAN_RENDERER
