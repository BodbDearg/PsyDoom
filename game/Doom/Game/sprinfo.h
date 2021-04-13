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
    int32_t                 numframes;          // How many frames in the sequence
    const spriteframe_t*    spriteframes;       // The list of frames in the sequence
};

// The list of sprite sequences
extern const spritedef_t gSprites[NUMSPRITES];
