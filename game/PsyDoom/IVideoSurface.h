#pragma once

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds an image (specific to a video backend) that can be populated manually and blitted directly to the screen.
// Useful for displaying frames during video playback and for simple intro logo display.
// The image is uploaded in the 16-bit format used by PlayStation Doom (and most PSX games).
//------------------------------------------------------------------------------------------------------------------------------------------
class IVideoSurface {
public:
    virtual ~IVideoSurface() noexcept {}
    virtual uint32_t getWidth() const noexcept = 0;     // Gets the pixel width of the surface
    virtual uint32_t getHeight() const noexcept = 0;    // Gets the pixel height of the surface

    // Populates the video surface with the given XBGR8888 32-bit pixels.
    // The pixel array is expected to have the same dimensions as the surface.
    virtual void setPixels(const uint32_t* const pSrcPixels) noexcept = 0;
};

END_NAMESPACE(Video)
