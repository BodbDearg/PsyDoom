#pragma once

#include "Doom/doomdef.h"

void START_Legals() noexcept;
void STOP_Legals(const gameaction_t exitAction) noexcept;
gameaction_t TIC_Legals() noexcept;
void DRAW_Legals() noexcept;
