#pragma once

#include "Doom/doomdef.h"

struct line_t;

// Type for a ceiling
enum ceiling_e : int32_t {
    lowerToFloor            = 0,
    raiseToHighest          = 1,
    lowerAndCrush           = 2,
    crushAndRaise           = 3,
    fastCrushAndRaise       = 4,
    silentCrushAndRaise     = 5,
};

// Config and state for a ceiling mover
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

// Maximum number of ceiling movers there can be active at once
static constexpr int32_t MAXCEILINGS = 30;

extern ceiling_t* gpActiveCeilings[MAXCEILINGS];

bool EV_DoCeiling(line_t& line, const ceiling_e ceilingType) noexcept;
bool EV_CeilingCrushStop(line_t& line) noexcept;
