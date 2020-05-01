#include "p_inter.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/st_main.h"
#include "g_game.h"
#include "info.h"
#include "p_local.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "PsxVm/PsxVm.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <algorithm>
END_THIRD_PARTY_INCLUDES

// How much to add to the 'bonus' effect strength counter anytime a bonus is picked up
static constexpr uint32_t BONUSADD = 4;

// The maximum amount of ammo for each ammo type and how much ammo each clip type gives
const VmPtr<int32_t[NUMAMMO]>   gMaxAmmo(0x800670D4);
const VmPtr<int32_t[NUMAMMO]>   gClipAmmo(0x800670E4);

// Item message pickup strings.
//
// TODO: eventually make these be actual C++ string constants.
// Can't to do that at the moment since these pointers need to be referenced by a 'VmPtr<T>', hence must be inside the executable itself.
static const VmPtr<const char> STR_HealthBonusPickedUpMsg(0x800103E8);
static const VmPtr<const char> STR_ArmorBonusPickedUpMsg(0x80010404);
static const VmPtr<const char> STR_SuperChargePickedUpMsg(0x80010420);
static const VmPtr<const char> STR_MegaSpherePickedUpMsg(0x80010430);
static const VmPtr<const char> STR_ClipPickedUpMsg(0x80010440);
static const VmPtr<const char> STR_BoxOfBulletsPickedUpMsg(0x80010454);
static const VmPtr<const char> STR_RocketPickedUpMsg(0x80010470);
static const VmPtr<const char> STR_BoxOfRocketsPickedUpMsg(0x80010484);
static const VmPtr<const char> STR_EnergyCellPickedUpMsg(0x800104A0);
static const VmPtr<const char> STR_EnergyCellPackPickedUpMsg(0x800104BC);
static const VmPtr<const char> STR_FourShotgunShellsPickedUpMsg(0x800104DC);
static const VmPtr<const char> STR_BoxOfShotgunShellsPickedUpMsg(0x800104F8);
static const VmPtr<const char> STR_BackpackPickedUpMsg(0x80010514);
static const VmPtr<const char> STR_BfgPickedUpMsg(0x8001052C);
static const VmPtr<const char> STR_ChaingunPickedUpMsg(0x8001054C);
static const VmPtr<const char> STR_ChainsawPickedUpMsg(0x80010564);
static const VmPtr<const char> STR_RocketLauncherPickedUpMsg(0x80010584);
static const VmPtr<const char> STR_PlasmaGunPickedUpMsg(0x800105A4);
static const VmPtr<const char> STR_ShotgunPickedUpMsg(0x800105BC);
static const VmPtr<const char> STR_SuperShotgunPickedUpMsg(0x800105D4);
static const VmPtr<const char> STR_ArmorPickedUpMsg(0x800105F0);
static const VmPtr<const char> STR_MegaArmorPickedUpMsg(0x80010608);
static const VmPtr<const char> STR_BlueKeycardPickedUpMsg(0x80010620);
static const VmPtr<const char> STR_YellowKeycardPickedUpMsg(0x8001063C);
static const VmPtr<const char> STR_RedKeycardPickedUpMsg(0x8001065C);
static const VmPtr<const char> STR_BlueSkullKeyPickedUpMsg(0x80010678);
static const VmPtr<const char> STR_YellowSkullKeyPickedUpMsg(0x80010698);
static const VmPtr<const char> STR_RedSkullKeyPickedUpMsg(0x800106B8);
static const VmPtr<const char> STR_StimpackPickedUpMsg(0x800106D8);
static const VmPtr<const char> STR_NeededMedKitPickedUpMsg(0x800106F0);
static const VmPtr<const char> STR_MedKitPickedUpMsg(0x8001071C);
static const VmPtr<const char> STR_InvunerabilityPickedUpMsg(0x80010734);
static const VmPtr<const char> STR_BerserkPickedUpMsg(0x80010748);
static const VmPtr<const char> STR_PartialInvisibilityPickedUpMsg(0x80010754);
static const VmPtr<const char> STR_RadSuitPickedUpMsg(0x8001076C);
static const VmPtr<const char> STR_ComputerAreaMapPickedUpMsg(0x80010788);
static const VmPtr<const char> STR_LightAmplificationGogglesUpMsg(0x8001079C);

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to give the specified player the specified number of ammo clips of the given type.
// If the clip count is '0' then a half clip is given instead.
// Returns 'true' if ammo was actually given, 'false' if giving failed.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_GiveAmmo(player_t& player, const ammotype_t ammoType, const int32_t numClips) noexcept {
    // Can't give no ammo
    if (ammoType == am_noammo)
        return false;
    
    // Sanity check ammo type.
    // PC-PSX: don't accept 'NUMAMMO' as a valid ammo type here, that's not valid either.
    #if PC_PSX_DOOM_MODS
        const bool bValidAmmoType = (ammoType < NUMAMMO);
    #else
        const bool bValidAmmoType = (ammoType < am_noammo);
    #endif

    if (!bValidAmmoType) {
        I_Error("P_GiveAmmo: bad type %i", ammoType);
        return false;
    }

    // If you're already maxed out then you cannot pickup
    if (player.ammo[ammoType] == player.maxammo[ammoType])
        return false;

    // How much actual ammo are we adding?
    // If '0' clips is specified then interpret that as a half clip.
    // Double the ammo amount also for the lowest difficulty level...
    int32_t ammoToAdd;

    if (numClips == 0) {
        ammoToAdd = gClipAmmo[ammoType] / 2;
    } else {
        ammoToAdd = numClips * gClipAmmo[ammoType];
    }

    if (*gGameSkill == sk_baby) {
        ammoToAdd *= 2;
    }

    // Add the new ammo amount and cap at the max
    const int32_t oldAmmoAmt = player.ammo[ammoType];
    player.ammo[ammoType] += ammoToAdd;
    player.ammo[ammoType] = std::min(player.ammo[ammoType], player.maxammo[ammoType]);

    // Do weapon auto-switching logic if we just got some ammo for a weapon where we previously had none
    if (oldAmmoAmt != 0)
        return true;

    switch (ammoType) {
        case am_clip: {
            if (player.readyweapon == wp_fist) {
                player.pendingweapon = (player.weaponowned[wp_chaingun]) ? wp_chaingun : wp_pistol;
            }
        }   break;

        case am_shell: {
            if ((player.readyweapon == wp_fist) || (player.readyweapon == wp_pistol)) {
                if (player.weaponowned[wp_shotgun]) {
                    player.pendingweapon = wp_shotgun;
                }
            }
        }   break;

        case am_cell: {
            if ((player.readyweapon == wp_fist) || (player.readyweapon == wp_pistol)) {
                if (player.weaponowned[wp_plasma]) {
                    player.pendingweapon = wp_plasma;
                }
            }
        }   break;

        case am_misl: {
            if (player.readyweapon == wp_fist) {
                if (player.weaponowned[wp_missile]) {
                    player.pendingweapon = wp_missile;
                }
            }
        }   break;

        default:
            break;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to give the weapon type to the specified player, along with any ammo that results from picking up the weapon.
// The weapon type may be 'dropped' by monsters or placed in the level at the start, dropped weapons yield less ammo.
// Returns 'true' if the pickup succeeded, 'false' if pickup is not allowed. 
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_GiveWeapon(player_t& player, const weapontype_t weapon, const bool bDropped) noexcept {
    // In co-op mode only allow placed weapons to be picked up if not already owned
    if ((*gNetGame == gt_coop) && (!bDropped)) {
        if (player.weaponowned[weapon])
            return false;
        
        player.weaponowned[weapon] = true;
        P_GiveAmmo(player, gWeaponInfo[weapon].ammo, 2);
        player.pendingweapon = weapon;
        S_StartSound(player.mo.get(), sfx_wpnup);
        return false;
    }

    // Give ammo for the weapon pickup if possible
    bool bGaveAmmo = false;

    if (gWeaponInfo[weapon].ammo != am_noammo) {
        // Placed weapons give 2 clips, dropped weapons only give 1 clip
        if (bDropped) {
            bGaveAmmo = P_GiveAmmo(player, gWeaponInfo[weapon].ammo, 1);
        } else {
            bGaveAmmo = P_GiveAmmo(player, gWeaponInfo[weapon].ammo, 2);
        }
    }
    
    // Give the weapon if not already owned
    bool bGaveWeapon = false;

    if (!player.weaponowned[weapon]) {
        // Giving a new weapon: mark the weapon as owned and switch to it
        bGaveWeapon = true;
        player.weaponowned[weapon] = true;
        player.pendingweapon = weapon;

        // If this player is picking up the gun then switch to the evil grin status bar face
        if (&player == &gPlayers[*gCurPlayerIndex]) {
            gStatusBar->specialFace = f_gotgat;
        }
    }

    // Pickup was successful if ammo or a new weapon resulted from the pickup
    return (bGaveWeapon || bGaveAmmo);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to give the specified amount of health to the player: returns 'true' if that is possible/allowed
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_GiveBody(player_t& player, const int32_t healthAmt) noexcept {
    // Can't give health if already maxed out
    if (player.health >= MAXHEALTH)
        return false;

    player.health = std::min(player.health + healthAmt, MAXHEALTH);
    player.mo->health = player.health;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to give the specified armor type (1 = regular armor, 2 = mega armor) to the player.
// Returns 'true' if that is allowed/possible to do.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_GiveArmor(player_t& player, const int32_t armorType) noexcept {
    const int32_t armorAmt = armorType * 100;
    
    // Can only give the armor if it's more than the current armor amount
    if (armorAmt <= player.armorpoints)
        return false;
    
    player.armortype = armorType;
    player.armorpoints = armorAmt;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gives the specified keycard to the player if the key is not already owned
//------------------------------------------------------------------------------------------------------------------------------------------
void P_GiveCard(player_t& player, const card_t card) noexcept {
    if (!player.cards[card]) {
        player.bonuscount = BONUSADD;
        player.cards[card] = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Give the specified power to the player and return 'true' if successful
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_GivePower(player_t& player, const powertype_t power) noexcept {
    switch (power) {
        case pw_invulnerability:
            player.powers[power] = INVULNTICS;
            return true;

        case pw_strength:
            if (player.health < MAXHEALTH) {
                player.health = std::min(player.health + 100, MAXHEALTH);
                player.mo->health = player.health;
            }

            player.powers[power] = 1;
            return true;

        case pw_invisibility:
            player.powers[power] = INVISTICS;
            player.mo->flags |= MF_BLEND_ADD_25;
            return true;

        case pw_ironfeet:
            player.powers[power] = IRONTICS;
            return true;

        case pw_allmap: {
            if (player.powers[power])
                return false;

            player.powers[power] = 1;
            return true;
        }

        case pw_infrared:
            player.powers[power] = INFRATICS;
            return true;
    }

    return false;   // Invalid power
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does logic for touching items that can be picked up.
// Tries to make the given touching object pickup the special object.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_TouchSpecialThing(mobj_t& special, mobj_t& toucher) noexcept {
    // See if the thing is out of reach vertically: do not pickup if that is the case
    const fixed_t dz = special.z - toucher.z;

    if ((dz > toucher.height) || (dz < -8 * FRACUNIT))
        return;

    // Touching object cannot pickup if it is dead
    if (toucher.health <= 0)
        return;
    
    // See what was picked up and play the item pickup sound by default
    player_t& player = *toucher.player.get();
    sfxenum_t soundId = sfx_itemup;

    switch (special.sprite) {
        //----------------------------------------------------------------------------------------------------------------------------------
        // Keys
        //----------------------------------------------------------------------------------------------------------------------------------
        case SPR_BKEY: {
            if (!player.cards[it_bluecard]) {
                player.message = STR_BlueKeycardPickedUpMsg;
                P_GiveCard(player, it_bluecard);
            }

		    if (*gNetGame != gt_single)     // Leave it around in co-op games for other players
                return;

        }   break;

        case SPR_RKEY: {
            if (!player.cards[it_redcard]) {
                player.message = STR_RedKeycardPickedUpMsg;
                P_GiveCard(player, it_redcard);
            }

		    if (*gNetGame != gt_single)     // Leave it around in co-op games for other players
                return;

        }   break;

        case SPR_YKEY: {
            if (!player.cards[it_yellowcard]) {
                player.message = STR_YellowKeycardPickedUpMsg;
                P_GiveCard(player, it_yellowcard);
            }

		    if (*gNetGame != gt_single)     // Leave it around in co-op games for other players
                return;

        }   break;

        case SPR_BSKU: {
            if (!player.cards[it_blueskull]) {
                player.message = STR_BlueSkullKeyPickedUpMsg;
                P_GiveCard(player, it_blueskull);
            }

		    if (*gNetGame != gt_single)     // Leave it around in co-op games for other players
                return;

        }   break;

        case SPR_RSKU: {
            if (!player.cards[it_redskull]) {
                player.message = STR_RedSkullKeyPickedUpMsg;
                P_GiveCard(player, it_redskull);
            }

		    if (*gNetGame != gt_single)     // Leave it around in co-op games for other players
                return;

        }   break;

        case SPR_YSKU: {
            if (!player.cards[it_yellowskull]) {
                player.message = STR_YellowSkullKeyPickedUpMsg;
                P_GiveCard(player, it_yellowskull);
            }

		    if (*gNetGame != gt_single)     // Leave it around in co-op games for other players
                return;

        }   break;

        //----------------------------------------------------------------------------------------------------------------------------------
        // Health
        //----------------------------------------------------------------------------------------------------------------------------------
        case SPR_STIM: {
            if (!P_GiveBody(player, 10))
                return;

            player.message = STR_StimpackPickedUpMsg;
        }   break;

        case SPR_MEDI: {
            if (!P_GiveBody(player, 25))
                return;

            // Bug: the < 25 health case would never trigger here, because we've just given 25 health.
            // Looks like the same issue is in Linux Doom and probably other versions too...
            if (player.health < 25) {
                player.message = STR_NeededMedKitPickedUpMsg;
            } else {
                player.message = STR_MedKitPickedUpMsg;
            }
        }   break;

        //----------------------------------------------------------------------------------------------------------------------------------
        // Armors
        //----------------------------------------------------------------------------------------------------------------------------------
        case SPR_ARM1: {
            if (!P_GiveArmor(player, 1))
                return;
                
            player.message = STR_ArmorPickedUpMsg;
        }   break;

        case SPR_ARM2: {
            if (!P_GiveArmor(player, 2))
                return;

            player.message = STR_MegaArmorPickedUpMsg;
        }   break;

        //----------------------------------------------------------------------------------------------------------------------------------
        // Bonus items
        //----------------------------------------------------------------------------------------------------------------------------------
        case SPR_BON1: {
            player.health = std::min(player.health + 2, 200);
            player.mo->health = player.health;
            player.message = STR_HealthBonusPickedUpMsg;
        }   break;

        case SPR_BON2: {
            player.armorpoints = std::min(player.armorpoints + 2, 200);
            
            if (player.armortype == 0) {
                player.armortype = 1;
            }

            player.message = STR_ArmorBonusPickedUpMsg;
        }   break;

        case SPR_SOUL: {
            player.health = std::min(player.health + 100, 200);
            player.mo->health = player.health;
            player.message = STR_SuperChargePickedUpMsg;
            soundId = sfx_getpow;
        }   break;

        case SPR_MEGA: {
            player.health = 200;
            player.mo->health = 200;
            P_GiveArmor(player, 2);
            player.message = STR_MegaSpherePickedUpMsg;
            soundId = sfx_getpow;
        }   break;

        //----------------------------------------------------------------------------------------------------------------------------------
        // Powerups
        //----------------------------------------------------------------------------------------------------------------------------------
        case SPR_PINV: {
            P_GivePower(player, pw_invulnerability);
            player.message = STR_InvunerabilityPickedUpMsg;
            soundId = sfx_getpow;
        }   break;

        case SPR_PSTR: {
            P_GivePower(player, pw_strength);

            if (player.readyweapon != wp_fist) {
                player.pendingweapon = wp_fist;
            }

            player.message = STR_BerserkPickedUpMsg;
            soundId = sfx_getpow;
        }   break;

        case SPR_PINS: {
            P_GivePower(player, pw_invisibility);
            player.message = STR_PartialInvisibilityPickedUpMsg;
            soundId = sfx_getpow;
        }   break;

        case SPR_SUIT: {
            P_GivePower(player, pw_ironfeet);
            player.message = STR_RadSuitPickedUpMsg;
            soundId = sfx_getpow;
        }   break;

        case SPR_PMAP: {
		    if (!P_GivePower(player, pw_allmap))
			    return;
            
            player.message = STR_ComputerAreaMapPickedUpMsg;
            soundId = sfx_getpow;
        }   break;

        case SPR_PVIS: {
            P_GivePower(player, pw_infrared);
            player.message = STR_LightAmplificationGogglesUpMsg;
            soundId = sfx_getpow;
        }   break;

        //----------------------------------------------------------------------------------------------------------------------------------
        // Ammo
        //----------------------------------------------------------------------------------------------------------------------------------
        case SPR_CLIP: {
            if (!P_GiveAmmo(player, am_clip, (special.flags & MF_DROPPED) ? 0 : 1))
                return;

            player.message = STR_ClipPickedUpMsg;
        }   break;

        case SPR_AMMO: {
            if (!P_GiveAmmo(player, am_clip, 5))
                return;

            player.message = STR_BoxOfBulletsPickedUpMsg;
        }   break;
        
        case SPR_ROCK: {
            if (!P_GiveAmmo(player, am_misl, 1))
                return;

            player.message = STR_RocketPickedUpMsg;
        }   break;

        case SPR_BROK: {
            if (!P_GiveAmmo(player, am_misl, 5))
                return;

            player.message = STR_BoxOfRocketsPickedUpMsg;
        }   break;

        case SPR_CELL: {
            if (!P_GiveAmmo(player, am_cell, 1))
                return;
            
            player.message = STR_EnergyCellPickedUpMsg;
        }   break;

        case SPR_CELP: {
            if (!P_GiveAmmo(player, am_cell, 5))
                return;
            
            player.message = STR_EnergyCellPackPickedUpMsg;
        }   break;

        case SPR_SHEL: {
            if (!P_GiveAmmo(player, am_shell, 1))
                return;
            
            player.message = STR_FourShotgunShellsPickedUpMsg;
        }   break;

        case SPR_SBOX: {
            if (!P_GiveAmmo(player, am_shell, 5))
                return;
            
            player.message = STR_BoxOfShotgunShellsPickedUpMsg;
        }   break;

        case SPR_BPAK: {
            // A backpack doubles your ammo capacity the first time you get it
            if (!player.backpack) {
                for (uint32_t ammoTypeIdx = 0; ammoTypeIdx < NUMAMMO; ++ammoTypeIdx) {
                    player.maxammo[ammoTypeIdx] *= 2;
                }

                player.backpack = true;
            }

            // Give one clip of each ammo type
            for (uint32_t ammoTypeIdx = 0; ammoTypeIdx < NUMAMMO; ++ammoTypeIdx) {
                P_GiveAmmo(player, (ammotype_t) ammoTypeIdx, 1);
            }

            player.message = STR_BackpackPickedUpMsg;
        }   break;

        //----------------------------------------------------------------------------------------------------------------------------------
        // Weapons
        //----------------------------------------------------------------------------------------------------------------------------------
        case SPR_BFUG: {
            if (!P_GiveWeapon(player, wp_bfg, false))
                return;
            
            player.message = STR_BfgPickedUpMsg;
            soundId = sfx_wpnup;
        }   break;

        case SPR_MGUN: {
            if (!P_GiveWeapon(player, wp_chaingun, (special.flags & MF_DROPPED)))
                return;
            
            player.message = STR_ChaingunPickedUpMsg;
            soundId = sfx_wpnup;
        }   break;

        case SPR_CSAW: {
            if (!P_GiveWeapon(player, wp_chainsaw, false))
                return;
            
            player.message = STR_ChainsawPickedUpMsg;
            soundId = sfx_wpnup;
        }   break;

        case SPR_LAUN: {
            if (!P_GiveWeapon(player, wp_missile, false))
                return;
            
            player.message = STR_RocketLauncherPickedUpMsg;
            soundId = sfx_wpnup;
        }   break;

        case SPR_PLAS: {
            if (!P_GiveWeapon(player, wp_plasma, false))
                return;

            player.message = STR_PlasmaGunPickedUpMsg;
            soundId = sfx_wpnup;
        }   break;

        case SPR_SHOT: {
            if (!P_GiveWeapon(player, wp_shotgun, (special.flags & MF_DROPPED)))
                return;
            
            player.message = STR_ShotgunPickedUpMsg;
            soundId = sfx_wpnup;
        }   break;

        case SPR_SGN2: {
            if (!P_GiveWeapon(player, wp_supershotgun, (special.flags & MF_DROPPED)))
                return;

            player.message = STR_SuperShotgunPickedUpMsg;
            soundId = sfx_wpnup;
        }   break;
    }
    
    // Include the item in item count stats if the flags allow it
    if (special.flags & MF_COUNTITEM) {
        player.itemcount++;
    }
    
    // Remove the item on pickup and increase the bonus flash amount
    P_RemoveMobj(special);
    player.bonuscount += BONUSADD;

    // Note: because no sound origin is passed in here, the item pickup sound will ALWAYS play with reverb.
    // This is regardless of the reverb enabled setting of the sector. Unclear if this is a bug or if this intentional...
    if (&player == &gPlayers[*gCurPlayerIndex]) {
        S_StartSound(nullptr, soundId);
    }
}

void P_KillMObj() noexcept {
loc_8001A57C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a1;
    v1 = 0xFEFF0000;                                    // Result = FEFF0000
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x64);
    v1 |= 0xBFFB;                                       // Result = FEFFBFFB
    a1 = v0 & v1;
    v1 = lw(s0 + 0x54);
    v0 = 0xE;                                           // Result = 0000000E
    sw(a1, s0 + 0x64);
    if (v1 == v0) goto loc_8001A5BC;
    v0 = -0x201;                                        // Result = FFFFFDFF
    v0 &= a1;
    sw(v0, s0 + 0x64);
loc_8001A5BC:
    v0 = 0x100000;                                      // Result = 00100000
    v1 = lw(s0 + 0x64);
    v0 |= 0x400;                                        // Result = 00100400
    v1 |= v0;
    v0 = lw(s0 + 0x44);
    s1 = 0;                                             // Result = 00000000
    sw(v1, s0 + 0x64);
    v1 = lw(s0 + 0x80);
    v0 = u32(i32(v0) >> 2);
    sw(v0, s0 + 0x44);
    if (v1 == 0) goto loc_8001A730;
    if (a0 == 0) goto loc_8001A608;
    a0 = lw(a0 + 0x80);
    if (a0 == 0) goto loc_8001A608;
    if (a0 != v1) goto loc_8001A624;
loc_8001A608:
    v1 = lw(s0 + 0x80);
    v0 = lw(v1 + 0x64);
    v0--;
    sw(v0, v1 + 0x64);
    goto loc_8001A634;
loc_8001A624:
    v0 = lw(a0 + 0x64);
    v0++;
    sw(v0, a0 + 0x64);
loc_8001A634:
    v0 = lw(s0 + 0x64);
    v1 = -3;                                            // Result = FFFFFFFD
    v0 &= v1;
    v1 = lw(s0 + 0x80);
    sw(v0, s0 + 0x64);
    v0 = 1;                                             // Result = 00000001
    sw(v0, v1 + 0x4);
    a0 = lw(s0 + 0x80);
    P_DropWeapon();
    v0 = lw(s0 + 0x68);
    v0 = (i32(v0) < -0x32);
    a0 = s0;
    if (v0 == 0) goto loc_8001A6B8;
    v0 = *gCurPlayerIndex;
    a0 = lw(s0 + 0x80);
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    s1 = 1;                                             // Result = 00000001
    if (a0 != v0) goto loc_8001A6AC;
    at = 0x800A0000;                                    // Result = 800A0000
    sw(s1, at - 0x78CC);                                // Store to: gStatusBar[7] (80098734)
loc_8001A6AC:
    a0 = s0;
    a1 = 0x23;                                          // Result = 00000023
    goto loc_8001A6BC;
loc_8001A6B8:
    a1 = sfx_pldeth;
loc_8001A6BC:
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v1 = *gNumMObjKilled;
    v0 = (i32(v1) < 0x20);
    {
        const bool bJump = (v0 != 0);
        v0 = v1 & 0x1F;
        if (bJump) goto loc_8001A708;
    }
    v0 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7894;                                       // Result = gMObjPendingRemovalQueue[0] (800A876C)
    at += v0;
    a0 = lw(at);
    P_RemoveMobj(*vmAddrToPtr<mobj_t>(a0));
    v1 = *gNumMObjKilled;
    v0 = v1 & 0x1F;
loc_8001A708:
    v0 <<= 2;
    v1++;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7894;                                       // Result = gMObjPendingRemovalQueue[0] (800A876C)
    at += v0;
    sw(s0, at);
    *gNumMObjKilled = v1;
    goto loc_8001A7B0;
loc_8001A730:
    if (a0 == 0) goto loc_8001A770;
    a0 = lw(a0 + 0x80);
    v1 = 0x400000;                                      // Result = 00400000
    if (a0 == 0) goto loc_8001A770;
    v0 = lw(s0 + 0x64);
    v0 &= v1;
    if (v0 == 0) goto loc_8001A770;
    v0 = lw(a0 + 0xC8);
    v0++;
    sw(v0, a0 + 0xC8);
    goto loc_8001A7B0;
loc_8001A770:
    v0 = *gNetGame;
    v1 = 0x400000;
    if (v0 != gt_single) goto loc_8001A7B0;
    v0 = lw(s0 + 0x64);
    v0 &= v1;
    if (v0 == 0) goto loc_8001A7B0;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x774C;                                       // Result = gPlayer1[32] (800A88B4)
    v0 = lw(v1);                                        // Load from: gPlayer1[32] (800A88B4)
    v0++;
    sw(v0, v1);                                         // Store to: gPlayer1[32] (800A88B4)
loc_8001A7B0:
    if (s1 != 0) goto loc_8001A7D8;
    v0 = lw(s0 + 0x58);
    v0 = lw(v0 + 0x8);
    v1 = lw(s0 + 0x68);
    v0 = -v0;
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_8001A7F0;
loc_8001A7D8:
    v0 = lw(s0 + 0x58);
    a1 = lw(v0 + 0x34);
    if (a1 != 0) goto loc_8001A7FC;
loc_8001A7F0:
    v0 = lw(s0 + 0x58);
    a1 = lw(v0 + 0x30);
loc_8001A7FC:
    a0 = s0;
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
    _thunk_P_Random();
    v1 = lw(s0 + 0x5C);
    v0 &= 1;
    v1 -= v0;
    sw(v1, s0 + 0x5C);
    if (i32(v1) > 0) goto loc_8001A828;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0x5C);
loc_8001A828:
    v1 = lw(s0 + 0x54);
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 3);
        if (bJump) goto loc_8001A864;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001A850;
    }
    a3 = 0x35;                                          // Result = 00000035
    if (v1 == v0) goto loc_8001A868;
    goto loc_8001A888;
loc_8001A850:
    v0 = 8;                                             // Result = 00000008
    a3 = 0x3F;                                          // Result = 0000003F
    if (v1 == v0) goto loc_8001A868;
    goto loc_8001A888;
loc_8001A864:
    a3 = 0x43;                                          // Result = 00000043
loc_8001A868:
    a0 = lw(s0);
    a1 = lw(s0 + 0x4);
    a2 = 0x80000000;                                    // Result = 80000000
    v0 = ptrToVmAddr(P_SpawnMObj(a0, a1, a2, (mobjtype_t) a3));
    v1 = lw(v0 + 0x64);
    a0 = 0x20000;                                       // Result = 00020000
    v1 |= a0;
    sw(v1, v0 + 0x64);
loc_8001A888:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_DamageMObj() noexcept {
loc_8001A8A0:
    sp -= 0x30;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s5, sp + 0x24);
    s5 = a1;
    sw(s6, sp + 0x28);
    s6 = a2;
    sw(ra, sp + 0x2C);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v1 = lw(s0 + 0x64);
    v0 = v1 & 4;
    s3 = a3;
    if (v0 == 0) goto loc_8001AD48;
    v0 = lw(s0 + 0x68);
    {
        const bool bJump = (i32(v0) <= 0);
        v0 = 0x1000000;                                 // Result = 01000000
        if (bJump) goto loc_8001AD48;
    }
    v0 &= v1;
    if (v0 == 0) goto loc_8001A90C;
    sw(0, s0 + 0x50);
    sw(0, s0 + 0x4C);
    sw(0, s0 + 0x48);
loc_8001A90C:
    s2 = lw(s0 + 0x80);
    if (s2 == 0) goto loc_8001A97C;
    v0 = *gGameSkill;
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(s3) < 0x1F);
        if (bJump) goto loc_8001A938;
    }
    s3 = u32(i32(s3) >> 1);
    v0 = (i32(s3) < 0x1F);
