#include "p_enemy.h"

#include "Asserts.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "g_game.h"
#include "info.h"
#include "p_doors.h"
#include "p_floor.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_move.h"
#include "p_setup.h"
#include "p_sight.h"
#include "p_spec.h"
#include "p_switch.h"
#include "p_telept.h"
#include "p_tick.h"
#include "PsyDoom/Config.h"
#include "PsyDoom/Game.h"
#include "sprinfo.h"

#include <algorithm>
#include <cmath>

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

// PsyDoom: Arch-vile related globals
#if PSYDOOM_MODS
    // The origin point from which the Arch-vile searches for nearby corpses: x
    static fixed_t gVileTryX;
    static fixed_t gVileTryY;

    // The current corpse found which can be raised by the Arch-vile
    static mobj_t* gpVileCorpse;
#endif

// PsyDoom: Icon Of Sin related globals
#if PSYDOOM_MODS
    static constexpr int32_t MAX_BRAIN_TARGETS = 64;        // Note: this is double the PC limit, should be more than enough...

    static mobj_t*  gpBrainTargets[MAX_BRAIN_TARGETS];      // Target points for the Icon Of Sin spawner
    static int32_t  gNumBrainTargets;                       // How many target points there are for the Icon Of Sin spawner
#endif

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom helper: gathers a list of spawner cube targets for the Icon Of Sin
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_GatherBrainTargets() noexcept {
    gNumBrainTargets = 0;

    for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead; pMobj = pMobj->next) {
        if (gNumBrainTargets >= MAX_BRAIN_TARGETS)
            break;

        if (pMobj->type == MT_BOSSTARGET) {
            gpBrainTargets[gNumBrainTargets] = pMobj;
            gNumBrainTargets++;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom helper: pick one of the spawner cube targets to use for the Icon Of Sin
//------------------------------------------------------------------------------------------------------------------------------------------
static mobj_t* P_PickBrainTarget() noexcept {
    // If there are no brain targets then we can't pick one
    if (gNumBrainTargets <= 0)
        return nullptr;

    // Find the first unused brain target (use 'threshold == 0' to signify this) and return it
    for (int32_t i = 0; i < gNumBrainTargets; ++i) {
        ASSERT(gpBrainTargets[i]);
        mobj_t& target = *gpBrainTargets[i];

        if (target.threshold == 0)
            return &target;
    }

    // If no brain targets are unused reset the 'used' marker for all of them and return the first
    for (int32_t i = 0; i < gNumBrainTargets; ++i) {
        gpBrainTargets[i]->threshold = 0;
    }

    return gpBrainTargets[0];
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// For the given attacker, checks to see if it's target is within melee range and returns 'true' if so
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_CheckMeleeRange(mobj_t& attacker) noexcept {
    // If the attacker can't see it's target then consider it not to be in melee range
    if ((attacker.flags & MF_SEETARGET) == 0)
        return false;

    // If there is no target then obviously nothing is in melee range
    mobj_t* const pTarget = attacker.target;

    if (!pTarget)
        return false;

    // Return whether the target is within melee range
    const fixed_t approxDist = P_AproxDistance(pTarget->x - attacker.x, pTarget->y - attacker.y);
    return (approxDist < MELEERANGE);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Not really a range check, more of a check to see if an attacker should to a missile attack against its target.
// Takes into account things like range and current sight status and also adds an element of randomness.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_CheckMissileRange(mobj_t& attacker) noexcept {
    ASSERT(attacker.target);

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
    mobj_t& target = *attacker.target;
    fixed_t distFrac = P_AproxDistance(attacker.x - target.x, attacker.y - target.y);
    distFrac -= 64 * FRACUNIT;

    // If the attacker has no melee attack then do a missile attack more frequently
    if (attacker.info->meleestate == S_NULL) {
        distFrac -= 128 * FRACUNIT;
    }

    // Convert to integer distance and if you are a skull reduce the missile distance even more
    int32_t dist = d_fixed_to_int(distFrac);

    if (attacker.type == MT_SKULL) {
        dist = d_rshift<1>(dist);
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

        // PsyDoom: snap motion if monster movement is not interpolated
        #if PSYDOOM_MODS
            if (!Config::gbInterpolateMonsters) {
                actor.x.snap();
                actor.y.snap();
            }
        #endif

        // If the actor is not floating, ensure it is grounded on whatever sector it is in (in case it's going down steps)
        if ((actor.flags & MF_FLOAT) == 0) {
            // PsyDoom: keep track of the old z value for snapping purposes
            #if PSYDOOM_MODS
                const fixed_t oldZ = actor.z;
            #endif

            actor.z = actor.floorz;

            // PsyDoom: snap motion if monster movement is not interpolated.
            // Only snap however if there was actual movement, don't want to upset any smooth motion due to platforms and such moving...
            #if PSYDOOM_MODS
                if (!Config::gbInterpolateMonsters) {
                    if (oldZ != actor.z) {
                        actor.z.snap();
                    }
                }
            #endif
        }

        return true;
    }

    // Move failed: if the actor can float then try and go up or down - depending on relative position to the line opening.
    // If that can be done then the move is still considered a success:
    if ((actor.flags & MF_FLOAT) && gbFloatOk) {
        actor.z += (actor.z >= gTmFloorZ) ? -FLOATSPEED : FLOATSPEED;
        actor.flags |= MF_INFLOAT;

        #if PSYDOOM_MODS
            // PsyDoom: snap floating motion if monster interpolation is not enabled
            if (!Config::gbInterpolateMonsters) {
                actor.z.snap();
            }
        #endif

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
    const mobj_t* const pTarget = actor.target;

    if (!pTarget) {
        I_Error("P_NewChaseDir: called with no target");
        return;
    }

    // Save the current movement direction and it's opposite for comparisons
    const dirtype_t oldMoveDir = actor.movedir;
    const dirtype_t turnaroundDir = gOppositeDir[oldMoveDir];

    // Figure out the horizontal and vertical directions to the target and the distances.
    // PsyDoom: if the external camera is active then make monsters walk away from their targets, to give the player time to react after the camera ends.
    #if PSYDOOM_MODS
        const int32_t tgtDistFlip = (gExtCameraTicsLeft > 0) ? -1 : 1;
        const fixed_t tgtDistX = (pTarget->x - actor.x) * tgtDistFlip;
        const fixed_t tgtDistY = (pTarget->y - actor.y) * tgtDistFlip;
    #else
        const fixed_t tgtDistX = pTarget->x - actor.x;
        const fixed_t tgtDistY = pTarget->y - actor.y;
    #endif

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
    P_SetMobjState(actor, actor.info->seestate);
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
            P_SetMobjState(actor, actor.info->spawnstate);
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

        P_SetMobjState(actor, actorInfo.meleestate);
        return;
    }

    // Try to do a missile attack if possible.
    // Note that on nightmare skill we don't have any cooldowns on attacking again!
    const bool bMissileAttackAllowed = ((actor.movecount == 0) || (gGameSkill == sk_nightmare));

    if ((actorInfo.missilestate != S_NULL) && bMissileAttackAllowed) {
        if (P_CheckMissileRange(actor)) {
            P_SetMobjState(actor, actorInfo.missilestate);

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
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
        return;

    // Monster is no longer in ambush mode and turn to face the target
    actor.flags &= ~MF_AMBUSH;
    actor.angle = R_PointToAngle2(actor.x, actor.y, pTarget->x, pTarget->y);

    // If the target has partial invisbility then vary the angle randomly a bit (by almost 45 degrees)
    if (pTarget->flags & MF_ALL_BLEND_FLAGS) {
        actor.angle += P_SubRandom() * (ANG45 / 256);
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

    const angle_t shootAngle = actor.angle + P_SubRandom() * (ANG45 / 512);     // Vary by up to 22.5 degrees (approximately)
    const int32_t damage = ((P_Random() & 7) + 1) * 3;                          // 3-24 damage

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
        const angle_t shootAngle =  actor.angle + P_SubRandom() * (ANG45 / 512);    // Vary by up to 22.5 degrees (approximately)
        const int32_t damage = (P_Random() % 5 + 1) * 3;                            // 3-15 damage

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
    const angle_t shootAngle = actor.angle + P_SubRandom() * (ANG45 / 512);         // Vary by up to 22.5 degrees (approximately)
    const int32_t damage = (P_Random() % 5 + 1) * 3;                                // 3-15 damage

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
        P_SetMobjState(actor, actor.info->seestate);
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
        const angle_t shootAngle = actor.angle + P_SubRandom() * (ANG45 / 512);     // Vary by up to 22.5 degrees (approximately)
        const int32_t damage = (P_Random() % 5 + 1) * 3;                            // 3-15 damage

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
        P_SetMobjState(actor, actor.info->seestate);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for an Arachnotron
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BspiAttack(mobj_t& actor) noexcept {
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
        return;

    A_FaceTarget(actor);
    P_SpawnMissile(actor, *pTarget, MT_ARACHPLAZ);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for an Imp, which can either be a melee attack or sending a fireball towards the target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_TroopAttack(mobj_t& actor) noexcept {
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
        return;

    A_FaceTarget(actor);

    // Do a melee attack if possible, otherwise spawn a fireball
    if (P_CheckMeleeRange(actor)) {
        S_StartSound(&actor, sfx_claw);
        const int32_t damage = ((P_Random() & 7) + 1) * 3;      // 3-24 damage
        P_DamageMobj(*pTarget, &actor, &actor, damage);
    } else {
        P_SpawnMissile(actor, *pTarget, MT_TROOPSHOT);
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
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
        return;

    A_FaceTarget(actor);

    // Do a melee attack if possible, otherwise spawn a fireball
    if (P_CheckMeleeRange(actor)) {
        const int32_t damage = ((P_Random() & 7) + 1) * 8;      // 8-64 damage
        P_DamageMobj(*pTarget, &actor, &actor, damage);
    } else {
        P_SpawnMissile(actor, *pTarget, MT_HEADSHOT);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for a Cyberdemon
//------------------------------------------------------------------------------------------------------------------------------------------
void A_CyberAttack(mobj_t& actor) noexcept {
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
        return;

    A_FaceTarget(actor);
    P_SpawnMissile(actor, *pTarget, MT_ROCKET);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the attack for a Baron or Hell Knight, which can either be a melee attack or sending a fireball towards the target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BruisAttack(mobj_t& actor) noexcept {
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
        return;

    if (P_CheckMeleeRange(actor)) {
        S_StartSound(&actor, sfx_claw);
        const int32_t damage = ((P_Random() & 7) + 1) * 11;     // 11-88 damage
        P_DamageMobj(*pTarget, &actor, &actor, damage);
    } else {
        P_SpawnMissile(actor, *pTarget, MT_BRUISERSHOT);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Make a Revenant fire it's missile towards a player
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SkelMissile(mobj_t& actor) noexcept {
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
        return;

    A_FaceTarget(actor);

    // Spawn the missile: also hack adjust the Revenant height slightly (temporarily) so the missile spawns higher
    actor.z += 16 * FRACUNIT;
    mobj_t& missile = *P_SpawnMissile(actor, *pTarget, MT_TRACER);
    actor.z -= 16 * FRACUNIT;

    // Move the missile a little and set it's target
    missile.x += missile.momx;
    missile.y += missile.momy;
    missile.tracer = pTarget;

    #if PSYDOOM_MODS
        R_SnapMobjInterpolation(missile);   // PsyDoom: snap the motion we just added since the missile is just spawning
    #endif
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
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
        return;

    A_FaceTarget(actor);

    if (P_CheckMeleeRange(actor)) {
        S_StartSound(&actor, sfx_skepch);
        const int32_t damage = (P_Random() % 10 + 1) * 6;       // 6-60 damage
        P_DamageMobj(*pTarget, &actor, &actor, damage);
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
    // PsyDoom: avoid undefined behavior if for some reason there is no target
    mobj_t* const pTarget = actor.target;

    #if PSYDOOM_MODS && PSYDOOM_FIX_UB
        if (!pTarget)
            return;
    #else
        ASSERT(pTarget);
    #endif

    A_FaceTarget(actor);

    // Spawn the projectiles for this round and adjust the mancubus's aim
    actor.angle += FATSPREAD;
    P_SpawnMissile(actor, *pTarget, MT_FATSHOT);

    mobj_t& missile = *P_SpawnMissile(actor, *pTarget, MT_FATSHOT);
    missile.angle += FATSPREAD;

    const uint32_t missileFineAngle = missile.angle >> ANGLETOFINESHIFT;
    missile.momx = FixedMul(missile.info->speed, gFineCosine[missileFineAngle]);
    missile.momy = FixedMul(missile.info->speed, gFineSine[missileFineAngle]);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fires the 2nd round of projectiles in the Mancubus's attack sequence
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FatAttack2(mobj_t& actor) noexcept {
    // PsyDoom: avoid undefined behavior if for some reason there is no target
    mobj_t* const pTarget = actor.target;

    #if PSYDOOM_MODS && PSYDOOM_FIX_UB
        if (!pTarget)
            return;
    #else
        ASSERT(pTarget);
    #endif

    A_FaceTarget(actor);

    // Spawn the projectiles for this round and adjust the mancubus's aim
    actor.angle -= FATSPREAD;
    P_SpawnMissile(actor, *pTarget, MT_FATSHOT);

    mobj_t& missile = *P_SpawnMissile(actor, *pTarget, MT_FATSHOT);
    missile.angle -= FATSPREAD * 2;

    const uint32_t missileFineAngle = missile.angle >> ANGLETOFINESHIFT;
    missile.momx = FixedMul(missile.info->speed, gFineCosine[missileFineAngle]);
    missile.momy = FixedMul(missile.info->speed, gFineSine[missileFineAngle]);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Fires the 3rd round of projectiles in the Mancubus's attack sequence
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FatAttack3(mobj_t& actor) noexcept {
    // PsyDoom: avoid undefined behavior if for some reason there is no target
    mobj_t* const pTarget = actor.target;

    #if PSYDOOM_MODS && PSYDOOM_FIX_UB
        if (!pTarget)
            return;
    #else
        ASSERT(pTarget);
    #endif

    A_FaceTarget(actor);

    // Spawn the projectiles for this round and adjust the mancubus's aim
    {
        mobj_t& missile = *P_SpawnMissile(actor, *pTarget, MT_FATSHOT);
        missile.angle -= FATSPREAD / 2;

        const uint32_t missileFineAngle = missile.angle >> ANGLETOFINESHIFT;
        missile.momx = FixedMul(missile.info->speed, gFineCosine[missileFineAngle]);
        missile.momy = FixedMul(missile.info->speed, gFineSine[missileFineAngle]);
    }

    {
        mobj_t& missile = *P_SpawnMissile(actor, *pTarget, MT_FATSHOT);
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
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
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
    const fixed_t distToTgt = P_AproxDistance(pTarget->x - actor.x, pTarget->y - actor.y);
    const int32_t travelTime = std::max(distToTgt / SKULLSPEED, 1);
    const fixed_t zDelta = pTarget->z + d_rshift<1>(pTarget->height) - actor.z;

    actor.momz = zDelta / travelTime;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawn a skull (Lost Soul) from the given Pain Elemental actor at the given angle away from the parent.
// If the skull is spawned in a wall, then it is immediately killed.
//------------------------------------------------------------------------------------------------------------------------------------------
static void A_PainShootSkull(mobj_t& actor, const angle_t angle) noexcept {
    // PsyDoom: disabling this logic as it was broken in the original PSX DOOM and NEVER limits the amount of skulls in a map.
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
    #if !PSYDOOM_MODS
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
    // Note: I've made the skull limit configurable also, to support the original behavior of both games.
    const int32_t skullLimit = Game::gSettings.lostSoulSpawnLimit;

    if (skullLimit > 0) {
        int32_t numSkulls = 0;

        for (mobj_t* pmobj = gMobjHead.next; pmobj != &gMobjHead; pmobj = pmobj->next) {
            if (pmobj->type == MT_SKULL) {
                numSkulls++;

                if (numSkulls > skullLimit)
                    return;
            }
        }
    }

    // Figure out where to spawn the skull
    const fixed_t spawnDist = gMobjInfo[MT_SKULL].radius + actor.info->radius + 4 * FRACUNIT;

    const fixed_t spawnX = actor.x + FixedMul(spawnDist, gFineCosine[angle >> ANGLETOFINESHIFT]);
    const fixed_t spawnY = actor.y + FixedMul(spawnDist, gFineSine[angle >> ANGLETOFINESHIFT]);
    const fixed_t spawnZ = actor.z + 8 * FRACUNIT;

    // Spawn the skull and if it can't move (is already stuck in a wall) then kill it immediately
    mobj_t& skull = *P_SpawnMobj(spawnX, spawnY, spawnZ, MT_SKULL);
    const bool bSpawnedInWall = (!P_TryMove(skull, skull.x, skull.y));

    if (bSpawnedInWall) {
        P_DamageMobj(skull, &actor, &actor, 10000);
        return;
    }

    // Bug fix for PsyDoom: the above method of skull spawning and collision testing will sometimes result in skulls being spawned outside the level.
    // This is because the new skull may have been pushed beyond the point where a collision test against walls would register anything.
    // The fix for this is to do a raycast from the skull position to the Pain Elemental and see if that is blocked by anything.
    // We must also apply this fix conditionally, since it can break demo compatibility with the original game.
    if (Game::gSettings.bUseLostSoulSpawnFix) {
        const bool bSpawnedPastWall = (!P_CheckSight(actor, skull));

        if (bSpawnedPastWall) {
            P_DamageMobj(skull, &actor, &actor, 10000);
            return;
        }
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
// PsyDoom: adding a special action for when all Commander Keens die (re-implemented actor type from PC)
#if PSYDOOM_MODS
    else if ((bossSpecialFlags & 0x40) && (actorType == MT_KEEN)) {
        triggerTag = 672;
    }
#endif
    else {
        return;     // No active boss special on this map for this boss type: nothing to trigger!
    }

    // If all map objects of the given actor type are dead then we can trigger the special for the boss death.
    // Otherwise if we find one that is alive, then we can't:
    for (mobj_t* pmobj = gMobjHead.next; pmobj != &gMobjHead; pmobj = pmobj->next) {
        if ((pmobj != &actor) && (pmobj->type == actorType) && (pmobj->health > 0))
            return;
    }

    // If we've gotten to here then we've killed all of this boss type and should trigger the appropriate special.
    // Use a dummy line structure (only want the 'tag' field really) to trigger some specials.
    //
    // PsyDoom: default init this structure for good measure, as a precaution against undefined behavior.
    #if PSYDOOM_MODS && PSYDOOM_FIX_UB
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

        // PsyDoom: adding a special action for when all Commander Keens die (re-implemented actor type from PC)
        #if PSYDOOM_MODS
            case 672:
                gMapBossSpecialFlags &= ~0x40;
                EV_DoDoor(dummyLine, Open);
                break;
        #endif

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
        P_DamageMobj(*pHitThing, &missile, pFirer, damage);
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
        P_DamageMobj(*pHitThing, &actor, &actor, damage);
    }

    // Kill all velocity of the lost soul, stop it flying and put back into the regular active state
    actor.momz = 0;
    actor.momy = 0;
    actor.momx = 0;
    actor.flags &= ~MF_SKULLFLY;
    P_SetMobjState(actor, actor.info->spawnstate);
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Arch-vile blockmap iterator function: checks to see if the specified thing can be raised/resurrected.
// If it can be raised 'false' will be returned and the thing will be remembered as a global.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PIT_VileCheck(mobj_t& thing) noexcept {
    // Must be a dead monster
    if ((thing.flags & MF_CORPSE) == 0)
        return true;

    // Must be completely still
    if (thing.tics != -1)
        return true;

    // Must be raisable (have a raise state defined)
    if (thing.info->raisestate == S_NULL)
        return true;

    // If the thing is too far away then don't raise
    const fixed_t maxDist = thing.info->radius + gMobjInfo[MT_VILE].radius;
    const fixed_t xDist = std::abs(thing.x - gVileTryX);
    const fixed_t yDist = std::abs(thing.y - gVileTryY);

    if ((xDist > maxDist) || (yDist > maxDist))
        return true;

    // If the thing was crushed (symptoms: has no radius or height) then don't raise.
    // This prevents an original Doom bug where 'ghost monsters' are created.
    if ((thing.radius <= 0) || (thing.height <= 0))
        return true;

    // Kill any momentum this corpse has and remember it as a potential resurrect corpse
    gpVileCorpse = &thing;
    thing.momx = 0;
    thing.momy = 0;

    // Check to make sure the enemy fits here if resurrected.
    // Need to expand it's height x4 also, since it is reduced that much on death, and will be expanded that much again on resurrection.
    thing.height = d_lshift<4>(thing.height);
    const bool bCorpseFits = P_CheckPosition(thing, thing.x, thing.y);
    thing.height = d_rshift<4>(thing.height);

    // Continue the search if the corpse doesn't fit, otherwise stop it
    return (!bCorpseFits);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Arch-vile movement and resurrect search code.
// Moves the Arch-vile around and searches for nearby corpses that can be resurrected.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_VileChase(mobj_t& actor) noexcept {
    // If moving around check nearby blockmap cells for dead monsters to raise
    if (actor.movedir != DI_NODIR) {
        // Set the origin point from which the Arch-vile starts searching for resurrect targets
        gVileTryX = actor.x + actor.info->speed * gMoveXSpeed[actor.movedir];
        gVileTryY = actor.y + actor.info->speed * gMoveYSpeed[actor.movedir];

        // Determine the blockmap extents (left/right, top/bottom) to be checked (for corpses) and clamp to a valid range
        const int32_t bmapLx = std::max(d_rshift<MAPBLOCKSHIFT>(gVileTryX - gBlockmapOriginX - MAXRADIUS * 2), 0);
        const int32_t bmapRx = std::min(d_rshift<MAPBLOCKSHIFT>(gVileTryX - gBlockmapOriginX + MAXRADIUS * 2), gBlockmapWidth - 1);
        const int32_t bmapTy = std::min(d_rshift<MAPBLOCKSHIFT>(gVileTryY - gBlockmapOriginY + MAXRADIUS * 2), gBlockmapHeight - 1);
        const int32_t bmapBy = std::max(d_rshift<MAPBLOCKSHIFT>(gVileTryY - gBlockmapOriginY - MAXRADIUS * 2), 0);

        // Search nearby blockmap cells for raisable corpses
        for (int32_t blockX = bmapLx; blockX <= bmapRx; ++blockX) {
            for (int32_t blockY = bmapBy; blockY <= bmapTy; ++blockY) {
                // Scan all things in this blockmap cell to see if any can be raised
                const bool bCanRaise = (!P_BlockThingsIterator(blockX, blockY, PIT_VileCheck));

                if (!bCanRaise)
                    continue;

                // Have a corpse that can be resurrected, make the Arch-Vile face that corpse
                ASSERT(gpVileCorpse);
                mobj_t& raiseMobj = *gpVileCorpse;

                {
                    mobj_t* const pVileTarget = actor.target;
                    actor.target = &raiseMobj;
                    A_FaceTarget(actor);
                    actor.target = pVileTarget;
                }

                // Arch-vile goes into the healing state and plays the slop sound for resurrection (at the corpse location)
                P_SetMobjState(actor, S_VILE_HEAL1);
                S_StartSound(&raiseMobj, sfx_slop);

                // Put the thing being raised into the raising state, make taller again, restore health, flags and clear target.
                // Note: PSX Doom blending flags need to be preserved however!
                mobjinfo_t& raiseObjInfo = *raiseMobj.info;
                P_SetMobjState(raiseMobj, raiseObjInfo.raisestate);
                const uint32_t flagsToPreserve = raiseMobj.flags & MF_ALL_BLEND_FLAGS;

                raiseMobj.height = d_lshift<2>(raiseMobj.height);
                raiseMobj.flags = raiseObjInfo.flags | flagsToPreserve;
                raiseMobj.health = raiseObjInfo.spawnhealth;
                raiseMobj.target = nullptr;

                // Done! Don't do any chasing while raising...
                return;
            }
        }
    }

    // Do normal chase logic if not raising
    A_Chase(actor);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Plays the attack start sound for the Arch-vile
//------------------------------------------------------------------------------------------------------------------------------------------
void A_VileStart(mobj_t& actor) noexcept {
    S_StartSound(&actor, sfx_vilatk);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawns Arch-vile's fire and makes it face the target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_VileTarget(mobj_t& actor) noexcept {
    // Don't do anything if there is no target, otherwise face it and spawn the fire
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
        return;

    A_FaceTarget(actor);
    mobj_t& fire = *P_SpawnMobj(pTarget->x, pTarget->x, pTarget->z, MT_FIRE);

    // Make the Arch-vile remember the fire, and the fire remember the Arch-vile and it's target
    actor.tracer = &fire;
    fire.target = &actor;
    fire.tracer = pTarget;

    // Move the fire to follow the Arch-vile's target
    A_Fire(fire);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the line of sight attack for the Arch-vile
//------------------------------------------------------------------------------------------------------------------------------------------
void A_VileAttack(mobj_t& actor) noexcept {
    // Don't do anything if there is no target, otherwise face it
    mobj_t* const pTarget = actor.target;

    if (!pTarget)
        return;

    A_FaceTarget(actor);

    // If there is no line of sight now to the target then don't do the attack
    if (!P_CheckSight(actor, *pTarget))
        return;

    // Play the attack sound, damage the target and make the target hop upwards
    S_StartSound(&actor, sfx_barexp);
    P_DamageMobj(*pTarget, &actor, &actor, 20);
    pTarget->momz = (1000 * FRACUNIT) / pTarget->info->mass;
    
    // Don't do splash damage unless there is fire
    mobj_t* const pFire = actor.tracer;

    if (!pFire)
        return;

    // Make sure the fire is in front of the target
    const uint32_t angleIdx = actor.angle >> ANGLETOFINESHIFT;
    pFire->x = pTarget->x - FixedMul(24 * FRACUNIT, gFineCosine[angleIdx]);
    pFire->y = pTarget->y - FixedMul(24 * FRACUNIT, gFineSine[angleIdx]);

    // Need to snap the flame motion since it's basically teleports around the map
    R_SnapMobjInterpolation(*pFire);

    // Do the splash damage at the fire location
    P_RadiusAttack(*pFire, &actor, 70);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Moves the Arch-vile's fire to be in front of the target, if the target is in sight of the Arch-vile
//------------------------------------------------------------------------------------------------------------------------------------------
void A_Fire(mobj_t& actor) noexcept {
    // Don't do anything if the fire doesn't have a target inherited from the Arch-vile
    mobj_t* const pVileTgt = actor.tracer;

    if (!pVileTgt)
        return;

    // Don't move the fire if there is no line of sight between the arch vile and it's target
    mobj_t* const pVileMobj = actor.target;
    const bool bVileHasLineOfSight = (pVileMobj && P_CheckSight(*pVileMobj, *pVileTgt));

    if (!bVileHasLineOfSight)
        return;

    // Place the fire in front of the Arch-vile's target
    const uint32_t angleIdx = pVileTgt->angle >> ANGLETOFINESHIFT;

    P_UnsetThingPosition(actor);
    actor.x = pVileTgt->x + FixedMul(24 * FRACUNIT, gFineCosine[angleIdx]);
    actor.y = pVileTgt->y + FixedMul(24 * FRACUNIT, gFineSine[angleIdx]);
    actor.z = pVileTgt->z;
    P_SetThingPosition(actor);

    // Need to snap the flame motion since it's basically teleports around the map
    R_SnapMobjInterpolation(actor);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the start fire sound for the Arch-vile's flame and position in front of the target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_StartFire(mobj_t& actor) noexcept {
    S_StartSound(&actor, sfx_flamst);
    A_Fire(actor);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the periodic fire sound for the Arch-vile's flame and position in front of the target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_FireCrackle(mobj_t& actor) noexcept {
    S_StartSound(&actor, sfx_flame);
    A_Fire(actor);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called when Commander Keen dies.
// Opens a door for sectors tagged '672' (note: originally '666' on PC) if all instances of Commander Keen are dead.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_KeenDie(mobj_t& actor) noexcept {
    // Can now walk over the Keen's corpse and trigger the special action (Open Door, sector tag '672') if all Keens are dead
    A_Fall(actor);
    A_BossDeath(actor);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called when the Icon Of Sin awakes
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BrainAwake([[maybe_unused]] mobj_t& actor) noexcept {
    // Play the sight sound. Note that originally (on PC) this function also gathered all of the target points for the spawner
    // but we now do that dynamically to avoid save/serialization issues.
    S_StartSound(nullptr, sfx_bossit);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Plays the pain sound for the Icon Of Sin
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BrainPain([[maybe_unused]] mobj_t& actor) noexcept {
    S_StartSound(nullptr, sfx_bospn);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called at the start of the Icon Of Sin's death sequence.
// Starts spawning a bunch of different explosions and plays the Icon Of Sin's death sound.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BrainScream(mobj_t& actor) noexcept {
    // Spawn a bunch of explosions
    for (fixed_t x = actor.x - 196 * FRACUNIT; x < actor.x + 320 * FRACUNIT; x += FRACUNIT * 8) {
        // Decide where to explode
        const fixed_t y = actor.y - 320 * FRACUNIT;
        const fixed_t z = 128 + P_Random() * 2 * FRACUNIT;

        // Spawn the rocket, give it upward momentum and put it into the special boss explosion state
        mobj_t& explosion = *P_SpawnMobj(x, y, z, MT_ROCKET);
        explosion.momz = P_Random() * 512;
        P_SetMobjState(explosion, S_BRAINEXPLODE1);

        // Randomize duration
        explosion.tics -= P_Random() & 7;
        explosion.tics = std::max(explosion.tics, 1);
    }

    // Play the Icon Of Sin death sound
    S_StartSound(nullptr, sfx_bosdth);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called when one of the explosions in the Icon Of Sin's death sequence is finished.
// Spawns another explosion.
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BrainExplode(mobj_t& actor) noexcept {
    // Decide where to place the new explosion
    const fixed_t x = actor.x + P_SubRandom() * 2048;
    const fixed_t y = actor.y;
    const fixed_t z = 128 + P_Random() * 2 * FRACUNIT;

    // Spawn the rocket, give it upward momentum and put it into the special boss explosion state
    mobj_t& explosion = *P_SpawnMobj(x, y, z, MT_ROCKET);
    explosion.momz = P_Random() * 512;
    P_SetMobjState(explosion, S_BRAINEXPLODE1);

    // Randomize duration
    explosion.tics -= P_Random() & 7;
    explosion.tics = std::max(explosion.tics, 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Invoked at the end of the Icon Of Sin's death sequence; ends the level
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BrainDie([[maybe_unused]] mobj_t& actor) noexcept {
    G_ExitLevel();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spits out a spawner cube for the Icon Of Sin
//------------------------------------------------------------------------------------------------------------------------------------------
void A_BrainSpit(mobj_t& actor) noexcept {
    // On easy mode or under allow a 50% chance for a spawn not to happen.
    // Note: originally (on PC) a static local was used to achieve this with a simple bit flip, but that has serialization/demo issues.
    if (gGameSkill <= sk_easy) {
        if (P_Random() & 1)
            return;
    }

    // Update the list of spawner targets and pick one
    P_GatherBrainTargets();
    mobj_t* const pTarget = P_PickBrainTarget();

    if (!pTarget)
        return;

    // Don't use this spawner again until we've exhausted all the other ones
    pTarget->threshold = 1;

    // Spawn the spawner cube and send it towards the target
    mobj_t& spawnCube = *P_SpawnMissile(actor, *pTarget, MT_SPAWNSHOT);
    spawnCube.target = pTarget;

    // Spawn in this many tics
    {
        const fixed_t travelFracTics = FixedDiv(pTarget->y - actor.y, spawnCube.momy * spawnCube.state->tics);
        const int32_t travelTics = d_fixed_to_int(travelFracTics + FRACUNIT - 1);
        spawnCube.reactiontime = travelTics;
    }

    // Play the spawn sound
    S_StartSound(NULL, sfx_bospit);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Plays the Icon Of Sin spawner cube spawn sound and flies it towards the target
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SpawnSound(mobj_t& actor) noexcept {
    S_StartSound(&actor, sfx_boscub);
    A_SpawnFly(actor);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flies the Icon Of Sin spawn cube towards it's target and spawns an enemy upon reaching it
//------------------------------------------------------------------------------------------------------------------------------------------
void A_SpawnFly(mobj_t& actor) noexcept {
    // Is the cube still flying?
    if (--actor.reactiontime > 0)
        return;

    // Note: the target should always exist, but add a safety check here just in case it doesn't...
    mobj_t* const pTarget = actor.target;

    if (pTarget) {
        // Do the spawn fire effect (re-uses Arch-vile fire).
        mobj_t& fireFx = *P_SpawnMobj(pTarget->x, pTarget->y, pTarget->z, MT_SPAWNFIRE);
        S_StartSound(&fireFx, sfx_telept);

        // Randomly decide which enemy to spawn.
        // Lower level enemies are generally weighted higher in terms of probability.
        const int32_t randNum = P_Random();
        mobjtype_t spawnType;

        if (randNum < 50) {
            spawnType = MT_TROOP;
        } else if (randNum < 120) {
            spawnType = MT_SERGEANT;
        } else if (randNum < 130) {
            spawnType = MT_PAIN;
        } else if (randNum < 160) {
            spawnType = MT_HEAD;
        } else if ((randNum < 162) && gbHaveSprites_ArchVile) {     // Pass over the Arch-vile if we haven't got the sprites available...
            spawnType = MT_VILE;
        } else if (randNum < 172) {
            spawnType = MT_UNDEAD;
        } else if (randNum < 192) {
            spawnType = MT_BABY;
        } else if (randNum < 222) {
            spawnType = MT_FATSO;
        } else if (randNum < 246) {
            spawnType = MT_KNIGHT;
        } else {
            spawnType = MT_BRUISER;
        }

        // Spawn the enemy and alert it immediately
        mobj_t& spawned = *P_SpawnMobj(pTarget->x, pTarget->y, pTarget->z, spawnType);

        if (P_LookForPlayers(spawned, true)) {
            P_SetMobjState(spawned, spawned.info->seestate);
        }

        // Telefrag anything where the enemy spawned and remove the cube.
        // Note: do not allow self-telefragging!
        P_Telefrag(spawned, spawned.x, spawned.y, false);
    }

    // Remove the spawner cube
    P_RemoveMobj(actor);
}
#endif  // #if PSYDOOM_MODS
