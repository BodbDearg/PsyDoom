#pragma once

#include "Doom/doomdef.h"

void START_Credits() noexcept;

void STOP_Credits(const gameaction_t exitAction) noexcept;
void _thunk_STOP_Credits() noexcept;

gameaction_t TIC_Credits() noexcept;
void _thunk_TIC_Credits() noexcept;

void DRAW_Credits() noexcept;
