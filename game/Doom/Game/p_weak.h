#pragma once

#include "Asserts.h"

#include <cstddef>
#include <cstdint>

struct mobj_t;

#if PSYDOOM_MODS

void P_InitWeakRefs() noexcept;
void P_ShutdownWeakRefs() noexcept;

// These functions are for internal use by 'MobjWeakPtr' only
uint32_t P_AddWeakRef(mobj_t* const pMobj) noexcept;
void P_AddWeakRef(const uint32_t weakCountIdx) noexcept;
void P_RemoveWeakRef(const uint32_t weakCountIdx) noexcept;
mobj_t* P_WeakDeref(const uint32_t weakCountIdx) noexcept;
void P_WeakReferencedDestroyed(mobj_t& mobj) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// A 'std::weak_ptr' style weak reference to a map object.
// Added as an addition to PsyDoom to prevent undefined behavior, since sometimes map objects can still reference others that are destroyed.
// 
// Note: if fixing undefined behavior is not enabled then this just functions like a raw pointer and CAN return an invalid 'mobj_t'.
// An assert will be triggered in debug mode however when this happens.
//------------------------------------------------------------------------------------------------------------------------------------------
class MobjWeakPtr {
public:
    // The index used to represent a null weak pointer.
    // Use zero so that 'MobjWeakPtr' fields are automatically null if zero-initialized via memset.
    static constexpr uint32_t NULL_WEAK_COUNT_IDX = 0;

    inline MobjWeakPtr() noexcept : weakCountIdx(NULL_WEAK_COUNT_IDX) {}
    inline MobjWeakPtr(std::nullptr_t) noexcept : weakCountIdx(NULL_WEAK_COUNT_IDX) {}
    inline MobjWeakPtr(mobj_t* const pMobj) noexcept : weakCountIdx(P_AddWeakRef(pMobj)) {}

    inline MobjWeakPtr(const MobjWeakPtr& other) noexcept : weakCountIdx(other.weakCountIdx) {
        P_AddWeakRef(weakCountIdx);
    }

    inline MobjWeakPtr(MobjWeakPtr&& other) noexcept : weakCountIdx(other.weakCountIdx) {
        other.weakCountIdx = NULL_WEAK_COUNT_IDX;
    }

    inline ~MobjWeakPtr() noexcept {
        P_RemoveWeakRef(weakCountIdx);
    }

    inline MobjWeakPtr& operator = (std::nullptr_t) noexcept {
        P_RemoveWeakRef(weakCountIdx);
        weakCountIdx = NULL_WEAK_COUNT_IDX;
        return *this;
    }

    inline MobjWeakPtr& operator = (mobj_t* const pMobj) noexcept {
        const int32_t oldWeakCountIdx = weakCountIdx;
        weakCountIdx = P_AddWeakRef(pMobj);
        P_RemoveWeakRef(oldWeakCountIdx);
        return *this;
    }

    inline MobjWeakPtr& operator = (const MobjWeakPtr& other) noexcept {
        P_AddWeakRef(other.weakCountIdx);
        P_RemoveWeakRef(weakCountIdx);
        weakCountIdx = other.weakCountIdx;
        return *this;
    }

    inline MobjWeakPtr& operator = (MobjWeakPtr&& other) noexcept {
        if (this != &other) {
            const int32_t oldWeakCountIdx = weakCountIdx;
            weakCountIdx = other.weakCountIdx;
            other.weakCountIdx = NULL_WEAK_COUNT_IDX;
            P_RemoveWeakRef(oldWeakCountIdx);
        }

        return *this;
    }

    inline mobj_t& operator * () const noexcept {
        mobj_t* const pMobj = P_WeakDeref(weakCountIdx);
        ASSERT(pMobj);
        return *pMobj;
    }

    inline mobj_t* operator -> () const noexcept { return P_WeakDeref(weakCountIdx); }
    inline mobj_t* get() const noexcept { return P_WeakDeref(weakCountIdx); }
    inline operator bool () const noexcept { return (P_WeakDeref(weakCountIdx) != nullptr); }
    inline operator mobj_t* () const noexcept { return P_WeakDeref(weakCountIdx); }

private:
    // Index of the weak count used to track the map object in question.
    // Will be '0' for a null/blank weak pointer.
    uint32_t weakCountIdx;
};

#endif  // #if PSYDOOM_MODS
