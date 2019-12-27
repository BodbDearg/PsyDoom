#pragma once

#include "Doom/doomdef.h"
#include "Doom/cdmaptbl.h"

extern const VmPtr<int32_t> gNumVertexes;

void P_LoadSegs() noexcept;
void P_LoadSectors() noexcept;
void P_LoadNodes() noexcept;
void P_LoadLineDefs() noexcept;
void P_LoadSideDefs() noexcept;
void P_LoadBlockMap() noexcept;
void P_LoadMapLump() noexcept;
void P_LoadLeafs() noexcept;
void P_GroupLines() noexcept;
void P_InitMapTextures() noexcept;
void P_SetupLevel(const int32_t mapNum, const skill_t skill) noexcept;
void P_LoadBlocks(const CdMapTbl_File file) noexcept;
void P_CacheSprite() noexcept;
void P_CacheMapTexturesWithWidth() noexcept;
