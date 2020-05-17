#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Various constants and settings specific to game code
//------------------------------------------------------------------------------------------------------------------------------------------

#include "Doom/doomdef.h"

// Blockmap related constants
static constexpr int32_t    MAPBLOCKUNITS   = 128;                          // Size of a blockmap square in integer units (TODO: CONFIRM)
static constexpr fixed_t    MAPBLOCKSIZE    = MAPBLOCKUNITS * FRACUNIT;     // Size of a blockmap square in 16.16 format
static constexpr uint32_t   MAPBLOCKSHIFT   = FRACBITS + 7;                 // How many bits to chop off to convert a 16.16 into blockmap units
static constexpr uint32_t   MAPBMASK        = MAPBLOCKSIZE - 1;             // A mask to restrict a 16.16 coordinate to 1 block

// The maximum 'radius' around sectors when determining blockmap extents.
// Determines how many blockmap blocks we should sweep in, when doing collision testing for a thing.
// Large enough most of the time but can cause issues sometimes, see: https://doomwiki.org/wiki/Flawed_collision_detection
static constexpr fixed_t MAXRADIUS = 32 * FRACUNIT;

// Special Z values to instruct the game to spawn a thing anchored to the floor or the ceiling
static constexpr fixed_t ONFLOORZ   = INT32_MIN;
static constexpr fixed_t ONCEILINGZ = INT32_MAX;

static constexpr int32_t    MAXHEALTH       = 100;              // Default maximum health for the player
static constexpr fixed_t    GRAVITY         = 4 * FRACUNIT;     // Gravity strength: same as Jaguar Doom
static constexpr fixed_t    MELEERANGE      = 70 * FRACUNIT;    // Range for melee attacks
static constexpr fixed_t    USERANGE        = 70 * FRACUNIT;    // Range for using things
static constexpr fixed_t    MAXMOVE         = 16 * FRACUNIT;    // Maximum movement amount in one go: larger moves must be split up unto multiple smaller moves
static constexpr fixed_t    FLOATSPEED      = 8 * FRACUNIT;     // Speed of floating up or down to meet the player for floating monsters
static constexpr int32_t    BASETHRESHOLD   = 100;              // How long for an AI to follow a target for (tics) - was about 3 seconds on PC, much longer on PSX. Conversion bug?

// Enum representing a movement direction (for AI actors)
enum dirtype_t : int32_t {
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
};
