#pragma once

#include "Doom/doomdef.h"

void M_ClearBox(fixed_t* const pBox) noexcept;
void M_AddToBox(fixed_t* const pBox, const fixed_t x, const fixed_t y) noexcept;
void M_AddPointToBox() noexcept;
