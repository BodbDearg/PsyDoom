#pragma once

#include "Doom/doomdef.h"

// PsyDoom: these are now defined in the 'MapInfo' module and can be overriden for new user maps
#if !PSYDOOM_MODS
    extern const char gMapNames_Doom[][32];
    extern const char gMapNames_FinalDoom[][32];
#endif

void IN_Start() noexcept;
void IN_Stop([[maybe_unused]] const gameaction_t exitAction) noexcept;
gameaction_t IN_Ticker() noexcept;
void IN_Drawer() noexcept;

void IN_SingleDrawer() noexcept;
void IN_CoopDrawer() noexcept;
void IN_DeathmatchDrawer() noexcept;
