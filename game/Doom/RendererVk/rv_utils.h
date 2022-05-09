#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Doom/doomdef.h"

namespace Gpu {
    enum class BlendMode : uint8_t;
    enum class TexFmt : uint8_t;
}

struct seg_t;
struct texture_t;

// PI to 96 digits: define here because it's not available in standard C/C++ (sigh... it's 2021 and this is still a thing)
template <class T> static constexpr T RV_PI = T(3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117);

// More convenience scalings of PI
template <class T> static constexpr T RV_2PI = RV_PI<T> * T(2.0);
template <class T> static constexpr T RV_PI_2 = RV_PI<T> / T(2.0);
template <class T> static constexpr T RV_PI_4 = RV_PI<T> / T(4.0);

float RV_FixedToFloat(const fixed_t num) noexcept;
fixed_t RV_FloatToFixed(const float num) noexcept;
float RV_AngleToFloat(const angle_t angle) noexcept;
angle_t RV_FloatToAngle(const float angle) noexcept;
void RV_ClutIdToClutXy(const uint16_t clutId, uint16_t& clutX, uint16_t& clutY) noexcept;

void RV_TexPageIdToTexParams(
    const uint16_t texPageId,
    Gpu::TexFmt& texFmt,
    uint16_t& texPageX,
    uint16_t& texPageY,
    Gpu::BlendMode& blendMode
) noexcept;

void RV_GetTexWinXyWh(const texture_t& tex, uint16_t& texWinX, uint16_t& texWinY, uint16_t& texWinW, uint16_t& texWinH) noexcept;
void RV_UploadDirtyTex(texture_t& tex) noexcept;

bool RV_GetLineNdcBounds(
    const float p1x,
    const float p1y,
    const float p2x,
    const float p2y,
    float& lx,
    float& rx,
    const float lrpCullAdjust = 1.0f    // Left/right plane angle adjustment for culling (1.0 = no change)
) noexcept;

void RV_ClearSubsecDrawIndexes() noexcept;
void RV_DrawWidescreenStatusBarLetterbox() noexcept;

bool RV_LinesIntersect(
    const float a1x,
    const float a1y,
    const float a2x,
    const float a2y,
    const float b1x,
    const float b1y,
    const float b2x,
    const float b2y
) noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER
