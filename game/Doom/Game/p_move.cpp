#include "p_move.h"

#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "p_local.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_setup.h"
#include "p_spec.h"
#include <algorithm>

static constexpr int32_t MAX_CROSS_LINES = 8;

const VmPtr<bool32_t>       gbTryMove2(0x8007813C);             // Whether the move attempt by 'P_TryMove2' was successful or not ('true' if move allowed)
const VmPtr<VmPtr<mobj_t>>  gpMoveThing(0x800782C4);            // The thing collided with (for code doing interactions with the thing)
const VmPtr<VmPtr<line_t>>  gpBlockLine(0x80078248);            // The line collided with
const VmPtr<fixed_t>        gTmFloorZ(0x800781E8);              // The Z value for the highest floor the collider is in contact with
const VmPtr<fixed_t>        gTmCeilingZ(0x80077F04);            // The Z value for the lowest ceiling the collider is in contact with
const VmPtr<fixed_t>        gTmDropoffZ(0x80077F3C);            // The Z value for the lowest floor the collider is in contact with. Used by monsters so they don't walk off cliffs.
const VmPtr<int32_t>        gNumCrossCheckLines(0x800780C0);    // How many lines to test for whether the thing crossed them or not: for determining when to trigger line specials
const VmPtr<bool32_t>       gbFloatOk(0x8007807C);              // P_TryMove2: if 'true' the up/down movement by floating monsters is allowed (there is vertical space to move)

static const VmPtr<VmPtr<subsector_t>>              gpNewSubsec(0x800782BC);            // Destination subsector for the current move: set by 'PM_CheckPosition'
static const VmPtr<uint32_t>                        gTmFlags(0x80078078);               // Flags for the thing being moved
static const VmPtr<fixed_t[4]>                      gTestTmBBox(0x80097C10);            // Bounding box for the current thing being collision tested. Set in 'PM_CheckPosition'.
static const VmPtr<VmPtr<line_t>[MAX_CROSS_LINES]>  gpCrossCheckLines(0x800A8F28);      // Lines to test for whether the thing crossed them or not: for determining when to trigger line specials
static const VmPtr<fixed_t>                         gOldX(0x80078240);                  // P_TryMove2: position of the thing before it was moved: x
static const VmPtr<fixed_t>                         gOldY(0x80078244);                  // P_TryMove2: position of the thing before it was moved: y

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
    mobj_t& tryMoveThing = *gpTryMoveThing->get();
    *gOldX = tryMoveThing.x;
    *gOldY = tryMoveThing.y;
    
    // Check if the move can be done and initially assume no movement (or floating) is allowed
    *gbTryMove2 = false;
    *gbFloatOk = false;

    PM_CheckPosition();

    // If only checking the move then stop here and clear that flag
    if (*gbCheckPosOnly) {
        *gbCheckPosOnly = false;
        return;
    }

    // If we can't move then stop here
    if (!*gbTryMove2)
        return;

    // Do height checks for this move (unless noclip is active)
    if ((tryMoveThing.flags & MF_NOCLIP) == 0) {
        // Assume unable to move until we reach the bottom of the function (when doing height checks)
        *gbTryMove2 = false;

        // If there is no room to fit then the move is NOT ok:
        if (tryMoveThing.height > *gTmCeilingZ - *gTmFloorZ)
            return;

        // If we can move up or down then floating movement (for floating monsters) is OK at this point
        *gbFloatOk = true;

        // Checks for when not teleporting
        if ((tryMoveThing.flags & MF_TELEPORT) == 0) {
            // If the object must lower itself to fit then we can't do the move
            if (tryMoveThing.height > *gTmCeilingZ - tryMoveThing.z)
                return;

            // If the vertical increase is too big of a step up then we can't make the move
            if (*gTmFloorZ - tryMoveThing.z > 24 * FRACUNIT)
                return;
        }

        // If the thing cannot fall over ledges or doesn't float then don't allow it to stand over a ledge.
        // A ledge is deemed to be a fall of more than 24.0 units.
        const bool bCanGoOverLedges = (tryMoveThing.flags & (MF_DROPOFF | MF_FLOAT));

        if ((!bCanGoOverLedges) && (*gTmFloorZ - *gTmDropoffZ > 24 * FRACUNIT))
            return;
    }
    
    // Move is OK at this point: update the thing's location in the blockmap and sector thing lists
    PM_UnsetThingPosition(tryMoveThing);
    
    tryMoveThing.floorz = *gTmFloorZ;
    tryMoveThing.ceilingz = *gTmCeilingZ;
    tryMoveThing.x = *gTryMoveX;
    tryMoveThing.y = *gTryMoveY;

    PM_SetThingPosition(tryMoveThing);

    // Do line special triggering for monsters if they are not noclipping or teleporting
    if ((!tryMoveThing.player.get()) && ((tryMoveThing.flags & (MF_NOCLIP | MF_TELEPORT)) == 0)) {
        // Process however many lines the thing crossed
        while (*gNumCrossCheckLines > 0) {
            *gNumCrossCheckLines -= 1;
            line_t& line = *gpCrossCheckLines[*gNumCrossCheckLines];

            // If the thing crossed this line then try to trigger its special
            const int32_t newLineSide = P_PointOnLineSide(tryMoveThing.x, tryMoveThing.y, line);
            const int32_t oldLineSide = P_PointOnLineSide(*gOldX, *gOldY, line);

            if (newLineSide != oldLineSide) {
                P_CrossSpecialLine(line, tryMoveThing);
            }
        }
    }

    // Move is OK! Note that we HAVE to set this here in all cases, regardless of whether height checks are done are not.
    // This is because 'P_CrossSpecialLine' might trigger move checks of it's own and override this global, due to teleportation and possibly other things.
    *gbTryMove2 = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell what side of the given line a point is on: returns '0' if on the front side, '1' if on the back side.
