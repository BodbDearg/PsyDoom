#pragma once

#include "Doom/doomdef.h"

extern const VmPtr<int32_t[NUMAMMO]>   gMaxAmmo;
extern const VmPtr<int32_t[NUMAMMO]>   gClipAmmo;

void P_GiveAmmo() noexcept;
void P_GiveWeapon() noexcept;
void P_GiveBody() noexcept;
void P_GiveArmor() noexcept;
void P_GiveCard() noexcept;
void P_GivePower() noexcept;
void P_TouchSpecialThing() noexcept;
void P_KillMObj() noexcept;
void P_DamageMObj() noexcept;
