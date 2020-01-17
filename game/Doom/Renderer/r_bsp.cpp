#include "r_bsp.h"

#include "Doom/Base/i_main.h"
#include "Doom/d_main.h"
#include "Doom/Game/p_setup.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGTE.h"
#include "r_main.h"

// Used by 'R_CheckBBox' to determine which BSP node bounding box coordinates to check against.
// The row index is determined by the position of the view point relative to the bounding box.
// The column index determines the particular bound or coordinate desired.
static const int32_t gCheckcoord[12][4] = {
    { 3, 0, 2, 1 },     // 0:   Above,  Left
    { 3, 0, 2, 0 },     // 1:   Above,  Inside
    { 3, 1, 2, 0 },     // 2:   Above,  Right
    { 0, 0, 0, 0 },     // 3:   -
    { 2, 0, 2, 1 },     // 4:   Inside, Left
    { 0, 0, 0, 0 },     // 5:   Inside, Inside
    { 3, 1, 3, 0 },     // 6:   Inside, Right
    { 0, 0, 0, 0 },     // 7:   -
    { 2, 0, 3, 1 },     // 8:   Below,  Left
    { 2, 1, 3, 1 },     // 9:   Below,  Inside
    { 2, 1, 3, 0 },     // 10:  Below,  Right
    { 0, 0, 0, 0 }      // 11:  -
};

// Which screen columns are fully occluded by geometry
static const VmPtr<bool[SCREEN_W]> gbSolidCols(0x800A8F48);

void R_BSP() noexcept {
    // TODO: what is this buffer?
    a0 = 0x800A8F48;                                    // Result = gbSolidCols (800A8F48)
    a1 = 0;                                             // Result = 00000000
    a2 = 0x100;                                         // Result = 00000100
    _thunk_D_memset();

    // The subsector draw list is initially empty and the sky not visible
    *gppEndDrawSubsector = gpDrawSubsectors;
    *gbIsSkyVisible = false;

    // Traverse the BSP tree to generate the list of subsectors to draw
    a0 = *gNumBspNodes - 1;
    R_RenderBSPNode();
}

