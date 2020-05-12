#pragma once

struct line_t;

void T_PlatRaise() noexcept;
void EV_DoPlat() noexcept;
void P_ActivateInStasis() noexcept;
void EV_StopPlat(line_t& line) noexcept;
