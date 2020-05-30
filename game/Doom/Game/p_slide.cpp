#include "p_slide.h"

#include "Doom/Base/m_fixed.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "p_local.h"
#include "p_setup.h"

#include <algorithm>

static constexpr int32_t CLIPRADIUS = 23;   // Radius of the player for collisions against lines
static constexpr int32_t SIDE_FRONT = +1;   // Return code for side checking: point is on the front side of the line
static constexpr int32_t SIDE_ON    =  0;   // Return code for side checking: point is on the line
static constexpr int32_t SIDE_BACK  = -1;   // Return code for side checking: point is on the back side of the line

const VmPtr<VmPtr<mobj_t>>      gpSlideThing(0x80077ED8);       // The thing being moved
const VmPtr<fixed_t>            gSlideX(0x80077F90);            // Where the player move is starting from: x
const VmPtr<fixed_t>            gSlideY(0x80077F94);            // Where the player move is starting from: y
const VmPtr<VmPtr<line_t>>      gpSpecialLine(0x80077F9C);      // The special line that would be crossed by player movement

static const VmPtr<fixed_t>         gSlideDx(0x80078070);           // How much the player is wanting to move: x
static const VmPtr<fixed_t>         gSlideDy(0x80078074);           // How much the player is wanting to move: y
static const VmPtr<fixed_t[4]>      gEndBox(0x80097BF0);            // Bounding box for the proposed movement
static const VmPtr<fixed_t>         gBlockFrac(0x80078228);         // Percentage of the current move allowed
static const VmPtr<fixed_t>         gBlockNvx(0x800781A8);          // The vector to slide along for the line collided with: x
static const VmPtr<fixed_t>         gBlockNvy(0x800781B0);          // The vector to slide along for the line collided with: y
static const VmPtr<fixed_t>         gNvx(0x80078158);               // Line being collided against, normalized normal: x
static const VmPtr<fixed_t>         gNvy(0x8007815C);               // Line being collided against, normalized normal: y
static const VmPtr<fixed_t>         gP1x(0x800780CC);               // Line being collided against, p1: x
static const VmPtr<fixed_t>         gP1y(0x800780D4);               // Line being collided against, p1: y
static const VmPtr<fixed_t>         gP2x(0x800780D0);               // Line being collided against, p2: x
static const VmPtr<fixed_t>         gP2y(0x800780E0);               // Line being collided against, p2: y
static const VmPtr<fixed_t>         gP3x(0x800780DC);               // Movement line, p1: x
static const VmPtr<fixed_t>         gP3y(0x800780F4);               // Movement line, p1: y
static const VmPtr<fixed_t>         gP4x(0x800780F0);               // Movement line, p2: x
static const VmPtr<fixed_t>         gP4y(0x800780FC);               // Movement line, p2: y