void R_RenderBSPNode() noexcept {
loc_8002AD3C:
    sp -= 0x18;
    v1 = a0;
    v0 = v1 & 0x8000;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_8002AD84;
    v0 = -1;                                            // Result = FFFFFFFF
    a0 = 0xFFFF0000;                                    // Result = FFFF0000
    if (v1 != v0) goto loc_8002AD70;
    a0 = 0;                                             // Result = 00000000
    R_Subsector();
    goto loc_8002AE60;
loc_8002AD70:
    a0 |= 0x7FFF;                                       // Result = FFFF7FFF
    a0 &= v1;
    R_Subsector();
    goto loc_8002AE60;
loc_8002AD84:
    v0 = v1 << 3;
    v0 -= v1;
    v1 = *gpBspNodes;
    v0 <<= 3;
    s0 = v0 + v1;   // bspnode
    v0 = *gViewY;
    v1 = lw(s0 + 0x4);
    v0 -= v1;
    v1 = lh(s0 + 0xA);
    v0 = u32(i32(v0) >> 16);
    mult(v0, v1);
    v0 = *gViewX;
    v1 = lw(s0);
    v0 -= v1;
    v1 = lo;
    a0 = lh(s0 + 0xE);
    v0 = u32(i32(v0) >> 16);
    mult(a0, v0);
    v0 = lo;
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_8002AE28;
    a0 = s0 + 0x10;
    v0 = R_CheckBBox(vmAddrToPtr<fixed_t>(a0));
    if (v0 == 0) goto loc_8002AE0C;
    a0 = lw(s0 + 0x30);
    R_RenderBSPNode();
loc_8002AE0C:
    a0 = s0 + 0x20;
    v0 = R_CheckBBox(vmAddrToPtr<fixed_t>(a0));
    if (v0 == 0) goto loc_8002AE60;
    a0 = lw(s0 + 0x34);
    goto loc_8002AE58;
loc_8002AE28:
    a0 = s0 + 0x20;
    v0 = R_CheckBBox(vmAddrToPtr<fixed_t>(a0));
    if (v0 == 0) goto loc_8002AE44;
    a0 = lw(s0 + 0x34);
    R_RenderBSPNode();
loc_8002AE44:
    a0 = s0 + 0x10;
    v0 = R_CheckBBox(vmAddrToPtr<fixed_t>(a0));
    if (v0 == 0) goto loc_8002AE60;
    a0 = lw(s0 + 0x30);
loc_8002AE58:
    R_RenderBSPNode();
loc_8002AE60:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks if the given bounding box for a BSP tree node is potentially visible.
// Returns 'false' if the bounding box is NOT potentially visible.
//------------------------------------------------------------------------------------------------------------------------------------------
bool R_CheckBBox(const fixed_t bspcoord[4]) noexcept {
    // Determine where the viewpoint lies in relation to the bsp node bounding box on the x and y dimensions.
    // Determine if the point is inside the box or which outside region it is in, if outside.
    int32_t boxx;

    if (*gViewX < bspcoord[BOXLEFT]) {
        boxx = 0;   // Left
    } else if (*gViewX <= bspcoord[BOXRIGHT]) {
        boxx = 1;   // Inside
    } else {
        boxx = 2;   // Right
    }

    int32_t boxy;

    if (*gViewY > bspcoord[BOXTOP]) {
        boxy = 0;   // Above
    } else if (*gViewY >= bspcoord[BOXBOTTOM]) {
        boxy = 1;   // Inside
    } else {
        boxy = 2;   // Below
    }

    // Build a lookup table code from our position relative to the two box dimensions.
    // The LUT determines which points to compare against.
    const int32_t boxpos = boxx + (boxy << 2);
    
    // If we are fully inside the BSP node bounding box then regard it as always visible
    if (boxpos == 5)
        return true;

    // Read the coords we need for visibility checks and determine if we can skip the check.
    // If we are close to the bounds then skip the check in some cases, if we are not in one of the diagonal spaces.
    // The fudge factor might have been added to account for renderer inaccuracies maybe?
    const fixed_t x1 = bspcoord[gCheckcoord[boxpos][0]];
    const fixed_t y1 = bspcoord[gCheckcoord[boxpos][1]];
    const fixed_t x2 = bspcoord[gCheckcoord[boxpos][2]];
    const fixed_t y2 = bspcoord[gCheckcoord[boxpos][3]];

    constexpr fixed_t SKIPVIS_FUDGE = FRACUNIT * 2;
    bool bSkipVisCheck;

    if (boxpos != 4) {
        if (boxpos < 5) {
            if (boxpos == 1) {
                // Above, Inside
                bSkipVisCheck = (*gViewY - y1 <= SKIPVIS_FUDGE);
            } else {
                // Above, Left or Right
                bSkipVisCheck = false;
            }
        } else if (boxpos == 6) {
            // Inside, Right
            bSkipVisCheck = (*gViewX - x1 <= SKIPVIS_FUDGE);
        } else if (boxpos == 9) {
            // Below, Inside
            bSkipVisCheck = (*gViewY - y1 >= -SKIPVIS_FUDGE);
        } else {
            // Below, Left or Right
            bSkipVisCheck = false;
        }
    } else {
        // Inside, Left
        bSkipVisCheck = (*gViewX - x1 >= -SKIPVIS_FUDGE);
    }

    // If we decided to skip the check just assume the node is visible
    if (bSkipVisCheck)
        return true;

    // Transform the extent points of the bounding box using the GTE
    int32_t vx1, vy1;
    int32_t vx2, vy2;

    {
        const SVECTOR vecIn = { (x1 - *gViewX) >> 16, 0, (y1 - *gViewY) >> 16 };
        int32_t flagsOut;
        VECTOR vecOut;

        LIBGTE_RotTrans(vecIn, vecOut, flagsOut);
        vx1 = vecOut.vx;
        vy1 = vecOut.vz;
    }
    {
        const SVECTOR vecIn = { (x2 - *gViewX) >> 16, 0, (y2 - *gViewY) >> 16 };
        int32_t flagsOut;
        VECTOR vecOut;

        LIBGTE_RotTrans(vecIn, vecOut, flagsOut);
        vx2 = vecOut.vx;
        vy2 = vecOut.vz;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Checks to see if the bounding box is fully outside of the view frustrum.
    // Note: all frustrum checks assume the mathematically simple case of a 90 degree FOV.
    // With a 90 degree FOV we simply check to see if 'x > y' (slope > 1) to determine if a point is outside the frustrum.
    //--------------------------------------------------------------------------------------------------------------------------------------

    // Is the node BB fully outside the view frustrum to the left?
    if ((-vx1 > vy1) && (-vx2 > vy2)) {        
        return false;   
    }
    
    // Is the node BB fully outside the view frustrum to the right?
    if ((vx1 > vy1) && (vx2 > vy2)) {        
        return false;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Clip the line connecting the two points defining the extent of the node's bounding box.
    // Clip it against the left and right side of the view frustrum respectively.
    // Use fixed point numbers where possible also to improve accuracy.
    //
    // Where a line is defined by 'P0' and 'P1', and a plane (ignoring Z) by 'Ax + By + D = 0',
    // It can be determined that the intersection time 't' of the line against the plane is:
    //
    //  t = (BP0y + AP0x + D) / (BP0y + AP0x - AP1x - BP1y)
    //
    // In the case of the left frustrum for example, by assuming plane normals of '1,1' (i.e a 90 degree FOV)
    // and the plane running through the origin (the viewpoint) then the equation simplifies to the following:
    //
    //  t = (P0y + P0x) / (P0y + P0x - P1x - P1y)
    //--------------------------------------------------------------------------------------------------------------------------------------

    // Is P1 outside the left view frustrum plane and P2 inside?
    // If so then clip against the LEFT plane:
    if ((-vx1 > vy1) && (-vx2 < vy2)) {
        const int32_t a = vx1 + vy1;
        const int32_t b = -vx2 -vy2;
        const fixed_t t = (a << FRACBITS) / (a + b);

        // Compute the 'y' value adjustment for the point and convert back to integer coords.
        // Also since the FOV is 90, the new 'x' value will simply be either +/- 'y' depending on the frustrum side:
        const fixed_t yAdjust = (vy2 - vy1) * t;
        vy1 += yAdjust >> FRACBITS;
        vx1 = -vy1;
    }

    // Is P2 outside the right view frustrum plane and P1 inside?
    // If so then clip against the RIGHT plane:
    if ((vx1 < vy1) && (vx2 > vy2)) {
        const int32_t a = vx1 - vy1;
        const int32_t b = -vx2 + vy2;
        const fixed_t t = (a << FRACBITS) / (a + b);

        // Compute the 'y' value adjustment for the point and convert back to integer coords.
        // Also since the FOV is 90, the new 'x' value will simply be either +/- 'y' depending on the frustrum side:
        const fixed_t yAdjust = (vy2 - vy1) * t;
        vy2 = vy1 + (yAdjust >> FRACBITS);
        vx2 = vy2;
    }

    // If the box is fully behind the camera then do not draw!
    // Not sure why this check isn't done earlier TBH, seems like it should be used for early-out?
    if ((vy1 < 0) && (vy2 < 0))
        return false;

    // If the bounding box depth is close to the camera by a certain fudge factor then draw always
    constexpr int32_t IS_VISIBLE_FUDGE = 2;

    if ((vy1 < IS_VISIBLE_FUDGE) && (vy2 < IS_VISIBLE_FUDGE))
        return true;

    // Prevent division by 0 - clamp depth to '1'
    if (vy1 < 1) {
        vy1 = 1;
    }

    if (vy2 < 1) {
        vy2 = 1;
    }
    
    // Do perspective division to compute the screen space start and end x values of the box.
    // Scale the result accordingly, noting at at this point the x values have a range of +/- HALF_SCREEN_W.
    int32_t begX = (vx1 * HALF_SCREEN_W) / vy1;
    int32_t endX = (vx2 * HALF_SCREEN_W) / vy2;

    // Bring into the range 0-SCREEN_W and clamp to the edges of the screen
    begX += HALF_SCREEN_W;
    endX += HALF_SCREEN_W;

    if (begX < 0) {
        begX = 0;
    }

    if (endX > SCREEN_W) {
        endX = SCREEN_W;
    }
    
    // As a final check, scan through all of the columns for the node's bounding box.
    // At least one bounding box column must NOT be occluded by an already drawn fully solid 1 sided or opaque column.
    // If we don't find any unobscured columns then the BSP node is obscured by blocking geometry, and thus can be ignored:
    {
        const bool* pSolidCol = &gbSolidCols[begX];

        for (int32_t curX = begX; curX < endX; ++curX, ++pSolidCol) {
            if (!pSolidCol[0])
                return true;
        }
    }

    return false;
}

void R_Subsector() noexcept {
loc_8002B2D8:
    a2 = *gNumSubsectors;
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x18);
    v0 = (i32(s0) < i32(a2));
    sw(s1, sp + 0x14);
    if (v0 != 0) goto loc_8002B30C;
    I_Error("R_Subsector: ss %i with numss = %i", (int32_t) s0, (int32_t) a2);
loc_8002B30C:
    a0 = *gppEndDrawSubsector;
    v0 = gpDrawSubsectors;
    v0 = a0 - v0;
    v0 = u32(i32(v0) >> 2);
    v0 = (i32(v0) < 0xC0);
    {
        const bool bJump = (v0 == 0);
        v0 = s0 << 4;
        if (bJump) goto loc_8002B3A0;
    }
    v1 = *gpSubsectors;
    v0 += v1;
    v1 = lw(v0);
    *gpCurDrawSector = v1;
    sw(v0, a0);
    a0 = lh(v0 + 0x6);
    s0 = lh(v0 + 0x4);
    v0 = *gppEndDrawSubsector;
    v1 = a0 << 2;
    v1 += a0;
    v1 <<= 3;
    a0 = *gpSegs;
    v0 += 4;
    *gppEndDrawSubsector = v0;
    s1 = v1 + a0;
    if (s0 == 0) goto loc_8002B3A0;
loc_8002B38C:
    a0 = s1;
    R_AddLine();
    s0--;
    s1 += 0x28;
    if (s0 != 0) goto loc_8002B38C;
loc_8002B3A0:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void R_AddLine() noexcept {
loc_8002B3B8:
    sp -= 0x48;
    sw(s3, sp + 0x3C);
    s3 = a0;
    sw(ra, sp + 0x40);
    sw(s2, sp + 0x38);
    sw(s1, sp + 0x34);
    sw(s0, sp + 0x30);
    v0 = lhu(s3 + 0x20);
    s0 = lw(s3);
    v0 &= 0xFFFE;
    sh(v0, s3 + 0x20);
    v1 = lw(s0 + 0x18);
    v0 = *gNumFramesDrawn;
    a0 = sp + 0x10;
    if (v1 == v0) goto loc_8002B4BC;
    v0 = lw(s0);
    v1 = *gViewX;
    a1 = sp + 0x18;
    sh(0, sp + 0x12);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    sh(v0, sp + 0x10);
    v0 = lw(s0 + 0x4);
    v1 = *gViewY;
    a2 = sp + 0x28;
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    sh(v0, sp + 0x14);
    _thunk_LIBGTE_RotTrans();
    v0 = lw(sp + 0x18);
    sw(v0, s0 + 0xC);
    v1 = lw(sp + 0x20);
    s2 = v0;
    s1 = v1;
    v0 = (i32(s1) < 4);
    sw(s1, s0 + 0x10);
    if (v0 != 0) goto loc_8002B4AC;
    v0 = 0x800000;                                      // Result = 00800000
    div(v0, s1);
    if (s1 != 0) goto loc_8002B474;
    _break(0x1C00);
loc_8002B474:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B48C;
    }
    if (v0 != at) goto loc_8002B48C;
    tge(zero, zero, 0x5D);
loc_8002B48C:
    v0 = lo;
    mult(s2, v0);
    sw(v0, s0 + 0x8);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
loc_8002B4AC:
    v0 = *gNumFramesDrawn;
    sw(v0, s0 + 0x18);
    goto loc_8002B4C4;
loc_8002B4BC:
    s2 = lw(s0 + 0xC);
    s1 = lw(s0 + 0x10);
loc_8002B4C4:
    s0 = lw(s3 + 0x4);
    v0 = *gNumFramesDrawn;
    v1 = lw(s0 + 0x18);
    a0 = sp + 0x10;
    if (v1 == v0) goto loc_8002B5A0;
    v0 = lw(s0);
    v1 = *gViewX;
    a1 = sp + 0x18;
    sh(0, sp + 0x12);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    sh(v0, sp + 0x10);
    v0 = lw(s0 + 0x4);
    v1 = *gViewY;
    a2 = sp + 0x28;
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    sh(v0, sp + 0x14);
    _thunk_LIBGTE_RotTrans();
    v0 = lw(sp + 0x18);
    sw(v0, s0 + 0xC);
    v1 = lw(sp + 0x20);
    a1 = v0;
    a0 = v1;
    v0 = (i32(a0) < 4);
    sw(a0, s0 + 0x10);
    if (v0 != 0) goto loc_8002B590;
    v0 = 0x800000;                                      // Result = 00800000
    div(v0, a0);
    if (a0 != 0) goto loc_8002B558;
    _break(0x1C00);
loc_8002B558:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B570;
    }
    if (v0 != at) goto loc_8002B570;
    tge(zero, zero, 0x5D);
loc_8002B570:
    v0 = lo;
    mult(a1, v0);
    sw(v0, s0 + 0x8);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
loc_8002B590:
    v0 = *gNumFramesDrawn;
    sw(v0, s0 + 0x18);
    goto loc_8002B5A8;
loc_8002B5A0:
    a1 = lw(s0 + 0xC);
    a0 = lw(s0 + 0x10);
loc_8002B5A8:
    v0 = -s1;
    a2 = (i32(s2) < i32(v0));
    v0 = -a0;
    if (a2 == 0) goto loc_8002B5C4;
    v0 = (i32(a1) < i32(v0));
    if (v0 != 0) goto loc_8002B988;
loc_8002B5C4:
    v0 = (i32(s1) < i32(s2));
    mult(a1, s1);
    if (v0 == 0) goto loc_8002B5DC;
    v0 = (i32(a0) < i32(a1));
    if (v0 != 0) goto loc_8002B988;
loc_8002B5DC:
    v0 = lo;
    mult(s2, a0);
    v1 = lo;
    v0 -= v1;
    if (i32(v0) <= 0) goto loc_8002B988;
    v0 = -a0;
    if (a2 == 0) goto loc_8002B658;
    v0 = (i32(v0) < i32(a1));
    {
        const bool bJump = (v0 == 0);
        v0 = s2 + s1;
        if (bJump) goto loc_8002B658;
    }
    v1 = v0 << 16;
    v0 -= a1;
    v0 -= a0;
    div(v1, v0);
    if (v0 != 0) goto loc_8002B624;
    _break(0x1C00);
loc_8002B624:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B63C;
    }
    if (v1 != at) goto loc_8002B63C;
    tge(zero, zero, 0x5D);
loc_8002B63C:
    v1 = lo;
    v0 = a0 - s1;
    mult(v1, v0);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    s1 += v0;
    s2 = -s1;
loc_8002B658:
    v0 = (i32(s2) < i32(s1));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a0) < i32(a1));
        if (bJump) goto loc_8002B6BC;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = s2 - s1;
        if (bJump) goto loc_8002B6BC;
    }
    v1 = v0 << 16;
    v0 -= a1;
    v0 += a0;
    div(v1, v0);
    if (v0 != 0) goto loc_8002B688;
    _break(0x1C00);
