#include "p_plats.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Renderer/r_local.h"
#include "g_game.h"
#include "p_floor.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PcPsx/Game.h"
#include "PcPsx/ScriptingEngine.h"

#include <algorithm>

static constexpr int32_t PLATWAIT   = 3;                // Number of seconds for platforms to be in the waiting state
static constexpr int32_t PLATSPEED  = 2 * FRACUNIT;     // Standard platform speed (some platforms are slower or faster)

// Contains all of the active platforms in the level (some slots may be empty)
#if PSYDOOM_LIMIT_REMOVING
    std::vector<plat_t*> gpActivePlats;
#else
    plat_t* gpActivePlats[MAXPLATS];
#endif

// Not required externally: making private to this module
static void P_AddActivePlat(plat_t& plat) noexcept;
static void P_RemoveActivePlat(plat_t& plat) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: default custom platform settings
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS
CustomPlatDef::CustomPlatDef() noexcept
    : bCrush(false)
    , bDoFinishScript(false)
    , startState(-1)
    , finishState(1)
    , minHeight(0)
    , maxHeight(0)
    , speed(PLATSPEED * 4)
    , waitTime(TICRATE * PLATWAIT)
    , startSound(sfx_pstart)
    , moveSound(sfx_None)
    , moveSoundFreq(8)
    , stopSound(sfx_pstop)
    , finishScriptActionNum(0)
    , finishScriptUserdata(0)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: do moving sounds for custom platforms
