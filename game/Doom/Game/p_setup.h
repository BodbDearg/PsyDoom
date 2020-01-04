#pragma once

#include "Doom/doomdef.h"
#include "Doom/cdmaptbl.h"

struct line_t;
struct seg_t;
struct side_t;
struct vertex_t;

extern const VmPtr<VmPtr<uint16_t>>         gpBlockmapLump;
extern const VmPtr<VmPtr<uint16_t>>         gpBlockmap;
extern const VmPtr<int32_t>                 gBlockmapWidth;
extern const VmPtr<int32_t>                 gBlockmapHeight;
extern const VmPtr<fixed_t>                 gBlockmapOriginX;
extern const VmPtr<fixed_t>                 gBlockmapOriginY;
extern const VmPtr<VmPtr<VmPtr<mobj_t>>>    gppBlockLinks;
extern const VmPtr<int32_t>                 gNumVertexes;
extern const VmPtr<VmPtr<vertex_t>>         gpVertexes;
extern const VmPtr<int32_t>                 gNumSectors;
extern const VmPtr<VmPtr<sector_t>>         gpSectors;
extern const VmPtr<int32_t>                 gNumSides;
extern const VmPtr<VmPtr<side_t>>           gpSides;
extern const VmPtr<int32_t>                 gNumLines;
extern const VmPtr<VmPtr<line_t>>           gpLines;
extern const VmPtr<int32_t>                 gNumSubsectors;
extern const VmPtr<VmPtr<subsector_t>>      gpSubsectors;
extern const VmPtr<int32_t>                 gNumSegs;
extern const VmPtr<VmPtr<seg_t>>            gpSegs;

void P_LoadNodes() noexcept;
void P_LoadMapLump() noexcept;
void P_GroupLines() noexcept;
void P_InitMapTextures() noexcept;
void P_SetupLevel(const int32_t mapNum, const skill_t skill) noexcept;
void P_LoadBlocks(const CdMapTbl_File file) noexcept;
void P_CacheSprite() noexcept;
void P_CacheMapTexturesWithWidth() noexcept;
