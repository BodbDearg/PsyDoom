#pragma once

struct sector_t;

void P_SpawnFireFlicker(sector_t& sector) noexcept;
void P_SpawnLightFlash(sector_t& sector) noexcept;
void T_StrobeFlash() noexcept;
void P_SpawnStrobeFlash() noexcept;
void P_SpawnRapidStrobeFlash() noexcept;
void EV_StartLightStrobing() noexcept;
void EV_TurnTagLightsOff() noexcept;
void EV_LightTurnOn() noexcept;
void T_Glow() noexcept;
void P_SpawnGlowingLight() noexcept;
