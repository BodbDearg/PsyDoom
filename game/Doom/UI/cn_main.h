#pragma once

#include "Doom/doomdef.h"

struct texture_t;

extern const VmPtr<texture_t> gTex_BUTTONS;

void START_ControlsScreen() noexcept;
void STOP_ControlsScreen(const gameaction_t exitAction) noexcept;
gameaction_t TIC_ControlsScreen() noexcept;
void DRAW_ControlsScreen() noexcept;
