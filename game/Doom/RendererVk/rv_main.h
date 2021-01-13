#pragma once

#include "Macros.h"
#include "Doom/doomdef.h"

#if PSYDOOM_VULKAN_RENDERER

// PI to 96 digits: define here because it's not available in standard C/C++ (sigh... it's 2021 and this is still a thing)
template <class T> static constexpr T RV_PI = T(3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117);

// More convenience scalings of PI
template <class T> static constexpr T RV_2PI = RV_PI<T> * T(2.0);
template <class T> static constexpr T RV_PI_2 = RV_PI<T> / T(2.0);
template <class T> static constexpr T RV_PI_4 = RV_PI<T> / T(4.0);

extern float    gViewXf;
extern float    gViewYf;
extern float    gViewZf;
extern float    gViewAnglef;
extern float    gViewCosf;
extern float    gViewSinf;

float RV_FixedToFloat(const fixed_t num) noexcept;
fixed_t RV_FloatToFixed(const float num) noexcept;
float RV_AngleToFloat(const angle_t angle) noexcept;
float RV_FloatToAngle(const angle_t angle) noexcept;
void RV_GetSectorColor(const sector_t& sec, uint8_t& r, uint8_t& g, uint8_t& b) noexcept;
void RV_RenderPlayerView() noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER
