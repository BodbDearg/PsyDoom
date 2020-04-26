#pragma once

#include "PsxVm/VmPtr.h"

struct mobj_t;

extern const VmPtr<int32_t>     gItemRespawnQueueHead;
extern const VmPtr<int32_t>     gItemRespawnQueueTail;
extern const VmPtr<int32_t>     gNumMObjKilled;

void P_RemoveMObj(mobj_t& mobj) noexcept;
void P_RespawnSpecials() noexcept;
void P_SetMObjState() noexcept;
void P_ExplodeMissile() noexcept;
void P_SpawnMObj() noexcept;
void P_SpawnPlayer() noexcept;
void P_SpawnMapThing() noexcept;
void P_SpawnPuff() noexcept;
void P_SpawnBlood() noexcept;
void P_CheckMissileSpawn() noexcept;
void P_SpawnMissile() noexcept;
void P_SpawnPlayerMissile() noexcept;
