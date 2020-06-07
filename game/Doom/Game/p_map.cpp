#include "p_map.h"

#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_move.h"
#include "p_setup.h"
#include "p_shoot.h"
#include "p_sight.h"
#include "p_spec.h"
#include "p_switch.h"

#include <algorithm>

mobj_t*     gpShooter;          // The map object currently taking a shot
fixed_t     gAttackRange;       // Maximum attack range for an attacker
angle_t     gAttackAngle;       // Angle of attack for an attacker
fixed_t     gAimTopSlope;       // Maximum Z slope for shooting (defines Z range that stuff can be hit within)
fixed_t     gAimBottomSlope;    // Minimum Z slope for shooting (defines Z range that stuff can be hit within)
mobj_t*     gpLineTarget;       // The thing being shot at in 'P_AimLineAttack' and 'P_LineAttack'.
mobj_t*     gpTryMoveThing;     // Try move: the thing being moved
fixed_t     gTryMoveX;          // Try move: position we're attempting to move to (X)
fixed_t     gTryMoveY;          // Try move: position we're attempting to move to (Y)
bool        gbCheckPosOnly;     // Try move: if 'true' then check if the position is valid to move to only, don't actually move there

static mobj_t*      gpBombSource;       // Radius attacks: the thing responsible for the explosion (player, monster)
static mobj_t*      gpBombSpot;         // Radius attacks: the object exploding and it's position (barrel, missile etc.)
static int32_t      gBombDamage;        // Radius attacks: how much damage the explosion does before falloff
static divline_t    gUseLine;           // The 'use' line being cast from the player towards walls; we try to activate walls that it hits
static fixed_t      gUseBBox[4];        // The bounding box for the 'use' line being cast from the player
static line_t*      gpCloseLine;        // The closest wall line currently being used
static fixed_t      gCloseDist;         // Fractional distance along the use line to the closest wall line being used

