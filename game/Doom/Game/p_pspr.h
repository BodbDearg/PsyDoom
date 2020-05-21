#pragma once

#include <cstdint>

enum statenum_t : int32_t;
struct player_t;
struct pspdef_t;

void P_SetPsprite(player_t& player, const int32_t spriteIdx, const statenum_t stateNum) noexcept;
void P_BringUpWeapon() noexcept;
void P_FireWeapon(player_t& player) noexcept;
void P_DropWeapon(player_t& player) noexcept;
void A_WeaponReady() noexcept;
void A_ReFire(player_t& player, pspdef_t& sprite) noexcept;
void A_CheckReload(player_t& player, pspdef_t& sprite) noexcept;
void A_Lower() noexcept;
void A_Raise() noexcept;
void A_GunFlash() noexcept;
void A_Punch() noexcept;
void A_Saw() noexcept;
void A_FireMissile() noexcept;
void A_FireBFG() noexcept;
void A_FirePlasma() noexcept;
void P_BulletSlope() noexcept;
void P_GunShot() noexcept;
void A_FirePistol() noexcept;
void A_FireShotgun() noexcept;
void A_FireShotgun2() noexcept;
void A_FireCGun() noexcept;
void A_Light0() noexcept;
void A_Light1() noexcept;
void A_Light2() noexcept;
void A_BFGSpray() noexcept;
void A_BFGsound() noexcept;
void A_OpenShotgun2() noexcept;
void A_LoadShotgun2() noexcept;
void A_CloseShotgun2() noexcept;
void P_SetupPsprites() noexcept;
void P_MovePsprites() noexcept;
