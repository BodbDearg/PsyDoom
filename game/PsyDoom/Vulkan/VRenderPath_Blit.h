#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Defines.h"
#include "IVRenderPath.h"

#include <vulkan/vulkan.h>

namespace vgl {
    class LogicalDevice;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// A Vulkan renderer path used for directly blitting to a swap chain image.
// Prepares the swap chain image for blitting at the beginning of a frame, and transitions it back for presentation on end.
// Used during video playback and logo display.
//------------------------------------------------------------------------------------------------------------------------------------------
class VRenderPath_Blit : public IVRendererPath {
public:
    VRenderPath_Blit() noexcept;
    ~VRenderPath_Blit() noexcept;

    void init(vgl::LogicalDevice& device) noexcept;
    void destroy() noexcept;
    virtual bool ensureValidFramebuffers(const uint32_t fbWidth, const uint32_t fbHeight) noexcept override;
    virtual void beginFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept override;
    virtual void endFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept override;

    inline bool isValid() const noexcept { return mbIsValid; }

private:
    bool                    mbIsValid;      // True if the render path has been initialized
    vgl::LogicalDevice*     mpDevice;       // The vulkan device used
};

#endif  // #if PSYDOOM_VULKAN_RENDERER
