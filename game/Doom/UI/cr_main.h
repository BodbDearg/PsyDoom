#pragma once

#include "Doom/doomdef.h"

void START_Credits() noexcept;
void STOP_Credits() noexcept;

gameaction_t TIC_Credits() noexcept;
void _thunk_TIC_Credits() noexcept;

void DRAW_Credits() noexcept;
