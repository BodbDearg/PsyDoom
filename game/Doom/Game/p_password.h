#pragma once

#include <cstdint>

enum skill_t : int32_t;
struct player_t;

void P_ComputePassword(uint8_t pOutput[10]) noexcept;
bool P_ProcessPassword(const uint8_t pPasswordIn[10], int32_t& mapNumOut, skill_t& skillOut, player_t* const pPlayer) noexcept;
