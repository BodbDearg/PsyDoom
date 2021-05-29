#pragma once

#include "Doom/doomdef.h"

struct line_t;

void P_Telefrag(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept;
bool EV_Teleport(line_t& line, mobj_t& mobj) noexcept;
