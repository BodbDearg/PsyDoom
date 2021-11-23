#include "p_weak.h"

#if PSYDOOM_MODS

#include "Doom/doomdef.h"

#include <vector>

// Weak count internal data.
// Stores the number of weak references to an object, whether it is alive and a pointer to the object itself.
struct WeakCount {
    uint32_t    count;          // How many weak references there are to the object
    bool        bMobjExists;    // Whether the object is still valid/existing
    mobj_t*     pMobj;          // The object (weakly) pointed to
};

static std::vector<WeakCount>   gWeakCounts;            // Weak references to various map objects, some of these slots may be unused
static std::vector<uint32_t>    gFreeWeakCountIdxs;     // Which weak count slots are currently free (by index)

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the specified weak count is still in use
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isWeakCountInUse(const WeakCount& weakCount) noexcept {
    return (weakCount.bMobjExists || (weakCount.count > 0));
}

#if ASSERTS_ENABLED
//------------------------------------------------------------------------------------------------------------------------------------------
// Debug helper: checks to see if any weak counts are still in use
//------------------------------------------------------------------------------------------------------------------------------------------
static bool areWeakCountsInUse() noexcept {
    for (const WeakCount& weakCount : gWeakCounts) {
        if (isWeakCountInUse(weakCount))
            return true;
    }

    return false;
}
#endif  // #if ASSERTS_ENABLED

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocates a weak count for the specified map object which is assumed to not already have one and returns it by index.
// The count is not initialized.
//------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t allocWeakCount(mobj_t& mobj) noexcept {
    // Object must not already have a count allocated
    ASSERT(mobj.weakCountIdx == MobjWeakPtr::NULL_WEAK_COUNT_IDX);

    // Is there a free count we can use?
    if (!gFreeWeakCountIdxs.empty()) {
        const uint32_t idx = gFreeWeakCountIdxs.back();
        gFreeWeakCountIdxs.pop_back();
        return idx;
    }

    // If there's no free count then alloc a new one
    const uint32_t idx = (uint32_t) gWeakCounts.size();
    gWeakCounts.emplace_back();
    return idx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the weak reference system: should be called on level setup, prior to creating map objects
//------------------------------------------------------------------------------------------------------------------------------------------
void P_InitWeakRefs() noexcept {
    ASSERT(gWeakCounts.empty());
    ASSERT(gFreeWeakCountIdxs.empty());
    gWeakCounts.reserve(2048);
    gWeakCounts.resize(1);              // The first index is unused (the null weak count index)
    gFreeWeakCountIdxs.reserve(1024);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the weak reference system: should be called on level teardown
//------------------------------------------------------------------------------------------------------------------------------------------
void P_ShutdownWeakRefs() noexcept {
    ASSERT_LOG(!areWeakCountsInUse(), "There should be no weak references to objects on shutting down this system!");
    gWeakCounts.clear();
    gFreeWeakCountIdxs.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds a weak reference to the specified map object and returns the index of the weak count used to keep track of it.
// If the object is a null pointer returns 'MobjWeakPtr::NULL_WEAK_COUNT_IDX'.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t P_AddWeakRef(mobj_t* const pMobj) noexcept {
    // Adding a reference to a null object?
    if (!pMobj)
        return MobjWeakPtr::NULL_WEAK_COUNT_IDX;

    // Does the count already exist? If so then just increment that:
    const uint32_t curCountIdx = pMobj->weakCountIdx;

    if (curCountIdx != MobjWeakPtr::NULL_WEAK_COUNT_IDX) {
        ASSERT(curCountIdx < gWeakCounts.size());
        WeakCount& weakCount = gWeakCounts[curCountIdx];
        weakCount.count++;
        return curCountIdx;
    }

    // If there's no count then make a new one and populate it with a single weak reference
    const uint32_t newCountIdx = allocWeakCount(*pMobj);

    WeakCount& weakCount = gWeakCounts[newCountIdx];
    weakCount = {};
    weakCount.count = 1;
    weakCount.bMobjExists = true;
    weakCount.pMobj = pMobj;

    // Assign the map object it's count and return which count is being used
    pMobj->weakCountIdx = newCountIdx;
    return newCountIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds an extra weak reference to the specified weak count (unless it's the null count index)
//------------------------------------------------------------------------------------------------------------------------------------------
void P_AddWeakRef(const uint32_t weakCountIdx) noexcept {
    if (weakCountIdx == MobjWeakPtr::NULL_WEAK_COUNT_IDX)
        return;

    ASSERT(weakCountIdx < gWeakCounts.size());
    WeakCount& weakCount = gWeakCounts[weakCountIdx];
    weakCount.count++;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes a weak reference to the specified weak count (unless it's the null count index)
//------------------------------------------------------------------------------------------------------------------------------------------
void P_RemoveWeakRef(const uint32_t weakCountIdx) noexcept {
    if (weakCountIdx == MobjWeakPtr::NULL_WEAK_COUNT_IDX)
        return;

    ASSERT(weakCountIdx < gWeakCounts.size());
    WeakCount& weakCount = gWeakCounts[weakCountIdx];
    ASSERT(weakCount.count > 0);
    weakCount.count--;

    // If the weak count is no longer in use then free it
    if (!isWeakCountInUse(weakCount)) {
        gFreeWeakCountIdxs.push_back(weakCountIdx);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the pointer to a weakly referenced map object
//------------------------------------------------------------------------------------------------------------------------------------------
mobj_t* P_WeakDeref(const uint32_t weakCountIdx) noexcept {
    if (weakCountIdx == MobjWeakPtr::NULL_WEAK_COUNT_IDX)
        return nullptr;

    ASSERT(weakCountIdx < gWeakCounts.size());
    WeakCount& weakCount = gWeakCounts[weakCountIdx];

    if (weakCount.bMobjExists) {
        return weakCount.pMobj;
    } else {
        #if PSYDOOM_FIX_UB
            return nullptr;
        #else
            ASSERT_FAIL("Undefined behavior! Referencing a map object that has been destroyed!");
            return weakCount.pMobj;
        #endif
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Informs the weak reference system that the specified map object is being destroyed.
// Causes all weak references to the object to be nulled.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_WeakReferencedDestroyed(mobj_t& mobj) noexcept {
    // If there's no weak count we can just stop here
    const uint32_t weakCountIdx = mobj.weakCountIdx;

    if (weakCountIdx == MobjWeakPtr::NULL_WEAK_COUNT_IDX)
        return;

    // Otherwise mark the object destroyed and free up the weak count if it's no longer used
    ASSERT(weakCountIdx < gWeakCounts.size());
    WeakCount& weakCount = gWeakCounts[weakCountIdx];
    ASSERT_LOG(weakCount.bMobjExists, "The map object weakly referenced should only be destroyed once!");
    weakCount.bMobjExists = false;

    if (weakCount.count <= 0) {
        gFreeWeakCountIdxs.push_back(weakCountIdx);
    }
}
#endif  // #if PSYDOOM_MODS
