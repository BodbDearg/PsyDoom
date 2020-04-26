#include "p_shoot.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "p_map.h"
#include "p_setup.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <algorithm>
END_THIRD_PARTY_INCLUDES

// The vertices used for the partial 'line_t' used for the shooters shoot line
struct thingline_t {
    vertex_t    p1;
    vertex_t    p2;
};

const VmPtr<VmPtr<mobj_t>>      gpShootMObj(0x800782D4);    // The thing that is being shot (if hit a thing)
const VmPtr<VmPtr<line_t>>      gpShootLine(0x800782D0);    // The line that is being shot (if hit a line)
const VmPtr<fixed_t>            gShootSlope(0x80077F4C);    // The Z slope for the line from the shooter origin to the hit point
const VmPtr<fixed_t>            gShootX(0x80077FC4);        // The point in space (X) that was hit when shooting (used for puff, blood spawn)
const VmPtr<fixed_t>            gShootY(0x80077FD0);        // The point in space (Y) that was hit when shooting (used for puff, blood spawn)
const VmPtr<fixed_t>            gShootZ(0x80077FD4);        // The point in space (Z) that was hit when shooting (used for puff, blood spawn)

static const VmPtr<fixed_t>             gAimMidSlope(0x80077FAC);           // The slope for the middle/center of the vertical aim range for the shooter: used to determine if we are hitting lower or upper walls
static const VmPtr<divline_t>           gShootDiv(0x800A9074);              // The start point and vector for shooting sight checking
static const VmPtr<fixed_t>             gShootX2(0x80078038);               // End point for shooting sight checking: x
static const VmPtr<fixed_t>             gShootY2(0x80078044);               // End point for shooting sight checking: y
static const VmPtr<int32_t>             gSsx1(0x800781F0);                  // Shooting sight line start, whole coords: x 
static const VmPtr<int32_t>             gSsy1(0x80078200);                  // Shooting sight line start, whole coords: y
static const VmPtr<int32_t>             gSsx2(0x800781FC);                  // Shooting sight line end, whole coords: x
static const VmPtr<int32_t>             gSsy2(0x8007820C);                  // Shooting sight line end, whole coords: y
static const VmPtr<VmPtr<void>>         gpOldValue(0x80078260);             // Intercept testing: previous closest line or thing
static const VmPtr<fixed_t>             gOldFrac(0x8007812C);               // Intercept testing: previous closest hit fractional distance (along line of sight)
static const VmPtr<bool32_t>            gbOldIsLine(0x80077EC8);            // Intercept testing: previous closest hit - was the hit against a line? (Was a thing if 'false')
static const VmPtr<bool32_t>            gbShootDivPositive(0x8007806C);     // True if the slope for the shooters shoot direction is positive
static const VmPtr<thingline_t>         gThingLineVerts(0x800A8A44);        // The vertices for the shooters shoot line
static const VmPtr<VmPtr<vertex_t>>     gPartialThingLine(0x80077B14);      // A partial/degenerate 'line_t' for the shooters line (just the two vertex pointer fields defined)
static const VmPtr<fixed_t>             gFirstLineFrac(0x800781D0);         // Fractional distance along the shooting line of the first/closest wall hit

