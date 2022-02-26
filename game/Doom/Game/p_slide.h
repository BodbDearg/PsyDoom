#pragma once

#include "Doom/doomdef.h"

struct line_t;

extern mobj_t*  gpSlideThing;
extern fixed_t  gSlideX;
extern fixed_t  gSlideY;

// PsyDoom: more than one special line can now be crossed per frame (depending on game settings)
#if PSYDOOM_MODS
    void P_GetSpecialLines(line_t**& ppSpecialLinesOut, uint32_t& numSpecialLinesOut) noexcept;
#else
    extern line_t* gpSpecialLine;
#endif

void P_SlideMove() noexcept;
