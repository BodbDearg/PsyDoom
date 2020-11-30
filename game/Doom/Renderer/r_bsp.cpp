#include "r_bsp.h"

#include "Asserts.h"
#include "Doom/Base/i_main.h"
#include "Doom/d_main.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_setup.h"
#include "PsyQ/LIBGTE.h"
#include "r_local.h"
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
static bool gbSolidCols[SCREEN_W];

//------------------------------------------------------------------------------------------------------------------------------------------
// Do BSP tree traversal (starting at the root node) to build up the list of subsectors to draw
//------------------------------------------------------------------------------------------------------------------------------------------
void R_BSP() noexcept {
    // Initially all screen columns are fully not occluded by geometry
    D_memset(gbSolidCols, std::byte(0), sizeof(gbSolidCols));

    // The subsector draw list is also initially empty and the sky not visible
    gppEndDrawSubsector = gpDrawSubsectors;
    gbIsSkyVisible = false;

    // Traverse the BSP tree to generate the list of subsectors to draw
    const int32_t bsproot = gNumBspNodes - 1;
    R_RenderBSPNode(bsproot);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does recursive traversal of the BSP tree to prepare a list of subsectors to draw.
// Contrary to the name this function doesn't do the actual drawing, just most of the prep work.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_RenderBSPNode(const int32_t bspnum) noexcept {
    // Is this node number a subsector?
    // If so then process it for potential drawing:
    if (bspnum & NF_SUBSECTOR) {
        // Note: this strange check is in the PC engine too...
        // Under what circumstances can the node number be '-1'?
        if (bspnum == -1) {
            R_Subsector(0);
        } else {
            R_Subsector(bspnum & (~NF_SUBSECTOR));
        }
    } else {
        // This is not a subsector, continue traversing the BSP tree.
        // Only stop when a particular node is determined to be not visible.
        node_t& node = gpBspNodes[bspnum];

        // Compute which side of the line the point is on using the cross product.
        // This is pretty much the same code found in 'R_PointOnSide':
        const int32_t dx = gViewX - node.line.x;
        const int32_t dy = gViewY - node.line.y;
        const int32_t lprod = d_fixed_to_int(node.line.dx) * d_fixed_to_int(dy);
        const int32_t rprod = d_fixed_to_int(node.line.dy) * d_fixed_to_int(dx);

        // Depending on which side of the halfspace we are on, reverse the traversal order:
        if (lprod < rprod) {
            if (R_CheckBBox(node.bbox[0])) {
                R_RenderBSPNode(node.children[0]);
            }

            if (R_CheckBBox(node.bbox[1])) {
                R_RenderBSPNode(node.children[1]);
            }
        } else {
            if (R_CheckBBox(node.bbox[1])) {
                R_RenderBSPNode(node.children[1]);
            }
            
            if (R_CheckBBox(node.bbox[0])) {
                R_RenderBSPNode(node.children[0]);
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks if the given bounding box for a BSP tree node is potentially visible.
// Returns 'false' if the bounding box is NOT potentially visible.
//------------------------------------------------------------------------------------------------------------------------------------------
bool R_CheckBBox(const fixed_t bspcoord[4]) noexcept {
    // Determine where the viewpoint lies in relation to the bsp node bounding box on the x and y dimensions.
    // Determine if the point is inside the box or which outside region it is in, if outside.
    int32_t boxx;

    if (gViewX < bspcoord[BOXLEFT]) {
        boxx = 0;   // Left
    } else if (gViewX <= bspcoord[BOXRIGHT]) {
        boxx = 1;   // Inside
    } else {
        boxx = 2;   // Right
    }

    int32_t boxy;

    if (gViewY > bspcoord[BOXTOP]) {
        boxy = 0;   // Above
    } else if (gViewY >= bspcoord[BOXBOTTOM]) {
        boxy = 1;   // Inside
    } else {
        boxy = 2;   // Below
    }

    // Build a lookup table code from our position relative to the two box dimensions.
    // The LUT determines which points to compare against.
    const int32_t boxpos = boxx + d_lshift<2>(boxy);
    
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
                bSkipVisCheck = (gViewY - y1 <= SKIPVIS_FUDGE);
            } else {
                // Above, Left or Right
                bSkipVisCheck = false;
            }
        } else if (boxpos == 6) {
            // Inside, Right
            bSkipVisCheck = (gViewX - x1 <= SKIPVIS_FUDGE);
        } else if (boxpos == 9) {
            // Below, Inside
            bSkipVisCheck = (gViewY - y1 >= -SKIPVIS_FUDGE);
        } else {
            // Below, Left or Right
            bSkipVisCheck = false;
        }
    } else {
        // Inside, Left
        bSkipVisCheck = (gViewX - x1 >= -SKIPVIS_FUDGE);
    }

    // If we decided to skip the check just assume the node is visible
    if (bSkipVisCheck)
        return true;

    // Transform the extent points of the bounding box using the GTE
    int32_t vx1, vy1;
    int32_t vx2, vy2;

    {
        const SVECTOR vecIn = {
            (int16_t) d_fixed_to_int(x1 - gViewX),
            0,
            (int16_t) d_fixed_to_int(y1 - gViewY)
        };
        int32_t flagsOut;
        VECTOR vecOut;

        LIBGTE_RotTrans(vecIn, vecOut, flagsOut);
        vx1 = vecOut.vx;
        vy1 = vecOut.vz;
    }
    {
        const SVECTOR vecIn = {
            (int16_t) d_fixed_to_int(x2 - gViewX),
            0,
            (int16_t) d_fixed_to_int(y2 - gViewY)
        };
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

    // Is P1 outside the LEFT view frustrum plane and P2 inside?
    // If so then clip against the LEFT plane:
    if ((-vx1 > vy1) && (-vx2 < vy2)) {
        const int32_t a = vx1 + vy1;
        const int32_t b = -vx2 - vy2;
        const fixed_t t = d_int_to_fixed(a) / (a + b);

        // Compute the 'y' value adjustment for the point and convert back to integer coords.
        // Also since the FOV is 90, the new 'x' value will simply be either +/- 'y' depending on the frustrum side:
        const fixed_t yAdjust = (vy2 - vy1) * t;
        vy1 += d_fixed_to_int(yAdjust);
        vx1 = -vy1;
    }

    // Is P2 outside the RIGHT view frustrum plane and P1 inside?
    // If so then clip against the RIGHT plane:
    if ((vx1 < vy1) && (vx2 > vy2)) {
        const int32_t a = vx1 - vy1;
        const int32_t b = -vx2 + vy2;
        const fixed_t t = d_int_to_fixed(a) / (a + b);

        // Compute the 'y' value adjustment for the point and convert back to integer coords.
        // Also since the FOV is 90, the new 'x' value will simply be either +/- 'y' depending on the frustrum side:
        const fixed_t yAdjust = (vy2 - vy1) * t;
        vy2 = vy1 + d_fixed_to_int(yAdjust);
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
    
    // PsyDoom: sometimes 'begX' here ends up offscreen - added an extra safety check to avoid overstepping array bounds:
    #if PSYDOOM_MODS
        if (begX >= SCREEN_W)
            return false;
    #endif
    
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Do preparation for drawing on the given subsector
//------------------------------------------------------------------------------------------------------------------------------------------
void R_Subsector(const int32_t subsecNum) noexcept {
    // Bad subsector number?
    if (subsecNum >= gNumSubsectors) {
        I_Error("R_Subsector: ss %i with numss = %i", subsecNum, gNumSubsectors);
    }
    
    // If we've hit the limit for the number of subsectors that can be drawn then stop emitting.
    // Otherwise add the subsector to the draw list.
    if (gppEndDrawSubsector - gpDrawSubsectors >= MAX_DRAW_SUBSECTORS)
        return;
    
    subsector_t& subsec = gpSubsectors[subsecNum];
    gpCurDrawSector = subsec.sector;
    *gppEndDrawSubsector = &subsec;
    gppEndDrawSubsector++;
    
    // Do draw preparation on all of the segs in the subsector.
    // This figures out what areas of the screen they occlude, updates transformed vertex positions, and more...
    seg_t* pSeg = &gpSegs[subsec.firstseg];

    for (int32_t segsleft = subsec.numsegs; segsleft > 0; --segsleft) {
        R_AddLine(*pSeg);
        ++pSeg;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do draw preparation for the given line segment.
// Also marks any columns that it occludes fully.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_AddLine(seg_t& seg) noexcept {
    // Initially assume no columns in the seg are visible
    seg.flags &= (~SGF_VISIBLE_COLS);
    
    // Transform the 2 seg vertices if required.
    // Transform into view space but also figure out screen X and scaling due to perspective.
    vertex_t& segv1 = *seg.vertex1;
    
    if (segv1.frameUpdated != gNumFramesDrawn) {
        const SVECTOR viewToPt = {
            (int16_t) d_fixed_to_int(segv1.x - gViewX),
            0,
            (int16_t) d_fixed_to_int(segv1.y - gViewY)
        };
        
        VECTOR viewVec;
        int32_t rotFlags;
        LIBGTE_RotTrans(viewToPt, viewVec, rotFlags);

        segv1.viewx = viewVec.vx;
        segv1.viewy = viewVec.vz;
        
        // Do perspective division if the point is not too close.
        // This check seems slightly incorrect, should be maybe be '>= NEAR_CLIP_DIST' instead?
        if (viewVec.vz > NEAR_CLIP_DIST + 1) {
            segv1.scale = (HALF_SCREEN_W * FRACUNIT) / viewVec.vz;
            segv1.screenx = d_fixed_to_int(viewVec.vx * segv1.scale) + HALF_SCREEN_W;
        }
        
        segv1.frameUpdated = gNumFramesDrawn;
    }
    
    vertex_t& segv2 = *seg.vertex2;
    
    if (segv2.frameUpdated != gNumFramesDrawn) {
        const SVECTOR viewToPt = {
            (int16_t) d_fixed_to_int(segv2.x - gViewX),
            0,
            (int16_t) d_fixed_to_int(segv2.y - gViewY)
        };
        
        VECTOR viewVec;
        int32_t rotFlags;
        LIBGTE_RotTrans(viewToPt, viewVec, rotFlags);
        
        segv2.viewx = viewVec.vx;
        segv2.viewy = viewVec.vz;

        // Do perspective division if the point is not too close.
        // This check seems slightly incorrect, should be maybe be '>= NEAR_CLIP_DIST' instead?
        if (viewVec.vz > NEAR_CLIP_DIST + 1) {
            segv2.scale = (HALF_SCREEN_W * FRACUNIT) / viewVec.vz;
            segv2.screenx = d_fixed_to_int(viewVec.vx * segv2.scale) + HALF_SCREEN_W;
        }
        
        segv2.frameUpdated = gNumFramesDrawn;
    }
    
    // Store the clipped line segment points here: initially the line is unclipped
    int32_t v1x = segv1.viewx;
    int32_t v1y = segv1.viewy;
    int32_t v2x = segv2.viewx;
    int32_t v2y = segv2.viewy;
    
    // Are both line segment points outside the LEFT view frustrum plane?
    if ((-v1x > v1y) && (-v2x > v2y))
        return;
    
    // Are both line segment points outside the RIGHT view frustrum plane?
    if ((v1x > v1y) && (v2x > v2y))
        return;
    
    // Use 'v2 x v1' (vector cross product) to determine the winding order for the line segment.
    // If the line segment is back facing then we ignore it.
    {
        const int32_t cross = (v2x * v1y) - (v1x * v2y);
        
        if (cross <= 0)
            return;
    }
    
    //--------------------------------------------------------------------------------------------------------------------------------------
    // Lots of line/plane clipping code follows below...
    // For more details on how the clipping works, see the comments in 'R_CheckBBox'.
    //--------------------------------------------------------------------------------------------------------------------------------------
    
    // Is P1 outside the LEFT view frustrum plane and P2 inside?
    // If so then clip against the LEFT plane:
    if ((-v1x > v1y) && (-v2x < v2y)) {
        const int32_t a = v1x + v1y;
        const int32_t b = -v2x - v2y;
        const fixed_t t = d_int_to_fixed(a) / (a + b);
        
        const fixed_t yAdjust = (v2y - v1y) * t;
        v1y += d_fixed_to_int(yAdjust);
        v1x = -v1y;
    }
    
    // Is P2 outside the RIGHT view frustrum plane and P1 inside?
    // If so then clip against the RIGHT plane:
    if ((v1x < v1y) && (v2x > v2y)) {
        const int32_t a = v1x - v1y;
        const int32_t b = -v2x + v2y;
        const fixed_t t = d_int_to_fixed(a) / (a + b);
        
        const fixed_t yAdjust = (v2y - v1y) * t;
        v2y = v1y + d_fixed_to_int(yAdjust);
        v2x = v2y;
    }
    
    // If both points are at or before the near plane after clipping then ignore the seg
    if (v1y <= NEAR_CLIP_DIST && v2y <= NEAR_CLIP_DIST)
        return;
    
    // Clip against the front plane if one of the points is on the inside and the other on the outside.
    // Note that clipping is slightly over eager and clips a little after the near plane.
    // This is probably due to accuracy issues...
    if ((v1y < NEAR_CLIP_DIST) && (v2y >= NEAR_CLIP_DIST + 1)) {
        const int32_t a = NEAR_CLIP_DIST - v1y;
        const int32_t b = v2y - NEAR_CLIP_DIST;
        const fixed_t t = d_int_to_fixed(a) / (a + b);
    
        const fixed_t xAdjust = (v2x - v1x) * t;
        v1x += d_fixed_to_int(xAdjust);
        v1y = NEAR_CLIP_DIST;
    }
    else if ((v2y < NEAR_CLIP_DIST) && (v1y >= NEAR_CLIP_DIST + 1)) {
        const int32_t a = NEAR_CLIP_DIST - v2y;
        const int32_t b = v1y - NEAR_CLIP_DIST;
        const fixed_t t = d_int_to_fixed(a) / (a + b);
        
        const fixed_t xAdjust = (v1x - v2x) * t;
        v2x += d_fixed_to_int(xAdjust);
        v2y = NEAR_CLIP_DIST;
    }
    
    // Do perspective division to compute the screen space start and end x values for the line seg.
    // Scale the result accordingly, noting at at this point the x values have a range of +/- HALF_SCREEN_W.
    int32_t begX = (v1x * HALF_SCREEN_W) / v1y;
    int32_t endX = (v2x * HALF_SCREEN_W) / v2y;
    
    // Bring into the range 0-SCREEN_W
    begX += HALF_SCREEN_W;
    endX += HALF_SCREEN_W;
    
    // If the seg is zero sized then ignore.
    // PsyDoom: change this to discard a malformed line also (end before beg)
    #if PSYDOOM_MODS
        if (begX >= endX)
            return;
    #else
        if (begX == endX)
            return;
    #endif

    // If there is no ceiling texture at this seg then the sky is visible and needs to be drawn
    if (gpCurDrawSector->ceilingpic == -1) {
        gbIsSkyVisible = true;
    }
    
    // Clamp the x values to the screen range
    if (begX < 0) {
        begX = 0;
    }
    
    if (endX > SCREEN_W) {
        endX = SCREEN_W;
    }
    
    // PsyDoom: sometimes 'begX' here ends up offscreen - added extra safety checks to avoid overstepping array bounds.
    // Also discard the line if the clamping above has reduced it to size zero (or negative!).
    #if PSYDOOM_MODS
        if ((begX >= SCREEN_W) || (endX <= begX))
            return;
    #endif
    
    // Determine the first visible (not fully occluded) column of the seg
    int32_t visibleBegX = SCREEN_W;
    
    {
        const bool* pbIsSolidCol = &gbSolidCols[begX];
        
        for (int32_t x = begX; x < endX; ++x, ++pbIsSolidCol) {
            if (!(*pbIsSolidCol)) {
                visibleBegX = x;
                break;
            }
        }
    }

    // Determine the last visible (not fully occluded) column of the seg
    int32_t visibleEndX = 0;
    
    {
        const bool* pbIsSolidCol = &gbSolidCols[endX - 1];
        
        for (int32_t x = endX - 1; x >= begX; --x, --pbIsSolidCol) {
            if (!(*pbIsSolidCol)) {
                visibleEndX = x;
                break;
            }
        }
    }

    // Save the visible column range and seg flags if at least 1 column is visible
    if (visibleEndX >= visibleBegX) {
        // Presumably the +/- 1 here are to try and paper over some of the cracks/inaccuracies?
        seg.visibleBegX = (int16_t)(visibleBegX - 1);
        seg.visibleEndX = (int16_t)(visibleEndX + 1);
        seg.flags |= SGF_VISIBLE_COLS;
    }
    
    // Mark any columns that this seg fully occludes as 'solid'.
    // Only do this however if the seg is not drawn with translucent parts or transparency:
    if ((seg.linedef->flags & ML_MIDMASKED) == 0) {
        const sector_t& frontSec = *gpCurDrawSector;
        const sector_t* const pBackSec = seg.backsector;
        
        // Is this seg something that fully occludes stuff behind it?
        // This will be the case if the seg is one sided (no back sector) or if there is no height gap between sectors.
        if ((!pBackSec) ||
            (frontSec.floorheight >= pBackSec->ceilingheight) ||
            (pBackSec->floorheight >= frontSec.ceilingheight)
        ) {
            // This seg occludes, mark all of it's columns as occluders.
            //
            // Note: a minor optimization was also missed here in the original code, we could have just set the
            // previously determined visible x range to be occluded, rather than the entire seg x range.
            // Probably doesn't make a huge difference in reality though..
            //
            bool* pbIsSolidCol = &gbSolidCols[begX];
            
            for (int32_t x = begX; x < endX; ++x, ++pbIsSolidCol) {
                *pbIsSolidCol = true;
            }
        }
    }
}
