#pragma once

#include <cstdint>

struct line_t;

void EV_DoPlat() noexcept;
void P_ActivateInStasis(const int32_t tag) noexcept;
void EV_StopPlat(line_t& line) noexcept;
