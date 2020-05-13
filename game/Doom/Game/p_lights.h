#pragma once

#include <cstdint>

struct sector_t;

void P_SpawnFireFlicker(sector_t& sector) noexcept;
void P_SpawnLightFlash(sector_t& sector) noexcept;
void P_SpawnStrobeFlash(sector_t& sector, const int32_t darkTime, const bool bInSync) noexcept;
void P_SpawnRapidStrobeFlash(sector_t& sector) noexcept;
void EV_StartLightStrobing() noexcept;
void EV_TurnTagLightsOff() noexcept;
void EV_LightTurnOn() noexcept;
void T_Glow() noexcept;
void P_SpawnGlowingLight() noexcept;
