#pragma once

#include "Doom/doomdef.h"

enum statenum_t : int32_t;
struct mapthing_t;

extern const VmPtr<int32_t>     gItemRespawnQueueHead;
extern const VmPtr<int32_t>     gItemRespawnQueueTail;
extern const VmPtr<int32_t>     gNumMObjKilled;

void P_RemoveMobj(mobj_t& mobj) noexcept;
void P_RespawnSpecials() noexcept;
bool P_SetMObjState(mobj_t& mobj, const statenum_t stateNum) noexcept;
void P_ExplodeMissile(mobj_t& mobj) noexcept;
mobj_t* P_SpawnMobj(const fixed_t x, const fixed_t y, const fixed_t z, const mobjtype_t type) noexcept;
void P_SpawnPlayer(const mapthing_t& mapThing) noexcept;
void P_SpawnMapThing(const mapthing_t& mapthing) noexcept;
void P_SpawnPuff(const fixed_t x, const fixed_t y, const fixed_t z) noexcept;
void P_SpawnBlood(const fixed_t x, const fixed_t y, const fixed_t z, const int32_t damage) noexcept;
void P_CheckMissileSpawn(mobj_t& mobj) noexcept;
mobj_t* P_SpawnMissile(mobj_t& source, mobj_t& dest, const mobjtype_t type) noexcept;
void P_SpawnPlayerMissile(mobj_t& source, const mobjtype_t missileType) noexcept;

void _thunk_P_RemoveMobj() noexcept; // TODO: remove eventually. Needed at the minute due to 'latecall' function pointer invocations of this function.
void _thunk_P_ExplodeMissile() noexcept;    // TODO: remove eventually. Needed at the minute due to 'latecall' function pointer invocations of this function.
