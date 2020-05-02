#pragma once

#include "Doom/doomdef.h"

struct line_t;

extern const VmPtr<VmPtr<mobj_t>>       gpShooter;
extern const VmPtr<fixed_t>             gAttackRange;
extern const VmPtr<angle_t>             gAttackAngle;
extern const VmPtr<fixed_t>             gAimTopSlope;
extern const VmPtr<fixed_t>             gAimBottomSlope;

void P_CheckPosition() noexcept;
void P_TryMove() noexcept;
void P_InterceptVector() noexcept;
bool PIT_UseLines(line_t& line) noexcept;
void P_UseLines() noexcept;
bool PIT_RadiusAttack(mobj_t& mobj) noexcept;
void P_RadiusAttack(mobj_t& bombSpot, mobj_t* const pSource, const int32_t damage) noexcept;
fixed_t P_AimLineAttack(mobj_t& shooter, const angle_t angle, const fixed_t maxDist) noexcept;
void P_LineAttack() noexcept;
