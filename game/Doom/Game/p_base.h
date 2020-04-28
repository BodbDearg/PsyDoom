#pragma once

#include <cstdint>

struct mobj_t;

void P_RunMobjBase() noexcept;
void P_XYMovement() noexcept;
void P_FloatChange() noexcept;
void P_ZMovement() noexcept;
void P_MobjThinker() noexcept;
void PB_TryMove() noexcept;
void PB_UnsetThingPosition(mobj_t& thing) noexcept;
void PB_SetThingPosition(mobj_t& mobj) noexcept;
void PB_CheckPosition() noexcept;
void PB_BoxCrossLine() noexcept;
void PB_CheckLine() noexcept;
void PB_BlockLinesIterator() noexcept;
bool PB_BlockThingsIterator(const int32_t x, const int32_t y) noexcept;
