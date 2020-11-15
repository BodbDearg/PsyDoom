#pragma once

#if PSYDOOM_MODS

#include "Doom/doomdef.h"

void Controls_Init() noexcept;
void Controls_Shutdown(const gameaction_t exitAction) noexcept;
gameaction_t Controls_Update() noexcept;
void Controls_Draw() noexcept;

#endif  // #if PSYDOOM_MODS