loc_8002B688:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B6A0;
    }
    if (v1 != at) goto loc_8002B6A0;
    tge(zero, zero, 0x5D);
loc_8002B6A0:
    v1 = lo;
    v0 = a0 - s1;
    mult(v1, v0);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    a0 = s1 + v0;
    a1 = a0;
loc_8002B6BC:
    v0 = (i32(s1) < 3);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a0) < 3);
        if (bJump) goto loc_8002B6D0;
    }
    if (v0 != 0) goto loc_8002B988;
loc_8002B6D0:
    v0 = (i32(s1) < 2);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a0) < 3);
        if (bJump) goto loc_8002B74C;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a0) < 2);
        if (bJump) goto loc_8002B750;
    }
    v0 = 2;                                             // Result = 00000002
    v0 -= s1;
    v0 <<= 16;
    v1 = a0 - s1;
    div(v0, v1);
    if (v1 != 0) goto loc_8002B704;
    _break(0x1C00);
loc_8002B704:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B71C;
    }
    if (v0 != at) goto loc_8002B71C;
    tge(zero, zero, 0x5D);
loc_8002B71C:
    v0 = lo;
    v1 = a1 - s2;
    mult(v0, v1);
    s1 = 2;                                             // Result = 00000002
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    s2 += v0;
    goto loc_8002B7B4;
