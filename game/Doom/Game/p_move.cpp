#include "p_move.h"

#include "Asserts.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "info.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_setup.h"
#include "p_spec.h"
#include "PcPsx/Game.h"

#include <algorithm>

// PsyDoom: raise the maximum number of line specials that can be triggered per tick to support more complex levels
#if PSYDOOM_MODS
    static constexpr int32_t ORIG_MAX_CROSS_LINES = 8;
    static constexpr int32_t MAX_CROSS_LINES = 64;
#else
    static constexpr int32_t MAX_CROSS_LINES = 8;
#endif

bool        gbTryMove2;             // Whether the move attempt by 'P_TryMove2' was successful or not ('true' if move allowed)
mobj_t*     gpMoveThing;            // The thing collided with (for code doing interactions with the thing)
line_t*     gpBlockLine;            // The line collided with
fixed_t     gTmFloorZ;              // The Z value for the highest floor the collider is in contact with
fixed_t     gTmCeilingZ;            // The Z value for the lowest ceiling the collider is in contact with
fixed_t     gTmDropoffZ;            // The Z value for the lowest floor the collider is in contact with. Used by monsters so they don't walk off cliffs.
int32_t     gNumCrossCheckLines;    // How many lines to test for whether the thing crossed them or not: for determining when to trigger line specials
bool        gbFloatOk;              // P_TryMove2: if 'true' the up/down movement by floating monsters is allowed (there is vertical space to move)

static subsector_t*     gpNewSubsec;                            // Destination subsector for the current move: set by 'PM_CheckPosition'
static uint32_t         gTmFlags;                               // Flags for the thing being moved
static fixed_t          gTestTmBBox[4];                         // Bounding box for the current thing being collision tested. Set in 'PM_CheckPosition'.
static line_t*          gpCrossCheckLines[MAX_CROSS_LINES];     // Lines to test for whether the thing crossed them or not: for determining when to trigger line specials
static fixed_t          gOldX;                                  // P_TryMove2: position of the thing before it was moved: x
static fixed_t          gOldY;                                  // P_TryMove2: position of the thing before it was moved: y

