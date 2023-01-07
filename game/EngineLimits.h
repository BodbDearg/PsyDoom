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
// 
// Note: I've raised this slightly for non-limit removing builds to account for PsyDoom's increased memory usage on some data structures.
// The new limit of 1430 KiB (1,464,320 bytes) is *just* about enough to run all the maps from 'GEC Master Edition Beta 3'.
// 
// TODO: GEC ME BETA 4: verify heap size requirements for non-limit removing builds
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_LIMIT_REMOVING
    static constexpr uint32_t Z_HEAP_DEFAULT_SIZE = 64 * 1024 * 1024;
#else
    static constexpr uint32_t Z_HEAP_SIZE = 1430 * 1024 * (IS_64_BIT_BUILD ? 2 : 1);
#endif

// Limit removing: we now dynamically size the WMD memory amount based on the .WMD file size.
// We always allocate a base minimum amount however.
#if !PSYDOOM_LIMIT_REMOVING
    //--------------------------------------------------------------------------------------------------------------------------------------
    // How much memory is available to hold the load .WMD (Williams Module File) for the game and also any currently loaded music sequences.
    //
    // The size of this memory chunk is as follows for the original games:
    //  - Doom          : 26,000 bytes
    //  - Final Doom    : 36,000 bytes
    // 
    // Note: I've raised this slightly for non-limit removing builds since Final Doom seemed to be over-running this buffer at times.
    //------------------------------------------------------------------------------------------------------------------------------------------
    static constexpr uint32_t WMD_MEM_SIZE = 38000 * (IS_64_BIT_BUILD ? 2 : 1);
#else 
    static constexpr uint32_t WMD_MIN_MEM_SIZE = 256 * 1024;    // 256 KiB minimum
#endif
