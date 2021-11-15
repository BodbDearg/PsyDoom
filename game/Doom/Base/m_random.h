#pragma once

#include <cstdint>

extern const uint8_t    gRndTable[256];
extern uint32_t         gPRndIndex;
extern uint32_t         gMRndIndex;

int32_t P_Random() noexcept;
int32_t P_SubRandom() noexcept;
int32_t M_Random() noexcept;
void M_ClearRandom() noexcept;
