#include "p_ceiling.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Renderer/r_local.h"
#include "g_game.h"
#include "p_floor.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsyDoom/ScriptingEngine.h"

#include <cmath>

// Normal move speed for ceilings/crushers
static constexpr fixed_t CEILSPEED = FRACUNIT * 2;

// The list of currently active ceilings (some slots may be empty)
#if PSYDOOM_LIMIT_REMOVING
    std::vector<ceiling_t*> gpActiveCeilings;
#else
    ceiling_t* gpActiveCeilings[MAXCEILINGS];
#endif

// Not required externally: making private to this module
static void P_ActivateInStasisCeiling(line_t& line) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: default custom ceiling settings
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS
CustomCeilingDef::CustomCeilingDef() noexcept
    : bCrush(true)
    , bDoFinishScript(false)
    , minHeight(0)
    , maxHeight(0)
    , startDir(-1)
    , normalSpeed(CEILSPEED)
    , crushSpeed(CEILSPEED / 8)
    , numDirChanges(-1)
    , startSound(sfx_None)
    , moveSound(sfx_stnmov)
    , moveSoundFreq(8)
    , changeDirSound(sfx_pstop)
    , stopSound(sfx_None)
    , finishScriptActionNum(0)
    , finishScriptUserdata(0)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: do moving sounds for custom ceilings
