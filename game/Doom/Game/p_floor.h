#pragma once

#include "Doom/doomdef.h"

// Represents the result of moving a floor/ceiling
enum result_e : uint32_t {
    ok          = 0,    // Movement for the floor/ceiling was fully OK
    crushed     = 1,    // The floor/ceiling is crushing things and may not have moved because of this
    pastdest    = 2     // Plane has reached its destination or very close to it (sometimes stops just before if crushing things)
};

result_e T_MovePlane(
    sector_t& sector,
    const fixed_t speed,
    const fixed_t destHeight,
    const bool bCrush,
    const int32_t floorOrCeiling,
    const int32_t direction
) noexcept;

void T_MoveFloor() noexcept;
void EV_DoFloor() noexcept;
void EV_BuildStairs() noexcept;
