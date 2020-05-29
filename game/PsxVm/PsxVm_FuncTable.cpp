#include "PsxVm.h"

// TODO: Eventually remove use of this entire file
#include "Doom/Game/p_enemy.h"
#include "Doom/Game/p_mobj.h"
#include "Doom/Game/p_pspr.h"
#include <map>

struct ceiling_t;
struct delayaction_t;
struct fireflicker_t;
struct floormove_t;
struct glow_t;
struct lightflash_t;
struct plat_t;
struct strobe_t;
struct vldoor_t;

void G_CompleteLevel() noexcept;
void T_DelayedAction(delayaction_t& action) noexcept;
void T_FireFlicker(fireflicker_t& flicker) noexcept;
void T_Glow(glow_t& glow) noexcept;
void T_LightFlash(lightflash_t& lightFlash) noexcept;
void T_MoveCeiling(ceiling_t& ceiling) noexcept;
void T_MoveFloor(floormove_t& floor) noexcept;
void T_PlatRaise(plat_t& plat) noexcept;
void T_StrobeFlash(strobe_t& strobe) noexcept;
void T_VerticalDoor(vldoor_t& door) noexcept;

namespace PsxVm {
    std::map<uint32_t, VmFunc> gFuncTable = {
        { 0x80014A30, (VmFunc) &T_MoveCeiling },
        { 0x800152FC, (VmFunc) &T_VerticalDoor },
        { 0x800164B4, (VmFunc) &A_Look },
        { 0x800165E0, (VmFunc) &A_Chase },
        { 0x80016928, (VmFunc) &A_FaceTarget },
        { 0x800169CC, (VmFunc) &A_PosAttack },
        { 0x80016AD4, (VmFunc) &A_SPosAttack },
        { 0x80016C24, (VmFunc) &A_CPosAttack },
        { 0x80016D70, (VmFunc) &A_CPosRefire },
        { 0x80016E6C, (VmFunc) &A_SpidAttack },
        { 0x80016FBC, (VmFunc) &A_SpidRefire },
        { 0x800170BC, (VmFunc) &A_BspiAttack },
        { 0x80017170, (VmFunc) &A_TroopAttack },
        { 0x800172B0, (VmFunc) &A_SargAttack },
        { 0x80017380, (VmFunc) &A_HeadAttack },
        { 0x800174B4, (VmFunc) &A_CyberAttack },
        { 0x80017568, (VmFunc) &A_BruisAttack },
        { 0x80017630, (VmFunc) &A_SkelMissile },
        { 0x80017730, (VmFunc) &A_Tracer },
        { 0x80017980, (VmFunc) &A_SkelWhoosh },
        { 0x80017A30, (VmFunc) &A_SkelFist },
        { 0x80017B90, (VmFunc) &A_FatRaise },
        { 0x80017C40, (VmFunc) &A_FatAttack1 },
        { 0x80017D7C, (VmFunc) &A_FatAttack2 },
        { 0x80017EB8, (VmFunc) &A_FatAttack3 },
        { 0x8001804C, (VmFunc) &A_SkullAttack },
        { 0x80018350, (VmFunc) &A_PainAttack },
        { 0x80018520, (VmFunc) &A_PainDie },
        { 0x800188DC, (VmFunc) &A_Scream },
        { 0x80018994, (VmFunc) &A_XScream },
        { 0x800189B4, (VmFunc) &A_Pain },
        { 0x800189EC, (VmFunc) &A_Fall },
        { 0x80018A00, (VmFunc) &A_Explode },
        { 0x80018A24, (VmFunc) &A_BossDeath },
        { 0x80018C44, (VmFunc) &A_Hoof },
        { 0x80018C78, (VmFunc) &A_Metal },
        { 0x80018CAC, (VmFunc) &A_BabyMetal },
        { 0x80018CE0, (VmFunc) &L_MissileHit },
        { 0x80018D54, (VmFunc) &L_SkullBash },
        { 0x80019010, (VmFunc) &T_MoveFloor },
        { 0x8001AD74, (VmFunc) &T_FireFlicker },
        { 0x8001AE8C, (VmFunc) &T_LightFlash },
        { 0x8001AFBC, (VmFunc) &T_StrobeFlash },
        { 0x8001B4A0, (VmFunc) &T_Glow },
        { 0x8001C724, (VmFunc) &P_RemoveMobj },
        { 0x8001CB9C, (VmFunc) &P_ExplodeMissile },
        { 0x8001F280, (VmFunc) &T_PlatRaise },
        { 0x8001FFBC, (VmFunc) &P_FireWeapon },
        { 0x80020298, (VmFunc) &A_WeaponReady },
        { 0x80020480, (VmFunc) &A_ReFire },
        { 0x8002051C, (VmFunc) &A_CheckReload },
        { 0x8002053C, (VmFunc) &A_Lower },
        { 0x800206B4, (VmFunc) &A_Raise },
        { 0x800207A0, (VmFunc) &A_GunFlash },
        { 0x80020874, (VmFunc) &A_Punch },
        { 0x8002096C, (VmFunc) &A_Saw },
        { 0x80020AE4, (VmFunc) &A_FireMissile },
        { 0x80020B48, (VmFunc) &A_FireBFG },
        { 0x80020BAC, (VmFunc) &A_FirePlasma },
        { 0x80020DF0, (VmFunc) &A_FirePistol },
        { 0x80020F7C, (VmFunc) &A_FireShotgun },
        { 0x8002112C, (VmFunc) &A_FireShotgun2 },
        { 0x80021374, (VmFunc) &A_FireCGun },
        { 0x8002155C, (VmFunc) &A_Light0 },
        { 0x80021564, (VmFunc) &A_Light1 },
        { 0x80021570, (VmFunc) &A_Light2 },
        { 0x8002157C, (VmFunc) &A_BFGSpray },
        { 0x8002166C, (VmFunc) &A_BFGsound },
        { 0x80021690, (VmFunc) &A_OpenShotgun2 },
        { 0x800216B4, (VmFunc) &A_LoadShotgun2 },
        { 0x800216D8, (VmFunc) &A_CloseShotgun2 },
        { 0x80027718, (VmFunc) &T_DelayedAction },
        { 0x80013384, (VmFunc) &G_CompleteLevel },
    };
}
