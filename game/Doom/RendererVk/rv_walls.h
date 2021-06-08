#pragma once

#if PSYDOOM_VULKAN_RENDERER

struct seg_t;
struct subsector_t;

#include <cstdint>

void RV_InitNextDrawSkyWalls() noexcept;
void RV_DrawSubsecOpaqueWalls(subsector_t& subsec) noexcept;
void RV_DrawSubsecBlendedWalls(subsector_t& subsec) noexcept;
void RV_DrawSubsecSkyWalls(const int32_t fromDrawSubsecIdx) noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER
