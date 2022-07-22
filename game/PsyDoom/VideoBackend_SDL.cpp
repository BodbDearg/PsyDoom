#include "VideoBackend_SDL.h"

#include "Asserts.h"
#include "Config/Config.h"
#include "Gpu.h"
#include "PsxVm.h"
#include "Video.h"
#include "VideoSurface_SDL.h"

#include <algorithm>
#include <cmath>
#include <SDL.h>

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the backend with the SDL renderer uninitialized
//------------------------------------------------------------------------------------------------------------------------------------------
VideoBackend_SDL::VideoBackend_SDL() noexcept 
    : mpSdlWindow(nullptr)
    , mpRenderer(nullptr)
    , mpFramebufferTexture(nullptr)
    , mpFramebufferPixels(nullptr)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ensures everything is cleaned up
//------------------------------------------------------------------------------------------------------------------------------------------
VideoBackend_SDL::~VideoBackend_SDL() noexcept {
    destroyRenderers();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the window create flags required for an SDL video backend window
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t VideoBackend_SDL::getSdlWindowCreateFlags() noexcept {
    // Use OpenGL where it is supported since that is the main implementation for SDL renderer.
    // On MacOS it's better to use Metal however since OpenGL is deprecated.
    #if __APPLE__
        return SDL_WINDOW_METAL;
    #else
        return SDL_WINDOW_OPENGL;
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the SDL renderer used by this backend
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_SDL::initRenderers(SDL_Window* const pSdlWindow) noexcept {
    ASSERT(pSdlWindow);

    // Must not already be initialized
    ASSERT(!mpRenderer);
    ASSERT(!mpFramebufferTexture);
    ASSERT(!mpFramebufferPixels);

    // Create the renderer and framebuffer texture
    mpSdlWindow = pSdlWindow;
    const Uint32 vsyncFlag = (Config::gbEnableVSync) ? SDL_RENDERER_PRESENTVSYNC : 0;
    mpRenderer = SDL_CreateRenderer(pSdlWindow, -1, SDL_RENDERER_ACCELERATED | vsyncFlag);

    if (!mpRenderer) {
        FatalErrors::raise("Failed to create renderer!");
    }

    mpFramebufferTexture = SDL_CreateTexture(
        mpRenderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        ORIG_DRAW_RES_X,
        ORIG_DRAW_RES_Y
    );

    if (!mpFramebufferTexture) {
        FatalErrors::raise("Failed to create a framebuffer texture!");
    }

    // Clear the renderer to black
    SDL_SetRenderDrawColor(mpRenderer, 0, 0, 0, 0);
    SDL_RenderClear(mpRenderer);

    // Immediately lock the framebuffer texture in preparation for the next update
    lockFramebufferTexture();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleans up and destroys the SDL renderer used by this video backend
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_SDL::destroyRenderers() noexcept {
    if (mpFramebufferPixels) {
        unlockFramebufferTexture();
    }

    if (mpFramebufferTexture) {
        SDL_DestroyTexture(mpFramebufferTexture);
        mpFramebufferTexture = nullptr;
    }

    if (mpRenderer) {
        SDL_DestroyRenderer(mpRenderer);
        mpRenderer = nullptr;
    }

    mpSdlWindow = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copies the output from the classic renderer (PSX framebuffer) to an SDL texture and then blits that to the screen.
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_SDL::displayFramebuffer() noexcept {
    copyPsxToSdlFramebufferTexture();
    presentSdlFramebufferTexture();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// These functions are no-ops for the SDL backend.
// Don't need to do anything special to display an external surface at any given time.
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_SDL::beginExternalSurfaceDisplay() noexcept {}
void VideoBackend_SDL::endExternalSurfaceDisplay() noexcept {}

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays the specified surface to the screen
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_SDL::displayExternalSurface(
    IVideoSurface& surface,
    const int32_t displayX,
    const int32_t displayY,
    const uint32_t displayW,
    const uint32_t displayH,
    const bool bUseFiltering
) noexcept {
    // Must be an SDL video surface!
    ASSERT(dynamic_cast<VideoSurface_SDL*>(&surface));
    VideoSurface_SDL& sdlSurface = static_cast<VideoSurface_SDL&>(surface);

    // Decide source and destination rectangles
    SDL_Rect srcRect = {};
    srcRect.w = (int) sdlSurface.getWidth();
    srcRect.h = (int) sdlSurface.getHeight();

    SDL_Rect dstRect = {};
    dstRect.x = displayX;
    dstRect.y = displayY;
    dstRect.w = (int) displayW;
    dstRect.h = (int) displayH;

    // Clear the screen and blit the surface to the display using the specified scaling.
    // If there is no valid texture then just clear the screen.
    SDL_RenderClear(mpRenderer);
    SDL_Texture* const pSdlTexture = sdlSurface.getTexture();

    if (pSdlTexture) {
        SDL_SetTextureScaleMode(pSdlTexture, (bUseFiltering) ? SDL_ScaleModeLinear : SDL_ScaleModeNearest);
        SDL_RenderCopy(mpRenderer, pSdlTexture, &srcRect, &dstRect);
    }

    // Present the rendered frame
    SDL_RenderPresent(mpRenderer);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gives the size of the swapchain/window in pixels
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_SDL::getScreenSizeInPixels(uint32_t& width, uint32_t& height) noexcept {
    int sdlWidth = 0;
    int sdlHeight = 0;

    if (mpRenderer) {
        if (SDL_GetRendererOutputSize(mpRenderer, &sdlWidth, &sdlHeight) != 0) {
            // Just to be safe, clear these again on an error...
            sdlWidth = 0;
            sdlHeight = 0;
        }
    }

    width = sdlWidth;
    height = sdlHeight;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates and returns an SDL format video surface.
// Fails if renderers have not been initialized.
//------------------------------------------------------------------------------------------------------------------------------------------
std::unique_ptr<IVideoSurface> VideoBackend_SDL::createSurface(const uint32_t width, const uint32_t height) noexcept {
    return (mpRenderer) ? std::make_unique<VideoSurface_SDL>(*mpRenderer, width, height) : std::unique_ptr<IVideoSurface>();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Lock the SDL texture we upload the PSX framebuffer to for writing
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_SDL::lockFramebufferTexture() noexcept {
    ASSERT(mpFramebufferTexture);
    ASSERT(!mpFramebufferPixels);

    int pitch = 0;

    if (SDL_LockTexture(mpFramebufferTexture, nullptr, reinterpret_cast<void**>(&mpFramebufferPixels), &pitch) != 0) {
        FatalErrors::raise("Failed to lock the framebuffer texture for writing!");
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unlock the SDL texture containing the PSX framebuffer after we finish writing to it
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_SDL::unlockFramebufferTexture() noexcept {
    ASSERT(mpFramebufferPixels);
    ASSERT(mpFramebufferTexture);

    SDL_UnlockTexture(mpFramebufferTexture);
    mpFramebufferPixels = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copies the rendered PSX GPU framebuffer the locked SDL texture, in preparation for blitting to the screen
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_SDL::copyPsxToSdlFramebufferTexture() noexcept {
    // Sanity checks
    ASSERT(mpFramebufferPixels);

    // Copy the framebuffer
    Gpu::Core& gpu = PsxVm::gGpu;
    const Gpu::Color16* const vramPixels = reinterpret_cast<const Gpu::Color16*>(gpu.pRam);
    uint32_t* pDstPixel = mpFramebufferPixels;

    for (uint32_t y = 0; y < ORIG_DRAW_RES_Y; ++y) {
        const Gpu::Color16* const rowPixels = vramPixels + ((intptr_t) y + gpu.displayAreaY) * gpu.ramPixelW;
        const uint32_t xStart = (uint32_t) gpu.displayAreaX;
        const uint32_t xEnd = xStart + ORIG_DRAW_RES_X;
        ASSERT(xEnd <= gpu.ramPixelW);

        // Note: don't bother doing multiple pixels at a time - compiler is smart and already optimizes this to use SIMD
        for (uint32_t x = xStart; x < xEnd; ++x, ++pDstPixel) {
            const Gpu::Color16 srcPixel = rowPixels[x];
            const uint32_t r = (uint32_t) srcPixel.getR() << 3;
            const uint32_t g = (uint32_t) srcPixel.getG() << 3;
            const uint32_t b = (uint32_t) srcPixel.getB() << 3;

            *pDstPixel = (0xFF000000 | (b << 16) | (g << 8 ) | (r << 0));
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Presents the SDL texture which contains a rendered frame from the PSX GPU to the screen
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_SDL::presentSdlFramebufferTexture() noexcept {
    // Sanity checks
    ASSERT(mpSdlWindow);
    ASSERT(mpRenderer);
    ASSERT(mpFramebufferTexture);

    // Get the size of the window in pixels and don't bother outputting to it if zero sized
    int32_t windowW = {};
    int32_t windowH = {};
    SDL_GetRendererOutputSize(mpRenderer, &windowW, &windowH);

    if ((windowW <= 0) || (windowH <= 0))
        return;

    // Get the window area to output the PSX framebuffer to  and don't bother outputting if zero sized
    float outputRectX = {};
    float outputRectY = {};
    float outputRectW = {};
    float outputRectH = {};

    Video::getClassicFramebufferWindowRect(
        (float) windowW,
        (float) windowH,
        outputRectX,
        outputRectY,
        outputRectW,
        outputRectH
    );

    if ((outputRectW <= 0.0f) || (outputRectH <= 0.0f))
        return;

    // These are the source and destination regions to blit
    ASSERT((Video::gTopOverscan >= 0) && (Video::gTopOverscan < Video::ORIG_DRAW_RES_Y / 2));
    ASSERT((Video::gBotOverscan >= 0) && (Video::gBotOverscan < Video::ORIG_DRAW_RES_Y / 2));

    SDL_Rect srcRect = {};
    srcRect.y = Video::gTopOverscan;
    srcRect.w = Video::ORIG_DRAW_RES_X;
    srcRect.h = Video::ORIG_DRAW_RES_Y - Video::gTopOverscan - Video::gBotOverscan;

    SDL_Rect dstRect = {};
    dstRect.x = (int) outputRectX;
    dstRect.y = (int) outputRectY;
    dstRect.w = (int) std::ceil(outputRectW);
    dstRect.h = (int) std::ceil(outputRectH);

    // Done writing to the locked framebuffer, update the texture with whatever writes we made
    unlockFramebufferTexture();

    // Need to clear the window if we are not filling the whole screen.
    // Some stuff like NVIDIA video recording notifications can leave marks in the unused regions otherwise...
    if ((dstRect.w != windowW) || (dstRect.h != windowH)) {
        SDL_RenderClear(mpRenderer);
    }

    // Blit the framebuffer to the display
    SDL_RenderCopy(mpRenderer, mpFramebufferTexture, &srcRect, &dstRect);

    // Present the rendered frame and re-lock the framebuffer texture
    SDL_RenderPresent(mpRenderer);
    lockFramebufferTexture();
}

END_NAMESPACE(Video)
