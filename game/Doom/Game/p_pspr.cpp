#include "p_pspr.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/w_wad.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "g_game.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_tick.h"
#include "PcPsx/Assert.h"
#include "PsxVm/PsxVm.h"

const weaponinfo_t gWeaponInfo[NUMWEAPONS] = {
    {   // Fist
        am_noammo,              // ammo
        S_PUNCHUP,              // upstate
        S_PUNCHDOWN,            // downstate
        S_PUNCH,                // readystate
        S_PUNCH1,               // atkstate
        S_NULL                  // flashstate
    },
    {   // Pistol
        am_clip,                // ammo
        S_PISTOLUP,             // upstate
        S_PISTOLDOWN,           // downstate
        S_PISTOL,               // readystate
        S_PISTOL2,              // atkstate
        S_PISTOLFLASH           // flashstate
    },
    {   // Shotgun
        am_shell,               // ammo
        S_SGUNUP,               // upstate
        S_SGUNDOWN,             // downstate
        S_SGUN,                 // readystate
        S_SGUN2,                // atkstate
        S_SGUNFLASH1            // flashstate
    },
    {   // Super Shotgun
        am_shell,               // ammo
        S_DSGUNUP,              // upstate
        S_DSGUNDOWN,            // downstate
        S_DSGUN,                // readystate
        S_DSGUN1,               // atkstate
        S_DSGUNFLASH1           // flashstate
    },
    {   // Chaingun
        am_clip,                // ammo
        S_CHAINUP,              // upstate
        S_CHAINDOWN,            // downstate
        S_CHAIN,                // readystate
        S_CHAIN1,               // atkstate
        S_CHAINFLASH1           // flashstate
    },
    {   // Rocket Launcher
        am_misl,                // ammo
        S_MISSILEUP,            // upstate
        S_MISSILEDOWN,          // downstate
        S_MISSILE,              // readystate
        S_MISSILE1,             // atkstate
        S_MISSILEFLASH1         // flashstate
    },
    {   // Plasma Rifle
        am_cell,                // ammo
        S_PLASMAUP,             // upstate
        S_PLASMADOWN,           // downstate
        S_PLASMA,               // readystate
        S_PLASMA1,              // atkstate
        S_PLASMAFLASH1          // flashstate
    },
    {   // BFG
        am_cell,                // ammo
        S_BFGUP,                // upstate
        S_BFGDOWN,              // downstate
        S_BFG,                  // readystate
        S_BFG1,                 // atkstate
        S_BFGFLASH1             // flashstate
    },
    {   // Chainsaw
        am_noammo,              // ammo
        S_SAWUP,                // upstate
        S_SAWDOWN,              // downstate
        S_SAW,                  // readystate
        S_SAW1,                 // atkstate
        S_NULL                  // flashstate
    }
};

static constexpr int32_t BFGCELLS       = 40;               // Number of cells in a BFG shot
static constexpr int32_t LOWERSPEED     = 12 * FRACUNIT;    // Speed of weapon lowering (pixels)
static constexpr int32_t RAISESPEED     = 12 * FRACUNIT;    // Speed of weapon raising (pixels)
static constexpr int32_t WEAPONX        = 1 * FRACUNIT;     // X offset for weapons
static constexpr int32_t WEAPONBOTTOM   = 96 * FRACUNIT;    // Y offset for weapons when they are lowered
static constexpr int32_t WEAPONTOP      = 0 * FRACUNIT;     // Y offset for weapons when they are raised

static mobj_t*  gpSoundTarget;                  // The current thing making noise
static fixed_t  gBulletSlope;                   // Vertical aiming slope for shooting: computed by 'P_BulletSlope'
static int32_t  gTicRemainder[MAXPLAYERS];      // How many unsimulated player sprite 60 Hz vblanks there are

