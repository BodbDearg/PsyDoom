#pragma once

#include "Doom/doomdef.h"

struct line_t;

enum ceiling_e : uint32_t {
    lowerToFloor            = 0,
    raiseToHighest          = 1,
    lowerAndCrush           = 2,
    crushAndRaise           = 3,
    fastCrushAndRaise       = 4,
    silentCrushAndRaise     = 5,
};

struct ceiling_t {
    thinker_t           thinker;
    ceiling_e           type;
    VmPtr<sector_t>     sector;
    fixed_t             bottomheight;   // TODO: COMMENT
    fixed_t             topheight;      // TODO: COMMENT
    fixed_t             speed;
    bool32_t            crush;
    int32_t             direction;      // 1 = up, 0 = waiting, -1 = down
    int32_t             tag;            // TODO: COMMENT
    int32_t             olddirection;   // TODO: COMMENT
};

static_assert(sizeof(ceiling_t) == 48);

void EV_DoCeiling() noexcept;
bool EV_CeilingCrushStop(line_t& line) noexcept;
