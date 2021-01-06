#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "BaseRenderPass.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// A simple Vulkan render pass that has a single color and depth attachment and which has a single subpass.
// Intended for drawing to a framebuffer of some sort.
//------------------------------------------------------------------------------------------------------------------------------------------
class VDrawRenderPass : public vgl::BaseRenderPass {
public:
    VDrawRenderPass() noexcept;
    ~VDrawRenderPass() noexcept;

    bool init(vgl::LogicalDevice& device, const VkFormat colorFormat, const VkFormat depthStencilFormat) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline VkFormat getColorFormat() const noexcept { return mColorFormat; }
    inline VkFormat getDepthStencilFormat() const noexcept { return mDepthStencilFormat; }

private:
    // Copy and move disallowed
    VDrawRenderPass(const VDrawRenderPass& other) = delete;
    VDrawRenderPass(VDrawRenderPass&& other) = delete;
    VDrawRenderPass& operator = (const VDrawRenderPass& other) = delete;
    VDrawRenderPass& operator = (VDrawRenderPass&& other) = delete;

    VkFormat    mColorFormat;
    VkFormat    mDepthStencilFormat;
};

#endif  // #if PSYDOOM_VULKAN_RENDERER
