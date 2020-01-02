#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Game data structures which are read from files
//------------------------------------------------------------------------------------------------------------------------------------------

#include "Doom/Base/z_zone.h"
#include "Doom/doomdef.h"

// Map lump offsets, relative to the 'MAPXX' marker
enum : uint32_t {
    ML_LABEL,       // The 'MAPXX' marker lump
    ML_THINGS,
    ML_LINEDEFS,
    ML_SIDEDEFS,
    ML_VERTEXES,
    ML_SEGS,
    ML_SSECTORS,
    ML_NODES,
    ML_SECTORS,
    ML_REJECT,
    ML_BLOCKMAP,
    ML_LEAFS
};

// Header for a block of memory in a memory blocks file.
// Deliberately the same size as 'memblock_t' so it can be repurposed as that later when the block is loaded into RAM.
// The data for the block immediately follows this header in the blocks file.
struct fileblock_t {
    int32_t     size;               // Size of the block including the header
    uint32_t    _unused1;
    int16_t     tag;                // Purge tags
    int16_t     id;                 // Should be ZONEID
    uint16_t    lumpNum;            // Which lump mumber this block of data is for
    uint16_t    isUncompressed;     // 0 = compressed, 1 = uncompressed, all other values are not acceptable
    uint32_t    _unused3;
    uint32_t    _unused4;
};

static_assert(sizeof(fileblock_t) == sizeof(memblock_t));

// A map vertex in a WAD. Note that unlike the PC version the coordinates here are in 16.16 format - presumably to help fight precision issues?
// The PSX renderer is probably more prone to cracks and other artifacts etc. from BSP splits due to the way so I guess this makes sense.
struct mapvertex_t {
    fixed_t x;
    fixed_t y;
};

static_assert(sizeof(mapvertex_t) == 8);

// Map data for a sector in a WAD
struct mapsector_t {
    int16_t     floorheight;
    int16_t     ceilingheight;
    char        floorpic[8];
    char        ceilingpic[8];
    uint8_t     lightlevel;
    uint8_t     colorid;            // TODO: Comment
    int16_t     special;
    int16_t     tag;
    uint16_t    flags;              // Affects sound fx (TODO: figure out what this means)
};

static_assert(sizeof(mapsector_t) == 28);
