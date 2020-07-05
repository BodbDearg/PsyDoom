#if !PC_PSX_DOOM_MODS

#include <cstdint>

static constexpr uint32_t HeapStartAddr = 0x800A9EC4;       // Where the heap starts in the PlayStation's address space (unwrapped)
static constexpr uint32_t StackEndAddr  = 0x200000;         // Where the stack ends (at 2 MiB) in the PlayStation's RAM (wrapped)
static constexpr uint32_t StackSize     = 0x8000;           // Size of the stack for PSX DOOM (32 KiB)

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the zone memory management system. DOOM doesn't use any PsyQ SDK allocation functions AT ALL (either directly or indirectly)
// so it just gobbles up the entire of the available heap space on the system for it's own purposes.
//------------------------------------------------------------------------------------------------------------------------------------------
void Z_Init() noexcept {
    // The 32-bit aligned heap start address and the same value wrapped to 2 MiB.
    // This value was 32-bit aligned anyway but I guess this code is just making sure of that?
    constexpr uint32_t AlignedHeapStartAddr = (HeapStartAddr + 3) & 0xFFFFFFFC;
    constexpr uint32_t WrappedHeapStartAddr = AlignedHeapStartAddr & 0x1FFFFFFF;

    // Figure out the 32-bit aligned heap size
    constexpr uint32_t StackStartAddr = StackEndAddr - StackSize;
    constexpr uint32_t AlignedHeapSize = (StackStartAddr - WrappedHeapStartAddr + 3) & 0xFFFFFFFC;

    // PC-PSX: allocate the native heap for the application
    gZoneHeap.reset(new std::byte[AlignedHeapSize]);

    // Setup and save the main memory zone (the only zone)
    gpMainMemZone = Z_InitZone(gZoneHeap.get(), AlignedHeapSize * 2);
}

#endif
