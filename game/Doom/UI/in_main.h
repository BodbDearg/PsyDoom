#pragma once

#include "Doom/doomdef.h"

extern const char gMapNames[][32];

void IN_Start() noexcept;
void IN_Stop([[maybe_unused]] const gameaction_t exitAction) noexcept;
gameaction_t IN_Ticker() noexcept;
void IN_Drawer() noexcept;

void IN_SingleDrawer() noexcept;
void IN_CoopDrawer() noexcept;
void IN_DeathmatchDrawer() noexcept;
