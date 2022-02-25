//------------------------------------------------------------------------------------------------------------------------------------------
// A module that allows patching original game maps to fix a few select issues.
// Provides mechanisms to check when patches should be applied and calls patch functions to do the modifications.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatcher.h"

#include "Doom/Base/i_main.h"
#include "Game.h"
#include "MapHash.h"
#include "MapPatches.h"

BEGIN_NAMESPACE(MapPatcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Chooses which set of map patches to apply for the current game
//------------------------------------------------------------------------------------------------------------------------------------------
static MapPatches::PatchList getGamePatchList() noexcept {
    switch (Game::gGameType) {
        case GameType::Doom:            return MapPatches::gPatches_Doom;
        case GameType::FinalDoom:       return MapPatches::gPatches_FinalDoom;
        case GameType::GEC_ME_Beta3:    return MapPatches::gPatches_GEC_ME_Beta3;
    }

    I_Error("MapPatcher: getGamePatchList(): unhandled game type!");
    return {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to apply patches for the current map
//------------------------------------------------------------------------------------------------------------------------------------------
void applyPatches() noexcept {
    // Cache these globals locally since they will be compared a lot
    const int32_t mapSize = MapHash::gDataSize;
    const uint64_t md5Word1 = MapHash::gWord1;
    const uint64_t md5Word2 = MapHash::gWord2;

    // Iterate through the list of patches and check to see which patch applies (if any)
    const MapPatches::PatchList patchList = getGamePatchList();

    for (uint32_t patchIdx = 0; patchIdx < patchList.numPatches; ++patchIdx) {
        const MapPatches::PatchDef& patch = patchList.pPatches[patchIdx];

        // Expect all patch definitions to have a valid function!
        ASSERT(patch.patcherFunc);

        // Wrong map data size?
        if (mapSize != patch.mapSize)
            continue;

        // Wrong MD5 hash for all the map data?
        if ((md5Word1 != patch.md5Word1) || (md5Word2 != patch.md5Word2))
            continue;

        // Match, apply the patch and abort the search:
        patch.patcherFunc();
        break;
    }
}

END_NAMESPACE(MapPatcher)