//------------------------------------------------------------------------------------------------------------------------------------------
static void doCustomCeilingMoveSounds(const ceiling_t& ceiling, sector_t& ceilingSector) noexcept {
    const sfxenum_t moveSound = ceiling.moveSound;

    if (moveSound != sfx_None) {
        const uint32_t moveSoundFreq = ceiling.moveSoundFreq;

        if ((moveSoundFreq <= 1) || ((gGameTic % moveSoundFreq) == 0)) {
            S_StartSound((mobj_t*) &ceilingSector.soundorg, moveSound);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: do logic for when a custom ceiling reaches a destination height
//------------------------------------------------------------------------------------------------------------------------------------------
static void onCustomCeilingDestHeightReached(ceiling_t& ceiling, sector_t& ceilingSector) noexcept {
    // Stop the and remove ceiling if it's got no more direction changes left
    const int32_t dirChangesLeft = ceiling.dirChangesLeft;

    if (dirChangesLeft == 0) {
        P_RemoveActiveCeiling(ceiling);

        // Play the stop sound (if any)
        const sfxenum_t stopSound = ceiling.stopSound;

        if (stopSound != sfx_None) {
            S_StartSound((mobj_t*) &ceilingSector.soundorg, stopSound);
        }

        // Execute the 'finished' script action (if any)
        if (ceiling.bDoFinishScript) {
            ScriptingEngine::doAction(ceiling.finishScriptActionNum, nullptr, &ceilingSector, nullptr, 0, ceiling.finishScriptUserdata);
        }

        // Done with this ceiling
        return;
    }

    // Is the ceiling on a finite series of direction changes? (there is now one less)
    if (dirChangesLeft > 0) {
        ceiling.dirChangesLeft = dirChangesLeft - 1;
    }

    // Play the direction change sound (if any)
    const sfxenum_t changeDirSound = ceiling.changeDirSound;

    if (changeDirSound != sfx_None) {
        S_StartSound((mobj_t*) &ceilingSector.soundorg, changeDirSound);
    }

    // Finally, change the ceiling direction itself
    ceiling.direction = (ceiling.direction < 0) ? +1 : -1;
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a moving ceiling or crusher: moves the ceiling, does state transitions and sounds etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void T_MoveCeiling(ceiling_t& ceiling) noexcept {
    sector_t& ceilingSector = *ceiling.sector;
    const ceiling_e ceilingType = ceiling.type;

    // PsyDoom: custom ceilings use a different speed when crushing something last frame
    #if PSYDOOM_MODS
        const fixed_t moveSpeed = ((ceilingType == customCeiling) && ceiling.bIsCrushing) ? ceiling.crushSpeed : ceiling.speed;
    #else
        const fixed_t moveSpeed = ceiling.speed;
    #endif

    switch (ceiling.direction) {
        // In stasis
        case 0:
            break;

        // Moving up
        case 1: {
            const result_e moveResult = T_MovePlane(ceilingSector, moveSpeed, ceiling.topheight, false, 1, ceiling.direction);

            // PsyDoom: remember if the ceiling was crushing something for the next frame
            #if PSYDOOM_MODS
                ceiling.bIsCrushing = (moveResult == crushed);
            #endif

            // Do moving sounds
            if ((gGameTic & 7) == 0) {
                switch (ceilingType) {
                #if PSYDOOM_MODS
                    case customCeiling:
                #endif
                    case silentCrushAndRaise:
                        break;

                    default:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_stnmov);
                        break;
                }
            }

            #if PSYDOOM_MODS
                if (ceilingType == customCeiling) {
                    doCustomCeilingMoveSounds(ceiling, ceilingSector);
                }
            #endif

            // Reached the destination?
            if (moveResult == pastdest) {
                switch (ceilingType) {
                    case raiseToHighest:
                        P_RemoveActiveCeiling(ceiling);
                        break;

                    case silentCrushAndRaise:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_pstop);
                        ceiling.direction = -1;
                        break;

                    case fastCrushAndRaise:
                    case crushAndRaise:
                        ceiling.direction = -1;
                        break;

                #if PSYDOOM_MODS
                    case customCeiling:
                        onCustomCeilingDestHeightReached(ceiling, ceilingSector);
                        break;
                #endif

                    default:
                        break;
                }
            }
        }   break;

        // Moving down
        case -1: {
            const result_e moveResult = T_MovePlane(ceilingSector, moveSpeed, ceiling.bottomheight, ceiling.crush, 1, ceiling.direction);

            // PsyDoom: remember if the ceiling was crushing something for the next frame
            #if PSYDOOM_MODS
                ceiling.bIsCrushing = (moveResult == crushed);
            #endif

            // Do moving sounds
            if ((gGameTic & 7) == 0) {
                switch (ceilingType) {
                #if PSYDOOM_MODS
                    case customCeiling:
                #endif
                    case silentCrushAndRaise:
                        break;

                    default:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_stnmov);
                        break;
                }
            }

            #if PSYDOOM_MODS
                if (ceilingType == customCeiling) {
                    doCustomCeilingMoveSounds(ceiling, ceilingSector);
                }
            #endif

            // Reached the destination or crushing something?
            if (moveResult == pastdest) {
                // Reached the destination
                switch (ceilingType) {
                    case silentCrushAndRaise:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_pstop);
                        [[fallthrough]];
                    case crushAndRaise:
                        ceiling.speed = CEILSPEED;
                        [[fallthrough]];
                    case fastCrushAndRaise:
                        ceiling.direction = 1;
                        break;

                    case lowerToFloor:
                    case lowerAndCrush:
                        P_RemoveActiveCeiling(ceiling);
                        break;

                #if PSYDOOM_MODS
                    case customCeiling:
                        onCustomCeilingDestHeightReached(ceiling, ceilingSector);
                        break;
                #endif

                    default:
                        break;
                }
            }
            else if (moveResult == crushed) {
                // Crushing/hitting something
                switch (ceilingType) {
                    case lowerAndCrush:
                    case crushAndRaise:
                    case silentCrushAndRaise:
                        ceiling.speed = CEILSPEED / 8;
                        break;

                    default:
                        break;
                }
            }
        }   break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger a ceiling (mover/crusher) special of the given type for sectors matching the given line's tag
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoCeiling(line_t& line, const ceiling_e ceilingType) noexcept {
    #if PSYDOOM_MODS
        ASSERT_LOG(ceilingType != customCeiling, "Custom ceilings must be created via 'EV_DoCustomCeiling'!");
    #endif

    // Try re-activate ceilings that are in stasis for certain ceiling types
    switch (ceilingType) {
        case crushAndRaise:
        case fastCrushAndRaise:
        case silentCrushAndRaise:
            P_ActivateInStasisCeiling(line);
            break;

        default:
            break;
    }

    // Spawn ceiling movers for all sectors matching the line tag (which don't already have specials)
    bool bActivatedACeiling = false;

    for (int32_t sectorIdx = P_FindSectorFromLineTag(line, -1); sectorIdx >= 0; sectorIdx = P_FindSectorFromLineTag(line, sectorIdx)) {
        // Only spawn the ceiling mover if there isn't already a special operating on this sector
        sector_t& sector = gpSectors[sectorIdx];

        if (sector.specialdata)
            continue;

        // Create the door thinker, link to it's sector and populate its state/settings
        bActivatedACeiling = true;
        ceiling_t& ceiling = *(ceiling_t*) Z_Malloc(*gpMainMemZone, sizeof(ceiling_t), PU_LEVSPEC, nullptr);

        #if PSYDOOM_MODS
            ceiling = {};   // PsyDoom: zero-init all fields, including ones unused by this function
        #endif

        P_AddThinker(ceiling.thinker);
        sector.specialdata = &ceiling;

        ceiling.thinker.function = (think_t) &T_MoveCeiling;
        ceiling.sector = &sector;
        ceiling.crush = false;

        // Ceiling specific setup and sounds
        switch (ceilingType) {
            case fastCrushAndRaise:
                ceiling.crush = true;
                ceiling.topheight = sector.ceilingheight;
                ceiling.bottomheight = sector.floorheight + 8 * FRACUNIT;
                ceiling.direction = -1;
                ceiling.speed = CEILSPEED * 2;
                break;

            case crushAndRaise:
            case silentCrushAndRaise:
                ceiling.crush = true;
                ceiling.topheight = sector.ceilingheight;
                [[fallthrough]];
            case lowerToFloor:
                [[fallthrough]];
            case lowerAndCrush:
                ceiling.bottomheight = sector.floorheight;

                if (ceilingType != lowerToFloor) {
                    ceiling.bottomheight += 8 * FRACUNIT;   // Leave a small gap
                }

                ceiling.direction = -1;
                ceiling.speed = CEILSPEED;
                break;

            case raiseToHighest:
                ceiling.topheight = P_FindHighestCeilingSurrounding(sector);
                ceiling.direction = 1;
                ceiling.speed = CEILSPEED;
                break;
                
            case customCeiling:
                ASSERT_FAIL("Custom ceilings should not use this function!");
                break;
        }

        // Remember the ceiling type and sector tag, and add to the active ceilings list
        ceiling.type = ceilingType;
        ceiling.tag = sector.tag;
        P_AddActiveCeiling(ceiling);
    }

    return bActivatedACeiling;
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: add the ability to trigger a customized crusher on a sector via scripts
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoCustomCeiling(sector_t& sector, const CustomCeilingDef& ceilDef) noexcept {
    // Can't do a crusher on the sector if it already has a special!
    if (sector.specialdata)
        return false;

    // Alloc the crusher, zero-init and set it up as a thinker for the sector
    ceiling_t& ceiling = *(ceiling_t*) Z_Malloc(*gpMainMemZone, sizeof(ceiling_t), PU_LEVSPEC, nullptr);
    ceiling = {};
    P_AddThinker(ceiling.thinker);
    sector.specialdata = &ceiling;
    ceiling.thinker.function = (think_t) &T_MoveCeiling;

    // Init all other crusher fields
    ceiling.type = customCeiling;
    ceiling.sector = &sector;
    ceiling.bottomheight = std::min(ceilDef.minHeight, ceilDef.maxHeight);
    ceiling.topheight = std::max(ceilDef.minHeight,  ceilDef.maxHeight);
    ceiling.speed = std::abs(ceilDef.normalSpeed);
    ceiling.crush = ceilDef.bCrush;
    ceiling.bDoFinishScript = ceilDef.bDoFinishScript;
    ceiling.direction = (ceilDef.startDir < 0) ? -1 : +1;
    ceiling.tag = sector.tag;
    ceiling.crushSpeed = std::abs(ceilDef.crushSpeed);
    ceiling.dirChangesLeft = ceilDef.numDirChanges;
    ceiling.moveSound = ceilDef.moveSound;
    ceiling.moveSoundFreq = ceilDef.moveSoundFreq;
    ceiling.changeDirSound = ceilDef.changeDirSound;
    ceiling.stopSound = ceilDef.stopSound;
    ceiling.finishScriptActionNum = ceilDef.finishScriptActionNum;
    ceiling.finishScriptUserdata = ceilDef.finishScriptUserdata;

    // Add to the active ceilings list and play the start sound, if any
    P_AddActiveCeiling(ceiling);
    const sfxenum_t startSound = ceilDef.startSound;

    if (startSound != sfx_None) {
        S_StartSound((mobj_t*) &sector.soundorg, startSound);
    }

    // This operation was successful
    return true;
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Add the given ceiling mover to a free slot in the 'active ceilings' list.
// Note: does NOT get added if there are no free slots.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_AddActiveCeiling(ceiling_t& ceiling) noexcept {
    #if PSYDOOM_LIMIT_REMOVING
        const int32_t numCeilings = (int32_t) gpActiveCeilings.size();
    #else
        const int32_t numCeilings = MAXCEILINGS;
    #endif

    for (int32_t i = 0; i < numCeilings; ++i) {
        if (!gpActiveCeilings[i]) {
            gpActiveCeilings[i] = &ceiling;
            return;
        }
    }

    #if PSYDOOM_LIMIT_REMOVING
        // No room in the array: make a new slot
        gpActiveCeilings.push_back(&ceiling);
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Remove the given ceiling from the active ceilings list; also dissociates it with it's sector and deallocs it's thinker
//------------------------------------------------------------------------------------------------------------------------------------------
void P_RemoveActiveCeiling(ceiling_t& ceiling) noexcept {
    #if PSYDOOM_LIMIT_REMOVING
        const int32_t numCeilings = (int32_t) gpActiveCeilings.size();
    #else
        const int32_t numCeilings = MAXCEILINGS;
    #endif

    for (int32_t i = 0; i < numCeilings; ++i) {
        if (gpActiveCeilings[i] == &ceiling) {
            ceiling.sector->specialdata = nullptr;
            P_RemoveThinker(ceiling.thinker);
            gpActiveCeilings[i] = nullptr;

            // PsyDoom: compact the array if it's the last element
            #if PSYDOOM_LIMIT_REMOVING
                if (i + 1 >= numCeilings) {
                    gpActiveCeilings.pop_back();
                }
            #endif

            return;
        }
    }
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: activate paused ceiling movers (crushers) matching the specified generic condition
//------------------------------------------------------------------------------------------------------------------------------------------
template <class CeilFilterT>
static bool P_ActivateMatchingInStasisCeilings(const CeilFilterT& ceilFilter) noexcept {
    #if PSYDOOM_LIMIT_REMOVING
        const int32_t numCeilings = (int32_t) gpActiveCeilings.size();
    #else
        const int32_t numCeilings = MAXCEILINGS;
    #endif

    bool bUnpausedACrusher = false;

    for (int32_t i = 0; i < numCeilings; ++i) {
        ceiling_t* const pCeiling = gpActiveCeilings[i];

        if (pCeiling && (pCeiling->direction == 0) && ceilFilter(*pCeiling)) {  // Direction 0 = in stasis
            pCeiling->direction = pCeiling->olddirection;
            pCeiling->thinker.function = (think_t) &T_MoveCeiling;
            bUnpausedACrusher = true;
        }
    }

    return bUnpausedACrusher;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: functions to activate in-stasis ceilings matching a sector tag or sector (exposed via Lua scripting)
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_ActivateInStasisCeilingsForTag(const int32_t tag) noexcept {
    return P_ActivateMatchingInStasisCeilings([&](const ceiling_t& c) noexcept { return (c.tag == tag); });
}

bool P_ActivateInStasisCeilingForSector(const sector_t& sector) noexcept {
    return P_ActivateMatchingInStasisCeilings([&](const ceiling_t& c) noexcept { return (c.sector == &sector); });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpauses ceiling movers (crushers) which have the same tag as the given line and which are in stasis (paused).
// PsyDoom: this function has been rewritten. For the original version see the 'Old' code folder.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_ActivateInStasisCeiling(line_t& line) noexcept {
    P_ActivateInStasisCeilingsForTag(line.tag);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: pauses active ceiling movers (crushers) matching the specified condition
//------------------------------------------------------------------------------------------------------------------------------------------
template <class CeilFilterT>
static bool EV_MatchingCeilingCrushStop(const CeilFilterT& ceilFilter) noexcept {
    #if PSYDOOM_LIMIT_REMOVING
        const int32_t numCeilings = (int32_t) gpActiveCeilings.size();
    #else
        const int32_t numCeilings = MAXCEILINGS;
    #endif

    bool bPausedACrusher = false;

    for (int32_t i = 0; i < numCeilings; ++i) {
        ceiling_t* const pCeiling = gpActiveCeilings[i];

        if (pCeiling && (pCeiling->direction != 0) && ceilFilter(*pCeiling)) {
            pCeiling->olddirection = pCeiling->direction;   // Remember which direction it was moving in for unpause
            pCeiling->direction = 0;                        // Now in stasis
            pCeiling->thinker.function = nullptr;           // Remove the thinker function until unpaused
            bPausedACrusher = true;
        }
    }

    return bPausedACrusher;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: functions to stop ceilings matching a sector tag or sector (exposed via Lua scripting)
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_CeilingCrushStopForTag(const int32_t tag) noexcept {
    return EV_MatchingCeilingCrushStop([&](const ceiling_t& c) noexcept { return (c.tag == tag); }); 
}

bool EV_CeilingCrushStopForSector(const sector_t& sector) noexcept {
    return EV_MatchingCeilingCrushStop([&](const ceiling_t& c) noexcept { return (c.sector == &sector); });
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pauses active ceiling movers (crushers) with the same sector tag as the given line's tag.
// PsyDoom: this function has been rewritten. For the original version see the 'Old' code folder.
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_CeilingCrushStop(line_t& line) noexcept {
    return EV_CeilingCrushStopForTag(line.tag);
}
#endif  // #if PSYDOOM_MODS
