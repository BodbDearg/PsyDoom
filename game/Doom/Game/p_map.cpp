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
#include "PsxVm/PsxVm.h"
#include <algorithm>

const VmPtr<VmPtr<mobj_t>>      gpShooter(0x800780B4);          // The map object currently taking a shot
const VmPtr<fixed_t>            gAttackRange(0x80077F98);       // Maximum attack range for an attacker
const VmPtr<angle_t>            gAttackAngle(0x80077F80);       // Angle of attack for an attacker
const VmPtr<fixed_t>            gAimTopSlope(0x80077FF8);       // Maximum Z slope for shooting (defines Z range that stuff can be hit within)
const VmPtr<fixed_t>            gAimBottomSlope(0x800782F8);    // Minimum Z slope for shooting (defines Z range that stuff can be hit within)
const VmPtr<VmPtr<mobj_t>>      gpTryMoveThing(0x8007808C);     // Try move: the thing being moved
const VmPtr<fixed_t>            gTryMoveX(0x80078150);          // Try move: position we're attempting to move to (X)
const VmPtr<fixed_t>            gTryMoveY(0x80078154);          // Try move: position we're attempting to move to (Y)
const VmPtr<bool32_t>           gbCheckPosOnly(0x800780E8);     // Try move: if 'true' then check if the position is valid to move to only, don't actually move there

static const VmPtr<VmPtr<mobj_t>>   gpLineTarget(0x80077EE8);   // The thing being shot at in 'P_AimLineAttack' and 'P_LineAttack'.
static const VmPtr<VmPtr<mobj_t>>   gpBombSource(0x80077EF0);   // Radius attacks: the thing responsible for the explosion (player, monster)
static const VmPtr<VmPtr<mobj_t>>   gpBombSpot(0x800781A0);     // Radius attacks: the object exploding and it's position (barrel, missile etc.)
static const VmPtr<int32_t>         gBombDamage(0x80077E94);    // Radius attacks: how much damage the explosion does before falloff
static const VmPtr<divline_t>       gUseLine(0x800A8748);       // The 'use' line being cast from the player towards walls; we try to activate walls that it hits
static const VmPtr<fixed_t[4]>      gUseBBox(0x800A875C);       // The bounding box for the 'use' line being cast from the player
static const VmPtr<VmPtr<line_t>>   gpCloseLine(0x80078274);    // The closest wall line currently being used
static const VmPtr<fixed_t>         gCloseDist(0x800782A8);     // Fractional distance along the use line to the closest wall line being used

//------------------------------------------------------------------------------------------------------------------------------------------
// Test if the given x/y position can be moved to for the given map object and return 'true' if the move is allowed
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_CheckPosition(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept {
    // Save inputs for P_TryMove2
    *gbCheckPosOnly = true;
    *gpTryMoveThing = &mobj;
    *gTryMoveX = x;
    *gTryMoveY = y;

    // Check if the move can be done and return the output result
    P_TryMove2();
    return *gbTryMove2;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try move the given map object to the x/y position and return 'true' if the move is allowed.
// This function will also do direct damage interactions for missiles, and item pickups if a player map object is being moved.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_TryMove(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept {
    // Save inputs for P_TryMove2 and attempt to do the move
    *gpTryMoveThing = &mobj;
    *gTryMoveX = x;
    *gTryMoveY = y;
    P_TryMove2();

    // If we collided with something then try and interact with it it
    mobj_t* const pCollideThing = gpMoveThing->get();

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

    return *gbTryMove2;     // Was the move a success?
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Computes the fraction along 'line1' that the intersection with 'line2' occurs at.
// Returns '(fixed_t) -1' if there is no intersection.
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_InterceptVector(divline_t& line1, divline_t& line2) noexcept {
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
bool PIT_UseLines(line_t& line) noexcept {
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
    const fixed_t intersectFrac = P_InterceptVector(*gUseLine, divline);

    if (intersectFrac < 0)  // No intersection, or intersection before the start of the use line
        return true;

    // If the intersection isn't closer than the current closest use line intersection then ignore also
    if (intersectFrac > *gCloseDist)
        return true;

    // If the line is not usable then try to narrow the vertical range for the use line
    if (!line.special) {
        P_LineOpening(line);

        // If the vertical range for the use line is now shut then bail
        if (*gOpenRange > 0)
            return true;
    }

    // This is the new closest line, save - along with the intersection fraction along the use line
    *gpCloseLine = &line;
    *gCloseDist = intersectFrac;
    return true;
}

void P_UseLines() noexcept {
loc_8001B9F4:
    sp -= 0x38;
    sw(s5, sp + 0x2C);
    s5 = a0;
    sw(ra, sp + 0x30);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    a0 = lw(s5);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v0 = lw(a0 + 0x24);
    a2 = lw(a0);
    a3 = lw(a0 + 0x4);
    v0 >>= 19;
    v0 <<= 2;
    v1 += v0;
    v1 = lw(v1);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v0;
    a0 = lw(at);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a2, at - 0x78B8);                                // Store to: gUseLine[0] (800A8748)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x78B4);                                // Store to: gUseLine[1] (800A874C)
    v0 = v1 << 3;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 1;
    a1 = a2 + v0;
    v1 = a1 - a2;
    v0 = a0 << 3;
    v0 += a0;
    v0 <<= 2;
    v0 -= a0;
    v0 <<= 1;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v1, at - 0x78B0);                                // Store to: gUseLine[2] (800A8750)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x78AC);                                // Store to: gUseLine[3] (800A8754)
    a0 = a3 + v0;
    if (i32(v1) <= 0) goto loc_8001BAC0;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a1, at - 0x7898);                                // Store to: gUseBBox[3] (800A8768)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a2, at - 0x789C);                                // Store to: gUseBBox[2] (800A8764)
    goto loc_8001BAD0;
loc_8001BAC0:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a2, at - 0x7898);                                // Store to: gUseBBox[3] (800A8768)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a1, at - 0x789C);                                // Store to: gUseBBox[2] (800A8764)
loc_8001BAD0:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78AC);                               // Load from: gUseLine[3] (800A8754)
    if (i32(v0) <= 0) goto loc_8001BAFC;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a0, at - 0x78A4);                                // Store to: gUseBBox[0] (800A875C)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x78A0);                                // Store to: gUseBBox[1] (800A8760)
    goto loc_8001BB0C;
