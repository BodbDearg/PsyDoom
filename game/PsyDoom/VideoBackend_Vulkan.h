#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "IVideoBackend.h"

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// A video backend that uses Vulkan (or Vulkan implemented via some other API) to display the current framebuffer to the screen.
// Supports both the classic PlayStation renderer and the new hardware accelerated Vulkan renderer added for PsyDoom.
//------------------------------------------------------------------------------------------------------------------------------------------
class VideoBackend_Vulkan : public IVideoBackend {
public:
    static bool isBackendSupported() noexcept;

    VideoBackend_Vulkan() noexcept;
    virtual ~VideoBackend_Vulkan() noexcept;

    virtual uint32_t getSdlWindowCreateFlags() noexcept override;
    virtual void initRenderers(SDL_Window* const pSdlWindow) noexcept override;
    virtual void destroyRenderers() noexcept override;
    virtual void displayFramebuffer() noexcept override;

private:
    SDL_Window*     mpSdlWindow;            // The SDL window used
};

END_NAMESPACE(Video)

#endif  // PSYDOOM_VULKAN_RENDERER
