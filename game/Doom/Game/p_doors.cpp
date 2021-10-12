#include "p_doors.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/UI/st_main.h"
#include "g_game.h"
#include "p_floor.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/ScriptingEngine.h"

#include <algorithm>

// State for a door thinker
struct vldoor_t {
    thinker_t   thinker;        // Basic thinker fields
    vldoor_e    type;           // What type of door it is
    sector_t*   sector;         // Which sector is being moved
    fixed_t     topheight;      // Sector ceiling height when opened
    fixed_t     speed;          // Speed of door movement
    int32_t     direction;      // Current movement direction: 1 = up, 0 = opened, -1 = down
    int32_t     topwait;        // Door setting: total number of tics for the door to wait in the opened state
    int32_t     topcountdown;   // Door state: how many tics before the door starts closing
};

// PsyDoom: state for a custom door thinker
#if PSYDOOM_MODS
    struct vlcustomdoor_t {
        thinker_t       thinker;            // Basic thinker fields
        sector_t*       sector;             // Which sector is being moved
        CustomDoorDef   def;                // Settings for the door
        int32_t         direction;          // Current direction of movement (1 = open, 0 = wait, -1 = close)
        int32_t         postWaitDirection;  // Direction that the door will go in after waiting
        int32_t         countdown;          // Current countdown (if waiting)
    };
#endif