// Same logic as 'R_PointOnSide' pretty much, but without the special optimized cases.
//
// PC-PSX: not compiling this as it is unused, and generates an unused warning.
//------------------------------------------------------------------------------------------------------------------------------------------
#if !PC_PSX_DOOM_MODS

static int32_t PM_PointOnLineSide(const fixed_t x, const fixed_t y, const line_t& line) noexcept {
    const int32_t dx = x - line.vertex1->x;
    const int32_t dy = y - line.vertex1->y;
    const int32_t lprod = (dx >> FRACBITS) * (line.dy >> FRACBITS);
    const int32_t rprod = (dy >> FRACBITS) * (line.dx >> FRACBITS);
    return (rprod >= lprod);
}

#endif  // !PC_PSX_DOOM_MODS

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
            const int32_t blockx = (thing.x - *gBlockmapOriginX) >> MAPBLOCKSHIFT;
            const int32_t blocky = (thing.y - *gBlockmapOriginY) >> MAPBLOCKSHIFT;

            // PC-PSX: prevent buffer overflow if the map object is out of bounds.
            // This is part of the fix for the famous 'linedef deletion' bug.
            #if PC_PSX_DOOM_MODS
                if ((blockx >= 0) && (blockx < *gBlockmapWidth)) {
                    if ((blocky >= 0) && (blocky < *gBlockmapHeight)) {
                        (*gppBlockLinks)[blocky * (*gBlockmapWidth) + blockx] = thing.bnext;
                    }
                }
            #else
                (*gppBlockLinks)[blocky * (*gBlockmapWidth) + blockx] = thing.bnext;
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
    subsector_t& newSubsec = *gpNewSubsec->get();
    mobj.subsector = &newSubsec;
    
    // Add the thing to sector thing lists, if the thing flags allow it
    if ((mobj.flags & MF_NOSECTOR) == 0) {
        sector_t& newSector = *newSubsec.sector.get();
        mobj.sprev = nullptr;
        mobj.snext = newSector.thinglist.get();
        
        if (newSector.thinglist) {
            newSector.thinglist->sprev = &mobj;
        }

        newSector.thinglist = &mobj;
    }

    // Add the thing to blockmap thing lists, if the thing flags allow it
    if ((mobj.flags & MF_NOBLOCKMAP) == 0) {
        const int32_t blockX = (mobj.x - *gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t blockY = (mobj.y - *gBlockmapOriginY) >> MAPBLOCKSHIFT;

        // Make sure the thing is bounds for the blockmap: if not then just don't add it to the blockmap
        if ((blockX >= 0) && (blockY >= 0) && (blockX < *gBlockmapWidth) && (blockY < *gBlockmapHeight)) {
            VmPtr<mobj_t>& blockList = gppBlockLinks->get()[blockY * (*gBlockmapWidth) + blockX];
            mobj.bprev = nullptr;
            mobj.bnext = blockList.get();

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
    mobj_t& tryMoveThing = *gpTryMoveThing->get();

    *gTmFlags = tryMoveThing.flags;
    gTestTmBBox[0] = *gTryMoveY + tryMoveThing.radius;
    gTestTmBBox[1] = *gTryMoveY - tryMoveThing.radius;
    gTestTmBBox[3] = *gTryMoveX + tryMoveThing.radius;
    gTestTmBBox[2] = *gTryMoveX - tryMoveThing.radius;

    // Precompute the subsector for the thing being moved
    subsector_t& newSubsec = *R_PointInSubsector(*gTryMoveX, *gTryMoveY);
    sector_t& newSector = *newSubsec.sector;
    *gpNewSubsec = &newSubsec;

    // Initialize various movement checking variables
    *gpMoveThing = nullptr;
    *gpBlockLine = nullptr;
    *gTmFloorZ = newSector.floorheight;         // Initial value: lowered as collision testing touches lines
    *gTmCeilingZ = newSector.ceilingheight;     // Initial value: raised as collision testing touches lines
    *gTmDropoffZ = newSector.floorheight;       // Initial value: Lowered as collision testing touches lines
    *gNumCrossCheckLines = 0;

    // Prep for new collision tests: increment this marker
    *gValidCount += 1;

    // If the thing is no-clipping then we can just exit and allow the move
    if (*gTmFlags & MF_NOCLIP) {
        *gbTryMove2 = true;
        return;
    }

    // Do collisions against things
    {
        // Compute the blockmap extents to check for collisions against other things and clamp to a valid range
        const int32_t bmapLx = std::max((gTestTmBBox[BOXLEFT] - *gBlockmapOriginX - MAXRADIUS) >> MAPBLOCKSHIFT, 0);
        const int32_t bmapRx = std::min((gTestTmBBox[BOXRIGHT] - *gBlockmapOriginX + MAXRADIUS) >> MAPBLOCKSHIFT, *gBlockmapWidth - 1);
        const int32_t bmapTy = std::min((gTestTmBBox[BOXTOP] - *gBlockmapOriginY + MAXRADIUS) >> MAPBLOCKSHIFT, *gBlockmapHeight - 1);
        const int32_t bmapBy = std::max((gTestTmBBox[BOXBOTTOM] - *gBlockmapOriginY - MAXRADIUS) >> MAPBLOCKSHIFT, 0);
    
        // Test against everything in this blockmap range; stop and set the result 'false' if a definite collision happens
        for (int32_t x = bmapLx; x <= bmapRx; ++x) {
            for (int32_t y = bmapBy; y <= bmapTy; ++y) {
                if (!PM_BlockThingsIterator(x, y)) {
                    *gbTryMove2 = false;
                    return;
                }
            }
        }
    }
    
    // Do collision against lines
    {
        const int32_t bmapLx = std::max((gTestTmBBox[BOXLEFT] - *gBlockmapOriginX) >> MAPBLOCKSHIFT, 0);
        const int32_t bmapRx = std::min((gTestTmBBox[BOXRIGHT] - *gBlockmapOriginX) >> MAPBLOCKSHIFT, *gBlockmapWidth - 1);
        const int32_t bmapTy = std::min((gTestTmBBox[BOXTOP] - *gBlockmapOriginY) >> MAPBLOCKSHIFT, *gBlockmapHeight - 1);
        const int32_t bmapBy = std::max((gTestTmBBox[BOXBOTTOM] - *gBlockmapOriginY) >> MAPBLOCKSHIFT, 0);

        // Test against everything in this blockmap range; stop and set the result 'false' if a definite collision happens
        for (int32_t x = bmapLx; x <= bmapRx; ++x) {
            for (int32_t y = bmapBy; y <= bmapTy; ++y) {
                if (!PM_BlockLinesIterator(x, y)) {
                    *gbTryMove2 = false;
                    return;
                }
            }
        }
    }

    // If we get to here then the collision test detected no collision: movement can be a success
    *gbTryMove2 = true;
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
    const int32_t ldx = line.dx >> FRACBITS;
    const int32_t ldy = line.dy >> FRACBITS;

    const int32_t dx1 = (x1 - lx) >> FRACBITS;
    const int32_t dy1 = (gTestTmBBox[BOXTOP] - ly) >> FRACBITS;
    const int32_t dx2 = (x2 - lx) >> FRACBITS;
    const int32_t dy2 = (gTestTmBBox[BOXBOTTOM] - ly) >> FRACBITS;

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
    mobj_t& tryMoveThing = *gpTryMoveThing->get();

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
        *gpBlockLine = &line;
        return false;
    }

    // Get the top and bottom height of the opening/gap and the lowest floor
    const fixed_t openTop = std::min(fsec.ceilingheight, bsec.ceilingheight);
    const fixed_t openBottom = std::max(fsec.floorheight, bsec.floorheight);
    const fixed_t lowFloor = std::min(fsec.floorheight, bsec.floorheight);

    // Adjust the global low ceiling, high floor and lowest floor values
    if (openTop < *gTmCeilingZ) {
        *gTmCeilingZ = openTop;
    }

    if (openBottom > *gTmFloorZ) {
        *gTmFloorZ = openBottom;
    }

    if (lowFloor < *gTmDropoffZ) {
        *gTmDropoffZ = lowFloor;
    }

    // PSX new addition: if the line has a special then save it for later testing to determine if the thing has crossed it and thus should trigger it.
    // This was added so that monsters could use teleporters again, since that ability was lost in the Jaguar version of the game.
    if (line.special) {
        if (*gNumCrossCheckLines < MAX_CROSS_LINES) {
            gpCrossCheckLines[*gNumCrossCheckLines] = &line;
            *gNumCrossCheckLines += 1;
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
    mobj_t& tryMoveThing = *gpTryMoveThing->get();

    if (&mobj == &tryMoveThing)
        return true;

    // See if the thing is within range: exit with no collision if it isn't
    const fixed_t totalRadius = mobj.radius + tryMoveThing.radius;
    const fixed_t dx = std::abs(mobj.x - *gTryMoveX);
    const fixed_t dy = std::abs(mobj.y - *gTryMoveY);

    if ((dx >= totalRadius) || (dy >= totalRadius))
        return true;
    
    // Is the thing being moved a skull which is slamming into this thing?
    if (tryMoveThing.flags & MF_SKULLFLY) {
        *gpMoveThing = &mobj;
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
        mobj_t& firingThing = *tryMoveThing.target.get();

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
            *gpMoveThing = &mobj;
            return false;
        }

        // Otherwise just explode the missile (but do no damage) if the thing hit was solid
        return ((mobj.flags & MF_SOLID) == 0);
    }
    
    // Are we colliding with an item that can be picked up?
    // If so then save it but return 'true' for no collision (pickups do not block).
    if ((mobj.flags & MF_SPECIAL) && (*gTmFlags & MF_PICKUP)) {
        *gpMoveThing = &mobj;
        return true;
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
    const int16_t* pLineNum = (int16_t*)(gpBlockmapLump->get() + gpBlockmap->get()[y * (*gBlockmapWidth) + x]);

    // Visit all lines in the cell, checking for intersection and potential collision.
    // Stop when there is a definite collision.
    line_t* const pLines = gpLines->get();

    for (; *pLineNum != -1; ++pLineNum) {
        line_t& line = pLines[*pLineNum];

        // Only check the line if not already checked this test
        if (line.validcount != *gValidCount) {
            line.validcount = *gValidCount;
            
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
    for (mobj_t* pmobj = gppBlockLinks->get()[x + y * (*gBlockmapWidth)].get(); pmobj; pmobj = pmobj->bnext.get()) {
        if (!PIT_CheckThing(*pmobj))
            return false;
    }

    return true;
}
