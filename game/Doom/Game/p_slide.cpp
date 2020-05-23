#include "p_slide.h"

#include "Doom/Base/m_fixed.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "p_local.h"
#include "p_setup.h"
#include "PsxVm/PsxVm.h"
#include <algorithm>

static constexpr int32_t CLIPRADIUS = 23;   // Radius of the player for collisions against lines
static constexpr int32_t SIDE_FRONT = +1;   // Return code for side checking: point is on the front side of the line
static constexpr int32_t SIDE_ON    =  0;   // Return code for side checking: point is on the line
static constexpr int32_t SIDE_BACK  = -1;   // Return code for side checking: point is on the back side of the line

const VmPtr<VmPtr<mobj_t>>      gpSlideThing(0x80077ED8);       // The thing being moved

static const VmPtr<fixed_t>         gSlideX(0x80077F90);            // Where the player move is starting from: x
static const VmPtr<fixed_t>         gSlideY(0x80077F94);            // Where the player move is starting from: y
static const VmPtr<fixed_t>         gSlideDx(0x80078070);           // How much the player is wanting to move: x
static const VmPtr<fixed_t>         gSlideDy(0x80078074);           // How much the player is wanting to move: y
static const VmPtr<fixed_t[4]>      gEndBox(0x80097BF0);            // Bounding box for the proposed movement
static const VmPtr<fixed_t>         gBlockFrac(0x80078228);         // Percentage of the current move allowed
static const VmPtr<fixed_t>         gBlockNvx(0x800781A8);          // The vector to slide along for the line collided with: x
static const VmPtr<fixed_t>         gBlockNvy(0x800781B0);          // The vector to slide along for the line collided with: y
static const VmPtr<VmPtr<line_t>>   gpSpecialLine(0x80077F9C);      // A special line that was crossed during player movement
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

void P_SlideMove() noexcept {
loc_8002502C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7ED8);                               // Load from: gpSlideThing (80077ED8)
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    s2 = lw(v0 + 0x48);
    s3 = lw(v0 + 0x4C);
    v1 = lw(v0);
    a0 = lw(v0 + 0x4);
    v0 = lw(v0 + 0x64);
    v0 &= 0x1000;
    sw(v1, gp + 0x9B0);                                 // Store to: gSlideX (80077F90)
    sw(a0, gp + 0x9B4);                                 // Store to: gSlideY (80077F94)
    s0 = 0x10000;                                       // Result = 00010000
    if (v0 == 0) goto loc_800250AC;
    a0 = s0;                                            // Result = 00010000
    goto loc_800250E0;
loc_80025084:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7ED8);                               // Load from: gpSlideThing (80077ED8)
    a0 = lw(v0);
    a1 = lw(v0 + 0x4);
    sw(s2, v0 + 0x48);
    sw(s3, v0 + 0x4C);
    SL_CheckSpecialLines(a0, a1, a2, a3);
    goto loc_80025198;
loc_800250AC:
    s4 = 0;                                             // Result = 00000000
    a0 = s2;
loc_800250B4:
    a1 = s3;
    P_CompletableFrac();
    s0 = v0;
    v0 = 0x10000;                                       // Result = 00010000
    if (s0 == v0) goto loc_800250D0;
    s0 -= 0x1000;
loc_800250D0:
    a0 = s0;
    if (i32(s0) >= 0) goto loc_800250E0;
    s0 = 0;                                             // Result = 00000000
    a0 = s0;                                            // Result = 00000000
