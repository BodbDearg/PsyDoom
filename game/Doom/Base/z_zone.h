#pragma once

#include "PsxVm/VmPtr.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Zone memory purge tags.
// Controls how and when particular blocks of memory are freed.
//  - Tags < 100 are not overwritten until freed.
//  - Tags >= 100 are purgable whenever needed.
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int16_t PU_STATIC      = 1;    // Static entire execution time
static constexpr int16_t PU_SOUND       = 2;    // Static while playing
static constexpr int16_t PU_MUSIC       = 3;    // Static while playing
static constexpr int16_t PU_DAVE        = 4;    // Linux Doom comment: "Anything else Dave wants static"
static constexpr int16_t PU_LEVEL       = 50;   // Static until level exited
static constexpr int16_t PU_LEVSPEC     = 51;   // A special thinker in a level
static constexpr int16_t PU_PURGELEVEL  = 100;
static constexpr int16_t PU_CACHE       = 101;

// Holds details on a block of memory
struct memblock_t {
    int32_t             size;           // Including the header and possibly tiny fragments
    VmPtr<void*>        user;           // NULL if a free block
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

void* Z_Malloc2(memzone_t* const pZone, const int32_t size, const int16_t tag, void* const pUser) noexcept;
void _thunk_Z_Malloc2() noexcept;

void Z_Malloc2_b() noexcept;
void Z_Free2() noexcept;
void Z_FreeTags() noexcept;
void Z_CheckHeap() noexcept;
void Z_ChangeTag() noexcept;
void Z_FreeMemory() noexcept;
void Z_DumpHeap() noexcept;
