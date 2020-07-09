#pragma once

// PC-PSX: no longer using the controller configuration screen
#if !PC_PSX_DOOM_MODS

#include "Doom/doomdef.h"

struct texture_t;

extern texture_t gTex_BUTTONS;

void START_ControlsScreen() noexcept;
void STOP_ControlsScreen(const gameaction_t exitAction) noexcept;
gameaction_t TIC_ControlsScreen() noexcept;
void DRAW_ControlsScreen() noexcept;

#endif  // #if !PC_PSX_DOOM_MODS
