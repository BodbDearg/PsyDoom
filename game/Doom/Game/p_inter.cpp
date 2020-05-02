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

BEGIN_THIRD_PARTY_INCLUDES
    #include <algorithm>
END_THIRD_PARTY_INCLUDES


static constexpr uint32_t BONUSADD          = 4;    // How much to add to the 'bonus' effect strength counter anytime a bonus is picked up
static constexpr uint32_t MAX_DEAD_PLAYERS  = 32;   // Maximum number of dead player corpses to leave lying around (deathmatch)

const VmPtr<int32_t[NUMAMMO]>                   gMaxAmmo(0x800670D4);                       // The maximum amount of ammo for each ammo type
const VmPtr<int32_t[NUMAMMO]>                   gClipAmmo(0x800670E4);                      // How much ammo a clip for each ammo type gives
const VmPtr<uint32_t>                           gDeadPlayerRemovalQueueIdx(0x80078010);     // Next index in the dead player removal queue to use (this index is wrapped)
const VmPtr<VmPtr<mobj_t>[MAX_DEAD_PLAYERS]>    gDeadPlayerMobjRemovalQueue(0x800A876C);    // A queue of player corpses that eventually get removed from the game when a new corpse uses an occupied queue slot

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

//------------------------------------------------------------------------------------------------------------------------------------------
// Kill the specified map object and put it into the dead/gibbed state.
// Optionally a killer can be specified, for frag and kill stat tracking purposes.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_KillMObj(mobj_t* const pKiller, mobj_t& target) noexcept {
    // Target can no longer be shot, float or fly
    target.flags &= ~(MF_SHOOTABLE | MF_FLOAT | MF_SKULLFLY);
    
    // Gravity is now always applied to the thing's corpse, except for lost souls
    if (target.type != MT_SKULL) {
        target.flags &= ~MF_NOGRAVITY;
    }

    // Target is now a corpse and can fall off cliffs.
    // Also reduce bounding box height considerably - not sure why that matters now though.
    target.flags |= MF_CORPSE | MF_DROPOFF;
    target.height /= 4;
    
    // Do stat counting adjustments for the kill
    player_t* const pTargetPlayer = target.player.get();
    player_t* const pKillerPlayer = (pKiller) ? pKiller->player.get() : nullptr;
    
    if (pTargetPlayer) {
        // A player was killed: someone must get or lose a frag for this
        if (pKillerPlayer && (pKillerPlayer != pTargetPlayer)) {
            // Target killed by another player: credit the killer with a frag
            pKiller->player->frags++;            
        } else {
            // The target killed itself somehow, or was killed by something else other than a player: therefore loses a frag
            pTargetPlayer->frags--;
        }
    }
    else if (pKillerPlayer && (target.flags & MF_COUNTKILL)) {
        // A killable thing (monster) killed by a player deliberately: add to that player's kill count
        pKillerPlayer->killcount += 1;
    }
    else if ((*gNetGame == gt_single) && (target.flags & MF_COUNTKILL)) {
        // In single player all monster deaths are credited towards the player.
        // This is true even for kills caused by other monsters (infighting) and environmental stuff.
        gPlayers[0].killcount += 1;
    }
    
    // Player specific death logic
    bool bDoGibbing = false;

    if (pTargetPlayer) {
        // Player is no longer blocking and now dead: also drop whatever weapon is held
        target.flags &= ~MF_SOLID;
        pTargetPlayer->playerstate = PST_DEAD;
        P_DropWeapon(*pTargetPlayer);

        // Do gibbing if the damage if the player was killed by a large amount of damage.
        // Also play the player death sound.
        if (target.health < -50) {
            bDoGibbing = true;

            if (pTargetPlayer == &gPlayers[*gCurPlayerIndex]) {
                gStatusBar->gotgibbed = true;
            }

            S_StartSound(&target, sfx_slop);    // Gib death sound
        } else {
            S_StartSound(&target, sfx_pldeth);  // Normal death sound
        }
        
        // New for PSX: Remove a player corpse if too many are lying around, to help memory usage.
        // Save this new corpse in a circular queue also for later removal.
        if (*gDeadPlayerRemovalQueueIdx >= MAX_DEAD_PLAYERS) {
            P_RemoveMobj(*gDeadPlayerMobjRemovalQueue[*gDeadPlayerRemovalQueueIdx % MAX_DEAD_PLAYERS]);
        }

        gDeadPlayerMobjRemovalQueue[*gDeadPlayerRemovalQueueIdx % MAX_DEAD_PLAYERS] = &target;
        *gDeadPlayerRemovalQueueIdx += 1;
    }

    // Monster gib triggering: trigger if end health is less than the negative amount of starting health
    if (target.health < -target.info->spawnhealth) {
        bDoGibbing = true;
    }

    // Switch to the next map object state (dead, or gibbed) and randomly vary the tics in the state
    const bool bUseGibState = (bDoGibbing && (target.info->xdeathstate != S_NULL));
    const statenum_t nextStateNum = (bUseGibState) ? target.info->xdeathstate : target.info->deathstate;
    P_SetMObjState(target, nextStateNum);

    target.tics = std::max(target.tics - (P_Random() & 1), 1);
    
    // Do item dropping for the dead thing
    mobjtype_t dropItemType = {};

    switch (target.type) {
        case MT_SHOTGUY:    dropItemType = MT_SHOTGUN;  break;
        case MT_POSSESSED:  dropItemType = MT_CLIP;     break;
        case MT_CHAINGUY:   dropItemType = MT_CHAINGUN; break;

        default:
            break;
    }

    if (dropItemType != mobjtype_t{}) {
        mobj_t& droppedItem = *P_SpawnMObj(target.x, target.y, ONFLOORZ, dropItemType);
        droppedItem.flags |= MF_DROPPED;    // Less ammo for picking up dropped items
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to damage the specified map object by the given amount.
// Inflictor is the map object where the damage is coming from, the source is the player etc. that is responsible.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_DamageMObj(mobj_t& target, mobj_t* const pInflictor, mobj_t* const pSource, const int32_t baseDamageAmt) noexcept {
    // Ignore if the target is not shootable (shouldn't hit this case in practice)
    if ((target.flags & MF_SHOOTABLE) == 0)
        return;

    // Ignore if the target is already dead
    if (target.health <= 0)
        return;
    
    // Shooting a lost soul kills all it's velocity
    if (target.flags & MF_SKULLFLY) {
        target.momz = 0;
        target.momy = 0;
        target.momx = 0;
    }
    
    // Do adjustments to damage for the player based on skill and special faces due to damage
    player_t* const pTargetPlayer = target.player.get();
    player_t& curPlayer = gPlayers[*gCurPlayerIndex];

    int32_t damageAmt = baseDamageAmt;
    
    if (pTargetPlayer) {
        // In the lowest skill mode only half damage is applied
        if (*gGameSkill == sk_baby) {
            damageAmt /= 2;
        }

        // If more than 30 HP are taken then do the special shocked face for that
        if ((damageAmt > 30) && (pTargetPlayer == &curPlayer)) {
            gStatusBar->specialFace = f_hurtbad;
        }
    }

    // Apply force to the object being attacked, unless the attacker is the player and using a chainsaw
    const bool bPlayerChainsawAttack = (pSource && pSource->player && (pSource->player->readyweapon == wp_chainsaw));
    angle_t forceAngle;

    if (pInflictor && (!bPlayerChainsawAttack)) {
        // Figure out the angle and thrust to apply for the damage based on angle to attacker, mass and damage amount
        forceAngle = R_PointToAngle2(pInflictor->x, pInflictor->y, target.x, target.y);
        constexpr fixed_t FORCE_SCALE = 25 * FRACUNIT;
        fixed_t thrust = (damageAmt * FORCE_SCALE) / target.info->mass;
        
        // Randomly make enemies fall forward off ledges sometimes, if enough damage is done and its up above
        if ((damageAmt < 40) && (damageAmt > target.health) && (target.z - pInflictor->z > 64 * FRACUNIT)) {
            if (P_Random() & 1) {
                forceAngle += ANG180;
                thrust *= 4;
            }
        }

        // Apply the thrust
        const int32_t thrustInt = thrust >> FRACBITS;
        const uint32_t fineAngle = forceAngle >> ANGLETOFINESHIFT;

        target.momx += thrustInt * gFineCosine[fineAngle];
        target.momy += thrustInt * gFineSine[fineAngle];
        
        // PSX new logic: if it is the player then cap the accumulated velocity amount
        if (target.player) {
            constexpr fixed_t MAX_VELOCITY = 16 * FRACUNIT;
            target.momx = std::clamp(target.momx, -MAX_VELOCITY, MAX_VELOCITY);
            target.momy = std::clamp(target.momy, -MAX_VELOCITY, MAX_VELOCITY);
        }
    } else {
        forceAngle = target.angle;
    }

    // Player specific logic
    if (pTargetPlayer) {
        // No damage to the player if god mode or invulnerability is active
        if ((pTargetPlayer->cheats & CF_GODMODE) || pTargetPlayer->powers[pw_invulnerability])
            return;

        // Make the player face look towards the direction of damage
        if (pTargetPlayer == &curPlayer) {
            const angle_t angleToAttacker = forceAngle - target.angle;

            if ((angleToAttacker > ANG45 + ANG45 / 2) && (angleToAttacker < ANG180)) {
                gStatusBar->specialFace = f_faceright;
            }
            else if ((angleToAttacker > ANG180) && (angleToAttacker < ANG270 + ANG45 / 2)) {
                gStatusBar->specialFace = f_faceleft;
            }
        }

        // Do armor soaking up damage and being worn by damage
        if (pTargetPlayer->armortype != 0) {
            // Mega armor soaks up a bit more damage
            int32_t armorDamage = (pTargetPlayer->armortype == 1) ? damageAmt / 3 : damageAmt / 2;

            // Is the armor now exhausted? Cap the damage soakage if so and remove the armor:
            if (armorDamage >= pTargetPlayer->armorpoints) {
                pTargetPlayer->armortype = 0;
                armorDamage = pTargetPlayer->armorpoints;
            }

            // Decrease damage amount and apply to armor instead
            damageAmt -= armorDamage;
            pTargetPlayer->armorpoints -= armorDamage;
        }
        
        // Do player pain sound
        S_StartSound(&target, sfx_plpain);

        // Apply the damage to the player, set the attacker and increase the damage palette effect
        pTargetPlayer->health = std::max(pTargetPlayer->health - damageAmt, 0);
        pTargetPlayer->attacker = pSource;
        pTargetPlayer->damagecount += 1 + (damageAmt / 2);  // Tweak made in PSX version: add '1' here so that all damage causes a palette flash
    }

    // Decrease the map object health
    target.health -= damageAmt;

    if (target.health <= 0) {
        // Map object is now dead - kill it
        P_KillMObj(pSource, target);
    } else {
        // Map object still alive: do the pain state for the thing randomly
        if ((target.info->painchance > P_Random()) && ((target.flags & MF_SKULLFLY) == 0)) {
            // Doing pain: make the monster fight back and go into the pain state
            target.flags |= MF_JUSTHIT; 
            P_SetMObjState(target, target.info->painstate);
        }
        
        // Monster is fully awake now
        target.reactiontime = 0;

        // If not intent on another player target this attacking one
        if ((target.threshold == 0) && pSource) {
            target.target = pSource;
            target.threshold = BASETHRESHOLD;

            // Go into the see state if just woken
            if ((target.state == &gStates[target.info->spawnstate]) && (target.info->seestate != S_NULL)) {
                P_SetMObjState(target, target.info->seestate);
            }
        }
    }
}
