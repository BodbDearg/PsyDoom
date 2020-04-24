#include "p_sight.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "p_local.h"
#include "p_setup.h"
#include "p_shoot.h"
#include "PsxVm/PsxVm.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <algorithm>
END_THIRD_PARTY_INCLUDES

static const VmPtr<fixed_t>     gSightZStart(0x80078020);   // Z position of thing looking
static const VmPtr<fixed_t>     gTopSlope(0x800781E0);      // Maximum/top unblocked viewing slope (clipped against upper walls)
static const VmPtr<fixed_t>     gBottomSlope(0x80078008);   // Minimum/bottom unblocked viewing slope (clipped against lower walls)
static const VmPtr<divline_t>   gSTrace(0x80097C00);        // The start point and vector for sight checking
static const VmPtr<fixed_t>     gT2x(0x80078100);           // End point for sight checking: x
static const VmPtr<fixed_t>     gT2y(0x80078108);           // End point for sight checking: y
static const VmPtr<int32_t>     gT1xs(0x800781F8);          // Sight line start, whole coords: x 
static const VmPtr<int32_t>     gT1ys(0x80078208);          // Sight line start, whole coords: y
static const VmPtr<int32_t>     gT2xs(0x80078204);          // Sight line end, whole coords: x
static const VmPtr<int32_t>     gT2ys(0x80078210);          // Sight line end, whole coords: y

void P_CheckSights() noexcept {
loc_80024908:
    sp -= 0x20;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    s0 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 -= 0x14;                                         // Result = gMObjHead[0] (800A8E90)
    s1 = 0x400000;                                      // Result = 00400000
    if (s0 == v0) goto loc_8002499C;
loc_80024930:
    a0 = lw(s0 + 0x64);
    v0 = a0 & s1;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80024988;
    }
    v1 = lw(s0 + 0x5C);
    {
        const bool bJump = (v1 != v0);
        v0 = 0xFBFF0000;                                // Result = FBFF0000
        if (bJump) goto loc_80024988;
    }
    v0 |= 0xFFFF;                                       // Result = FBFFFFFF
    a1 = lw(s0 + 0x74);
    v0 &= a0;
    sw(v0, s0 + 0x64);
    if (a1 == 0) goto loc_80024988;
    a0 = s0;
    P_CheckSight();
    v1 = 0x4000000;                                     // Result = 04000000
    if (v0 == 0) goto loc_80024988;
    v0 = lw(s0 + 0x64);
    v0 |= v1;
    sw(v0, s0 + 0x64);