loc_8002B73C:
    t0 = a0;
    goto loc_8002B898;
loc_8002B744:
    v1 = a0;
    goto loc_8002B8D4;
loc_8002B74C:
    v0 = (i32(a0) < 2);
loc_8002B750:
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s1) < 3);
        if (bJump) goto loc_8002B7B4;
    }
    v1 = s2 << 7;
    if (v0 != 0) goto loc_8002B7B8;
    v0 = 2;                                             // Result = 00000002
    v0 -= a0;
    v0 <<= 16;
    v1 = s1 - a0;
    div(v0, v1);
    if (v1 != 0) goto loc_8002B780;
    _break(0x1C00);
loc_8002B780:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B798;
    }
    if (v0 != at) goto loc_8002B798;
    tge(zero, zero, 0x5D);
loc_8002B798:
    v0 = lo;
    v1 = s2 - a1;
    mult(v0, v1);
    a0 = 2;                                             // Result = 00000002
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    a1 += v0;
loc_8002B7B4:
    v1 = s2 << 7;
loc_8002B7B8:
    div(v1, s1);
    if (s1 != 0) goto loc_8002B7C8;
    _break(0x1C00);
loc_8002B7C8:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B7E0;
    }
    if (v1 != at) goto loc_8002B7E0;
    tge(zero, zero, 0x5D);
