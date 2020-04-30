#pragma once

#include "Doom/doomdef.h"

enum statenum_t : uint32_t;
struct mapthing_t;

extern const VmPtr<int32_t>     gItemRespawnQueueHead;
extern const VmPtr<int32_t>     gItemRespawnQueueTail;
extern const VmPtr<int32_t>     gNumMObjKilled;

void P_RemoveMobj(mobj_t& mobj) noexcept;
void _thunk_P_RemoveMobj() noexcept; // TODO: remove eventually. Needed at the minute due to 'latecall' function pointer invocations of this function.

void P_RespawnSpecials() noexcept;
bool P_SetMObjState(mobj_t& mobj, const statenum_t stateNum) noexcept;

void P_ExplodeMissile(mobj_t& mobj) noexcept;
void _thunk_P_ExplodeMissile() noexcept; // TODO: remove eventually. Needed at the minute due to 'latecall' function pointer invocations of this function.

mobj_t* P_SpawnMObj(const fixed_t x, const fixed_t y, const fixed_t z, const mobjtype_t type) noexcept;
void P_SpawnPlayer() noexcept;
void P_SpawnMapThing(const mapthing_t& mapthing) noexcept;
void P_SpawnPuff(const fixed_t x, const fixed_t y, const fixed_t z) noexcept;
void P_SpawnBlood(const fixed_t x, const fixed_t y, const fixed_t z, const int32_t damage) noexcept;
void P_CheckMissileSpawn() noexcept;
void P_SpawnMissile() noexcept;
void P_SpawnPlayerMissile() noexcept;
