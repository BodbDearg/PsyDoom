#pragma once

#include "Doom/doomdef.h"

struct line_t;

extern const VmPtr<VmPtr<mobj_t>>       gpShooter;
extern const VmPtr<fixed_t>             gAttackRange;
extern const VmPtr<angle_t>             gAttackAngle;
extern const VmPtr<fixed_t>             gAimTopSlope;
extern const VmPtr<fixed_t>             gAimBottomSlope;
extern const VmPtr<VmPtr<mobj_t>>       gpLineTarget;
extern const VmPtr<VmPtr<mobj_t>>       gpTryMoveThing;
extern const VmPtr<fixed_t>             gTryMoveX;
extern const VmPtr<fixed_t>             gTryMoveY;
extern const VmPtr<bool32_t>            gbCheckPosOnly;

bool P_CheckPosition(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept;
bool P_TryMove(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept;
void P_UseLines(player_t& player) noexcept;
void P_RadiusAttack(mobj_t& bombSpot, mobj_t* const pSource, const int32_t damage) noexcept;
fixed_t P_AimLineAttack(mobj_t& shooter, const angle_t angle, const fixed_t maxDist) noexcept;
void P_LineAttack(mobj_t& shooter, const angle_t angle, const fixed_t maxDist, const fixed_t zSlope, const int32_t damage) noexcept;
