#include "z_zone.h"

#include "Doom/psx_main.h"
#include "i_main.h"
#include "PsxVm/PsxVm.h"

// The minimum size that a memory block must be
static constexpr int32_t MINFRAGMENT = 64;

// The main (and only) memory zone used by PSX DOOM
extern const VmPtr<VmPtr<memzone_t>> gpMainMemZone(0x80078198);

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
    *gpMainMemZone = Z_InitZone(vmAddrToPtr<void>(AlignedHeapStartAddr), AlignedHeapSize);
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
                    a0 = 0x800113DC;        // Result = STR_Z_Malloc_AllocFailed_Err[0] (800113DC)
                    a1 = allocSize;
                    I_Error();
                }

                continue;
            }

            // Chuck out this block!
            Z_Free2(**gpMainMemZone, &pRover[1]);
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
            a0 = 0x80011400;    // Result = STR_Z_Malloc_NoBlockOwner_Err[0] (80011400)
            I_Error();
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

void _thunk_Z_Malloc() noexcept {
    v0 = ptrToVmAddr(Z_Malloc(*vmAddrToPtr<memzone_t>(a0), a1, (int16_t) a2, vmAddrToPtr<VmPtr<void>>(a3)));
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
                a0 = 0x800113DC;    // Result = STR_Z_Malloc_AllocFailed_Err[0] (800113DC)
                a1 = allocSize;
                I_Error();
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
                    a0 = 0x800113DC;    // Result = STR_Z_Malloc_AllocFailed_Err[0] (800113DC)
                    a1 = allocSize;
                    I_Error();
                }

                continue;
            }

            // Chuck out this block!
            Z_Free2(**gpMainMemZone, &pRover[1]);
        }

        // Merge adjacent free memory blocks where possible
        if (pBase != pRover) {
            pRover->size = pRover->size + pBase->size;
            pRover->next = pBase->next;
            pBase = pRover;

            if (pBase->next) {
                pBase->next->prev = pRover;
            }
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
            a0 = 0x80011400;    // Result = STR_Z_Malloc_NoBlockOwner_Err[0] (80011400)
            I_Error();
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

void _thunk_Z_EndMalloc() noexcept {
    v0 = ptrToVmAddr(Z_EndMalloc(*vmAddrToPtr<memzone_t>(a0), a1, (int16_t) a2, vmAddrToPtr<VmPtr<void>>(a3)));
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
        a0 = 0x80011434;    // Result = STR_Z_Free_PtrNoZoneId_Err[0] (80011434)
        I_Error();
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

void _thunk_Z_Free2() noexcept {
    Z_Free2(*vmAddrToPtr<memzone_t>(a0), vmAddrToPtr<void>(a1));
}

void Z_FreeTags() noexcept {
loc_80032640:
    sp -= 0x28;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(s3, sp + 0x1C);
    s3 = a1;
    sw(s0, sp + 0x10);
    s0 = s2 + 8;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s1, sp + 0x14);
    if (s0 == 0) goto loc_800326E8;
    s4 = 0x1D4A;                                        // Result = 00001D4A
loc_80032670:
    v0 = lw(s0 + 0x4);
    s1 = lw(s0 + 0x10);
    if (v0 == 0) goto loc_800326D8;
    v0 = lh(s0 + 0x8);
    v0 &= s3;
    if (v0 == 0) goto loc_800326D8;
    v0 = lh(s0 + 0xA);
    if (v0 == s4) goto loc_800326B4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1434;                                       // Result = STR_Z_Free_PtrNoZoneId_Err[0] (80011434)
    I_Error();
loc_800326B4:
    v1 = lw(s0 + 0x4);
    v0 = (v1 < 0x101);
    if (v0 != 0) goto loc_800326CC;
    sw(0, v1);
loc_800326CC:
    sw(0, s0 + 0x4);
    sh(0, s0 + 0x8);
    sh(0, s0 + 0xA);
loc_800326D8:
    s0 = s1;
    if (s0 != 0) goto loc_80032670;
    s0 = s2 + 8;
loc_800326E8:
    v0 = s2 + 8;
    if (s0 == 0) goto loc_80032748;
loc_800326F0:
    v0 = lw(s0 + 0x4);
    s1 = lw(s0 + 0x10);
    if (v0 != 0) goto loc_8003273C;
    if (s1 == 0) goto loc_8003273C;
    v0 = lw(s1 + 0x4);
    if (v0 != 0) goto loc_8003273C;
    v0 = lw(s0);
    v1 = lw(s1);
    v0 += v1;
    sw(v0, s0);
    v0 = lw(s1 + 0x10);
    s1 = s0;
    sw(v0, s0 + 0x10);
    sw(s1, v0 + 0x14);
loc_8003273C:
    s0 = s1;
    v0 = s2 + 8;
    if (s0 != 0) goto loc_800326F0;
loc_80032748:
    sw(v0, s2 + 0x4);
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void Z_CheckHeap() noexcept {
loc_80032770:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s0, sp + 0x10);
    s0 = s1 + 8;
    sw(ra, sp + 0x18);
    if (s0 == 0) goto loc_80032820;
loc_8003278C:
    v1 = lw(s0 + 0x10);
    if (v1 != 0) goto loc_800327C4;
    v0 = lw(s0);
    v1 = lw(s1);
    v0 += s0;
    v0 -= s1;
    if (v0 == v1) goto loc_80032810;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x145C;                                       // Result = STR_Z_CheckHeap_ZoneSizeChanged_Err[0] (8001145C)
    goto loc_80032808;
loc_800327C4:
    v0 = lw(s0);
    v0 += s0;
    if (v0 == v1) goto loc_800327E8;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x147C;                                       // Result = STR_Z_CheckHeap_BlockNotTouching_Err[0] (8001147C)
    I_Error();
loc_800327E8:
    v0 = lw(s0 + 0x10);
    v0 = lw(v0 + 0x14);
    if (v0 == s0) goto loc_80032810;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x14B4;                                       // Result = STR_Z_CheckHeap_BadBlockBackLink_Err[0] (800114B4)
loc_80032808:
    I_Error();
loc_80032810:
    s0 = lw(s0 + 0x10);
    if (s0 != 0) goto loc_8003278C;
loc_80032820:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void Z_ChangeTag() noexcept {
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s2, sp + 0x18);
    sw(ra, sp + 0x1C);
    v1 = lh(s0 - 0xE);
    v0 = 0x1D4A;                                        // Result = 00001D4A
    s2 = s0 - 0x18;
    if (v1 == v0) goto loc_80032874;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x14EC;                                       // Result = STR_Z_ChangeTag_PtrNoZoneId_Err[0] (800114EC)
    I_Error();
loc_80032874:
    v0 = (i32(s1) < 0x10);
    if (v0 != 0) goto loc_800328A4;
    v0 = lw(s0 - 0x14);
    v0 = (v0 < 0x100);
    if (v0 == 0) goto loc_800328A4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1518;                                       // Result = STR_Z_ChangeTag_NoBlockOwner_Err[0] (80011518)
    I_Error();
loc_800328A4:
    sh(s1, s2 + 0x8);
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
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
