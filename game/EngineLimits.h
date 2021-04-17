#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// A convenience header containing all modified hardcoded engine limits for PsyDoom.
// Gathers all these limits in one place, so they can be seen and compared with the old limits at a glance.
//------------------------------------------------------------------------------------------------------------------------------------------

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Is this a 64-bit build? (i.e the main target platform)
// For 64-bit builds we will need to extend original limits, even if we are trying to be as faithful to them as possible.
// This is because 64-bit pointers require more space, and bloat the size of certain structs - causing less memory to be available than before.
// Thus if you want memory behavior more accurate to the original, create a 32-bit build where possible.
//
// It's a guessing game how much bloat would be introduced by 64-bit, so in most cases I simply 2x the available memory just to be safe.
// That should be more than enough for most usage patterns.
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr bool IS_64_BIT_BUILD = (sizeof(void*) > 4);

//------------------------------------------------------------------------------------------------------------------------------------------
// How much heap space is available to Doom's zone memory allocator, via functions like 'Z_Malloc'.
// In a limit removing build this is just a default value and can be overriden by the user.
//
// The heap space available was roughly as follows for the original games:
//  - Doom          : 1,368,380 bytes
//  - Final Doom    : 1,357,328 bytes
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LIMIT_REMOVING
    static constexpr uint32_t Z_HEAP_DEFAULT_SIZE = 64 * 1024 * 1024;
#else
    static constexpr uint32_t Z_HEAP_SIZE = 1368380 * (IS_64_BIT_BUILD ? 2 : 1);
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// How much memory is available to hold the load .WMD (Williams Module File) for the game and also any currently loaded music sequences.
//
// The size of this memory chunk is as follows for the original games:
//  - Doom          : 26,000 bytes
//  - Final Doom    : 36,000 bytes
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint32_t WMD_MEM_SIZE = 36000 * (IS_64_BIT_BUILD ? 2 : 1);
