#pragma once

#if PSYDOOM_MODS

#include "Doom/doomdef.h"

extern const char* gNetErrorMenuMsg;

gameaction_t RunNetErrorMenu_FailedToConnect() noexcept;
gameaction_t RunNetErrorMenu_GameTypeOrVersionMismatch() noexcept;
gameaction_t RunNetErrorMenu(const char* const msg) noexcept;

void NetError_Init() noexcept;
void NetError_Shutdown(const gameaction_t exitAction) noexcept;
gameaction_t NetError_Update() noexcept;
void NetError_Draw() noexcept;

#endif  // #if PSYDOOM_MODS
