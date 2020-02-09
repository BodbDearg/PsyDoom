#pragma once

#include <cstdint>

struct texture_t;

// Size of the firesky texture
static constexpr int32_t FIRESKY_W = 64;
static constexpr int32_t FIRESKY_H = 128;

void P_UpdateFireSky(texture_t& skyTex) noexcept;
