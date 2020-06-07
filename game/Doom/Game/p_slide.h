#pragma once

#include "Doom/doomdef.h"

struct line_t;

extern mobj_t*  gpSlideThing;
extern fixed_t  gSlideX;
extern fixed_t  gSlideY;
extern line_t*  gpSpecialLine;

void P_SlideMove() noexcept;
