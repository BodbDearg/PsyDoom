#pragma once

#include "Doom/doomdef.h"

extern const VmPtr<int32_t[NUMAMMO]>   gMaxAmmo;
extern const VmPtr<int32_t[NUMAMMO]>   gClipAmmo;

bool P_GiveAmmo(player_t& player, const ammotype_t ammoType, const int32_t numClips) noexcept;
bool P_GiveWeapon(player_t& player, const weapontype_t weapon, const bool bDropped) noexcept;
bool P_GiveBody(player_t& player, const int32_t healthAmt) noexcept;
bool P_GiveArmor(player_t& player, const int32_t armorType) noexcept;
void P_GiveCard(player_t& player, const card_t card) noexcept;
bool P_GivePower(player_t& player, const powertype_t power) noexcept;
void P_TouchSpecialThing(mobj_t& special, mobj_t& toucher) noexcept;
void P_KillMObj(mobj_t* const pKiller, mobj_t& target) noexcept;
void P_DamageMObj() noexcept;
