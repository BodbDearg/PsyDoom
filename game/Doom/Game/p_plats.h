#pragma once

#include "Doom/doomdef.h"

struct line_t;

// Moving platform type
enum plattype_e : int32_t {
    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS
};

// Current status for a moving platform
enum plat_e : int32_t {
    up,
    down,
    waiting,
    in_stasis
};

// Config and state for a moving platform
struct plat_t {
    thinker_t       thinker;        // Basic thinker properties
    sector_t*       sector;         // What sector is affected by the moving platform
    fixed_t         speed;          // Speed that the platform moves at
    fixed_t         low;            // Height of the lowest floor surrounding the platform's sector
    fixed_t         high;           // Height of the highest floor surrounding the platform's sector
    int32_t         wait;           // How long the moving platform waits before returning to it's original position
    int32_t         count;          // Tick counter: how long of a wait there is left for the platform before it returns to it's original position
    plat_e          status;         // Current platform state (up/down/wait/paused)
    plat_e          oldstatus;      // Platform state before it was paused or put into stasis
    bool            crush;          // If true then the moving platform damages things which don't fit
    int32_t         tag;            // The tag for the line which activated this platform
    plattype_e      type;           // What type of behavior the moving platform has
};

// Maximum number of platforms there can be active at once
static constexpr int32_t MAXPLATS = 30;

extern plat_t* gpActivePlats[MAXPLATS];

bool EV_DoPlat(line_t& line, const plattype_e platType, const int32_t moveAmount) noexcept;
void P_ActivateInStasis(const int32_t tag) noexcept;
void EV_StopPlat(line_t& line) noexcept;