//------------------------------------------------------------------------------------------------------------------------------------------
// Recursively flood fill sound starting from the given sector to other neighboring sectors considered valid for sound transfer.
// Aside from closed doors, walls etc. stopping sound propagation sound will also be stopped after two sets of 'ML_SOUNDBLOCK' 
// lines are encountered.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_RecursiveSound(sector_t& sector, const bool bStopOnSoundBlock) noexcept {
    // Don't flood the sector if it's already done and it didn't have sound coming into it blocked
    const int32_t soundTraversed = (bStopOnSoundBlock) ? 2 : 1;

    if ((sector.validcount == gValidCount) && (sector.soundtraversed <= soundTraversed))
        return;
    
    // Flood fill this sector and save the thing that made noise and whether sound was blocked
    sector.validcount = gValidCount;
    sector.soundtraversed = soundTraversed;
    sector.soundtarget = gpSoundTarget;

    // Recurse into adjoining sectors and flood fill with noise
    for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
        line_t& line = *sector.lines[lineIdx];
        sector_t* const pBackSector = line.backsector;

        // Sound can't pass single sided lines
        if (!pBackSector)
            continue;
        
        sector_t& frontSector = *line.frontsector;

        // If the sector is a closed door then sound can't pass through it
        if (frontSector.floorheight >= pBackSector->ceilingheight)
            continue;

        if (frontSector.ceilingheight <= pBackSector->floorheight)
            continue;

        // Need to recurse into the sector on the opposite side of this sector's line
        sector_t& checkSector = (&frontSector == &sector) ? *pBackSector : frontSector;
        
        if (line.flags & ML_SOUNDBLOCK) {
            if (!bStopOnSoundBlock) {
                P_RecursiveSound(checkSector, true);
            }
        }
        else {
            P_RecursiveSound(checkSector, bStopOnSoundBlock);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called after using weapons: make noise to alert sleeping monsters of the given player
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_NoiseAlert(player_t& player) noexcept {
    // Optimization: don't bother doing a noise alert again if we are still in the same sector as the last one
    mobj_t& playerMobj = *player.mo;
    sector_t& curSector = *playerMobj.subsector->sector;

    if (player.lastsoundsector == &curSector) 
        return;

    player.lastsoundsector = &curSector;
    gValidCount++;
    gpSoundTarget = &playerMobj;

    // Recursively flood fill sectors with sound
    P_RecursiveSound(curSector, false);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the specified player sprite to the given state and invoke the state action.
// Note that if the state is over in an instant, multiple states might be transitioned to.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SetPsprite(player_t& player, const int32_t spriteIdx, const statenum_t stateNum) noexcept {
    pspdef_t& sprite = player.psprites[spriteIdx];
    statenum_t nextStateNum = stateNum;

    do {
        // Did the object remove itself?
        if (nextStateNum == S_NULL) {
            sprite.state = nullptr;
            return;
        }

        // Advance to the next state
        state_t& state = gStates[nextStateNum];
        sprite.state = &state;
        sprite.tics = state.tics;

        // Perform the state action
        if (state.action) {
            state.action(player, sprite);

            // Finish if we no longer have a state
            if (!sprite.state)
                break;
        }
        
        // Execute the next state if the tics left is zero (state is an instant cycle through)
        nextStateNum = sprite.state->nextstate;

    }  while (sprite.tics == 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begins raising the player's current pending weapon from the bottom of the screen.
// If there is no pending weapon, then the current weapon is raised instead - which is assumed to be a weapon we just switched to.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_BringUpWeapon(player_t& player) noexcept {
    // If there is no pending weapon for some reason use the ready weapon as the pending one
    if (player.pendingweapon == wp_nochange) {
        player.pendingweapon = player.readyweapon;
    }
    
    // If we're raising the chainsaw then play its up sound.
    // Exception: don't do this on level start.
    if ((player.pendingweapon == wp_chainsaw) && gbIsLevelDataCached) {
        S_StartSound(player.mo, sfx_sawup);
    }
    
    // No longer have a pending weapon but remember what it was (for what comes next)
    const statenum_t nextWeaponState = gWeaponInfo[player.pendingweapon].upstate;
    player.pendingweapon = wp_nochange;

    // Put the weapon sprite beyond the bottom of the screen and start raising it (go into the 'up' state)
    player.psprites[ps_weapon].sx = WEAPONX;
    player.psprites[ps_weapon].sy = WEAPONBOTTOM;
    P_SetPsprite(player, ps_weapon, nextWeaponState);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check if the player has enough ammo to fire the current weapon and return 'true' if that is case.
// If there is not enough ammo then this function also attempts to switch to an appropriate weapon with ammo.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool P_CheckAmmo(player_t& player) noexcept {
    // Get how much ammo the shot takes
    int32_t ammoForShot;

    if (player.readyweapon == wp_bfg) {
        ammoForShot = BFGCELLS;
    } else if (player.readyweapon == wp_supershotgun) {
        ammoForShot = 2;    // Double barrel shotgun
    } else {
        ammoForShot = 1;
    }

    // Can shoot if the weapon has no ammo or if we have enough ammo for the shot
    const ammotype_t ammoType = gWeaponInfo[player.readyweapon].ammo;

    if ((ammoType == am_noammo) || (player.ammo[ammoType] >= ammoForShot))
        return true;

    // Not enough ammo: figure out what weapon to switch to next
    if (player.weaponowned[wp_plasma] && (player.ammo[am_cell] != 0)) {
        player.pendingweapon = wp_plasma;
    }
    else if (player.weaponowned[wp_supershotgun] && (player.ammo[am_shell] > 2)) {  // Bug? Won't switch when ammo is '2', even though a shot can be taken...
        player.pendingweapon = wp_supershotgun;
    }
    else if (player.weaponowned[wp_chaingun] && (player.ammo[am_clip] != 0)) {
        player.pendingweapon = wp_chaingun;
    }
    else if (player.weaponowned[wp_shotgun] && (player.ammo[am_shell] != 0)) {
        player.pendingweapon = wp_shotgun;
    }
    else if (player.ammo[am_clip] != 0) {
        player.pendingweapon = wp_pistol;
    }
    else if (player.weaponowned[wp_chainsaw]) {
        player.pendingweapon = wp_chainsaw;
    } 
    else if (player.weaponowned[wp_missile] && (player.ammo[am_misl] != 0)) {
        player.pendingweapon = wp_missile;
    }
    else if (player.weaponowned[wp_bfg] && (player.ammo[am_cell] > BFGCELLS)) {     // Bug? Won't switch when ammo is 'BFGCELLS', even though a shot can be taken...
        player.pendingweapon = wp_bfg;
    }
    else {
        player.pendingweapon = wp_fist;
    }
    
    // Start lowering the current weapon
    P_SetPsprite(player, ps_weapon, gWeaponInfo[player.readyweapon].downstate);
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fires the player's weapon
//------------------------------------------------------------------------------------------------------------------------------------------
void P_FireWeapon(player_t& player) noexcept {
    // If there is not enough ammo then you can't fire
    if (!P_CheckAmmo(player))
        return;
    
    // Player is now in the attacking state
    P_SetMObjState(*player.mo, S_PLAY_ATK1);

    // Switch the player sprite into the attacking state and ensure the weapon sprite offset is correct
    pspdef_t& weaponSprite = player.psprites[ps_weapon];
    weaponSprite.sx = WEAPONX;
    weaponSprite.sy = WEAPONTOP;

    P_SetPsprite(player, ps_weapon, gWeaponInfo[player.readyweapon].atkstate);

    // Alert monsters to the noise
    P_NoiseAlert(player);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Drops down the player's currently equipped weapon: used when the player dies
//------------------------------------------------------------------------------------------------------------------------------------------
void P_DropWeapon(player_t& player) noexcept {
    P_SetPsprite(player, ps_weapon, gWeaponInfo[player.readyweapon].downstate);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called when the player's weapon is in a state of being ready to fire. Checks for the fire button being pressed to initiate firing,
// does weapon bobbing, and lowers the current weapon if there is a change pending or if the player has just died.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_WeaponReady(player_t& player, pspdef_t& sprite) noexcept {
    // Play the idle sound for the chainsaw (if selected)
    if ((player.readyweapon == wp_chainsaw) && (sprite.state == &gStates[S_SAW])) {
        S_StartSound(player.mo, sfx_sawidl);
    }

    // If the player is changing a weapon or dying then put the current weapon away
    if ((player.pendingweapon != wp_nochange) || (player.health == 0)) {
        P_SetPsprite(player, ps_weapon, gWeaponInfo[player.readyweapon].downstate);
        return;
    }

    // If the fire button is pressed then try fire the player's weapon
    const padbuttons_t fireBtn = gpPlayerCtrlBindings[gPlayerNum][cbind_attack];

    if (gTicButtons[gPlayerNum] & fireBtn) {
        P_FireWeapon(player);
        return;
    }
    
    // Otherwise do weapon bobbing based on current movement speed
    constexpr uint32_t COSA_MASK = FINEANGLES - 1;
    constexpr uint32_t SINA_MASK = FINEANGLES / 2 - 1;

    const int32_t bobAngle = gTicCon << 6;
    sprite.sx = WEAPONX + (player.bob >> 16) * gFineCosine[bobAngle & COSA_MASK];
    sprite.sy = WEAPONTOP + (player.bob >> 16) * gFineSine[bobAngle & SINA_MASK];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Re-fire the current weapon if the appropriate button is pressed and if the conditions are right.
// Otherwise switch to another weapon if out of ammo after firing.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_ReFire(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    const padbuttons_t fireBtn = gpPlayerCtrlBindings[gPlayerNum][cbind_attack];

    if ((gTicButtons[gPlayerNum] & fireBtn) && (player.pendingweapon == wp_nochange) && (player.health != 0)) {
        player.refire++;
        P_FireWeapon(player);
    } else {
        player.refire = 0;
        P_CheckAmmo(player);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check if we have enough ammo to fire before reloading, and if there is not enough switch weapons
//------------------------------------------------------------------------------------------------------------------------------------------
void A_CheckReload(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    P_CheckAmmo(player);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the process of lowering the player's weapon.
// Once the weapon is fully lowered, does the switch to the new weapon and begins raising it - unless the player is dead.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Lower(player_t& player, pspdef_t& sprite) noexcept {
    // Lower the weapon a little bit more: if we're not finished then we can just stop there
    sprite.sy += LOWERSPEED;

    if (sprite.sy < WEAPONBOTTOM)
        return;

    // The old weapon is now lowered to be fully offscreen.
    // If the player is now in the dead state then don't raise anything following this.
    if (player.playerstate == PST_DEAD) {
        sprite.sy = WEAPONBOTTOM;
        return;
    }

    // If the player just died put the weapon into the NULL state since it is now fully offscreen
    if (player.health == 0) {
        P_SetPsprite(player, ps_weapon, S_NULL);
        return;
    }

    // Normal case: switch to the pending weapon and begin raising it
    player.readyweapon = player.pendingweapon;
    P_BringUpWeapon(player);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the process of raising the player's weapon and puts it into the 'ready' state once fully raised
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Raise(player_t& player, pspdef_t& sprite) noexcept {
    // Raise the weapon a little bit more: if we're not finished then we can just stop there
    sprite.sy -= RAISESPEED;

    if (sprite.sy > WEAPONTOP)
        return;

    // Clamp the weapon in the fully raised position and go into the ready state for the current weapon
    sprite.sy = WEAPONTOP;
    P_SetPsprite(player, ps_weapon, gWeaponInfo[player.readyweapon].readystate);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the gun flash sprite for the current weapon into the flash state
//------------------------------------------------------------------------------------------------------------------------------------------
void A_GunFlash(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    P_SetPsprite(player, ps_flash, gWeaponInfo[player.readyweapon].flashstate);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does a punch attack for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Punch(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    // Figure out the damage amount (3-24) and 10x it if we have beserk
    int32_t damage = ((P_Random() & 7) + 1) * 3;
    
    if (player.powers[pw_strength]) {
        damage *= 10;
    }

    // Randomly vary the attack angle a bit and do the line attack for the punch.
    //
    // IMPORTANT: the cast to 'angle_t' (unsigned integer) before shifting is a *MUST* here for correct demo syncing!
    // Left shift of signed numbers when there is overflow is undefined behavior in C/C++, and produces an implementation defined result.
    // I've found if I omit ths cast then demos will break, due to different behavior:
    const angle_t angleVariance = (angle_t)(P_Random() - P_Random()) << 18;
    
    mobj_t& playerMobj = *player.mo;
    const angle_t attackAngle = playerMobj.angle + angleVariance;
    P_LineAttack(playerMobj, attackAngle, MELEERANGE, INT32_MAX, damage);

    // If we hit a thing then adjust the player's angle to the attack angle and play a sound
    mobj_t* const pHitThing = gpLineTarget;

    if (pHitThing) {
        S_StartSound(&playerMobj, sfx_punch);
        playerMobj.angle = R_PointToAngle2(playerMobj.x, playerMobj.y, pHitThing->x, pHitThing->y);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does a chainsaw attack for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Saw(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    // Compute the damage amount (3-24)
    const int32_t damage = ((P_Random() & 7) + 1) * 3;

    // Randomly vary the attack angle a bit and do the line attack for the chainsaw.
    //
    // IMPORTANT: the cast to 'angle_t' (unsigned integer) before shifting is a *MUST* here for correct demo syncing!
    // Left shift of signed numbers when there is overflow is undefined behavior in C/C++, and produces an implementation defined result.
    // The original instruction was 'sll' (shift logical left) so the shift needs to be unsigned!
    const angle_t angleVariance = (angle_t)(P_Random() - P_Random()) << 18;

    mobj_t& playerMobj = *player.mo;
    const angle_t attackAngle = playerMobj.angle + angleVariance;
    P_LineAttack(playerMobj, attackAngle, MELEERANGE + 1, INT32_MAX, damage);   // Melee range +1 so the 'puff doesn't skip the flash'

    // If we didn't hit a thing just play the normal chainsaw sound and exit
    mobj_t* const pHitThing = gpLineTarget;

    if (!pHitThing) {
        S_StartSound(&playerMobj, sfx_sawful);
        return;
    }

    // Hit a thing, play the chainsaw hit sound:
    S_StartSound(&playerMobj, sfx_sawhit);

    // Turn the player towards the target and also cause the rotation to ping-pong around that angle.
    // This makes the chainsaw feel like it's 'wiggling' when it's stuck into something:
    const angle_t angleToTarget = R_PointToAngle2(playerMobj.x, playerMobj.y, pHitThing->x, pHitThing->y);
    const angle_t angleDelta = angleToTarget - playerMobj.angle;

    const angle_t ANGLE_WIGGLE_1 = ANG90 / 20;
    const angle_t ANGLE_WIGGLE_2 = ANG90 / 21;

    if (angleDelta > ANG180) {
        if (angleDelta >= -ANGLE_WIGGLE_1) {
            playerMobj.angle -= ANGLE_WIGGLE_1;
        } else {
            playerMobj.angle = angleToTarget + ANGLE_WIGGLE_2;
        }
    } else {
        if (angleDelta > ANGLE_WIGGLE_1) {
            playerMobj.angle = angleToTarget - ANGLE_WIGGLE_2;
        } else {
            playerMobj.angle += ANGLE_WIGGLE_1;
        }
    }

    // Just did an attack
    playerMobj.flags |= MF_JUSTATTACKED;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fire a rocket for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FireMissile(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    const ammotype_t ammoType = gWeaponInfo[player.readyweapon].ammo;
    player.ammo[ammoType]--;
    P_SpawnPlayerMissile(*player.mo, MT_ROCKET);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fire the BFG for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FireBFG(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    const ammotype_t ammoType = gWeaponInfo[player.readyweapon].ammo;
    player.ammo[ammoType] -= BFGCELLS;
    P_SpawnPlayerMissile(*player.mo, MT_BFG);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fire the Plasma gun for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FirePlasma(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    const ammotype_t ammoType = gWeaponInfo[player.readyweapon].ammo;
    player.ammo[ammoType]--;

    // The plasma gun does its muzzle flash a bit randomly
    P_SetPsprite(player, ps_flash, (statenum_t)(gWeaponInfo[player.readyweapon].flashstate + (P_Random() & 1)));
    P_SpawnPlayerMissile(*player.mo, MT_PLASMA);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does a little bit of aim wiggle to figure out the vertical aim slope for an attack.
// Saves the resulting vertical slope to 'gBulletSlope'.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_BulletSlope(mobj_t& mobj) noexcept {
    gBulletSlope = P_AimLineAttack(mobj, mobj.angle, 1024 * FRACUNIT);

    constexpr angle_t AIM_WIGGLE = ANG45 / 8;

    if (!gpLineTarget) {
        gBulletSlope = P_AimLineAttack(mobj, mobj.angle + AIM_WIGGLE, 1024 * FRACUNIT);

        if (!gpLineTarget) {
            gBulletSlope = P_AimLineAttack(mobj, mobj.angle - AIM_WIGGLE, 1024 * FRACUNIT);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does a pistol or chaingun shot for the player (4-16 damage).
// Randomly varies the attack angle a little if the shot is intended to be inaccurate.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_GunShot(mobj_t& mobj, const bool bAccurate) noexcept {
    const int32_t damage = ((P_Random() & 3) + 1) * 4;
    angle_t angle = mobj.angle;
    
    if (!bAccurate) {
        // IMPORTANT: the cast to 'angle_t' (unsigned integer) before shifting is a *MUST* here for correct demo syncing!
        // The original instruction was 'sll' (shift logical left) so the shift needs to be unsigned!
        angle += (angle_t)(P_Random() - P_Random()) << 18;
    }

    P_LineAttack(mobj, angle, MISSILERANGE, INT32_MAX, damage);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fire the Pistol for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FirePistol(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    // Play the sound and decrement ammo count
    mobj_t& playerMobj = *player.mo;
    S_StartSound(&playerMobj, sfx_pistol);
    
    const weaponinfo_t& weaponInfo = gWeaponInfo[player.readyweapon];
    player.ammo[weaponInfo.ammo]--;

    // Do the muzzle flash and fire the shot; note: shot is more inaccurate if refiring immediately!
    P_SetPsprite(player, ps_flash, weaponInfo.flashstate);
    P_GunShot(playerMobj, (player.refire == 0));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fire the Shotgun for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FireShotgun(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    // Play the sound and decrement ammo count
    mobj_t& playerMobj = *player.mo;
    S_StartSound(&playerMobj, sfx_shotgn);

    const weaponinfo_t& weaponInfo = gWeaponInfo[player.readyweapon];
    player.ammo[weaponInfo.ammo]--;
    
    // Do muzzle flash
    P_SetPsprite(player, ps_flash, weaponInfo.flashstate);

    // Decide on vertical aim and fire the 7 shotgun pellets (4-16 damage each)
    const fixed_t aimZSlope = P_AimLineAttack(playerMobj, playerMobj.angle, MISSILERANGE);

    for (int32_t pelletIdx = 0; pelletIdx < 7; ++pelletIdx) {
        // IMPORTANT: the cast to 'angle_t' (unsigned integer) before shifting is a *MUST* here for correct demo syncing!
        // The original instruction was 'sll' (shift logical left) so the shift needs to be unsigned!
        const int32_t damage = ((P_Random() & 3) + 1) * 4;
        const angle_t angleVariance = (angle_t)(P_Random() - P_Random()) << 18;
        const angle_t angle = playerMobj.angle + angleVariance;

        P_LineAttack(playerMobj, angle, MISSILERANGE, aimZSlope, damage);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fire the Super Shotgun for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FireShotgun2(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    // Play the double barrel shotgun sound and set the player thing into the 'attack 2' state
    mobj_t& playerMobj = *player.mo;
    S_StartSound(&playerMobj, sfx_dshtgn);
    P_SetMObjState(playerMobj, S_PLAY_ATK2);

    // Decrement ammo amount and do the muzzle flash
    const weaponinfo_t& weaponInfo = gWeaponInfo[player.readyweapon];
    player.ammo[weaponInfo.ammo] -= 2;
    
    P_SetPsprite(player, ps_flash, weaponInfo.flashstate);

    // Figure out the vertical aim slope
    P_BulletSlope(playerMobj);

    // Fire all of the 20 shotgun pellets (5-15 damage each)
    for (int32_t pelletIdx = 0; pelletIdx < 20; ++pelletIdx) {
        // IMPORTANT: the cast to 'angle_t' (unsigned integer) before shifting is a *MUST* here for correct demo syncing!
        // The original instruction was 'sll' (shift logical left) so the shift needs to be unsigned!
        const int32_t damage = (P_Random() % 3 + 1) * 5;
        const angle_t angleVariance = (angle_t)(P_Random() - P_Random()) << 19;
        const angle_t angle = playerMobj.angle + angleVariance;
        const fixed_t aimSlope = gBulletSlope + (P_Random() - P_Random()) * 32;

        P_LineAttack(playerMobj, angle, MISSILERANGE, aimSlope, damage);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fire the Chaingun for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FireCGun(player_t& player, pspdef_t& sprite) noexcept {
    // Play the sound and decrement ammo count (if we have any ammo)
    mobj_t& playerMobj = *player.mo;
    S_StartSound(&playerMobj, sfx_pistol);

    const weaponinfo_t& weaponInfo = gWeaponInfo[player.readyweapon];
    const ammotype_t ammoType = weaponInfo.ammo;

    if (player.ammo[ammoType] == 0)
        return;

    player.ammo[ammoType]--;

    // Do a muzzle flash for the chaingun and sync to the animation frames of the chaingun
    const statenum_t flashStateNum = (statenum_t)(weaponInfo.flashstate + sprite.state - &gStates[S_CHAIN1]);
    P_SetPsprite(player, ps_flash, flashStateNum);

    // Fire a bullet
    P_GunShot(playerMobj, (player.refire == 0));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set extra light from muzzle flash for the player: 0
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Light0(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    player.extralight = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set extra light from muzzle flash for the player: 8
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Light1(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    player.extralight = 8;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set extra light from muzzle flash for the player: 16
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Light2(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    player.extralight = 16;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to spawn a BFG explosion on every monster in view.
// Up to 40 explosions are created.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BFGSpray(mobj_t& mobj) noexcept {
    // PC-PSX: add extra assert here - it's expected the map object has a target (the firer of the BFG, i.e the player thing)
    ASSERT(mobj.target);
    mobj_t& target = *mobj.target;

    // Spawn explosions from -45 degrees relative to the player to +45 degrees
    constexpr int32_t NUM_EXPLOSIONS = 40;

    for (int32_t explosionIdx = 0; explosionIdx < NUM_EXPLOSIONS; ++explosionIdx) {
        P_AimLineAttack(target, mobj.angle - ANG45 + (ANG90 / NUM_EXPLOSIONS) * explosionIdx, 1024 * FRACUNIT);
        mobj_t* const pLineTarget = gpLineTarget;

        if (!pLineTarget)
            continue;

        // Spawn the BFG explosion on the monster
        P_SpawnMobj(pLineTarget->x, pLineTarget->y, pLineTarget->z + (pLineTarget->height >> 2), MT_EXTRABFG);

        // Figure out the damage amount (15-120) in a series of damage 'rounds' and deal the damage
        int32_t damageAmt = 0;

        for (int32_t damageRound = 0; damageRound < 15; ++damageRound) {
            damageAmt += (P_Random() & 7) + 1;
        }

        P_DamageMObj(*pLineTarget, &target, &target, damageAmt);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do the BFG firing sound
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BFGsound(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    S_StartSound(player.mo, sfx_bfg);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the Super Shotgun opening sound (during reload sequence)
//------------------------------------------------------------------------------------------------------------------------------------------
void A_OpenShotgun2(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    S_StartSound(player.mo, sfx_dbopn);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the Super Shotgun loading sound (during reload sequence)
//------------------------------------------------------------------------------------------------------------------------------------------
void A_LoadShotgun2(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    S_StartSound(player.mo, sfx_dbload);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called at the end of the Super Shotgun firing sequence: possibly refires, and plays the gun closing sound
//------------------------------------------------------------------------------------------------------------------------------------------
void A_CloseShotgun2(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    S_StartSound(player.mo, sfx_dbcls);
    A_ReFire(player, sprite);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the player sprites (weapon sprites) for a specified player: called at the start of each level for each player
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SetupPsprites(const int32_t playerIdx) noexcept {
    // Clear unsimulated tic count
    gTicRemainder[playerIdx] = 0;

    // Remove all player sprites
    player_t& player = gPlayers[playerIdx];

    for (int32_t i = 0; i < NUMPSPRITES; ++i) {
        player.psprites[i].state = nullptr;
    }
    
    // Raise the current weapon
    player.pendingweapon = player.readyweapon;
    P_BringUpWeapon(player);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Updates/ticks player weapon sprites
//------------------------------------------------------------------------------------------------------------------------------------------
void P_MovePsprites(player_t& player) noexcept {
    // Keep simulating player sprite tics while we are behind by one tick
    gTicRemainder[gPlayerNum] += gPlayersElapsedVBlanks[gPlayerNum];
    
    while (gTicRemainder[gPlayerNum] >= VBLANKS_PER_TIC) {
        gTicRemainder[gPlayerNum] -= VBLANKS_PER_TIC;
        
        // Tic all player sprites and advance them to the next state if required
        for (int32_t playerSprIdx = 0; playerSprIdx < NUMPSPRITES; ++playerSprIdx) {
            pspdef_t& playerSpr = player.psprites[playerSprIdx];

            // A null state means not active:
            if (!playerSpr.state)
                continue;

            // A tic count of -1 means never change state
            if (playerSpr.tics == -1)
                continue;

            // Advance the tic count and move onto the next state if required
            playerSpr.tics--;

            if (playerSpr.tics == 0) {
                P_SetPsprite(player, playerSprIdx, playerSpr.state->nextstate);
            }
        }
    }
    
    // Sync the muzzle flash offset to the weapon offset
    player.psprites[ps_flash].sx = player.psprites[ps_weapon].sx;
    player.psprites[ps_flash].sy = player.psprites[ps_weapon].sy;
}
