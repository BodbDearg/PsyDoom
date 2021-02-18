#pragma once

#if PSYDOOM_VULKAN_RENDERER

namespace vgl {
    class CmdBufferRecorder;
    class Swapchain;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents an interface that a specific render path of the Vulkan renderer must implement.
// Depending on the render path, different techniques and render passes may be used to generate the final onscreen images.
//------------------------------------------------------------------------------------------------------------------------------------------
class IVRendererPath {
public:
    // Ensures there are valid framebuffers for this render path and creates or re-creates them if neccessary.
    // The current global rendering resolution is passed in as information but this may be ignored if required.
    // Returns 'false' if creating the framebuffers fails, in which case the frame should not be started.
    virtual bool ensureValidFramebuffers(const uint32_t fbWidth, const uint32_t fbHeight) noexcept = 0;

    // Called to start the frame for this render path.
    // Should begin any render passes required, prepare for drawing commands etc.
    // The current (valid) swapchain and the primary command buffer are passed in.
    virtual void beginFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept = 0;

    // Called to end the frame for this render path.
    // Should finish up any render passes, command buffer recording etc. and whatever other work that needs to be done.
    // The current (valid) swapchain and the primary command buffer are passed in.
    virtual void endFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept = 0;
};

#endif  // #if PSYDOOM_VULKAN_RENDERER