//------------------------------------------------------------------------------------------------------------------------------------------
// Test if the given x/y position can be moved to for the given map object and return 'true' if the move is allowed
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_CheckPosition(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept {
    // Save inputs for P_TryMove2
    gbCheckPosOnly = true;
    gpTryMoveThing = &mobj;
    gTryMoveX = x;
    gTryMoveY = y;

    // Check if the move can be done and return the output result
    P_TryMove2();
    return gbTryMove2;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try move the given map object to the x/y position and return 'true' if the move is allowed.
// This function will also do direct damage interactions for missiles, and item pickups if a player map object is being moved.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_TryMove(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept {
    // Save inputs for P_TryMove2 and attempt to do the move
    gpTryMoveThing = &mobj;
    gTryMoveX = x;
    gTryMoveY = y;
    P_TryMove2();

    // If we collided with something then try and interact with it it
    mobj_t* const pCollideThing = gpMoveThing;

    if (pCollideThing) {
        if (mobj.flags & MF_MISSILE) {
            // This thing being moved is a missile bashing into something: damage the thing hit
            const int32_t damage = ((P_Random() & 7) + 1) * mobj.info->damage;
            P_DamageMObj(*pCollideThing, &mobj, mobj.target.get(), damage);
        }
        else if (mobj.flags & MF_SKULLFLY) {
            // This thing being moved is a skull which has based into something: damage the thing hit
            const int32_t damage = ((P_Random() & 7) + 1) * mobj.info->damage;
            P_DamageMObj(*pCollideThing, &mobj, &mobj, damage);

            // Kill the skull velocity and stop it flying
            mobj.momz = 0;
            mobj.momy = 0;
            mobj.momx = 0;
            mobj.flags &= ~MF_SKULLFLY;

            // Skull goes back to it's idle state on hitting something
            P_SetMObjState(mobj, mobj.info->spawnstate);
        }
        else {
            // In all other cases try to pickup the item collided with if possible.
            // Note: if we are hitting this case then the thing being moved MUST be a player.
            P_TouchSpecialThing(*pCollideThing, mobj);
        }
    }

    return gbTryMove2;      // Was the move a success?
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Computes the fraction along 'line1' that the intersection with 'line2' occurs at.
// Returns '(fixed_t) -1' if there is no intersection.
//------------------------------------------------------------------------------------------------------------------------------------------
static fixed_t P_InterceptVector(divline_t& line1, divline_t& line2) noexcept {
    // This is pretty much the standard line/line intersection equation.
    // See: https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection 
    // Specifically the case "Given two points on each line".
    const int32_t ldx1 = line1.dx >> FRACBITS;
    const int32_t ldy1 = line1.dy >> FRACBITS;
    const int32_t ldx2 = line2.dx >> FRACBITS;
    const int32_t ldy2 = line2.dy >> FRACBITS;

    const int32_t dx = (line2.x - line1.x) >> FRACBITS;
    const int32_t dy = (line1.y - line2.y) >> FRACBITS;

    const int32_t num = (ldy2 * dx  ) + (dy   * ldx2);
    const int32_t den = (ldy2 * ldx1) - (ldx2 * ldy1);

    // Note: if the denominator is '0' then there is no intersect - parallel lines
    return (den != 0) ? (num << FRACBITS) / den : -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Iterator function called on all lines nearby the player when trying to activate a line special.
// Saves the line if it intersects the 'use' line being cast from the player, and if it is closer than the current intersecting line.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PIT_UseLines(line_t& line) noexcept {
    // If the 'use' bounding box doesn't cross the line then ignore the line and early out
    const bool noBBoxOverlap = (
        (gUseBBox[BOXTOP] <= line.bbox[BOXBOTTOM]) ||
        (gUseBBox[BOXBOTTOM] >= line.bbox[BOXTOP]) ||
        (gUseBBox[BOXLEFT] >= line.bbox[BOXRIGHT]) ||
        (gUseBBox[BOXRIGHT] <= line.bbox[BOXLEFT])
    );

    if (noBBoxOverlap)
        return true;

    // Intersect the two lines and bail out if there is no intersection
    divline_t divline;
    P_MakeDivline(line, divline);
    const fixed_t intersectFrac = P_InterceptVector(gUseLine, divline);

    if (intersectFrac < 0)  // No intersection, or intersection before the start of the use line
        return true;

    // If the intersection isn't closer than the current closest use line intersection then ignore also
    if (intersectFrac > gCloseDist)
        return true;

    // If the line is not usable then try to narrow the vertical range for the use line
    if (!line.special) {
        P_LineOpening(line);

        // If the vertical range for the use line is now shut then bail
        if (gOpenRange > 0)
            return true;
    }

    // This is the new closest line, save - along with the intersection fraction along the use line
    gpCloseLine = &line;
    gCloseDist = intersectFrac;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to use/activate surrounding map lines for the given player; called when the 'use' action is being done
//------------------------------------------------------------------------------------------------------------------------------------------
void P_UseLines(player_t& player) noexcept {
    // Figure out the start point and vector for the use line
    mobj_t& mobj = *player.mo;

    divline_t& useline = gUseLine;
    useline.x = mobj.x;
    useline.y = mobj.y;

    const uint32_t fineAngle = mobj.angle >> ANGLETOFINESHIFT;
    useline.dx = gFineCosine[fineAngle] * (USERANGE >> FRACBITS);
    useline.dy = gFineSine[fineAngle] * (USERANGE >> FRACBITS);

    // Figure out the bounding box for the use line
    if (useline.dx > 0) {
        gUseBBox[BOXLEFT] = useline.x;
        gUseBBox[BOXRIGHT] = useline.x + useline.dx;
    } else {
        gUseBBox[BOXLEFT] = useline.x + useline.dx;
        gUseBBox[BOXRIGHT] = useline.x;
    }
    
    if (useline.dy > 0) {
        gUseBBox[BOXTOP] = useline.y + useline.dy;
        gUseBBox[BOXBOTTOM] = useline.y;
    } else {
        gUseBBox[BOXTOP] = useline.y;
        gUseBBox[BOXBOTTOM] = useline.y + useline.dy;
    }
    
    // Initially no wall is hit and the closest thing is at fraction 1.0 (end of the line)
    gCloseDist = FRACUNIT;
    gpCloseLine = nullptr;

    // Now doing new checks
    gValidCount++;

    // Compute the blockmap extents to check for use lines.
    // PC-PSX: ensure these are always within a valid range to prevent undefined behavior at map edges.
    #if PC_PSX_DOOM_MODS
        const int32_t bmapTy = std::min((gUseBBox[BOXTOP] - gBlockmapOriginY) >> MAPBLOCKSHIFT, gBlockmapHeight - 1);
        const int32_t bmapBy = std::max((gUseBBox[BOXBOTTOM] - gBlockmapOriginY) >> MAPBLOCKSHIFT, 0);
        const int32_t bmapLx = std::max((gUseBBox[BOXLEFT] - gBlockmapOriginX) >> MAPBLOCKSHIFT, 0);
        const int32_t bmapRx = std::min((gUseBBox[BOXRIGHT] - gBlockmapOriginX) >> MAPBLOCKSHIFT, gBlockmapWidth - 1);
    #else
        const int32_t bmapTy = (gUseBBox[BOXTOP] - gBlockmapOriginY) >> MAPBLOCKSHIFT;
        const int32_t bmapBy = (gUseBBox[BOXBOTTOM] - gBlockmapOriginY) >> MAPBLOCKSHIFT;
        const int32_t bmapLx = (gUseBBox[BOXLEFT] - gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t bmapRx = (gUseBBox[BOXRIGHT] - gBlockmapOriginX) >> MAPBLOCKSHIFT;
    #endif

    // Check against all of the lines in these block map blocks to find the closest line to use
    for (int32_t y = bmapBy; y <= bmapTy; ++y) {
        for (int32_t x = bmapLx; x <= bmapRx; ++x) {
            P_BlockLinesIterator(x, y, PIT_UseLines);
        }
    }

    // Try to use the closest line (if any).
    // If the line has no special then play the grunting noise.
    line_t* const pClosestLine = gpCloseLine;

    if (!pClosestLine)
        return;
    
    if (pClosestLine->special) {
        P_UseSpecialLine(mobj, *pClosestLine);
    } else {
        S_StartSound(&mobj, sfx_noway);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Apply splash/bomb damage to the given thing from the current explosion, if applicable and in range etc.
// Note: the 'bomb source' is the thing that caused the explosion, not the projectile itself ('bomb spot').
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PIT_RadiusAttack(mobj_t& mobj) noexcept {
    // Non shootable things get no splash damage
    if ((mobj.flags & MF_SHOOTABLE) == 0)
        return true;

    // Cyberdemons and Masterminds don't get splash damage
    if ((mobj.type == MT_CYBORG) || (mobj.type == MT_SPIDER))
        return true;

    // Get a distance estimate to the source of the blast
    mobj_t& bombSpot = *gpBombSpot;
    
    const fixed_t dx = std::abs(mobj.x - bombSpot.x);
    const fixed_t dy = std::abs(mobj.y - bombSpot.y);
    const int32_t approxDist = std::max(dx, dy);

    // Compute how much to fade out damage based on the approx distance
    const int32_t damageFade = std::max((approxDist - mobj.radius) >> FRACBITS, 0);

    // Apply the actual damage if > 0 and if the thing has a line of sight to the explosion
    const int32_t bombBaseDamage = gBombDamage;
    mobj_t* pBombSource = gpBombSource;

    if ((bombBaseDamage > damageFade) && P_CheckSight(mobj, bombSpot)) {
        P_DamageMObj(mobj, &bombSpot, pBombSource, bombBaseDamage - damageFade);
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do an explosion at the given bomb spot, doing the specified amount of damage. The damage falls off linearly over distance.
// The given 'source' object (optional) is the thing responsible for causing the explosion.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_RadiusAttack(mobj_t& bombSpot, mobj_t* const pSource, const int32_t damage) noexcept {
    // Compute the range of the blockmap to search based on the damage amount.
    // Splash damage falls off linearly, so the damage amount is also pretty much the distance range:
    const fixed_t blastDist = damage << FRACBITS;

    #if PC_PSX_DOOM_MODS
        // PC-PSX: clamp these coords to the valid range of the blockmap to avoid potential undefined behavior near map edges
        const int32_t bmapLx = std::max((bombSpot.x - blastDist - gBlockmapOriginX) >> MAPBLOCKSHIFT, 0);
        const int32_t bmapRx = std::min((bombSpot.x + blastDist - gBlockmapOriginX) >> MAPBLOCKSHIFT, gBlockmapWidth - 1);
        const int32_t bmapBy = std::max((bombSpot.y - blastDist - gBlockmapOriginY) >> MAPBLOCKSHIFT, 0);
        const int32_t bmapTy = std::min((bombSpot.y + blastDist - gBlockmapOriginY) >> MAPBLOCKSHIFT, gBlockmapHeight - 1);
    #else
        const int32_t bmapLx = (bombSpot.x - blastDist - gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t bmapRx = (bombSpot.x + blastDist - gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t bmapBy = (bombSpot.y - blastDist - gBlockmapOriginY) >> MAPBLOCKSHIFT;
        const int32_t bmapTy = (bombSpot.y + blastDist - gBlockmapOriginY) >> MAPBLOCKSHIFT;
    #endif

    // Save bomb properties globally and apply the blast damage (where possible) to things within the blockmap search range
    gpBombSpot = &bombSpot;
    gpBombSource = pSource;
    gBombDamage = damage;

    for (int32_t y = bmapBy; y <= bmapTy; ++y) {
        for (int32_t x = bmapLx; x <= bmapRx; ++x) {
            P_BlockThingsIterator(x, y, PIT_RadiusAttack);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Figures out the vertical slope (up/down auto-aiming) for a shot to be taken by the given shooter.
// Returns the slope to use for the shot.
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_AimLineAttack(mobj_t& shooter, const angle_t angle, const fixed_t maxDist) noexcept {
    // Can't shoot outside view angles: set the allowed Z slope range for the shot
    gAimTopSlope = (100 * FRACUNIT) / 160;
    gAimBottomSlope = (-100 * FRACUNIT) / 160;

    // Setup for shot simulation and see what this shot would hit
    gValidCount++;
    gAttackAngle = angle;
    gAttackRange = maxDist;
    gpShooter = &shooter;
    P_Shoot2();

    // Save what thing is being targeted and return the computed slope.
    // If we hit a thing then use the slope to the thing, otherwise shoot level ahead (slope 0).
    gpLineTarget = gpShootMObj;
    return (gpShootMObj) ? gShootSlope : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Takes an instant (raycast) shot for the shooter in the specified direction, applying a certain amount of base damage if a thing is hit.
// If the vertical aim slope is 'INT32_MAX' then the bounds of the view frustrum are used to determine the min/max vertical aim range.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_LineAttack(mobj_t& shooter, const angle_t angle, const fixed_t maxDist, const fixed_t zSlope, const int32_t damage) noexcept {
    // If the aim slope is INT32_MAX then we use the screen bounds to determine the min/max slope.
    // Otherwise just expand the specified slope range by 1 each way, so it's not zero sized.
    if (zSlope == INT32_MAX) {
        gAimTopSlope = 100 * FRACUNIT / 160;
        gAimBottomSlope = -100 * FRACUNIT / 160;
    } else {
        gAimTopSlope = zSlope + 1;
        gAimBottomSlope = zSlope - 1;
    }

    // Take the shot!
    gValidCount++;
    gAttackAngle = angle;
    gAttackRange = maxDist;
    gpShooter = &shooter;
    P_Shoot2();

    // Grab the details for what was shot
    const fixed_t shootX = gShootX;
    const fixed_t shootY = gShootY;
    const fixed_t shootZ = gShootZ;
    line_t* const pShootLine = gpShootLine;
    mobj_t* const pShootMobj = gpShootMObj;

    // Save what thing was hit globally
    gpLineTarget = pShootMobj;

    // Shooting a thing?
    if (pShootMobj) {
        // Do blood (or smoke) then damage the thing
        if (pShootMobj->flags & MF_NOBLOOD) {
            P_SpawnPuff(shootX, shootY, gShootZ);
        } else {
            P_SpawnBlood(shootX, shootY, gShootZ, damage);
        }

        P_DamageMObj(*pShootMobj, &shooter, &shooter, damage);
        return;
    }

    // Shooting a line?
    if (pShootLine) {
        // Try activate a line special if the line has it
        if (pShootLine->special) {
            P_ShootSpecialLine(shooter, *pShootLine);
        }

        // Spawn a puff unless we are shooting the sky (ceiling texture -1)
        sector_t& sector = *pShootLine->frontsector;

        if (sector.ceilingpic == -1) {
            if (shootZ > sector.ceilingheight)
                return;     // Not allowed to shoot the sky ceiling!

            if (pShootLine->backsector && pShootLine->backsector->ceilingpic == -1)
                return;     // Shooting a sky upper/lower wall!
        }

        P_SpawnPuff(shootX, shootY, shootZ);
        return;
    }
}
