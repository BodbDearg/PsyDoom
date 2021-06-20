#pragma once

#include <cstdint>

enum statenum_t : int32_t;
struct mobj_t;
struct player_t;
struct pspdef_t;

#if PSYDOOM_MODS
    void P_NoiseAlertToMobj(mobj_t& noiseMaker) noexcept;
#endif

void P_SetPsprite(player_t& player, const int32_t spriteIdx, const statenum_t stateNum) noexcept;
void P_FireWeapon(player_t& player) noexcept;
void P_DropWeapon(player_t& player) noexcept;
void A_WeaponReady(player_t& player, pspdef_t& sprite) noexcept;
void A_ReFire(player_t& player, pspdef_t& sprite) noexcept;
void A_CheckReload(player_t& player, pspdef_t& sprite) noexcept;
void A_Lower(player_t& player, pspdef_t& sprite) noexcept;
void A_Raise(player_t& player, pspdef_t& sprite) noexcept;
void A_GunFlash(player_t& player, pspdef_t& sprite) noexcept;
void A_Punch(player_t& player, pspdef_t& sprite) noexcept;
void A_Saw(player_t& player, pspdef_t& sprite) noexcept;
void A_FireMissile(player_t& player, pspdef_t& sprite) noexcept;
void A_FireBFG(player_t& player, pspdef_t& sprite) noexcept;
void A_FirePlasma(player_t& player, pspdef_t& sprite) noexcept;
void A_FirePistol(player_t& player, pspdef_t& sprite) noexcept;
void A_FireShotgun(player_t& player, pspdef_t& sprite) noexcept;
void A_FireShotgun2(player_t& player, pspdef_t& sprite) noexcept;
void A_FireCGun(player_t& player, pspdef_t& sprite) noexcept;
void A_Light0(player_t& player, pspdef_t& sprite) noexcept;
void A_Light1(player_t& player, pspdef_t& sprite) noexcept;
void A_Light2(player_t& player, pspdef_t& sprite) noexcept;
void A_BFGSpray(mobj_t& mobj) noexcept;
void A_BFGsound(player_t& player, pspdef_t& sprite) noexcept;
void A_OpenShotgun2(player_t& player, pspdef_t& sprite) noexcept;
void A_LoadShotgun2(player_t& player, pspdef_t& sprite) noexcept;
void A_CloseShotgun2(player_t& player, pspdef_t& sprite) noexcept;
void P_SetupPsprites(const int32_t playerIdx) noexcept;
void P_MovePsprites(player_t& player) noexcept;
