#pragma once

#include <cstdint>

// How many bits to chop off for viewing angles to get the X offset to apply to the sky
static constexpr uint32_t ANGLETOSKYSHIFT = 22;

extern uint16_t gPaletteClutId_CurMapSky;

void R_DrawSky() noexcept;
