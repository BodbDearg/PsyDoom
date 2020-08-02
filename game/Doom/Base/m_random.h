#pragma once

#include <cstdint>

extern const uint8_t gRndTable[256];

int32_t P_Random() noexcept;
int32_t P_SubRandom() noexcept;
int32_t M_Random() noexcept;
void M_ClearRandom() noexcept;
