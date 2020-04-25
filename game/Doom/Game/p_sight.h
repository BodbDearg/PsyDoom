#pragma once

#include "Doom/doomdef.h"

struct line_t;
struct mobj_t;
struct subsector_t;

void P_CheckSights() noexcept;
bool P_CheckSight(mobj_t& mobj1, mobj_t& mobj2) noexcept;
bool PS_CrossBSPNode(const int32_t nodeNum) noexcept;
