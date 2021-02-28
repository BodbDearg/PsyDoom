#pragma once

#if PSYDOOM_MODS

#include "Doom/doomdef.h"

void XOptions_Init() noexcept;
void XOptions_Shutdown(const gameaction_t exitAction) noexcept;
gameaction_t XOptions_Update() noexcept;
void XOptions_Draw() noexcept;

#endif  // #if PSYDOOM_MODS