loc_800250E0:
    a1 = s2;
    _thunk_FixedMul();
    a0 = s0;
    a1 = s3;
    s1 = v0;
    _thunk_FixedMul();
    v1 = lw(gp + 0x9B0);                                // Load from: gSlideX (80077F90)
    a0 = v0;
    a2 = s1 + v1;
    v1 = lw(gp + 0x9B4);                                // Load from: gSlideY (80077F94)
    v0 = 0x10000;                                       // Result = 00010000
    sw(a2, gp + 0x9B0);                                 // Store to: gSlideX (80077F90)
    a3 = a0 + v1;
    sw(a3, gp + 0x9B4);                                 // Store to: gSlideY (80077F94)
    if (s0 == v0) goto loc_80025084;
    s3 -= a0;
    a0 = s2 - s1;
    a1 = lw(gp + 0xBC8);                                // Load from: gBlockNvx (800781A8)
    s4++;
    _thunk_FixedMul();
    s0 = v0;
    a1 = lw(gp + 0xBD0);                                // Load from: gBlockNvy (800781B0)
    a0 = s3;
    _thunk_FixedMul();
    s0 += v0;
    a1 = lw(gp + 0xBC8);                                // Load from: gBlockNvx (800781A8)
    a0 = s0;
    _thunk_FixedMul();
    s2 = v0;
    a1 = lw(gp + 0xBD0);                                // Load from: gBlockNvy (800781B0)
    a0 = s0;
    _thunk_FixedMul();
    s3 = v0;
    v0 = (i32(s4) < 3);
    a0 = s2;
    if (v0 != 0) goto loc_800250B4;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7ED8);                               // Load from: gpSlideThing (80077ED8)
    v1 = lw(v0);
    a0 = lw(v0 + 0x4);
    sw(0, v0 + 0x4C);
    sw(0, v0 + 0x48);
    sw(v1, gp + 0x9B0);                                 // Store to: gSlideX (80077F90)
    sw(a0, gp + 0x9B4);                                 // Store to: gSlideY (80077F94)
loc_80025198:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_CompletableFrac() noexcept {
loc_800251BC:
    sp -= 0x40;
    a2 = a0;
    v0 = 0x10000;                                       // Result = 00010000
    v1 = lw(gp + 0x9B4);                                // Load from: gSlideY (80077F94)
    a0 = 0x170000;                                      // Result = 00170000
    sw(ra, sp + 0x3C);
    sw(s6, sp + 0x38);
    sw(s5, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    sw(v0, gp + 0xC48);                                 // Store to: gBlockFrac (80078228)
    sw(a2, gp + 0xA90);                                 // Store to: gSlideDx (80078070)
    sw(a1, gp + 0xA94);                                 // Store to: gSlideDy (80078074)
    v0 = v1 + a0;
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7BF0);                                // Store to: gEndBox[0] (80097BF0)
    v0 = lw(gp + 0x9B0);                                // Load from: gSlideX (80077F90)
    v1 -= a0;
    at = 0x80090000;                                    // Result = 80090000
    sw(v1, at + 0x7BF4);                                // Store to: gEndBox[1] (80097BF4)
    v1 = v0 + a0;
    v0 -= a0;
    at = 0x80090000;                                    // Result = 80090000
    sw(v1, at + 0x7BFC);                                // Store to: gEndBox[3] (80097BFC)
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7BF8);                                // Store to: gEndBox[2] (80097BF8)
    v0 += a2;
    if (i32(a2) <= 0) goto loc_8002524C;
    v0 = a2 + v1;
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7BFC);                                // Store to: gEndBox[3] (80097BFC)
    goto loc_80025254;
loc_8002524C:
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7BF8);                                // Store to: gEndBox[2] (80097BF8)
loc_80025254:
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7BF4;                                       // Result = gEndBox[1] (80097BF4)
    if (i32(a1) <= 0) goto loc_8002526C;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7BF0;                                       // Result = gEndBox[0] (80097BF0)
loc_8002526C:
    v0 = lw(v1);
    v0 += a1;
    sw(v0, v1);
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7BF8);                               // Load from: gEndBox[2] (80097BF8)
    a0 = *gBlockmapOriginX;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7BFC);                               // Load from: gEndBox[3] (80097BFC)
    v0 -= a0;
    a1 = u32(i32(v0) >> 23);
    v1 -= a0;
    s5 = u32(i32(v1) >> 23);
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7BF4);                               // Load from: gEndBox[1] (80097BF4)
    a0 = *gBlockmapOriginY;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7BF0);                               // Load from: gEndBox[0] (80097BF0)
    v0 -= a0;
    s6 = u32(i32(v0) >> 23);
    v1 -= a0;
    v0 = *gValidCount;
    v0++;
    *gValidCount = v0;
    s3 = u32(i32(v1) >> 23);
    if (i32(a1) >= 0) goto loc_800252EC;
    a1 = 0;                                             // Result = 00000000
loc_800252EC:
    if (i32(s6) >= 0) goto loc_800252F8;
    s6 = 0;                                             // Result = 00000000
loc_800252F8:
    v1 = *gBlockmapWidth;
    v0 = (i32(s5) < i32(v1));
    if (v0 != 0) goto loc_80025314;
    s5 = v1 - 1;
loc_80025314:
    v1 = *gBlockmapHeight;
    v0 = (i32(s3) < i32(v1));
    s2 = a1;
    if (v0 != 0) goto loc_80025330;
    s3 = v1 - 1;
