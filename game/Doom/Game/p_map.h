#pragma once

#include "Doom/doomdef.h"

struct line_t;

extern mobj_t*      gpShooter;
extern fixed_t      gAttackRange;
extern angle_t      gAttackAngle;
extern fixed_t      gAimTopSlope;
extern fixed_t      gAimBottomSlope;
extern mobj_t*      gpLineTarget;
extern mobj_t*      gpTryMoveThing;
extern fixed_t      gTryMoveX;
extern fixed_t      gTryMoveY;
extern bool         gbCheckPosOnly;

bool P_CheckPosition(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept;
bool P_TryMove(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept;
void P_UseLines(player_t& player) noexcept;
void P_RadiusAttack(mobj_t& bombSpot, mobj_t* const pSource, const int32_t damage) noexcept;
fixed_t P_AimLineAttack(mobj_t& shooter, const angle_t angle, const fixed_t maxDist) noexcept;
void P_LineAttack(mobj_t& shooter, const angle_t angle, const fixed_t maxDist, const fixed_t zSlope, const int32_t damage) noexcept;
