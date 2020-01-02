#pragma once

#include "Doom/doomdef.h"

// Represents a vertex used by the renderer
struct vertex_t {
    fixed_t     x;
    fixed_t     y;
    uint32_t    unknown1;       // TODO: what is this field for?
    uint32_t    unknown2;       // TODO: what is this field for?
    uint32_t    unknown3;       // TODO: what is this field for?
    uint32_t    unknown4;       // TODO: what is this field for?
    uint32_t    unknown5;       // TODO: what is this field for?
};

static_assert(sizeof(vertex_t) == 28);
