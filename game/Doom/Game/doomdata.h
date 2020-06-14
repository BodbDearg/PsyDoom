#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Game data structures which are read from WAD files
//------------------------------------------------------------------------------------------------------------------------------------------

#include "Doom/Base/z_zone.h"
#include "Doom/doomdef.h"

// Map lump offsets, relative to the 'MAPXX' marker
enum : int32_t {
    ML_LABEL,           // The 'MAPXX' marker lump
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

// Linedef flags
static constexpr int16_t ML_BLOCKING        = 0x1;      // The line blocks all movement
static constexpr int16_t ML_BLOCKMONSTERS   = 0x2;      // The line blocks monsters
static constexpr int16_t ML_TWOSIDED        = 0x4;      // Unset for single sided lines
static constexpr int16_t ML_DONTPEGTOP      = 0x8;      // If unset then upper texture is anchored to the ceiling rather than bottom edge
static constexpr int16_t ML_DONTPEGBOTTOM   = 0x10;     // If unset then lower texture is anchored to the floor rather than top edge
static constexpr int16_t ML_SECRET          = 0x20;     // Don't show as two sided in the automap, because it's a secret
static constexpr int16_t ML_SOUNDBLOCK      = 0x40;     // Stops sound propagation
static constexpr int16_t ML_DONTDRAW        = 0x80;     // Hide on the automap
static constexpr int16_t ML_MAPPED          = 0x100;    // Set when the line is to be shown on the automap
static constexpr int16_t ML_MIDMASKED       = 0x200;    // PSX DOOM: Middle texture has translucent or alpha blended pixels
static constexpr int16_t ML_MIDTRANSLUCENT  = 0x400;    // PSX DOOM: Middle texture drawn with alpha blending
static constexpr int16_t ML_BLOCKPRJECTILE  = 0x800;    // PSX DOOM: Line stops projectiles

// Map thing flags
static constexpr int16_t MTF_EASY               = 0x1;      // Difficulty flags, determines which difficulties the thing appears at
static constexpr int16_t MTF_NORMAL             = 0x2;
static constexpr int16_t MTF_HARD               = 0x4;
static constexpr int16_t MTF_AMBUSH             = 0x8;      // Monster flag: activate only on sight, ignore noise
static constexpr int16_t MTF_DEATHMATCH         = 0x10;     // Un-named flag in all DOOM versions that I'm giving a name: thing is for deathmatch only
static constexpr int16_t MTF_BLEND_ON           = 0x20;     // PSX DOOM: if set then blending is enabled for the object (alpha, additive or subtractive)
static constexpr int16_t MTF_BLEND_MODE_BIT1    = 0x40;     // PSX DOOM: 1 of 2 bits determining blend mode if blending is enabled. See 'MF_BLEND' mobj flags for more details.
static constexpr int16_t MTF_BLEND_MODE_BIT2    = 0x80;     // PSX DOOM: 1 of 2 bits determining blend mode if blending is enabled. See 'MF_BLEND' mobj flags for more details.

// If this flag is set for a BSP node child in a wad then it means the child is a subsector.
// This flag should be removed when retrieving the actual subsector number.
static constexpr uint32_t NF_SUBSECTOR = 0x8000;

// Header for a block of memory in a memory blocks file.
// The data for the block immediately follows this header in the blocks file.
//
// This struct was deliberately the same size as the original PSX 'memblock_t' so it could be repurposed as that later when
// the memory block was loaded into RAM. Of course with 64-bit pointers this simple reinterpretation is no longer possible,
// since all would-be pointer fields in this struct are 32-bits in size.
// 
struct fileblock_t {
    int32_t     size;               // Size of the block including the header
    uint32_t    _unused1;           // Unused field...
    int16_t     tag;                // Purge tags
    int16_t     id;                 // Should be ZONEID
    uint16_t    lumpNum;            // Which lump mumber this block of data is for
    uint16_t    isUncompressed;     // 0 = compressed, 1 = uncompressed, all other values are not acceptable
    uint32_t    _unused3;           // Unused field...
    uint32_t    _unused4;           // Unused field...
};

static_assert(sizeof(fileblock_t) == 24);

// A map vertex in a WAD. Note that unlike the PC version the coordinates here are in 16.16 format - presumably to help fight precision issues?
// The PSX renderer is probably more prone to cracks and other artifacts etc. from BSP splits due to the way so I guess this makes sense.
struct mapvertex_t {
    fixed_t x;
    fixed_t y;
};

static_assert(sizeof(mapvertex_t) == 8);

// Map data for a sectors, sides, lines, subsectors, nodes and line segments in a WAD
struct mapsector_t {
    int16_t     floorheight;        // Integer floor height for the sector
    int16_t     ceilingheight;      // Integer ceiling height for the sector
    char        floorpic[8];        // Floor texture lump name
    char        ceilingpic[8];      // Ceiling texture lump name
    uint8_t     lightlevel;         // Light level for the sector (normally 0-255)
    uint8_t     colorid;            // Which of the sector light colors to use for the sector
    int16_t     special;            // Special action for the sector: damage, secret, light flicker etc.
    int16_t     tag;                // Tag for the sector for use in targetted actions (triggered by switches, line crossings etc.)
    uint16_t    flags;              // Affects sound fx (TODO: figure out what this means)
};

static_assert(sizeof(mapsector_t) == 28);

struct mapsidedef_t {
    int16_t     textureoffset;          // Texture x offset
    int16_t     rowoffset;              // Texture y offset
    char        toptexture[8];          // Upper texture lump name
    char        bottomtexture[8];       // Lower texture lump name
    char        midtexture[8];          // Mid or wall texture lump name
    int16_t     sector;                 // Which sector (by index) the side belongs to
};

static_assert(sizeof(mapsidedef_t) == 30);

struct maplinedef_t {
    int16_t     vertex1;        // Index of the 1st vertex in the line
    int16_t     vertex2;        // Index of the 2nd vertex in the line
    int16_t     flags;          // A combination of 'ML_XXX' line flags
    int16_t     special;        // Line special action (switch, trigger etc.)
    int16_t     tag;            // Target for the line special action (if applicable)
    int16_t     sidenum[2];     // If -1 then the line is 1 sided
};

static_assert(sizeof(maplinedef_t) == 14);

struct mapsubsector_t {
    int16_t     numsegs;        // How many segs this subsector has
    int16_t     firstseg;       // Index of the first seg this subsector has (all are stored sequentially)
};

static_assert(sizeof(mapsubsector_t) == 4);

struct mapnode_t {
    int16_t     x;              // The partition line: 1st point in integer coords (x & y) 
    int16_t     y;
    int16_t     dx;             // The partition line: vector from the 1st point to the end point in integer coords (x & y)
    int16_t     dy;
    int16_t     bbox[2][4];     // Bounding box for both child nodes
    uint16_t    children[2];    // When 'NF_SUBSECTOR' is set then it means it's a subsector number
};

static_assert(sizeof(mapnode_t) == 28);

struct mapseg_t {
    int16_t     vertex1;        // Index of the 1st vertex in the line segment
    int16_t     vertex2;        // Index of the 2nd vertex in the line segment
    int16_t     angle;          // Precomputed angle for the line segment direction
    int16_t     linedef;        // Index of the line that the segment belongs to
    int16_t     side;           // '0' or '1': which side of the line the seg is on. Always '0' for one sided lines.
    int16_t     offset;         // Horizontal offset for the line segment's texture
};

static_assert(sizeof(mapseg_t) == 12);

// New to PSX: header for a leaf
struct mapleaf_t {
    uint16_t    numedges;       // How many edges in this leaf
};

static_assert(sizeof(mapleaf_t) == 2);

// New to PSX: definition for an edge in a leaf
struct mapleafedge_t {
    int16_t     vertexnum;      // 1st vertex in the edge; 2nd vertex is found in the following leaf edge
    int16_t     segnum;         // Index of the seg the leaf edge belongs to
};

static_assert(sizeof(mapleafedge_t) == 4);

// Describes the type, position, angle and flags of a thing in a WAD
struct mapthing_t {
    int16_t     x;          // Integer position: x
    int16_t     y;          // Integer position: y
    int16_t     angle;      // Starting orientation from 0-359 degrees
    int16_t     type;       // DoomEd number for the thing
    int16_t     options;    // MTF_XXX map thing flags
};

static_assert(sizeof(mapthing_t) == 10);

// Metadata for a texture in the TEXTURE1 lump
struct maptexture_t {
    int16_t     offsetX;    // Used for anchoring/offsetting in some UIs
    int16_t     offsetY;    // Used for anchoring/offsetting in some UIs
    int16_t     width;      // Texture width in pixels
    int16_t     height;     // Texture height in pixels
};

static_assert(sizeof(maptexture_t) == 8);
