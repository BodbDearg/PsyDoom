#pragma once

#if PSYDOOM_MODS

#include "Doom/doomdef.h"

void SaveRoot_Init() noexcept;
void SaveRoot_Shutdown(const gameaction_t exitAction) noexcept;
gameaction_t SaveRoot_Update() noexcept;
void SaveRoot_Draw() noexcept;

#endif  // #if PSYDOOM_MODS
