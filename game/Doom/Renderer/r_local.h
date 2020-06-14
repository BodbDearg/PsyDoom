#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Rendering and geometry related data structures.
// A lot of these are used by game logic too, in addition to the renderer.
//------------------------------------------------------------------------------------------------------------------------------------------

#include "Doom/doomdef.h"
#include "Doom/Game/info.h"
#include "PcPsx/Types.h"

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
    fixed_t     x;                  // World x position
    fixed_t     y;                  // World y position
    fixed_t     scale;              // Render var: scaling due to perspective. Higher values mean the vertex is closer
    int32_t     viewx;              // Render var: X position after view translation + rotation
    int32_t     viewy;              // Render var: Y position after view translation + rotation
    int32_t     screenx;            // Render var: X location on screen
    uint32_t    frameUpdated;       // Render var: When (in game frames) the vertex was last transformed: used to avoid unnecessary transforms
};

// PSX sector flags: just 1 defined - to disable reverb on a sector
static constexpr uint32_t SF_NO_REVERB = 0x1;

// Describes a sector or collection of lines and subsectors
struct sector_t {
    fixed_t         floorheight;        // Current floor height for the sector
    fixed_t         ceilingheight;      // Current ceiling height for the sector
    int32_t         floorpic;           // Index of the flat texture used for the sector floor
    int32_t         ceilingpic;         // Index of the flat texture used for the sector ceiling
    int16_t         colorid;            // Which of the colored light colors (by index) to use for the sector
    int16_t         lightlevel;         // Sector light level (should be 0-255)
    int32_t         special;            // Special action for the sector: damage, secret, light flicker etc.
    int32_t         tag;                // Tag for the sector for use in targetted actions (triggered by switches, line crossings etc.)
    int32_t         soundtraversed;     // Has sound reached the sector? (0 = not checked, 1 = yes, 2 = yes but one ML_SOUNDBLOCK line has been passed)
    mobj_t*         soundtarget;        // The thing that last made a noise in the sector to alert monsters
    uint32_t        flags;              // Sector flags (new addition for PSX)
    int32_t         blockbox[4];        // Bounding box for the sector in blockmap units
    degenmobj_t     soundorg;           // A partial 'mobj_t' which defines where sounds come from in the sector, for sectors that make noises
    int32_t         validcount;         // A marker used to avoid re-doing certain checks
    mobj_t*         thinglist;          // The list of things in the sector; each thing stores next/previous sector thing links
    void*           specialdata;        // Stores a pointer to a thinker which is operating on the sector (if any)
    int32_t         linecount;          // How many lines in the sector
    line_t**        lines;              // The list of sector lines
};

// Describes a side of a line
struct side_t {
    fixed_t     textureoffset;      // Horizontal texture offset for the side
    fixed_t     rowoffset;          // Vertical texture offset for the side
    int32_t     toptexture;         // Wall texture index for the side's top texture
    int32_t     bottomtexture;      // Wall texture index for the side's bottom texture
    int32_t     midtexture;         // Wall texture index for the side's mid/wall texture
    sector_t*   sector;             // What sector the side belongs to
};

// What type of slope a line has
enum slopetype_t : int32_t {
    ST_HORIZONTAL,      // Line has no change in the y direction
    ST_VERTICAL,        // Line has no change in the x direction
    ST_POSITIVE,        // Line changes in both x and y directions: line dx/dy is > 0
    ST_NEGATIVE         // Line changes in both x and y directions: line dx/dy is < 0
};

