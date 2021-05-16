#pragma once

#include "info.h"

// Holds information for a sprite frame
struct spriteframe_t {
    uint32_t    rotate;     // When '0' or 'false' 'lump[0]' should be used for all angles
    uint32_t    lump[8];    // The lump number to use for viewing angles 0-7
    uint8_t     flip[8];    // Whether to flip horizontally viewing angles 0-7, non zero if flip
};

// Holds information for a sequence of sprite frames
struct spritedef_t {
    // PsyDoom: define a name so we can search for sprite entries by name
    #if PSYDOOM_MODS
        sprname_t name;
    #endif

    int32_t         numframes;      // How many frames in the sequence
    spriteframe_t*  spriteframes;   // The list of frames in the sequence
};

// The list of sprite sequences.
// PsyDoom: the size of this list is now determined at runtime, and can expand depending on WADs loaded.
#if PSYDOOM_MODS
    extern const spritedef_t*   gSprites;
    extern int32_t              gNumSprites;
#else
    extern const spritedef_t gSprites[BASE_NUM_SPRITES];
#endif

void P_InitSprites() noexcept;
int32_t P_SpriteCheckNumForName(const sprname_t name) noexcept;
int32_t P_SpriteGetNumForName(const sprname_t name) noexcept;
