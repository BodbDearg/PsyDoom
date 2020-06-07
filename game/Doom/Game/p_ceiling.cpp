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

// Normal move speed for ceilings/crushers
static constexpr fixed_t CEILSPEED = FRACUNIT * 2;

// The list of currently active ceilings
ceiling_t* gpActiveCeilings[MAXCEILINGS];

// Not required externally: making private to this module
static void P_AddActiveCeiling(ceiling_t& ceiling) noexcept;
static void P_RemoveActiveCeiling(ceiling_t& ceiling) noexcept;
static void P_ActivateInStasisCeiling(line_t& line) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a moving ceiling or crusher: moves the ceiling, does state transitions and sounds etc.
// TODO: Make private to the module eventually.
//------------------------------------------------------------------------------------------------------------------------------------------
void T_MoveCeiling(ceiling_t& ceiling) noexcept {
    sector_t& ceilingSector = *ceiling.sector;

    switch (ceiling.direction) {
        // In stasis
        case 0:
            break;

        // Moving up
        case 1: {
            const result_e moveResult = T_MovePlane(ceilingSector, ceiling.speed, ceiling.topheight, false, 1, ceiling.direction);

            // Do moving sounds
            if ((gGameTic & 7) == 0) {
                switch (ceiling.type) {
                    case silentCrushAndRaise:
                        break;

                    default:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_stnmov);
                        break;
                }
            }

            // Reached the destination?
            if (moveResult == pastdest) {
                switch (ceiling.type) {
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

                    default:
                        break;
                }
            }
        }   break;

        // Moving down
        case -1: {
            const result_e moveResult = T_MovePlane(ceilingSector, ceiling.speed, ceiling.bottomheight, ceiling.crush, 1, ceiling.direction);

            if ((gGameTic & 7) == 0) {
                switch (ceiling.type) {
                    case silentCrushAndRaise:
                        break;

                    default:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_stnmov);
                        break;
                }
            }

            if (moveResult == pastdest) {
                // Reached the destination
                switch (ceiling.type) {
                    case silentCrushAndRaise:
                        S_StartSound((mobj_t*) &ceilingSector.soundorg, sfx_pstop);
                    case crushAndRaise:
                        ceiling.speed = CEILSPEED;
                    case fastCrushAndRaise:
                        ceiling.direction = 1;
                        break;

                    case lowerToFloor:
                    case lowerAndCrush:
                        P_RemoveActiveCeiling(ceiling);
                        break;
                        
                    default:
                        break;
                }
            }
            else if (moveResult == crushed) {
                // Crushing/hitting something
                switch (ceiling.type) {
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
            case lowerToFloor:
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
        }

        // Remember the ceiling type and sector tag, and add to the active ceilings list
        ceiling.type = ceilingType;
        ceiling.tag = sector.tag;
        P_AddActiveCeiling(ceiling);
    }

    return bActivatedACeiling;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add the given ceiling mover to a free slot in the 'active ceilings' list.
// Note: does NOT get added if there are no free slots.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_AddActiveCeiling(ceiling_t& ceiling) noexcept {
    for (int32_t i = 0; i < MAXCEILINGS; ++i) {
        if (!gpActiveCeilings[i]) {
            gpActiveCeilings[i] = &ceiling;
            return;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Remove the given ceiling from the active ceilings list; also dissociates it with it's sector and deallocs it's thinker
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_RemoveActiveCeiling(ceiling_t& ceiling) noexcept {
    for (int32_t i = 0; i < MAXCEILINGS; ++i) {
        if (gpActiveCeilings[i] == &ceiling) {
            ceiling.sector->specialdata = nullptr;
            P_RemoveThinker(ceiling.thinker);
            gpActiveCeilings[i] = nullptr;
            return;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpauses ceiling movers which have the same tag as the given line and which are in stasis (paused)
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_ActivateInStasisCeiling(line_t& line) noexcept {
    for (int32_t i = 0; i < MAXCEILINGS; ++i) {
        ceiling_t* const pCeiling = gpActiveCeilings[i];

        if (pCeiling && (pCeiling->tag == line.tag) && (pCeiling->direction == 0)) {    // Direction 0 = in stasis
            pCeiling->direction = pCeiling->olddirection;
            pCeiling->thinker.function = (think_t) &T_MoveCeiling;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pauses active ceiling movers (crushers) with the same sector tag as the given line's tag
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_CeilingCrushStop(line_t& line) noexcept {
    bool bPausedACrusher = false;

    for (int32_t i = 0; i < MAXCEILINGS; ++i) {
        ceiling_t* const pCeiling = gpActiveCeilings[i];

        if (pCeiling && (pCeiling->tag == line.tag) && (pCeiling->direction != 0)) {
            pCeiling->olddirection = pCeiling->direction;       // Remember which direction it was moving in for unpause
            pCeiling->direction = 0;                            // Now in stasis
            pCeiling->thinker.function = nullptr;               // Remove the thinker function until unpaused
            bPausedACrusher = true;
        }
    }

    return bPausedACrusher;
}
