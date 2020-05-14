#pragma once

struct line_t;
struct mobj_t;

void P_InitSwitchList() noexcept;
void P_ChangeSwitchTexture(line_t& line, const bool bUseAgain) noexcept;
bool P_UseSpecialLine(mobj_t& mobj, line_t& line) noexcept;
