#pragma once

#include "Doom/doomdef.h"

#include <vector>

struct line_t;

// Identifies which part of a line/wall has a button texture on it (top / middle / bottom)
enum bwhere_e : int32_t {
    top,
    middle,
    bottom
};

// Used to hold state for a button which is 'active' and poised to switch back to it's original state after a short amount of time.
// Holds all of the info we need to revert the button back to it's prior state and when.
struct button_t {
    line_t*     line;           // The linedef which has the button/switch
    bwhere_e    where;          // What part of the line was activated
    int32_t     btexture;       // The texture to switch the line back to
    int32_t     btimer;         // The countdown for when the button reverts to it's former state; reverts when it reaches '0'
    mobj_t*     soundorg;       // When playing a sound to switch back, play it at this location
};

#if PSYDOOM_LIMIT_REMOVING
    extern std::vector<button_t> gButtonList;
#else
    // How many buttons can be active at a time (to be switched back to their original state).
    // This limit was inherited from PC DOOM where the thinking was that 4x buttons for 4x players was more than enough.
    static constexpr int32_t MAXBUTTONS = 16;

    extern button_t gButtonList[MAXBUTTONS];
#endif

#if PSYDOOM_MODS
    void P_InitSwitchDefs() noexcept;
#endif

void P_InitSwitchList() noexcept;
void P_ChangeSwitchTexture(line_t& line, const bool bUseAgain) noexcept;
bool P_UseSpecialLine(mobj_t& mobj, line_t& line) noexcept;

#if PSYDOOM_MODS
    uint32_t P_GetMaxButtons() noexcept;
#endif
