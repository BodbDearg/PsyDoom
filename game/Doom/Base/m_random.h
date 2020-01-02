#pragma once

#include <cstdint>

int32_t P_Random() noexcept;
void _thunk_P_Random() noexcept;

int32_t M_Random() noexcept;
void _thunk_M_Random() noexcept;

void M_ClearRandom() noexcept;
