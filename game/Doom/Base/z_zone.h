#pragma once

#include "PsxVm/VmPtr.h"

// Holds details on a block of memory
struct memblock_t {
	int32_t				size;			// Including the header and possibly tiny fragments
	VmPtr<void*>		user;			// NULL if a free block
	int16_t				tag;			// Purgelevel
	int16_t				id;				// Should be ZONEID
	int32_t				lockframe;		// Don't purge on this frame
	VmPtr<memblock_t>	next;
	VmPtr<memblock_t>	prev;
};

static_assert(sizeof(memblock_t) == 24);

// Info for a memory allocation zone
struct memzone_t {
	int32_t				size;			// Total bytes malloced, including header	
	memblock_t			blocklist;		// Start / end cap for linked list
	VmPtr<memblock_t>	rover;
};

static_assert(sizeof(memzone_t) == 32);

extern const VmPtr<VmPtr<memzone_t>> gpMainMemZone;

void Z_Init() noexcept;
void Z_InitZone() noexcept;
void Z_Malloc2() noexcept;
void Z_Malloc2_b() noexcept;
void Z_Free2() noexcept;
void Z_FreeTags() noexcept;
void Z_CheckHeap() noexcept;
void Z_ChangeTag() noexcept;
void Z_FreeMemory() noexcept;
void Z_DumpHeap() noexcept;
