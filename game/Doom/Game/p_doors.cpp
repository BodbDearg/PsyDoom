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
#include "PsxVm/PsxVm.h"

// State for a door thinker
struct vldoor_t {
    thinker_t           thinker;        // Basic thinker fields
    vldoor_e            type;           // What type of door it is
    VmPtr<sector_t>     sector;         // Which sector is being moved
    fixed_t             topheight;      // Sector ceiling height when opened
    fixed_t             speed;          // Speed of door movement
    int32_t             direction;      // Current movement direction: 1 = up, 0 = opened, -1 = down
    int32_t             topwait;        // Door setting: total number of tics for the door to wait in the opened state
    int32_t             topcountdown;   // Door state: how many tics before the door starts closing
};

static_assert(sizeof(vldoor_t) == 40);

static constexpr fixed_t VDOORSPEED = FRACUNIT * 6;     // Regular speed of vertical doors
static constexpr int32_t VDOORWAIT  = 70;               // How long vertical doors normally wait before closing (game tics)

// TODO: eventually make these be actual C++ string constants.
// Can't to do that at the moment since these pointers need to be referenced by a 'VmPtr<T>', hence must be inside the executable itself.
static const VmPtr<const char> STR_BlueKeyNeededMsg(0x80010030);
static const VmPtr<const char> STR_YellowKeyNeededMsg(0x80010048);
static const VmPtr<const char> STR_RedKeyNeededMsg(0x80010060);

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a door: moves the door, does door state transitions and sounds etc.
// TODO: Make private to the module eventually.
//------------------------------------------------------------------------------------------------------------------------------------------
void T_VerticalDoor(vldoor_t& door) noexcept {
    // Which way is the door moving?
    switch (door.direction) {
        // Door is waiting
        case 0: {
            if (--door.topcountdown != 0)
                break;
            
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
            if (--door.topcountdown != 0)
                break;
            
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
    player_t& curPlayer = gPlayers[*gCurPlayerIndex];

    switch (line.special) {
        // Blue keycard and skull key
        case 26:
        case 32:
        case 99:
        case 133: {
            if ((!player.cards[it_bluecard]) && (!player.cards[it_blueskull])) {
                player.message = STR_BlueKeyNeededMsg;
                S_StartSound(&user, sfx_oof);

                if (&player == &curPlayer) {    // Only flash the HUD message if it's this player triggering the door
                    gStatusBar->tryopen[*gMapBlueKeyType] = true;
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
                player.message = STR_YellowKeyNeededMsg;
                S_StartSound(&user, sfx_oof);

                if (&player == &curPlayer) {    // Only flash the HUD message if it's this player triggering the door
                    gStatusBar->tryopen[*gMapYellowKeyType] = true;
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
                player.message = STR_RedKeyNeededMsg;
                S_StartSound(&user, sfx_oof);

                if (&player == &curPlayer) {    // Only flash the HUD message if it's this player triggering the door
                    gStatusBar->tryopen[*gMapRedKeyType] = true;
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
        sector_t& sector = gpSectors->get()[secIdx];

        if (sector.specialdata)
            continue;

        // Create the door thinker and populate its state/settings
        bActivatedADoor = true;

        vldoor_t& door = *(vldoor_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(vldoor_t), PU_LEVSPEC, nullptr);
        P_AddThinker(door.thinker);

        door.thinker.function = PsxVm::getNativeFuncVmAddr(T_VerticalDoor);
        door.type = doorType;
        door.sector = &sector;
        door.speed = VDOORSPEED;
        door.topwait = VDOORWAIT;

        // This thinker is now the special for the sector
        sector.specialdata = ptrToVmAddr(&door);

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
    sector_t& doorSector = *gpSides->get()[line.sidenum[1]].sector;
    vldoor_t* const pExistingDoor = (vldoor_t*) doorSector.specialdata.get();

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
    vldoor_t& newDoor = *(vldoor_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(vldoor_t), PU_LEVSPEC, nullptr);
    P_AddThinker(newDoor.thinker);
    doorSector.specialdata = &newDoor;

    // Default door config
    newDoor.thinker.function = PsxVm::getNativeFuncVmAddr(T_VerticalDoor);
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
    // Spawn the door thinker and link it to the sector
    vldoor_t& door = *(vldoor_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(vldoor_t), PU_LEVSPEC, nullptr);
    P_AddThinker(door.thinker);
    sector.specialdata = &door;
    sector.special = 0;

    // Configure door settings
    door.thinker.function = PsxVm::getNativeFuncVmAddr(T_VerticalDoor);
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
    // Spawn the door thinker and link it to the sector
    vldoor_t& door = *(vldoor_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(vldoor_t), PU_LEVSPEC, nullptr);
    P_AddThinker(door.thinker);
    sector.specialdata = &door;
    sector.special = 0;

    // Configure door settings
    door.thinker.function = PsxVm::getNativeFuncVmAddr(T_VerticalDoor);
    door.sector = &sector;
    door.direction = 2;         // Do initial wait
    door.type = RaiseIn5Mins;
    door.speed = VDOORSPEED;
    door.topheight = P_FindLowestCeilingSurrounding(sector) - 4 * FRACUNIT;
    door.topwait = VDOORWAIT;
    door.topcountdown = 5 * 60 * TICRATE;
}
