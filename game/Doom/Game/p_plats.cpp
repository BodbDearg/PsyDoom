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

#include <algorithm>

static constexpr int32_t PLATWAIT   = 3;                // Number of seconds for platforms to be in the waiting state
static constexpr int32_t PLATSPEED  = 2 * FRACUNIT;     // Standard platform speed (some platforms are slower or faster)

// Contains all of the active platforms in the level (some slots may be empty)
plat_t* gpActivePlats[MAXPLATS];

// Not required externally: making private to this module
static void P_AddActivePlat(plat_t& plat) noexcept;
static void P_RemoveActivePlat(plat_t& plat) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a moving platform: moves the platform, does state transitions and sounds etc.
//------------------------------------------------------------------------------------------------------------------------------------------
static void T_PlatRaise(plat_t& plat) noexcept {
    sector_t& sector = *plat.sector;

    switch (plat.status) {
        // Going up
        case up: {
            const result_e moveResult = T_MovePlane(sector, plat.speed, plat.high, plat.crush, false, 1);

            // Do movement sounds every so often for certain platform types
            if ((plat.type == raiseAndChange) || (plat.type == raiseToNearestAndChange)) {
                if ((gGameTic & 7U) == 0) {
                    S_StartSound((mobj_t*) &sector.soundorg, sfx_stnmov);
                }
            }

            // Decide what to do base on the move result and platform settings
            if ((moveResult == crushed) && (!plat.crush)) {
                // Crushing something and this platform doesn't crush: change direction
                plat.status = down;
                plat.count = plat.wait;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstart);
            }
            else if (moveResult == pastdest) {
                // Reached the destination for the platform: wait by default (overrides below) and play the stopped sound
                plat.status = waiting;
                plat.count = plat.wait;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstop);

                // Certain platform types stop now at this point
                switch (plat.type) {
                    case downWaitUpStay:
                    case raiseAndChange:
                    case blazeDWUS:
                        P_RemoveActivePlat(plat);
                        break;
                        
                    default:
                        break;
                }
            }
        }   break;

        // Going down
        case down: {
            const result_e moveResult = T_MovePlane(sector, plat.speed, plat.low, false, 0, -1);
            
            // Time to start waiting before going back up again?
            if (moveResult == pastdest) {
                plat.status = waiting;
                plat.count = plat.wait;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstop);
            }
        }   break;

        // Waiting to go back up (or down) again?
        case waiting: {
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
                S_StartSound((mobj_t*) &sector.soundorg, sfx_pstart);
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
        case perpetualRaise:
            P_ActivateInStasis(line.tag);
            break;

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

//------------------------------------------------------------------------------------------------------------------------------------------
// Reactivates moving platforms that were paused which match the given sector tag
//------------------------------------------------------------------------------------------------------------------------------------------
void P_ActivateInStasis(const int32_t tag) noexcept {
    for (int32_t i = 0; i < MAXPLATS; ++i) {
        plat_t* pPlat = gpActivePlats[i];

        if (pPlat && (pPlat->tag == tag) && (pPlat->status == in_stasis)) {
            pPlat->status = pPlat->oldstatus;
            pPlat->thinker.function = (think_t) &T_PlatRaise;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stops moving platforms that are moving that have a sector tag matching the given line's tag
//------------------------------------------------------------------------------------------------------------------------------------------
void EV_StopPlat(line_t& line) noexcept {
    for (int32_t i = 0; i < MAXPLATS; ++i) {
        plat_t* const pPlat = gpActivePlats[i];

        if (pPlat && (pPlat->status != in_stasis) && (pPlat->tag == line.tag)) {
            // Stop this moving platform: remember the status before stopping and put into stasis
            pPlat->oldstatus = pPlat->status;
            pPlat->status = in_stasis;
            pPlat->thinker.function = nullptr;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add the given moving platform to a free slot in the 'active platforms' list
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_AddActivePlat(plat_t& plat) noexcept {
    for (int32_t i = 0; i < MAXPLATS; ++i) {
        if (!gpActivePlats[i]) {
            gpActivePlats[i] = &plat;
            return;
        }
    }

    I_Error("P_AddActivePlat: no more plats!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Remove the given moving platform from the active platforms list; also dissociates it with it's sector and deallocs it's thinker
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_RemoveActivePlat(plat_t& plat) noexcept {
    for (int32_t i = 0; i < MAXPLATS; ++i) {
        if (gpActivePlats[i] == &plat) {
            plat.sector->specialdata = nullptr;
            P_RemoveThinker(plat.thinker);
            gpActivePlats[i] = nullptr;
            return;
        }
    }

    I_Error("P_RemoveActivePlat: can\'t find plat!");
}
