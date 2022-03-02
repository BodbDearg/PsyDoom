#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"
#include "Framebuffer.h"
#include "IVRenderPath.h"
#include "RenderPass.h"
#include "VMsaaResolver.h"

#include <vector>

class VRenderPath_Main;

namespace vgl {
    class Framebuffer;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// A render path used for crossfading two previously screens which were done via the 'main' renderpath.
// Outputs directly to the swapchain image, so it doesn't disturb the previous screens drawn.
//------------------------------------------------------------------------------------------------------------------------------------------
class VRenderPath_Crossfade : public IVRendererPath {
public:
    VRenderPath_Crossfade() noexcept;
    ~VRenderPath_Crossfade() noexcept;

    void init(
        vgl::LogicalDevice& device,
        vgl::Swapchain& swapchain,
        const VkFormat presentSurfaceFormat,
        VRenderPath_Main& mainRenderPath
    ) noexcept;

    void destroy() noexcept;
    virtual bool ensureValidFramebuffers(const uint32_t fbWidth, const uint32_t fbHeight) noexcept override;
    virtual void beginFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept override;
    virtual void endFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept override;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline const vgl::RenderPass& getRenderPass() const { return mRenderPass; }

    void setOldFramebufferTextures(vgl::RenderTexture* const pOldFbTex1, vgl::RenderTexture* const pOldFbTex2) noexcept;

private:
    bool initRenderPass() noexcept;
    bool doFramebuffersNeedRecreate() noexcept;
    void transitionOldFramebufferTexLayout(vgl::RenderTexture& tex, vgl::CmdBufferRecorder& cmdRec) noexcept;

    bool                            mbIsValid;                  // True if the render path has been initialized
    vgl::LogicalDevice*             mpDevice;                   // The vulkan device used
    vgl::Swapchain*                 mpSwapchain;                // The swapchain used
    VkFormat                        mPresentSurfaceFormat;      // The format for the swapchain image we present to (the output destination for this render path)
    VRenderPath_Main*               mpMainRenderPath;           // The main render path: used to source the images to crossfade between
    vgl::RenderPass                 mRenderPass;                // The Vulkan renderpass for this render path
    std::vector<vgl::Framebuffer>   mFramebuffers;              // Framebuffers for each swapchain image
    vgl::RenderTexture*             mpOldFbTextures[2];         // The old framebuffer textures to crossfade
};

#endif  // #if PSYDOOM_VULKAN_RENDERER
