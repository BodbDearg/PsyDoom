#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "IVideoBackend.h"
#include <functional>

namespace vgl {
    class VulkanInstance;
}

class IVRendererPath;

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// A video backend that uses Vulkan (or Vulkan implemented via some other API) to display the current framebuffer to the screen.
// Supports both the classic PlayStation renderer and the new hardware accelerated Vulkan renderer added for PsyDoom.
//------------------------------------------------------------------------------------------------------------------------------------------
class VideoBackend_Vulkan : public IVideoBackend {
public:
    static bool withTempVkInstance(const std::function<void (vgl::VulkanInstance& vkInstance)>& doLogic) noexcept;
    static bool isBackendSupported() noexcept;

    VideoBackend_Vulkan() noexcept;
    virtual ~VideoBackend_Vulkan() noexcept;

    virtual uint32_t getSdlWindowCreateFlags() noexcept override;
    virtual void initRenderers(SDL_Window* const pSdlWindow) noexcept override;
    virtual void destroyRenderers() noexcept override;
    virtual void displayFramebuffer() noexcept override;

    virtual void beginExternalSurfaceDisplay() noexcept override;
    virtual void endExternalSurfaceDisplay() noexcept override;

    virtual void displayExternalSurface(
        IVideoSurface& surface,
        const int32_t displayX,
        const int32_t displayY,
        const uint32_t displayW,
        const uint32_t displayH,
        const bool bUseFiltering
    ) noexcept override;

    virtual void getScreenSizeInPixels(uint32_t& width, uint32_t& height) noexcept override;
    [[nodiscard]] virtual std::unique_ptr<IVideoSurface> createSurface(const uint32_t width, const uint32_t height) noexcept override;

private:
    SDL_Window*         mpSdlWindow;                    // The SDL window used
    IVRendererPath*     mpDispExtSurfOldRenderPath;     // The old render path used prior to displaying external surfaces manually (restored on completion of that)
};

END_NAMESPACE(Video)

#endif  // PSYDOOM_VULKAN_RENDERER
