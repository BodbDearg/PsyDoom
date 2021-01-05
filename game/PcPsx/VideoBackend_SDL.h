#pragma once

#include "IVideoBackend.h"

struct SDL_Renderer;
struct SDL_Texture;

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// A video backend that uses an SDL renderer to display the PlayStation 1 framebuffer to the screen.
// Uses whatever platform specific graphics API that SDL uses for it's renderer.
// Supports only the classic renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
class VideoBackend_SDL : public IVideoBackend {
public:
    VideoBackend_SDL() noexcept;
    virtual ~VideoBackend_SDL() noexcept;

    virtual uint32_t getSdlWindowCreateFlags() noexcept override;
    virtual void initRenderers(SDL_Window* const pSdlWindow) noexcept override;
    virtual void destroyRenderers() noexcept override;
    virtual void displayFramebuffer() noexcept override;

private:
    void lockFramebufferTexture() noexcept;
    void unlockFramebufferTexture() noexcept;
    void copyPsxToSdlFramebufferTexture() noexcept;
    void presentSdlFramebufferTexture() noexcept;

    SDL_Window*     mpSdlWindow;            // The SDL window used
    SDL_Renderer*   mpRenderer;             // The SDL renderer used for blitting to the display
    SDL_Texture*    mpFramebufferTexture;   // A texture we populate for blitting to the display
    uint32_t*       mpFramebufferPixels;    // The pixels for framebuffer texture when locked for writing
};

END_NAMESPACE(Video)
