#include "VideoSurface_SDL.h"

#include "Asserts.h"

#include <cstring>
#include <SDL.h>

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to create the surface with the specified dimensions.
// If creation fails then the failure is silent and usage of the surface results in NO-OPs.
//------------------------------------------------------------------------------------------------------------------------------------------
VideoSurface_SDL::VideoSurface_SDL(SDL_Renderer& renderer, const uint32_t width, const uint32_t height) noexcept
    : mpTexture(nullptr)
    , mWidth(width)
    , mHeight(height)
{
    if ((width > 0) && (height > 0)) {
        mpTexture = SDL_CreateTexture(&renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, width, height);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleans up surface resources
//------------------------------------------------------------------------------------------------------------------------------------------
VideoSurface_SDL::~VideoSurface_SDL() noexcept {
    if (mpTexture) {
        SDL_DestroyTexture(mpTexture);
        mpTexture = nullptr;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the dimensions of the surface
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t VideoSurface_SDL::getWidth() const noexcept { return mWidth; }
uint32_t VideoSurface_SDL::getHeight() const noexcept { return mHeight; }

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the pixels for the surface.
// Ignores the call if the surface was not successfully initialized.
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoSurface_SDL::setPixels(const uint32_t* const pSrcPixels) noexcept {
    ASSERT(pSrcPixels);

    // Abort if the texture wasn't successfully initialized
    if (!mpTexture)
        return;

    // Abort if we can't lock the texture for some reason
    void* pCurDstRow = nullptr;
    int pitch = 0;

    if (SDL_LockTexture(mpTexture, nullptr, &pCurDstRow, &pitch) != 0) {
        ASSERT_FAIL("VideoSurface_SDL::setPixels: locking the texture failed!");
        return;
    }

    // Copy the texture data row by row
    const uint32_t* pCurSrcPixels = pSrcPixels;

    for (uint32_t y = 0; y < mHeight; ++y) {
        std::memcpy(pCurDstRow, pCurSrcPixels, sizeof(uint32_t) * mWidth);
        pCurSrcPixels += mWidth;
        pCurDstRow = (char*) pCurDstRow + pitch;
    }

    // Done copying, unlock the texture
    SDL_UnlockTexture(mpTexture);
}

END_NAMESPACE(Video)
