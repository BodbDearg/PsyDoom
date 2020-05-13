#include "p_lights.h"

#include "Doom/Base/m_random.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Renderer/r_local.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"
#include <algorithm>

// Definition and state for a fire flicker light
struct fireflicker_t {
    thinker_t           thinker;
    VmPtr<sector_t>     sector;
    int32_t             count;
    int32_t             maxlight;
    int32_t             minlight;
};

static_assert(sizeof(fireflicker_t) == 28);

// Definition and state for a flashing light
struct lightflash_t {
    thinker_t           thinker;
    VmPtr<sector_t>     sector;
    int32_t             count;
    int32_t             maxlight;
    int32_t             minlight;
    int32_t             maxtime;
    int32_t             mintime;
};

static_assert(sizeof(lightflash_t) == 36);

// Definition and state for a strobing light
struct strobe_t {
    thinker_t           thinker;
    VmPtr<sector_t>     sector;
    int32_t             count;
    int32_t             minlight;
    int32_t             maxlight;
    int32_t             darktime;
    int32_t             brighttime;
};

static_assert(sizeof(strobe_t) == 36);

static constexpr int32_t GLOWSPEED      = 3;    // TODO: COMMENT
static constexpr int32_t STROBEBRIGHT   = 3;    // Number of tics in the bright state for the strobe flash
static constexpr int32_t TURBODARK      = 4;    // TODO: COMMENT
static constexpr int32_t FASTDARK       = 8;    // TODO: COMMENT
static constexpr int32_t SLOWDARK       = 15;   // Number of tics in the dark phase for a slow light strobe flash

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a light that flickers like fire
//------------------------------------------------------------------------------------------------------------------------------------------
static void T_FireFlicker(fireflicker_t& flicker) noexcept {
    // Time to flicker yet?
    if (--flicker.count != 0)
        return;

    // Do the flicker and reset the countdown till the next flicker
    sector_t& sector = *flicker.sector;
    const int32_t variation = (P_Random() & 3) * 16;

    if (sector.lightlevel - variation < flicker.minlight) {
        sector.lightlevel = (int16_t)(flicker.minlight);
    } else {
        sector.lightlevel = (int16_t)(flicker.maxlight - variation);
    }

    flicker.count = 3;
}

