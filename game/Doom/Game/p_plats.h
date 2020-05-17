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
    thinker_t           thinker;
    VmPtr<sector_t>     sector;
    fixed_t             speed;
    fixed_t             low;            // TODO: COMMENT
    fixed_t             high;           // TODO: COMMENT
    int32_t             wait;           // TODO: COMMENT
    int32_t             count;          // TODO: COMMENT
    plat_e              status;         // TODO: COMMENT
    plat_e              oldstatus;      // TODO: COMMENT
    bool32_t            crush;
    int32_t             tag;            // TODO: COMMENT
    plattype_e          type;
};

static_assert(sizeof(plat_t) == 56);

// Maximum number of platforms there can be active at once
static constexpr int32_t MAXPLATS = 30;

extern const VmPtr<VmPtr<plat_t>[MAXPLATS]> gpActivePlats;

bool EV_DoPlat(line_t& line, const plattype_e platType, const int32_t moveAmount) noexcept;
void P_ActivateInStasis(const int32_t tag) noexcept;
void EV_StopPlat(line_t& line) noexcept;
