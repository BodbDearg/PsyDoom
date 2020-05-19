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
#include "p_spec.h"
#include "p_switch.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"
#include <algorithm>

static constexpr angle_t TRACEANGLE = 0xC000000;        // How much Revenant missiles adjust their angle by when homing towards their target (angle adjust increment)
static constexpr angle_t FATSPREAD  = ANG90 / 8;        // Angle adjustment increment for the Mancubus when attacking; varies it's shoot direction in multiples of this constant
static constexpr fixed_t SKULLSPEED = 40 * FRACUNIT;    // Speed that lost souls fly at

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
// Does the attack for a Zombieman (pistol attack)
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for a Shotgun Guy
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SPosAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;

    S_StartSound(&actor, sfx_shotgn);
    A_FaceTarget(actor);

    // The shotgun fires 3 pellets
    for (int32_t i = 0; i < 3; ++i) {
        const angle_t shootAngle =  actor.angle + (P_Random() - P_Random()) * (ANG45 / 512);    // Vary by up to 22.5 degrees (approximately)
        const int32_t damage = (P_Random() % 5 + 1) * 3;                                        // 3-15 damage

        P_LineAttack(actor, shootAngle, MISSILERANGE, INT32_MAX, damage);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for a Heavy Weapons Dude (Chaingun Guy)
//------------------------------------------------------------------------------------------------------------------------------------------
void A_CPosAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;

    S_StartSound(&actor, sfx_pistol);
    A_FaceTarget(actor);

    const fixed_t aimZSlope = P_AimLineAttack(actor, actor.angle, MISSILERANGE);
    const angle_t shootAngle = actor.angle + (P_Random() - P_Random()) * (ANG45 / 512);     // Vary by up to 22.5 degrees (approximately)
    const int32_t damage = (P_Random() % 5 + 1) * 3;                                        // 3-15 damage

    P_LineAttack(actor, shootAngle, MISSILERANGE, aimZSlope, damage);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called just before firing again for the Heavy Weapons Dude (Chaingun Guy); decides whether to break out of the firing loop
//------------------------------------------------------------------------------------------------------------------------------------------
void A_CPosRefire(mobj_t& actor) noexcept {
    A_FaceTarget(actor);

    // Randomly keep firing every so often regardless of whether the target has been lost or not (trigger happy!)
    if (P_Random() < 40)
        return;

    // If the target has been lost go back to the normal active state
    mobj_t* const pTarget = actor.target.get();

    const bool bTargetLost = (
        (!pTarget) ||
        (pTarget->health <= 0) ||
        (!P_CheckSight(actor, *pTarget))
    );

    if (bTargetLost) {
        P_SetMObjState(actor, actor.info->seestate);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for the Spider Mastermind
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SpidAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;

    S_StartSound(&actor, sfx_pistol);
    A_FaceTarget(actor);
    
    // The Spider Mastermind fires 3 pellets per shot
    for (int32_t i = 0; i < 3; ++i) {
        const angle_t shootAngle = actor.angle + (P_Random() - P_Random()) * (ANG45 / 512);     // Vary by up to 22.5 degrees (approximately)
        const int32_t damage = (P_Random() % 5 + 1) * 3;                                        // 3-15 damage
        
        P_LineAttack(actor, shootAngle, MISSILERANGE, INT32_MAX, damage);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called just before firing again for the Spider Mastermind; decides whether to break out of the firing loop
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SpidRefire(mobj_t& actor) noexcept {
    A_FaceTarget(actor);

    // Randomly keep firing every so often regardless of whether the target has been lost or not (trigger happy!)
    if (P_Random() < 10)
        return;

    // If the target has been lost go back to the normal active state
    mobj_t* const pTarget = actor.target.get();

    const bool bTargetLost = (
        (!pTarget) ||
        (pTarget->health <= 0) ||
        ((actor.flags & MF_SEETARGET) == 0)
    );

    if (bTargetLost) {
        P_SetMObjState(actor, actor.info->seestate);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for an Arachnotron
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BspiAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;

    A_FaceTarget(actor);
    P_SpawnMissile(actor, *actor.target, MT_ARACHPLAZ);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for an Imp, which can either be a melee attack or sending a fireball towards the target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_TroopAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;

    A_FaceTarget(actor);

    // Do a melee attack if possible, otherwise spawn a fireball
    if (P_CheckMeleeRange(actor)) {
        S_StartSound(&actor, sfx_claw);
        const int32_t damage = ((P_Random() & 7) + 1) * 3;      // 3-24 damage
        P_DamageMObj(*actor.target, &actor, &actor, damage);
    } else {
        P_SpawnMissile(actor, *actor.target, MT_TROOPSHOT);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the melee attack for a Demon
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SargAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;
    
    A_FaceTarget(actor);
    const int32_t damage = ((P_Random() & 7) + 1) * 4;          // 4-32 damage
    P_LineAttack(actor, actor.angle, MELEERANGE, 0, damage);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for a Cacodemon, which can either be a melee attack or sending a fireball towards the target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_HeadAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;

    A_FaceTarget(actor);
    
    // Do a melee attack if possible, otherwise spawn a fireball
    if (P_CheckMeleeRange(actor)) {
        const int32_t damage = ((P_Random() & 7) + 1) * 8;      // 8-64 damage
        P_DamageMObj(*actor.target, &actor, &actor, damage);
    } else {
        P_SpawnMissile(actor, *actor.target, MT_HEADSHOT);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for a Cyberdemon
//------------------------------------------------------------------------------------------------------------------------------------------
void A_CyberAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;

    A_FaceTarget(actor);
    P_SpawnMissile(actor, *actor.target, MT_ROCKET);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for a Baron or Hell Knight, which can either be a melee attack or sending a fireball towards the target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BruisAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;
    
    mobj_t& target = *actor.target;

    if (P_CheckMeleeRange(actor)) {
        S_StartSound(&actor, sfx_claw);
        const int32_t damage = ((P_Random() & 7) + 1) * 11;     // 11-88 damage
        P_DamageMObj(target, &actor, &actor, damage);
    } else {
        P_SpawnMissile(actor, target, MT_BRUISERSHOT);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make a Revenant fire it's missile towards a player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SkelMissile(mobj_t& actor) noexcept {
    if (!actor.target)
        return;

    A_FaceTarget(actor);

    // Spawn the missile: also hack adjust the Revenant height slightly (temporarily) so the missile spawns higher
    actor.z += 16 * FRACUNIT;
    mobj_t& missile = *P_SpawnMissile(actor, *actor.target, MT_TRACER);
    actor.z -= 16 * FRACUNIT;

    // Move the missile a little and set it's target
    missile.x += missile.momx;
    missile.y += missile.momy;
    missile.tracer = actor.target;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the Revenant's homing missiles: adjusts the direction of the missile, does smoke effects etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Tracer(mobj_t& actor) noexcept {
    // Only spawn smoke and change direction every 4 tics
    if ((*gGameTic & 3) != 0)
        return;

    // Spawn a puff and some smoke behind the rocket
    P_SpawnPuff(actor.x, actor.y, actor.z);

    mobj_t& smoke = *P_SpawnMobj(actor.x - actor.momx, actor.y - actor.momy, actor.z, MT_SMOKE);
    smoke.momz = FRACUNIT;
    smoke.tics = std::max(smoke.tics - (P_Random() & 3), 1);

    // Only follow the target and change course if it's still alive
    mobj_t* const pTarget = actor.tracer.get();

    if ((!pTarget) || (pTarget->health <= 0))
        return;
    
    // Gradually adjust to face the target if we're not already pointing directly at it
    const angle_t angleToTgt = R_PointToAngle2(actor.x, actor.y, pTarget->x, pTarget->y);
    
    if (angleToTgt != actor.angle) {
        if (angleToTgt - actor.angle > ANG180) {
            actor.angle -= TRACEANGLE;

            if (angleToTgt - actor.angle < ANG180) {    // Did we over adjust?
                actor.angle = angleToTgt;
            }
        } else {
            actor.angle += TRACEANGLE;

            if (angleToTgt - actor.angle > ANG180) {    // Did we over adjust?
                actor.angle = angleToTgt;
            }
        }
    }

    // Figure out the x/y momentum based on the direction the missile is headed in
    {
        const uint32_t fineAngle = actor.angle >> ANGLETOFINESHIFT;
        actor.momx = FixedMul(actor.info->speed, gFineCosine[fineAngle]);
        actor.momy = FixedMul(actor.info->speed, gFineSine[fineAngle]);
    }

    // Figure out how long it will take to reach the target
    const fixed_t distToTgt = P_AproxDistance(pTarget->x - actor.x, pTarget->y - actor.y);
    const int32_t travelTics = std::max(distToTgt / actor.info->speed, 1);

    // Figure out the desired z velocity to fly in a straight line to the target.
    // Note: with the '-40.0' adjustment we make the missile aim for the target 40.0 units above it's floor position.
    const fixed_t missileAnchoredPos = actor.z - 40 * FRACUNIT;
    const fixed_t zDelta = pTarget->z - missileAnchoredPos;
    const fixed_t tgtZVelocity = zDelta / travelTics;

    // Gradually ramp up or down towards the desired Z velocity to hit the target at the position wanted
    if (actor.momz <= tgtZVelocity) {
        actor.momz += FRACUNIT / 8;
    } else {
        actor.momz -= FRACUNIT / 8;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// If there is a target plays the Revenant's punch swing sound and faces the target in preparation for a melee attack
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SkelWhoosh(mobj_t& actor) noexcept {
    if (!actor.target)
        return;
    
    A_FaceTarget(actor);
    S_StartSound(&actor, sfx_skeswg);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the melee attack of the Revenant
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SkelFist(mobj_t& actor) noexcept {
    if (!actor.target)
        return;

    A_FaceTarget(actor);

    if (P_CheckMeleeRange(actor)) {
        S_StartSound(&actor, sfx_skepch);
        const int32_t damage = (P_Random() % 10 + 1) * 6;       // 6-60 damage
        P_DamageMObj(*actor.target, &actor, &actor, damage);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// First action in the Mancubus's attack sequence: face the target and make a noise
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FatRaise(mobj_t& actor) noexcept {
    A_FaceTarget(actor);
    S_StartSound(&actor, sfx_manatk);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fires the 1st round of projectiles in the Mancubus's attack sequence
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FatAttack1(mobj_t& actor) noexcept {
    // PC-PSX: avoid undefined behavior if for some reason there is no target
    #if PC_PSX_DOOM_MODS
        if (!actor.target)
            return;
    #endif

    A_FaceTarget(actor);
    mobj_t& target = *actor.target;

    // Spawn the projectiles for this round and adjust the mancubus's aim
    actor.angle += FATSPREAD;
    P_SpawnMissile(actor, target, MT_FATSHOT);

    mobj_t& missile = *P_SpawnMissile(actor, target, MT_FATSHOT);
    missile.angle += FATSPREAD;

    const uint32_t missileFineAngle = missile.angle >> ANGLETOFINESHIFT;
    missile.momx = FixedMul(missile.info->speed, gFineCosine[missileFineAngle]);
    missile.momy = FixedMul(missile.info->speed, gFineSine[missileFineAngle]);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fires the 2nd round of projectiles in the Mancubus's attack sequence
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FatAttack2(mobj_t& actor) noexcept {
    // PC-PSX: avoid undefined behavior if for some reason there is no target
    #if PC_PSX_DOOM_MODS
        if (!actor.target)
            return;
    #endif

    A_FaceTarget(actor);
    mobj_t& target = *actor.target;

    // Spawn the projectiles for this round and adjust the mancubus's aim
    actor.angle -= FATSPREAD;
    P_SpawnMissile(actor, target, MT_FATSHOT);

    mobj_t& missile = *P_SpawnMissile(actor, target, MT_FATSHOT);
    missile.angle -= FATSPREAD * 2;

    const uint32_t missileFineAngle = missile.angle >> ANGLETOFINESHIFT;
    missile.momx = FixedMul(missile.info->speed, gFineCosine[missileFineAngle]);
    missile.momy = FixedMul(missile.info->speed, gFineSine[missileFineAngle]);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fires the 3rd round of projectiles in the Mancubus's attack sequence
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FatAttack3(mobj_t& actor) noexcept {
    // PC-PSX: avoid undefined behavior if for some reason there is no target
    #if PC_PSX_DOOM_MODS
        if (!actor.target)
            return;
    #endif

    A_FaceTarget(actor);
    mobj_t& target = *actor.target;

    // Spawn the projectiles for this round and adjust the mancubus's aim
    {
        mobj_t& missile = *P_SpawnMissile(actor, target, MT_FATSHOT);
        missile.angle -= FATSPREAD / 2;

        const uint32_t missileFineAngle = missile.angle >> ANGLETOFINESHIFT;
        missile.momx = FixedMul(missile.info->speed, gFineCosine[missileFineAngle]);
        missile.momy = FixedMul(missile.info->speed, gFineSine[missileFineAngle]);
    }

    {
        mobj_t& missile = *P_SpawnMissile(actor, target, MT_FATSHOT);
        missile.angle += FATSPREAD / 2;

        const uint32_t missileFineAngle = missile.angle >> ANGLETOFINESHIFT;
        missile.momx = FixedMul(missile.info->speed, gFineCosine[missileFineAngle]);
        missile.momy = FixedMul(missile.info->speed, gFineSine[missileFineAngle]);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for a Lost Soul, sets it flying towards it's target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SkullAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;
    
    // Skull is now flying, play the attack sound and face the target
    actor.flags |= MF_SKULLFLY;
    S_StartSound(&actor, actor.info->attacksound);
    A_FaceTarget(actor);
    
    // Set the xy velocity
    const uint32_t actorFineAngle = actor.angle >> ANGLETOFINESHIFT;
    
    actor.momx = FixedMul(SKULLSPEED, gFineCosine[actorFineAngle]);
    actor.momy = FixedMul(SKULLSPEED, gFineSine[actorFineAngle]);

    // Figure out the z velocity based on the travel time and z delta to the target
    mobj_t& target = *actor.target.get();

    const fixed_t distToTgt = P_AproxDistance(target.x - actor.x, target.y - actor.y);
    const int32_t travelTime = std::max(distToTgt / SKULLSPEED, 1);
    const fixed_t zDelta = target.z + (target.height >> 1) - actor.z;

    actor.momz = zDelta / travelTime;
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
    A_SkullAttack(*vmAddrToPtr<mobj_t>(a0));
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
    A_SkullAttack(*vmAddrToPtr<mobj_t>(a0));
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
    A_Fall(*vmAddrToPtr<mobj_t>(a0));
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
    A_SkullAttack(*vmAddrToPtr<mobj_t>(a0));
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
    A_SkullAttack(*vmAddrToPtr<mobj_t>(a0));
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
    A_SkullAttack(*vmAddrToPtr<mobj_t>(a0));
loc_800188C0:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the death sound for the given actor
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Scream(mobj_t& actor) noexcept {
    sfxenum_t soundId = {};

    switch (actor.info->deathsound) {
        case sfx_None:
            return;

        // Randomize former human death sounds.
        // Bug? Notice here that it never picks 'sfx_podth3': the sound goes unused....
        // I tested out that audio piece and it doesn't sound so good anyway, so maybe the choice was deliberate?
        case sfx_podth1:
        case sfx_podth2:
        case sfx_podth3:
            soundId = (sfxenum_t)(sfx_podth1 + (P_Random() & 1));
            break;

        // Randomize Imp death sounds
        case sfx_bgdth1:
        case sfx_bgdth2:
            soundId = (sfxenum_t)(sfx_bgdth1 + (P_Random() & 1));
            break;

        default:
            soundId = actor.info->deathsound;
            break;
    }

    const bool bPlayFullVol = ((actor.type == MT_SPIDER) || (actor.type == MT_CYBORG));
    mobj_t* const pSoundOrigin = (!bPlayFullVol) ? &actor : nullptr;
    S_StartSound(pSoundOrigin, soundId);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play a sound for the given actor being gibbed
//------------------------------------------------------------------------------------------------------------------------------------------
void A_XScream(mobj_t& actor) noexcept {
    S_StartSound(&actor, sfx_slop);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Plays the actor's pain sound (if it has one)
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Pain(mobj_t& actor) noexcept {
    if (actor.info->painsound != sfx_None) {
        S_StartSound(&actor, actor.info->painsound);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes an actor non solid: called when turning enemies into corpses, so they can be walked over
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Fall(mobj_t& actor) noexcept {
    actor.flags &= ~MF_SOLID;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does splash damage for a missile or explosive barrel exploding
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Explode(mobj_t& actor) noexcept {
    P_RadiusAttack(actor, actor.target.get(), 128);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// See whether it is time to trigger specials that occur when a boss type monster dies.
// If all enemies of the actor's type are dead and the map has a special for this enemy dying, then it will be triggered.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BossDeath(mobj_t& actor) noexcept {
    // Determine which line tag would be triggered based on the active boss specials for the current map, and what died.
    // This tag will be triggered when all enemies of the actor's type die:
    const uint32_t bossSpecialFlags = *gMapBossSpecialFlags;
    const mobjtype_t actorType = actor.type;

    int32_t triggerTag;

    if ((bossSpecialFlags & 0x1) && (actorType == MT_FATSO)) {
        triggerTag = 666;
    }
    else if ((bossSpecialFlags & 0x2) && (actorType == MT_BABY)) {
        triggerTag = 667;
    }
    else if ((bossSpecialFlags & 0x04) && (actorType == MT_SPIDER)) {
        triggerTag = 668;
    }
    else if ((bossSpecialFlags & 0x08) && (actorType == MT_KNIGHT)) {
        triggerTag = 669;
    }
    else if ((bossSpecialFlags & 0x10) && (actorType == MT_CYBORG)) {
        triggerTag = 670;
    }
    else if ((bossSpecialFlags & 0x20) && (actorType == MT_BRUISER)) {
        triggerTag = 671;
    }
    else {
        return;     // No active boss special on this map for this boss type: nothing to trigger!
    }

    // If all map objects of the given actor type are dead then we can trigger the special for the boss death.
    // Otherwise if we find one that is alive, then we can't:
    mobj_t& mobjHead = *gMObjHead;

    for (mobj_t* pmobj = mobjHead.next.get(); pmobj != &mobjHead; pmobj = pmobj->next.get()) {
        if ((pmobj != &actor) && (pmobj->type == actorType) && (pmobj->health > 0))
            return;
    }

    // If we've gotten to here then we've killed all of this boss type and should trigger the appropriate special.
    // Use a dummy line structure (only want the 'tag' field really) to trigger some specials.
    //
    // PC-PSX: default init this structure for good measure, as a precaution against undefined behavior.
    #if PC_PSX_DOOM_MODS
        line_t dummyLine = {};
    #else
        line_t dummyLine;
    #endif

    dummyLine.tag = triggerTag;

    switch (dummyLine.tag) {
        case 666:
            *gMapBossSpecialFlags &= ~0x1;  // Don't attempt to trigger this special again!
            EV_DoFloor(dummyLine, lowerFloorToLowest);
            break;

        case 667:
            *gMapBossSpecialFlags &= ~0x2;
            EV_DoFloor(dummyLine, raiseFloor24);
            break;

        case 668:
            *gMapBossSpecialFlags &= ~0x4;
            EV_DoFloor(dummyLine, lowerFloorToLowest);
            break;

        case 669:
            *gMapBossSpecialFlags &= ~0x8;
            EV_DoFloor(dummyLine, lowerFloorToLowest);
            break;

        case 670:
            *gMapBossSpecialFlags &= ~0x10;
            EV_DoDoor(dummyLine, Open);
            return;

        case 671:
            *gMapBossSpecialFlags &= ~0x20;
            EV_DoFloor(dummyLine, lowerFloorToLowest);
            break;

        default:
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Used by the Cyberdemon; plays the hoof sound while chasing the current target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Hoof(mobj_t& actor) noexcept {
    S_StartSound(&actor, sfx_hoof);
    A_Chase(actor);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Used by the Cyberdemon and Spider Mastermind; plays a mechanized sound while chasing the current target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Metal(mobj_t& actor) noexcept {
    S_StartSound(&actor, sfx_metal);
    A_Chase(actor);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Used by the Arachnotron; plays it's mechanized sound while chasing the current target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BabyMetal(mobj_t& actor) noexcept {
    S_StartSound(&actor, sfx_bspwlk);
    A_Chase(actor);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// A late call action set in p_base: explode a missile after it has hit something and damage what it hit
//------------------------------------------------------------------------------------------------------------------------------------------
void L_MissileHit(mobj_t& missile) noexcept {
    mobj_t* const pHitThing = vmAddrToPtr<mobj_t>(missile.extradata);

    if (pHitThing) {
        const int32_t damage = missile.info->damage * ((P_Random() & 7) + 1);   // 1-8x damage
        mobj_t* const pFirer = missile.target.get();
        P_DamageMObj(*pHitThing, &missile, pFirer, damage);
    }

    P_ExplodeMissile(missile);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// A late call action set in p_base: damage the thing a Lost Soul has bashed into
//------------------------------------------------------------------------------------------------------------------------------------------
void L_SkullBash(mobj_t& actor) noexcept {
    // Damage the the map object which was bashed (if anything)
    mobj_t* const pHitThing = vmAddrToPtr<mobj_t>(actor.extradata);

    if (pHitThing) {
        const int32_t damage = actor.info->damage * ((P_Random() & 7) + 1);     // Info damage x1-x8
        P_DamageMObj(*pHitThing, &actor, &actor, damage);
    }

    // Kill all velocity of the lost soul, stop it flying and put back into the regular active state
    actor.momz = 0;
    actor.momy = 0;
    actor.momx = 0;
    actor.flags &= ~MF_SKULLFLY;
    P_SetMObjState(actor, actor.info->spawnstate);
}

// TODO: remove all these thunks
void _thunk_A_Look() noexcept { A_Look(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_Chase() noexcept { A_Chase(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_FaceTarget() noexcept { A_FaceTarget(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_PosAttack() noexcept { A_PosAttack(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_SPosAttack() noexcept { A_SPosAttack(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_CPosAttack() noexcept { A_CPosAttack(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_CPosRefire() noexcept { A_CPosRefire(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_SpidAttack() noexcept { A_SpidAttack(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_SpidRefire() noexcept { A_SpidRefire(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_BspiAttack() noexcept { A_BspiAttack(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_TroopAttack() noexcept { A_TroopAttack(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_SargAttack() noexcept { A_SargAttack(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_HeadAttack() noexcept { A_HeadAttack(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_CyberAttack() noexcept { A_CyberAttack(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_BruisAttack() noexcept { A_BruisAttack(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_SkelMissile() noexcept { A_SkelMissile(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_Tracer() noexcept { A_Tracer(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_SkelWhoosh() noexcept { A_SkelWhoosh(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_SkelFist() noexcept { A_SkelFist(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_FatRaise() noexcept { A_FatRaise(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_FatAttack1() noexcept { A_FatAttack1(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_FatAttack2() noexcept { A_FatAttack2(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_FatAttack3() noexcept { A_FatAttack3(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_SkullAttack() noexcept { A_SkullAttack(*vmAddrToPtr<mobj_t>(a0)); }

void _thunk_A_Scream() noexcept { A_Scream(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_XScream() noexcept { A_XScream(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_Pain() noexcept { A_Pain(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_Fall() noexcept { A_Fall(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_Explode() noexcept { A_Explode(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_BossDeath() noexcept { A_BossDeath(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_Hoof() noexcept { A_Hoof(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_Metal() noexcept { A_Metal(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_A_BabyMetal() noexcept { A_BabyMetal(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_L_MissileHit() noexcept { L_MissileHit(*vmAddrToPtr<mobj_t>(a0)); }
void _thunk_L_SkullBash() noexcept { L_SkullBash(*vmAddrToPtr<mobj_t>(a0)); }
