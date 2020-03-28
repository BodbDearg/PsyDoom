#pragma once

#include <cstdint>

enum PanMode : uint8_t;

uint8_t wess_master_sfx_volume_get() noexcept;
uint8_t wess_master_mus_volume_get() noexcept;
void wess_master_sfx_vol_set(const uint8_t vol) noexcept;
void wess_master_mus_vol_set() noexcept;
PanMode wess_pan_mode_get() noexcept;
void wess_pan_mode_set(const PanMode mode) noexcept;