//------------------------------------------------------------------------------------------------------------------------------------------
// Does a raycast for the current shooter taking a shot and determines what is hit.
// Saves the wall or thing which is hit, and the hit point etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_Shoot2() noexcept {
    // Save the start and precompute the end point for the shot line
    mobj_t& shooter = *gpShooter->get();

    gShootDiv->x = shooter.x;
    gShootDiv->y = shooter.y;
    
    const int32_t attackRangeInt = (*gAttackRange) >> FRACBITS;
    const uint32_t attackFineAngle = (*gAttackAngle) >> ANGLETOFINESHIFT;

    *gShootX2 = shooter.x + attackRangeInt * gFineCosine[attackFineAngle];
    *gShootY2 = shooter.y + attackRangeInt * gFineSine[attackFineAngle];
    
    // Precompute the line vector for the shot line and whether it's slope is positive
    gShootDiv->dx = *gShootX2 - gShootDiv->x;
    gShootDiv->dy = *gShootY2 - gShootDiv->y;
    *gbShootDivPositive = ((gShootDiv->dx ^ gShootDiv->dy) > 0);

    // Figure out the shot height and shot center line slope
    *gShootZ = shooter.z + shooter.height / 2 + (8 * FRACUNIT);
    *gAimMidSlope = (*gAimTopSlope + *gAimBottomSlope) / 2;

    // Precompute the start and end points for the shot line (integer/whole coords)
    *gSsx1 = gShootDiv->x >> FRACBITS;
    *gSsy1 = gShootDiv->y >> FRACBITS;
    *gSsx2 = *gShootX2 >> FRACBITS;
    *gSsy2 = *gShootY2 >> FRACBITS;

    // Initially nothing is hit
    *gpShootLine = nullptr;
    *gpShootMObj = nullptr;
    *gOldFrac = 0;

    // Test against all lines and things in the BSP tree
    PA_CrossBSPNode(*gNumBspNodes - 1);

    // If we didn't hit a thing then try check against the saved previous closest thing (if any)
    if (!gpShootMObj->get()) {
        PA_DoIntercept(nullptr, false, FRACUNIT);
    }
    
    // If we hit a wall then adjust the hit spot slightly so the puff isn't in the wall - move it out
    if ((!gpShootMObj->get()) && gpShootLine->get()) {
        const fixed_t hitFracAdjust = FixedDiv(4 * FRACUNIT, *gAttackRange);
        *gFirstLineFrac -= hitFracAdjust;

        *gShootX = gShootDiv->x + FixedMul(gShootDiv->dx, *gFirstLineFrac);
        *gShootY = gShootDiv->y + FixedMul(gShootDiv->dy, *gFirstLineFrac);
        *gShootZ += FixedMul(*gAimMidSlope, FixedMul(*gFirstLineFrac, *gAttackRange));
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Test for a hit against the given line or thing if it's closer than the previous closest line or thing we checked against.
// Otherwise, if there was something previously closer - test against that first and remember this line or thing for later potential tests.
//
// This function essentially tries to ensure intersection tests are performed against the closest things first.
// I don't think it can neccesarily guarantee that is the case, although it's probably good enough for most gameplay scenarios?
//------------------------------------------------------------------------------------------------------------------------------------------
bool PA_DoIntercept(void* const pObj, const bool bIsLine, const fixed_t hitFrac) noexcept {
    // Initially, the input is what we will test against
    void* test_pObj = pObj;
    bool test_bIsLine = bIsLine;
    fixed_t test_hitFrac = hitFrac;

    // But if the previous thing we remembered is closer, test against that instead and remember this input for later
    if (hitFrac > *gOldFrac) {
        test_pObj = gpOldValue->get();
        test_bIsLine = *gbOldIsLine;
        test_hitFrac = *gOldFrac;

        *gpOldValue = pObj;
        *gbOldIsLine = bIsLine;
        *gOldFrac = hitFrac;
    }

    // If the test is beyond the range (or at the ends) of the shoot sight line then there is no hit
    if ((test_hitFrac == 0) || (test_hitFrac >= FRACUNIT))
        return true;

    // PC-PSX: added safety - just in case.
    // I don't think it's ever possible to get into a scenario where the test object is null here, but I'm going to guarantee it anyhow...
    #if PC_PSX_DOOM_MODS
        if (!test_pObj)
            return true;
    #endif
    
    // Otherwise do the test against the line or thing
    if (test_bIsLine) {
        return PA_ShootLine(*(line_t*) test_pObj, test_hitFrac);
    } else {
        return PA_ShootThing(*(mobj_t*) test_pObj, test_hitFrac);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Try to shoot a line assuming an unobstructed line of sight from the shooter to the line.
// Returns 'false' if no other lines should be tested against.
//------------------------------------------------------------------------------------------------------------------------------------------
bool PA_ShootLine(line_t& line, const fixed_t hitFrac) noexcept {
    // If the line is not two sided (solid/impassible) then it blocks shots always
    if ((line.flags & ML_TWOSIDED) == 0) {
        // Save this as the first line hit if there already isn't one
        if (!gpShootLine->get()) {
            *gFirstLineFrac = hitFrac;
            *gpShootLine = &line;
        }

        *gOldFrac = 0;      // Don't shoot anything past this line
        return false;
    }

    // Figure out the extent of the gap that can be shot through (area between the highest floor and lowest ceiling)
    const sector_t& fsec = *line.frontsector;
    const sector_t& bsec = *line.backsector;
    const fixed_t highestFloorHeight = std::max(fsec.floorheight, bsec.floorheight);        
    const fixed_t lowestCeilHeight = std::min(fsec.ceilingheight, bsec.ceilingheight);

    // How far away is the hit point?
    const fixed_t hitDist = FixedMul(*gAttackRange, hitFrac);
    
    // Is there a lower wall which can be hit?
    if (fsec.floorheight != bsec.floorheight) {
        const fixed_t slopeToFloor = FixedDiv(highestFloorHeight - *gShootZ, hitDist);
        
        // The lower wall can be hit if its top is above the aim line and nothing else has been hit
        if ((!gpShootLine->get()) && (slopeToFloor >= *gAimMidSlope)) {
            *gFirstLineFrac = hitFrac;
            *gpShootLine = &line;
        }

        // Narrow the allowed vertical aim range by this lower wall - can only shoot above it now 
        *gAimBottomSlope = std::max(*gAimBottomSlope, slopeToFloor);
    }
    
    // Is there an upper wall which can be hit?
    if (fsec.ceilingheight != bsec.ceilingheight) {
        const fixed_t slopeToCeiling = FixedDiv(lowestCeilHeight - *gShootZ, hitDist);

        // The upper wall can be hit if its bottom is below the aim line and nothing else has been hit
        if ((!gpShootLine->get()) && (slopeToCeiling <= *gAimMidSlope)) {
            *gFirstLineFrac = hitFrac;
            *gpShootLine = &line;
        }
        
        // Narrow the allowed vertical aim range by this upper wall - can only shoot below it now
        *gAimTopSlope = std::min(*gAimTopSlope, slopeToCeiling);
    }

    // If the opening is fully closed then the shot stops here
    return (*gAimTopSlope > *gAimBottomSlope);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to shoot a thing assuming an unobstructed line of sight from the shooter to the thing.
// Returns 'false' if the thing was shot and saves the details of the hit thing and the hit point etc.
//------------------------------------------------------------------------------------------------------------------------------------------
bool PA_ShootThing(mobj_t& thing, const fixed_t hitFrac) noexcept {
    // A shooter cannot shoot itself
    if (&thing == gpShooter->get())
        return true;

    // Can't shoot the thing if it's not shootable (corpse etc.)
    if ((thing.flags & MF_SHOOTABLE) == 0)
        return true;
    
    // How far is the hit point away?
    const fixed_t hitDist = FixedMul(*gAttackRange, hitFrac);

    // Are we shooting over the thing? If so then it cannot be hit.
    // Check the allowed shooting vertical range against the top of the thing's bounding box.
    fixed_t thingTopSlope = FixedDiv(thing.z + thing.height - *gShootZ, hitDist);
    
    if (*gAimBottomSlope > thingTopSlope)
        return true;

    // Are we shooting under over the thing? If so then it cannot be hit.
    // Check the allowed shooting vertical range against the bottom of the thing's bounding box.
    fixed_t thingBottomSlope = FixedDiv(thing.z - *gShootZ, hitDist);

    if (*gAimTopSlope < thingBottomSlope)
        return true;
    
    // Clamp the parts of the thing we can hit to the allowed ranges for shooting
    thingTopSlope = std::min(thingTopSlope, *gAimTopSlope);
    thingBottomSlope = std::max(thingBottomSlope, *gAimBottomSlope);

    // Shoot the thing midway along the visible parts of it and remember what is being shot
    *gShootSlope = (thingTopSlope + thingBottomSlope) / 2;
    *gpShootMObj = &thing;

    // Adjust the hit point so it is a little closer - move away from thing center by 10.0 units.
    // Done so blood drops are spawned in front of the thing.
    const fixed_t adjustedHitFrac = hitFrac - FixedDiv(10 * FRACUNIT, *gAttackRange);

    // Figure out the hit point
    *gShootX = gShootDiv->x + FixedMul(gShootDiv->dx, adjustedHitFrac);
    *gShootY = gShootDiv->y + FixedMul(gShootDiv->dy, adjustedHitFrac);
    *gShootZ += FixedMul(*gShootSlope, FixedMul(adjustedHitFrac, *gAttackRange));

    // We hit something so the shooting ray has been obstructed
    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Intersects the given line against the current sight line and returns the fraction of intersection along the sight line.
// When the intersect ratio is > 0.0 and < 1.0 then there is a valid intersection with the sight line, otherwise there is no
// intersection or the intersection occurs beyond the range of the line.
//------------------------------------------------------------------------------------------------------------------------------------------
static fixed_t PA_SightCrossLine(const line_t& line) noexcept {
    // Get the integer coordinates of the line and the sight line
    const int32_t lineX1 = line.vertex1->x >> FRACBITS;
    const int32_t lineY1 = line.vertex1->y >> FRACBITS;
    const int32_t lineX2 = line.vertex2->x >> FRACBITS;
    const int32_t lineY2 = line.vertex2->y >> FRACBITS;
    const int32_t sightX1 = *gSsx1;
    const int32_t sightY1 = *gSsy1;
    const int32_t sightX2 = *gSsx2;
    const int32_t sightY2 = *gSsy2;

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
// Test the shooting 'sight' line against lines and things in the given subsector.
// Returns 'false' if there was a hit (sight obstructed) and remembers what was hit for later logic to act upon. 
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PA_CrossSubsector(subsector_t& subsec) noexcept {
    // Check for hit against all things in the subsector
    for (mobj_t* pmobj = subsec.sector->thinglist.get(); pmobj != nullptr; pmobj = pmobj->snext.get()) {
        // Ignore the thing if its not in this subsector
        if (pmobj->subsector.get() != &subsec)
            continue;

        // Setup the shooters degenerate 'line_t' for the shoot line.
        // This creates a line between the two corners of the hit box.
        if (*gbShootDivPositive) {
            gThingLineVerts->p1.x = pmobj->x - pmobj->radius;
            gThingLineVerts->p1.y = pmobj->y + pmobj->radius;
            gThingLineVerts->p2.x = pmobj->x + pmobj->radius;
            gThingLineVerts->p2.y = pmobj->y - pmobj->radius;
        } else {
            gThingLineVerts->p1.x = pmobj->x - pmobj->radius;
            gThingLineVerts->p1.y = pmobj->y - pmobj->radius;
            gThingLineVerts->p2.x = pmobj->x + pmobj->radius;
            gThingLineVerts->p2.y = pmobj->y + pmobj->radius;
        }
        
        // See if the thing line intersects the sight/shoot line, ignore if it doesn't
        const fixed_t hitFrac = PA_SightCrossLine(*(line_t*) gPartialThingLine.get());

        if ((hitFrac < 0) || (hitFrac > FRACUNIT))
            continue;

        // Register a hit against this thing or the current closest object hit (returns 'false' if sight obstructed)
        if (!PA_DoIntercept(pmobj, false, hitFrac))
            return false;
    }

    // Check for hits against all segs in the subsector
    const int32_t curValidCount = *gValidCount;
    seg_t* const pSegs = gpSegs->get() + subsec.firstseg;
    const int16_t numSegs = subsec.numsegs;
    
    for (int32_t segIdx = 0; segIdx < numSegs; ++segIdx) {
        // Don't check this line if we already checked for this sight check
        seg_t& seg = pSegs[segIdx];
        line_t& line = *seg.linedef;

        if (line.validcount == curValidCount)
            continue;

        // Don't check again for this sight test (mark) and get where the line intersects the sight line
        line.validcount = curValidCount;
        const fixed_t hitFrac = PA_SightCrossLine(line);
            
        // Ignore the intersection if it's not along the sight line
        if ((hitFrac < 0) || (hitFrac > FRACUNIT))
            continue;

        // Register a hit against this line or the current closest object hit (returns 'false' if sight obstructed)
        if (!PA_DoIntercept(&line, true, hitFrac))
            return false;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells what side of the given divline the given point is on.
// Returns '0' if the point is on the 'front' side of the line, otherwise '1' if on the back side.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t PA_DivlineSide(const fixed_t x, const fixed_t y, const divline_t& line) noexcept {
    // This is pretty much the same cross product method as found in 'R_PointOnSide', without the special cases
    const int32_t dx1 = (x - line.x) >> FRACBITS;
    const int32_t dy1 = (y - line.y) >> FRACBITS;
    const int32_t dx2 = line.dx >> FRACBITS;
    const int32_t dy2 = line.dy >> FRACBITS;
    const int32_t sideNum = (dx1 * dy2 <= dy1 * dx2);
    return sideNum;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Recursive sight checking: tells if the 'gShootDiv' line is blocked by the BSP tree halfspace represented by the given node.
// Returns 'true' if the shooting sight line is unobstructed.
//------------------------------------------------------------------------------------------------------------------------------------------
bool PA_CrossBSPNode(const int32_t nodeNum) noexcept {
    // Is this bsp node actually a subsector? (leaf node) If so then do sight checks against that:
    if (nodeNum & NF_SUBSECTOR) {
        const int32_t subsecNum = nodeNum & (~NF_SUBSECTOR);
        
        if (subsecNum < *gNumSubsectors) {
            return PA_CrossSubsector(gpSubsectors->get()[subsecNum]);
        } else {
            I_Error("PA_CrossSubsector: ss %i with numss = %i", subsecNum, *gNumSubsectors);    // Bad subsector number!
            return false;
        }
    }

    // See what side of the bsp split the point is on: will check to see if the sight line is blocked by that half-space first
    node_t& bspNode = gpBspNodes->get()[nodeNum];
    const int32_t sideNum = PA_DivlineSide(gShootDiv->x, gShootDiv->y, bspNode.line);

    // If the sight line cannot cross the closest half-space then we are done: sight is obstructed
    if (!PA_CrossBSPNode(bspNode.children[sideNum]))
        return false;
    
    // Check to see what side of the bsp split the end point for sight checking is on.
    // If it's in the same half-space we just raycasted against then we are done - sight is unobstructed. 
    if (sideNum == PA_DivlineSide(*gShootX2, *gShootY2, bspNode.line))
        return true;

    // Failing that recurse into the opposite side of the BSP split and raycast against that, returning the result
    return PA_CrossBSPNode(bspNode.children[sideNum ^ 1]);
}
