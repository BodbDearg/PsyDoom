#pragma once

#include "Doom/doomdef.h"

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

// Definition and state for a fire flicker light
struct fireflicker_t {
    thinker_t   thinker;
    sector_t*   sector;
    int32_t     count;
    int32_t     maxlight;
    int32_t     minlight;
};

// Definition and state for a flashing light
struct lightflash_t {
    thinker_t   thinker;
    sector_t*   sector;
    int32_t     count;
    int32_t     maxlight;
    int32_t     minlight;
    int32_t     maxtime;
    int32_t     mintime;
};

// Definition and state for a strobing light
struct strobe_t {
    thinker_t   thinker;
    sector_t*   sector;
    int32_t     count;
    int32_t     minlight;
    int32_t     maxlight;
    int32_t     darktime;
    int32_t     brighttime;
};

// Definition and state for a glowing light
struct glow_t {
    thinker_t   thinker;
    sector_t*   sector;
    int32_t     minlight;
    int32_t     maxlight;
    int32_t     direction;
};

void T_FireFlicker(fireflicker_t& flicker) noexcept;
void P_SpawnFireFlicker(sector_t& sector) noexcept;
void T_LightFlash(lightflash_t& lightFlash) noexcept;
void P_SpawnLightFlash(sector_t& sector) noexcept;
void T_StrobeFlash(strobe_t& strobe) noexcept;
void P_SpawnStrobeFlash(sector_t& sector, const int32_t darkTime, const bool bInSync) noexcept;
void P_SpawnRapidStrobeFlash(sector_t& sector) noexcept;
void EV_StartLightStrobing(line_t& line) noexcept;
void EV_TurnTagLightsOff(line_t& line) noexcept;
void EV_LightTurnOn(line_t& line, const int32_t onLightLevel) noexcept;
void T_Glow(glow_t& glow) noexcept;
void P_SpawnGlowingLight(sector_t& sector, const glowtype_e glowType) noexcept;
