#pragma once

#include "Doom/doomdef.h"

struct line_t;

void P_TryMove2() noexcept;
int32_t PM_PointOnDivlineSide(const fixed_t x, const fixed_t y, const line_t& line) noexcept;
void PM_UnsetThingPosition(mobj_t& thing) noexcept;
void PM_SetThingPosition(mobj_t& mobj) noexcept;
void PM_CheckPosition() noexcept;
void PM_BoxCrossLine() noexcept;
void PIT_CheckLine() noexcept;
bool PIT_CheckThing(mobj_t& mobj) noexcept;
void PM_BlockLinesIterator() noexcept;
bool PM_BlockThingsIterator(const int32_t x, const int32_t y) noexcept;
