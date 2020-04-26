#pragma once

#include "Doom/doomdef.h"

struct mobj_t;

fixed_t P_AproxDistance(const fixed_t dx, const fixed_t dy) noexcept;
void P_PointOnLineSide() noexcept;
void P_PointOnDivlineSide() noexcept;
void P_MakeDivline() noexcept;
void P_LineOpening() noexcept;

void P_UnsetThingPosition(mobj_t& thing) noexcept;
void _thunk_P_UnsetThingPosition() noexcept;

void P_SetThingPosition(mobj_t& thing) noexcept;
void _thunk_P_SetThingPosition() noexcept;

void P_BlockLinesIterator() noexcept;
void P_BlockThingsIterator() noexcept;
