#pragma once

#include "Doom/doomdef.h"

void P_SlideMove() noexcept;
void P_CompletableFrac() noexcept;
void SL_PointOnSide() noexcept;
void SL_CrossFrac() noexcept;
void CheckLineEnds() noexcept;
void ClipToLine() noexcept;
void SL_CheckLine() noexcept;
void SL_CheckSpecialLines(const fixed_t moveX1, const fixed_t moveY1, const fixed_t moveX2, const fixed_t moveY2) noexcept;
