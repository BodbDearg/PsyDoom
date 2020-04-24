#pragma once

#include "Doom/doomdef.h"

struct line_t;
struct subsector_t;

void P_CheckSights() noexcept;
void P_CheckSight() noexcept;
bool PS_CrossBSPNode(const int32_t nodeNum) noexcept;
