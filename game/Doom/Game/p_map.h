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
void P_RadiusAttack() noexcept;
void P_AimLineAttack() noexcept;
void P_LineAttack() noexcept;
