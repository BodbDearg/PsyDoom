#pragma once

#include "Doom/doomdef.h"

void F1_Start() noexcept;

void F1_Stop(const gameaction_t exitAction) noexcept;
void _thunk_F1_Stop() noexcept;

void _thunk_F1_Ticker() noexcept;

void F1_Drawer() noexcept;
void F2_Start() noexcept;
void F2_Stop() noexcept;
void F2_Ticker() noexcept;
void F2_Drawer() noexcept;