loc_8002B7E0:
    v1 = lo;
    v0 = a1 << 7;
    div(v0, a0);
    if (a0 != 0) goto loc_8002B7F8;
    _break(0x1C00);
loc_8002B7F8:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B810;
    }
    if (v0 != at) goto loc_8002B810;
    tge(zero, zero, 0x5D);
loc_8002B810:
    v0 = lo;
    a2 = v1 + 0x80;
    a3 = v0 + 0x80;
    if (a2 == a3) goto loc_8002B988;
    v0 = *gpCurDrawSector;
    v1 = lw(v0 + 0xC);
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8002B848;
    }
    *gbIsSkyVisible = v0;
loc_8002B848:
    v0 = (i32(a3) < 0x101);
    if (i32(a2) >= 0) goto loc_8002B854;
    a2 = 0;                                             // Result = 00000000
loc_8002B854:
    t0 = 0x100;                                         // Result = 00000100
    if (v0 != 0) goto loc_8002B860;
    a3 = 0x100;                                         // Result = 00000100
loc_8002B860:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x70B8;                                       // Result = gbSolidCols (800A8F48)
    a1 = a2 + v0;
    v0 = (i32(a2) < i32(a3));
    a0 = a2;
    if (v0 == 0) goto loc_8002B898;
loc_8002B878:
    v0 = lbu(a1);
    a1++;
    if (v0 == 0) goto loc_8002B73C;
    a0++;
    v0 = (i32(a0) < i32(a3));
    if (v0 != 0) goto loc_8002B878;