loc_80024988:
    s0 = lw(s0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    if (s0 != v0) goto loc_80024930;
loc_8002499C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_CheckSight() noexcept {
loc_800249B4:
    sp -= 0x18;
    t0 = a0;
    t1 = a1;
    a1 = 0xE9BD0000;                                    // Result = E9BD0000
    sw(ra, sp + 0x10);
    v0 = lw(t0 + 0xC);
    a0 = *gpSectors;
    v0 = lw(v0);
    a1 |= 0x37A7;                                       // Result = E9BD37A7
    v0 -= a0;
    mult(v0, a1);
    v0 = lw(t1 + 0xC);
    v0 = lw(v0);
    v1 = lo;
    v0 -= a0;
    mult(v0, a1);
    v0 = lo;
    a0 = *gNumSectors;
    v1 = u32(i32(v1) >> 2);
    mult(v1, a0);
    v0 = u32(i32(v0) >> 2);
    v1 = 1;                                             // Result = 00000001
    a0 = lo;
    a0 += v0;
    a1 = u32(i32(a0) >> 3);
    v0 = *gpRejectMatrix;
    a0 &= 7;
    v0 += a1;
    v0 = lbu(v0);
    v1 = v1 << a0;
    v0 &= v1;
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80024B2C;
    }
    a3 = 0xFFFE0000;                                    // Result = FFFE0000
    a2 = 0x10000;                                       // Result = 00010000
    a1 = lw(t0);
    v0 = *gValidCount;
    a1 &= a3;
    a1 |= a2;
    at = 0x80090000;                                    // Result = 80090000
    sw(a1, at + 0x7C00);                                // Store to: gSTrace[0] (80097C00)
    a0 = lw(t0 + 0x4);
    v0++;
    *gValidCount = v0;
    v0 = u32(i32(a1) >> 16);
    sw(v0, gp + 0xC18);                                 // Store to: gT1xs (800781F8)
    a0 &= a3;
    a0 |= a2;
    at = 0x80090000;                                    // Result = 80090000
    sw(a0, at + 0x7C04);                                // Store to: gSTrace[1] (80097C04)
    v0 = lw(t1);
    v1 = u32(i32(a0) >> 16);
    sw(v1, gp + 0xC28);                                 // Store to: gT1ys (80078208)
    v1 = lw(t1 + 0x4);
    v0 &= a3;
    v0 |= a2;
    v1 &= a3;
    v1 |= a2;
    a1 = v0 - a1;
    a0 = v1 - a0;
    at = 0x80090000;                                    // Result = 80090000
    sw(a1, at + 0x7C08);                                // Store to: gSTrace[2] (80097C08)
    at = 0x80090000;                                    // Result = 80090000
    sw(a0, at + 0x7C0C);                                // Store to: gSTrace[3] (80097C0C)
    a1 = lw(t0 + 0x8);
    a0 = lw(t0 + 0x44);
    sw(v0, gp + 0xB20);                                 // Store to: gT2x (80078100)
    v0 = u32(i32(v0) >> 16);
    sw(v0, gp + 0xC24);                                 // Store to: gT2xs (80078204)
    v0 = lw(t1 + 0x8);
    sw(v1, gp + 0xB28);                                 // Store to: gT2y (80078108)
    v1 = u32(i32(v1) >> 16);
    sw(v1, gp + 0xC30);                                 // Store to: gT2ys (80078210)
    v1 = lw(t1 + 0x44);
    a1 += a0;
    a0 = u32(i32(a0) >> 2);
    a1 -= a0;
    v0 += v1;
    v0 -= a1;
    sw(v0, gp + 0xC00);                                 // Store to: gTopSlope (800781E0)
    v0 = lw(t1 + 0x8);
    a0 = *gNumBspNodes;
    sw(a1, gp + 0xA40);                                 // Store to: gSightZStart (80078020)
    v0 -= a1;
    sw(v0, gp + 0xA28);                                 // Store to: gBottomSlope (80078008)
    a0--;
    v0 = PS_CrossBSPNode(a0);
loc_80024B2C:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
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
    const int32_t sightX1 = *gT1xs;
    const int32_t sightY1 = *gT1ys;
    const int32_t sightX2 = *gT2xs;
    const int32_t sightY2 = *gT2ys;

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
    seg_t* const pSegs = &gpSegs->get()[subsec.firstseg];

    for (int32_t segIdx = 0; segIdx < numSegs; ++segIdx) {
        seg_t& seg = pSegs[segIdx];
        line_t& line = *seg.linedef;

        // Skip past this seg's line if we've already done it this sight check.
        // Multiple segs might reference the same line, so this saves redundant work:
        if (line.validcount == *gValidCount)
            continue;
        
        // Don't check the line again until the next sight check
        line.validcount = *gValidCount;

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
        sector_t& bsec = *line.backsector.get();
        sector_t& fsec = *line.frontsector.get();

        if ((fsec.floorheight == bsec.floorheight) && (fsec.ceilingheight == bsec.ceilingheight))
            continue;

        // If there is no vertical gap in the two sided line then sight is completely obscured
        const fixed_t lowestCeil = std::min(fsec.ceilingheight, bsec.ceilingheight);
        const fixed_t highestFloor = std::max(fsec.floorheight, bsec.floorheight);
        
        if (lowestCeil <= highestFloor)
            return false;
        
        // Narrow the allowed vertical sight range: against bottom wall
        if (fsec.floorheight != bsec.floorheight) {
            const fixed_t dz = highestFloor - *gSightZStart;
            const int32_t slope = ((dz << 6) / (intersectFrac >> 2)) << 8;      // Note: chops off the low 8 bits of computed intersect slope

            if (slope > *gBottomSlope) {
                *gBottomSlope = slope;
            }
        }

        // Narrow the allowed vertical sight range: against top wall
        if (fsec.ceilingheight != bsec.ceilingheight) {
            const fixed_t dz = lowestCeil - *gSightZStart;
            const int32_t slope = ((dz << 6) / (intersectFrac >> 2)) << 8;      // Note: chops off the low 8 bits of computed intersect slope

            if (slope < *gTopSlope) {
                *gTopSlope = slope;
            }
        }

        // If the allowed vertical sight range has become completely closed then sight is blocked
        if (*gTopSlope <= *gBottomSlope)
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
        
        if (subsecNum < *gNumSubsectors) {
            return PS_CrossSubsector(gpSubsectors->get()[subsecNum]);
        } else {
            I_Error("PS_CrossSubsector: ss %i with numss = %i", subsecNum, *gNumSubsectors);    // Bad subsector number!
            return false;
        }
    }

    // See what side of the bsp split the point is on: will check to see if the sight line is blocked by that half-space first
    node_t& bspNode = gpBspNodes->get()[nodeNum];
    const int32_t sideNum = PA_DivlineSide(gSTrace->x, gSTrace->y, bspNode.line);

    // If the sight line cannot cross the closest half-space then we are done: sight is obstructed
    if (!PS_CrossBSPNode(bspNode.children[sideNum]))
        return false;
    
    // Check to see what side of the bsp split the end point for sight checking is on.
    // If it's in the same half-space we just raycasted against then we are done - sight is unobstructed. 
    if (sideNum == PA_DivlineSide(*gT2x, *gT2y, bspNode.line))
        return true;

    // Failing that recurse into the opposite side of the BSP split and raycast against that, returning the result
    return PS_CrossBSPNode(bspNode.children[sideNum ^ 1]);
}
