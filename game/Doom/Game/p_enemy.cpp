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
#include "PcPsx/Game.h"

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
    if ((actor.flags & MF_FLOAT) && gbFloatOk) {
        actor.z += (actor.z >= gTmFloorZ) ? -FLOATSPEED : FLOATSPEED;
        actor.flags |= MF_INFLOAT;
        return true;
    }

    // Try to see if we can open a door to unblock movement.
    // If that is possible to do then consider the move a success:
    line_t* const pBlockLine = gpBlockLine;

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
    mobj_t* const pTarget = actor.target;

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
        mobj_t* const pNoiseMaker = actor.subsector->sector->soundtarget;

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
    mobj_t* const pTarget = actor.target;

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
    const bool bMissileAttackAllowed = ((actor.movecount == 0) || (gGameSkill == sk_nightmare));

    if ((actorInfo.missilestate != S_NULL) && bMissileAttackAllowed) {
        if (P_CheckMissileRange(actor)) {
            P_SetMObjState(actor, actorInfo.missilestate);

            // Don't attack again for a bit unless on nightmare
            if (gGameSkill != sk_nightmare) {
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
    mobj_t* const pTarget = actor.target;

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
    mobj_t* const pTarget = actor.target;

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
    if ((gGameTic & 3) != 0)
        return;

    // Spawn a puff and some smoke behind the rocket
    P_SpawnPuff(actor.x, actor.y, actor.z);

    mobj_t& smoke = *P_SpawnMobj(actor.x - actor.momx, actor.y - actor.momy, actor.z, MT_SMOKE);
    smoke.momz = FRACUNIT;
    smoke.tics = std::max(smoke.tics - (P_Random() & 3), 1);

    // Only follow the target and change course if it's still alive
    mobj_t* const pTarget = actor.tracer;

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
    mobj_t& target = *actor.target;

    const fixed_t distToTgt = P_AproxDistance(target.x - actor.x, target.y - actor.y);
    const int32_t travelTime = std::max(distToTgt / SKULLSPEED, 1);
    const fixed_t zDelta = target.z + (target.height >> 1) - actor.z;

    actor.momz = zDelta / travelTime;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a skull (Lost Soul) from the given Pain Elemental actor at the given angle away from the parent.
// If the skull is spawned in a wall, then it is immediately killed.
//------------------------------------------------------------------------------------------------------------------------------------------
static void A_PainShootSkull(mobj_t& actor, const angle_t angle) noexcept {
    // PC-PSX: disabling this logic as it was broken in the original PSX DOOM and NEVER limits the amount of skulls in a map.
    // It effectively did nothing except consuming CPU cycles. Note also that Final Doom fixes this issue and limits the skull
    // count to '16', instead of the original limit of '24' on PC.
    //
    // In PC Doom 'mobj_t' used to be a thinker but in Jaguar Doom changes were made so that 'mobj_t' is no longer a thinker
    // or found in the global list of thinkers. In PSX Doom no thinker will ever have 'P_MobjThinker' assigned to it as the
    // 'think' function, hence this logic will never find any skulls to include in the count.
    //
    // I'm guessing this bug arose after this function was imported from Doom 2 on PC, since it would have been missing from
    // the Jaguar port - which only included Doom 1. Perhaps there was an oversight and the logic was not updated correctly
    // to reflect the new layout of 'mobj_t'? Because the code silently fails and doesn't seem to cause any visible problems
    // other than many Lost Souls (if you let it) I can see why this might have been missed...
    //
    // The correct fix for PSX Doom here would have been to search through the global list of things instead, and count the
    // skulls found in that list. We can't fix the issue however for PSX Doom without breaking demo compatibility, so I'm
    // just going to disable the code instead since it is somewhat ill formed (with strange casts), and does nothing.
    //
    #if !PC_PSX_DOOM_MODS
        // Count the number of skulls active in the level: if there are 21 or more then don't spawn any additional ones
        int32_t numActiveSkulls = 0;

        for (thinker_t* pThinker = gThinkerCap.next; pThinker != &gThinkerCap; pThinker = pThinker->next) {
            if (pThinker->function != P_MobjThinker)
                continue;
            
            if (((mobj_t*) pThinker)->type == MT_SKULL) {
                ++numActiveSkulls;
            }
        }

        if (numActiveSkulls >= 21)
            return;
    #endif

    // Corrected logic from Final Doom for limiting the skull count.
    // TODO: make the skull limit configurable.
    const int32_t skullLimit = (Game::isFinalDoom()) ? 16 : -1;

    if (skullLimit > 0) {
        int32_t numSkulls = 0;

        for (mobj_t* pmobj = gMObjHead.next; pmobj != &gMObjHead; pmobj = pmobj->next) {
            if (pmobj->type == MT_SKULL) {
                numSkulls++;

                if (numSkulls > skullLimit)
                    return;
            }
        }
    }

    // Figure out where to spawn the skull
    const fixed_t spawnDist = gMObjInfo[MT_SKULL].radius + actor.info->radius + 4 * FRACUNIT;

    const fixed_t spawnX = actor.x + FixedMul(spawnDist, gFineCosine[angle >> ANGLETOFINESHIFT]);
    const fixed_t spawnY = actor.y + FixedMul(spawnDist, gFineSine[angle >> ANGLETOFINESHIFT]);
    const fixed_t spawnZ = actor.z + 8 * FRACUNIT;

    // Spawn the skull and if it can't move (is already stuck in a wall) then kill it immediately.
    // 
    // BUG: note that this method of skull spawning and collision testing will sometimes result in skulls being spawned outside of the level.
    // This is because it may have been pushed beyond a point where a collision test against a wall would fail.
    // One fix might be to do a raycast from the skull position to the Pain Elemental and see if that is blocked by anything.
    // I can't do that fix however without breaking demo compatibility, so I'll just leave the bug alone for authentic behavior...
    mobj_t& skull = *P_SpawnMobj(spawnX, spawnY, spawnZ, MT_SKULL);

    if (!P_TryMove(skull, skull.x, skull.y)) {
        P_DamageMObj(skull, &actor, &actor, 10000);
        return;
    }
    
    // Otherwise make the skull adopt the parent Pain Elemental's target player and begin it's attack rush
    skull.target = actor.target;
    A_SkullAttack(skull);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for a Pain Elemental and shoots out a Lost Soul
//------------------------------------------------------------------------------------------------------------------------------------------
void A_PainAttack(mobj_t& actor) noexcept {
    if (!actor.target)
        return;

    A_FaceTarget(actor);
    A_PainShootSkull(actor, actor.angle);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the death event for a Pain Elemental: marks it as non blocking and shoots out 3 Lost Souls
//------------------------------------------------------------------------------------------------------------------------------------------
void A_PainDie(mobj_t& actor) noexcept {
    A_Fall(actor);
    A_PainShootSkull(actor, actor.angle + ANG90);
    A_PainShootSkull(actor, actor.angle + ANG180);
    A_PainShootSkull(actor, actor.angle + ANG270);
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
    P_RadiusAttack(actor, actor.target, 128);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// See whether it is time to trigger specials that occur when a boss type monster dies.
// If all enemies of the actor's type are dead and the map has a special for this enemy dying, then it will be triggered.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BossDeath(mobj_t& actor) noexcept {
    // Determine which line tag would be triggered based on the active boss specials for the current map, and what died.
    // This tag will be triggered when all enemies of the actor's type die:
    const uint32_t bossSpecialFlags = gMapBossSpecialFlags;
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
    for (mobj_t* pmobj = gMObjHead.next; pmobj != &gMObjHead; pmobj = pmobj->next) {
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
            gMapBossSpecialFlags &= ~0x1;   // Don't attempt to trigger this special again!
            EV_DoFloor(dummyLine, lowerFloorToLowest);
            break;

        case 667:
            gMapBossSpecialFlags &= ~0x2;
            EV_DoFloor(dummyLine, raiseFloor24);
            break;

        case 668:
            gMapBossSpecialFlags &= ~0x4;
            EV_DoFloor(dummyLine, lowerFloorToLowest);
            break;

        case 669:
            gMapBossSpecialFlags &= ~0x8;
            EV_DoFloor(dummyLine, lowerFloorToLowest);
            break;

        case 670:
            gMapBossSpecialFlags &= ~0x10;
            EV_DoDoor(dummyLine, Open);
            return;

        case 671:
            gMapBossSpecialFlags &= ~0x20;
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
    mobj_t* const pHitThing = (mobj_t*) missile.extradata;

    if (pHitThing) {
        const int32_t damage = missile.info->damage * ((P_Random() & 7) + 1);   // 1-8x damage
        mobj_t* const pFirer = missile.target;
        P_DamageMObj(*pHitThing, &missile, pFirer, damage);
    }

    P_ExplodeMissile(missile);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// A late call action set in p_base: damage the thing a Lost Soul has bashed into
//------------------------------------------------------------------------------------------------------------------------------------------
void L_SkullBash(mobj_t& actor) noexcept {
    // Damage the the map object which was bashed (if anything)
    mobj_t* const pHitThing = (mobj_t*) actor.extradata;

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
