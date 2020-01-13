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

// The maximum 'radius' around sectors when determining blockmap extents
static constexpr fixed_t MAXRADIUS = 32 * FRACUNIT;
