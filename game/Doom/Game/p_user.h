#pragma once

struct mobj_t;
struct player_t;

void P_PlayerMove(mobj_t& mobj) noexcept;
void P_PlayerXYMovement(mobj_t& mobj) noexcept;
void P_PlayerZMovement(mobj_t& mobj) noexcept;
void P_PlayerMobjThink(mobj_t& mobj) noexcept;
void P_BuildMove(player_t& player) noexcept;
void P_Thrust() noexcept;
void P_CalcHeight() noexcept;
void P_MovePlayer() noexcept;
void P_DeathThink() noexcept;
void P_PlayerThink() noexcept;
