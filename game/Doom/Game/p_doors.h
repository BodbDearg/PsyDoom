#pragma once

#include "Doom/doomdef.h"

struct line_t;

// Enum representing a door type
enum vldoor_e : uint32_t {
    Normal              = 0,
    Close30ThenOpen     = 1,
    Close               = 2,
    Open                = 3,
    RaiseIn5Mins        = 4,
    BlazeRaise          = 5,
    BlazeOpen           = 6,
    BlazeClose          = 7
};

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

bool P_CheckKeyLock(line_t& line, mobj_t& user) noexcept;
bool EV_DoDoor(line_t& line, const vldoor_e doorType) noexcept;
void EV_VerticalDoor(line_t& line, mobj_t& mobj) noexcept;
void P_SpawnDoorCloseIn30(sector_t& sector) noexcept;
void P_SpawnDoorRaiseIn5Mins(sector_t& sector, [[maybe_unused]] const int32_t secNum) noexcept;
