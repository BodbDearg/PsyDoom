#pragma once

#include "Doom/cdmaptbl.h"
#include "Doom/doomdef.h"
#include "PcPsx/FixedIndexSet.h"

struct leafedge_t;
struct line_t;
struct mapthing_t;
struct node_t;
struct seg_t;
struct side_t;
struct texture_t;
struct vertex_t;

// Maximum amount of deathmatch starts
static constexpr uint32_t MAX_DEATHMATCH_STARTS = 10;

extern uint16_t*        gpBlockmapLump;
extern uint16_t*        gpBlockmap;
extern int32_t          gBlockmapWidth;
extern int32_t          gBlockmapHeight;
extern fixed_t          gBlockmapOriginX;
extern fixed_t          gBlockmapOriginY;
extern mobj_t**         gppBlockLinks;
extern int32_t          gNumVertexes;
extern vertex_t*        gpVertexes;
extern int32_t          gNumSectors;
extern sector_t*        gpSectors;
extern int32_t          gNumSides;
extern side_t*          gpSides;
extern int32_t          gNumLines;
extern line_t*          gpLines;
extern int32_t          gNumSubsectors;
extern subsector_t*     gpSubsectors;
extern int32_t          gNumBspNodes;
extern node_t*          gpBspNodes;
extern int32_t          gNumSegs;
extern seg_t*           gpSegs;
extern int32_t          gTotalNumLeafEdges;
extern leafedge_t*      gpLeafEdges;
extern uint8_t*         gpRejectMatrix;
extern mapthing_t       gPlayerStarts[MAXPLAYERS];
extern mapthing_t       gDeathmatchStarts[MAX_DEATHMATCH_STARTS];
extern mapthing_t*      gpDeathmatchP;

#if PSYDOOM_MODS
    extern char gLevelStartupWarning[64];
#endif

#if PSYDOOM_LIMIT_REMOVING
    extern FixedIndexSet gCacheTextureSet;
    extern FixedIndexSet gCacheFlatTextureSet;
#endif

extern void (*gUpdateFireSkyFunc)(texture_t& skyTex);

void P_SetupLevel(const int32_t mapNum, const skill_t skill) noexcept;
