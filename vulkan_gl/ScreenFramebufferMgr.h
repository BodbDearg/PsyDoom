#pragma once

#include "DeviceSurfaceCaps.h"
#include "RenderTexture.h"
#include "Swapchain.h"

BEGIN_NAMESPACE(vgl)

class BaseRenderPass;
class Framebuffer;
class LogicalDevice;

//------------------------------------------------------------------------------------------------------------------------------------------
// Manages the swap chain and framebuffers used for the screen. Keeps track of the current framebuffer index and so forth and also
// holds fences that are used to tell when particular frames are finished and so forth.
//------------------------------------------------------------------------------------------------------------------------------------------
class ScreenFramebufferMgr {
public:
    // Constant for an invalid/non-existant framebuffer index
    static constexpr const uint32_t INVALID_FRAMEBUFFER_IDX = UINT32_MAX;

    ScreenFramebufferMgr() noexcept;
    ~ScreenFramebufferMgr() noexcept;

    bool init(LogicalDevice& device) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline VkFormat getColorFormat() const noexcept { return mColorFormat; }
    inline VkFormat getDepthStencilFormat() const noexcept { return mDepthStencilFormat; }
    inline LogicalDevice* getDevice() const noexcept { return mpDevice; }
    inline const DeviceSurfaceCaps& getDeviceSurfaceCaps() const noexcept { return mDeviceSurfaceCaps; }
    inline const Swapchain& getSwapchain() const noexcept { return mSwapchain; }
    inline const RenderTexture& getDepthStencilBuffer() const noexcept { return mDepthStencilBuffer; }
    inline const std::vector<Framebuffer>& getDrawFramebuffers() const noexcept { return mDrawFramebuffers; }
    inline const std::vector<Framebuffer>& getPresentFramebuffers() const noexcept { return mPresentFramebuffers; }

    void invalidateFramebuffers() noexcept;
    bool framebuffersNeedRecreate() noexcept;
    bool createFramebuffers(BaseRenderPass& mainDrawRenderPass, BaseRenderPass& presentRenderPass) noexcept;
    bool recreateFramebuffers(BaseRenderPass& mainDrawRenderPass, BaseRenderPass& presentRenderPass) noexcept;
    void destroyFramebuffers() noexcept;
    bool hasZeroSizedWindowSurface() noexcept;
    bool hasValidFramebuffers() const noexcept;
    const Framebuffer& getCurrentDrawFramebuffer() const noexcept;
    const Framebuffer& getCurrentPresentFramebuffer() const noexcept;
    uint32_t getFramebuffersWidth() const noexcept;
    uint32_t getFramebuffersHeight() const noexcept;
    bool acquireFramebuffer(Semaphore& framebufferReadySemaphore) noexcept;
    bool presentCurrentFramebuffer(Semaphore& renderFinishedSemaphore) noexcept;

private:
    // Copy and move are disallowed
    ScreenFramebufferMgr(const ScreenFramebufferMgr& other) = delete;
    ScreenFramebufferMgr(ScreenFramebufferMgr&& other) = delete;
    ScreenFramebufferMgr& operator = (const ScreenFramebufferMgr& other) = delete;
    ScreenFramebufferMgr& operator = (ScreenFramebufferMgr&& other) = delete;

    bool chooseSurfaceAndDepthStencilFormats() noexcept;
    
    bool                        mbIsValid;                  // True if the manager was created & initialized successfully
    VkFormat                    mColorFormat;               // Format to use for the color attachment for the main surface
    VkFormat                    mDepthStencilFormat;        // Format to use for the depth stencil buffer
    uint32_t                    mCurrentFramebufferIdx;     // The index of current framebuffer acquired for rendering. 'INVALID_FRAMEBUFFER_IDX' if none is acquired
    LogicalDevice*              mpDevice;                   // The logical device used by the framebuffer manager
    DeviceSurfaceCaps           mDeviceSurfaceCaps;         // Capabilities of the physical device with respect to device surface
    Swapchain                   mSwapchain;                 // Swap chain for the screen framebuffers
    RenderTexture               mDepthStencilBuffer;        // Depth stencil buffer for the device. Note: only currently need 1!
    std::vector<Framebuffer>    mDrawFramebuffers;          // The framebuffers used for drawing
    std::vector<Framebuffer>    mPresentFramebuffers;       // The framebuffers used for presentation: same as drawing, but without depth buffer
};

END_NAMESPACE(vgl)
