#pragma once

#include "Doom/doomdef.h"

#include <vector>

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
    thinker_t   thinker;            // Basic thinker properties
    ceiling_e   type;               // What type of behavior the ceiling mover has
    sector_t*   sector;             // Sector affected
    fixed_t     bottomheight;       // Lowest destination height
    fixed_t     topheight;          // Highest destination height
    fixed_t     speed;              // Speed of movement up or down
    bool        crush;              // Does the ceiling damage things when they don't fit?
    int32_t     direction;          // 1 = up, 0 = waiting, -1 = down
    int32_t     tag;                // Sector tag for the ceiling mover's sector
    int32_t     olddirection;       // In-stasis ceilings: which way the ceiling was moving before it was paused
};

#if PSYDOOM_LIMIT_REMOVING
    extern std::vector<ceiling_t*> gpActiveCeilings;
#else
    static constexpr int32_t MAXCEILINGS = 30;          // Maximum number of ceiling movers there can be active at once
    extern ceiling_t* gpActiveCeilings[MAXCEILINGS];
#endif

bool EV_DoCeiling(line_t& line, const ceiling_e ceilingType) noexcept;

#if PSYDOOM_MODS
    bool P_ActivateInStasisCeilingsForTag(const int32_t tag) noexcept;
    bool P_ActivateInStasisCeilingForSector(const sector_t& sector) noexcept;
    bool EV_CeilingCrushStopForTag(const int32_t tag) noexcept;
    bool EV_CeilingCrushStopForSector(const sector_t& sector) noexcept;
#endif

bool EV_CeilingCrushStop(line_t& line) noexcept;
