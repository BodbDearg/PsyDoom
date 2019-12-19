#pragma once

#include "Doom/doomdef.h"

extern VmPtr<int32_t>               gGameMap;
extern VmPtr<player_t[MAXPLAYERS]>  gPlayers;
extern VmPtr<bool32_t[MAXPLAYERS]>  gbPlayerInGame;

void G_DoLoadLevel() noexcept;
void G_PlayerFinishLevel() noexcept;
void G_PlayerReborn() noexcept;
void G_DoReborn() noexcept;
void G_SetGameComplete() noexcept;
void G_InitNew() noexcept;
void G_RunGame() noexcept;
void G_PlayDemoPtr() noexcept;
void empty_func1() noexcept;