loc_8001A938:
    if (v0 != 0) goto loc_8001A97C;
    v0 = *gCurPlayerIndex;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    {
        const bool bJump = (s2 != v0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_8001A97C;
    }
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78E8);                                // Store to: gStatusBar[0] (80098718)
loc_8001A97C:
    if (s5 == 0) goto loc_8001AB20;
    if (s6 == 0) goto loc_8001A9AC;
    v0 = lw(s6 + 0x80);
    if (v0 == 0) goto loc_8001A9AC;
    v1 = lw(v0 + 0x6C);
    v0 = 8;                                             // Result = 00000008
    if (v1 == v0) goto loc_8001AB20;
loc_8001A9AC:
    a0 = lw(s5);
    a1 = lw(s5 + 0x4);
    a2 = lw(s0);
    a3 = lw(s0 + 0x4);
    _thunk_R_PointToAngle2();
    s4 = v0;
    v0 = s3 << 1;
    v0 += s3;
    v0 <<= 3;
    v1 = lw(s0 + 0x58);
    v0 += s3;
    v1 = lw(v1 + 0x48);
    v0 <<= 16;
    div(v0, v1);
    if (v1 != 0) goto loc_8001A9F4;
    _break(0x1C00);
loc_8001A9F4:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001AA0C;
    }
    if (v0 != at) goto loc_8001AA0C;
    tge(zero, zero, 0x5D);
