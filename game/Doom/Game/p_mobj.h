#pragma once

#include "Doom/doomdef.h"

enum statenum_t : uint32_t;

extern const VmPtr<int32_t>     gItemRespawnQueueHead;
extern const VmPtr<int32_t>     gItemRespawnQueueTail;
extern const VmPtr<int32_t>     gNumMObjKilled;

void P_RemoveMObj(mobj_t& mobj) noexcept;
void P_RespawnSpecials() noexcept;
bool P_SetMObjState(mobj_t& mobj, const statenum_t stateNum) noexcept;
void P_ExplodeMissile(mobj_t& mobj) noexcept;
mobj_t* P_SpawnMObj(const fixed_t x, const fixed_t y, const fixed_t z, const mobjtype_t type) noexcept;
void P_SpawnPlayer() noexcept;
void P_SpawnMapThing() noexcept;
void P_SpawnPuff() noexcept;
void P_SpawnBlood() noexcept;
void P_CheckMissileSpawn() noexcept;
void P_SpawnMissile() noexcept;
void P_SpawnPlayerMissile() noexcept;
