#pragma once

#include "Doom/doomdef.h"

struct player_t;

#if PSYDOOM_MODS
    extern angle_t gPlayerUncommittedTurning;
    extern angle_t gPlayerNextTickViewAngle;
#endif

void P_PlayerThink(player_t& player) noexcept;

#if PSYDOOM_MODS
    void P_PlayerInitTurning() noexcept;
    void P_PlayerDoTurning() noexcept;
    void P_UncommitTurningTickInputs() noexcept;
#endif
