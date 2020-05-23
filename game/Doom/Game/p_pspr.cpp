#include "p_pspr.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/w_wad.h"
#include "Doom/d_main.h"
#include "Doom/doomdef.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "info.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_tick.h"
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
static constexpr int32_t WEAPONX        = 1 * FRACUNIT;     // TODO: COMMENT
static constexpr int32_t WEAPONBOTTOM   = 96 * FRACUNIT;    // TODO: COMMENT
static constexpr int32_t WEAPONTOP      = 0 * FRACUNIT;     // TODO: COMMENT


static const VmPtr<VmPtr<mobj_t>>   gpSoundTarget(0x80077FFC);      // The current thing making noise
static const VmPtr<fixed_t>         gBulletSlope(0x80077FF0);       // Vertical aiming slope for shooting: computed by 'P_BulletSlope'

//------------------------------------------------------------------------------------------------------------------------------------------
// Recursively flood fill sound starting from the given sector to other neighboring sectors considered valid for sound transfer.
// Aside from closed doors, walls etc. stopping sound propagation sound will also be stopped after two sets of 'ML_SOUNDBLOCK' 
// lines are encountered.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_RecursiveSound(sector_t& sector, const bool bStopOnSoundBlock) noexcept {
    // Don't flood the sector if it's already done and it didn't have sound coming into it blocked
    const int32_t soundTraversed = (bStopOnSoundBlock) ? 2 : 1;

    if ((sector.validcount == *gValidCount) && (sector.soundtraversed <= soundTraversed))
        return;
    
    // Flood fill this sector and save the thing that made noise and whether sound was blocked
    sector.validcount = *gValidCount;
    sector.soundtraversed = soundTraversed;
    sector.soundtarget = *gpSoundTarget;

    // Recurse into adjoining sectors and flood fill with noise
    for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
        line_t& line = *sector.lines[lineIdx];
        sector_t* const pBackSector = line.backsector.get();

        // Sound can't pass single sided lines
        if (!pBackSector)
            continue;
        
        sector_t& frontSector = *line.frontsector.get();

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
    sector_t& curSector = *playerMobj.subsector->sector.get();

    if (player.lastsoundsector.get() == &curSector) 
        return;

    player.lastsoundsector = &curSector;
    *gValidCount += 1;
    *gpSoundTarget = &playerMobj;

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
            // FIXME: convert to native function call
            a0 = ptrToVmAddr(&player);
            a1 = ptrToVmAddr(&sprite);
            void (* const pActionFunc)() = PsxVm::getVmFuncForAddr(state.action);
            pActionFunc();

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
    if ((player.pendingweapon == wp_chainsaw) && (*gbIsLevelDataCached)) {
        S_StartSound(player.mo.get(), sfx_sawup);
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
    if ((player.readyweapon == wp_chainsaw) && (sprite.state.get() == &gStates[S_SAW])) {
        S_StartSound(player.mo.get(), sfx_sawidl);
    }

    // If the player is changing a weapon or dying then put the current weapon away
    if ((player.pendingweapon != wp_nochange) || (player.health == 0)) {
        P_SetPsprite(player, ps_weapon, gWeaponInfo[player.readyweapon].downstate);
        return;
    }

    // If the fire button is pressed then try fire the player's weapon
    const padbuttons_t fireBtn = gpPlayerCtrlBindings[*gPlayerNum][cbind_attack];

    if (gTicButtons[*gPlayerNum] & fireBtn) {
        P_FireWeapon(player);
        return;
    }
    
    // Otherwise do weapon bobbing based on current movement speed
    constexpr uint32_t COSA_MASK = FINEANGLES - 1;
    constexpr uint32_t SINA_MASK = FINEANGLES / 2 - 1;

    const int32_t bobAngle = *gTicCon << 6;
    sprite.sx = WEAPONX + (player.bob >> 16) * gFineCosine[bobAngle & COSA_MASK];
    sprite.sy = WEAPONTOP + (player.bob >> 16) * gFineSine[bobAngle & SINA_MASK];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Re-fire the current weapon if the appropriate button is pressed and if the conditions are right.
// Otherwise switch to another weapon if out of ammo after firing.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_ReFire(player_t& player, [[maybe_unused]] pspdef_t& sprite) noexcept {
    const padbuttons_t fireBtn = gpPlayerCtrlBindings[*gPlayerNum][cbind_attack];

    if ((gTicButtons[*gPlayerNum] & fireBtn) && (player.pendingweapon == wp_nochange) && (player.health != 0)) {
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
    mobj_t* const pHitThing = gpLineTarget->get();

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
    mobj_t* const pHitThing = gpLineTarget->get();

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
    *gBulletSlope = P_AimLineAttack(mobj, mobj.angle, 1024 * FRACUNIT);

    constexpr angle_t AIM_WIGGLE = ANG45 / 8;

    if (!gpLineTarget->get()) {
        *gBulletSlope = P_AimLineAttack(mobj, mobj.angle + AIM_WIGGLE, 1024 * FRACUNIT);

        if (!gpLineTarget->get()) {
            *gBulletSlope = P_AimLineAttack(mobj, mobj.angle - AIM_WIGGLE, 1024 * FRACUNIT);
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
        const fixed_t aimSlope = *gBulletSlope + (P_Random() - P_Random()) * 32;

        P_LineAttack(playerMobj, angle, MISSILERANGE, aimSlope, damage);
    }
}

void A_FireCGun() noexcept {
    sp -= 0x30;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(s0, sp + 0x18);
    s0 = a1;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    a0 = lw(s1);
    a1 = sfx_pistol;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = lw(s1 + 0x6C);
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v1;
    v0 = lw(at);
    v0 <<= 2;
    v1 = v0 + s1;
    v0 = lw(v1 + 0x98);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002153C;
    }
    sw(v0, v1 + 0x98);
    v1 = lw(s1 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7108;                                       // Result = WeaponInfo_Fist[5] (80067108)
    at += v0;
    v0 = lw(at);
    v1 = v0 << 3;
    v1 -= v0;
    v1 <<= 2;
    v0 = lw(s0);
    v1 += v0;
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x6CC4;                                       // Result = State_S_CHAIN1[0] (8005933C)
    v1 -= v0;
    v0 = v1 << 3;
    v0 += v1;
    a0 = v0 << 6;
    v0 += a0;
    v0 <<= 3;
    v0 += v1;
    a0 = v0 << 15;
    v0 += a0;
    v0 <<= 3;
    v0 += v1;
    v0 = -v0;
    a0 = u32(i32(v0) >> 2);
    s0 = s1 + 0x100;
    if (a0 != 0) goto loc_80021470;
    sw(0, s1 + 0x100);
    goto loc_800214E0;
loc_80021470:
    v0 = a0 << 3;
loc_80021474:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_800214C0;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_800214E0;
loc_800214C0:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_800214E0;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_80021474;
    sw(0, s0);
loc_800214E0:
    s0 = lw(s1 + 0xC4);
    s3 = lw(s1);
    s0 = (s0 < 1);
    _thunk_P_Random();
    v0 &= 3;
    v0++;
    s1 = lw(s3 + 0x24);
    s2 = v0 << 2;
    if (s0 != 0) goto loc_80021520;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 18;
    s1 += s0;
loc_80021520:
    sw(s2, sp + 0x10);
    a0 = s3;
    a1 = s1;
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    a2 = 0x8000000;                                     // Result = 08000000
    P_LineAttack(*vmAddrToPtr<mobj_t>(a0), a1, a2, a3, lw(sp + 0x10));
loc_8002153C:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_Light0() noexcept {
    sw(0, a0 + 0xE4);
}

void A_Light1() noexcept {
    v0 = 8;
    sw(v0, a0 + 0xE4);
}

void A_Light2() noexcept {
    v0 = 0x10;
    sw(v0, a0 + 0xE4);
}

void A_BFGSpray() noexcept {
    sp -= 0x28;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s4, sp + 0x20);
    s4 = 0;                                             // Result = 00000000
    sw(s2, sp + 0x18);
    s2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x24);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    a2 = 0x4000000;                                     // Result = 04000000
loc_800215A8:
    a1 = 0xE0000000;                                    // Result = E0000000
    a1 += s2;
    v0 = lw(s3 + 0x24);
    a0 = lw(s3 + 0x74);
    a1 += v0;
    v0 = P_AimLineAttack(*vmAddrToPtr<mobj_t>(a0), a1, a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    a3 = 0x20;                                          // Result = 00000020
    if (v0 == 0) goto loc_8002162C;
    s1 = 0;                                             // Result = 00000000
    s0 = 0;                                             // Result = 00000000
    a0 = lw(v0);
    a1 = lw(v0 + 0x4);
    a2 = lw(v0 + 0x44);
    v0 = lw(v0 + 0x8);
    a2 = u32(i32(a2) >> 2);
    a2 += v0;
    v0 = ptrToVmAddr(P_SpawnMobj(a0, a1, a2, (mobjtype_t) a3));
loc_800215F8:
    s0++;
    _thunk_P_Random();
    v1 = s1 + 1;
    v0 &= 7;
    s1 = v1 + v0;
    v0 = (i32(s0) < 0xF);
    a3 = s1;
    if (v0 != 0) goto loc_800215F8;
    a1 = lw(s3 + 0x74);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    a2 = a1;
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
loc_8002162C:
    v0 = 0x1990000;                                     // Result = 01990000
    v0 |= 0x9999;                                       // Result = 01999999
    s2 += v0;
    s4++;
    v0 = (i32(s4) < 0x28);
    a2 = 0x4000000;                                     // Result = 04000000
    if (v0 != 0) goto loc_800215A8;
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void A_BFGsound() noexcept {
    a0 = lw(a0);
    a1 = sfx_bfg;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
}

void A_OpenShotgun2() noexcept {
    a0 = lw(a0);
    a1 = sfx_dbopn;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
}

void A_LoadShotgun2() noexcept {
    a0 = lw(a0);
    a1 = sfx_dbload;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
}

void A_CloseShotgun2() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a0 = lw(s0);
    a1 = sfx_dbcls;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = ptrToVmAddr(&gpPlayerCtrlBindings[0]);
    at += v0;
    v1 = lw(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += v0;
    v0 = lw(at);
    v1 = lw(v1);
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0xA;                                       // Result = 0000000A
        if (bJump) goto loc_80021774;
    }
    v1 = lw(s0 + 0x70);
    if (v1 != v0) goto loc_80021774;
    v0 = lw(s0 + 0x24);
    a0 = s0;
    if (v0 == 0) goto loc_80021774;
    v0 = lw(s0 + 0xC4);
    v0++;
    sw(v0, a0 + 0xC4);
    P_FireWeapon(*vmAddrToPtr<player_t>(a0));
    goto loc_80021780;
loc_80021774:
    sw(0, s0 + 0xC4);
    a0 = s0;
    v0 = P_CheckAmmo(*vmAddrToPtr<player_t>(a0));
loc_80021780:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_SetupPsprites() noexcept {
loc_80021794:
    sp -= 0x20;
    v1 = a0 << 2;
    a1 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7F70;                                       // Result = gTicRemainder[0] (80078090)
    at += v1;
    sw(0, at);
    v1 += a0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    s1 = v0 + v1;
    v0 = s1 + 0x10;
loc_800217DC:
    sw(0, v0 + 0xF0);
    a1--;
    v0 -= 0x10;
    if (i32(a1) >= 0) goto loc_800217DC;
    v1 = lw(s1 + 0x6C);
    v0 = 0xA;                                           // Result = 0000000A
    sw(v1, s1 + 0x70);
    if (v1 != v0) goto loc_80021808;
    v0 = lw(s1 + 0x6C);
    sw(v0, s1 + 0x70);
loc_80021808:
    v1 = lw(s1 + 0x70);
    v0 = 8;                                             // Result = 00000008
    s0 = s1 + 0xF0;
    if (v1 != v0) goto loc_8002183C;
    v0 = *gbIsLevelDataCached;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 1;
        if (bJump) goto loc_80021844;
    }
    a0 = lw(s1);
    a1 = sfx_sawup;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v1 = lw(s1 + 0x70);
loc_8002183C:
    v0 = v1 << 1;
loc_80021844:
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F8;                                       // Result = WeaponInfo_Fist[1] (800670F8)
    at += v0;
    v1 = lw(at);
    v0 = 0xA;                                           // Result = 0000000A
    sw(v0, s1 + 0x70);
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s1 + 0xF8);
    v0 = 0x600000;                                      // Result = 00600000
    a0 = v1;
    sw(v0, s1 + 0xFC);
    if (a0 != 0) goto loc_80021884;
    sw(0, s1 + 0xF0);
    goto loc_800218F4;
loc_80021884:
    v0 = a0 << 3;
loc_80021888:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_800218D4;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_800218F4;
loc_800218D4:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_800218F4;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_80021888;
    sw(0, s0);
loc_800218F4:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_MovePsprites() noexcept {
loc_8002190C:
    sp -= 0x38;
    sw(s1, sp + 0x1C);
    s1 = a0;
    v1 = *gPlayerNum;
    a1 = 0x80080000;                                    // Result = 80080000
    a1 -= 0x7F70;                                       // Result = gTicRemainder[0] (80078090)
    sw(ra, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s0, sp + 0x18);
    v1 <<= 2;
    a0 = v1 + a1;
    v0 = lw(a0);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v1;
    v1 = lw(at);
    v0 += v1;
    sw(v0, a0);
    v0 = (i32(v0) < 4);
    if (v0 != 0) goto loc_80021A94;
loc_80021974:
    s3 = s1 + 0xF0;
    s5 = 0;                                             // Result = 00000000
    v1 = *gPlayerNum;
    s2 = s1 + 0xF4;
    v1 <<= 2;
    v1 += a1;
    v0 = lw(v1);
    s4 = 0xF0;                                          // Result = 000000F0
    v0 -= 4;
    sw(v0, v1);
loc_800219A0:
    v0 = lw(s3);
    {
        const bool bJump = (v0 == 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_80021A48;
    }
    v1 = lw(s2);
    {
        const bool bJump = (v1 == v0);
        v0 = v1 - 1;
        if (bJump) goto loc_80021A48;
    }
    sw(v0, s2);
    if (v0 != 0) goto loc_80021A48;
    v0 = lw(s3);
    a0 = lw(v0 + 0x10);
    s0 = s1 + s4;
    goto loc_80021A3C;
loc_800219DC:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_80021A28;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_80021A48;
loc_80021A28:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_80021A48;
loc_80021A3C:
    v0 = a0 << 3;
    if (a0 != 0) goto loc_800219DC;
    sw(0, s0);
loc_80021A48:
    s4 += 0x10;
    s5++;
    s2 += 0x10;
    v0 = (i32(s5) < 2);
    s3 += 0x10;
    if (v0 != 0) goto loc_800219A0;
    v0 = *gPlayerNum;
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7F70;                                       // Result = gTicRemainder[0] (80078090)
    at += v0;
    v0 = lw(at);
    a1 = 0x80080000;                                    // Result = 80080000
    a1 -= 0x7F70;                                       // Result = gTicRemainder[0] (80078090)
    v0 = (i32(v0) < 4);
    if (v0 == 0) goto loc_80021974;
loc_80021A94:
    v0 = lw(s1 + 0xF8);
    v1 = lw(s1 + 0xFC);
    sw(v0, s1 + 0x108);
    sw(v1, s1 + 0x10C);
    ra = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x38;
    return;
}

// TODO: remove all these thunks
void _thunk_P_FireWeapon() noexcept { P_FireWeapon(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0)); }
void _thunk_A_WeaponReady() noexcept { A_WeaponReady(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_ReFire() noexcept { A_ReFire(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_CheckReload() noexcept { A_CheckReload(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_Lower() noexcept { A_Lower(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_Raise() noexcept { A_Raise(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_GunFlash() noexcept { A_GunFlash(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_Punch() noexcept { A_Punch(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_Saw() noexcept { A_Saw(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_FireMissile() noexcept { A_FireMissile(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_FireBFG() noexcept { A_FireBFG(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_FirePlasma() noexcept { A_FirePlasma(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_FirePistol() noexcept { A_FirePistol(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_FireShotgun() noexcept { A_FireShotgun(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
void _thunk_A_FireShotgun2() noexcept { A_FireShotgun2(*vmAddrToPtr<player_t>(*PsxVm::gpReg_a0), *vmAddrToPtr<pspdef_t>(*PsxVm::gpReg_a1)); }
