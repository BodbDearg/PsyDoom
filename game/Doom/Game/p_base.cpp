//------------------------------------------------------------------------------------------------------------------------------------------
// Module responsible for non player map object movement (and collision) due to velocity, as well as state advancement/ticking.
// The movement handled here is physics based and caused by velocity - it is NOT the manual movement done by the AI code.
//
// Note: state transition action functions are NOT performed here, they are instead performed during the 'latecall' phase.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "p_base.h"

#include "Asserts.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "info.h"
#include "p_enemy.h"
#include "p_local.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_setup.h"
#include "p_tick.h"

#include <algorithm>

static constexpr fixed_t STOPSPEED  = 0x1000;   // Speed under which to stop a thing fully
static constexpr fixed_t FRICTION   = 0xD200;   // Friction amount to apply (note: 0xD240 in Jaguar Doom)

static mobj_t*          gpBaseThing;        // The current thing that is doing collision testing against other stuff: used by various functions in the module
static fixed_t          gTestX;             // The thing position to use for collision testing - X
static fixed_t          gTestY;             // The thing position to use for collision testing - Y
static fixed_t          gTestBBox[4];       // Bounding box for various collision tests
static uint32_t         gTestFlags;         // Used in place of 'mobj_t' flags for various functions in this module
static subsector_t*     gpTestSubSec;       // Current cached thing subsector: input and output for some functions in this module
static mobj_t*          gpHitThing;         // The thing that was collided against during collision testing
static line_t*          gpCeilingLine;      // Collision testing: the line for the lowest ceiling edge the collider is in contact with
static fixed_t          gTestCeilingz;      // Collision testing: the Z value for the lowest ceiling the collider is in contact with
static fixed_t          gTestFloorZ;        // Collision testing: the Z value for the highest floor the collider is in contact with
static fixed_t          gTestDropoffZ;      // Collision testing: the Z value for the lowest floor the collider is in contact with. Used by monsters so they don't walk off cliffs.

