#pragma once

#include <cstdint>

struct line_t;

enum ceiling_e : uint32_t {
    lowerToFloor            = 0,
    raiseToHighest          = 1,
    lowerAndCrush           = 2,
    crushAndRaise           = 3,
    fastCrushAndRaise       = 4,
    silentCrushAndRaise     = 5,
};

bool EV_DoCeiling(line_t& line, const ceiling_e ceilingType) noexcept;
bool EV_CeilingCrushStop(line_t& line) noexcept;
