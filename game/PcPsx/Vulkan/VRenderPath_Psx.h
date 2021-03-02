#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Defines.h"
#include "IVRenderPath.h"
#include "MutableTexture.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// A Vulkan renderer path which takes the output from the emulated PSX GPU and blits it to the current swapchain image for display.
// Allows the classic PlayStation renderer to be passed through the Vulkan renderer and output that way.
// This architecture is key to allowing rapid toggling between the old PSX renderer and the new Vulkan renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
class VRenderPath_Psx : public IVRendererPath {
public:
    VRenderPath_Psx() noexcept;
    ~VRenderPath_Psx() noexcept;

    void init(vgl::LogicalDevice& device, const VkFormat psxFramebufferFormat) noexcept;
    void destroy() noexcept;
    virtual bool ensureValidFramebuffers(const uint32_t fbWidth, const uint32_t fbHeight) noexcept override;
    virtual void beginFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept override;
    virtual void endFrame(vgl::Swapchain& swapchain, vgl::CmdBufferRecorder& cmdRec) noexcept override;

    inline bool isValid() const noexcept { return mbIsValid; }

private:
    void copyPsxFramebufferToFbTexture_A1R5G5B5(const uint16_t* const pSrcPixels, uint16_t* pDstPixels) noexcept;
    void copyPsxFramebufferToFbTexture_B8G8R8A8(const uint16_t* const pSrcPixels, uint32_t* pDstPixels) noexcept;

    bool                    mbIsValid;      // True if the render path has been initialized
    vgl::LogicalDevice*     mpDevice;       // The vulkan device used

    // PSX renderer framebuffers, as copied from the PSX GPU - these are blitted onto the current swapchain image.
    // One for each ringbuffer slot, so we can update while a previous frame's image is still blitting to the screen.
    vgl::MutableTexture mPsxFramebufferTextures[vgl::Defines::RINGBUFFER_SIZE];
};

#endif