// Not required externally: making private to this module
static bool PB_TryMove(const fixed_t tryX, const fixed_t tryY) noexcept;
static void PB_UnsetThingPosition(mobj_t& thing) noexcept;
static void PB_SetThingPosition(mobj_t& mobj) noexcept;
static bool PB_CheckPosition() noexcept;
static bool PB_BlockLinesIterator(const int32_t x, const int32_t y) noexcept;
static bool PB_BlockThingsIterator(const int32_t x, const int32_t y) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Does movement and state ticking for all map objects except players
//------------------------------------------------------------------------------------------------------------------------------------------
void P_RunMobjBase() noexcept {
    gpBaseThing = gMobjHead.next;

    // Run through all the map objects
    while (gpBaseThing != &gMobjHead) {
        mobj_t& mobj = *gpBaseThing;

        // Only run the think logic if it's not the player.
        if (!mobj.player) {
            // Note: clear the latecall here to signify (initially) no mobj action to execute during the 'latecall' phase.
            // The think function might set an action though, typically for a state transition or sometimes a missile explosion etc.
            mobj.latecall = nullptr;
            P_MobjThinker(mobj);
        }

        gpBaseThing = mobj.next;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does movement along the xy plane for the specified map object
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_XYMovement(mobj_t& mobj) noexcept {
    // How much movement is left to do: chop off the lowest 3 bits also
    fixed_t xleft = mobj.momx & (~7);
    fixed_t yleft = mobj.momy & (~7);

    // The size of each movement step: find an acceptable amount
    fixed_t xuse = xleft;
    fixed_t yuse = yleft;

    while ((std::abs(xuse) > MAXMOVE) || (std::abs(yuse) > MAXMOVE)) {
        xuse = d_rshift<1>(xuse);
        yuse = d_rshift<1>(yuse);
    }

    // Continue moving until we are done
    while ((xleft != 0) || (yleft != 0)) {
        xleft -= xuse;
        yleft -= yuse;

        // Try to do this move step
        if (!PB_TryMove(mobj.x + xuse, mobj.y + yuse)) {
            // Move failed: if it's a skull flying then do the skull bash
            if (mobj.flags & MF_SKULLFLY) {
                mobj.latecall = &L_SkullBash;
                mobj.extradata = (uintptr_t) gpHitThing;
            }

            // If it's a missile explode or remove, otherwise stop momentum fully
            if (mobj.flags & MF_MISSILE) {
                // Missile: if the missile hit the sky then just remove it rather than exploding
                const bool bHitSky = (
                    gpCeilingLine &&
                    gpCeilingLine->backsector &&
                    (gpCeilingLine->backsector->ceilingpic == -1)
                );

                if (bHitSky) {
                    // Hit the sky: just remove quietly
                    mobj.latecall = &P_RemoveMobj;
                } else {
                    // Usual case: exploding on hitting a wall or thing
                    mobj.latecall = &L_MissileHit;
                    mobj.extradata = (uintptr_t) gpHitThing;
                }
            } else {
                // Remove all momentum since the thing cannot move
                mobj.momx = 0;
                mobj.momy = 0;
            }

            // We are done: no more movement
            return;
        }
    }

    // Missiles and lost souls do not have friction
    if (mobj.flags & (MF_MISSILE | MF_SKULLFLY))
        return;

    // No friction when airborne
    if (mobj.z > mobj.floorz)
        return;

    // Don't stop corpses sliding until they hit the floor
    if ((mobj.flags & MF_CORPSE) && (mobj.floorz != mobj.subsector->sector->floorheight))
        return;

    // Stop the thing fully if it's now slow enough, otherwise apply friction
    if ((std::abs(mobj.momx) < STOPSPEED) && (std::abs(mobj.momy) < STOPSPEED)) {
        mobj.momx = 0;
        mobj.momy = 0;
    } else {
        mobj.momx = d_rshift<8>(mobj.momx) * d_rshift<8>(FRICTION);
        mobj.momy = d_rshift<8>(mobj.momy) * d_rshift<8>(FRICTION);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does upward and downward floating towards the target for the given map object which is a floating monster.
// Assumes the map object has a target.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_FloatChange(mobj_t& mobj) noexcept {
    // Get the approximate distance to the target
    ASSERT(mobj.target);
    mobj_t& target = *mobj.target;
    const fixed_t approxDist = P_AproxDistance(target.x - mobj.x, target.y - mobj.y);

    // Get the height difference to the target and multiply by 3 for an apparent fudge factor. Not sure why the fudge was added in...
    // I'm wondering also should this have been getting the (vertical) center to center height difference instead?
    const fixed_t dz = (d_rshift<1>(mobj.height) + target.z - mobj.z) * 3;

    if (dz < 0) {
        if (approxDist < -dz) {     // N.B: 'dz' is signed difference and negative here, need to adjust
            mobj.z -= FLOATSPEED;
        }
    }
    else if (dz > 0) {
        if (approxDist < dz) {
            mobj.z += FLOATSPEED;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does upwards and downwards movement for the given map object, including applying gravity and floating up/down for floating monsters
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_ZMovement(mobj_t& mobj) noexcept {
    // Advance Z position by current Z motion
    mobj.z += mobj.momz;

    #if PSYDOOM_MODS
        // PsyDoom: allow things to 'noclip' in addition to the player.
        // This is required for the "Icon Of Sin" spawner boxes to work properly.
        const bool bNoClip = (mobj.flags & MF_NOCLIP);
    #else
        constexpr bool bNoClip = false;
    #endif

    // Do floating up and down to meet the target for floating monsters
    if ((mobj.flags & MF_FLOAT) && mobj.target) {
        P_FloatChange(mobj);
    }

    // Collide with the floor, or if no collision then do gravity
    if (mobj.z <= mobj.floorz) {
        // Hitting the floor: stop all downwards momentum
        if (mobj.momz < 0) {
            mobj.momz = 0;
        }

        // Clamp to the floor and explode missiles (unless noclipping)
        mobj.z = mobj.floorz;

        if ((mobj.flags & MF_MISSILE) && (!bNoClip)) {
            mobj.latecall = &P_ExplodeMissile;  // BOOM!
            return;
        }
    }
    else if ((mobj.flags & MF_NOGRAVITY) == 0) {
        // Not hitting the floor and gravity is enabled, so apply it:
        if (mobj.momz == 0) {
            mobj.momz = -P_GetGravity();
        } else {
            mobj.momz -= P_GetGravity() / 2;
        }
    }

    // Check for a collision against the ceiling
    if (mobj.z + mobj.height > mobj.ceilingz) {
        // Hitting the ceiling: stop all upwards momentum
        if (mobj.momz > 0) {
            mobj.momz = 0;
        }

        // Clamp to the ceiling and explode missiles (unless noclipping)
        mobj.z = mobj.ceilingz - mobj.height;

        if ((mobj.flags & MF_MISSILE) && (!bNoClip)) {
            mobj.latecall = &P_ExplodeMissile;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does movement and state ticking for the specified map object.
// Note: doesn't run state functions, that's done during the phase for running map object late calls.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_MobjThinker(mobj_t& mobj) noexcept {
    // Do xy plane movement if there is current velocity
    if ((mobj.momx != 0) || (mobj.momy != 0)) {
        P_XYMovement(mobj);

        // Stop if the object is being removed via a latecall or has some other special action to perform
        if (mobj.latecall)
            return;
    }

    // Do up/down movement if there is current velocity or if the thing is in the air
    if ((mobj.momz != 0) || (mobj.z != mobj.floorz)) {
        P_ZMovement(mobj);

        // Stop if the object is being removed via a latecall or has some other special action to perform
        if (mobj.latecall)
            return;
    }

    // Do state advancement if this state does not last forever (-1 tics duration)
    if (mobj.tics != -1) {
        mobj.tics--;

        // Is it time to change to the next state?
        if (mobj.tics <= 0) {
            const statenum_t nextStateNum = mobj.state->nextstate;

            // Is there a next state?
            if (nextStateNum != S_NULL) {
                // There is a next state: setup the map object's sprite, pending action and remaining state tics for this state
                state_t& nextState = gStates[nextStateNum];
                mobj.state = &nextState;
                mobj.tics = nextState.tics;
                mobj.sprite = nextState.sprite;
                mobj.frame = nextState.frame;
                mobj.latecall = nextState.action.mobjFn;
            } else {
                // No next state: schedule a removal for this map object
                mobj.latecall = &P_RemoveMobj;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to move 'gpBaseThing' to the specified x/y position.
// Returns 'true' if the move was successful and updates the thing position.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PB_TryMove(const fixed_t tryX, const fixed_t tryY) noexcept {
    // Save the position we are attempting to move to
    gTestX = tryX;
    gTestY = tryY;

    // If we collided for sure with something (ignoring height) then stop now
    if (!PB_CheckPosition())
        return false;

    // If the floor/ceiling height gap is too small to move through then the move cannot happen
    mobj_t& baseThing = *gpBaseThing;

    #if PSYDOOM_MODS
        // PsyDoom: allow things to 'noclip' in addition to the player.
        // This is required for the "Icon Of Sin" spawner boxes to work properly.
        const bool bDoHeightChecks = ((baseThing.flags & MF_NOCLIP) == 0);
    #else
        constexpr bool bDoHeightChecks = true;
    #endif

    if (bDoHeightChecks) {
        if (gTestCeilingz - gTestFloorZ < baseThing.height)
            return false;

        // If the thing is too high up in the air to pass under the upper wall then the move cannot happen
        if (gTestCeilingz - baseThing.z < baseThing.height)
            return false;

        // If the step up is too big for a step then the move cannot happen
        if (gTestFloorZ - baseThing.z > 24 * FRACUNIT)
            return false;

        // See if the fall is too large (monsters).
        // Only do this test for things that don't float and which care about falling off cliffs.
        if ((gTestFlags & (MF_DROPOFF | MF_FLOAT)) == 0) {
            if (gTestFloorZ - gTestDropoffZ > 24 * FRACUNIT) {
                // Drop is too large for this thing!
                return false;
            }
        }
    }

    // If we've gotten to here then we have cleared all height check hurdles.
    // Remove the thing from the sector thing lists and the blockmap, update it's position etc. and then re-add.
    PB_UnsetThingPosition(baseThing);

    baseThing.floorz = gTestFloorZ;
    baseThing.ceilingz = gTestCeilingz;
    baseThing.x = gTestX;
    baseThing.y = gTestY;

    PB_SetThingPosition(baseThing);

    // This move was successful
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unlinks the given thing from sector thing lists and the blockmap.
// Very similar to 'P_UnsetThingPosition' except the thing is always unlinked from sectors and thing flags are read from a global.
//------------------------------------------------------------------------------------------------------------------------------------------
static void PB_UnsetThingPosition(mobj_t& thing) noexcept {
    // Remove the thing from sector thing lists
    if (thing.snext) {
        thing.snext->sprev = thing.sprev;
    }

    if (thing.sprev) {
        thing.sprev->snext = thing.snext;
    } else {
        thing.subsector->sector->thinglist = thing.snext;
    }

    // Remove the thing from the blockmap, if it is added to the blockmap
    if ((gTestFlags & MF_NOBLOCKMAP) == 0) {
        if (thing.bnext) {
            thing.bnext->bprev = thing.bprev;
        }

        if (thing.bprev) {
            thing.bprev->bnext = thing.bnext;
        } else {
            const int32_t blockx = d_rshift<MAPBLOCKSHIFT>(thing.x - gBlockmapOriginX);
            const int32_t blocky = d_rshift<MAPBLOCKSHIFT>(thing.y - gBlockmapOriginY);

            // PsyDoom: prevent buffer overflow if the map object is out of bounds.
            // This is part of the fix for the famous 'linedef deletion' bug.
            #if PSYDOOM_MODS && PSYDOOM_FIX_UB
                if (blockx >= 0 && blockx < gBlockmapWidth) {
                    if (blocky >= 0 && blocky < gBlockmapHeight) {
                        gppBlockLinks[blocky * gBlockmapWidth + blockx] = thing.bnext;
                    }
                }
            #else
                ASSERT((blockx >= 0) && (blockx < gBlockmapWidth));
                ASSERT((blocky >= 0) && (blocky < gBlockmapHeight));

                gppBlockLinks[blocky * gBlockmapWidth + blockx] = thing.bnext;
            #endif
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Links the given thing to sector thing lists and the blockmap. Very similar to 'P_SetThingPosition' except the thing is always linked
// to sectors and thing flags and the current subsector are taken from globals set elsewhere.
//------------------------------------------------------------------------------------------------------------------------------------------
static void PB_SetThingPosition(mobj_t& mobj) noexcept {
    // Add the thing into the sector thing linked list
    subsector_t& subsec = *gpTestSubSec;
    sector_t& sec = *subsec.sector;

    mobj.subsector = &subsec;
    mobj.sprev = nullptr;
    mobj.snext = sec.thinglist;

    if (sec.thinglist) {
        sec.thinglist->sprev = &mobj;
    }

    sec.thinglist = &mobj;

    // Add the thing into the blockmap unless the thing flags specify otherwise (inert things)
    if ((gTestFlags & MF_NOBLOCKMAP) == 0) {
        // Compute the blockmap cell and see if it's in range for the blockmap
        const int32_t bmapX = d_rshift<MAPBLOCKSHIFT>(mobj.x - gBlockmapOriginX);
        const int32_t bmapY = d_rshift<MAPBLOCKSHIFT>(mobj.y - gBlockmapOriginY);

        if ((bmapX >= 0) && (bmapY >= 0) && (bmapX < gBlockmapWidth) && (bmapY < gBlockmapHeight)) {
            // In range: link the thing into the blockmap list for this blockmap cell
            mobj_t*& blockmapList = gppBlockLinks[bmapX + bmapY * gBlockmapWidth];
            mobj_t* const pPrevListHead = blockmapList;

            mobj.bprev = nullptr;
            mobj.bnext = pPrevListHead;

            if (pPrevListHead) {
                pPrevListHead->bprev = &mobj;
            }

            blockmapList = &mobj;
        } else {
            // Thing is outside the blockmap
            mobj.bprev = nullptr;
            mobj.bnext = nullptr;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do a collision test for a thing against lines and other things, ignoring height differences.
// Returns 'false' if there was a collision, 'true' if there was no collision when height differences are ignored.
// Note: height difference blocking logic is handled externally to this function.
//
// Inputs:
//  gpBaseThing     : The thing doing the collision test
//  gTestX, gTestY  : Position to use for the thing for the collision test (can be set different to actual pos to test a move)
//
// Outputs:
//  gTestBBox       : The bounding box for the thing
//  gpTestSubSec    : The new subsector the thing would be in at the given position
//  gpHitThing      : The thing collided with
//  gpCeilingLine   : The upper wall line for the lowest ceiling touched
//  gTestCeilingz   : The Z value for the lowest ceiling touched
//  gTestFloorZ     : The Z value for the highest floor touched
//  gTestDropoffZ   : The Z value for the lowest floor touched
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PB_CheckPosition() noexcept {
    // Save the bounding box, flags and subsector for the thing having collision testing done
    mobj_t& baseThing = *gpBaseThing;
    gTestFlags = baseThing.flags;

    {
        const fixed_t radius = baseThing.radius;
        gTestBBox[BOXTOP] = gTestY + radius;
        gTestBBox[BOXBOTTOM] = gTestY - radius;
        gTestBBox[BOXLEFT] = gTestX - radius;
        gTestBBox[BOXRIGHT] = gTestX + radius;
    }

    subsector_t& testSubsec = *R_PointInSubsector(gTestX, gTestY);
    sector_t& testSec = *testSubsec.sector;
    gpTestSubSec = &testSubsec;

    // Initially have collided with nothing
    gpCeilingLine = nullptr;
    gpHitThing = nullptr;

    // Initialize the lowest ceiling, and highest/lowest floor values to that of the initial subsector
    gTestFloorZ = testSec.floorheight;
    gTestDropoffZ = testSec.floorheight;
    gTestCeilingz = testSec.ceilingheight;

    #if PSYDOOM_MODS
        // PsyDoom: allow things to 'noclip' in addition to the player.
        // This is required for the "Icon Of Sin" spawner boxes to work properly.
        if (baseThing.flags & MF_NOCLIP)
            return true;
    #endif

    // Determine the blockmap extents (left/right, top/bottom) to be tested against for collision and clamp to a valid range
    const int32_t bmapLx = std::max(d_rshift<MAPBLOCKSHIFT>(gTestBBox[BOXLEFT] - gBlockmapOriginX - MAXRADIUS), 0);
    const int32_t bmapRx = std::min(d_rshift<MAPBLOCKSHIFT>(gTestBBox[BOXRIGHT] - gBlockmapOriginX + MAXRADIUS), gBlockmapWidth - 1);
    const int32_t bmapTy = std::min(d_rshift<MAPBLOCKSHIFT>(gTestBBox[BOXTOP] - gBlockmapOriginY + MAXRADIUS), gBlockmapHeight - 1);
    const int32_t bmapBy = std::max(d_rshift<MAPBLOCKSHIFT>(gTestBBox[BOXBOTTOM] - gBlockmapOriginY - MAXRADIUS), 0);

    // This is a new collision test so increment this stamp
    gValidCount++;

    // Test against everything in this blockmap range.
    // Stop and return 'false' for a definite collision if that happens.
    for (int32_t x = bmapLx; x <= bmapRx; ++x) {
        for (int32_t y = bmapBy; y <= bmapTy; ++y) {
            // Test against lines first then things
            if (!PB_BlockLinesIterator(x, y))
                return false;

            if (!PB_BlockThingsIterator(x, y))
                return false;
        }
    }

    // No definite collision, there may still be collisions due to height differences however
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Test if 'gTestBBox' intersects the given line: returns 'true' if there is an intersection
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PB_BoxCrossLine(line_t& line) noexcept {
    // Check if the test bounding box is outside the bounding box of the line: if it is then early out
    const bool bTestBBOutsideLineBB = (
        (gTestBBox[BOXTOP] <= line.bbox[BOXBOTTOM]) ||
        (gTestBBox[BOXBOTTOM] >= line.bbox[BOXTOP]) ||
        (gTestBBox[BOXLEFT] >= line.bbox[BOXRIGHT]) ||
        (gTestBBox[BOXRIGHT] <= line.bbox[BOXLEFT])
    );

    if (bTestBBOutsideLineBB)
        return false;

    // Choose what line diagonal in the test box to test for crossing the line.
    // This code is trying to get a box diagonal that is as perpendicular to the line as possible.
    // Some lines for instance might run at 45 degrees and be parallel to the opposite box diagonal...
    fixed_t x1;
    fixed_t x2;

    if (line.slopetype == ST_POSITIVE) {
        x1 = gTestBBox[BOXLEFT];
        x2 = gTestBBox[BOXRIGHT];
    } else {
        x1 = gTestBBox[BOXRIGHT];
        x2 = gTestBBox[BOXLEFT];
    }

    // Use the cross product trick found in many functions such as 'R_PointOnSide' to determine what side of the line
    // both points of the test bounding box diagonal lie on.
    const fixed_t lx = line.vertex1->x;
    const fixed_t ly = line.vertex1->y;
    const int32_t ldx = d_fixed_to_int(line.dx);
    const int32_t ldy = d_fixed_to_int(line.dy);

    const int32_t dx1 = d_fixed_to_int(x1 - lx);
    const int32_t dy1 = d_fixed_to_int(gTestBBox[BOXTOP] - ly);
    const int32_t dx2 = d_fixed_to_int(x2 - lx);
    const int32_t dy2 = d_fixed_to_int(gTestBBox[BOXBOTTOM] - ly);

    const uint32_t side1 = (ldy * dx1 < dy1 * ldx);
    const uint32_t side2 = (ldy * dx2 < dy2 * ldx);

    // If the bounding box diagonal line points are on opposite sides of the line, then the box crosses the line
    return (side1 != side2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Assuming a collider intersects the given line, tells if the given line will potentially block - ignoring height differences.
// Returns 'false' if the line is considered blocking ignoring height differences.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PB_CheckLine(line_t& line) noexcept {
    // A 1 sided line cannot be crossed: register a collision against this
    if (!line.backsector)
        return false;

    // If not a projectile and the line is marked as explicitly blocking then block
    if ((gTestFlags & MF_MISSILE) == 0) {
        if (line.flags & (ML_BLOCKING | ML_BLOCKMONSTERS)) {
            return false;
        }
    }

    // PSX Doom addition: block always if the line blocks projectiles
    if (line.flags & ML_BLOCKPRJECTILE)
        return false;

    // Get the top and bottom height of the opening/gap and the lowest floor
    sector_t& fsec = *line.frontsector;
    sector_t& bsec = *line.backsector;

    const fixed_t openTop = std::min(fsec.ceilingheight, bsec.ceilingheight);
    const fixed_t openBottom = std::max(fsec.floorheight, bsec.floorheight);
    const fixed_t lowFloor = std::min(fsec.floorheight, bsec.floorheight);

    // Adjust the global low ceiling, high floor and lowest floor values
    if (openTop < gTestCeilingz) {
        gTestCeilingz = openTop;
        gpCeilingLine = &line;
    }

    if (openBottom > gTestFloorZ) {
        gTestFloorZ = openBottom;
    }

    if (lowFloor < gTestDropoffZ) {
        gTestDropoffZ = lowFloor;
    }

    // This line does not block, ignoring height differences.
    // This function does NOT check whether the thing can pass the line due to height differences!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does collision testing for the current thing doing collision testing against the given thing.
// Returns 'false' if there is a collision and hence a blockage.
// If a collision occurs the hit thing is saved in most cases, except where damage is not desired for missiles.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PB_CheckThing(mobj_t& mobj) noexcept {
    // If the thing is not solid you can't collide against it
    if ((mobj.flags & MF_SOLID) == 0)
        return true;

    // Get the thing which is doing the collision test and see if it is close enough to this thing.
    // If it isn't then we can early out here and return 'true' for no collision:
    mobj_t& baseThing = *gpBaseThing;
    const fixed_t totalRadius = mobj.radius + baseThing.radius;

    const fixed_t dx = std::abs(mobj.x - gTestX);
    const fixed_t dy = std::abs(mobj.y - gTestY);

    if ((dx >= totalRadius) || (dy >= totalRadius))
        return true;

    // The thing can't collide with itself
    if (&mobj == &baseThing)
        return true;

    // Check for a lost soul slamming into things
    const int32_t testFlags = gTestFlags;

    if (testFlags & MF_SKULLFLY) {
        gpHitThing = &mobj;
        return false;
    }

    // Missiles are special and are allowed to fly over and under things.
    // Most things have 'infinite' height in DOOM with regard to collision detection.
    if (testFlags & MF_MISSILE) {
        // Is the missile flying over this thing?
        if (baseThing.z > mobj.z + mobj.height)
            return true;

        // Is the missile flying under this thing?
        if (baseThing.z + baseThing.height < mobj.z)
            return true;

        // Is the missile hitting the same species that it came from?
        // PsyDoom: add a safety check to ensure the firer still exists; this is needed now that we have weak 'mobj_t' pointers.
        mobj_t* const pFirer = baseThing.target;

        #if PSYDOOM_FIX_UB
            const bool bFirerExists = (pFirer != nullptr);
        #else
            // The original code did not check if the firer existed ('target' field) because that pointer should always be set for a missile.
            // The firer could technically be destroyed and freed however (undefined behavior) as the original code did not have weak pointers.
            constexpr bool bFirerExists = true;
            ASSERT(pFirer);
        #endif

        if (bFirerExists) {
            const mobjtype_t sourceObjType = pFirer->type;

            if (sourceObjType == mobj.type) {
                // Colliding with the same species type: don't explode the missile if it's hitting the shooter of the missile
                if (&mobj == pFirer)
                    return true;

                // If it's hitting anything other than the player, explode the missile but do no damage (set no 'hit' thing).
                // Players can still damage each other with missiles however, hence the exception.
                if (sourceObjType != MT_PLAYER)
                    return false;
            }
        }

        // So long as the thing is shootable then the missile can hit it
        if (mobj.flags & MF_SHOOTABLE) {
            gpHitThing = &mobj;
            return false;
        }
    }

    // Non missile: the collider is colliding against this thing if it is solid.
    // Set no hit thing here however because this is not a missile that can potentially do damage.
    return ((mobj.flags & MF_SOLID) == 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check for potential collisions against all lines in the given blockmap cell, ignoring height differences.
// Returns 'false' if there is a definite collision, 'true' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PB_BlockLinesIterator(const int32_t x, const int32_t y) noexcept {
    // Get the line list for this blockmap cell
    const int16_t* pLineNum = (int16_t*)(gpBlockmapLump + gpBlockmap[y * gBlockmapWidth + x]);

    // Visit all lines in the cell, checking for intersection and potential collision.
    // Stop when there is a definite collision.
    line_t* const pLines = gpLines;

    for (; *pLineNum != -1; ++pLineNum) {
        line_t& line = pLines[*pLineNum];

        // Only check the line if not already checked this test
        if (line.validcount != gValidCount) {
            line.validcount = gValidCount;

            // If it's collided with and definitely blocking then stop
            if (PB_BoxCrossLine(line) && (!PB_CheckLine(line)))
                return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does collision detection against all things in the specified blockmap cell's linked list of things.
// Stops when a collision is detected and returns 'false', otherwise returns 'true' for no collision.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PB_BlockThingsIterator(const int32_t x, const int32_t y) noexcept {
    mobj_t* pmobj = gppBlockLinks[x + y * gBlockmapWidth];

    while (pmobj) {
        if (!PB_CheckThing(*pmobj))
            return false;

        pmobj = pmobj->bnext;
    }

    return true;
}
