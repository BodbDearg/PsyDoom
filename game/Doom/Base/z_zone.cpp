#include "z_zone.h"

#include "Doom/psx_main.h"
#include "i_main.h"
#include "PsxVm/PsxVm.h"

// The minimum size that a memory block must be
static constexpr int32_t MINFRAGMENT = 64;

// The main (and only) memory zone used by PSX DOOM
memzone_t* gpMainMemZone;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the zone memory management system.
// DOOM doesn't use any PsyQ SDK allocation functions AT ALL (either directly or indirectly) so it just gobbles up the entire of the
// available heap space on the system for it's own purposes.
//------------------------------------------------------------------------------------------------------------------------------------------
void Z_Init() noexcept {
    // The 32-bit aligned heap start address and the same value wrapped to 2 MiB.
    // This value was 32-bit aligned anyway but I guess this code is just making sure of that?
    constexpr uint32_t AlignedHeapStartAddr = (HeapStartAddr + 3) & 0xFFFFFFFC;
    constexpr uint32_t WrappedHeapStartAddr = AlignedHeapStartAddr & 0x1FFFFFFF;

    // Figure out the 32-bit aligned heap size
    constexpr uint32_t StackStartAddr = StackEndAddr - StackSize;
    constexpr uint32_t AlignedHeapSize = (StackStartAddr - WrappedHeapStartAddr + 3) & 0xFFFFFFFC;

    // Setup and save the main memory zone (the only zone)
    gpMainMemZone = Z_InitZone(vmAddrToPtr<void>(AlignedHeapStartAddr), AlignedHeapSize);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up the given block of memory as a memory zone
//------------------------------------------------------------------------------------------------------------------------------------------
memzone_t* Z_InitZone(void* const pBase, const int32_t size) noexcept {
    memzone_t* const pZone = (memzone_t*) pBase;

    pZone->size = size;
    pZone->rover = &pZone->blocklist;
    pZone->blocklist.size = size - sizeof(uint32_t) * 2;
    pZone->blocklist.user = nullptr;
    pZone->blocklist.tag = 0;
    pZone->blocklist.id = ZONEID;

    // This field was not being initialized in PSX DOOM.
    // It was not serving any useful purpose anyway so probably doesn't matter? Just initialize here though for good measure:
    #if PC_PSX_DOOM_MODS
        pZone->blocklist.lockframe = -1;
    #endif

    pZone->blocklist.next = nullptr;
    pZone->blocklist.prev = nullptr;
    return pZone;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocate a block of memory in the given memory zone with the given purgability tags.
// Optionally, a referencing pointer field can also be supplied which is updated when the block is allocated or freed.
//------------------------------------------------------------------------------------------------------------------------------------------
void* Z_Malloc(memzone_t& zone, const int32_t size, const int16_t tag, VmPtr<void>* const ppUser) noexcept {
    // This is the real size to allocate: have to add room for a memblock and also 4-byte align
    const int32_t allocSize = (size + sizeof(memblock_t) + 3) & 0xFFFFFFFC;

    // Scan through the block list looking for the first free block of sufficient size.
    // Also throw out any purgable blocks along the way.
    memblock_t* pBase = zone.rover.get();
    memblock_t* const pStart = pBase;

    while (pBase->user || (pBase->size < allocSize)) {
        // Set the rover to the next block if the current is free, so we can merge free blocks:
        memblock_t* const pRover = (pBase->user) ? pBase : pBase->next.get();

        // Wraparound to the beginning of the block list if the rover has reached the end
        if (!pRover)
            goto block_list_begin;

        // Free the block if it is purgable, otherwise skip over it
        if (pRover->user) {
            // Is this block exempt from being purged?
            // If that is the case then we need to try the next one after it.
            if (pRover->tag < PU_PURGELEVEL) {
                pBase = pRover->next.get();

                if (!pBase) {
                block_list_begin:
                    pBase = &zone.blocklist;
                }

                // If we have wrapped around back to where we started then we're out of RAM.
                // In this case we have searched all blocks for one big enough and not found one :(
                if (pBase == pStart) {
                    Z_DumpHeap();
                    I_Error("Z_Malloc: failed allocation on %i", allocSize);
                }

                continue;
            }

            // Chuck out this block!
            Z_Free2(*gpMainMemZone, &pRover[1]);
        }

        // Merge adjacent free memory blocks where possible
        if (pBase != pRover) {
            pBase->size += pRover->size;
            pBase->next = pRover->next;

            if (pRover->next) {
                pRover->next->prev = pBase;
            }
        }
    }

    // If there are enough free bytes following the allocation then make a new memory block
    // and add it into the linked list of blocks:
    const int32_t numUnusedBytes = pBase->size - allocSize;
    
    if (numUnusedBytes > MINFRAGMENT) {
        std::byte* const pUnusedBytes = (std::byte*) pBase + allocSize;

        memblock_t& newBlock = (memblock_t&) *pUnusedBytes;
        newBlock.prev = pBase;
        newBlock.next = pBase->next;

        if (pBase->next) {
            pBase->next->prev = &newBlock;
        }

        pBase->next = &newBlock;
        pBase->size = allocSize;

        newBlock.size = numUnusedBytes;
        newBlock.user = nullptr;
        newBlock.tag = 0;
    }

    // Setup the links on the memory block back to the pointer referencing it.
    // Also populate the pointer referencing it (if given):
    if (ppUser) {
        pBase->user = VmPtr<VmPtr<void>>(ppUser);
        *ppUser = &pBase[1];
    } else {
        if (tag >= PU_PURGELEVEL) {
            I_Error("Z_Malloc: an owner is required for purgable blocks");
        }

        // Non purgable blocks without any owner are assigned a pointer value of '1'
        pBase->user = VmPtr<VmPtr<void>>(0x00000001);
    }
    
    // Set the tag and id for the block
    pBase->tag = tag;
    pBase->id = ZONEID;

    // Move along the rover to the next block and return the usable memory allocated (past the allocated block header)
    zone.rover = (pBase->next) ? pBase->next : &zone.blocklist;
    return &pBase[1];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// An alternate version of Z_Malloc that attempts to allocate at the end of the heap, or at least as close as possible to the end.
// Ignores the rover used by the memory zone also and always starts from the very end.
//------------------------------------------------------------------------------------------------------------------------------------------
void* Z_EndMalloc(memzone_t& zone, const int32_t size, const int16_t tag, VmPtr<void>* const ppUser) noexcept {
    // This is the real size to allocate: have to add room for a memblock and also 4-byte align
    const int32_t allocSize = (size + sizeof(memblock_t) + 3) & 0xFFFFFFFC;
    
    // Start at the very last block in the list, since we want to alloc at the end of the heap
    memblock_t* pBase = &zone.blocklist;

    while (pBase->next) {
        pBase = pBase->next.get();
    }

    while (pBase->user || (pBase->size < allocSize)) {
        // Set the rover to the previous block if the current is free, so we can merge free blocks:
        memblock_t* pRover;

        if (pBase->user) {
            pRover = pBase;
        } else {
            pRover = pBase->prev.get();

            // Have we gone past the start? If so then we have failed...
            if (!pRover) {
                I_Error("Z_Malloc: failed allocation on %i", allocSize);
            }
        }

        if (pRover->user) {
            // Is this block exempt from being purged?
            // If that is the case then we need to try the next one after it.
            if (pRover->tag < PU_PURGELEVEL) {
                pBase = pRover->prev.get();

                // If we have wrapped around back to the start of the zone then we're out of RAM.
                // In this case we have searched all blocks for one big enough and not found one :(
                if (!pBase) {
                    I_Error("Z_Malloc: failed allocation on %i", allocSize);
                }

                continue;
            }

            // Chuck out this block!
            Z_Free2(*gpMainMemZone, &pRover[1]);
        }

        // Merge adjacent free memory blocks where possible
        if (pBase != pRover) {
            pRover->size = pRover->size + pBase->size;
            pRover->next = pBase->next;

            if (pBase->next) {
                pBase->next->prev = pRover;
            }

            pBase = pRover;
        }
    }

    // If there are enough free bytes following the allocation then make a new memory block and add it into the linked list of blocks.
    // Unlike the regular Z_Malloc, the new block is added BEFORE the allocated memory.
    const int32_t numUnusedBytes = pBase->size - allocSize;
    memblock_t& freeBlock = *pBase;

    if (numUnusedBytes > MINFRAGMENT) {
        pBase = (memblock_t*)((std::byte*) pBase + numUnusedBytes);
        pBase->size = allocSize;
        pBase->prev = &freeBlock;
        pBase->next = freeBlock.next;
        
        if (freeBlock.next) {
            freeBlock.next->prev = pBase;
        }

        freeBlock.next = pBase;
        freeBlock.size = numUnusedBytes;
        freeBlock.user = nullptr;
        freeBlock.tag = 0;
    }

    // Setup the links on the memory block back to the pointer referencing it.
    // Also populate the pointer referencing it (if given):
    if (ppUser) {
        pBase->user = VmPtr<VmPtr<void>>(ppUser);
        *ppUser = &pBase[1];
    } else {
        if (tag >= PU_PURGELEVEL) {
            I_Error("Z_Malloc: an owner is required for purgable blocks");
        }
        
        // Non purgable blocks without any owner are assigned a pointer value of '1'
        pBase->user = VmPtr<VmPtr<void>>(0x00000001);
    }

    pBase->id = ZONEID;
    pBase->tag = tag;

    // Set the rover for the zone and return the usable memory allocated (past the allocated block header)
    zone.rover = &zone.blocklist;
    return (void*) &pBase[1];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Free the given block of memory.
// Note that the zone param is actually not needed here to perform the dealloc, perhaps it was passed in case it was needed in future?
//------------------------------------------------------------------------------------------------------------------------------------------
void Z_Free2([[maybe_unused]] memzone_t& zone, void* const ptr) noexcept {
    // Get the memory block header which is located before the actual memory.
    // Verify also that the id is sane and that we are not just being passed a garbage pointer:
    memblock_t& block = ((memblock_t*) ptr)[-1];

    if (block.id != ZONEID) {
        I_Error("Z_Free: freed a pointer without ZONEID");
    }

    // Clear the pointer field referencing the memory block too.
    // Treat very small addresses as not pointers also:
    if (block.user.get() > vmAddrToPtr<void>(0x100)) {
        *block.user = nullptr;
    }

    block.user = nullptr;
    block.tag = 0;
    block.id = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Free memory blocks that have one or more of the given tag bits
//------------------------------------------------------------------------------------------------------------------------------------------
void Z_FreeTags(memzone_t& zone, const int16_t tagBits) noexcept {
    // Free each block if it is in use and matches one of the given tags
    for (memblock_t* pBlock = &zone.blocklist; pBlock; pBlock = pBlock->next.get()) {
        if (pBlock->user) {
            if ((pBlock->tag & tagBits) != 0) {
                Z_Free2(zone, &pBlock[1]);
            }
        }
    }

    // Merge any adjacent free blocks that we find together to form bigger memory blocks
    memblock_t* pNextBlock;

    for (memblock_t* pBlock = &zone.blocklist; pBlock; pBlock = pNextBlock) {
        pNextBlock = pBlock->next.get();

        // See if there are two adjacent free blocks
        if ((!pBlock->user) && pNextBlock && (!pNextBlock->user)) {
            // Merge the two blocks together
            pBlock->size = pBlock->size + pNextBlock->size;
            pBlock->next = pNextBlock->next;

            // Update the back reference of the block following the next block.
            //
            // Note: in the original code there was no null check when updating this pointer block following the next block.
            // Perhaps the PSX simply ignored a write to address 0x14 or that was a valid write to BIOS reserved memory?
            // In any case that won't fly on PC obviously, so I'm adding a safety check here:
            #if PC_PSX_DOOM_MODS
                if (pNextBlock->next) {
            #endif
                    pNextBlock->next->prev = pBlock;
            #if PC_PSX_DOOM_MODS
                }
            #endif
            
            // Move back one block, because we need to examine the next block of the current block again
            pNextBlock = pBlock;
        }
    }

    // Reset the rover back to the start of the heap
    zone.rover = &zone.blocklist;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs basic sanity checks for the integrity of the heap.
// If any sanity checks fail, then a fatal error is emitted.
//------------------------------------------------------------------------------------------------------------------------------------------
void Z_CheckHeap(const memzone_t& zone) noexcept {
    // Sanity check all blocks in the heap
    for (const memblock_t* pBlock = &zone.blocklist; pBlock; pBlock = pBlock->next.get()) {
        // If we have reached the end of the block list, make sure we haven't 'lost' any heap memory.
        // Computed size for all the blocks should match the zone size:
        if (!pBlock->next) {
            const std::byte* const pZoneStartByte = (const std::byte*) &zone;
            const std::byte* const pBlockStartByte = (const std::byte*) pBlock;
            const std::byte* const pBlockEndByte = pBlockStartByte + pBlock->size;
            const int32_t actualZoneSize = (int32_t)(pBlockEndByte - pZoneStartByte);

            if (actualZoneSize != zone.size) {
                I_Error("Z_CheckHeap: zone size changed\n");
            }

            continue;
        }

        // The next block after this block should touch the current block
        const memblock_t* const pNextBlock = (const memblock_t*)((const std::byte*) pBlock + pBlock->size);

        if (pNextBlock != pBlock->next.get()) {
            I_Error("Z_CheckHeap: block size does not touch the next block\n");
        }

        // The next block should point back to this block
        if (pBlock->next->prev.get() != pBlock) {
            I_Error("Z_CheckHeap: next block doesn't have proper back link\n");
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update the tags for a given block of memory
//------------------------------------------------------------------------------------------------------------------------------------------
void Z_ChangeTag(void* const ptr, const int16_t tagBits) noexcept {
    memblock_t& block = ((memblock_t*) ptr)[-1];

    // Sanity check the zoneid for the block
    if (block.id != ZONEID) {
        I_Error("Z_ChangeTag: freed a pointer without ZONEID");
    }

    // If the block tag makes it purgeable then it must have an owner.
    // Note: regard very small user addresses as NOT pointers.
    if (tagBits >= PU_PURGELEVEL) {
        if (block.user.get() < vmAddrToPtr<void>(0x100)) {
            I_Error("Z_ChangeTag: an owner is required for purgable blocks");
        }
    }

    block.tag = (int16_t) tagBits;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Counts and returns the number of free bytes in the given memory zone
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t Z_FreeMemory(memzone_t& zone) noexcept {
    int32_t bytesFree = 0;

    for (memblock_t* pBlock = &zone.blocklist; pBlock; pBlock = pBlock->next.get()) {
        if (!pBlock->user) {
            bytesFree += pBlock->size;
        }
    }

    return bytesFree;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// This function is empty in PSX DOOM - probably compiled out of the release build.
// If you want this functionality you could take a look at the Linux DOOM source.
//------------------------------------------------------------------------------------------------------------------------------------------
void Z_DumpHeap() noexcept {
}