loc_8001AA0C:
    s1 = lo;
    v0 = (i32(s3) < 0x28);
    a0 = s4 >> 19;
    if (v0 == 0) goto loc_8001AA70;
    v0 = lw(s0 + 0x68);
    v0 = (i32(v0) < i32(s3));
    if (v0 == 0) goto loc_8001AA70;
    v0 = lw(s0 + 0x8);
    v1 = lw(s5 + 0x8);
    v0 -= v1;
    v1 = 0x400000;                                      // Result = 00400000
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_8001AA70;
    _thunk_P_Random();
    v0 &= 1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001AA6C;
    }
    s4 -= v0;
    s1 <<= 2;
loc_8001AA6C:
    a0 = s4 >> 19;
loc_8001AA70:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a0 <<= 2;
    v0 += a0;
    v0 = lw(v0);
    s1 = u32(i32(s1) >> 16);
    mult(s1, v0);
    v1 = lw(s0 + 0x48);
    v0 = lo;
    v0 += v1;
    sw(v0, s0 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += a0;
    v0 = lw(at);
    mult(s1, v0);
    v1 = lw(s0 + 0x4C);
    a0 = lw(s0 + 0x80);
    v0 = lo;
    v0 += v1;
    sw(v0, s0 + 0x4C);
    if (a0 == 0) goto loc_8001AB24;
    v1 = lw(s0 + 0x48);
    a0 = 0x100000;                                      // Result = 00100000
    v0 = (i32(a0) < i32(v1));
    if (v0 != 0) goto loc_8001AAF0;
    a0 = 0xFFF00000;                                    // Result = FFF00000
    v0 = (i32(v1) < i32(a0));
    if (v0 == 0) goto loc_8001AAF4;
loc_8001AAF0:
    sw(a0, s0 + 0x48);
loc_8001AAF4:
    v1 = lw(s0 + 0x4C);
    a0 = 0x100000;                                      // Result = 00100000
    v0 = (i32(a0) < i32(v1));
    if (v0 != 0) goto loc_8001AB18;
    a0 = 0xFFF00000;                                    // Result = FFF00000
    v0 = (i32(v1) < i32(a0));
    if (v0 == 0) goto loc_8001AB24;
loc_8001AB18:
    sw(a0, s0 + 0x4C);
    goto loc_8001AB24;
loc_8001AB20:
    s4 = lw(s0 + 0x24);
loc_8001AB24:
    if (s2 == 0) goto loc_8001AC70;
    v0 = lw(s2 + 0xC0);
    v0 &= 2;
    if (v0 != 0) goto loc_8001AD48;
    v0 = lw(s2 + 0x30);
    if (i32(v0) > 0) goto loc_8001AD48;
    v0 = *gCurPlayerIndex;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    {
        const bool bJump = (s2 != v0);
        v0 = 0xCFFF0000;                                // Result = CFFF0000
        if (bJump) goto loc_8001ABCC;
    }
    v0 |= 0xFFFF;                                       // Result = CFFFFFFF
    a0 = 0x4FFF0000;                                    // Result = 4FFF0000
    v1 = lw(s0 + 0x24);
    a0 |= 0xFFFE;                                       // Result = 4FFFFFFE
    s4 -= v1;
    v0 += s4;
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x7FFF0000;                                // Result = 7FFF0000
        if (bJump) goto loc_8001ABB0;
    }
    v0 = 4;                                             // Result = 00000004
    goto loc_8001ABC4;
