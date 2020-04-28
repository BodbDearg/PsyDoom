#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Game data structures which are read from WAD files
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

// Linedef flags
static constexpr int16_t ML_BLOCKING        = 0x1;
static constexpr int16_t ML_BLOCKMONSTERS   = 0x2;
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
static constexpr int16_t MTF_EASY           = 0x1;      // Difficulty flags, determines which difficulties the thing appears at
static constexpr int16_t MTF_NORMAL         = 0x2;
static constexpr int16_t MTF_HARD           = 0x4;
static constexpr int16_t MTF_AMBUSH         = 0x8;      // Monster flag: activate only on sight, ignore noise
static constexpr int16_t MTF_DEATHMATCH     = 0x10;     // Un-named flag in all DOOM versions that I'm giving a name: thing is for deathmatch only
static constexpr int16_t MTF_BLENDMASK1     = 0x20;     // PSX DOOM: TODO: comment/explain
static constexpr int16_t MTF_BLENDMASK2     = 0x40;     // PSX DOOM: TODO: comment/explain
static constexpr int16_t MTF_BLENDMASK3     = 0x80;     // PSX DOOM: TODO: comment/explain

// If this flag is set for a BSP node child in a wad then it means the child is a subsector.
// This flag should be removed when retrieving the actual subsector number.
static constexpr uint32_t NF_SUBSECTOR = 0x8000;

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

// Map data for a sectors, sides, lines, subsectors, nodes and line segments in a WAD
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

struct mapsidedef_t {
    int16_t     textureoffset;          // Texture x offset
    int16_t     rowoffset;              // Texture y offset
    char        toptexture[8];
    char        bottomtexture[8];
    char        midtexture[8];
    int16_t     sector;
};

static_assert(sizeof(mapsidedef_t) == 30);

struct maplinedef_t {
    int16_t     vertex1;
    int16_t     vertex2;
    int16_t     flags;          // A combination of 'ML_XXX' line flags
    int16_t     special;
    int16_t     tag;
    int16_t     sidenum[2];     // If -1 then the line is 1 sided
};

static_assert(sizeof(maplinedef_t) == 14);

struct mapsubsector_t {
    int16_t     numsegs;        // How many segs this subsector has
    int16_t     firstseg;       // Index of the first seg this subsector has (all are stored sequentially)
};

static_assert(sizeof(mapsubsector_t) == 4);

struct mapnode_t {
    int16_t     x;              // The partition line
    int16_t     y;
    int16_t     dx;
    int16_t     dy;
    int16_t     bbox[2][4];     // Bounding box for both child nodes
    uint16_t    children[2];    // When 'NF_SUBSECTOR' is set then it means it's a subsector number
};

static_assert(sizeof(mapnode_t) == 28);

struct mapseg_t {
    int16_t     vertex1;
    int16_t     vertex2;
    int16_t     angle;
    int16_t     linedef;
    int16_t     side;           // '0' or '1': which side of the line the seg is on. Always '0' for one sided lines.
    int16_t     offset;         // TODO: comment
};

static_assert(sizeof(mapseg_t) == 12);

// Header for a leaf and leaf edges
struct mapleaf_t {
    uint16_t    numedges;
};

static_assert(sizeof(mapleaf_t) == 2);

struct mapleafedge_t {
    int16_t     vertexnum;
    int16_t     segnum;
};

static_assert(sizeof(mapleafedge_t) == 4);

// Describes the type, position, angle and flags of a thing in a WAD
struct mapthing_t {
    int16_t     x;
    int16_t     y;
    int16_t     angle;
    int16_t     type;           // DoomEd number for the thing
    int16_t     options;        // MTF_XXX map thing flags
};

static_assert(sizeof(mapthing_t) == 10);

// Metadata for a texture in the TEXTURE1 lump
struct maptexture_t {
    int16_t     offsetX;
    int16_t     offsetY;
    int16_t     width;
    int16_t     height;
};

static_assert(sizeof(maptexture_t) == 8);
