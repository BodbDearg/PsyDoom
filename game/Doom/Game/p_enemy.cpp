#include "p_enemy.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "g_game.h"
#include "p_doors.h"
#include "p_floor.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_move.h"
#include "p_sight.h"
#include "p_switch.h"
#include "PsxVm/PsxVm.h"
#include <algorithm>

// Monster movement speed multiplier for the 8 movement directions: x & y
constexpr static fixed_t gMoveXSpeed[8] = { FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000, 0, 47000 };
constexpr static fixed_t gMoveYSpeed[8] = { 0, 47000, FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000 };

// The opposite direction to every move direction
constexpr static dirtype_t gOppositeDir[NUMDIRS] = {
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_NODIR
};

// The four diagonal directions
constexpr static dirtype_t gDiagonalDirs[4] = {
    DI_NORTHWEST,
    DI_NORTHEAST,
    DI_SOUTHWEST,
    DI_SOUTHEAST
};

//------------------------------------------------------------------------------------------------------------------------------------------
// For the given attacker, checks to see if it's target is within melee range and returns 'true' if so
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_CheckMeleeRange(mobj_t& attacker) noexcept {
    // If the attacker can't see it's target then consider it not to be in melee range
    if ((attacker.flags & MF_SEETARGET) == 0)
        return false;

    // If there is no target then obviously nothing is in melee range
    if (!attacker.target)
        return false;

    // Return whether the target is within melee range
    mobj_t& target = *attacker.target;
    const fixed_t approxDist = P_AproxDistance(target.x - attacker.x, target.y - attacker.y);
    return (approxDist < MELEERANGE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Not really a range check, more of a check to see if an attacker should to a missile attack against its target.
// Takes into account things like range and current sight status and also adds an element of randomness.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_CheckMissileRange(mobj_t& attacker) noexcept {
    // If the attacker can't see it's target then it can't do a missile attack
    if ((attacker.flags & MF_SEETARGET) == 0)
        return false;

    // If the attacker was just hit then fight back immediately! (just this once)
    if (attacker.flags & MF_JUSTHIT) {
        attacker.flags &= ~MF_JUSTHIT;
        return true;
    }

    // If the attacker has to wait a bit then an attack is not possible
    if (attacker.reactiontime != 0)
        return false;

    // Get the distance to the target and do a tweak adjust for the sake of the random logic below
    fixed_t distFrac = P_AproxDistance(attacker.x - attacker.target->x, attacker.y - attacker.target->y);
    distFrac -= 64 * FRACUNIT;

    // If the attacker has no melee attack then do a missile attack more frequently
    if (attacker.info->meleestate == S_NULL) {
        distFrac -= 128 * FRACUNIT;
    }
    
    // Convert to integer distance and if you are a skull reduce the missile distance even more
    int32_t dist = distFrac >> FRACBITS;
    
    if (attacker.type == MT_SKULL) {
        dist >>= 1;
    }
    
    // Cap the distance so there is always at least some chance of firing and decide randomly whether to fire.
    // The closer the target is the more likely we will fire:
    dist = std::min(dist, 200);
    return (dist <= P_Random());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to move the specified actor in it's current movement direction.
// For floating monsters also attempt to do up/down movement if appropriate, and if a move is blocked try to open doors.
// Returns 'true' if the move was considered a success.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_Move(mobj_t& actor) noexcept {
    // Move is unsuccessful if there is no direction
    if (actor.movedir == DI_NODIR)
        return false;
    
    // Decide on where the actor will move to and try to move there
    const fixed_t moveSpeed = actor.info->speed;
    const fixed_t tryX = actor.x + moveSpeed * gMoveXSpeed[actor.movedir];
    const fixed_t tryY = actor.y + moveSpeed * gMoveYSpeed[actor.movedir];
    
    if (P_TryMove(actor, tryX, tryY)) {
        // Move was successful: stop trying to float up/down (can reach whatever we're after)
        actor.flags &= ~MF_INFLOAT;

        // If the actor is not floating, ensure it is grounded on whatever sector it is in (in case it's going down steps)
        if ((actor.flags & MF_FLOAT) == 0) {
            actor.z = actor.floorz;
        }

        return true;
    }

    // Move failed: if the actor can float then try and go up or down - depending on relative position to the line opening.
    // If that can be done then the move is still considered a success:
    if ((actor.flags & MF_FLOAT) && *gbFloatOk) {
        actor.z += (actor.z >= *gTmFloorZ) ? -FLOATSPEED : FLOATSPEED;
        actor.flags |= MF_INFLOAT;
        return true;
    }

    // Try to see if we can open a door to unblock movement.
    // If that is possible to do then consider the move a success:
    line_t* const pBlockLine = gpBlockLine->get();

    if (!pBlockLine)
        return false;
    
    if (pBlockLine->special) {
        // This line has a special: try to use it and set the movement direction to none
        actor.movedir = DI_NODIR;
        return P_UseSpecialLine(actor, *pBlockLine);
    }

    // Move failed!
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to move the specified actor in it's current movement direction.
// For floating monsters also attempt to do up/down movement if appropriate, and if a move is blocked try to open doors.
// Returns 'true' if the move was successful, or if the move was only blocked by a door which can be opened.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_TryWalk(mobj_t& actor) noexcept {
    if (!P_Move(actor))
        return false;

    actor.movecount = P_Random() & 0xf;     // Randomly vary time until a new movement dir is chosen
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decides on a new movement direction for the given AI thing and moves a little in that direction.
// Precondition: the actor must have a target that it is pursuing before this is called!
//------------------------------------------------------------------------------------------------------------------------------------------
void P_NewChaseDir(mobj_t& actor) noexcept {
    // Actor is expected to have a target if called
    if (!actor.target) {
        I_Error("P_NewChaseDir: called with no target");
        return;
    }

    // Save the current movement direction and it's opposite for comparisons
    const dirtype_t oldMoveDir = actor.movedir;
    const dirtype_t turnaroundDir = gOppositeDir[oldMoveDir];

    // Figure out the horizontal and vertical directions to the target and the distances
    const fixed_t tgtDistX = actor.target->x - actor.x;
    const fixed_t tgtDistY = actor.target->y - actor.y;

    dirtype_t hdirToTgt;

    if (tgtDistX > 10 * FRACUNIT) {
        hdirToTgt = DI_EAST;
    } else if (tgtDistX < -10 * FRACUNIT) {
        hdirToTgt = DI_WEST;
    } else {
        hdirToTgt = DI_NODIR;
    }

    dirtype_t vdirToTgt;

    if (tgtDistY > 10 * FRACUNIT) {
        vdirToTgt = DI_NORTH;
    } else if (tgtDistY < -10 * FRACUNIT) {
        vdirToTgt = DI_SOUTH;
    } else {
        vdirToTgt = DI_NODIR;
    }
    
    // Try to move diagonally to the target
    if ((hdirToTgt != DI_NODIR) && (vdirToTgt != DI_NODIR)) {
        // Go east if dx > 0; Go south if dy < 0:
        const uint32_t diagIdx = ((tgtDistX > 0) ? 1 : 0) + ((tgtDistY < 0) ? 2 : 0);
        const dirtype_t diagDir = gDiagonalDirs[diagIdx];

        // Walk this way unless it means turning around fully
        actor.movedir = diagDir;

        if (diagDir != turnaroundDir) {
            if (P_TryWalk(actor))
                return;
        }
    }

    // Try moving left/right and up down with a preference to move in the largest distance direction.
    // Some random variation IS added in however to make the behavior more interesting.
    // Note: movement in the opposite of the current direction is NOT allowed at this point.
    dirtype_t tryDir1 = hdirToTgt;
    dirtype_t tryDir2 = vdirToTgt;

    if ((P_Random() > 200) || (std::abs(tgtDistY) > std::abs(tgtDistX))) {
        std::swap(tryDir1, tryDir2);
    }
    
    if (tryDir1 == turnaroundDir) {
        tryDir1 = DI_NODIR;
    }
    
    if (tryDir2 == turnaroundDir) {
        tryDir2 = DI_NODIR;
    }
    
    if (tryDir1 != DI_NODIR) {
        actor.movedir = tryDir1;
        
        if (P_TryWalk(actor))
            return;
    }

    if (tryDir2 != DI_NODIR) {
        actor.movedir = tryDir2;
        
        if (P_TryWalk(actor))
            return;
    }
    
    // If all that movement failed, try going in the previous movement direction
    if (oldMoveDir != DI_NODIR) {
        actor.movedir = oldMoveDir;

        if (P_TryWalk(actor))
            return;
    }
    
    // Next try all of the possible move directions, except the turnaround dir.
    // Randomly start from opposite ends of the directions lists to change things up every so often.
    if (P_Random() & 1) {
        for (int32_t dirIdx = DI_EAST; dirIdx <= DI_SOUTHEAST; ++dirIdx) {
            if (dirIdx == turnaroundDir)
                continue;

            actor.movedir = (dirtype_t) dirIdx;

            if (P_TryWalk(actor))
                return;
        }
    } else {
        for (int32_t dirIdx = DI_SOUTHEAST; dirIdx >= DI_EAST; --dirIdx) {
            if (dirIdx == turnaroundDir)
                continue;

            actor.movedir = (dirtype_t) dirIdx;

            if (P_TryWalk(actor))
                return;
        }
    }
    
    // Last ditch attempt: try to move in the opposite (turnaround) direction
    if (turnaroundDir != DI_NODIR) {
        actor.movedir = turnaroundDir;

        if (P_TryWalk(actor))
            return;
    }

    // Unable to find a direction to move in, exhausted all options!
    actor.movedir = DI_NODIR;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Look around for players to target, potentially setting a new target on the actor.
// Returns 'true' when a player is regarded as targetted by the actor.
// The vision cone can be restricted to 180 degrees in front if required, if the 'all around' flag is NOT set.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_LookForPlayers(mobj_t& actor, const bool bAllAround) noexcept {
    // Pick a new target if we don't see the current one, or if we don't have a live target
    mobj_t* const pTarget = actor.target.get();

    if (((actor.flags & MF_SEETARGET) == 0) || (!pTarget) || (pTarget->health <= 0)) {
        int32_t tgtPlayerIdx = 0;

        // In co-op choose a player to target randomly.
        // This code assumes just 2 co-op players, if we wanted more it would need to chage.
        if (gbPlayerInGame[1]) {
            tgtPlayerIdx = P_Random() & 1;
            
            // If the player we chose is already dead then pick the other player
            if (gPlayers[tgtPlayerIdx].health <= 0) {
                tgtPlayerIdx ^= 1;
            }
        }

        actor.target = gPlayers[tgtPlayerIdx].mo;
        return false;
    }

    // If something made a sound in the sector then regard it as seen.
    // Note that the target field is not actually updated here, it's only done in 'A_Look':
    if (actor.subsector->sector->soundtarget == pTarget)
        return true;
    
    // If not allowed to look all around then make sure the angle to the target is in range.
    // Must be within a 180 degree vision range:
    if (!bAllAround) {
        const angle_t angleToTgt = R_PointToAngle2(actor.x, actor.y, pTarget->x, pTarget->y) - actor.angle;
        
        if (angleToTgt > ANG90 && angleToTgt < ANG270) {
            // Target not within the right vision angle range: when really close to the target regard it as seen anyway:
            const fixed_t distToTgt = P_AproxDistance(pTarget->x - actor.x, pTarget->y - actor.y);
            return (distToTgt <= MELEERANGE);
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// An enemy state that runs until the enemy has seen a player, or until a player has made noise to alert the enemy
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Look(mobj_t& actor) noexcept {
    // Unless the monster has found a target then it can't proceed to the 'see' state:
    if (!P_LookForPlayers(actor, false)) {
        // Not chasing anything currently
        actor.threshold = 0;

        // If something is making noise in the current sector then try to target that instead if possible
        mobj_t* const pNoiseMaker = actor.subsector->sector->soundtarget.get();

        const bool bTargetNoiseMaker = (
            pNoiseMaker &&                          // Is there something making noise?
            (pNoiseMaker->flags & MF_SHOOTABLE) &&  // Noise maker must be attackable
            ((actor.flags & MF_AMBUSH) == 0)        // Ambush monsters don't react to sound
        );

        if (!bTargetNoiseMaker)     // Continue in 'look' state if we can't target something making noise
            return;

        actor.target = pNoiseMaker;
    }
    
    // Play the see sound for the monster
    if (actor.info->seesound != sfx_None) {
        // Vary some sounds played randomly (imps and former humans):
        sfxenum_t soundId = actor.info->seesound;

        switch (actor.info->seesound) {
            // Bug? Notice here that it never picks 'sfx_posit3': the sound goes unused....
            // I tested out that audio piece and it doesn't sound so good anyway, so maybe the choice was deliberate?
            case sfx_posit1:
            case sfx_posit2:
            case sfx_posit3:
                soundId = (sfxenum_t)(sfx_posit1 + (P_Random() & 1));
                break;

            case sfx_bgsit1:
            case sfx_bgsit2:
                soundId = (sfxenum_t)(sfx_bgsit1 + (P_Random() & 1));
                break;

            default:
                break;
        }

        // Play the sound: if the enemy is a Spiderdemon or Cyberdemon, play at full volume also (non-positional sound)
        mobj_t* const pSoundSrc = ((actor.type != MT_SPIDER) && (actor.type != MT_CYBORG)) ? &actor : nullptr;
        S_StartSound(pSoundSrc, soundId);
    }

    // Go into the see/chase state
    P_SetMObjState(actor, actor.info->seestate);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Core AI function that handles a lot of movement and attacking logic for enemies. Tries to get the monster as close as possible to the
// target player (doing movements) and makes the monster attack with missiles and melee attacks where possible. Also turns the monster
// towards the current move direction, tries to look for targets (if the current one is lost) and does active sounds among other things...
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Chase(mobj_t& actor) noexcept {
    // Reduce time left until attack is allowed or when a new target can be acquired
    if (actor.reactiontime != 0) {
        actor.reactiontime--;
    }

    if (actor.threshold != 0) {
        actor.threshold--;
    }
    
    // Turn towards the movement direction (in 45 degree increments) if required
    if (actor.movedir < DI_NODIR) {
        // Mask the actor angle to increments of 45 degrees and see if we need to adjust to point in the direction we want
        actor.angle &= 0xE0000000;
        const angle_t desiredAngle = actor.movedir * ANG45;
        const int32_t angleDelta = (int32_t)(actor.angle - desiredAngle);   // N.B: must make signed for comparison below!

        if (angleDelta > 0) {
            actor.angle -= ANG45;
        } else if (angleDelta < 0) {
            actor.angle += ANG45;
        }
    }

    // Look for a new target if required
    mobj_t* const pTarget = actor.target.get();

    if ((!pTarget) || ((pTarget->flags & MF_SHOOTABLE) == 0)) {
        // If no target can be found then go into the spawn/idle state
        if (!P_LookForPlayers(actor, true)) {
            P_SetMObjState(actor, actor.info->spawnstate);
        }

        // Can't chase if no target: or if we have a new one we need to wait a little
        return;
    }
    
    // If the monster just attacked then change move direction
    if (actor.flags & MF_JUSTATTACKED) {
        actor.flags &= ~MF_JUSTATTACKED;
        P_NewChaseDir(actor);
        return;
    }

    // Try and do a melee attack if possible
    mobjinfo_t& actorInfo = *actor.info;

    if ((actorInfo.meleestate != S_NULL) && P_CheckMeleeRange(actor)) {
        if (actorInfo.attacksound != sfx_None) {
            S_StartSound(&actor, actorInfo.attacksound);
        }

        P_SetMObjState(actor, actorInfo.meleestate);
        return;
    }

    // Try to do a missile attack if possible.
    // Note that on nightmare skill we don't have any cooldowns on attacking again!
    const bool bMissileAttackAllowed = ((actor.movecount == 0) || (*gGameSkill == sk_nightmare));

    if ((actorInfo.missilestate != S_NULL) && bMissileAttackAllowed) {
        if (P_CheckMissileRange(actor)) {
            P_SetMObjState(actor, actorInfo.missilestate);

            // Don't attack again for a bit unless on nightmare
            if (*gGameSkill != sk_nightmare) {
                actor.flags |= MF_JUSTATTACKED;
            }

            return;
        }
    }

    // See if it's time to move in a different direction.
    // Change direction every so often, or if the monster becomes blocked:
    actor.movecount--;

    if ((actor.movecount < 0) || (!P_Move(actor))) {
        P_NewChaseDir(actor);
    }

    // Make an active sound every so often
    if ((actorInfo.activesound != sfx_None) && (P_Random() < 3)) {
        S_StartSound(&actor, actorInfo.activesound);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make the actor face towards it's target, if it has one
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FaceTarget(mobj_t& actor) noexcept {
    // Can't face target if there is none
    if (!actor.target)
        return;

    // Monster is no longer in ambush mode and turn to face the target
    mobj_t& target = *actor.target;
    actor.flags &= ~MF_AMBUSH;
    actor.angle = R_PointToAngle2(actor.x, actor.y, target.x, target.y);

    // If the target has partial invisbility then vary the angle randomly a bit (by almost 45 degrees)
    if (target.flags & MF_ALL_BLEND_FLAGS) {
        actor.angle += (P_Random() - P_Random()) * (ANG45 / 256);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for a zombieman (pistol)
//------------------------------------------------------------------------------------------------------------------------------------------
void A_PosAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;
    
    A_FaceTarget(actor);
    S_StartSound(&actor, sfx_pistol);

    const angle_t shootAngle = actor.angle + (P_Random() - P_Random()) * (ANG45 / 512);     // Vary by up to 22.5 degrees (approximately)
    const int32_t damage = ((P_Random() & 7) + 1) * 3;                                      // 3-24 damage
    P_LineAttack(actor, shootAngle, MISSILERANGE, INT32_MAX, damage);
}

void A_SPosAttack() noexcept {
    sp -= 0x30;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s0, sp + 0x18);
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_80016C04;
    a1 = sfx_shotgn;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = lw(s1 + 0x74);
    s2 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80016B84;
    a2 = -0x21;                                         // Result = FFFFFFDF
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80016B84;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80016B84:
    s3 = lw(s1 + 0x24);
loc_80016B88:
    s2++;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 20;
    s0 += s3;
    _thunk_P_Random();
    v1 = 0x66660000;                                    // Result = 66660000
    v1 |= 0x6667;                                       // Result = 66666667
    mult(v0, v1);
    a0 = s1;
    a1 = s0;
    a2 = 0x8000000;                                     // Result = 08000000
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    v1 = u32(i32(v0) >> 31);
    t0 = hi;
    t0 = u32(i32(t0) >> 1);
    t0 -= v1;
    v1 = t0 << 2;
    v1 += t0;
    v0 -= v1;
    v0++;
    v1 = v0 << 1;
    v1 += v0;
    sw(v1, sp + 0x10);
    P_LineAttack(*vmAddrToPtr<mobj_t>(a0), a1, a2, a3, lw(sp + 0x10));
    v0 = (i32(s2) < 3);
    if (v0 != 0) goto loc_80016B88;
loc_80016C04:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_CPosAttack() noexcept {
    sp -= 0x30;
    sw(s3, sp + 0x24);
    s3 = a0;
    sw(ra, sp + 0x28);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    v0 = lw(s3 + 0x74);
    if (v0 == 0) goto loc_80016D50;
    a1 = sfx_pistol;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = lw(s3 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80016CD0;
    a0 = lw(s3);
    a1 = lw(s3 + 0x4);
    v0 = lw(s3 + 0x64);
    v1 = lw(s3 + 0x74);
    v0 &= a2;
    sw(v0, s3 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s3 + 0x74);
    sw(v0, s3 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s3;
    if (v0 == 0) goto loc_80016CD4;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s3 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s3 + 0x24);
loc_80016CD0:
    a0 = s3;
loc_80016CD4:
    s1 = lw(s3 + 0x24);
    a2 = 0x8000000;                                     // Result = 08000000
    a1 = s1;
    v0 = P_AimLineAttack(*vmAddrToPtr<mobj_t>(a0), a1, a2);
    s2 = v0;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 20;
    s1 += s0;
    _thunk_P_Random();
    v1 = 0x66660000;                                    // Result = 66660000
    v1 |= 0x6667;                                       // Result = 66666667
    mult(v0, v1);
    a0 = s3;
    a1 = s1;
    a2 = 0x8000000;                                     // Result = 08000000
    a3 = s2;
    v1 = u32(i32(v0) >> 31);
    t0 = hi;
    t0 = u32(i32(t0) >> 1);
    t0 -= v1;
    v1 = t0 << 2;
    v1 += t0;
    v0 -= v1;
    v0++;
    v1 = v0 << 1;
    v1 += v0;
    sw(v1, sp + 0x10);
    P_LineAttack(*vmAddrToPtr<mobj_t>(a0), a1, a2, a3, lw(sp + 0x10));
loc_80016D50:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_CPosRefire() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80016DFC;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80016DFC;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80016DFC:
    _thunk_P_Random();
    v0 = (i32(v0) < 0x28);
    if (v0 != 0) goto loc_80016E54;
    a1 = lw(s1 + 0x74);
    if (a1 == 0) goto loc_80016E40;
    v0 = lw(a1 + 0x68);
    if (i32(v0) <= 0) goto loc_80016E40;
    a0 = s1;
    v0 = P_CheckSight(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1));
    if (v0 != 0) goto loc_80016E54;
loc_80016E40:
    v0 = lw(s1 + 0x58);
    a1 = lw(v0 + 0xC);
    a0 = s1;
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
loc_80016E54:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_SpidAttack() noexcept {
    sp -= 0x30;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s0, sp + 0x18);
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_80016F9C;
    a1 = sfx_pistol;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = lw(s1 + 0x74);
    s2 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80016F1C;
    a2 = -0x21;                                         // Result = FFFFFFDF
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80016F1C;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80016F1C:
    s3 = lw(s1 + 0x24);
loc_80016F20:
    s2++;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    s0 <<= 20;
    s0 += s3;
    _thunk_P_Random();
    v1 = 0x66660000;                                    // Result = 66660000
    v1 |= 0x6667;                                       // Result = 66666667
    mult(v0, v1);
    a0 = s1;
    a1 = s0;
    a2 = 0x8000000;                                     // Result = 08000000
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    v1 = u32(i32(v0) >> 31);
    t0 = hi;
    t0 = u32(i32(t0) >> 1);
    t0 -= v1;
    v1 = t0 << 2;
    v1 += t0;
    v0 -= v1;
    v0++;
    v1 = v0 << 1;
    v1 += v0;
    sw(v1, sp + 0x10);
    P_LineAttack(*vmAddrToPtr<mobj_t>(a0), a1, a2, a3, lw(sp + 0x10));
    v0 = (i32(s2) < 3);
    if (v0 != 0) goto loc_80016F20;
loc_80016F9C:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_SpidRefire() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017048;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80017048;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80017048:
    _thunk_P_Random();
    v0 = (i32(v0) < 0xA);
    if (v0 != 0) goto loc_800170A4;
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_80017090;
    v0 = lw(v0 + 0x68);
    v1 = 0x4000000;                                     // Result = 04000000
    if (i32(v0) <= 0) goto loc_80017090;
    v0 = lw(s1 + 0x64);
    v0 &= v1;
    if (v0 != 0) goto loc_800170A4;
loc_80017090:
    v0 = lw(s1 + 0x58);
    a1 = lw(v0 + 0xC);
    a0 = s1;
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
loc_800170A4:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_BspiAttack() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017158;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_8001714C;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
    a0 = s1;
loc_8001714C:
    a1 = lw(a0 + 0x74);
    a2 = 0x1A;                                          // Result = 0000001A
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
loc_80017158:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_TroopAttack() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017298;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_800171FC;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_800171FC:
    v0 = lw(s1 + 0x64);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8001724C;
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_8001724C;
    v1 = lw(v0);
    a0 = lw(s1);
    v0 = lw(v0 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    v0 = P_AproxDistance(a0, a1);
    v1 = 0x450000;                                      // Result = 00450000
    v1 |= 0xFFFF;                                       // Result = 0045FFFF
    v1 = (i32(v1) < i32(v0));
    v1 ^= 1;
loc_8001724C:
    a0 = s1;
    if (v1 == 0) goto loc_8001728C;
    a1 = sfx_claw;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    _thunk_P_Random();
    a0 = lw(s1 + 0x74);
    a1 = s1;
    a2 = a1;
    v0 &= 7;
    v0++;
    a3 = v0 << 1;
    a3 += v0;
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    goto loc_80017298;
loc_8001728C:
    a1 = lw(a0 + 0x74);
    a2 = 0x14;                                          // Result = 00000014
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
loc_80017298:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_SargAttack() noexcept {
    sp -= 0x28;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(ra, sp + 0x20);
    sw(s0, sp + 0x18);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017368;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_8001733C;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_8001733C:
    _thunk_P_Random();
    a0 = s1;
    a2 = 0x460000;                                      // Result = 00460000
    v0 &= 7;
    v0++;
    v0 <<= 2;
    sw(v0, sp + 0x10);
    a1 = lw(a0 + 0x24);
    a3 = 0;                                             // Result = 00000000
    P_LineAttack(*vmAddrToPtr<mobj_t>(a0), a1, a2, a3, lw(sp + 0x10));
loc_80017368:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void A_HeadAttack() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_8001749C;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_8001740C;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_8001740C:
    v0 = lw(s1 + 0x64);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8001745C;
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_8001745C;
    v1 = lw(v0);
    a0 = lw(s1);
    v0 = lw(v0 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    v0 = P_AproxDistance(a0, a1);
    v1 = 0x450000;                                      // Result = 00450000
    v1 |= 0xFFFF;                                       // Result = 0045FFFF
    v1 = (i32(v1) < i32(v0));
    v1 ^= 1;
loc_8001745C:
    a0 = s1;
    if (v1 == 0) goto loc_80017490;
    _thunk_P_Random();
    a0 = lw(s1 + 0x74);
    a1 = s1;
    a2 = s1;
    v0 &= 7;
    v0++;
    a3 = v0 << 3;
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    goto loc_8001749C;
loc_80017490:
    a1 = lw(a0 + 0x74);
    a2 = 0x15;                                          // Result = 00000015
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
loc_8001749C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_CyberAttack() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017550;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80017544;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
    a0 = s1;
loc_80017544:
    a1 = lw(a0 + 0x74);
    a2 = 0x17;                                          // Result = 00000017
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
loc_80017550:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_BruisAttack() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a1 = lw(s0 + 0x74);
    v1 = 0x4000000;                                     // Result = 04000000
    if (a1 == 0) goto loc_8001761C;
    v0 = lw(s0 + 0x64);
    v0 &= v1;
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_800175C8;
    v1 = lw(a1);
    a0 = lw(s0);
    v0 = lw(a1 + 0x4);
    a1 = lw(s0 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    v0 = P_AproxDistance(a0, a1);
    v1 = 0x450000;                                      // Result = 00450000
    v1 |= 0xFFFF;                                       // Result = 0045FFFF
    v1 = (i32(v1) < i32(v0));
    v1 ^= 1;
loc_800175C8:
    a0 = s0;
    if (v1 == 0) goto loc_80017610;
    a1 = sfx_claw;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    _thunk_P_Random();
    a0 = lw(s0 + 0x74);
    a1 = s0;
    a2 = a1;
    v0 &= 7;
    v0++;
    a3 = v0 << 1;
    a3 += v0;
    a3 <<= 2;
    a3 -= v0;
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    goto loc_8001761C;
loc_80017610:
    a1 = lw(a0 + 0x74);
    a2 = 0x16;                                          // Result = 00000016
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
loc_8001761C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_SkelMissile() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017718;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_800176C0;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
    a0 = s1;
loc_800176C0:
    a2 = 4;                                             // Result = 00000004
    s0 = 0x100000;                                      // Result = 00100000
    v0 = lw(s1 + 0x8);
    a1 = lw(s1 + 0x74);
    v0 += s0;
    sw(v0, s1 + 0x8);
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
    v1 = lw(s1 + 0x8);
    v1 -= s0;
    sw(v1, s1 + 0x8);
    v1 = lw(v0);
    a1 = lw(v0 + 0x48);
    a0 = lw(v0 + 0x4);
    a2 = lw(v0 + 0x4C);
    v1 += a1;
    a0 += a2;
    sw(v1, v0);
    sw(a0, v0 + 0x4);
    v1 = lw(s1 + 0x74);
    sw(v1, v0 + 0x90);
loc_80017718:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_Tracer() noexcept {
    v0 = *gGameTic;
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    v0 &= 3;
    sw(s0, sp + 0x10);
    if (v0 != 0) goto loc_80017964;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    a2 = lw(s1 + 0x8);
    P_SpawnPuff(a0, a1, a2);
    a3 = 5;                                             // Result = 00000005
    a2 = lw(s1 + 0x8);
    v1 = lw(s1);
    a0 = lw(s1 + 0x48);
    v0 = lw(s1 + 0x4);
    a1 = lw(s1 + 0x4C);
    a0 = v1 - a0;
    a1 = v0 - a1;
    v0 = ptrToVmAddr(P_SpawnMobj(a0, a1, a2, (mobjtype_t) a3));
    s0 = v0;
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s0 + 0x50);
    _thunk_P_Random();
    v1 = lw(s0 + 0x5C);
    v0 &= 3;
    v1 -= v0;
    sw(v1, s0 + 0x5C);
    if (i32(v1) > 0) goto loc_800177BC;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0x5C);
loc_800177BC:
    s2 = lw(s1 + 0x90);
    if (s2 == 0) goto loc_80017964;
    v0 = lw(s2 + 0x68);
    if (i32(v0) <= 0) goto loc_80017964;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    a2 = lw(s2);
    a3 = lw(s2 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x24);
    s0 = v0;
    v0 = s0 - v1;
    if (s0 == v1) goto loc_80017850;
    a0 = 0x80000000;                                    // Result = 80000000
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 == 0);
        v0 = 0xF4000000;                                // Result = F4000000
        if (bJump) goto loc_80017830;
    }
    v0 += v1;
    sw(v0, s1 + 0x24);
    v0 = s0 - v0;
    if (i32(v0) < 0) goto loc_80017850;
    sw(s0, s1 + 0x24);
    goto loc_80017850;
loc_80017830:
    v0 = 0xC000000;                                     // Result = 0C000000
    v0 += v1;
    sw(v0, s1 + 0x24);
    v0 = s0 - v0;
    v0 = (a0 < v0);
    if (v0 == 0) goto loc_80017850;
    sw(s0, s1 + 0x24);
loc_80017850:
    v0 = lw(s1 + 0x24);
    v1 = lw(s1 + 0x58);
    s0 = v0 >> 19;
    s0 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a0 = lw(v1 + 0x3C);
    v0 += s0;
    a1 = lw(v0);
    _thunk_FixedMul();
    sw(v0, s1 + 0x48);
    v0 = lw(s1 + 0x58);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = lw(v0 + 0x3C);
    _thunk_FixedMul();
    sw(v0, s1 + 0x4C);
    v1 = lw(s2);
    a0 = lw(s1);
    v0 = lw(s2 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    v0 = P_AproxDistance(a0, a1);
    v1 = lw(s1 + 0x58);
    v1 = lw(v1 + 0x3C);
    div(v0, v1);
    if (v1 != 0) goto loc_800178E0;
    _break(0x1C00);
loc_800178E0:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_800178F8;
    }
    if (v0 != at) goto loc_800178F8;
    tge(zero, zero, 0x5D);
loc_800178F8:
    a1 = lo;
    a0 = 0xFFD80000;                                    // Result = FFD80000
    if (i32(a1) > 0) goto loc_8001790C;
    a1 = 1;                                             // Result = 00000001
loc_8001790C:
    v1 = lw(s1 + 0x8);
    v0 = lw(s2 + 0x8);
    v1 += a0;
    v0 -= v1;
    div(v0, a1);
    if (a1 != 0) goto loc_8001792C;
    _break(0x1C00);
loc_8001792C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80017944;
    }
    if (v0 != at) goto loc_80017944;
    tge(zero, zero, 0x5D);
loc_80017944:
    v0 = lo;
    v1 = lw(s1 + 0x50);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = v1 - 0x2000;
        if (bJump) goto loc_80017960;
    }
    v0 = v1 + 0x2000;
loc_80017960:
    sw(v0, s1 + 0x50);
loc_80017964:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_SkelWhoosh() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017A18;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80017A10;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
    a0 = s1;
loc_80017A10:
    a1 = sfx_skeswg;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
loc_80017A18:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_SkelFist() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017B78;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80017ABC;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80017ABC:
    v0 = lw(s1 + 0x64);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 &= v1;
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80017B0C;
    v0 = lw(s1 + 0x74);
    if (v0 == 0) goto loc_80017B0C;
    v1 = lw(v0);
    a0 = lw(s1);
    v0 = lw(v0 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    v0 = P_AproxDistance(a0, a1);
    v1 = 0x450000;                                      // Result = 00450000
    v1 |= 0xFFFF;                                       // Result = 0045FFFF
    v1 = (i32(v1) < i32(v0));
    v1 ^= 1;
loc_80017B0C:
    if (v1 == 0) goto loc_80017B78;
    _thunk_P_Random();
    v1 = 0x66660000;                                    // Result = 66660000
    v1 |= 0x6667;                                       // Result = 66666667
    mult(v0, v1);
    a0 = s1;
    a1 = sfx_skepch;
    v1 = u32(i32(v0) >> 31);
    a2 = hi;
    a2 = u32(i32(a2) >> 2);
    a2 -= v1;
    v1 = a2 << 2;
    v1 += a2;
    v1 <<= 1;
    v0 -= v1;
    v0++;
    s0 = v0 << 1;
    s0 += v0;
    s0 <<= 1;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    a0 = lw(s1 + 0x74);
    a1 = s1;
    a2 = a1;
    a3 = s0;
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
loc_80017B78:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_FatRaise() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017C1C;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80017C20;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80017C1C:
    a0 = s1;
loc_80017C20:
    a1 = sfx_manatk;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_FatAttack1() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    s2 = 0x8000000;                                     // Result = 08000000
    if (v0 == 0) goto loc_80017CD4;
    a2 = -0x21;                                         // Result = FFFFFFDF
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80017CD8;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80017CD4:
    a0 = s1;
loc_80017CD8:
    a2 = 7;                                             // Result = 00000007
    v0 = lw(s1 + 0x24);
    a1 = lw(s1 + 0x74);
    v0 += s2;
    sw(v0, s1 + 0x24);
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
    a0 = s1;
    a1 = lw(a0 + 0x74);
    a2 = 7;                                             // Result = 00000007
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
    s1 = v0;
    s0 = lw(s1 + 0x24);
    v0 = lw(s1 + 0x58);
    s0 += s2;
    sw(s0, s1 + 0x24);
    s0 >>= 19;
    a0 = lw(v0 + 0x3C);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    s0 <<= 2;
    v0 += s0;
    a1 = lw(v0);
    _thunk_FixedMul();
    sw(v0, s1 + 0x48);
    v0 = lw(s1 + 0x58);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = lw(v0 + 0x3C);
    _thunk_FixedMul();
    sw(v0, s1 + 0x4C);
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_FatAttack2() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x74);
    a0 = s1;
    if (v0 == 0) goto loc_80017E10;
    a2 = -0x21;                                         // Result = FFFFFFDF
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80017E10;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
    a0 = s1;
loc_80017E10:
    a2 = 7;                                             // Result = 00000007
    v1 = 0xF8000000;                                    // Result = F8000000
    v0 = lw(s1 + 0x24);
    a1 = lw(s1 + 0x74);
    v0 += v1;
    sw(v0, s1 + 0x24);
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
    a0 = s1;
    a1 = lw(a0 + 0x74);
    a2 = 7;                                             // Result = 00000007
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
    s1 = v0;
    v0 = 0xF0000000;                                    // Result = F0000000
    s0 = lw(s1 + 0x24);
    v1 = lw(s1 + 0x58);
    s0 += v0;
    sw(s0, s1 + 0x24);
    s0 >>= 19;
    s0 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a0 = lw(v1 + 0x3C);
    v0 += s0;
    a1 = lw(v0);
    _thunk_FixedMul();
    sw(v0, s1 + 0x48);
    v0 = lw(s1 + 0x58);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = lw(v0 + 0x3C);
    _thunk_FixedMul();
    sw(v0, s1 + 0x4C);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_FatAttack3() noexcept {
    sp -= 0x28;
    sw(s2, sp + 0x20);
    s2 = a0;
    sw(ra, sp + 0x24);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    v0 = lw(s2 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80017F48;
    a0 = lw(s2);
    a1 = lw(s2 + 0x4);
    v0 = lw(s2 + 0x64);
    v1 = lw(s2 + 0x74);
    v0 &= a2;
    sw(v0, s2 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s2 + 0x74);
    sw(v0, s2 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    a0 = s2;
    if (v0 == 0) goto loc_80017F4C;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s2 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s2 + 0x24);
loc_80017F48:
    a0 = s2;
loc_80017F4C:
    a1 = lw(s2 + 0x74);
    a2 = 7;                                             // Result = 00000007
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
    s1 = v0;
    v0 = lw(s1 + 0x24);
    v1 = 0xFC000000;                                    // Result = FC000000
    v0 += v1;
    s0 = v0 >> 19;
    s0 <<= 2;
    sw(v0, s1 + 0x24);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = lw(s1 + 0x58);
    v0 += s0;
    a0 = lw(v1 + 0x3C);
    a1 = lw(v0);
    _thunk_FixedMul();
    sw(v0, s1 + 0x48);
    v0 = lw(s1 + 0x58);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = lw(v0 + 0x3C);
    _thunk_FixedMul();
    a0 = s2;
    sw(v0, s1 + 0x4C);
    a1 = lw(a0 + 0x74);
    a2 = 7;                                             // Result = 00000007
    v0 = ptrToVmAddr(P_SpawnMissile(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1), (mobjtype_t) a2));
    s1 = v0;
    v0 = lw(s1 + 0x24);
    v1 = 0x4000000;                                     // Result = 04000000
    v0 += v1;
    s0 = v0 >> 19;
    s0 <<= 2;
    sw(v0, s1 + 0x24);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = lw(s1 + 0x58);
    v0 += s0;
    a0 = lw(v1 + 0x3C);
    a1 = lw(v0);
    _thunk_FixedMul();
    sw(v0, s1 + 0x48);
    v0 = lw(s1 + 0x58);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = lw(v0 + 0x3C);
    _thunk_FixedMul();
    sw(v0, s1 + 0x4C);
    ra = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void A_SkullAttack() noexcept {
loc_8001804C:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
    a2 = lw(s1 + 0x74);
    v1 = 0x1000000;                                     // Result = 01000000
    if (a2 == 0) goto loc_800181E0;
    v0 = lw(s1 + 0x64);
    a1 = lw(s1 + 0x58);
    v0 |= v1;
    sw(v0, s1 + 0x64);
    a1 = lw(a1 + 0x18);
    s2 = a2;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = lw(s1 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80018108;
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    v0 = lw(s1 + 0x64);
    v1 = lw(s1 + 0x74);
    v0 &= a2;
    sw(v0, s1 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s1 + 0x74);
    sw(v0, s1 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_80018108;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s1 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s1 + 0x24);
loc_80018108:
    s0 = lw(s1 + 0x24);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    s0 >>= 19;
    s0 <<= 2;
    v0 += s0;
    a1 = lw(v0);
    a0 = 0x280000;                                      // Result = 00280000
    _thunk_FixedMul();
    sw(v0, s1 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s0;
    a1 = lw(at);
    a0 = 0x280000;                                      // Result = 00280000
    _thunk_FixedMul();
    sw(v0, s1 + 0x4C);
    v1 = lw(s2);
    a0 = lw(s1);
    v0 = lw(s2 + 0x4);
    a1 = lw(s1 + 0x4);
    a0 = v1 - a0;
    a1 = v0 - a1;
    v0 = P_AproxDistance(a0, a1);
    a1 = v0;
    v0 = 0x66660000;                                    // Result = 66660000
    v0 |= 0x6667;                                       // Result = 66666667
    mult(a1, v0);
    v1 = u32(i32(a1) >> 31);
    v0 = hi;
    v0 = u32(i32(v0) >> 20);
    a1 = v0 - v1;
    if (i32(a1) > 0) goto loc_80018194;
    a1 = 1;                                             // Result = 00000001
loc_80018194:
    v0 = lw(s2 + 0x44);
    v1 = lw(s2 + 0x8);
    a0 = lw(s1 + 0x8);
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    v0 -= a0;
    div(v0, a1);
    if (a1 != 0) goto loc_800181BC;
    _break(0x1C00);
loc_800181BC:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_800181D4;
    }
    if (v0 != at) goto loc_800181D4;
    tge(zero, zero, 0x5D);
loc_800181D4:
    v0 = lo;
    sw(v0, s1 + 0x50);
loc_800181E0:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_PainShootSkull() noexcept {
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a0;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6554;                                       // Result = gThinkerCap[1] (80096554)
    sw(ra, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v1 = lw(v0);                                        // Load from: gThinkerCap[1] (80096554)
    v0 -= 4;                                            // Result = gThinkerCap[0] (80096550)
    a0 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_80018270;
    t0 = 0x80010000;                                    // Result = 80010000
    t0 += 0x3DE0;                                       // Result = P_MobjThinker (80013DE0)
    a3 = 0xE;                                           // Result = 0000000E
    a2 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_8001823C:
    v0 = lw(v1 + 0x8);
    if (v0 != t0) goto loc_80018260;
    v0 = lw(v1 + 0x54);
    if (v0 != a3) goto loc_80018260;
    a0++;                                               // Result = 00000001
loc_80018260:
    v1 = lw(v1 + 0x4);
    if (v1 != a2) goto loc_8001823C;
loc_80018270:
    v0 = (i32(a0) < 0x15);                              // Result = 00000001
    s1 = a1 >> 19;
    if (v0 == 0) goto loc_80018334;
    s1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = 0x40000;                                       // Result = 00040000
    v0 += s1;
    a1 = lw(v0);
    v0 = lw(s2 + 0x58);
    s0 = 0x80060000;                                    // Result = 80060000
    s0 = lw(s0 - 0x1AB4);                               // Load from: MObjInfo_MT_SKULL[10] (8005E54C)
    v0 = lw(v0 + 0x40);
    s0 += v1;
    s0 += v0;
    a0 = s0;
    _thunk_FixedMul();
    a0 = s0;
    v1 = lw(s2);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s1;
    a1 = lw(at);
    s0 = v0 + v1;
    _thunk_FixedMul();
    a0 = s0;
    a1 = lw(s2 + 0x4);
    a3 = 0xE;                                           // Result = 0000000E
    a1 += v0;
    v0 = lw(s2 + 0x8);
    a2 = 0x80000;                                       // Result = 00080000
    a2 += v0;
    v0 = ptrToVmAddr(P_SpawnMobj(a0, a1, a2, (mobjtype_t) a3));
    s0 = v0;
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a0 = s0;
    v0 = P_TryMove(*vmAddrToPtr<mobj_t>(a0), a1, a2);
    a0 = s0;
    if (v0 != 0) goto loc_80018328;
    a1 = s2;
    a2 = a1;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    goto loc_80018334;
loc_80018328:
    v0 = lw(s2 + 0x74);
    sw(v0, a0 + 0x74);
    A_SkullAttack();
loc_80018334:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_PainAttack() noexcept {
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(ra, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lw(s2 + 0x74);
    a2 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_80018504;
    a0 = lw(s2);
    a1 = lw(s2 + 0x4);
    v0 = lw(s2 + 0x64);
    v1 = lw(s2 + 0x74);
    v0 &= a2;
    sw(v0, s2 + 0x64);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    v0 = R_PointToAngle2(a0, a1, a2, a3);
    v1 = lw(s2 + 0x74);
    sw(v0, s2 + 0x24);
    v0 = lw(v1 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 &= v1;
    if (v0 == 0) goto loc_800183E0;
    _thunk_P_Random();
    s0 = v0;
    _thunk_P_Random();
    s0 -= v0;
    v0 = lw(s2 + 0x24);
    s0 <<= 21;
    s0 += v0;
    sw(s0, s2 + 0x24);
loc_800183E0:
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x6554);                               // Load from: gThinkerCap[1] (80096554)
    s1 = lw(s2 + 0x24);
    a0 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_80018440;
    a3 = 0x80010000;                                    // Result = 80010000
    a3 += 0x3DE0;                                       // Result = P_MobjThinker (80013DE0)
    a2 = 0xE;                                           // Result = 0000000E
    a1 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_8001840C:
    v0 = lw(v1 + 0x8);
    if (v0 != a3) goto loc_80018430;
    v0 = lw(v1 + 0x54);
    if (v0 != a2) goto loc_80018430;
    a0++;                                               // Result = 00000001
loc_80018430:
    v1 = lw(v1 + 0x4);
    if (v1 != a1) goto loc_8001840C;
loc_80018440:
    v0 = (i32(a0) < 0x15);                              // Result = 00000001
    s1 >>= 19;
    if (v0 == 0) goto loc_80018504;
    s1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = 0x40000;                                       // Result = 00040000
    v0 += s1;
    a1 = lw(v0);
    v0 = lw(s2 + 0x58);
    s0 = 0x80060000;                                    // Result = 80060000
    s0 = lw(s0 - 0x1AB4);                               // Load from: MObjInfo_MT_SKULL[10] (8005E54C)
    v0 = lw(v0 + 0x40);
    s0 += v1;
    s0 += v0;
    a0 = s0;
    _thunk_FixedMul();
    a0 = s0;
    v1 = lw(s2);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s1;
    a1 = lw(at);
    s0 = v0 + v1;
    _thunk_FixedMul();
    a0 = s0;
    a1 = lw(s2 + 0x4);
    a3 = 0xE;                                           // Result = 0000000E
    a1 += v0;
    v0 = lw(s2 + 0x8);
    a2 = 0x80000;                                       // Result = 00080000
    a2 += v0;
    v0 = ptrToVmAddr(P_SpawnMobj(a0, a1, a2, (mobjtype_t) a3));
    s0 = v0;
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a0 = s0;
    v0 = P_TryMove(*vmAddrToPtr<mobj_t>(a0), a1, a2);
    a0 = s0;
    if (v0 != 0) goto loc_800184F8;
    a1 = s2;
    a2 = a1;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    goto loc_80018504;
loc_800184F8:
    v0 = lw(s2 + 0x74);
    sw(v0, a0 + 0x74);
    A_SkullAttack();
loc_80018504:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_PainDie() noexcept {
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(ra, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    A_Fall();
    v0 = 0x40000000;                                    // Result = 40000000
    v1 = lw(s2 + 0x24);
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x6554);                               // Load from: gThinkerCap[1] (80096554)
    v1 += v0;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    a1 = 0;                                             // Result = 00000000
    if (a0 == v0) goto loc_800185A4;
    t0 = 0x80010000;                                    // Result = 80010000
    t0 += 0x3DE0;                                       // Result = P_MobjThinker (80013DE0)
    a3 = 0xE;                                           // Result = 0000000E
    a2 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_80018570:
    v0 = lw(a0 + 0x8);
    if (v0 != t0) goto loc_80018594;
    v0 = lw(a0 + 0x54);
    if (v0 != a3) goto loc_80018594;
    a1++;                                               // Result = 00000001
loc_80018594:
    a0 = lw(a0 + 0x4);
    if (a0 != a2) goto loc_80018570;
loc_800185A4:
    v0 = (i32(a1) < 0x15);                              // Result = 00000001
    s1 = v1 >> 19;
    if (v0 == 0) goto loc_80018668;
    s1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = 0x40000;                                       // Result = 00040000
    v0 += s1;
    a1 = lw(v0);
    v0 = lw(s2 + 0x58);
    s0 = 0x80060000;                                    // Result = 80060000
    s0 = lw(s0 - 0x1AB4);                               // Load from: MObjInfo_MT_SKULL[10] (8005E54C)
    v0 = lw(v0 + 0x40);
    s0 += v1;
    s0 += v0;
    a0 = s0;
    _thunk_FixedMul();
    a0 = s0;
    v1 = lw(s2);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s1;
    a1 = lw(at);
    s0 = v0 + v1;
    _thunk_FixedMul();
    a0 = s0;
    a1 = lw(s2 + 0x4);
    a3 = 0xE;                                           // Result = 0000000E
    a1 += v0;
    v0 = lw(s2 + 0x8);
    a2 = 0x80000;                                       // Result = 00080000
    a2 += v0;
    v0 = ptrToVmAddr(P_SpawnMobj(a0, a1, a2, (mobjtype_t) a3));
    s0 = v0;
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a0 = s0;
    v0 = P_TryMove(*vmAddrToPtr<mobj_t>(a0), a1, a2);
    a0 = s0;
    if (v0 != 0) goto loc_8001865C;
    a1 = s2;
    a2 = s2;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    v0 = 0x80000000;                                    // Result = 80000000
    goto loc_8001866C;
loc_8001865C:
    v0 = lw(s2 + 0x74);
    sw(v0, a0 + 0x74);
    A_SkullAttack();
loc_80018668:
    v0 = 0x80000000;                                    // Result = 80000000
loc_8001866C:
    v1 = lw(s2 + 0x24);
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x6554);                               // Load from: gThinkerCap[1] (80096554)
    v1 -= v0;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    a1 = 0;                                             // Result = 00000000
    if (a0 == v0) goto loc_800186D0;
    t0 = 0x80010000;                                    // Result = 80010000
    t0 += 0x3DE0;                                       // Result = P_MobjThinker (80013DE0)
    a3 = 0xE;                                           // Result = 0000000E
    a2 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_8001869C:
    v0 = lw(a0 + 0x8);
    if (v0 != t0) goto loc_800186C0;
    v0 = lw(a0 + 0x54);
    if (v0 != a3) goto loc_800186C0;
    a1++;                                               // Result = 00000001
loc_800186C0:
    a0 = lw(a0 + 0x4);
    if (a0 != a2) goto loc_8001869C;
loc_800186D0:
    v0 = (i32(a1) < 0x15);                              // Result = 00000001
    s1 = v1 >> 19;
    if (v0 == 0) goto loc_80018794;
    s1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = 0x40000;                                       // Result = 00040000
    v0 += s1;
    a1 = lw(v0);
    v0 = lw(s2 + 0x58);
    s0 = 0x80060000;                                    // Result = 80060000
    s0 = lw(s0 - 0x1AB4);                               // Load from: MObjInfo_MT_SKULL[10] (8005E54C)
    v0 = lw(v0 + 0x40);
    s0 += v1;
    s0 += v0;
    a0 = s0;
    _thunk_FixedMul();
    a0 = s0;
    v1 = lw(s2);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s1;
    a1 = lw(at);
    s0 = v0 + v1;
    _thunk_FixedMul();
    a0 = s0;
    a1 = lw(s2 + 0x4);
    a3 = 0xE;                                           // Result = 0000000E
    a1 += v0;
    v0 = lw(s2 + 0x8);
    a2 = 0x80000;                                       // Result = 00080000
    a2 += v0;
    v0 = ptrToVmAddr(P_SpawnMobj(a0, a1, a2, (mobjtype_t) a3));
    s0 = v0;
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a0 = s0;
    v0 = P_TryMove(*vmAddrToPtr<mobj_t>(a0), a1, a2);
    a0 = s0;
    if (v0 != 0) goto loc_80018788;
    a1 = s2;
    a2 = s2;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    v0 = 0xC0000000;                                    // Result = C0000000
    goto loc_80018798;
loc_80018788:
    v0 = lw(s2 + 0x74);
    sw(v0, a0 + 0x74);
    A_SkullAttack();
loc_80018794:
    v0 = 0xC0000000;                                    // Result = C0000000
loc_80018798:
    v1 = lw(s2 + 0x24);
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x6554);                               // Load from: gThinkerCap[1] (80096554)
    v1 += v0;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    a1 = 0;                                             // Result = 00000000
    if (a0 == v0) goto loc_800187FC;
    t0 = 0x80010000;                                    // Result = 80010000
    t0 += 0x3DE0;                                       // Result = P_MobjThinker (80013DE0)
    a3 = 0xE;                                           // Result = 0000000E
    a2 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_800187C8:
    v0 = lw(a0 + 0x8);
    if (v0 != t0) goto loc_800187EC;
    v0 = lw(a0 + 0x54);
    if (v0 != a3) goto loc_800187EC;
    a1++;                                               // Result = 00000001
loc_800187EC:
    a0 = lw(a0 + 0x4);
    if (a0 != a2) goto loc_800187C8;
loc_800187FC:
    v0 = (i32(a1) < 0x15);                              // Result = 00000001
    s1 = v1 >> 19;
    if (v0 == 0) goto loc_800188C0;
    s1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = 0x40000;                                       // Result = 00040000
    v0 += s1;
    a1 = lw(v0);
    v0 = lw(s2 + 0x58);
    s0 = 0x80060000;                                    // Result = 80060000
    s0 = lw(s0 - 0x1AB4);                               // Load from: MObjInfo_MT_SKULL[10] (8005E54C)
    v0 = lw(v0 + 0x40);
    s0 += v1;
    s0 += v0;
    a0 = s0;
    _thunk_FixedMul();
    a0 = s0;
    v1 = lw(s2);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += s1;
    a1 = lw(at);
    s0 = v0 + v1;
    _thunk_FixedMul();
    a0 = s0;
    a1 = lw(s2 + 0x4);
    a3 = 0xE;                                           // Result = 0000000E
    a1 += v0;
    v0 = lw(s2 + 0x8);
    a2 = 0x80000;                                       // Result = 00080000
    a2 += v0;
    v0 = ptrToVmAddr(P_SpawnMobj(a0, a1, a2, (mobjtype_t) a3));
    s0 = v0;
    a1 = lw(s0);
    a2 = lw(s0 + 0x4);
    a0 = s0;
    v0 = P_TryMove(*vmAddrToPtr<mobj_t>(a0), a1, a2);
    a0 = s0;
    if (v0 != 0) goto loc_800188B4;
    a1 = s2;
    a2 = a1;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    goto loc_800188C0;
loc_800188B4:
    v0 = lw(s2 + 0x74);
    sw(v0, a0 + 0x74);
    A_SkullAttack();
loc_800188C0:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_Scream() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v0 = lw(s0 + 0x58);
    v1 = lw(v0 + 0x38);
    v0 = (v1 < 0x33);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_8001894C;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x2C0;                                        // Result = JumpTable_A_Scream[0] (800102C0)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80018980: goto loc_80018980;
        case 0x8001894C: goto loc_8001894C;
        case 0x80018924: goto loc_80018924;
        case 0x80018938: goto loc_80018938;
        default: jump_table_err(); break;
    }
loc_80018924:
    _thunk_P_Random();
    v0 &= 1;
    a1 = v0 + 0x27;
    goto loc_80018958;
loc_80018938:
    _thunk_P_Random();
    v0 &= 1;
    a1 = v0 + 0x31;
    goto loc_80018958;
loc_8001894C:
    v0 = lw(s0 + 0x58);
    a1 = lw(v0 + 0x38);
loc_80018958:
    v1 = lw(s0 + 0x54);
    v0 = 0xF;                                           // Result = 0000000F
    a0 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_80018978;
    v0 = 0x11;                                          // Result = 00000011
    if (v1 == v0) goto loc_80018978;
    a0 = s0;
loc_80018978:
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
loc_80018980:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_XScream() noexcept {
    a1 = sfx_slop;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
}

void A_Pain() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(a0 + 0x58);
    a1 = lw(v0 + 0x24);
    if (a1 == 0) goto loc_800189DC;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
loc_800189DC:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_Fall() noexcept {
loc_800189EC:
    v0 = lw(a0 + 0x64);
    v1 = -3;                                            // Result = FFFFFFFD
    v0 &= v1;
    sw(v0, a0 + 0x64);
    return;
}

void A_Explode() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = lw(a0 + 0x74);
    a2 = 0x80;                                          // Result = 00000080
    P_RadiusAttack(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), a2);
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_BossDeath() noexcept {
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    sp -= 0x68;
    v0 = a1 & 1;
    sw(ra, sp + 0x60);
    if (v0 == 0) goto loc_80018A54;
    v0 = 0x29A;                                         // Result = 0000029A
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 6;                                             // Result = 00000006
    if (v1 == v0) goto loc_80018AF4;
loc_80018A54:
    v0 = a1 & 2;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x29B;                                     // Result = 0000029B
        if (bJump) goto loc_80018A74;
    }
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 0x10;                                          // Result = 00000010
    if (v1 == v0) goto loc_80018AF4;
loc_80018A74:
    v0 = a1 & 4;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x29C;                                     // Result = 0000029C
        if (bJump) goto loc_80018A94;
    }
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 0xF;                                           // Result = 0000000F
    if (v1 == v0) goto loc_80018AF4;
loc_80018A94:
    v0 = a1 & 8;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x29D;                                     // Result = 0000029D
        if (bJump) goto loc_80018AB4;
    }
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 0xD;                                           // Result = 0000000D
    if (v1 == v0) goto loc_80018AF4;
loc_80018AB4:
    v0 = a1 & 0x10;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x29E;                                     // Result = 0000029E
        if (bJump) goto loc_80018AD4;
    }
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 0x11;                                          // Result = 00000011
    if (v1 == v0) goto loc_80018AF4;
loc_80018AD4:
    v0 = a1 & 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x29F;                                     // Result = 0000029F
        if (bJump) goto loc_80018C34;
    }
    sw(v0, sp + 0x28);
    v1 = lw(a0 + 0x54);
    v0 = 0xC;                                           // Result = 0000000C
    if (v1 != v0) goto loc_80018C34;
loc_80018AF4:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    a1 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 -= 0x14;                                         // Result = gMObjHead[0] (800A8E90)
    if (a1 == v0) goto loc_80018B4C;
    a2 = v0;                                            // Result = gMObjHead[0] (800A8E90)
loc_80018B10:
    if (a1 == a0) goto loc_80018B3C;
    v1 = lw(a1 + 0x54);
    v0 = lw(a0 + 0x54);
    if (v1 != v0) goto loc_80018B3C;
    v0 = lw(a1 + 0x68);
    if (i32(v0) > 0) goto loc_80018C34;
loc_80018B3C:
    a1 = lw(a1 + 0x14);
    if (a1 != a2) goto loc_80018B10;
loc_80018B4C:
    v0 = lw(sp + 0x28);
    v1 = v0 - 0x29A;
    v0 = (v1 < 6);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80018C34;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x390;                                        // Result = JumpTable_A_BossDeath[0] (80010390)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80018B80: goto loc_80018B80;
        case 0x80018B98: goto loc_80018B98;
        case 0x80018BB0: goto loc_80018BB0;
        case 0x80018BC8: goto loc_80018BC8;
        case 0x80018BE0: goto loc_80018BE0;
        case 0x80018C0C: goto loc_80018C0C;
        default: jump_table_err(); break;
    }
loc_80018B80:
    a0 = sp + 0x10;
    a1 = 1;                                             // Result = 00000001
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -2;                                            // Result = FFFFFFFE
    goto loc_80018C20;
loc_80018B98:
    a0 = sp + 0x10;
    a1 = 7;                                             // Result = 00000007
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -3;                                            // Result = FFFFFFFD
    goto loc_80018C20;
loc_80018BB0:
    a0 = sp + 0x10;
    a1 = 1;                                             // Result = 00000001
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -5;                                            // Result = FFFFFFFB
    goto loc_80018C20;
loc_80018BC8:
    a0 = sp + 0x10;
    a1 = 1;                                             // Result = 00000001
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -9;                                            // Result = FFFFFFF7
    goto loc_80018C20;
loc_80018BE0:
    a0 = sp + 0x10;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -0x11;                                         // Result = FFFFFFEF
    v0 &= v1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7F88);                                // Store to: gMapBossSpecialFlags (80077F88)
    a1 = 3;                                             // Result = 00000003
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    goto loc_80018C34;
loc_80018C0C:
    a0 = sp + 0x10;
    a1 = 1;                                             // Result = 00000001
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v1 = -0x21;                                         // Result = FFFFFFDF
loc_80018C20:
    v0 &= v1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7F88);                                // Store to: gMapBossSpecialFlags (80077F88)
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
loc_80018C34:
    ra = lw(sp + 0x60);
    sp += 0x68;
    return;
}

void A_Hoof() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a1 = sfx_hoof;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    a0 = s0;
    A_Chase(*vmAddrToPtr<mobj_t>(a0));
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_Metal() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a1 = sfx_metal;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    a0 = s0;
    A_Chase(*vmAddrToPtr<mobj_t>(a0));
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_BabyMetal() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a1 = sfx_bspwlk;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    a0 = s0;
    A_Chase(*vmAddrToPtr<mobj_t>(a0));
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void L_MissileHit() noexcept {
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    s1 = lw(s0 + 0x84);
    if (s1 == 0) goto loc_80018D34;
    _thunk_P_Random();
    v1 = lw(s0 + 0x58);
    v0 &= 7;
    v1 = lw(v1 + 0x4C);
    v0++;
    mult(v0, v1);
    a0 = s1;
    a2 = lw(s0 + 0x74);
    a3 = lo;
    a1 = s0;
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
loc_80018D34:
    a0 = s0;
    P_ExplodeMissile(*vmAddrToPtr<mobj_t>(a0));
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void L_SkullBash() noexcept {
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    s1 = lw(s0 + 0x84);
    v0 = 0xFEFF0000;                                    // Result = FEFF0000
    if (s1 == 0) goto loc_80018DAC;
    _thunk_P_Random();
    v1 = lw(s0 + 0x58);
    v0 &= 7;
    v1 = lw(v1 + 0x4C);
    v0++;
    mult(v0, v1);
    a0 = s1;
    a1 = s0;
    a3 = lo;
    a2 = s0;
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    v0 = 0xFEFF0000;                                    // Result = FEFF0000
loc_80018DAC:
    v1 = lw(s0 + 0x64);
    a0 = lw(s0 + 0x58);
    v0 |= 0xFFFF;                                       // Result = FEFFFFFF
    sw(0, s0 + 0x50);
    sw(0, s0 + 0x4C);
    sw(0, s0 + 0x48);
    v1 &= v0;
    sw(v1, s0 + 0x64);
    a1 = lw(a0 + 0x4);
    a0 = s0;
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

// TODO: remove all these thunks
void _thunk_A_Look() noexcept { A_Look(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_Chase() noexcept { A_Chase(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_FaceTarget() noexcept { A_FaceTarget(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_PosAttack() noexcept { A_PosAttack(*vmAddrToPtr<mobj_t>(a0)); }
