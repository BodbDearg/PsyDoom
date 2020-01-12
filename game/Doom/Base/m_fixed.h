#pragma once

#include "Doom/doomdef.h"

fixed_t FixedMul(const fixed_t a, const fixed_t b) noexcept;
void _thunk_FixedMul() noexcept;

void FixedDiv() noexcept;
