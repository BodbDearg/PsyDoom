#pragma once

struct mobj_t;

void P_TryMove2() noexcept;
void PM_PointOnDivlineSide() noexcept;
void PM_UnsetThingPosition(mobj_t& thing) noexcept;
void PM_SetThingPosition(mobj_t& mobj) noexcept;
void PM_CheckPosition() noexcept;
void PM_BoxCrossLine() noexcept;
void PIT_CheckLine() noexcept;
void PIT_CheckThing() noexcept;
void PM_BlockLinesIterator() noexcept;
void PM_BlockThingsIterator() noexcept;
