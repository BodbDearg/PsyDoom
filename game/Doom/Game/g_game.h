#pragma once

#include "Doom/doomdef.h"

extern VmPtr<gameaction_t>          gGameAction;
extern VmPtr<skill_t>               gGameSkill;
extern VmPtr<int32_t>               gGameMap;
extern VmPtr<player_t[MAXPLAYERS]>  gPlayers;
extern VmPtr<bool32_t[MAXPLAYERS]>  gbPlayerInGame;
extern VmPtr<int32_t>               gGameTic;
extern VmPtr<int32_t>               gPrevGameTic;
extern VmPtr<int32_t>               gLastTgtGameTicCount;
extern VmPtr<bool32_t>              gbDemoPlayback;
extern VmPtr<bool32_t>              gbDemoRecording;

void G_DoLoadLevel() noexcept;
void G_PlayerFinishLevel() noexcept;
void G_PlayerReborn() noexcept;
void G_DoReborn() noexcept;
void G_SetGameComplete() noexcept;
void G_InitNew() noexcept;
void G_RunGame() noexcept;
void G_PlayDemoPtr() noexcept;
void empty_func1() noexcept;
