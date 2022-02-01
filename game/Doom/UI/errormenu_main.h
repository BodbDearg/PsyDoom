#pragma once

#if PSYDOOM_MODS

#include "Doom/doomdef.h"

gameaction_t RunNetErrorMenu(const char* const msg) noexcept;
gameaction_t RunNetErrorMenu_FailedToConnect() noexcept;
gameaction_t RunNetErrorMenu_GameTypeOrVersionMismatch() noexcept;

gameaction_t RunLoadGameErrorMenu(const char* const msg) noexcept;
gameaction_t RunLoadGameErrorMenu_BadFileId() noexcept;
gameaction_t RunLoadGameErrorMenu_BadFileVersion() noexcept;
gameaction_t RunLoadGameErrorMenu_BadMapNum() noexcept;
gameaction_t RunLoadGameErrorMenu_IOError() noexcept;
gameaction_t RunLoadGameErrorMenu_BadMapHash() noexcept;
gameaction_t RunLoadGameErrorMenu_BadMapData() noexcept;

gameaction_t RunDemoErrorMenu(const char* const msg) noexcept;
gameaction_t RunDemoErrorMenu_UnexpectedEOF() noexcept;
gameaction_t RunDemoErrorMenu_InvalidDemoVersion() noexcept;
gameaction_t RunDemoErrorMenu_InvalidSkill() noexcept;
gameaction_t RunDemoErrorMenu_InvalidMapNumber() noexcept;
gameaction_t RunDemoErrorMenu_InvalidGameType() noexcept;
gameaction_t RunDemoErrorMenu_InvalidPlayerNum() noexcept;
gameaction_t RunDemoErrorMenu_BadMapHash() noexcept;

void ErrorMenu_Init() noexcept;
void ErrorMenu_Shutdown(const gameaction_t exitAction) noexcept;
gameaction_t ErrorMenu_Update() noexcept;
void ErrorMenu_Draw() noexcept;

#endif  // #if PSYDOOM_MODS
