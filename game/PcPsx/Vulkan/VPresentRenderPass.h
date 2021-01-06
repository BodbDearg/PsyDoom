#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "BaseRenderPass.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// A simple Vulkan render pass that has a single color attachment and which has a single subpass.
// Intended for presenting a previously rendered attachment to the screen and transitioning image layouts.
//------------------------------------------------------------------------------------------------------------------------------------------
class VPresentRenderPass : public vgl::BaseRenderPass {
public:
    VPresentRenderPass() noexcept;
    ~VPresentRenderPass() noexcept;

    bool init(vgl::LogicalDevice& device, const VkFormat colorFormat) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline VkFormat getColorFormat() const noexcept { return mColorFormat; }

private:
    // Copy and move disallowed
    VPresentRenderPass(const VPresentRenderPass& other) = delete;
    VPresentRenderPass(VPresentRenderPass&& other) = delete;
    VPresentRenderPass& operator = (const VPresentRenderPass& other) = delete;
    VPresentRenderPass& operator = (VPresentRenderPass&& other) = delete;

    VkFormat    mColorFormat;
};

#endif  // #if PSYDOOM_VULKAN_RENDERER
