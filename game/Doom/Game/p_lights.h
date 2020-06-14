#pragma once

#include <cstdint>

struct line_t;
struct sector_t;

// New for PSX: what type of glowing light to spawn
enum glowtype_e : int32_t {
    glowtolower,
    glowto10,
    glowto255
};

static constexpr int32_t GLOWSPEED      = 3;    // Speed that glowing lights ramp up and down at
static constexpr int32_t STROBEBRIGHT   = 3;    // Number of tics in the bright state for the strobe flash
static constexpr int32_t TURBODARK      = 4;    // Flashing lights: time spent in the 'dark' phase (short duration setting)
static constexpr int32_t FASTDARK       = 8;    // Flashing lights: time spent in the 'dark' phase (medium duration setting)
static constexpr int32_t SLOWDARK       = 15;   // Flashing lights: time spent in the 'dark' phase (longer duration setting)

void P_SpawnFireFlicker(sector_t& sector) noexcept;
void P_SpawnLightFlash(sector_t& sector) noexcept;
void P_SpawnStrobeFlash(sector_t& sector, const int32_t darkTime, const bool bInSync) noexcept;
void P_SpawnRapidStrobeFlash(sector_t& sector) noexcept;
void EV_StartLightStrobing(line_t& line) noexcept;
void EV_TurnTagLightsOff(line_t& line) noexcept;
void EV_LightTurnOn(line_t& line, const int32_t onLightLevel) noexcept;
void P_SpawnGlowingLight(sector_t& sector, const glowtype_e glowType) noexcept;