loc_8001BAFC:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x78A4);                                // Store to: gUseBBox[0] (800A875C)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a0, at - 0x78A0);                                // Store to: gUseBBox[1] (800A8760)
loc_8001BB0C:
    a1 = *gBlockmapOriginY;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A0);                               // Load from: gUseBBox[1] (800A8760)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A4);                               // Load from: gUseBBox[0] (800A875C)
    v1 = 0x10000;                                       // Result = 00010000
    sw(v1, gp + 0xCC8);                                 // Store to: gCloseDist (800782A8)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x7898);                               // Load from: gUseBBox[3] (800A8768)
    sw(0, gp + 0xC94);                                  // Store to: gpCloseLine (80078274)
    v0 -= a1;
    s1 = u32(i32(v0) >> 23);
    a0 -= a1;
    s3 = u32(i32(a0) >> 23);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    a1 = *gBlockmapOriginX;
    v0++;
    v1 -= a1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x789C);                               // Load from: gUseBBox[2] (800A8764)
    v0 -= a1;
    s4 = u32(i32(v0) >> 23);
    v0 = (i32(s3) < i32(s1));
    s2 = u32(i32(v1) >> 23);
    if (v0 != 0) goto loc_8001BBC8;
    v0 = (i32(s2) < i32(s4));
loc_8001BB8C:
    s0 = s4;
    if (v0 != 0) goto loc_8001BBB8;
    a0 = s0;
loc_8001BB98:
    a2 = 0x80020000;                                    // Result = 80020000
    a2 -= 0x47B8;                                       // Result = PIT_UseLines (8001B848)
    a1 = s1;
    v0 = P_BlockLinesIterator(a0, a1, PIT_UseLines);
    s0++;
    v0 = (i32(s2) < i32(s0));
    a0 = s0;
    if (v0 == 0) goto loc_8001BB98;
loc_8001BBB8:
    s1++;
    v0 = (i32(s3) < i32(s1));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s2) < i32(s4));
        if (bJump) goto loc_8001BB8C;
    }
loc_8001BBC8:
    a1 = lw(gp + 0xC94);                                // Load from: gpCloseLine (80078274)
    if (a1 == 0) goto loc_8001BC08;
    v0 = lw(a1 + 0x14);
    if (v0 != 0) goto loc_8001BBFC;
    a0 = lw(s5);
    a1 = sfx_noway;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    goto loc_8001BC08;
loc_8001BBFC:
    a0 = lw(s5);
    v0 = P_UseSpecialLine(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<line_t>(a1));
