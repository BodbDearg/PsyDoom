#pragma once

#include <cstdint>

uint8_t wess_master_sfx_volume_get() noexcept;
uint8_t wess_master_mus_volume_get() noexcept;
void wess_master_sfx_vol_set() noexcept;
void wess_master_mus_vol_set() noexcept;
void wess_pan_mode_get() noexcept;
void wess_pan_mode_set() noexcept;
