#pragma once

#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Renderer/r_local.h"
#include "Macros.h"

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
static inline void modifyLinedefs([[maybe_unused]] const ModFuncT & func) noexcept {}

template <class ModFuncT, class ...Int32List>
static inline void modifyLinedefs(const ModFuncT & func, const int32_t linedefIdx, Int32List... linedefIndexes) noexcept {
    ASSERT(linedefIdx < gNumLines);
    func(gpLines[linedefIdx]);
    modifyLinedefs(func, linedefIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: apply a transformation to a number of sectors
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ModFuncT>
static inline void modifySectors([[maybe_unused]] const ModFuncT & func) noexcept {}

template <class ModFuncT, class ...Int32List>
static inline void modifySectors(const ModFuncT & func, const int32_t sectorIdx, Int32List... sectorIndexes) noexcept {
    ASSERT(sectorIdx < gNumSectors);
    func(gpSectors[sectorIdx]);
    modifySectors(func, sectorIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: adds the 'ML_VOID' flag to a series of lines
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Int32List>
static inline void addVoidFlagToLinedefs(Int32List... linedefIndexes) noexcept {
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_VOID; },
        linedefIndexes...
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: adds the 'ML_ADD_SKY_WALL_HINT' flag to a series of lines
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Int32List>
static inline void addSkyWallFlagToLinedefs(Int32List... linedefIndexes) noexcept {
    modifyLinedefs(
        [](line_t& line) { line.flags |= ML_ADD_SKY_WALL_HINT; },
        linedefIndexes...
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility function: removes the 'ML_DONTDRAW' flag on the specified linedefs, allowing them to be seen in the automap
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Int32List>
static inline void unhideLinedefs(Int32List... linedefIndexes) noexcept {
    modifyLinedefs(
        [](line_t& line) { line.flags &= ~ML_DONTDRAW; },
        linedefIndexes...
    );
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
// Helper: applies patches common to ALL original 'Doom' and 'Final Doom' maps
//------------------------------------------------------------------------------------------------------------------------------------------
static inline void applyOriginalMapCommonPatches() noexcept {
    clearMysterySectorFlags();
}

END_NAMESPACE(MapPatches)
