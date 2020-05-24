#pragma once

#include "Doom/doomdef.h"

struct line_t;

extern const VmPtr<VmPtr<mobj_t>>   gpSlideThing;
extern const VmPtr<fixed_t>         gSlideX;
extern const VmPtr<fixed_t>         gSlideY;
extern const VmPtr<VmPtr<line_t>>   gpSpecialLine;

void P_SlideMove() noexcept;
