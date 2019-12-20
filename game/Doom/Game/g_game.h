#pragma once

#include "Doom/doomdef.h"

extern const VmPtr<gameaction_t>            gGameAction;
extern const VmPtr<skill_t>                 gGameSkill;
extern const VmPtr<int32_t>                 gGameMap;
extern const VmPtr<player_t[MAXPLAYERS]>    gPlayers;
extern const VmPtr<bool32_t[MAXPLAYERS]>    gbPlayerInGame;
extern const VmPtr<int32_t>                 gGameTic;
extern const VmPtr<int32_t>                 gPrevGameTic;
extern const VmPtr<int32_t>                 gLastTgtGameTicCount;
extern const VmPtr<bool32_t>                gbDemoPlayback;
extern const VmPtr<bool32_t>                gbDemoRecording;

void G_DoLoadLevel() noexcept;
void G_PlayerFinishLevel() noexcept;
void G_PlayerReborn() noexcept;
void G_DoReborn() noexcept;
void G_SetGameComplete() noexcept;
void G_InitNew() noexcept;
void G_RunGame() noexcept;
void G_PlayDemoPtr() noexcept;
void empty_func1() noexcept;
