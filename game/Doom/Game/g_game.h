#pragma once

#include "Doom/doomdef.h"

extern const VmPtr<gameaction_t>            gGameAction;
extern const VmPtr<skill_t>                 gGameSkill;
extern const VmPtr<gametype_t>              gNetGame;
extern const VmPtr<int32_t>                 gGameMap;
extern const VmPtr<int32_t>                 gNextMap;
extern const VmPtr<player_t[MAXPLAYERS]>    gPlayers;
extern const VmPtr<bool32_t[MAXPLAYERS]>    gbPlayerInGame;
extern const VmPtr<int32_t>                 gGameTic;
extern const VmPtr<int32_t>                 gPrevGameTic;
extern const VmPtr<int32_t>                 gLastTgtGameTicCount;
extern const VmPtr<bool32_t>                gbDemoPlayback;
extern const VmPtr<bool32_t>                gbDemoRecording;
extern const VmPtr<bool32_t>                gbIsLevelBeingRestarted;

void G_DoLoadLevel() noexcept;
void G_PlayerFinishLevel() noexcept;
void G_PlayerReborn() noexcept;
void G_DoReborn() noexcept;
void G_SetGameComplete() noexcept;
void G_InitNew(const skill_t skill, const int32_t mapNum, const gametype_t gameType) noexcept;
void G_RunGame() noexcept;
void G_PlayDemoPtr() noexcept;
void empty_func1() noexcept;
