#pragma once

#include "PsxVm/VmPtr.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Zone memory purge tags.
// Controls how and when particular blocks of memory are freed.
//  - Tags < 100 are not overwritten until freed.
//  - Tags >= 100 are purgable whenever needed.
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int16_t PU_STATIC      = 1;    // Static the entire execution time

// TODO: FIGURE OUT PURGE MODES
#if 0
    // static constexpr int16_t PU_SOUND       = 2;    // Static while playing (TODO: CONFIRM)
    // static constexpr int16_t PU_MUSIC       = 3;    // Static while playing (TODO: CONFIRM)
    // static constexpr int16_t PU_DAVE        = 4;    // Linux Doom comment: "Anything else Dave wants static" (TODO: CONFIRM)
    // static constexpr int16_t PU_LEVEL       = 16;   // Static until level exited (TODO: CONFIRM)
    // static constexpr int16_t PU_LEVSPEC     = 51;   // A special thinker in a level (TODO: CONFIRM)
#endif

static constexpr int16_t PU_PURGELEVEL  = 16;   // (TODO: CONFIRM)

// TODO: FIGURE OUT PURGE MODES
#if 0
    // static constexpr int16_t PU_CACHE       = 101;
#endif

// All blocks must have this id
static constexpr int16_t ZONEID = 0x1D4A;

// Holds details on a block of memory
struct memblock_t {
    int32_t             size;           // Including the header and possibly tiny fragments
    VmPtr<VmPtr<void>>  user;           // NULL if a free block
    int16_t             tag;            // Purgelevel
    int16_t             id;             // Should be ZONEID
    int32_t             lockframe;      // Don't purge on this frame
    VmPtr<memblock_t>   next;
    VmPtr<memblock_t>   prev;
};

static_assert(sizeof(memblock_t) == 24);

// Info for a memory allocation zone
struct memzone_t {
    int32_t             size;           // Total bytes malloced, including header       
    VmPtr<memblock_t>   rover;
    memblock_t          blocklist;      // Start / end cap for linked list
};

static_assert(sizeof(memzone_t) == 32);

extern const VmPtr<VmPtr<memzone_t>> gpMainMemZone;

void Z_Init() noexcept;
memzone_t* Z_InitZone(void* const pBase, const int32_t size) noexcept;

void* Z_Malloc(memzone_t& zone, const int32_t size, const int16_t tag, VmPtr<void>* const ppUser) noexcept;
void _thunk_Z_Malloc() noexcept;

void* Z_EndMalloc(memzone_t& zone, const int32_t size, const int16_t tag, VmPtr<void>* const ppUser) noexcept;
void _thunk_Z_EndMalloc() noexcept;

void Z_Free2(memzone_t& zone, void* const ptr) noexcept;
void _thunk_Z_Free2() noexcept;

void Z_FreeTags() noexcept;
void Z_CheckHeap() noexcept;
void Z_ChangeTag() noexcept;
int32_t Z_FreeMemory(memzone_t& zone) noexcept;
void Z_DumpHeap() noexcept;
