#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"
#include "Framebuffer.h"
#include "IVRenderPath.h"
#include "RenderPass.h"
#include "VMsaaResolver.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// This is the primary renderer path for the new Vulkan renderer.
// All normal drawing operations are done via this render path.
//------------------------------------------------------------------------------------------------------------------------------------------
class VRenderPath_Main : public IVRendererPath {
public:
    VRenderPath_Main() noexcept;
    ~VRenderPath_Main() noexcept;

    void init(
        vgl::LogicalDevice& device,
        const uint32_t numDrawSamples,
        const VkFormat colorFormat,
        const VkFormat resolveFormat
    ) noexcept;

    void destroy() noexcept;
    virtual bool ensureValidFramebuffers(const uint32_t fbWidth, const uint32_t fbHeight) noexcept override;
    virtual void beginFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept override;
    virtual void endFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept override;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline uint32_t getNumDrawSamples() const noexcept { return mNumDrawSamples; }
    inline const vgl::RenderPass& getRenderPass() const noexcept { return mRenderPass; }
    inline VMsaaResolver& getMsaaResolver() noexcept { return mMsaaResolver; }

    inline vgl::RenderTexture& getFramebufferAttachment(const uint32_t idx) noexcept {
        ASSERT(idx < vgl::Defines::RINGBUFFER_SIZE);
        return mColorAttachments[idx];
    }

    bool didRenderToAllFramebuffers() noexcept;

private:
    bool initRenderPass() noexcept;

    bool                    mbIsValid;          // True if the render path has been initialized
    vgl::LogicalDevice*     mpDevice;           // The vulkan device used
    uint32_t                mNumDrawSamples;    // How many samples to use during drawing, '1' if MSAA is disabled
    VkFormat                mColorFormat;       // The format used for color attachments during drawing
    VkFormat                mResolveFormat;     // If doing MSAA this is the format to use for the MSAA resolve target
    vgl::RenderPass         mRenderPass;        // The Vulkan renderpass for this render path
    VMsaaResolver           mMsaaResolver;      // Only initialized if doing MSAA: helper to help resolve the multi-sampled framebuffer

    // Framebuffer color attachments and framebuffers - one per ringbuffer slot.
    // Note that the framebuffer might contain an additional MSAA resolve attachment (owned by the MSAA resolver) if that feature is active.
    vgl::RenderTexture  mColorAttachments[vgl::Defines::RINGBUFFER_SIZE];
    vgl::Framebuffer    mFramebuffers[vgl::Defines::RINGBUFFER_SIZE];

    // Whether each of the framebuffers have been involved in a frame yet
    bool mbRenderedToFramebuffer[vgl::Defines::RINGBUFFER_SIZE];
};

#endif  // #if PSYDOOM_VULKAN_RENDERER
