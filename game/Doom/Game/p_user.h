#pragma once

#include "Doom/doomdef.h"

void P_PlayerMove(mobj_t& mobj) noexcept;
void P_PlayerXYMovement(mobj_t& mobj) noexcept;
void P_PlayerZMovement(mobj_t& mobj) noexcept;
void P_PlayerMobjThink(mobj_t& mobj) noexcept;
void P_BuildMove(player_t& player) noexcept;
void P_Thrust(player_t& player, const angle_t angle, const fixed_t amount) noexcept;
void P_CalcHeight() noexcept;
void P_MovePlayer(player_t& player) noexcept;
void P_DeathThink() noexcept;
void P_PlayerThink() noexcept;
