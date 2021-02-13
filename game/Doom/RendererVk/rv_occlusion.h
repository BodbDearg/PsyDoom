#pragma once

#if PSYDOOM_VULKAN_RENDERER

void RV_ClearOcclussion() noexcept;
void RV_OccludeRange(float xMin, float xMax) noexcept;
bool RV_IsRangeVisible(float xMin, float xMax) noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER
