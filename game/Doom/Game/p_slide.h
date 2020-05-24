#pragma once

#include "Doom/doomdef.h"

struct line_t;

extern const VmPtr<VmPtr<mobj_t>>   gpSlideThing;

void P_SlideMove() noexcept;
