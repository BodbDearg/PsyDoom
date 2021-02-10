#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Doom/doomdef.h"

#include <memory>

struct side_t;
struct line_t;

//------------------------------------------------------------------------------------------------------------------------------------------
// Line segment for the Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
struct rvseg_t {
    float       v1x, v1y;       // Vertex 1: xy position (Doom world coord system)
    float       v2x, v2y;       // Vertex 2: xy position (Doom world coord system)
    float       uOffset;        // Texture coordinate 'U' offset for the seg
    float       length;         // Length of the line segment
    angle_t     angle;          // Precomputed angle for the line segment direction
    uint32_t    flags;          // Flags for the line segment (SGF_XXX flags)
    side_t*     sidedef;        // Which side the segment belongs to
    line_t*     linedef;        // Which line the segment belongs to
    sector_t*   frontsector;    // Sector on the front side of the line segment
    sector_t*   backsector;     // Sector on the back side of the line segment (null if 1 sided line)
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Leaf edge for the Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
struct rvleafedge_t {
    float       v1x, v1y;   // The 1st vertex in the edge, xy position (Doom world coord system)
    rvseg_t*    seg;        // The seg associated with the edge
};

extern std::unique_ptr<rvseg_t[]>       gpRvSegs;
extern std::unique_ptr<rvleafedge_t[]>  gpRvLeafEdges;

void RV_InitLevelData() noexcept;
void RV_FreeLevelData() noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER
