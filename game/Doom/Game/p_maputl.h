#pragma once

#include "Doom/doomdef.h"

struct divline_t;
struct line_t;
struct mobj_t;

extern const VmPtr<fixed_t>     gOpenBottom;
extern const VmPtr<fixed_t>     gOpenTop;
extern const VmPtr<fixed_t>     gOpenRange;
extern const VmPtr<fixed_t>     gLowFloor;

fixed_t P_AproxDistance(const fixed_t dx, const fixed_t dy) noexcept;
int32_t P_PointOnLineSide(const fixed_t x, const fixed_t y, const line_t& line) noexcept;
int32_t P_PointOnDivlineSide(const fixed_t x, const fixed_t y, const divline_t& divline) noexcept;
void P_MakeDivline(const line_t& line, divline_t& divline) noexcept;
void P_LineOpening(const line_t& line) noexcept;
void P_UnsetThingPosition(mobj_t& thing) noexcept;
void P_SetThingPosition(mobj_t& thing) noexcept;
void P_BlockLinesIterator() noexcept;
void P_BlockThingsIterator() noexcept;
