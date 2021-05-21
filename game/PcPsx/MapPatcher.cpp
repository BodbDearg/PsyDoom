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
#include "MapHash.h"

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
    int32_t         mapNumber;      // Filter: map number
    GameType        gameType;       // Filter: game type
    int32_t         mapSize;        // Filter: combined size (in bytes) of all the lumps for the map
    uint64_t        md5Word1;       // Filter: The MD5 for the combined map lump data (bytes 0-7)
    uint64_t        md5Word2;       // Filter: The MD5 for the combined map lump data (bytes 8-15)
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
    { 19, GameType::Doom,       75515, 0x2A4A41FE016B71FE, 0x97F66D95A1AFA7A3, fixHouseOfPainDoorBug },
    { 47, GameType::Doom,      186755, 0xD51FE28AF00EE173, 0xCAD32CFE67F41581, fixTheCitadelStartingHut },
    { 23, GameType::FinalDoom, 167847, 0xB3062D0C57E6C33B, 0x86BE982C0D6B7518, fixBallistyxIssues }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to apply patches for the current map
//------------------------------------------------------------------------------------------------------------------------------------------
void applyPatches() noexcept {
    // Cache these globals locally since they will be compared a lot
    const int32_t mapNum = gGameMap;
    const GameType gameType = Game::gGameType;
    const int32_t mapSize = MapHash::gDataSize;
    const uint64_t md5Word1 = MapHash::gWord1;
    const uint64_t md5Word2 = MapHash::gWord2;

    // Check to see which patch applies (if any)
    for (const PatchDef& patch : gPatches) {
        ASSERT(patch.patcherFunc);

        // Wrong map number?
        if (mapNum != patch.mapNumber)
            continue;

        // Wrong game type?
        if (gameType != patch.gameType)
            continue;

        // Wrong map data size?
        if (mapSize != patch.mapSize)
            continue;

        // Wrong MD5 hash for all the map data?
        if ((md5Word1 != patch.md5Word1) || (md5Word2 != patch.md5Word2))
            continue;

        // Match, apply the patch and abort the search
        patch.patcherFunc();
        break;
    }
}

END_NAMESPACE(MapPatcher)
