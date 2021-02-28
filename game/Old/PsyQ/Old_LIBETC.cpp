#include "Old_LIBETC.h"

#if !PSYDOOM_MODS

#include "Asserts.h"

// An imitation of the 1 KiB scratchpad memory/cache that the PlayStation had; now just a simple c-array
static uint8_t gScratchpad[1024];

//------------------------------------------------------------------------------------------------------------------------------------------
// Return a pointer to the specified 32-bit word in the PSX 'scratchpad' memory/cache.
// There is 1 KiB of memory in total in the scratchpad.
//------------------------------------------------------------------------------------------------------------------------------------------
void* LIBETC_getScratchAddr(const uint32_t word32Index) {
    // Sanity check: allow the 'end' pointer to be taken, but no further...
    #if ASSERTS_ENABLED
        constexpr int32_t SCRATCHPAD_END_WORD_IDX = sizeof(gScratchpad) / sizeof(uint32_t);
        ASSERT(word32Index <= SCRATCHPAD_END_WORD_IDX);
    #endif
    
    // Return the requested memory
    return gScratchpad + word32Index * sizeof(uint32_t);
}

#endif  // #if !PSYDOOM_MODS
