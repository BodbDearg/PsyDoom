#pragma once

#include "Asserts.h"
#include "Doom/doomdef.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_local.h"

#include <cstdint>
#include <optional>

BEGIN_NAMESPACE(MapPatcherUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Apply a transformation to a number of lines
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ModFuncT>
static inline void modifyLines([[maybe_unused]] const ModFuncT& func) noexcept {}

template <class ModFuncT, class ...Int32List>
static inline void modifyLines(const ModFuncT& func, const int32_t linedefIdx, Int32List... lineIndexes) noexcept {
    ASSERT(linedefIdx < gNumLines);
    func(gpLines[linedefIdx]);
    modifyLines(func, lineIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Apply a transformation to a number of sectors
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
// Adds specified flags to a series of lines
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Int32List>
static inline void addFlagsToLines(const uint32_t flags, Int32List... lineIndexes) noexcept {
    modifyLines(
        [flags](line_t& line) { line.flags |= flags; },
        lineIndexes...
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes the specified flags from a series of lines
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Int32List>
static inline void removeFlagsFromLines(const uint32_t flags, Int32List... lineIndexes) noexcept {
    modifyLines(
        [flags](line_t& line) { line.flags &= ~flags; },
        lineIndexes...
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Functions to add and remove various commonly used flags to linedefs
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Int32List>
static inline void addVoidFlagToLines(Int32List... lineIndexes) noexcept {
    addFlagsToLines(ML_VOID, lineIndexes...);
}

template <class ...Int32List>
static inline void addSkyWallFlagToLines(Int32List... lineIndexes) noexcept {
    addFlagsToLines(ML_ADD_SKY_WALL_HINT, lineIndexes...);
}

template <class ...Int32List>
static inline void hideLines(Int32List... lineIndexes) noexcept {
    addFlagsToLines(ML_DONTDRAW, lineIndexes...);
}

template <class ...Int32List>
static inline void unhideLines(Int32List... lineIndexes) noexcept {
    removeFlagsFromLines(ML_DONTDRAW, lineIndexes...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes actions from the specified lines
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Int32List>
static inline void removeLineActions(Int32List... lineIndexes) noexcept {
    modifyLines(
        [](line_t& line) noexcept {
            line.special = 0;
            line.tag = 0;
        },
        lineIndexes...
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Calls the specified lambda for all map objects
//------------------------------------------------------------------------------------------------------------------------------------------
template <class FuncT>
static inline void forAllMobj(const FuncT& func) noexcept {
    for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead;) {
        mobj_t* const pNextMobj = pMobj->next;
        func(*pMobj);
        pMobj = pNextMobj;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Calls the specified lambda for all map objects in the specified sectors
//------------------------------------------------------------------------------------------------------------------------------------------
template <class FuncT>
static inline void forAllMobjInSectors([[maybe_unused]] const FuncT& func) noexcept {}

template <class FuncT, class ...Int32List>
static inline void forAllMobjInSectors(const FuncT& func, const int32_t sectorIdx, Int32List... sectorIndexes) noexcept {
    ASSERT((sectorIdx >= 0) && (sectorIdx < gNumSectors));
    mobj_t* pMobj = gpSectors[sectorIdx].thinglist;

    while (pMobj) {
        mobj_t* const pNextMobj = pMobj->snext;
        func(*pMobj);
        pMobj = pNextMobj;
    }

    forAllMobjInSectors(func, sectorIndexes...);
}

void clearMysterySectorFlags() noexcept;
void applyOriginalMapCommonPatches() noexcept;

bool moveMobj(
    const std::optional<int32_t> srcSectorIdx,
    const std::optional<mobjtype_t> srcType,
    const std::optional<int32_t> srcX,
    const std::optional<int32_t> srcY,
    const std::optional<int32_t> dstX,
    const std::optional<int32_t> dstY,
    const std::optional<angle_t> dstAngle = {}
) noexcept;

END_NAMESPACE(MapPatcherUtils)
