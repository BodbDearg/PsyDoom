#pragma once

#include "Doom/doomdef.h"

struct divline_t;

void P_Shoot2() noexcept;
void PA_DoIntercept() noexcept;
void PA_ShootLine() noexcept;
void PA_ShootThing() noexcept;
void PA_SightCrossLine() noexcept;
void PA_CrossSubsector() noexcept;
void PointOnVectorSide() noexcept;
void PA_CrossBSPNode() noexcept;
int32_t PA_DivlineSide(const fixed_t x, const fixed_t y, const divline_t& line) noexcept;