// TODO: REMOVE eventually
void _thunk_T_FireFlicker() noexcept {
    T_FireFlicker(*vmAddrToPtr<fireflicker_t>(*PsxVm::gpReg_a0));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a fire flicker light effect on the given sector
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnFireFlicker(sector_t& sector) noexcept {
    // Clear the current sector special (no hurt for example) and spawn the thinker
    sector.special = 0;
    fireflicker_t& flicker = *(fireflicker_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(fireflicker_t), PU_LEVSPEC, nullptr);
    P_AddThinker(flicker.thinker);

    // Setup flicker settings
    flicker.thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_FireFlicker);
    flicker.sector = &sector;
    flicker.maxlight = sector.lightlevel;
    flicker.minlight = P_FindMinSurroundingLight(sector, sector.lightlevel) + 16;
    flicker.count = 3;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a flashing light
//------------------------------------------------------------------------------------------------------------------------------------------
void T_LightFlash(lightflash_t& lightFlash) noexcept {
    // Time to flash yet?
    if (--lightFlash.count != 0)
        return;

    // Do the flash and reset the countdown till the next flash
    sector_t& sector = *lightFlash.sector;

    if (sector.lightlevel == lightFlash.maxlight) {
        sector.lightlevel = (int16_t) lightFlash.minlight;
        lightFlash.count = (P_Random() & lightFlash.mintime) + 1;
    } else {
        sector.lightlevel = (int16_t) lightFlash.maxlight;
        lightFlash.count = (P_Random() & lightFlash.maxtime) + 1;
    }
}

// TODO: REMOVE eventually
void _thunk_T_LightFlash() noexcept {
    T_LightFlash(*vmAddrToPtr<lightflash_t>(*PsxVm::gpReg_a0));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a light flash special for the given sector
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnLightFlash(sector_t& sector) noexcept {
    // Clear the current sector special (no hurt for example) and spawn the thinker
    sector.special = 0;
    lightflash_t& lightFlash = *(lightflash_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(lightflash_t), PU_LEVSPEC, nullptr);
    P_AddThinker(lightFlash.thinker);

    // Setup flash settings
    lightFlash.thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_LightFlash);
    lightFlash.sector = &sector;
    lightFlash.maxlight = sector.lightlevel;
    lightFlash.minlight = P_FindMinSurroundingLight(sector, sector.lightlevel);
    lightFlash.maxtime = 64;
    lightFlash.mintime = 7;
    lightFlash.count = (P_Random() & lightFlash.maxtime) + 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a strobe flash light
//------------------------------------------------------------------------------------------------------------------------------------------
void T_StrobeFlash(strobe_t& strobe) noexcept {
    // Time to flash yet?
    if (--strobe.count != 0)
        return;

    // Do the flash and reset the countdown till the next flash
    sector_t& sector = *strobe.sector;

    if (sector.lightlevel == strobe.minlight) {
        sector.lightlevel = (int16_t) strobe.maxlight;
        strobe.count = strobe.brighttime;
    } else {
        sector.lightlevel = (int16_t) strobe.minlight;
        strobe.count = strobe.darktime;
    }
}

// TODO: REMOVE eventually
void _thunk_T_StrobeFlash() noexcept {
    T_StrobeFlash(*vmAddrToPtr<strobe_t>(*PsxVm::gpReg_a0));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a strobe flash special for the given sector
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnStrobeFlash(sector_t& sector, const int32_t darkTime, const bool bInSync) noexcept {
    // Create the strobe thinker and populate it's settings
    strobe_t& strobe = *(strobe_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(strobe_t), PU_LEVSPEC, nullptr);
    P_AddThinker(strobe.thinker);

    strobe.thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_StrobeFlash);
    strobe.sector = &sector;
    strobe.darktime = darkTime;
    strobe.brighttime = STROBEBRIGHT;
    strobe.minlight = P_FindMinSurroundingLight(sector, sector.lightlevel);
    strobe.maxlight = sector.lightlevel;

    // If there's no light difference to the min surrounding light then flash to 0
    if (strobe.minlight == strobe.maxlight) {
        strobe.minlight = 0;
    }

    // Is the flash in sync?
    if (bInSync) {
        strobe.count = 1;
    } else {
        strobe.count = (P_Random() & 7) + 1;
    }

    // Remove any other sector specials like damage etc.
    sector.special = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a rapid strobe flash special for the given sector
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnRapidStrobeFlash(sector_t& sector) noexcept {
    // Create the strobe thinker and populate it's settings
    strobe_t& strobe = *(strobe_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(strobe_t), PU_LEVSPEC, nullptr);
    P_AddThinker(strobe.thinker);
    
    strobe.thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_StrobeFlash);
    strobe.sector = &sector;
    strobe.darktime = 1;
    strobe.brighttime = 1;
    strobe.minlight = 10;
    strobe.maxlight = sector.lightlevel;
    strobe.count = 1;
    
    // If the min value is the same as the sector light level then flash to zero
    if (strobe.minlight == strobe.maxlight) {
        strobe.minlight = 0;
    }

    // Remove any other sector specials like damage etc.
    sector.special = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Starts a slow (in the dark phase) light strobe flash in sectors with the same tag as the given line
//------------------------------------------------------------------------------------------------------------------------------------------
void EV_StartLightStrobing(line_t& line) noexcept {
    for (int32_t sectorIdx = P_FindSectorFromLineTag(line, -1); sectorIdx >= 0; sectorIdx = P_FindSectorFromLineTag(line, sectorIdx)) {
        sector_t& sector = gpSectors->get()[sectorIdx];

        // Only spawn the strobe if there isn't a special on the sector already
        if (!sector.specialdata) {
            P_SpawnStrobeFlash(sector, SLOWDARK, false);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Turns 'off' lights for sectors matching the given line's tag; makes those sectors use the lowest surrounding light level
//------------------------------------------------------------------------------------------------------------------------------------------
void EV_TurnTagLightsOff(line_t& line) noexcept {
    // Turn 'off' the light for all sectors with a matching tag
    sector_t* const pSectors = gpSectors->get();
    
    for (int32_t sectorIdx = 0; sectorIdx < *gNumSectors; ++sectorIdx) {
        sector_t& sector = pSectors[sectorIdx];

        if (sector.tag != line.tag)
            continue;

        // Tag matches: find the lowest light level of surrounding sectors and use that as the new light level
        int16_t minLightLevel = sector.lightlevel;
        line_t* const pLines = sector.lines->get();

        for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
            sector_t* const pNextSector = getNextSector(pLines[lineIdx], sector);

            if (pNextSector) {
                minLightLevel = std::min(minLightLevel, pNextSector->lightlevel);
            }
        }

        sector.lightlevel = minLightLevel;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Turns 'on' lights for sectors matching the given line's tag.
// Uses the given light level as the 'on' light level, or the highest surrounding light level if '0' is specified.
//------------------------------------------------------------------------------------------------------------------------------------------
void EV_LightTurnOn(line_t& line, const int32_t onLightLevel) noexcept {
    // Turn 'on' the light for all sectors with a matching tag
    sector_t* const pSectors = gpSectors->get();

    for (int32_t sectorIdx = 0; sectorIdx < *gNumSectors; ++sectorIdx) {
        sector_t& sector = pSectors[sectorIdx];

        if (sector.tag != line.tag)
            continue;

        // Tag matches: use the given light level as the 'on' light level, or if '0' is specified for that
        // use the highest light level found in surrounding sectors.
        int32_t newLightLevel = onLightLevel;

        if (onLightLevel == 0) {
            line_t* const pLines = sector.lines->get();

            for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
                sector_t* const pNextSector = getNextSector(pLines[lineIdx], sector);
                
                if (pNextSector) {
                    newLightLevel = std::max(newLightLevel, (int32_t) pNextSector->lightlevel);
                }
            }
        }

        sector.lightlevel = (int16_t) newLightLevel;
    }
}

void T_Glow() noexcept {
    v1 = lw(a0 + 0x18);
    a2 = -1;                                            // Result = FFFFFFFF
    v0 = 1;                                             // Result = 00000001
    if (v1 == a2) goto loc_8001B4C0;
    if (v1 == v0) goto loc_8001B50C;
    goto loc_8001B550;
loc_8001B4C0:
    v1 = lw(a0 + 0xC);
    v0 = lhu(v1 + 0x12);
    v0 -= 3;
    sh(v0, v1 + 0x12);
    a1 = lw(a0 + 0xC);
    v1 = lw(a0 + 0x10);
    v0 = lh(a1 + 0x12);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001B550;
    v0 = lhu(a0 + 0x10);
    sh(v0, a1 + 0x12);
    v0 = 1;                                             // Result = 00000001
    sw(v0, a0 + 0x18);
    goto loc_8001B550;
loc_8001B50C:
    v1 = lw(a0 + 0xC);
    v0 = lhu(v1 + 0x12);
    v0 += 3;
    sh(v0, v1 + 0x12);
    a1 = lw(a0 + 0xC);
    v0 = lw(a0 + 0x14);
    v1 = lh(a1 + 0x12);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001B550;
    v0 = lhu(a0 + 0x14);
    sh(v0, a1 + 0x12);
    sw(a2, a0 + 0x18);
loc_8001B550:
    return;
}

void P_SpawnGlowingLight() noexcept {
loc_8001B558:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    a1 = 0x1C;                                          // Result = 0000001C
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x1C);
    sw(s0, sp + 0x10);
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x4B60;                                       // Result = T_Glow (8001B4A0)
    a0 = 1;                                             // Result = 00000001
    sw(s1, s0 + 0xC);
    sw(v0, s0 + 0x8);
    if (s2 == a0) goto loc_8001B5F4;
    v0 = (i32(s2) < 2);
    if (v0 == 0) goto loc_8001B5CC;
    if (s2 == 0) goto loc_8001B5E0;
    sw(0, s1 + 0x14);
    goto loc_8001B624;
loc_8001B5CC:
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (s2 == v0);
        v0 = 0xFF;                                      // Result = 000000FF
        if (bJump) goto loc_8001B610;
    }
    sw(0, s1 + 0x14);
    goto loc_8001B624;
loc_8001B5E0:
    a1 = lh(s1 + 0x12);
    a0 = s1;
    v0 = P_FindMinSurroundingLight(*vmAddrToPtr<sector_t>(a0), a1);
    sw(v0, s0 + 0x10);
    goto loc_8001B5FC;
loc_8001B5F4:
    v0 = 0xA;                                           // Result = 0000000A
    sw(v0, s0 + 0x10);
loc_8001B5FC:
    v1 = lh(s1 + 0x12);
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, s0 + 0x18);
    sw(v1, s0 + 0x14);
    goto loc_8001B620;
loc_8001B610:
    v1 = lh(s1 + 0x12);
    sw(v0, s0 + 0x14);
    sw(a0, s0 + 0x18);
    sw(v1, s0 + 0x10);
loc_8001B620:
    sw(0, s1 + 0x14);
loc_8001B624:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