loc_8002B898:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x70B9;                                       // Result = 800A8F47
    a1 = a3 + v0;
    a0 = a3 - 1;
    v0 = (i32(a0) < i32(a2));
    v1 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_8002B8D4;
loc_8002B8B4:
    v0 = lbu(a1);
    a1--;
    if (v0 == 0) goto loc_8002B744;
    a0--;
    v0 = (i32(a0) < i32(a2));
    if (v0 == 0) goto loc_8002B8B4;
loc_8002B8D4:
    v0 = (i32(v1) < i32(t0));
    {
        const bool bJump = (v0 != 0);
        v0 = t0 - 1;                                    // Result = 000000FF
        if (bJump) goto loc_8002B8F8;
    }
    sh(v0, s3 + 0x22);
    v0 = lhu(s3 + 0x20);
    v1++;
    sh(v1, s3 + 0x24);
    v0 |= 1;
    sh(v0, s3 + 0x20);
loc_8002B8F8:
    v0 = lw(s3 + 0x14);
    v0 = lw(v0 + 0x10);
    v0 &= 0x200;
    if (v0 != 0) goto loc_8002B988;
    a0 = lw(s3 + 0x1C);
    if (a0 == 0) goto loc_8002B95C;
    a1 = *gpCurDrawSector;
    v1 = lw(a0 + 0x4);
    v0 = lw(a1);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002B95C;
    v0 = lw(a0);
    v1 = lw(a1 + 0x4);
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_8002B988;
loc_8002B95C:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x70B8;                                       // Result = gbSolidCols (800A8F48)
    a1 = a2 + v0;
    v0 = (i32(a2) < i32(a3));
    v1 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8002B988;
loc_8002B974:
    sb(v1, a1);
    a2++;
    v0 = (i32(a2) < i32(a3));
    a1++;
    if (v0 != 0) goto loc_8002B974;
loc_8002B988:
    ra = lw(sp + 0x40);
    s3 = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x48;
    return;
}
