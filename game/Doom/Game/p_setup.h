#pragma once

#include "Doom/doomdef.h"

void P_LoadSegs() noexcept;
void P_LoadSubSectors() noexcept;
void P_LoadSectors() noexcept;
void P_LoadNodes() noexcept;
void P_LoadThings() noexcept;
void P_LoadLineDefs() noexcept;
void P_LoadSideDefs() noexcept;
void P_LoadBlockMap() noexcept;
void P_LoadMapLump() noexcept;
void P_LoadLeafs() noexcept;
void P_GroupLines() noexcept;
void P_InitMapTextures() noexcept;
void P_SetupLevel(const int32_t mapNum, const skill_t skill) noexcept;
void P_LoadBlocks() noexcept;
void P_CacheSprite() noexcept;
void P_CacheMapTexturesWithWidth() noexcept;
