#pragma once

#include "Doom/doomdef.h"

extern const VmPtr<int32_t[NUMAMMO]>   gMaxAmmo;
extern const VmPtr<int32_t[NUMAMMO]>   gClipAmmo;

bool P_GiveAmmo(player_t& player, const ammotype_t ammoType, const int32_t numClips) noexcept;
bool P_GiveWeapon(player_t& player, const weapontype_t weapon, const bool bDropped) noexcept;
void P_GiveBody() noexcept;
void P_GiveArmor() noexcept;
void P_GiveCard() noexcept;
void P_GivePower() noexcept;
void P_TouchSpecialThing() noexcept;
void P_KillMObj() noexcept;
void P_DamageMObj() noexcept;
