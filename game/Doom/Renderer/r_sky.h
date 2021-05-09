#pragma once

#include <cstdint>

struct leafedge_t;
struct subsector_t;

// How many bits to chop off for viewing angles to get the X offset to apply to the sky
static constexpr uint32_t ANGLETOSKYSHIFT = 22;

extern uint16_t gPaletteClutId_CurMapSky;

void R_DrawSky() noexcept;

#if PSYDOOM_LIMIT_REMOVING
    void R_DrawSegSkyWalls(const subsector_t& subsec, const leafedge_t& edge) noexcept;
#endif
