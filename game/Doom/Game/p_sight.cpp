#include "p_sight.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "p_setup.h"
#include "p_shoot.h"
#include "p_tick.h"

#include <algorithm>

static fixed_t      gSightZStart;       // Z position of thing looking
static fixed_t      gTopSlope;          // Maximum/top unblocked viewing slope (clipped against upper walls)
static fixed_t      gBottomSlope;       // Minimum/bottom unblocked viewing slope (clipped against lower walls)
static divline_t    gSTrace;            // The start point and vector for sight checking
static fixed_t      gT2x;               // End point for sight checking: x
static fixed_t      gT2y;               // End point for sight checking: y
static int32_t      gT1xs;              // Sight line start, whole coords: x
static int32_t      gT1ys;              // Sight line start, whole coords: y
static int32_t      gT2xs;              // Sight line end, whole coords: x
static int32_t      gT2ys;              // Sight line end, whole coords: y

//------------------------------------------------------------------------------------------------------------------------------------------
// Updates target visibility checking for all map objects that are due an update
//------------------------------------------------------------------------------------------------------------------------------------------
void P_CheckSights() noexcept {
    for (mobj_t* pmobj = gMObjHead.next; pmobj != &gMObjHead; pmobj = pmobj->next) {
        // Must be killable (enemy) to do sight checking
        if ((pmobj->flags & MF_COUNTKILL) == 0)
            continue;

        // Must be about to change states for up-to-date sight info to be useful
        if (pmobj->tics == 1) {
            // See if we can see the target - if any.
            // Add or remove the visibility flag based on this:
            if (pmobj->target && P_CheckSight(*pmobj, *pmobj->target)) {
                pmobj->flags |= MF_SEETARGET;
            } else {
                pmobj->flags &= (~MF_SEETARGET);    // No longer can see target
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if 'mobj1' can see 'mobj2'. Returns 'true' if that is the case.
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_CheckSight(mobj_t& mobj1, mobj_t& mobj2) noexcept {
    // Figure out the reject matrix entry to lookup
    const int32_t secnum1 = (int32_t)(mobj1.subsector->sector - gpSectors);
    const int32_t secnum2 = (int32_t)(mobj2.subsector->sector - gpSectors);
    const int32_t rejectMapEntry = secnum1 * gNumSectors + secnum2;

    // Lookup the reject matrix to see if these two sectors can possibly see each other.
    // If they can't then we can early out here.
    const int32_t rejectMapByte = rejectMapEntry / 8;
    const int32_t rejectMapBit = rejectMapEntry & 7;

    if ((gpRejectMatrix[rejectMapByte] & (1 << rejectMapBit)) != 0)
        return false;
    
    // Store the start and end points of the sight line.
    // Note that the coordinates are truncated to be on odd integer coordinates.
    // Not sure why this is done, or what it's trying to avoid - it's in the 3DO and Jag Doom sources but not explained.
    const int32_t COORD_MASK = 0xFFFE0000;
    gSTrace.x = (mobj1.x & COORD_MASK) | FRACUNIT;
    gSTrace.y = (mobj1.y & COORD_MASK) | FRACUNIT;
    gT2x = (mobj2.x & COORD_MASK) | FRACUNIT;
    gT2y = (mobj2.y & COORD_MASK) | FRACUNIT;

    // Precalculate the vector for the sight line
    gSTrace.dx = gT2x - gSTrace.x;
    gSTrace.dy = gT2y - gSTrace.y;
    
    // Precalculate the truncated start and end points for the sight line for later use
    gT1xs = gSTrace.x >> FRACBITS;
    gT1ys = gSTrace.y >> FRACBITS;
    gT2xs = gT2x >> FRACBITS;
    gT2ys = gT2y >> FRACBITS;

    // This is how high the sight point is at (eyeball level -1/4 height down from the top)
    const fixed_t sightZStart = mobj1.z + mobj1.height - (mobj1.height >> 2);
    gSightZStart = sightZStart;
    
    // Figure out the initial top and bottom slopes for the the vertical sight range
    gTopSlope = mobj2.z + mobj2.height - sightZStart;
    gBottomSlope = mobj2.z - sightZStart;
    
    // Doing a new raycast so update the visitation mark which tells us if stuff has already been processed
    gValidCount++;

    // Do a raycast against the BSP tree and return if sight is unobstructed.
    // Also narrows the vertical sight range with each lower and upper wall encountered.
    return PS_CrossBSPNode(gNumBspNodes - 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Intersects the given line against the current sight line and returns the fraction of intersection along the sight line.
// When the intersect ratio is > 0.0 and < 1.0 then there is a valid intersection with the sight line, otherwise there is no
// intersection or the intersection occurs beyond the range of the line.
//------------------------------------------------------------------------------------------------------------------------------------------
static fixed_t PS_SightCrossLine(line_t& line) noexcept {
    // Get the integer coordinates of the line and the sight line
    const int32_t lineX1 = line.vertex1->x >> FRACBITS;
    const int32_t lineY1 = line.vertex1->y >> FRACBITS;
    const int32_t lineX2 = line.vertex2->x >> FRACBITS;
    const int32_t lineY2 = line.vertex2->y >> FRACBITS;
    const int32_t sightX1 = gT1xs;
    const int32_t sightY1 = gT1ys;
    const int32_t sightX2 = gT2xs;
    const int32_t sightY2 = gT2ys;

    // Compute which sides of the sight line the line points are on.
    // Use the same cross product trick found in 'PA_DivlineSide' and 'R_PointOnSide'.
    {
        const int32_t sightDx = sightX2 - sightX1;
        const int32_t sightDy = sightY2 - sightY1;
        const int32_t side1 = ((lineY1 - sightY1) * sightDx > (lineX1 - sightX1) * sightDy);
        const int32_t side2 = ((lineY2 - sightY1) * sightDx > (lineX2 - sightX1) * sightDy);

        // If both line points are on the same side of the sight line then there is no intersection
        if (side1 == side2)
            return -1;
    }
    
    // Compute the normal vector for the line
    const int32_t lineNx = lineY1 - lineY2;
    const int32_t lineNy = lineX2 - lineX1;

    // Compute the distance magnitude of the sight points from the line using vector dot product with the normal.
    // Note that after these multiplies we can reinterpret the results as fixed point numbers for the intersect ratio calculation.
    // The relative ratios are what is important, not the actual numbers.
    const fixed_t dist1 = (lineX1 - sightX1) * lineNx + (lineY1 - sightY1) * lineNy;
    const fixed_t dist2 = (lineX1 - sightX2) * lineNx + (lineY1 - sightY2) * lineNy;

    // Use the distance magnitudes to figure out the intersection ratio.
    // Note: distance sign correction is being done here also, so the distance on both sides of the line has the same sign.
    const fixed_t totalDist = dist1 - dist2;
    return FixedDiv(dist1, totalDist);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks the current sight line against lines in the given subsector.
// Returns 'true' if the sight line is unobstructed, returns 'false' otherwise.
// This function also updates/narrows the allowed vertical view range.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PS_CrossSubsector(subsector_t& subsec) noexcept {
    // Check the sight line against the lines for all segs in the subsector
    const int32_t numSegs = subsec.numsegs;
    seg_t* const pSegs = &gpSegs[subsec.firstseg];

    for (int32_t segIdx = 0; segIdx < numSegs; ++segIdx) {
        seg_t& seg = pSegs[segIdx];
        line_t& line = *seg.linedef;

        // Skip past this seg's line if we've already done it this sight check.
        // Multiple segs might reference the same line, so this saves redundant work:
        if (line.validcount == gValidCount)
            continue;
        
        // Don't check the line again until the next sight check
        line.validcount = gValidCount;

        // If the sight line does not intersect along the actual line points then ignore.
        // Not sure where the magics here came from, probably through hacking/experimentation?
        const fixed_t intersectFrac = PS_SightCrossLine(line);

        if ((intersectFrac < 4) || (intersectFrac > FRACUNIT))
            continue;

        // The sight line crosses the subsector line.
        // If the line is impassible (no back sector) then it blocks sight:
        if (!line.backsector)
            return false;
        
        // If there is no height difference between the front and back sectors then the line can't block.
        // In this case it has no upper or lower walls:
        sector_t& bsec = *line.backsector;
        sector_t& fsec = *line.frontsector;

        if ((fsec.floorheight == bsec.floorheight) && (fsec.ceilingheight == bsec.ceilingheight))
            continue;

        // If there is no vertical gap in the two sided line then sight is completely obscured
        const fixed_t lowestCeil = std::min(fsec.ceilingheight, bsec.ceilingheight);
        const fixed_t highestFloor = std::max(fsec.floorheight, bsec.floorheight);
        
        if (lowestCeil <= highestFloor)
            return false;
        
        // Narrow the allowed vertical sight range: against bottom wall
        if (fsec.floorheight != bsec.floorheight) {
            const fixed_t dz = highestFloor - gSightZStart;
            const int32_t slope = ((dz << 6) / (intersectFrac >> 2)) << 8;      // Note: chops off the low 8 bits of computed intersect slope

            if (slope > gBottomSlope) {
                gBottomSlope = slope;
            }
        }

        // Narrow the allowed vertical sight range: against top wall
        if (fsec.ceilingheight != bsec.ceilingheight) {
            const fixed_t dz = lowestCeil - gSightZStart;
            const int32_t slope = ((dz << 6) / (intersectFrac >> 2)) << 8;      // Note: chops off the low 8 bits of computed intersect slope

            if (slope < gTopSlope) {
                gTopSlope = slope;
            }
        }

        // If the allowed vertical sight range has become completely closed then sight is blocked
        if (gTopSlope <= gBottomSlope)
            return false;
    }

    // The sight line does not cross any blocking lines in the subsector
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recursive sight checking: tells if the 'gSTrace' line is blocked by the BSP tree halfspace represented by the given node.
// Returns 'true' if the sight line is unobstructed.
//------------------------------------------------------------------------------------------------------------------------------------------
bool PS_CrossBSPNode(const int32_t nodeNum) noexcept {
    // Is this bsp node actually a subsector? (leaf node) If so then do sight checks against that:
    if (nodeNum & NF_SUBSECTOR) {
        const int32_t subsecNum = nodeNum & (~NF_SUBSECTOR);
        
        if (subsecNum < gNumSubsectors) {
            return PS_CrossSubsector(gpSubsectors[subsecNum]);
        } else {
            I_Error("PS_CrossSubsector: ss %i with numss = %i", subsecNum, gNumSubsectors);     // Bad subsector number!
            return false;
        }
    }

    // See what side of the bsp split the point is on: will check to see if the sight line is blocked by that half-space first
    node_t& bspNode = gpBspNodes[nodeNum];
    const int32_t sideNum = PA_DivlineSide(gSTrace.x, gSTrace.y, bspNode.line);

    // If the sight line cannot cross the closest half-space then we are done: sight is obstructed
    if (!PS_CrossBSPNode(bspNode.children[sideNum]))
        return false;
    
    // Check to see what side of the bsp split the end point for sight checking is on.
    // If it's in the same half-space we just raycasted against then we are done - sight is unobstructed.
    if (sideNum == PA_DivlineSide(gT2x, gT2y, bspNode.line))
        return true;

    // Failing that recurse into the opposite side of the BSP split and raycast against that, returning the result
    return PS_CrossBSPNode(bspNode.children[sideNum ^ 1]);
}
