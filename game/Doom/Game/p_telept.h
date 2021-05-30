#pragma once

#include "Doom/doomdef.h"

struct line_t;

// PsyDoom: allow self-telefragging to be disabled (required for the 'Icon Of Sin' spawner boxes)
#if PSYDOOM_MODS
    void P_Telefrag(mobj_t& mobj, const fixed_t x, const fixed_t y, const bool bCanSelfTelefrag) noexcept;
#else
    void P_Telefrag(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept;
#endif

bool EV_Teleport(line_t& line, mobj_t& mobj) noexcept;