loc_8001ABB0:
    v0 |= 0xFFFF;                                       // Result = 7FFFFFFF
    v0 += s4;
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = 3;                                         // Result = 00000003
        if (bJump) goto loc_8001ABCC;
    }
loc_8001ABC4:
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78E8);                                // Store to: gStatusBar[0] (80098718)
loc_8001ABCC:
    v1 = lw(s2 + 0x2C);
    v0 = 1;                                             // Result = 00000001
    if (v1 == 0) goto loc_8001AC34;
    {
        const bool bJump = (v1 != v0);
        v0 = s3 >> 31;
        if (bJump) goto loc_8001AC00;
    }
    v0 = 0x55550000;                                    // Result = 55550000
    v0 |= 0x5556;                                       // Result = 55555556
    mult(s3, v0);
    v1 = u32(i32(s3) >> 31);
    v0 = hi;
    v1 = v0 - v1;
    goto loc_8001AC08;
loc_8001AC00:
    v0 += s3;
    v1 = u32(i32(v0) >> 1);
loc_8001AC08:
    a0 = lw(s2 + 0x28);
    v0 = (i32(v1) < i32(a0));
    if (v0 != 0) goto loc_8001AC24;
    v1 = a0;
    sw(0, s2 + 0x2C);
loc_8001AC24:
    v0 = lw(s2 + 0x28);
    s3 -= v1;
    v0 -= v1;
    sw(v0, s2 + 0x28);