// Describes a line in the map
struct line_t {
    vertex_t*       vertex1;        // 1st vertex of the line
    vertex_t*       vertex2;        // 2nd vertex of the line
    fixed_t         dx;             // Precomputed 'v2 - v1': x direction
    fixed_t         dy;             // Precomputed 'v2 - v1': y direction
    int32_t         flags;          // ML_XXX line flags
    int32_t         special;        // What special action (switch, trigger etc.) the line does
    int32_t         tag;            // Tag for the action: what to affect in some cases, in terms of sectors
    int32_t         sidenum[2];     // If sidenum[1] is '-1' then the line is one sided
    fixed_t         bbox[4];        // Worlspace bounding box for the line
    slopetype_t     slopetype;      // Used to simplify some collision detection
    sector_t*       frontsector;    // Sector on the front side of the line
    sector_t*       backsector;     // Sector on the back side of the line (null for a 1 sided line)
    int32_t         validcount;     // Marker used to avoid re-doing certain checks
    void*           specialdata;    // Used by thinkers doing special logic
    fixed_t         fineangle;      // So sine/cosine can be looked up quicker
};

// Describes a convex region within a sector
struct subsector_t {
    sector_t*   sector;             // Parent sector for the subsector
    int16_t     numsegs;            // How many line segments in this subsector
    int16_t     firstseg;           // Index of the first line segment for the subsector, in the global list of line segments
    int16_t     numLeafEdges;       // How many leaf edges there are for the subsector
    int16_t     firstLeafEdge;      // Index of the first leaf edge for the subsector, in the global list of leaf edges
    int16_t     _unknown1;          // Unused field: can't infer purpose because it is not used
    int16_t     _unknown2;          // Unused field: can't infer purpose because it is not used
};

// The partition/dividing line for a bsp tree node: stores a position and line vector
struct divline_t {
    fixed_t     x;      // 1st point in the line: x
    fixed_t     y;      // 1st point in the line: y
    fixed_t     dx;     // Vector from line p1 to p2: x
    fixed_t     dy;     // Vector from line p1 to p2: y
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
    vertex_t*   vertex1;            // 1st vertex in the line segment
    vertex_t*   vertex2;            // 2nd vertex in the line segment
    fixed_t     offset;             // Horizontal offset for the line segment's texture
    angle_t     angle;              // Precomputed angle for the line segment direction
    side_t*     sidedef;            // Which side the segment belongs to
    line_t*     linedef;            // Which line the segment belongs to
    sector_t*   frontsector;        // Sector on the front side of the line segment
    sector_t*   backsector;         // Sector on the back side of the line segment (null if 1 sided line)
    uint16_t    flags;              // Flags for the line segment (SGF_XXX flags)
    int16_t     visibleBegX;        // First visible screenspace column: only set if SGF_VISIBLE_COLS is set
    int16_t     visibleEndX;        // Last visible screenspace column (inclusive): only set if SGF_VISIBLE_COLS is set
};

// Describes an edge of a leaf.
// A leaf corresponds to a single subsector, and contains a collection of leaf edges.
// The leaf edge simply contains a vertex and seg reference.
struct leafedge_t {
    vertex_t*   vertex;     // The 1st vertex in the edge
    seg_t*      seg;        // The seg associated with the edge
};

// Runtime render structure used for rendering leafs.
// This is cached in scratchpad memory of the PSX, hence the small limits here.
static constexpr int32_t MAX_LEAF_EDGES = 20;

struct leaf_t {
    int32_t     numEdges;                       // How many edges are actually in use for the leaf
    leafedge_t  edges[MAX_LEAF_EDGES + 1];      // +1 so we store the first edge at the end of the list and avoid bound checking
};

// Holds information for a sprite frame
struct spriteframe_t {
    bool32_t    rotate;     // When false 'lump[0]' should be used for all angles
    uint32_t    lump[8];    // The lump number to use for viewing angles 0-7
    uint8_t     flip[8];    // Whether to flip horizontally viewing angles 0-7, non zero if flip
};

// Holds information for a sequence of sprite frames
struct spritedef_t {
    int32_t                 numframes;          // How many frames in the sequence
    const spriteframe_t*    spriteframes;       // The list of frames in the sequence
};

// The list of sprite sequences
extern const spritedef_t gSprites[NUMSPRITES];
