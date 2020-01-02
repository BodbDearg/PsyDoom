#pragma once

#include "Doom/doomdef.h"

struct line_t;

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

// Describes a sector used by the game and renderer
struct sector_t {
    fixed_t                 floorheight;
    fixed_t                 ceilingheight;
    int32_t                 floorpic;
    int32_t                 ceilingpic;
    int16_t                 colorid;
    int16_t                 lightlevel;
    int32_t                 special;
    int32_t                 tag;
    int32_t                 soundtraversed;
    VmPtr<mobj_t>           soundtarget;
    uint32_t                flags;              // TODO: CONFIRM LAYOUT    
    int32_t                 blockbox[4];        // TODO: CONFIRM LAYOUT
    degenmobj_t             soundorg;           // TODO: CONFIRM LAYOUT
    int32_t                 validcount;
    VmPtr<mobj_t>           thinglist;
    VmPtr<void>             specialdata;
    int32_t                 linecount;
    VmPtr<VmPtr<line_t>>    lines;
};

static_assert(sizeof(sector_t) == 92);
