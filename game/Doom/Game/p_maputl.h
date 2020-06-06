#pragma once

#include "Doom/doomdef.h"

struct divline_t;
struct line_t;
struct mobj_t;

extern fixed_t gOpenBottom;
extern fixed_t gOpenTop;
extern fixed_t gOpenRange;
extern fixed_t gLowFloor;

fixed_t P_AproxDistance(const fixed_t dx, const fixed_t dy) noexcept;
int32_t P_PointOnLineSide(const fixed_t x, const fixed_t y, const line_t& line) noexcept;
int32_t P_PointOnDivlineSide(const fixed_t x, const fixed_t y, const divline_t& divline) noexcept;
void P_MakeDivline(const line_t& line, divline_t& divline) noexcept;
void P_LineOpening(const line_t& line) noexcept;
void P_UnsetThingPosition(mobj_t& thing) noexcept;
void P_SetThingPosition(mobj_t& thing) noexcept;
bool P_BlockLinesIterator(const int32_t x, const int32_t y, bool (*pFunc)(line_t&)) noexcept;
bool P_BlockThingsIterator(const int32_t x, const int32_t y, bool (*pFunc)(mobj_t&)) noexcept;
