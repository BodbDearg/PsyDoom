#pragma once

struct line_t;

void P_InitSwitchList() noexcept;
void P_ChangeSwitchTexture(line_t& line, const bool bUseAgain) noexcept;
void P_UseSpecialLine() noexcept;