loc_8001BC08:
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Apply splash/bomb damage to the given thing from the current explosion, if applicable and in range etc.
// Note: the 'bomb source' is the thing that caused the explosion, not the projectile itself ('bomb spot').
//------------------------------------------------------------------------------------------------------------------------------------------
bool PIT_RadiusAttack(mobj_t& mobj) noexcept {
    // Non shootable things get no splash damage
    if ((mobj.flags & MF_SHOOTABLE) == 0)
        return true;

    // Cyberdemons and Masterminds don't get splash damage
    if ((mobj.type == MT_CYBORG) || (mobj.type == MT_SPIDER))
        return true;

    // Get a distance estimate to the source of the blast
    mobj_t& bombSpot = *gpBombSpot->get();
    
    const fixed_t dx = std::abs(mobj.x - bombSpot.x);
    const fixed_t dy = std::abs(mobj.y - bombSpot.y);
    const int32_t approxDist = std::max(dx, dy);

    // Compute how much to fade out damage based on the approx distance
    const int32_t damageFade = std::max((approxDist - mobj.radius) >> FRACBITS, 0);

    // Apply the actual damage if > 0 and if the thing has a line of sight to the explosion
    const int32_t bombBaseDamage = *gBombDamage;
    mobj_t* pBombSource = gpBombSource->get();

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
        const int32_t bmapLx = std::max((bombSpot.x - blastDist - *gBlockmapOriginX) >> MAPBLOCKSHIFT, 0);
        const int32_t bmapRx = std::min((bombSpot.x + blastDist - *gBlockmapOriginX) >> MAPBLOCKSHIFT, *gBlockmapWidth - 1);
        const int32_t bmapBy = std::max((bombSpot.y - blastDist - *gBlockmapOriginY) >> MAPBLOCKSHIFT, 0);
        const int32_t bmapTy = std::min((bombSpot.y + blastDist - *gBlockmapOriginY) >> MAPBLOCKSHIFT, *gBlockmapHeight - 1);
    #else
        const int32_t bmapLx = (bombSpot.x - blastDist - *gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t bmapRx = (bombSpot.x + blastDist - *gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t bmapBy = (bombSpot.y - blastDist - *gBlockmapOriginY) >> MAPBLOCKSHIFT;
        const int32_t bmapTy = (bombSpot.y + blastDist - *gBlockmapOriginY) >> MAPBLOCKSHIFT;
    #endif

    // Save bomb properties globally and apply the blast damage (where possible) to things within the blockmap search range
    *gpBombSpot = &bombSpot;
    *gpBombSource = pSource;
    *gBombDamage = damage;

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
    *gAimTopSlope = (100 * FRACUNIT) / 160;
    *gAimBottomSlope = (-100 * FRACUNIT) / 160;

    // Setup for shot simulation and see what this shot would hit
    *gValidCount += 1;
    *gAttackAngle = angle;
    *gAttackRange = maxDist;
    *gpShooter = &shooter;
    P_Shoot2();

    // Save what thing is being targeted and return the computed slope.
    // If we hit a thing then use the slope to the thing, otherwise shoot level ahead (slope 0).
    *gpLineTarget = gpShootMObj->get();
    return (gpShootMObj->get()) ? *gShootSlope : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Takes an instant (raycast) shot for the shooter in the specified direction, applying a certain amount of base damage if a thing is hit.
// If the vertical aim slope is 'INT32_MAX' then the bounds of the view frustrum are used to determine the min/max vertical aim range.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_LineAttack(mobj_t& shooter, const angle_t angle, const fixed_t maxDist, const fixed_t zSlope, const int32_t damage) noexcept {
    // If the aim slope is INT32_MAX then we use the screen bounds to determine the min/max slope.
    // Otherwise just expand the specified slope range by 1 each way, so it's not zero sized.
    if (zSlope == INT32_MAX) {
        *gAimTopSlope = 100 * FRACUNIT / 160;
        *gAimBottomSlope = -100 * FRACUNIT / 160;
    } else {
        *gAimTopSlope = zSlope + 1;
        *gAimBottomSlope = zSlope - 1;
    }

    // Take the shot!
    *gValidCount += 1;
    *gAttackAngle = angle;
    *gAttackRange = maxDist;
    *gpShooter = &shooter;
    P_Shoot2();

    // Grab the details for what was shot
    const fixed_t shootX = *gShootX;
    const fixed_t shootY = *gShootY;
    const fixed_t shootZ = *gShootZ;
    line_t* const pShootLine = gpShootLine->get();
    mobj_t* const pShootMobj = gpShootMObj->get();

    // Save what thing was hit globally
    *gpLineTarget = pShootMobj;

    // Shooting a thing?
    if (pShootMobj) {
        // Do blood (or smoke) then damage the thing
        if (pShootMobj->flags & MF_NOBLOOD) {
            P_SpawnPuff(shootX, shootY, *gShootZ);
        } else {
            P_SpawnBlood(shootX, shootY, *gShootZ, damage);
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
