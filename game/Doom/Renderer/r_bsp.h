#pragma once

#include "Doom/doomdef.h"

void R_BSP() noexcept;
void R_RenderBSPNode(const int32_t bspnum) noexcept;
bool R_CheckBBox(const fixed_t bspcoord[4]) noexcept;
void R_Subsector() noexcept;
void R_AddLine() noexcept;
