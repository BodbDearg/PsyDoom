//------------------------------------------------------------------------------------------------------------------------------------------
// A module that allows patching original game maps to fix a few select issues.
// Provides mechanisms to check when patches should be applied and calls patch functions to do the modifications.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapPatcher.h"

#include "Asserts.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Renderer/r_local.h"
#include "Game.h"

#include <md5.h>

BEGIN_NAMESPACE(MapPatcher)

//------------------------------------------------------------------------------------------------------------------------------------------
// Format for a function that patches map data
//------------------------------------------------------------------------------------------------------------------------------------------
typedef void (*PatcherFunc)();

//------------------------------------------------------------------------------------------------------------------------------------------
// Describes a patch to be applied to map data.
// Defines when the patch is applied and the function that will apply the patch.
//------------------------------------------------------------------------------------------------------------------------------------------
struct PatchDef {
    int32_t         mapNumber;      // Which map number the patch applies to
    GameType        gameType;       // What game type the patch applies to
    int32_t         mapLumpIndex;   // Which map lump index the patch applies to, e.g 'ML_SECTORS' or 'ML_LINEDEFS'
    int32_t         lumpSize;       // The size of the lump that should be patched, if mismatching the patch is not applied
    uint64_t        md5Word1;       // The MD5 for the lump that should be patched (bytes 0-7), if mismatching the patch is not applied.
    uint64_t        md5Word2;       // The MD5 for the lump that should be patched (bytes 8-15), if mismatching the patch is not applied.
    PatcherFunc     patcherFunc;    // The function that does the work of patching
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: apply a transformation to a number of linedefs
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ModFuncT>
static void modifyLinedefs([[maybe_unused]] const ModFuncT & func) noexcept {}

template <class ModFuncT, class ...Int32List> 
static void modifyLinedefs(const ModFuncT & func, const int32_t linedefIdx, Int32List... linedefIndexes) noexcept {
    ASSERT(linedefIdx < gNumLines);
    func(gpLines[linedefIdx]);
    modifyLinedefs(func, linedefIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix a bug in Doom with the House Of Pain where an unintended door linedef causes one of the ceilings to collapse permanently.
// Removes the line action which causes the bug.
//------------------------------------------------------------------------------------------------------------------------------------------
static void fixHouseOfPainDoorBug() noexcept {
    gpLines[435].special = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fixes the starting hut for MAP47, The Citadel for Doom. Makes it so you can see past it, since it is shorter than buildings around it
//------------------------------------------------------------------------------------------------------------------------------------------
static void fixTheCitadelStartingHut() noexcept {
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        170, 171, 172, 173, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fix various issues in MAP23, Ballistyx for Final Doom
//------------------------------------------------------------------------------------------------------------------------------------------
static void fixBallistyxIssues() noexcept {
    // Make various linedefs not render sky walls or be see through for occlusion purposes
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        // Altar hole: don't draw sky walls
        1212, 1211, 1215, 1210, 1214, 1213,
        // Altar pillars: don't occlude and prevent geometry behind from rendering (needed for floating platform hack)
        1216, 1291, 1290, 1207, 1209, 1292, 1293, 1199,
        // Yellow key cage area: the outer wall floors sometimes don't render due to occluding sky walls; make them not occlude:
        1525, 1064, 1526, 1052, 371, 1053, 374, 1509, 373, 1054, 1524, 1055
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// All of the map fixes to apply
//------------------------------------------------------------------------------------------------------------------------------------------
static const PatchDef gPatches[] = {
    { 19, GameType::Doom,      ML_LINEDEFS, 8610,  0x417CD93948684803, 0xB43273CA53916561, fixHouseOfPainDoorBug },
    { 47, GameType::Doom,      ML_LINEDEFS, 17626, 0xF64A150D7F4D40A5, 0xE92E10C8CD886A7D, fixTheCitadelStartingHut },
    { 23, GameType::FinalDoom, ML_LINEDEFS, 21392, 0xCADEBB9043C70D3F, 0x4061F4281078EC52, fixBallistyxIssues }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to find a patch for the specified map lump in the map currently being loaded
//------------------------------------------------------------------------------------------------------------------------------------------
void applyPatches(const int32_t mapLumpIndex, const std::byte* const pLumpBytes, const int32_t lumpSize) noexcept {
    // Cache these globals locally since they will be compared a lot
    const int32_t mapNum = gGameMap;
    const GameType gameType = Game::gGameType;

    // Check to see which patch applies (if any)
    for (const PatchDef& patch : gPatches) {
        ASSERT(patch.patcherFunc);

        // Wrong map number?
        if (mapNum != patch.mapNumber)
            continue;

        // Wrong game type?
        if (gameType != patch.gameType)
            continue;

        // Wrong map lump type?
        if (mapLumpIndex != patch.mapLumpIndex)
            continue;

        // Wrong map lump size?
        if (lumpSize != patch.lumpSize)
            continue;

        // All checks have passed, need to do an MD5 of the data to see if it is to be patched
        const uint64_t md5W1 = patch.md5Word1;
        const uint64_t md5W2 = patch.md5Word2;

        const uint8_t expectedMD5[16] = {
            (uint8_t)(md5W1 >>  0), (uint8_t)(md5W1 >>  8), (uint8_t)(md5W1 >> 16), (uint8_t)(md5W1 >> 24),
            (uint8_t)(md5W1 >> 32), (uint8_t)(md5W1 >> 40), (uint8_t)(md5W1 >> 48), (uint8_t)(md5W1 >> 56),
            (uint8_t)(md5W2 >>  0), (uint8_t)(md5W2 >>  8), (uint8_t)(md5W2 >> 16), (uint8_t)(md5W2 >> 24),
            (uint8_t)(md5W2 >> 32), (uint8_t)(md5W2 >> 40), (uint8_t)(md5W2 >> 48), (uint8_t)(md5W2 >> 56),
        };

        MD5 md5;
        md5.add(pLumpBytes, (size_t) lumpSize);

        uint8_t actualMD5[16] = {};
        md5.getHash(actualMD5);

        // Does the MD5 hash match?
        if (std::memcmp(expectedMD5, actualMD5, sizeof(expectedMD5)) != 0)
            continue;

        // Match, apply the patch and abort search:
        patch.patcherFunc();
        break;
    }
}

END_NAMESPACE(MapPatcher)
