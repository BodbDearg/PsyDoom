#pragma once

#include "Macros.h"

#include <cstdint>
#include <memory>

BEGIN_NAMESPACE(LogoPlayer)

//------------------------------------------------------------------------------------------------------------------------------------------
// Container for a logo to be displayed.
// The logo is displayed centered in an area with the same logical resolution as the original PS1 game, 256x240.
//------------------------------------------------------------------------------------------------------------------------------------------
struct Logo {
    std::unique_ptr<uint32_t[]>     pPixels;        // Pixels for the logo in XBGR8888 format
    uint16_t                        width;          // Width of the logo in pixels
    uint16_t                        height;         // Height of the logo in pixels
    float                           fadeInTime;     // How long to fade in the logo for (seconds)
    float                           holdTime;       // How long to hold the logo at full brightness (seconds)
    float                           fadeOutTime;    // How long to fade out the logo for (seconds)
};

bool play(const Logo& logo) noexcept;

END_NAMESPACE(LogoPlayer)
