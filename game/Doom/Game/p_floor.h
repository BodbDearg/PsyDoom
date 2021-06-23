#pragma once

#include "Doom/doomdef.h"

enum sfxenum_t : int32_t;
struct line_t;

// Represents the result of moving a floor/ceiling
enum result_e : int32_t {
    ok          = 0,    // Movement for the floor/ceiling was fully OK
    crushed     = 1,    // The floor/ceiling is crushing things and may not have moved because of this
    pastdest    = 2     // Floor/ceiling has reached its destination or very close to it (sometimes stops just before if crushing things)
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
    donutRaise              = 10,

    // PsyDoom: adding support for missing line specials from PC
    #if PSYDOOM_MODS
        raiseFloorTurbo = 11,
        raiseFloor512   = 12,
    #endif

    // PsyDoom: adding new a 'custom' floor type
    #if PSYDOOM_MODS
        customFloor = 13
    #endif
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

    #if PSYDOOM_MODS
        bool    bDoFinishScript;    // PsyDoom: if 'type' is 'customFloor' then this can be 'true' to execute a script action on finish
    #endif

    sector_t*   sector;             // The sector affected
    int32_t     direction;          // 1 = up, -1 = down
    int32_t     newspecial;         // For certain floor mover types, a special to assign to the sector when the movement is done
    int16_t     texture;            // For certain floor mover types, a texture to assign to the sector when the movement is done
    fixed_t     floordestheight;    // Destination height for the floor mover
    fixed_t     speed;              // Speed that the floor moves at

    // PsyDoom: new fields used by custom floors
    #if PSYDOOM_MODS
        sfxenum_t   moveSound;                  // Sound to make when moving ('sfx_None' if none)
        uint32_t    moveSoundFreq;              // How many tics between instances of the move sound playing
        sfxenum_t   stopSound;                  // Sound to make when stopping ('sfx_None' if none)
        int32_t     finishScriptActionNum;      // If enabled, the script action number to execute when the floor is done moving
        int32_t     finishScriptUserdata;       // A userdata field which will be sent along to the finish action when it executes
    #endif
};

// Standard speed for floors moving up and down
static constexpr fixed_t FLOORSPEED = FRACUNIT * 3;

// PsyDoom: definition for a custom floor mover.
// The constructor tries to construct with reasonable default settings.
#if PSYDOOM_MODS
    struct CustomFloorDef {
        CustomFloorDef() noexcept;

        bool        bCrush;                     // Is the floor crushing?
        bool        bDoFinishScript;            // Call the finish script action when completed moving?
        fixed_t     destHeight;                 // Destination height for the floor
        fixed_t     speed;                      // Speed that the floor moves at
        sfxenum_t   startSound;                 // Sound to make when starting ('sfx_None' if none)
        sfxenum_t   moveSound;                  // Sound to make when moving ('sfx_None' if none)
        uint32_t    moveSoundFreq;              // How many tics between instances of the move sound playing
        sfxenum_t   stopSound;                  // Sound to make when stopping ('sfx_None' if none)
        int32_t     finishScriptActionNum;      // If enabled, a script action to execute when the floor has come to a complete stop/finished
        int32_t     finishScriptUserdata;       // Userdata to pass to the 'finish' script action
    };
#endif

result_e T_MovePlane(
    sector_t& sector,
    fixed_t speed,
    const fixed_t destHeight,
    const bool bCrush,
    const int32_t floorOrCeiling,
    const int32_t direction
) noexcept;

void T_MoveFloor(floormove_t& floor) noexcept;
bool EV_DoFloor(line_t& line, const floor_e floorType) noexcept;
bool EV_BuildStairs(line_t& line, const stair_e stairType) noexcept;

#if PSYDOOM_MODS
    bool EV_DoCustomFloor(sector_t& sector, const CustomFloorDef& floorDef) noexcept;
#endif