// Not required externally: making private to this module
static fixed_t P_CompletableFrac(const fixed_t dx, const fixed_t dy) noexcept;
static void SL_CheckLine(line_t& line) noexcept;
static void SL_CheckSpecialLines(const fixed_t moveX1, const fixed_t moveY1, const fixed_t moveX2, const fixed_t moveY2) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to do movement (with wall sliding) for the player's map object.
//
// Global inputs:
//      gpSlideThing        : The player thing being moved
//
// Global outputs:
//      gSlideX, gSlideY    : The final position of the thing after movement
//      gpSpecialLine       : The special line that was crossed by movement (null if none)
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SlideMove() noexcept {
    // Set the starting position and grab the initial movement amount
    mobj_t& slideThing = *gpSlideThing->get();
    *gSlideX = slideThing.x;
    *gSlideY = slideThing.y;

    fixed_t moveDx = slideThing.momx;
    fixed_t moveDy = slideThing.momy;

    // If the noclip cheat is on then simply move to where we want to and check for line specials
    if (slideThing.flags & MF_NOCLIP) {
        *gSlideX += slideThing.momx;
        *gSlideY += slideThing.momy;
        SL_CheckSpecialLines(slideThing.x, slideThing.y, *gSlideX, *gSlideY);
        return;
    }

    // Do 3 movements or bounces off walls before giving up
    for (int32_t attemptCount = 0; attemptCount < 3; ++attemptCount) {
        // Figure out the percentage we can move this step.
        // If the movement was not fully completable then reduce the move amount slightly to account for imprecision:
        fixed_t moveFrac = P_CompletableFrac(moveDx, moveDy);

        if (moveFrac != FRACUNIT) {
            moveFrac -= FRACUNIT / 16;
        }

        moveFrac = std::max(moveFrac, 0);   // Never let it be negative!

        // Move by the amount we were allowed to move in the movement direction
        const fixed_t allowedDx = FixedMul(moveFrac, moveDx);
        const fixed_t allowedDy = FixedMul(moveFrac, moveDy);
        *gSlideX += allowedDx;
        *gSlideY += allowedDy;

        // If the entire move was allowed then we are done, and check for crossing special lines
        if (moveFrac == FRACUNIT) {
            slideThing.momx = moveDx;
            slideThing.momy = moveDy;
            SL_CheckSpecialLines(slideThing.x, slideThing.y, *gSlideX, *gSlideY);
            return;
        }

        // For the next iteration try slide along the wall.
        // Slide by the movement amount that was disallowed in the direction of the wall.
        const fixed_t blockedDx = moveDx - allowedDx;
        const fixed_t blockedDy = moveDy - allowedDy;
        const fixed_t slideDist = FixedMul(blockedDx, *gBlockNvx) + FixedMul(blockedDy, *gBlockNvy);

        moveDx = FixedMul(slideDist, *gBlockNvx);
        moveDy = FixedMul(slideDist, *gBlockNvy);
    }

    // If we get to here then movement failed, set the result position to the original position.
    // Kill the player's momentum in this case also and don't bother checking for special lines crossed.
    *gSlideX = slideThing.x;
    *gSlideY = slideThing.y;
    slideThing.momy = 0;
    slideThing.momx = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the fraction (0-1) of the given movement amount (dx, dy) that is completable.
//
// Global inputs:
//      gpSlideThing            : The thing being moved
//      gSlideX, gSlideY        : Move start point
//
// Global outputs:
//      gBlockNvx, gBlockNvy    : Normalized vector to slide along the collision wall (set if the collision is closer than current closest)
//------------------------------------------------------------------------------------------------------------------------------------------
static fixed_t P_CompletableFrac(const fixed_t dx, const fixed_t dy) noexcept {
    // Assume we can move the entire distance until we find otherwise and save movement amount globally
    *gBlockFrac = FRACUNIT;
    *gSlideDx = dx;
    *gSlideDy = dy;

    // Compute the bounding box for the move
    gEndBox[BOXTOP]    = *gSlideY + CLIPRADIUS * FRACUNIT;
    gEndBox[BOXBOTTOM] = *gSlideY - CLIPRADIUS * FRACUNIT;
    gEndBox[BOXLEFT]   = *gSlideX - CLIPRADIUS * FRACUNIT;
    gEndBox[BOXRIGHT]  = *gSlideX + CLIPRADIUS * FRACUNIT;
    
    if (dx > 0) {
        gEndBox[BOXRIGHT] += dx;
    } else {
        gEndBox[BOXLEFT] += dx;
    }
    
    if (dy > 0) {
        gEndBox[BOXTOP] += dy;
    } else {
        gEndBox[BOXBOTTOM] += dy;
    }

    // Compute the blockmap extents for the move
    const int32_t bmapTy = std::min((gEndBox[BOXTOP] - *gBlockmapOriginY) >> MAPBLOCKSHIFT, *gBlockmapHeight - 1);
    const int32_t bmapBy = std::max((gEndBox[BOXBOTTOM] - *gBlockmapOriginY) >> MAPBLOCKSHIFT, 0);
    const int32_t bmapLx = std::max((gEndBox[BOXLEFT] - *gBlockmapOriginX) >> MAPBLOCKSHIFT, 0);
    const int32_t bmapRx = std::min((gEndBox[BOXRIGHT] - *gBlockmapOriginX) >> MAPBLOCKSHIFT, *gBlockmapWidth - 1);
    
    // Increment this counter for the line checks that follow: doing new checks
    *gValidCount += 1;

    // Run through all of the blockmap cells covering the movemet range.
    // Collide the movement line against all lines found in these cells.
    for (int32_t bmapX = bmapLx; bmapX <= bmapRx; ++bmapX) {
        for (int32_t bmapY = bmapBy; bmapY <= bmapTy; ++bmapY) {
            // Get where the line numbers list for this blockmap cell starts in the blockmap
            int16_t* pLineNum = (int16_t*) gpBlockmapLump->get() + gpBlockmap->get()[bmapX + bmapY * (*gBlockmapWidth)];

            // Collide against all of the lines in this cell
            for (; *pLineNum != -1; ++pLineNum) {
                line_t& line = gpLines->get()[*pLineNum];
                
                // Only collide against this line if we didn't already do it
                if (line.validcount != *gValidCount) {
                    line.validcount = *gValidCount;
                    SL_CheckLine(line);
                }
            }
        }
    }

    // If the movement amount is less than 1/16 of what is allowed then stop movement entirely
    if (*gBlockFrac < FRACUNIT / 16) {
        *gBlockFrac = 0;
        *gpSpecialLine = nullptr;
    }

    return *gBlockFrac;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell what side of the current test line the given point is on.
// Answer is one of 'SIDE_FRONT', 'SIDE_BACK' or 'SIDE_ON'; 'SIDE_ON' is returned with the distance is <= 1.0.
//
// Global inputs:
//      gP1x, gP1y  : First point of the line being tested against
//      gNvx, gNvy  : Normalized normal for the line being tested against
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t SL_PointOnSide(const fixed_t x, const fixed_t y) noexcept {
    // Use the dot product of a line relative vector with the normal to tell the side
    const fixed_t dist = FixedMul(x - *gP1x, *gNvx) + FixedMul(y - *gP1y, *gNvy);

    if (dist > FRACUNIT) {
        return SIDE_FRONT;
    } else if (dist < -FRACUNIT) {
        return SIDE_BACK;
    } else {
        return SIDE_ON;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Computes and returns the fraction along the movement line that the intersection with the test line occurs at.
//
// Global inputs:
//      gP3x, gP3y, gP4x, gP4y  : Move line coordinates
//      gP1x, gP1y              : First point of the line being tested against
//      gNvx, gNvy              : Normalized normal for the line being tested against
//------------------------------------------------------------------------------------------------------------------------------------------
static fixed_t SL_CrossFrac() noexcept {
    // Project the move start and end points onto the normalized normal of the line being tested against.
    // This gives us the perpendicular distance of these points to the line.
    const fixed_t dist1 = FixedMul(*gP3x - *gP1x, *gNvx) + FixedMul(*gP3y - *gP1y, *gNvy);
    const fixed_t dist2 = FixedMul(*gP4x - *gP1x, *gNvx) + FixedMul(*gP4y - *gP1y, *gNvy);

    // If the points don't cross the line then there 
    if ((dist1 < 0) == (dist2 < 0))
        return FRACUNIT;
    
    // Otherwise compute the fraction along the move line that the intersection occurs at
    return FixedDiv(dist1, dist1 - dist2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the line defined by P1/P2 crosses the line defined by P3/P4; this function is unused in PSX Doom.
//
// PC-PSX: not compiling this as it generates an unused warning.
//------------------------------------------------------------------------------------------------------------------------------------------
#if !PC_PSX_DOOM_MODS

static bool CheckLineEnds() noexcept {
    // Compute line normal
    const fixed_t nx = *gP4y - *gP3y;
    const fixed_t ny = *gP3x - *gP4x;

    // Compute vectors relative to the line's first point
    const fixed_t rx1 = *gP1x - *gP3x;
    const fixed_t ry1 = *gP1y - *gP3y;
    const fixed_t rx2 = *gP2x - *gP3x;
    const fixed_t ry2 = *gP2y - *gP3y;

    // Return true if P1 & P2 are on opposite sides of the line by using the dot product to compute perpendicular distance to the line
    const fixed_t dist1 = FixedMul(rx1, nx) + FixedMul(ry1, ny);
    const fixed_t dist2 = FixedMul(rx2, nx) + FixedMul(ry2, ny);

    return ((dist1 < 0) != (dist2 < 0));
}

#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Intersect the current movement line against the current collision line being tested.
// Reduces the allowed movement amount if there is a collision, and it's closer than the current closest collision.
//
// Global inputs:
//      gSlideX, gSlideY        : Move start point
//      gSlideDx, gSlideDy      : Movement amount/delta
//      gP1x, gP1y              : 1st point of the line being collided against
//      gNvx, gNvy              : Normalized normal for the line being collided against
//
// Global outputs:
//      gBlockFrac              : Allowed movement percent (set if the collision is closer than current closest)
//      gBlockNvx, gBlockNvy    : Normalized vector to slide along the collision wall (set if the collision is closer than current closest)
//------------------------------------------------------------------------------------------------------------------------------------------
static void SL_ClipToLine() noexcept {
    // Move start point is on the circumference of the player circle, the closest point to the wall (use the normal to compute that).
    // Move end point is that plus the movement amount.
    *gP3x = *gSlideX - (*gNvx) * CLIPRADIUS;
    *gP3y = *gSlideY - (*gNvy) * CLIPRADIUS;
    *gP4x = *gP3x + *gSlideDx;
    *gP4y = *gP3y + *gSlideDy;

    // If the move start point is on the wrong side of the line then ignore (can't be colliding with it)
    const int32_t moveP1Side = SL_PointOnSide(*gP3x, *gP3y);

    if (moveP1Side == SIDE_BACK)
        return;
    
    // If the move end point is (roughly) along or in front of the line then the move is allowed and we don't need to clip
    const int32_t moveP2Side = SL_PointOnSide(*gP4x, *gP4y);

    if ((moveP2Side == SIDE_ON) || (moveP2Side == SIDE_FRONT))
        return;
    
    // If the move start point is already on the line and the end point is behind then disallow the move entirely
    if (moveP1Side == SIDE_ON) {
        *gBlockNvx = -*gNvy;
        *gBlockNvy = *gNvx;
        *gBlockFrac = 0;
        return;
    }

    // Compute how much percent of the move is allowed.
    // Only save this collision if it's closer than the current closest:
    const fixed_t intersectFrac = SL_CrossFrac();

    if (intersectFrac < *gBlockFrac) {
        *gBlockNvx = -*gNvy;
        *gBlockNvy = *gNvx;
        *gBlockFrac = intersectFrac;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clips the current proposed movement to the given line.
//
// Global inputs:
//      gpSlideThing            : The thing being moved
//      gSlideX, gSlideY        : Move start point
//      gSlideDx, gSlideDy      : Movement amount/delta
//      gEndBox                 : Bounding box for the entire movement (including start and end points)
//
// Global outputs:
//      gBlockFrac              : Allowed movement percent (set if the collision is closer than current closest)
//      gBlockNvx, gBlockNvy    : Normalized vector to slide along the collision wall (set if the collision is closer than current closest)
//------------------------------------------------------------------------------------------------------------------------------------------
static void SL_CheckLine(line_t& line) noexcept {
    // If the movement bounding box does NOT overlap the line bounding box then we can ignore this line
    const bool bNoBBOverlap = (
        (gEndBox[BOXTOP] < line.bbox[BOXBOTTOM]) ||
        (gEndBox[BOXBOTTOM] > line.bbox[BOXTOP]) ||
        (gEndBox[BOXLEFT] > line.bbox[BOXRIGHT]) ||
        (gEndBox[BOXRIGHT] < line.bbox[BOXLEFT])
    );

    if (bNoBBOverlap)
        return;

    // See if the line is actually blocking of movement - ignore it if it isn't.
    // Firstly see if the line is one sided or marked as explicitly blocking:
    sector_t* const pBackSector = line.backsector.get();
    const bool bImpassibleLine = ((!pBackSector) || (line.flags & ML_BLOCKING));

    if (!bImpassibleLine) {
        // Not yet blocking, check to see is the step too high:
        sector_t& frontSector = *line.frontsector;

        const fixed_t openingBottom = std::max(frontSector.floorheight, pBackSector->floorheight);
        const fixed_t stepUp = openingBottom - gpSlideThing->get()->z;
        const bool bStepTooHigh = (stepUp > 24 * FRACUNIT);

        if (!bStepTooHigh) {
            // Step is fine to go up: see if the overall line gap is too small to fit the player
            const fixed_t openingTop = std::min(frontSector.ceilingheight, pBackSector->ceilingheight);
            const bool bGapTooSmall = (openingTop - openingBottom < 56 * FRACUNIT);

            // If the line still isn't blocking after this last check then we can ignore it
            if (!bGapTooSmall)
                return;
        }
    }

    // Save the points and normal globally for the line being collided against - needed by 'SL_ClipToLine':
    *gP1x = line.vertex1->x;
    *gP1y = line.vertex1->y;
    *gP2x = line.vertex2->x;
    *gP2y = line.vertex2->y;
    *gNvx = gFineSine[line.fineangle];
    *gNvy = -gFineCosine[line.fineangle];

    // Get what side of the line the start point for the move is on
    const int32_t lineSide = SL_PointOnSide(*gSlideX, *gSlideY);

    // If the movement start is already nearly on the line then it is ignored
    if (lineSide == SIDE_ON)
        return;

    // If we are on the back side of the line then we only collide if the line is two sided.
    // If the line is two sided and we are doing a collision, then we need to reverse the points and the normal.
    if (lineSide == SIDE_BACK) {
        // Only do back side collision against two sided lines!
        if (!line.backsector)
            return;

        // Swap around everything because we are on the opposite side
        std::swap(*gP1x, *gP2x);
        std::swap(*gP1y, *gP2y);
        *gNvx = -*gNvx;
        *gNvy = -*gNvy;
    }

    // Clip the movement line to the line being collided against
    SL_ClipToLine();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell what side of a line a point is on: answer is either 'SIDE_FRONT' or 'SIDE_BACK'
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t SL_PointOnSide2(
    const fixed_t px,
    const fixed_t py,
    const fixed_t lx1,
    const fixed_t ly1,
    const fixed_t lx2,
    const fixed_t ly2
) noexcept {
    // Use the dot product of a line relative vector against the line normal to determine which side of the line we are on
    const fixed_t rx = px - lx1;
    const fixed_t ry = py - ly1;
    const fixed_t nx = ly2 - ly1;
    const fixed_t ny = lx1 - lx2;
    const fixed_t signedDist = FixedMul(rx, nx) + FixedMul(ry, ny);

    return (signedDist >= 0) ? SIDE_FRONT : SIDE_BACK;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks to see if a move would cross a special line.
// Saves the crossed special line to 'gpSpecialLine', which is null if no special line is crossed.
//------------------------------------------------------------------------------------------------------------------------------------------
static void SL_CheckSpecialLines(const fixed_t moveX1, const fixed_t moveY1, const fixed_t moveX2, const fixed_t moveY2) noexcept {
    // Compute the blockmap extents of the move
    const fixed_t minMoveX = std::min(moveX1, moveX2);
    const fixed_t maxMoveX = std::max(moveX1, moveX2);
    const fixed_t minMoveY = std::min(moveY1, moveY2);
    const fixed_t maxMoveY = std::max(moveY1, moveY2);

    const int32_t bmapLx = std::max((minMoveX - *gBlockmapOriginX) >> MAPBLOCKSHIFT, 0);
    const int32_t bmapRx = std::min((maxMoveX - *gBlockmapOriginX) >> MAPBLOCKSHIFT, *gBlockmapWidth - 1);
    const int32_t bmapBy = std::max((minMoveY - *gBlockmapOriginY) >> MAPBLOCKSHIFT, 0);
    const int32_t bmapTy = std::min((maxMoveY - *gBlockmapOriginY) >> MAPBLOCKSHIFT, *gBlockmapHeight - 1);
    
    // Hit no special line yet and increment the valid count for a fresh check
    *gpSpecialLine = nullptr;
    *gValidCount += 1;

    // Check for crossing lines in this blockmap area
    for (int32_t bmapX = bmapLx; bmapX <= bmapRx; ++bmapX) {
        for (int32_t bmapY = bmapBy; bmapY <= bmapTy; ++bmapY) {
            const int32_t firstLineOffset = gpBlockmap->get()[bmapX + bmapY * (*gBlockmapWidth)];
            
            for (int16_t* pLineNum = (int16_t*) &gpBlockmapLump->get()[firstLineOffset]; *pLineNum != -1; ++pLineNum) {
                // Ignore the line if it has no special or if we already checked
                line_t& line = gpLines->get()[*pLineNum];

                if (line.special == 0)
                    continue;

                if (line.validcount == *gValidCount)
                    continue;

                // Don't check again
                line.validcount = *gValidCount;

                // Make sure the move is within the bounding box of the line, ignore if not:
                const bool bNoOverlap = (
                    (maxMoveX < line.bbox[BOXLEFT]) ||
                    (minMoveX > line.bbox[BOXRIGHT]) ||
                    (maxMoveY < line.bbox[BOXBOTTOM]) ||
                    (minMoveY > line.bbox[BOXTOP])
                );

                if (bNoOverlap)
                    continue;

                // See if the move crosses the line, if it doesn't then skip this line
                const fixed_t v1x = line.vertex1->x;
                const fixed_t v1y = line.vertex1->y;
                const fixed_t v2x = line.vertex2->x;
                const fixed_t v2y = line.vertex2->y;

                {
                    const int32_t side1 = SL_PointOnSide2(moveX1, moveY1, v1x, v1y, v2x, v2y);
                    const int32_t side2 = SL_PointOnSide2(moveX2, moveY2, v1x, v1y, v2x, v2y);

                    if (side1 == side2)
                        continue;
                }

                // See if the line crosses the move, if it doesn't then skip this line
                {
                    const int32_t side1 = SL_PointOnSide2(v1x, v1y, moveX1, moveY1, moveX2, moveY2);
                    const int32_t side2 = SL_PointOnSide2(v2x, v2y, moveX1, moveY1, moveX2, moveY2);

                    if (side1 == side2)
                        continue;
                }

                // Crossed a special line, done:
                *gpSpecialLine = &line;
                return;
            }
        }
    }
}
