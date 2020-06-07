#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Rendering and geometry related data structures.
// A lot of these are used by game logic too, in addition to the renderer.
//------------------------------------------------------------------------------------------------------------------------------------------

#include "Doom/doomdef.h"
#include "Doom/Game/info.h"

struct line_t;

// Texture coordinates in PSX DOOM cannot be higher than '255' due to hardware limitations. All texture coordinates on the PS1 are
// a single byte only, which restricts the maximum texture size to 256x256 and also the maximum number of times textures can wrap.
// This is why the map designers for PSX DOOM were restricted to room heights <= 256 units, although there were some workarounds to
// extend past the limits a little - mainly by using upper and lower walls to do 'full' walls of up to 256 x 3 (768) units.
//
// I guess if the engine was a little more advanced it could have removed this limit by splitting up columns that exceeded the PS1's
// limitations - but that would have also incurred a higher draw cost naturally.
static constexpr int32_t TEXCOORD_MAX = UINT8_MAX;

// Min/max light diminishing intensities
static constexpr int32_t LIGHT_INTENSTIY_MIN = 64;
static constexpr int32_t LIGHT_INTENSTIY_MAX = 160;

// Represents a vertex in a line
struct vertex_t {
    fixed_t     x;
    fixed_t     y;
    fixed_t     scale;              // Scaling due to perspective. Higher values mean the vertex is closer
    int32_t     viewx;              // X position after view translation + rotation
    int32_t     viewy;              // Y position after view translation + rotation
    int32_t     screenx;            // X location on screen
    uint32_t    frameUpdated;       // When (in game frames) the vertex was last transformed: used to avoid unnecessary transforms
};

// PSX sector flags: just 1 defined - to disable reverb on a sector
static constexpr uint32_t SF_NO_REVERB = 0x1;

// Describes a sector or collection of lines and subsectors
struct sector_t {
    fixed_t         floorheight;
    fixed_t         ceilingheight;
    int32_t         floorpic;
    int32_t         ceilingpic;
    int16_t         colorid;
    int16_t         lightlevel;
    int32_t         special;
    int32_t         tag;
    int32_t         soundtraversed;     // Has sound reached the sector? (0 = not checked, 1 = yes, 2 = yes but one ML_SOUNDBLOCK line has been passed)
    mobj_t*         soundtarget;
    uint32_t        flags;              // Sector flags (new addition for PSX)
    int32_t         blockbox[4];        // TODO: CONFIRM LAYOUT
    degenmobj_t     soundorg;           // TODO: CONFIRM LAYOUT
    int32_t         validcount;
    mobj_t*         thinglist;
    void*           specialdata;
    int32_t         linecount;
    line_t**        lines;
};

// Describes a side of a line
struct side_t {
    fixed_t     textureoffset;
    fixed_t     rowoffset;
    int32_t     toptexture;
    int32_t     bottomtexture;
    int32_t     midtexture;
    sector_t*   sector;
};

// What type of slope a line has
enum slopetype_t : int32_t {
    ST_HORIZONTAL,
    ST_VERTICAL,
    ST_POSITIVE,
    ST_NEGATIVE
};

// Describes a line in the map
struct line_t {
    vertex_t*       vertex1;
    vertex_t*       vertex2;
    fixed_t         dx;             // Precomputed 'v2 - v1' in the x and y directions
    fixed_t         dy;
    int32_t         flags;
    int32_t         special;
    int32_t         tag;
    int32_t         sidenum[2];     // If sidenum[1] is '-1' then the line is one sided
    fixed_t         bbox[4];
    slopetype_t     slopetype;      // Used to simplify some collision detection
    sector_t*       frontsector;
    sector_t*       backsector;
    int32_t         validcount;
    void*           specialdata;    // Used by thinkers doing special logic
    fixed_t         fineangle;      // So sine/cosine can be looked up quicker
};

// Describes a convex region within a sector
struct subsector_t {
    sector_t*   sector;
    int16_t     numsegs;
    int16_t     firstseg;
    int16_t     numLeafEdges;
    int16_t     firstLeafEdge;
    int16_t     unknown5;           // TODO: find out what this field is
    int16_t     unknown6;           // TODO: find out what this field is
};

// The partition/dividing line for a bsp tree node: stores a position and line vector
struct divline_t {
    fixed_t     x;
    fixed_t     y;
    fixed_t     dx;
    fixed_t     dy;
};

// Descrbes a node in the bsp tree
struct node_t {
    divline_t   line;           // The partition line for the node
    fixed_t     bbox[2][4];     // Bounding box for both child nodes
    int32_t     children[2];    // When 'NF_SUBSECTOR' is set then it means it's a subsector number
};

// Seg flags
static constexpr uint16_t SGF_VISIBLE_COLS = 0x1;   // The seg has at least 1 visible (non fully occluded column)

// Describes a segment of a line
struct seg_t {
    vertex_t*   vertex1;
    vertex_t*   vertex2;
    fixed_t     offset;
    angle_t     angle;
    side_t*     sidedef;
    line_t*     linedef;
    sector_t*   frontsector;
    sector_t*   backsector;
    uint16_t    flags;              // TODO: find out more about the flags
    int16_t     visibleBegX;        // First visible screenspace column: only set if SGF_VISIBLE_COLS is set
    int16_t     visibleEndX;        // Last visible screenspace column (inclusive): only set if SGF_VISIBLE_COLS is set
    uint16_t    pad;                // TODO: used for any purpose, or just padding?
};

// Describes an edge of a leaf.
// A leaf corresponds to a single subsector, and contains a collection of leaf edges.
// The leaf edge simply contains a vertex and seg reference.
struct leafedge_t {
    vertex_t*   vertex;
    seg_t*      seg;
};

// Runtime render structure used for rendering leafs.
// This is cached in scratchpad memory of the PSX, hence the small limits here.
static constexpr int32_t MAX_LEAF_EDGES = 20;

struct leaf_t {
    int32_t     numEdges;
    leafedge_t  edges[MAX_LEAF_EDGES + 1];  // +1 so we store the first edge at the end of the list and avoid bound checking
};

// Holds information for a sprite frame
struct spriteframe_t {
    bool32_t    rotate;     // When false 'lump[0]' should be used for all angles
    uint32_t    lump[8];    // The lump number to use for viewing angles 0-7
    uint8_t     flip[8];    // Whether to flip horizontally viewing angles 0-7, non zero if flip
};

// Holds information for a sequence of sprite frames
struct spritedef_t {
    int32_t                 numframes;
    const spriteframe_t*    spriteframes;
};

// The list of sprite sequences
extern const spritedef_t gSprites[NUMSPRITES];