static constexpr fixed_t VDOORSPEED = FRACUNIT * 6;     // Regular speed of vertical doors
static constexpr int32_t VDOORWAIT  = 70;               // How long vertical doors normally wait before closing (game tics)

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: default custom door settings
//------------------------------------------------------------------------------------------------------------------------------------------
CustomDoorDef::CustomDoorDef() noexcept
    : bOpen(true)
    , bDoReturn(true)
    , bBlockable(true)
    , bDoFinishScript(false)
    , minHeight(0)
    , maxHeight(0)
    , speed(VDOORSPEED)
    , waitTime(VDOORWAIT)
    , openSound(sfx_doropn)
    , closeSound(sfx_dorcls)
    , finishScriptActionNum(0)
    , finishScriptUserdata(0)
{
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a door: moves the door, does door state transitions and sounds etc.
//------------------------------------------------------------------------------------------------------------------------------------------
static void T_VerticalDoor(vldoor_t& door) noexcept {
    // Which way is the door moving?
    switch (door.direction) {
        // Door is waiting
        case 0: {
            // PsyDoom: wait period is halved in 'turbo' mode.
            // Also make the condition check a little more resilent and handle negative numbers.
            #if PSYDOOM_MODS
                const int32_t ticksToCountdown = (Game::gSettings.bTurboMode) ? 2 : 1;
                door.topcountdown = std::max(door.topcountdown - ticksToCountdown, 0);

                if (door.topcountdown > 0)
                    break;
            #else
                door.topcountdown--;

                if (door.topcountdown != 0)
                    break;
            #endif

            switch (door.type) {
                case BlazeRaise:
                    door.direction = -1;
                    S_StartSound((mobj_t*) &door.sector->soundorg, sfx_bdcls);
                    break;

                case Normal:
                    door.direction = -1;
                    S_StartSound((mobj_t*) &door.sector->soundorg, sfx_dorcls);
                    break;

                case Close30ThenOpen:
                    door.direction = 1;     // Opens after waiting, unlike the other doors (which normally close)
                    S_StartSound((mobj_t*) &door.sector->soundorg, sfx_doropn);
                    break;

                default:
                    break;
            }
        }   break;

        // Door is doing initial wait
        case 2: {
            // PsyDoom: wait period is halved in 'turbo' mode.
            // Also make the condition check a little more resilent and handle negative numbers.
            #if PSYDOOM_MODS
                const int32_t ticksToCountdown = (Game::gSettings.bTurboMode) ? 2 : 1;
                door.topcountdown = std::max(door.topcountdown - ticksToCountdown, 0);

                if (door.topcountdown > 0)
                    break;
            #else
                door.topcountdown--;

                if (door.topcountdown != 0)
                    break;
            #endif

            switch (door.type) {
                case RaiseIn5Mins: {
                    door.direction = 1;     // Open thyself!
                    door.type = Normal;
                    S_StartSound((mobj_t*) &door.sector->soundorg, sfx_doropn);
                }   break;

                default:
                    break;
            }
        }   break;

        // Door is closing (moving ceiling down)
        case -1: {
            const result_e planeMoveResult = T_MovePlane(*door.sector, door.speed, door.sector->floorheight, false, 1, door.direction);

            if (planeMoveResult == pastdest) {
                // Door has finished closing: for most door types we now deactivate and cleanup the door by removing it's thinker.
                // We also remove the special reference on the sector too.
                switch (door.type) {
                    case BlazeRaise:
                    case BlazeClose:
                        door.sector->specialdata = nullptr;
                        P_RemoveThinker(door.thinker);
                        S_StartSound((mobj_t*) &door.sector->soundorg, sfx_bdcls);
                        return;

                    case Normal:
                    case Close:
                        door.sector->specialdata = nullptr;
                        P_RemoveThinker(door.thinker);
                        break;

                    // This door is the exception and is only beginning it's work:
                    case Close30ThenOpen:
                        door.direction = 0;                 // Waiting to open
                        door.topcountdown = TICRATE * 30;   // Wait 30 seconds
                        break;

                    default:
                        break;
                }
            }
            else if (planeMoveResult == crushed) {
                // Door movement possibly impeded by things underneath it
                switch (door.type) {
                    // These door types don't care if they would crush things and won't go back up in this situation
                    case BlazeClose:
                    case Close:
                        break;

                    // For most normal doors when a thing is blocking it closing, go back up again
                    default:
                        door.direction = 1;
                        S_StartSound((mobj_t*) &door.sector->soundorg, sfx_doropn);
                        break;
                }
            }
        }   break;

        // Door is opening (moving ceiling up)
        case 1: {
            // Has the door fully opened?
            const result_e planeMoveResult = T_MovePlane(*door.sector, door.speed, door.topheight, 0, 1, door.direction);

            if (planeMoveResult != pastdest)
                break;

            // Door has fully opened: decide what to do
            switch (door.type) {
                // These doors will wait and close again after a while
                case Normal:
                case BlazeRaise:
                    door.direction = 0;                 // Wait for a bit
                    door.topcountdown = door.topwait;   // Duration to wait
                    break;

                // These doors stay open permanently so we can now remove the door thinker
                case Close30ThenOpen:
                case Open:
                case BlazeOpen:
                    door.sector->specialdata = nullptr;
                    P_RemoveThinker(door.thinker);
                    break;

                default:
                    break;
            }
        }   break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// New PSX function: check to see if a door being used is locked due to key lock constraints and flash a message to the player if it is.
// Returns 'false' if the door cannot be used by the given map object, or if the door does NOT have a key lock.
// Note: the function should not be called for doors that require no keys!
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_CheckKeyLock(line_t& line, mobj_t& user) noexcept {
    // Monsters can never open key doors
    if (!user.player)
        return false;

    player_t& player = *user.player;
    player_t& curPlayer = gPlayers[gCurPlayerIndex];

    switch (line.special) {
        // Blue keycard and skull key
        case 26:
        case 32:
        case 99:
        case 133: {
            if ((!player.cards[it_bluecard]) && (!player.cards[it_blueskull])) {
                player.message = "You need a blue key.";
                S_StartSound(&user, sfx_oof);

                // Only flash the HUD message if it's this player triggering the door
                if (&player == &curPlayer) {
                    gStatusBar.tryopen[gMapBlueKeyType] = true;
                }

                return false;
            }
        }   break;

        // Yellow keycard and skull key
        case 27:
        case 34:
        case 136:
        case 137: {
            if ((!player.cards[it_yellowcard]) && (!player.cards[it_yellowskull])) {
                player.message = "You need a yellow key.";
                S_StartSound(&user, sfx_oof);

                // Only flash the HUD message if it's this player triggering the door
                if (&player == &curPlayer) {
                    gStatusBar.tryopen[gMapYellowKeyType] = true;
                }

                return false;
            }
        }   break;

        // Red keycard and skull key
        case 28:
        case 33:
        case 134:
        case 135: {
            if ((!player.cards[it_redcard]) && (!player.cards[it_redskull])) {
                player.message = "You need a red key.";
                S_StartSound(&user, sfx_oof);

                // Only flash the HUD message if it's this player triggering the door
                if (&player == &curPlayer) {
                    gStatusBar.tryopen[gMapRedKeyType] = true;
                }

                return false;
            }
        }   break;

        default:
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// For each sector matching the given line's tag, spawn a door thinker/process of the given door type.
// Note that if a sector already has some special action going on, then a door thinker will NOT be added to the sector.
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoDoor(line_t& line, const vldoor_e doorType) noexcept {
    bool bActivatedADoor = false;

    // Create door thinkers for all sectors matching the given line tag
    for (int32_t secIdx = P_FindSectorFromLineTag(line, -1); secIdx >= 0; secIdx = P_FindSectorFromLineTag(line, secIdx)) {
        // Only spawn the door if there isn't already a special operating on this sector
        sector_t& sector = gpSectors[secIdx];

        if (sector.specialdata)
            continue;

        // Create the door thinker and populate its state/settings
        bActivatedADoor = true;
        vldoor_t& door = *(vldoor_t*) Z_Malloc(*gpMainMemZone, sizeof(vldoor_t), PU_LEVSPEC, nullptr);

        #if PSYDOOM_MODS
            door = {};      // PsyDoom: zero-init this struct for good measure
        #endif

        P_AddThinker(door.thinker);

        door.thinker.function = (think_t) &T_VerticalDoor;
        door.type = doorType;
        door.sector = &sector;
        door.speed = VDOORSPEED;
        door.topwait = VDOORWAIT;

        // This thinker is now the special for the sector
        sector.specialdata = &door;

        // Door specific setup and sounds
        switch (doorType) {
            case BlazeClose:
                door.topheight = P_FindLowestCeilingSurrounding(sector) -4 * FRACUNIT;
                door.direction = -1;
                door.speed = VDOORSPEED * 4;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_bdcls);
                break;

            case Close:
                door.topheight = P_FindLowestCeilingSurrounding(sector) -4 * FRACUNIT;
                door.direction = -1;
                S_StartSound((mobj_t*) &sector.soundorg, sfx_dorcls);
                break;

            case Close30ThenOpen:
                door.topheight = sector.ceilingheight;
                door.direction = -1;
                S_StartSound((mobj_t*) &door.sector->soundorg, sfx_dorcls);
                break;

            case BlazeRaise:
            case BlazeOpen: {
                door.topheight = P_FindLowestCeilingSurrounding(sector) -4 * FRACUNIT;
                door.direction = 1;
                door.speed = VDOORSPEED * 4;

                // Only play a sound if not already fully open
                if (door.topheight != sector.ceilingheight) {
                    S_StartSound((mobj_t*) &sector.soundorg, sfx_bdopn);
                }
            }   break;

            case Normal:
            case Open: {
                door.topheight = P_FindLowestCeilingSurrounding(sector) -4 * FRACUNIT;
                door.direction = 1;

                // Only play a sound if not already fully open
                if (door.topheight != sector.ceilingheight) {
                    S_StartSound((mobj_t*) &sector.soundorg, sfx_doropn);
                }
            }   break;

            default:
                break;
        }
    }

    return bActivatedADoor;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called to trigger a door directly via a line special on one of it's sector's lines (without a tag reference or via a switch).
// This is the most common way of triggering a door.
//------------------------------------------------------------------------------------------------------------------------------------------
void EV_VerticalDoor(line_t& line, mobj_t& user) noexcept {
    // Try to activate an already existing door thinker if the sector already has that
    sector_t& doorSector = *gpSides[line.sidenum[1]].sector;

    #if PSYDOOM_MODS
        // PsyDoom: fixing a dangerous cast here.
        // The special operating on the sector might not be a regular door, it could be a custom door or something else.
        // Detect that scenario here by checking the thinker function, and no-op if current thinker is incompatible:
        thinker_t* const pThinker = (thinker_t*) doorSector.specialdata;
        const bool bIsNormalDoorThinker = (pThinker && (pThinker->function == (think_t) &T_VerticalDoor));

        if (pThinker && (!bIsNormalDoorThinker))
            return;

        vldoor_t* const pExistingDoor = (vldoor_t*) pThinker;
    #else
        vldoor_t* const pExistingDoor = (vldoor_t*) doorSector.specialdata;
    #endif

    if (pExistingDoor) {
        switch (line.special) {
            case 1:     // Only for "RAISE" type doors not "OPEN" (permanent) type doors
            case 26:
            case 27:
            case 28:
            case 117: {
                // Make the door go back up if going down
                if (pExistingDoor->direction == -1) {
                    pExistingDoor->direction = 1;
                    return;
                } 
                else {
                    // Only players can close doors if the door is going up or waiting
                    if (user.player) {
                        pExistingDoor->direction = -1;
                    }
                }

                return;     // Don't make a new door thinker or sound
            }
        }
    }

    // Play the door sound
    switch (line.special) {
        case 1:     // Normal door sound
        case 31:
            S_StartSound((mobj_t*) &doorSector.soundorg, sfx_doropn);
            break;

        case 117:   // Blazing door raise
        case 118:   // Blazing door open
            S_StartSound((mobj_t*) &doorSector.soundorg, sfx_bdopn);
            break;

        default:    // All other door tyoes
            S_StartSound((mobj_t*) &doorSector.soundorg, sfx_doropn);
            break;
    }

    // Need to create a new door thinker to run the door logic: create and set as the sector special
    vldoor_t& newDoor = *(vldoor_t*) Z_Malloc(*gpMainMemZone, sizeof(vldoor_t), PU_LEVSPEC, nullptr);

    #if PSYDOOM_MODS
        newDoor = {};   // PsyDoom: zero-init this struct for good measure
    #endif

    P_AddThinker(newDoor.thinker);
    doorSector.specialdata = &newDoor;

    // Default door config
    newDoor.thinker.function = (think_t) &T_VerticalDoor;
    newDoor.speed = VDOORSPEED;
    newDoor.sector = &doorSector;
    newDoor.direction = 1;
    newDoor.topwait = VDOORWAIT;

    // Door specific config
    switch (line.special) {
        case 1:
        case 26:
        case 27:
        case 28:
            newDoor.type = Normal;
            break;

        case 31:
        case 32:
        case 33:
        case 34:
            newDoor.type = Open;
            line.special = 0;
            break;

        case 117:   // Blazing door raise 
            newDoor.type = BlazeRaise;
            newDoor.speed = VDOORSPEED * 4;
            break;

        case 118:   // Blazing door open
            newDoor.type = BlazeOpen;
            newDoor.speed =  VDOORSPEED * 4;
            line.special = 0;
            break;
    }

    // Figure out where the door stops and create a slight lip when fully open
    newDoor.topheight = P_FindLowestCeilingSurrounding(doorSector);
    newDoor.topheight -= 4 * FRACUNIT;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a door thinker that closes after 30 seconds
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnDoorCloseIn30(sector_t& sector) noexcept {
    // PsyDoom: if the sector already has a special active then we shouldn't clobber the existing one, abort if that is the case:
    #if PSYDOOM_MODS
        if (sector.specialdata)
            return;
    #endif

    // Spawn the door thinker and link it to the sector
    vldoor_t& door = *(vldoor_t*) Z_Malloc(*gpMainMemZone, sizeof(vldoor_t), PU_LEVSPEC, nullptr);
    P_AddThinker(door.thinker);
    sector.specialdata = &door;
    sector.special = 0;

    // Configure door settings
    door.thinker.function = (think_t) &T_VerticalDoor;
    door.sector = &sector;
    door.direction = 0;
    door.type = Normal;
    door.speed = VDOORSPEED;
    door.topcountdown = 30 * TICRATE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a door thinker that opens after 5 minutes
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnDoorRaiseIn5Mins(sector_t& sector, [[maybe_unused]] const int32_t secNum) noexcept {
    // PsyDoom: if the sector already has a special active then we shouldn't clobber the existing one, abort if that is the case:
    #if PSYDOOM_MODS
        if (sector.specialdata)
            return;
    #endif

    // Spawn the door thinker and link it to the sector
    vldoor_t& door = *(vldoor_t*) Z_Malloc(*gpMainMemZone, sizeof(vldoor_t), PU_LEVSPEC, nullptr);
    P_AddThinker(door.thinker);
    sector.specialdata = &door;
    sector.special = 0;

    // Configure door settings
    door.thinker.function = (think_t) &T_VerticalDoor;
    door.sector = &sector;
    door.direction = 2;         // Do initial wait
    door.type = RaiseIn5Mins;
    door.speed = VDOORSPEED;
    door.topheight = P_FindLowestCeilingSurrounding(sector) - 4 * FRACUNIT;
    door.topwait = VDOORWAIT;
    door.topcountdown = 5 * 60 * TICRATE;
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: helper, plays a custom door sound if it's valid
//------------------------------------------------------------------------------------------------------------------------------------------
static void PlayCustomDoorSound(const sfxenum_t sound, sector_t& atSector) noexcept {
    if (sound != sfx_None) {
        S_StartSound((mobj_t*) &atSector.soundorg, sound);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: thinker/update logic for a custom door: moves the door, does door state transitions and sounds etc.
//------------------------------------------------------------------------------------------------------------------------------------------
static void T_CustomDoor(vlcustomdoor_t& door) noexcept {
    sector_t& sector = *door.sector;

    // Handle a waiting door and tick the wait
    if (door.direction == 0) {
        const int32_t ticksToCountdown = (Game::gSettings.bTurboMode) ? 2 : 1;
        door.countdown = std::max(door.countdown - ticksToCountdown, 0);

        // Play the sound for the door opening or closing after the wait, then go into the next state
        if (door.countdown <= 0) {
            const bool bIsOpening = (door.postWaitDirection >= 0);
            PlayCustomDoorSound(bIsOpening ? door.def.openSound : door.def.closeSound, sector);
            door.direction = door.postWaitDirection;
        }

        return;
    }

    // Door is opening or closing, move it
    const bool bIsClosing = (door.direction < 0);
    const fixed_t destHeight = bIsClosing ? door.def.minHeight : door.def.maxHeight;
    const result_e planeMoveResult = T_MovePlane(sector, door.def.speed, destHeight, false, 1, door.direction);

    if (planeMoveResult == pastdest) {
        // Door has reached it's destination, 3 possibilities here:
        //  (1) The door is finished
        //  (2) The door waits, then returns
        //  (3) The door immediately returns
        const bool bDoorEndsOpen = door.def.bOpen ^ door.def.bDoReturn;
        const bool bDoorEndsClosed = (!bDoorEndsOpen);
        const bool bIsEndMove = (bIsClosing == bDoorEndsClosed);

        if (bIsEndMove) {
            // (1) The door is finished - cleanup the thinker
            P_RemoveThinker(door.thinker);
            sector.specialdata = nullptr;

            // Call the 'finish' script action if one is defined
            if (door.def.bDoFinishScript) {
                ScriptingEngine::doAction(door.def.finishScriptActionNum, nullptr, &sector, nullptr, 0, door.def.finishScriptUserdata);
            }
        }
        else {
            // Door is either going to wait or return immediately
            if (door.def.waitTime > 0) {
                // (2) The door waits, then returns
                door.postWaitDirection = -door.direction;
                door.direction = 0;
                door.countdown = door.def.waitTime;
            } else {
                // (3) The door immediately returns
                door.direction = -door.direction;
                PlayCustomDoorSound(bIsClosing ? door.def.openSound : door.def.closeSound, sector);
            }
        }
    }
    else if (planeMoveResult == crushed) {
        // If the door is being blocked by a thing then move it back up (if that's allowed)
        if (door.def.bBlockable) {
            door.direction = 1;
            PlayCustomDoorSound(door.def.openSound, sector);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: spawn a custom door thinker for the specified sector.
// Returns 'true' if successful, otherwise 'false' if there is already a special operating on the sector.
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoCustomDoor(sector_t& sector, const CustomDoorDef& def) noexcept {
    // Can't do this special if the sector already has one!
    if (sector.specialdata)
        return false;

    // Alloc the door, zero-init and set it up as a thinker for the sector
    vlcustomdoor_t& door = *(vlcustomdoor_t*) Z_Malloc(*gpMainMemZone, sizeof(vlcustomdoor_t), PU_LEVSPEC, nullptr);
    door = {};
    P_AddThinker(door.thinker);
    door.thinker.function = (think_t) &T_CustomDoor;
    door.sector = &sector;
    sector.specialdata = &door;

    // Save the door settings and setup the initial move direction
    door.def = def;
    door.direction = (def.bOpen) ? 1 : -1;

    // Sanitize some of the script inputs
    door.def.speed = std::abs(door.def.speed);
    door.def.waitTime = std::max(door.def.waitTime, 0);

    // Play the open or close sound and return 'true' for success
    PlayCustomDoorSound(door.def.bOpen ? door.def.openSound : door.def.closeSound, sector);
    return true;
}
#endif  // #if PSYDOOM_MODS
