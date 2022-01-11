#pragma once

#include "Macros.h"

#include <cstdint>
#include <memory>

struct SDL_Window;

BEGIN_NAMESPACE(Video)

class IVideoSurface;

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

    // Must be called prior to calling 'displayExternalSurface' for a succession of images.
    // Performs any setup for that needs to be done for that process.
    virtual void beginExternalSurfaceDisplay() noexcept = 0;

    // Must be called after calling 'displayExternalSurface' for a succession of images.
    // Performs any cleanup for that needs to be done for that process.
    virtual void endExternalSurfaceDisplay() noexcept = 0;

    // Displays the specified externally populated surface to the screen, which must be a surface for this backend.
    // This display method is slow, but sufficient for displaying video frames and intro logos etc.
    // The surface is displayed at the specified location and with the specified size and optionally with bi-linear filtering.
    // Areas of the screen not covered by the surface are displayed black.
    //
    // Note: 'beginExternalSurfaceDisplay()' should be called prior to calling this function for a series of display frames.
    // Also 'endExternalSurfaceDisplay()' should be called once you are done invoking this for a series of frames.
    virtual void displayExternalSurface(
        IVideoSurface& surface,
        const int32_t displayX,
        const int32_t displayY,
        const uint32_t displayW,
        const uint32_t displayH,
        const bool bUseFiltering
    ) noexcept = 0;

    // Get the size of the destination screen/window in pixels
    virtual void getScreenSizeInPixels(uint32_t& width, uint32_t& height) noexcept = 0;

    // Create a surface for the video backend that can be manually populated
    [[nodiscard]] virtual std::unique_ptr<IVideoSurface> createSurface(const uint32_t width, const uint32_t height) noexcept = 0;
};

END_NAMESPACE(Video)
