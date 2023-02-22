#pragma once

#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_local.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Game.h"

#include <cstdint>

BEGIN_NAMESPACE(MapPatches)

//------------------------------------------------------------------------------------------------------------------------------------------
// Format for a function that patches map data
//------------------------------------------------------------------------------------------------------------------------------------------
typedef void (*PatcherFunc)();

//------------------------------------------------------------------------------------------------------------------------------------------
// Describes a patch to be applied to map data.
// Defines when the patch is applied and the function that will apply the patch.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PatchDef {
    int32_t         mapSize;        // Filter: combined size (in bytes) of all the lumps for the map
    uint64_t        md5Word1;       // Filter: The MD5 for the combined map lump data (bytes 0-7)
    uint64_t        md5Word2;       // Filter: The MD5 for the combined map lump data (bytes 8-15)
    PatcherFunc     patcherFunc;    // The function that does the work of patching
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Struct containing a list of patches
//------------------------------------------------------------------------------------------------------------------------------------------
struct PatchList {
    const PatchDef* pPatches;
    uint32_t        numPatches;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// The list of patches for each game type
//------------------------------------------------------------------------------------------------------------------------------------------
extern const PatchList gPatches_Doom;
extern const PatchList gPatches_FinalDoom;
extern const PatchList gPatches_GEC_ME_Beta3;
extern const PatchList gPatches_GEC_ME_Beta4;

//------------------------------------------------------------------------------------------------------------------------------------------
// Helpers: check if various forms of map patches should be applied
//------------------------------------------------------------------------------------------------------------------------------------------
static inline bool shouldApplyMapPatches_GamePlay() noexcept {
    return Game::gSettings.bEnableMapPatches_GamePlay;
}

static inline bool shouldApplyMapPatches_Visual() noexcept {
    return Config::gbEnableMapPatches_Visual;
}

static inline bool shouldApplyMapPatches_PsyDoom() noexcept {
    return Config::gbEnableMapPatches_PsyDoom;
}

END_NAMESPACE(MapPatches)
