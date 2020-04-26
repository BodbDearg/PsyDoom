#pragma once

#include "Doom/doomdef.h"

extern const VmPtr<VmPtr<mobj_t>>       gpShooter;
extern const VmPtr<fixed_t>             gAttackRange;
extern const VmPtr<fixed_t>             gAimTopSlope;
extern const VmPtr<fixed_t>             gAimBottomSlope;

void P_CheckPosition() noexcept;
void P_TryMove() noexcept;
void P_InterceptVector() noexcept;
void PIT_UseLines() noexcept;
void P_UseLines() noexcept;
void PIT_RadiusAttack() noexcept;
void P_RadiusAttack() noexcept;
void P_AimLineAttack() noexcept;
void P_LineAttack() noexcept;
