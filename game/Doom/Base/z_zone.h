#pragma once

#include "PsxVm/VmPtr.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Zone memory purge tags.
// Controls how and when particular blocks of memory are freed.
//  - Tags < PU_PURGELEVEL are not overwritten until freed.
//  - Tags >= PU_PURGELEVEL are purgable whenever needed.
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int16_t PU_STATIC      = 0x01;     // Static the entire execution time
static constexpr int16_t PU_LEVEL       = 0x02;     // Remains until level is unloaded
static constexpr int16_t PU_LEVSPEC     = 0x04;     // TODO: comment on typical use
static constexpr int16_t PU_ANIMATION   = 0x08;     // TODO: comment on typical use
static constexpr int16_t PU_PURGELEVEL  = 0x10;     // TODO: comment on typical use
static constexpr int16_t PU_CACHE       = 0x20;     // TODO: comment on typical use

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

extern memzone_t* gpMainMemZone;

void Z_Init() noexcept;
memzone_t* Z_InitZone(void* const pBase, const int32_t size) noexcept;
void* Z_Malloc(memzone_t& zone, const int32_t size, const int16_t tag, VmPtr<void>* const ppUser) noexcept;
void* Z_EndMalloc(memzone_t& zone, const int32_t size, const int16_t tag, VmPtr<void>* const ppUser) noexcept;
void Z_Free2(memzone_t& zone, void* const ptr) noexcept;
void Z_FreeTags(memzone_t& zone, const int16_t tagBits) noexcept;
void Z_CheckHeap(const memzone_t& zone) noexcept;
void Z_ChangeTag(void* const ptr, const int16_t tagBits) noexcept;
int32_t Z_FreeMemory(memzone_t& zone) noexcept;
void Z_DumpHeap() noexcept;
