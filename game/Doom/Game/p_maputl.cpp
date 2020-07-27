#include "p_maputl.h"

#include "Doom/Base/m_fixed.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "p_local.h"
#include "p_setup.h"
#include "PcPsx/Assert.h"

#include <algorithm>

fixed_t gOpenBottom;    // Line opening (floor/ceiling gap) info: bottom Z value of the opening
fixed_t gOpenTop;       // Line opening (floor/ceiling gap) info: top Z value of the opening
fixed_t gOpenRange;     // Line opening (floor/ceiling gap) info: Z size of the opening
fixed_t gLowFloor;      // Line opening (floor/ceiling gap) info: the lowest (front/back sector) floor of the opening

//------------------------------------------------------------------------------------------------------------------------------------------
// Gives a cheap approximate/estimated length for the given vector
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_AproxDistance(const fixed_t dx, const fixed_t dy) noexcept {
    const fixed_t udx = std::abs(dx);
    const fixed_t udy = std::abs(dy);
    return udx + udy - (std::min(udx, udy) >> 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell what side of a line a point is on.
// Returns '0' if on the front side, '1' if on the back side.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t P_PointOnLineSide(const fixed_t x, const fixed_t y, const line_t& line) noexcept {
    // Easy case: north/south wall.
    // Check what side of the line we are on, then return the answer based on what way the wall is winding.
    if (line.dx == 0) {
        if (x > line.vertex1->x) {
            return (line.dy < 0);
        } else {
            return (line.dy > 0);
        }
    }

    // Easy case: east/west wall.
    // Check what side of the line we are on, then return the answer based on what way the wall is winding.
    if (line.dy == 0) {
        if (y > line.vertex1->y) {
            return (line.dx > 0);
        } else {
            return (line.dx < 0);
        }
    }

    // Harder case: use the same cross product trick found in places like 'R_PointOnSide' to tell what side of the line we are on
    const int32_t vdx = (x - line.vertex1->x) >> FRACBITS;
    const int32_t vdy = (y - line.vertex1->y) >> FRACBITS;
    const int32_t ldx = line.dx >> FRACBITS;
    const int32_t ldy = line.dy >> FRACBITS;

    return (vdx * ldy <= vdy * ldx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell what side of a dividing line a point is on.
// Returns '0' if on the front side, '1' if on the back side.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t P_PointOnDivlineSide(const fixed_t x, const fixed_t y, const divline_t& divline) noexcept {
    // Easy case: north/south wall.
    // Check what side of the line we are on, then return the answer based on what way the wall is winding.
    if (divline.dx == 0) {
        if (x > divline.x) {
            return (divline.dy < 0);
        } else {
            return (divline.dy > 0);
        }
    }

    // Easy case: east/west wall.
    // Check what side of the line we are on, then return the answer based on what way the wall is winding.
    if (divline.dy == 0) {
        if (y > divline.y) {
            return (divline.dx > 0);
        } else {
            return (divline.dx < 0);
        }
    }

    // Try to decide based on the sign bits if there is a sign difference between the left and right products.
    // The comparsion below is ultimately 'lprod <= rprod' and if we can tell just by examining signs, do that now:
    const fixed_t vdx = x - divline.x;
    const fixed_t vdy = y - divline.y;

    if ((divline.dy ^ divline.dx ^ vdx ^ vdy) < 0) {
        // Sign difference: use the left product sign to decide - it must be the opposite of the right product sign (see below)
        return ((vdx ^ divline.dy) < 0);
    }

    // Finally fallback to using the 2d vector cross product to decide what side we are on
    const fixed_t lprod = FixedMul(vdx >> 8, divline.dy >> 8);
    const fixed_t rprod = FixedMul(vdy >> 8, divline.dx >> 8);
    return (lprod <= rprod);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: populate a divline struct from the given line
//------------------------------------------------------------------------------------------------------------------------------------------
void P_MakeDivline(const line_t& line, divline_t& divline) noexcept {
    divline.x = line.vertex1->x;
    divline.y = line.vertex1->y;
    divline.dx = line.dx;
    divline.dy = line.dy;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: figures out the size of the opening (floor/ceiling gap) for the given line and saves the info to globals.
// Also saves the lowest floor of the opening.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_LineOpening(const line_t& line) noexcept {
    // If the line is 1-sided (a solid wall) then there is no opening
    if (line.sidenum[1] == -1) {
        gOpenRange = 0;
        return;
    }

    // Otherwise compute the opening values
    const sector_t& fsec = *line.frontsector;
    const sector_t& bsec = *line.backsector;

    gOpenTop = std::min(fsec.ceilingheight, bsec.ceilingheight);
    gOpenBottom = std::max(fsec.floorheight, bsec.floorheight);
    gLowFloor = std::min(fsec.floorheight, bsec.floorheight);
    gOpenRange = gOpenTop - gOpenBottom;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unlinks the given thing from sector thing lists and the blockmap
//------------------------------------------------------------------------------------------------------------------------------------------
void P_UnsetThingPosition(mobj_t& thing) noexcept {
    // Does this thing get assigned to sector thing lists?
    // If so remove it from the sector thing list.
    if ((thing.flags & MF_NOSECTOR) == 0) {
        if (thing.snext) {
            thing.snext->sprev = thing.sprev;
        }
        
        if (thing.sprev) {
            thing.sprev->snext = thing.snext;
        } else {
            thing.subsector->sector->thinglist = thing.snext;
        }
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
            const int32_t blockx = (thing.x - gBlockmapOriginX) >> MAPBLOCKSHIFT;
            const int32_t blocky = (thing.y - gBlockmapOriginY) >> MAPBLOCKSHIFT;

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
// Update the subsector for the thing.
// Also add the thing to sector and blockmap thing lists if applicable.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SetThingPosition(mobj_t& mobj) noexcept {
    // First update what subsector the thing is in
    subsector_t* const pSubsec = R_PointInSubsector(mobj.x, mobj.y);
    mobj.subsector = pSubsec;

    // Add the thing to sector thing lists, if the thing flags allow it
    if ((mobj.flags & MF_NOSECTOR) == 0) {
        sector_t& sec = *pSubsec->sector;
        mobj.sprev = nullptr;
        mobj.snext = sec.thinglist;
    
        if (sec.thinglist) {
            sec.thinglist->sprev = &mobj;
        }

        sec.thinglist = &mobj;
    }

    // Add the thing to blockmap thing lists, if the thing flags allow it
    if ((mobj.flags & MF_NOBLOCKMAP) == 0) {
        const int32_t blockX = (mobj.x - gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t blockY = (mobj.y - gBlockmapOriginY) >> MAPBLOCKSHIFT;

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
// Visit all unvisited lines in the specified blockmap cell, calling the given function for each line in the cell.
// The called function can abort iteration by returning 'false'.
// This function returns 'false' if iteration was aborted.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_BlockLinesIterator(const int32_t x, const int32_t y, bool (*pFunc)(line_t&)) noexcept {
    // If the blockmap block is out of range then there is nothing to do
    if ((x < 0) || (y < 0) || (x >= gBlockmapWidth) || (y >= gBlockmapHeight))
        return true;
    
    // Get the line list offset for this blockmap cell in the blockmap lump.
    // Note that the offset to the line list is in terms of 16-bit words, not bytes.
    const uint16_t lineListOffset = gpBlockmap[x + y * gBlockmapWidth];

    // Visit all the lines in the block unless the callee asks to quit
    const int16_t* pLineIdx = (int16_t*)(gpBlockmapLump + lineListOffset);
    line_t* const pLines = gpLines;

    while (*pLineIdx != -1) {
        line_t& line = pLines[*pLineIdx];
        pLineIdx++;

        // Only visit the line if not already visited for this set of checks
        if (line.validcount != gValidCount) {
            line.validcount = gValidCount;

            // Call the function and stop if requested
            if (!pFunc(line))
                return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Visit all things in the specified blockmap cell, calling the given function for each line in the cell.
// The called function can abort iteration by returning 'false'.
// This function returns 'false' if iteration was aborted.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_BlockThingsIterator(const int32_t x, const int32_t y, bool (*pFunc)(mobj_t&)) noexcept {
    // If the blockmap block is out of range then there is nothing to do
    if ((x < 0) || (y < 0) || (x >= gBlockmapWidth) || (y >= gBlockmapHeight))
        return true;
    
    // Visit all of the things in this blockmap cell unless the callee asks to quit
    mobj_t* pmobj = gppBlockLinks[x + y * gBlockmapWidth];
    
    while (pmobj) {
        // Call the function and stop if requested
        if (!pFunc(*pmobj))
            return false;
        
        pmobj = pmobj->bnext;
    }
    
    return true;
}
