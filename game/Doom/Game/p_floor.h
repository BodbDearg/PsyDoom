#pragma once

#include "Doom/doomdef.h"

struct line_t;

// Represents the result of moving a floor/ceiling
enum result_e : int32_t {
    ok          = 0,    // Movement for the floor/ceiling was fully OK
    crushed     = 1,    // The floor/ceiling is crushing things and may not have moved because of this
    pastdest    = 2     // Plane has reached its destination or very close to it (sometimes stops just before if crushing things)
};

// Enum for a moving floor type
enum floor_e : int32_t {
    lowerFloor              = 0,    // Lower floor to highest surrounding floor
    lowerFloorToLowest      = 1,    // Lower floor to lowest surrounding floor
    turboLower              = 2,    // Lower floor to highest surrounding floor VERY FAST
    raiseFloor              = 3,    // Raise floor to lowest surrounding CEILING
    raiseFloorToNearest     = 4,    // Raise floor to next highest surrounding floor
    raiseToTexture          = 5,    // Raise floor to shortest height texture around it
    lowerAndChange          = 6,    // Lower floor to lowest surrounding floor and change the floorpic
    raiseFloor24            = 7,
    raiseFloor24AndChange   = 8,
    raiseFloorCrush         = 9,
    donutRaise              = 10
};

// What type of stair building to do when building stairs
enum stair_e : int32_t {
    build8,     // Slowly build by 8 units
    turbo16     // Quickly build by 16 units
};

// Holds the state and settings for a moving floor
struct floormove_t {
    thinker_t   thinker;            // Basic thinker properties
    floor_e     type;               // What type of behavior the floor mover has
    bool        crush;              // Does the floor movement cause crushing when things don't fit?
    sector_t*   sector;             // The sector affected
    int32_t     direction;          // 1 = up, -1 = down
    int32_t     newspecial;         // For certain floor mover types, a special to assign to the sector when the movement is done
    int16_t     texture;            // For certain floor mover types, a texture to assign to the sector when the movement is done
    fixed_t     floordestheight;    // Destination height for the floor mover
    fixed_t     speed;              // Speed that the floor moves at
};

// Standard speed for floors moving up and down
static constexpr fixed_t FLOORSPEED = FRACUNIT * 3;

result_e T_MovePlane(
    sector_t& sector,
    const fixed_t speed,
    const fixed_t destHeight,
    const bool bCrush,
    const int32_t floorOrCeiling,
    const int32_t direction
) noexcept;

void T_MoveFloor(floormove_t& floor) noexcept;
bool EV_DoFloor(line_t& line, const floor_e floorType) noexcept;
bool EV_BuildStairs(line_t& line, const stair_e stairType) noexcept;
