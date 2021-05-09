#pragma once

#if PSYDOOM_VULKAN_RENDERER

void RV_CacheSkyTex() noexcept;
void RV_DrawBackgroundSky() noexcept;

void RV_AddInfiniteSkyWall(
    const float x1,
    const float z1,
    const float x2,
    const float z2,
    const float y,
    const bool bIsUpperSkyWall
) noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER
