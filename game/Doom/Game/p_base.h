#pragma once

#include "Doom/doomdef.h"

void P_RunMobjBase() noexcept;
void P_MobjThinker() noexcept;
bool PB_TryMove(const fixed_t tryX, const fixed_t tryY) noexcept;