loc_8001AC34:
    a0 = s0;
    a1 = sfx_plpain;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = lw(s2 + 0x24);
    v0 -= s3;
    sw(v0, s2 + 0x24);
    if (i32(v0) >= 0) goto loc_8001AC58;
    sw(0, s2 + 0x24);
loc_8001AC58:
    v0 = lw(s2 + 0xD8);
    v1 = u32(i32(s3) >> 1);
    sw(s6, s2 + 0xE0);
    v0++;
    v0 += v1;
    sw(v0, s2 + 0xD8);
loc_8001AC70:
    v0 = lw(s0 + 0x68);
    v0 -= s3;
    sw(v0, s0 + 0x68);
    if (i32(v0) > 0) goto loc_8001AC98;
    a0 = s6;
    a1 = s0;
    P_KillMObj();
    goto loc_8001AD48;
loc_8001AC98:
    _thunk_P_Random();
    v1 = lw(s0 + 0x58);
    v1 = lw(v1 + 0x20);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0x1000000;                                 // Result = 01000000
        if (bJump) goto loc_8001ACE4;
    }
    a0 = lw(s0 + 0x64);
    v0 &= a0;
    {
        const bool bJump = (v0 != 0);
        v0 = a0 | 0x40;
        if (bJump) goto loc_8001ACE4;
    }
    v1 = lw(s0 + 0x58);
    sw(v0, s0 + 0x64);
    a1 = lw(v1 + 0x1C);
    a0 = s0;
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
loc_8001ACE4:
    v0 = lw(s0 + 0x7C);
    sw(0, s0 + 0x78);
    if (v0 != 0) goto loc_8001AD48;
    v0 = 0x64;                                          // Result = 00000064
    if (s6 == 0) goto loc_8001AD48;
    a1 = lw(s0 + 0x58);
    sw(s6, s0 + 0x74);
    sw(v0, s0 + 0x7C);
    v1 = lw(a1 + 0x4);
    a0 = lw(s0 + 0x60);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    if (a0 != v0) goto loc_8001AD48;
    a1 = lw(a1 + 0xC);
    if (a1 == 0) goto loc_8001AD48;
    a0 = s0;
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
loc_8001AD48:
    ra = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x30;
    return;
}