//------------------------------------------------------------------------------------------------------------------------------------------
static void doCustomPlatMoveSounds(const plat_t& plat, sector_t& platSector) noexcept {
    const sfxenum_t moveSound = plat.moveSound;

    if (moveSound != sfx_None) {
        const uint32_t moveSoundFreq = plat.moveSoundFreq;

        if ((moveSoundFreq <= 1) || ((gGameTic % moveSoundFreq) == 0)) {
            S_StartSound((mobj_t*) &platSector.soundorg, moveSound);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: finish a custom platform if it's reached the correct state and execute the 'finish' script action if appropriate.
// Returns 'true' if the platform is a custom platform which has been stopped.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool checkCustomPlatFinished(plat_t& plat, sector_t& platSector, const plat_e stateJustFinished) noexcept {
    // Not a custom platform that is finished?
    if ((plat.type != customPlat) || (stateJustFinished != plat.finishState))
        return false;

    // This custom platform is done, remove it and execute the finish script (if existing)
    P_RemoveActivePlat(plat);

    if (plat.bDoFinishScript) {
        ScriptingEngine::doAction(plat.finishScriptActionNum, nullptr, &platSector, nullptr, 0, plat.finishScriptUserdata);
    }

    return true;
}
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a moving platform: moves the platform, does state transitions and sounds etc.
//------------------------------------------------------------------------------------------------------------------------------------------
static void T_PlatRaise(plat_t& plat) noexcept {
    sector_t& sector = *plat.sector;
    const plattype_e platType = plat.type;

    // PsyDoom: a helper that plays one sound for regular platform types and another sound for custom platform types
    const auto playCustomizableSound = [&](const sfxenum_t sound, const sfxenum_t customPlatSound) noexcept {
        #if PSYDOOM_MODS
            if (platType == customPlat) {
                if (customPlatSound != sfx_None) {
                    S_StartSound((mobj_t*) &sector.soundorg, customPlatSound);
                }
            } else {
                S_StartSound((mobj_t*) &sector.soundorg, sound);
            }
        #else
            S_StartSound((mobj_t*) &sector.soundorg, sound);
        #endif
    };

    // Do the platform logic and movement
    switch (plat.status) {
        // Going up
        case up: {
            const result_e moveResult = T_MovePlane(sector, plat.speed, plat.high, plat.crush, false, 1);

            // Do movement sounds every so often for certain platform types
            if ((platType == raiseAndChange) || (platType == raiseToNearestAndChange)) {
                if ((gGameTic & 7U) == 0) {
                    S_StartSound((mobj_t*) &sector.soundorg, sfx_stnmov);
                }
            }

            #if PSYDOOM_MODS
                if (platType == customPlat) {
                    doCustomPlatMoveSounds(plat, sector);
                }
            #endif

            // Decide what to do base on the move result and platform settings
            if ((moveResult == crushed) && (!plat.crush)) {
                // Crushing something and this platform doesn't crush: change direction
                plat.status = down;
                plat.count = plat.wait;
                playCustomizableSound(sfx_pstart, plat.startSound);
            }
            else if (moveResult == pastdest) {
                // Reached the destination for the platform: wait by default (overrides below) and play the stopped sound
                plat.status = waiting;
                plat.count = plat.wait;
                playCustomizableSound(sfx_pstop, plat.stopSound);

                // Certain platform types stop now at this point
                switch (platType) {
                    case downWaitUpStay:
                    case raiseAndChange:
                // PsyDoom: fix a bug where certain 'raise platform' actions would not fully finish when the target floor height was reached.
                // This bug would prevent further specials from being executed on sectors, because their floor movers were still regarded as 'active'.
                // The bug is present in the Jaguar and PSX versions of Doom, but does not appear to be in the PC/Linux sources.
                #if PSYDOOM_MODS
                    case raiseToNearestAndChange:
                #endif
                    case blazeDWUS:
                        P_RemoveActivePlat(plat);
                        break;

                    default:
                        break;
                }

                // PsyDoom: check if the custom platform has reached it's destination
                #if PSYDOOM_MODS
                    checkCustomPlatFinished(plat, sector, up);
                #endif
            }
        }   break;

        // Going down
        case down: {
            const result_e moveResult = T_MovePlane(sector, plat.speed, plat.low, false, 0, -1);

            // PsyDoom: do custom platform moving sounds
            #if PSYDOOM_MODS
                if (platType == customPlat) {
                    doCustomPlatMoveSounds(plat, sector);
                }
            #endif

            // Time to start waiting before going back up again?
            if (moveResult == pastdest) {
                plat.status = waiting;
                plat.count = plat.wait;
                playCustomizableSound(sfx_pstop, plat.stopSound);

                // PsyDoom: check if the custom platform has reached it's destination
                #if PSYDOOM_MODS
                    checkCustomPlatFinished(plat, sector, down);
                #endif
            }
        }   break;

        // Waiting to go back up (or down) again?
        case waiting: {
            // PsyDoom: check if the custom platform has reached it's destination
            #if PSYDOOM_MODS
                if (checkCustomPlatFinished(plat, sector, waiting))
                    return;
            #endif

            plat.count--;

            // PsyDoom: wait period is halved in 'turbo' mode
            #if PSYDOOM_MODS
                if (Game::gSettings.bTurboMode) {
                    if (plat.count > 0) {
                        plat.count--;
                    }
                }
            #endif

            // Time to end the wait and begin moving again?
            if (plat.count == 0) {
                plat.status = (sector.floorheight == plat.low) ? up : down;     // Go back in the direction we came from
                playCustomizableSound(sfx_pstart, plat.startSound);
            }
        }   break;

        case in_stasis:
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// For each sector matching the given line's tag, spawn a moving platform thinker/process of the given platform type.
//
// Notes:
//  (1) If a sector already has some special action going on, then a platform thinker will NOT be added to the sector.
//  (2) The movement amount is only used for certain platform types.
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoPlat(line_t& line, const plattype_e platType, const int32_t moveAmount) noexcept {
    // Try re-activate moving platforms that are in stasis for certain platform types
    switch (platType) {
        case perpetualRaise: {
            // PsyDoom: this was renamed for clarity
            #if PSYDOOM_MODS
                P_ActivateInStasisPlatForTag(line.tag);
            #else
                P_ActivateInStasis(line.tag);
            #endif
        }   break;

        default:
            break;
    }

    // Spawn moving platforms for all sectors matching the line tag (which don't already have specials)
    bool bActivatedPlats = false;

    for (int32_t sectorIdx = P_FindSectorFromLineTag(line, -1); sectorIdx >= 0; sectorIdx = P_FindSectorFromLineTag(line, sectorIdx)) {
        // Only spawn the platform if there isn't already a special operating on this sector
        sector_t& sector = gpSectors[sectorIdx];

        if (sector.specialdata)
            continue;

        // Create the platform thinker, link to it's sector and populate its state/settings
        bActivatedPlats = true;
        plat_t& plat = *(plat_t*) Z_Malloc(*gpMainMemZone, sizeof(plat_t), PU_LEVSPEC, nullptr);

        #if PSYDOOM_MODS
            plat = {};  // PsyDoom: zero-init all fields, including ones unused by this function
        #endif

        P_AddThinker(plat.thinker);
        sector.specialdata = &plat;

        plat.type = platType;
        plat.sector = &sector;
        plat.thinker.function = (think_t) &T_PlatRaise;
        plat.crush = false;
        plat.tag = line.tag;

        // Platform specific setup and sounds
        switch (platType) {
            case raiseToNearestAndChange: {
                plat.speed = PLATSPEED / 2;
                sector.floorpic = gpSides[line.sidenum[0]].sector->floorpic;
                plat.high = P_FindNextHighestFloor(sector, sector.floorheight);
                plat.wait = 0;
                plat.status = up;
                sector.special = 0;     // Remove any damaging (slime, lava etc.) or other specials
                S_StartSound((mobj_t*) &sector.soundorg, sfx_stnmov);
            }   break;

            case raiseAndChange: {
                plat.speed = PLATSPEED / 2;
                sector.floorpic = gpSides[line.sidenum[0]].sector->floorpic;
                plat.high = sector.floorheight + moveAmount * FRACUNIT;
                plat.wait = 0;
                plat.status = up;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_stnmov);
            }   break;

            case downWaitUpStay: {
                plat.speed = PLATSPEED * 4;
                plat.low = std::min(P_FindLowestFloorSurrounding(sector), sector.floorheight);
                plat.wait = TICRATE * PLATWAIT;
                plat.status = down;
                plat.high = sector.floorheight;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstart);
            }   break;

            case blazeDWUS: {
                plat.speed = PLATSPEED * 8;
                plat.low = std::min(P_FindLowestFloorSurrounding(sector), sector.floorheight);
                plat.wait = TICRATE * PLATWAIT;
                plat.status = down;
                plat.high = sector.floorheight;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstart);
            }   break;

            case perpetualRaise: {
                plat.speed = PLATSPEED;
                plat.low = std::min(P_FindLowestFloorSurrounding(sector), sector.floorheight);
                plat.high = std::max(P_FindHighestFloorSurrounding(sector), sector.floorheight);
                plat.wait = TICRATE * PLATWAIT;
                plat.status = (P_Random() & 1) ? down : up;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstart);
            }   break;

            default:
                break;
        }

        // Add the platform to the active platforms list
        P_AddActivePlat(plat);
    }

    return bActivatedPlats;
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: add the ability to trigger a customized platform/elevator on a sector via scripts
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoCustomPlat(sector_t& sector, const CustomPlatDef platDef) noexcept {
    // Can't do a platform on a sector that already has a special
    if (sector.specialdata)
        return false;

    // Alloc the platform, zero-init and set it up as a thinker for the sector
    plat_t& plat = *(plat_t*) Z_Malloc(*gpMainMemZone, sizeof(plat_t), PU_LEVSPEC, nullptr);
    plat = {};
    P_AddThinker(plat.thinker);
    sector.specialdata = &plat;
    plat.thinker.function = (think_t) &T_PlatRaise;

    // Init all other platform fields
    plat.sector = &sector;
    plat.speed = std::abs(platDef.speed);
    plat.low = std::min(platDef.minHeight, platDef.maxHeight);
    plat.high = std::max(platDef.minHeight, platDef.maxHeight);
    plat.wait = std::max(platDef.waitTime, 1);
    plat.count = (platDef.startState == 0) ? plat.wait : 0;

    if (platDef.startState < 0) {
        plat.status = down;
    } else if (platDef.startState > 0) {
        plat.status = up;
    } else {
        plat.status = waiting;
    }

    if (platDef.finishState < 0) {
        plat.finishState = down;
    } else if (platDef.finishState > 0) {
        plat.finishState = up;
    } else {
        plat.finishState = waiting;
    }

    plat.oldstatus = waiting;
    plat.crush = platDef.bCrush;
    plat.bDoFinishScript = platDef.bDoFinishScript;
    plat.tag = sector.tag;
    plat.type = customPlat;
    plat.startSound = platDef.startSound;
    plat.moveSound = platDef.moveSound;
    plat.moveSoundFreq = platDef.moveSoundFreq;
    plat.stopSound = platDef.stopSound;
    plat.finishScriptActionNum = platDef.finishScriptActionNum;
    plat.finishScriptUserdata = platDef.finishScriptUserdata;

    // Add the platform to the active platforms list and play the start sound, if any
    P_AddActivePlat(plat);
    const sfxenum_t startSound = platDef.startSound;

    if (startSound != sfx_None) {
        S_StartSound((mobj_t*) &sector.soundorg, startSound);
    }

    // This operation was successful
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: activate paused platforms/elevators matching the specified generic condition
//------------------------------------------------------------------------------------------------------------------------------------------
template <class PlatFilterT>
static bool P_ActivateMatchingInStasisPlats(const PlatFilterT& platFilter) noexcept {
    bool bUnpausedAPlat = false;

    #if PSYDOOM_LIMIT_REMOVING
        const int32_t numPlats = (int32_t) gpActivePlats.size();
    #else
        const int32_t numPlats = MAXPLATS;
    #endif

    for (int32_t i = 0; i < numPlats; ++i) {
        plat_t* pPlat = gpActivePlats[i];

        if (pPlat && (pPlat->status == in_stasis) && platFilter(*pPlat)) {
            pPlat->status = pPlat->oldstatus;
            pPlat->thinker.function = (think_t) &T_PlatRaise;
            bUnpausedAPlat = true;
        }
    }

    return bUnpausedAPlat;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reactivates moving platforms that were paused which match the given sector tag. Returns 'true' if any platforms were affected.
// PsyDoom: this function has been rewritten and renamed from 'P_ActivateInStasis'. For the original version see the 'Old' code folder.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_ActivateInStasisPlatForTag(const int32_t tag) noexcept {
    return P_ActivateMatchingInStasisPlats([&](const plat_t& p) noexcept { return (p.tag == tag); });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: this is a new variant of 'P_ActivateInStasisPlat' that targets a specific sector
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_ActivateInStasisPlatForSector(const sector_t& sector) noexcept {
    return P_ActivateMatchingInStasisPlats([&](const plat_t& p) noexcept { return (p.sector == &sector); });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: stops moving platforms matching a specified generic condition. Returns 'true' if any platforms were stoppped
//------------------------------------------------------------------------------------------------------------------------------------------
template <class PlatFilterT>
static bool EV_StopMatchingPlats(const PlatFilterT& platFilter) noexcept {
    bool bStoppedAPlat = false;

    #if PSYDOOM_LIMIT_REMOVING
        const int32_t numPlats = (int32_t) gpActivePlats.size();
    #else
        const int32_t numPlats = MAXPLATS;
    #endif

    for (int32_t i = 0; i < numPlats; ++i) {
        plat_t* const pPlat = gpActivePlats[i];

        if (pPlat && (pPlat->status != in_stasis) && platFilter(*pPlat)) {
            // Stop this moving platform: remember the status before stopping and put into stasis
            pPlat->oldstatus = pPlat->status;
            pPlat->status = in_stasis;
            pPlat->thinker.function = nullptr;
            bStoppedAPlat = true;
        }
    }

    return bStoppedAPlat;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: functions to stop moving platforms matching a sector tag or sector (exposed via Lua scripting)
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_StopPlatForTag(const int32_t tag) noexcept {
    return EV_StopMatchingPlats([&](const plat_t& p) noexcept { return (p.tag == tag); });
}

bool EV_StopPlatForSector(const sector_t& sector) noexcept {
    return EV_StopMatchingPlats([&](const plat_t& p) noexcept { return (p.sector == &sector); });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stops moving platforms that are moving that have a sector tag matching the given line's tag.
// PsyDoom: this function has been rewritten. For the original version see the 'Old' code folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void EV_StopPlat(line_t& line) noexcept {
    EV_StopPlatForTag(line.tag);
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Add the given moving platform to a free slot in the 'active platforms' list
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_AddActivePlat(plat_t& plat) noexcept {
    #if PSYDOOM_LIMIT_REMOVING
        const int32_t numPlats = (int32_t) gpActivePlats.size();
    #else
        const int32_t numPlats = MAXPLATS;
    #endif

    for (int32_t i = 0; i < numPlats; ++i) {
        if (!gpActivePlats[i]) {
            gpActivePlats[i] = &plat;
            return;
        }
    }

    #if PSYDOOM_LIMIT_REMOVING
        // No room in the array: make a new slot
        gpActivePlats.push_back(&plat);
    #else
        I_Error("P_AddActivePlat: no more plats!");
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Remove the given moving platform from the active platforms list; also dissociates it with it's sector and deallocs it's thinker
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_RemoveActivePlat(plat_t& plat) noexcept {
    #if PSYDOOM_LIMIT_REMOVING
        const int32_t numPlats = (int32_t) gpActivePlats.size();
    #else
        const int32_t numPlats = MAXPLATS;
    #endif

    for (int32_t i = 0; i < numPlats; ++i) {
        if (gpActivePlats[i] == &plat) {
            plat.sector->specialdata = nullptr;
            P_RemoveThinker(plat.thinker);
            gpActivePlats[i] = nullptr;

            // PsyDoom: compact the array if it's the last element
            #if PSYDOOM_LIMIT_REMOVING
                if (i + 1 >= numPlats) {
                    gpActivePlats.pop_back();
                }
            #endif

            return;
        }
    }

    I_Error("P_RemoveActivePlat: can\'t find plat!");
}
