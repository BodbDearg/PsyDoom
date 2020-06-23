#pragma once

#include "Doom/doomdef.h"

struct player_t;

#if PC_PSX_DOOM_MODS
    extern angle_t gPlayerUncommittedMouseTurning;
    extern angle_t gPlayerUncommittedAxisTurning;
    extern angle_t gPlayerNextTickViewAngle;
#endif

void P_PlayerThink(player_t& player) noexcept;

#if PC_PSX_DOOM_MODS
    void P_PlayerInitTurning() noexcept;
    void P_PlayerDoTurning() noexcept;
#endif