loc_80025330:
    v0 = (i32(s5) < i32(s2));
    if (v0 != 0) goto loc_80025410;
    v0 = (i32(s3) < i32(s6));
loc_80025340:
    s1 = s6;
    if (v0 != 0) goto loc_80025400;
loc_80025348:
    v0 = *gBlockmapWidth;
    mult(s1, v0);
    v1 = *gpBlockmap;
    v0 = lo;
    v0 += s2;
    v0 <<= 1;
    v0 += v1;
    v0 = lh(v0);
    v1 = *gpBlockmapLump;
    v0 <<= 1;
    s0 = v0 + v1;
    v0 = -1;                                            // Result = FFFFFFFF
    v1 = lh(s0);
    a0 = lhu(s0);
    {
        const bool bJump = (v1 == v0);
        v1 = a0 << 16;
        if (bJump) goto loc_800253F0;
    }
    s4 = -1;                                            // Result = FFFFFFFF
loc_8002539C:
    v1 = u32(i32(v1) >> 16);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v1 = *gpLines;
    v0 <<= 2;
    a0 = v0 + v1;
    v0 = lw(a0 + 0x40);
    v1 = *gValidCount;
    s0 += 2;
    if (v0 == v1) goto loc_800253E0;
    sw(v1, a0 + 0x40);
    SL_CheckLine(*vmAddrToPtr<line_t>(a0));
loc_800253E0:
    v0 = lh(s0);
    a0 = lhu(s0);
    v1 = a0 << 16;
    if (v0 != s4) goto loc_8002539C;
loc_800253F0:
    s1++;
    v0 = (i32(s3) < i32(s1));
    if (v0 == 0) goto loc_80025348;
loc_80025400:
    s2++;
    v0 = (i32(s5) < i32(s2));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s3) < i32(s6));
        if (bJump) goto loc_80025340;
    }
loc_80025410:
    v1 = lw(gp + 0xC48);                                // Load from: gBlockFrac (80078228)
    v0 = (i32(v1) < 0x1000);
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8002542C;
    }
    v0 = v1;
    goto loc_80025434;
loc_8002542C:
    sw(0, gp + 0xC48);                                  // Store to: gBlockFrac (80078228)
    sw(0, gp + 0x9BC);                                  // Store to: gpSpecialLine (80077F9C)
loc_80025434:
    ra = lw(sp + 0x3C);
    s6 = lw(sp + 0x38);
    s5 = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x40;
    return;
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

void CheckLineEnds() noexcept {
    v1 = lw(gp + 0xB14);                                // Load from: gP3y (800780F4)
    v0 = lw(gp + 0xAF4);                                // Load from: gP1y (800780D4)
    sp -= 0x28;
    sw(s2, sp + 0x18);
    s2 = lw(gp + 0xAFC);                                // Load from: gP3x (800780DC)
    a0 = lw(gp + 0xAEC);                                // Load from: gP1x (800780CC)
    sw(s0, sp + 0x10);
    s0 = lw(gp + 0xB1C);                                // Load from: gP4y (800780FC)
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s1, sp + 0x14);
    s3 = v0 - v1;
    a0 -= s2;
    s0 -= v1;
    v0 = lw(gp + 0xB10);                                // Load from: gP4x (800780F0)
    a1 = s0;
    s2 -= v0;
    _thunk_FixedMul();
    s1 = v0;
    a0 = s3;
    a1 = s2;
    _thunk_FixedMul();
    s1 += v0;
    a1 = s0;
    a2 = lw(gp + 0xAF0);                                // Load from: gP2x (800780D0)
    v1 = lw(gp + 0xB00);                                // Load from: gP2y (800780E0)
    v0 = lw(gp + 0xB14);                                // Load from: gP3y (800780F4)
    a0 = lw(gp + 0xAFC);                                // Load from: gP3x (800780DC)
    s3 = v1 - v0;
    a0 = a2 - a0;
    _thunk_FixedMul();
    s0 = v0;
    a0 = s3;
    a1 = s2;
    _thunk_FixedMul();
    s0 += v0;
    s1 = ~s1;
    s1 >>= 31;
    s0 >>= 31;
    v0 = s1 ^ s0;
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

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
void SL_ClipToLine() noexcept {
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
void SL_CheckLine(line_t& line) noexcept {
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
void SL_CheckSpecialLines(const fixed_t moveX1, const fixed_t moveY1, const fixed_t moveX2, const fixed_t moveY2) noexcept {
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
