#pragma once

#include "Config.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_local.h"
#include "Game.h"

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

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: apply a transformation to a number of linedefs
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ModFuncT>
static inline void modifyLinedefs([[maybe_unused]] const ModFuncT& func) noexcept {}

template <class ModFuncT, class ...Int32List>
static inline void modifyLinedefs(const ModFuncT& func, const int32_t linedefIdx, Int32List... linedefIndexes) noexcept {
    ASSERT(linedefIdx < gNumLines);
    func(gpLines[linedefIdx]);
    modifyLinedefs(func, linedefIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: apply a transformation to a number of sectors
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ModFuncT>
static inline void modifySectors([[maybe_unused]] const ModFuncT& func) noexcept {}

template <class ModFuncT, class ...Int32List>
static inline void modifySectors(const ModFuncT& func, const int32_t sectorIdx, Int32List... sectorIndexes) noexcept {
    ASSERT(sectorIdx < gNumSectors);
    func(gpSectors[sectorIdx]);
    modifySectors(func, sectorIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: adds specified flags to a series of lines
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Int32List>
static inline void addFlagsToLinedefs(const uint32_t flags, Int32List... linedefIndexes) noexcept {
    modifyLinedefs(
        [flags](line_t& line) { line.flags |= flags; },
        linedefIndexes...
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: removes the specified flags from a series of lines
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Int32List>
static inline void removeFlagsFromLinedefs(const uint32_t flags, Int32List... linedefIndexes) noexcept {
    modifyLinedefs(
        [flags](line_t& line) { line.flags &= ~flags; },
        linedefIndexes...
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility functions to add and remove various commonly used flags to linedefs
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Int32List>
static inline void addVoidFlagToLinedefs(Int32List... linedefIndexes) noexcept {
    addFlagsToLinedefs(ML_VOID, linedefIndexes...);
}

template <class ...Int32List>
static inline void addSkyWallFlagToLinedefs(Int32List... linedefIndexes) noexcept {
    addFlagsToLinedefs(ML_ADD_SKY_WALL_HINT, linedefIndexes...);
}

template <class ...Int32List>
static inline void hideLinedefs(Int32List... linedefIndexes) noexcept {
    addFlagsToLinedefs(ML_DONTDRAW, linedefIndexes...);
}

template <class ...Int32List>
static inline void unhideLinedefs(Int32List... linedefIndexes) noexcept {
    removeFlagsFromLinedefs(ML_DONTDRAW, linedefIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: clears mysterious sector flags (of unknown purpose) other than 'SF_NO_REVERB' found in original 'Doom' and 'Final Doom' maps.
// 
// The 'SF_NO_REVERB' flag was the only one originally used by the retail version of the game but for some reason various sectors in the
// game can use any of the 16 available bits as sector flags. This would not be a problem except that PsyDoom now assigns meaning to the
// most of the flag bits (for 2-colored lighting and other features) and this can cause issues for original game maps.
// Hence we must clear all sector flag bits (other than the 1st) for original game maps.
//------------------------------------------------------------------------------------------------------------------------------------------
static inline void clearMysterySectorFlags() noexcept {
    const int32_t numSectors = gNumSectors;
    sector_t* const pSectors = gpSectors;

    for (int32_t i = 0; i < numSectors; ++i) {
        // Note: flags is now only interpreted as 8-bits in the level data, the other 8-bits are used for 'ceilColorid'
        pSectors[i].flags &= (~SF_GHOSTPLAT);

        // Originally this was the high 8-bits of the 'flags' field.
        // Original maps never use 2 colored lighting so just set it to the floor color.
        pSectors[i].ceilColorid = pSectors[i].colorid;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: calls the specified lambda for all things
//------------------------------------------------------------------------------------------------------------------------------------------
template <class FuncT>
static inline void forAllThings(const FuncT& func) noexcept {
    for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead;) {
        mobj_t* const pNextMobj = pMobj->next;
        func(*pMobj);
        pMobj = pNextMobj;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: applies patches common to ALL original 'Doom' and 'Final Doom' maps
//------------------------------------------------------------------------------------------------------------------------------------------
static inline void applyOriginalMapCommonPatches() noexcept {
    // Note: always apply this patch regardless of map patch settings.
    // This one is CRITICAL to being able to play original game maps in PsyDoom!
    clearMysterySectorFlags();
}

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
