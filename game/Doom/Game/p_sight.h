#pragma once

#include <cstdint>

struct subsector_t;

void P_CheckSights() noexcept;
void P_CheckSight() noexcept;
void PS_SightCrossLine() noexcept;
bool PS_CrossSubsector(const subsector_t& subsec) noexcept;
bool PS_CrossBSPNode(const int32_t nodeNum) noexcept;