// Not required externally: making private to this module
static void PM_UnsetThingPosition(mobj_t& thing) noexcept;
static void PM_SetThingPosition(mobj_t& mobj) noexcept;
static void PM_CheckPosition() noexcept;
static bool PM_BlockLinesIterator(const int32_t x, const int32_t y) noexcept;
static bool PM_BlockThingsIterator(const int32_t x, const int32_t y) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to do a move for the given thing, or just check if a move might be possible.
// If not checking and the move is allowed, then the thing will be moved to it's new position and blockmap references etc. updated.
// Note that when we are just checking if the move is allowed, height differences are IGNORED.
//
// Global inputs:
//  gpTryMoveThing          : The thing doing the move
//  gTryMoveX, gTryMoveY    : Position to move to
//
// Global outputs:
//  gbTryMove2      : Whether the move was allowed, 'true' if the move was possible
//  gpMoveThing     : The thing collided with
//  gpBlockLine     : The line collided with (only set for certain line collisions though)
//  gTmCeilingZ     : The Z value for the lowest ceiling touched
//  gTmFloorZ       : The Z value for the highest floor touched
//  gTmDropoffZ     : The Z value for the lowest floor touched
//------------------------------------------------------------------------------------------------------------------------------------------
void P_TryMove2() noexcept {
    // Grab the thing being moved and the position we're moving to
    mobj_t& tryMoveThing = *gpTryMoveThing;
    gOldX = tryMoveThing.x;
    gOldY = tryMoveThing.y;

    // Check if the move can be done and initially assume no movement (or floating) is allowed
    gbTryMove2 = false;
    gbFloatOk = false;

    PM_CheckPosition();

    // If only checking the move then stop here and clear that flag
    if (gbCheckPosOnly) {
        gbCheckPosOnly = false;
        return;
    }

    // If we can't move then stop here
    if (!gbTryMove2)
        return;

    // Do height checks for this move (unless noclip is active)
    if ((tryMoveThing.flags & MF_NOCLIP) == 0) {
        // Assume unable to move until we reach the bottom of the function (when doing height checks)
        gbTryMove2 = false;

        // If there is no room to fit then the move is NOT ok:
        if (tryMoveThing.height > gTmCeilingZ - gTmFloorZ)
            return;

        // If we can move up or down then floating movement (for floating monsters) is OK at this point
        gbFloatOk = true;

        // Checks for when not teleporting
        if ((tryMoveThing.flags & MF_TELEPORT) == 0) {
            // If the object must lower itself to fit then we can't do the move
            if (tryMoveThing.height > gTmCeilingZ - tryMoveThing.z)
                return;

            // If the vertical increase is too big of a step up then we can't make the move
            if (gTmFloorZ - tryMoveThing.z > 24 * FRACUNIT)
                return;
        }

        // If the thing cannot fall over ledges or doesn't float then don't allow it to stand over a ledge.
        // A ledge is deemed to be a fall of more than 24.0 units.
        const bool bCanGoOverLedges = (tryMoveThing.flags & (MF_DROPOFF | MF_FLOAT));

        if ((!bCanGoOverLedges) && (gTmFloorZ - gTmDropoffZ > 24 * FRACUNIT))
            return;
    }

    // Move is OK at this point: update the thing's location in the blockmap and sector thing lists
    PM_UnsetThingPosition(tryMoveThing);

    tryMoveThing.floorz = gTmFloorZ;
    tryMoveThing.ceilingz = gTmCeilingZ;
    tryMoveThing.x = gTryMoveX;
    tryMoveThing.y = gTryMoveY;

    PM_SetThingPosition(tryMoveThing);

    // Do line special triggering for monsters if they are not noclipping or teleporting
    if ((!tryMoveThing.player) && ((tryMoveThing.flags & (MF_NOCLIP | MF_TELEPORT)) == 0)) {
        // Process however many lines the thing crossed
        while (gNumCrossCheckLines > 0) {
            gNumCrossCheckLines--;
            line_t& line = *gpCrossCheckLines[gNumCrossCheckLines];

            // If the thing crossed this line then try to trigger its special
            const int32_t newLineSide = P_PointOnLineSide(tryMoveThing.x, tryMoveThing.y, line);
            const int32_t oldLineSide = P_PointOnLineSide(gOldX, gOldY, line);

            if (newLineSide != oldLineSide) {
                P_CrossSpecialLine(line, tryMoveThing);
            }
        }
    }

    // Move is OK! Note that we HAVE to set this here in all cases, regardless of whether height checks are done are not.
    // This is because 'P_CrossSpecialLine' might trigger move checks of it's own and override this global, due to teleportation and possibly other things.
    gbTryMove2 = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell what side of the given line a point is on: returns '0' if on the front side, '1' if on the back side.
// Same logic as 'R_PointOnSide' pretty much, but without the special optimized cases.
//
// PsyDoom: not compiling this as it is unused, and generates an unused warning.
//------------------------------------------------------------------------------------------------------------------------------------------
#if !PSYDOOM_MODS

static int32_t PM_PointOnLineSide(const fixed_t x, const fixed_t y, const line_t& line) noexcept {
    const int32_t dx = x - line.vertex1->x;
    const int32_t dy = y - line.vertex1->y;
    const int32_t lprod = d_fixed_to_int(dx) * d_fixed_to_int(line.dy);
    const int32_t rprod = d_fixed_to_int(dy) * d_fixed_to_int(line.dx);
    return (rprod >= lprod);
}

#endif  // !PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Unlinks the given thing from sector thing lists and the blockmap.
// Very similar to 'P_UnsetThingPosition' except the thing is always unlinked from sectors.
//------------------------------------------------------------------------------------------------------------------------------------------
static void PM_UnsetThingPosition(mobj_t& thing) noexcept {
    // Remove the thing from sector thing lists
    if (thing.snext) {
        thing.snext->sprev = thing.sprev;
    }

    if (thing.sprev) {
        thing.sprev->snext = thing.snext;
    } else {
        thing.subsector->sector->thinglist = thing.snext;
    }

    // Does this thing get added to the blockmap?
    // If so remove it from the blockmap.
    if ((thing.flags & MF_NOBLOCKMAP) == 0) {
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
                if ((blockx >= 0) && (blockx < gBlockmapWidth)) {
                    if ((blocky >= 0) && (blocky < gBlockmapHeight)) {
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
// Update the subsector for the thing. Also add the thing to sector and blockmap thing lists if applicable.
// Almost the exact same as 'P_SetThingPosition' except the subsector for the thing must be precomputed first by 'PM_CheckPosition'.
//------------------------------------------------------------------------------------------------------------------------------------------
static void PM_SetThingPosition(mobj_t& mobj) noexcept {
    // Note: this function needs the subsector precomputed externally
    subsector_t& newSubsec = *gpNewSubsec;
    mobj.subsector = &newSubsec;

    // Add the thing to sector thing lists, if the thing flags allow it
    if ((mobj.flags & MF_NOSECTOR) == 0) {
        sector_t& newSector = *newSubsec.sector;
        mobj.sprev = nullptr;
        mobj.snext = newSector.thinglist;

        if (newSector.thinglist) {
            newSector.thinglist->sprev = &mobj;
        }

        newSector.thinglist = &mobj;
    }

    // Add the thing to blockmap thing lists, if the thing flags allow it
    if ((mobj.flags & MF_NOBLOCKMAP) == 0) {
        const int32_t blockX = d_rshift<MAPBLOCKSHIFT>(mobj.x - gBlockmapOriginX);
        const int32_t blockY = d_rshift<MAPBLOCKSHIFT>(mobj.y - gBlockmapOriginY);

        // Make sure the thing is bounds for the blockmap: if not then just don't add it to the blockmap
        if ((blockX >= 0) && (blockY >= 0) && (blockX < gBlockmapWidth) && (blockY < gBlockmapHeight)) {
            mobj_t*& blockList = gppBlockLinks[blockY * gBlockmapWidth + blockX];
            mobj.bprev = nullptr;
            mobj.bnext = blockList;

            if (blockList) {
                blockList->bprev = &mobj;
            }

            blockList = &mobj;
        } else {
            mobj.bprev = nullptr;
            mobj.bnext = nullptr;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do a collision test for a thing against lines and other things, ignoring height differences.
// Sets 'gbTryMove2' to 'false' if there was a collision, 'true' if there was no collision when height differences are ignored.
// Note: height difference blocking logic is handled externally to this function.
//
// Global inputs:
//  gpTryMoveThing          : The thing doing the collision test
//  gTryMoveX, gTryMoveY    : Position to use for the thing for the collision test (can be set different to actual pos to test a move)
//
// Global outputs:
//  gTestTmBBox             : The bounding box for the thing
//  gpNewSubsec             : The new subsector the thing would be in at the given position
//  gpMoveThing             : The thing collided with
//  gpBlockLine             : The line collided with (only set for certain line collisions though)
//  gTmCeilingZ             : The Z value for the lowest ceiling touched
//  gTmFloorZ               : The Z value for the highest floor touched
//  gTmDropoffZ             : The Z value for the lowest floor touched
//  gNumCrossCheckLines     : The number of lines to test for the thing crossing (for specials/line activation)
//  gpCrossCheckLines       : The pointers to the lines to test for the thing crossing
//------------------------------------------------------------------------------------------------------------------------------------------
static void PM_CheckPosition() noexcept {
    // Save the flags for the thing being moved and precompute it's bounding box
    mobj_t& tryMoveThing = *gpTryMoveThing;

    gTmFlags = tryMoveThing.flags;
    gTestTmBBox[BOXTOP] = gTryMoveY + tryMoveThing.radius;
    gTestTmBBox[BOXBOTTOM] = gTryMoveY - tryMoveThing.radius;
    gTestTmBBox[BOXLEFT] = gTryMoveX - tryMoveThing.radius;
    gTestTmBBox[BOXRIGHT] = gTryMoveX + tryMoveThing.radius;

    // Precompute the subsector for the thing being moved
    subsector_t& newSubsec = *R_PointInSubsector(gTryMoveX, gTryMoveY);
    sector_t& newSector = *newSubsec.sector;
    gpNewSubsec = &newSubsec;

    // Initialize various movement checking variables
    gpMoveThing = nullptr;
    gpBlockLine = nullptr;
    gTmFloorZ = newSector.floorheight;          // Initial value: lowered as collision testing touches lines
    gTmCeilingZ = newSector.ceilingheight;      // Initial value: raised as collision testing touches lines
    gTmDropoffZ = newSector.floorheight;        // Initial value: Lowered as collision testing touches lines
    gNumCrossCheckLines = 0;

    // Prep for new collision tests: increment this marker
    gValidCount++;

    // If the thing is no-clipping then we can just exit and allow the move
    if (gTmFlags & MF_NOCLIP) {
        gbTryMove2 = true;
        return;
    }

    // Do collisions against things
    {
        // Compute the blockmap extents to check for collisions against other things and clamp to a valid range
        const int32_t bmapLx = std::max(d_rshift<MAPBLOCKSHIFT>(gTestTmBBox[BOXLEFT] - gBlockmapOriginX - MAXRADIUS), 0);
        const int32_t bmapRx = std::min(d_rshift<MAPBLOCKSHIFT>(gTestTmBBox[BOXRIGHT] - gBlockmapOriginX + MAXRADIUS), gBlockmapWidth - 1);
        const int32_t bmapTy = std::min(d_rshift<MAPBLOCKSHIFT>(gTestTmBBox[BOXTOP] - gBlockmapOriginY + MAXRADIUS), gBlockmapHeight - 1);
        const int32_t bmapBy = std::max(d_rshift<MAPBLOCKSHIFT>(gTestTmBBox[BOXBOTTOM] - gBlockmapOriginY - MAXRADIUS), 0);

        // Test against everything in this blockmap range; stop and set the result 'false' if a definite collision happens
        for (int32_t x = bmapLx; x <= bmapRx; ++x) {
            for (int32_t y = bmapBy; y <= bmapTy; ++y) {
                if (!PM_BlockThingsIterator(x, y)) {
                    gbTryMove2 = false;
                    return;
                }
            }
        }
    }

    // Do collision against lines
    {
        const int32_t bmapLx = std::max(d_rshift<MAPBLOCKSHIFT>(gTestTmBBox[BOXLEFT] - gBlockmapOriginX), 0);
        const int32_t bmapRx = std::min(d_rshift<MAPBLOCKSHIFT>(gTestTmBBox[BOXRIGHT] - gBlockmapOriginX), gBlockmapWidth - 1);
        const int32_t bmapTy = std::min(d_rshift<MAPBLOCKSHIFT>(gTestTmBBox[BOXTOP] - gBlockmapOriginY), gBlockmapHeight - 1);
        const int32_t bmapBy = std::max(d_rshift<MAPBLOCKSHIFT>(gTestTmBBox[BOXBOTTOM] - gBlockmapOriginY), 0);

        // Test against everything in this blockmap range; stop and set the result 'false' if a definite collision happens
        for (int32_t x = bmapLx; x <= bmapRx; ++x) {
            for (int32_t y = bmapBy; y <= bmapTy; ++y) {
                if (!PM_BlockLinesIterator(x, y)) {
                    gbTryMove2 = false;
                    return;
                }
            }
        }
    }

    // If we get to here then the collision test detected no collision: movement can be a success
    gbTryMove2 = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Test if 'gTestTmBBox' intersects the given line: returns 'true' if there is an intersection
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PM_BoxCrossLine(line_t& line) noexcept {
    // Check if the test bounding box is outside the bounding box of the line: if it is then early out
    const bool bTestBBOutsideLineBB = (
        (gTestTmBBox[BOXTOP] <= line.bbox[BOXBOTTOM]) ||
        (gTestTmBBox[BOXBOTTOM] >= line.bbox[BOXTOP]) ||
        (gTestTmBBox[BOXLEFT] >= line.bbox[BOXRIGHT]) ||
        (gTestTmBBox[BOXRIGHT] <= line.bbox[BOXLEFT])
    );

    if (bTestBBOutsideLineBB)
        return false;

    // Choose what line diagonal in the test box to test for crossing the line.
    // This code is trying to get a box diagonal that is as perpendicular to the line as possible.
    // Some lines for instance might run at 45 degrees and be parallel to the opposite box diagonal...
    fixed_t x1;
    fixed_t x2;

    if (line.slopetype == ST_POSITIVE) {
        x1 = gTestTmBBox[BOXLEFT];
        x2 = gTestTmBBox[BOXRIGHT];
    } else {
        x1 = gTestTmBBox[BOXRIGHT];
        x2 = gTestTmBBox[BOXLEFT];
    }

    // Use the cross product trick found in many functions such as 'R_PointOnSide' to determine what side of the line
    // both points of the test bounding box diagonal lie on.
    const fixed_t lx = line.vertex1->x;
    const fixed_t ly = line.vertex1->y;
    const int32_t ldx = d_fixed_to_int(line.dx);
    const int32_t ldy = d_fixed_to_int(line.dy);

    const int32_t dx1 = d_fixed_to_int(x1 - lx);
    const int32_t dy1 = d_fixed_to_int(gTestTmBBox[BOXTOP] - ly);
    const int32_t dx2 = d_fixed_to_int(x2 - lx);
    const int32_t dy2 = d_fixed_to_int(gTestTmBBox[BOXBOTTOM] - ly);

    const uint32_t side1 = (ldy * dx1 < dy1 * ldx);
    const uint32_t side2 = (ldy * dx2 < dy2 * ldx);

    // If the bounding box diagonal line points are on opposite sides of the line, then the box crosses the line
    return (side1 != side2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Assuming a collider intersects the given line, tells if the given line will potentially block - ignoring height differences.
// Returns 'false' if the line is considered blocking ignoring height differences.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PIT_CheckLine(line_t& line) noexcept {
    // A 1 sided line cannot be crossed: register a collision against this
    if (!line.backsector)
        return false;

    // If not a projectile and the line is marked as explicitly blocking then block
    mobj_t& tryMoveThing = *gpTryMoveThing;

    if ((tryMoveThing.flags & MF_MISSILE) == 0) {
        // If the line blocks everything then register a collision
        if (line.flags & ML_BLOCKING)
            return false;

        // If the line blocks monsters and the thing is not a player then block
        if ((line.flags & ML_BLOCKMONSTERS) && (!tryMoveThing.player))
            return false;
    }

    // Is the line adjoining a sector that is fully closed up (probably a door)? If so then it always blocks:
    sector_t& fsec = *line.frontsector;
    sector_t& bsec = *line.backsector;

    if ((fsec.floorheight == fsec.ceilingheight) || (bsec.floorheight == bsec.ceilingheight)) {
        gpBlockLine = &line;
        return false;
    }

    // Get the top and bottom height of the opening/gap and the lowest floor
    const fixed_t openTop = std::min(fsec.ceilingheight, bsec.ceilingheight);
    const fixed_t openBottom = std::max(fsec.floorheight, bsec.floorheight);
    const fixed_t lowFloor = std::min(fsec.floorheight, bsec.floorheight);

    // Adjust the global low ceiling, high floor and lowest floor values
    if (openTop < gTmCeilingZ) {
        gTmCeilingZ = openTop;
    }

    if (openBottom > gTmFloorZ) {
        gTmFloorZ = openBottom;
    }

    if (lowFloor < gTmDropoffZ) {
        gTmDropoffZ = lowFloor;
    }

    // PSX new addition: if the line has a special then save it for later testing to determine if the thing has crossed it and thus should trigger it.
    // This was added so that monsters could use teleporters again, since that ability was lost in the Jaguar version of the game.
    if (line.special) {
        #if PSYDOOM_MODS
            const int32_t maxCrossLines = (Game::gSettings.bUseNewMaxCrossLinesLimit) ? MAX_CROSS_LINES : ORIG_MAX_CROSS_LINES;
        #else
            const int32_t maxCrossLines = MAX_CROSS_LINES;
        #endif

        if (gNumCrossCheckLines < maxCrossLines) {
            gpCrossCheckLines[gNumCrossCheckLines] = &line;
            gNumCrossCheckLines++;
        }
    }

    // This line does not block, ignoring height differences.
    // This function does NOT check whether the thing can pass the line due to height differences!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check for a collision for the current thing being moved (in it's test position) against the input thing to this function.
// Returns 'false' if there was a collision and saves the thing globally, if it is to be interacted with (damage, pickup).
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PIT_CheckThing(mobj_t& mobj) noexcept {
    // If it's not a special, blocking or shootable then we can't collide with it
    if ((mobj.flags & (MF_SPECIAL | MF_SOLID | MF_SHOOTABLE)) == 0)
        return true;

    // The thing cannot collide with itself
    mobj_t& tryMoveThing = *gpTryMoveThing;

    if (&mobj == &tryMoveThing)
        return true;

    // See if the thing is within range: exit with no collision if it isn't
    const fixed_t totalRadius = mobj.radius + tryMoveThing.radius;
    const fixed_t dx = std::abs(mobj.x - gTryMoveX);
    const fixed_t dy = std::abs(mobj.y - gTryMoveY);

    if ((dx >= totalRadius) || (dy >= totalRadius))
        return true;

    // Is the thing being moved a skull which is slamming into this thing?
    if (tryMoveThing.flags & MF_SKULLFLY) {
        gpMoveThing = &mobj;
        return false;
    }

    // Special logic for missiles: unlike most other things, they can pass over/under things
    if (tryMoveThing.flags & MF_MISSILE) {
        // Is the missile flying above the thing? If so then no collision:
        if (tryMoveThing.z > mobj.z + mobj.height)
            return true;

        // Is the missile flying below the thing? If so then no collision:
        if (tryMoveThing.z + tryMoveThing.height < mobj.z)
            return true;

        // If we are colliding with the same species which fired the missile in most cases explode/collide the missile, but don't damage what was hit.
        // The firing thing is in the 'target' field for missiles.
        mobj_t& firingThing = *tryMoveThing.target;

        if (mobj.type == firingThing.type) {
            // Missiles don't collide with the things which fired them
            if (&mobj == &firingThing)
                return true;

            // Explode, but do no damage by just returning 'false' and not saving what was hit.
            // The exception to this is if the thing type is a player; players can splash damage other players with rockets...
            if (mobj.type != MT_PLAYER)
                return false;
        }

        // If the thing hit is shootable then save it for damage purposes and return 'false' for a collision
        if (mobj.flags & MF_SHOOTABLE) {
            gpMoveThing = &mobj;
            return false;
        }

        // Otherwise just explode the missile (but do no damage) if the thing hit was solid
        return ((mobj.flags & MF_SOLID) == 0);
    }

    // Are we colliding with an item that can be picked up?
    // If so then save it but return 'true' for no collision (pickups do not block).
    if ((mobj.flags & MF_SPECIAL) && (gTmFlags & MF_PICKUP)) {
        // PsyDoom: fix not being able to pickup items that SHOULD be possible to pickup due to other nearby/overlapping items which CAN'T be picked up.
        // Only set this thing as the current thing being touched if it's actually possible to pick it up.
        #if PSYDOOM_MODS
            const bool bCanPickup = ((!Game::gSettings.bUseItemPickupFix) || P_CanTouchSpecialThing(mobj, tryMoveThing));
        #else
            const bool bCanPickup = true;
        #endif

        if (bCanPickup) {
            gpMoveThing = &mobj;
            return true;
        }
    }

    // In all other cases there is a collision if the item collided with is solid
    return ((mobj.flags & MF_SOLID) == 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check for potential collisions against all lines in the given blockmap cell, ignoring height differences.
// Returns 'false' if there is a definite collision, 'true' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PM_BlockLinesIterator(const int32_t x, const int32_t y) noexcept {
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
            if (PM_BoxCrossLine(line) && (!PIT_CheckLine(line)))
                return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check for collisions against all things in the given blockmap cell. Returns 'true' if there were no collisions, 'false' otherwise.
// In some cases the thing collided with is saved in 'gpMoveThing' for futher interactions like pickups and damaging.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PM_BlockThingsIterator(const int32_t x, const int32_t y) noexcept {
    for (mobj_t* pmobj = gppBlockLinks[x + y * gBlockmapWidth]; pmobj; pmobj = pmobj->bnext) {
        if (!PIT_CheckThing(*pmobj))
            return false;
    }

    return true;
}
