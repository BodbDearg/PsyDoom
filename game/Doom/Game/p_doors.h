#pragma once

#include <cstdint>

struct line_t;
struct mobj_t;
struct sector_t;

// Enum representing a door type
enum vldoor_e : int32_t {
    Normal              = 0,
    Close30ThenOpen     = 1,
    Close               = 2,
    Open                = 3,
    RaiseIn5Mins        = 4,
    BlazeRaise          = 5,
    BlazeOpen           = 6,
    BlazeClose          = 7
};

bool P_CheckKeyLock(line_t& line, mobj_t& user) noexcept;
bool EV_DoDoor(line_t& line, const vldoor_e doorType) noexcept;
void EV_VerticalDoor(line_t& line, mobj_t& mobj) noexcept;
void P_SpawnDoorCloseIn30(sector_t& sector) noexcept;
void P_SpawnDoorRaiseIn5Mins(sector_t& sector, [[maybe_unused]] const int32_t secNum) noexcept;
