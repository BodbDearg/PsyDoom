#pragma once

#include "Macros.h"

#include <cstdint>

struct SDL_Window;

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// Interface for a backend that supports presentation of the current framebuffer to the screen.
// Can support either just the classic renderer or additional new renderers, like the Vulkan renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
class IVideoBackend {
public:
    virtual ~IVideoBackend() noexcept {}

    // Get flags that should be added for SDL window creation specifically for this backend.
    // E.G: 'SDL_WINDOW_VULKAN' for a Vulkan based backend.
    virtual uint32_t getSdlWindowCreateFlags() noexcept = 0;

    // Initialize the renderers for this backend
    virtual void initRenderers(SDL_Window* const pSdlWindow) noexcept = 0;

    // Destroy the renderers for this backend
    virtual void destroyRenderers() noexcept = 0;

    // Displays the current framebuffer to the screen
    virtual void displayFramebuffer() noexcept = 0;
};

END_NAMESPACE(Video)
