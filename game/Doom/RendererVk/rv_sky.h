#pragma once

#if PSYDOOM_VULKAN_RENDERER

void RV_CacheSkyTex() noexcept;

void RV_AddSkyQuad(
    const float x1,
    const float y1,
    const float z1,
    const float x2,
    const float y2,
    const float z2,
    const float x3,
    const float y3,
    const float z3,
    const float x4,
    const float y4,
    const float z4
) noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER
