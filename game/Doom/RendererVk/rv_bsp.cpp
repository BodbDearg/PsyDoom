//------------------------------------------------------------------------------------------------------------------------------------------
// This module is the PsyDoom re-implementation/equivalent of 'r_bsp' in PSX Doom.
// It concerns itself with traversing the BSP tree according to the viewpoint to build an ordered list of draw subsectors.
// The draw subsectors list generated is ordered front to back.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "rv_bsp.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "rv_data.h"
#include "rv_main.h"
#include "rv_occlusion.h"
#include "rv_utils.h"

// This is the list of subsectors to be drawn by the Vulkan renderer, in front to back order
std::vector<subsector_t*> gRvDrawSubsecs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given line segment is occluding for the purposes of visibility testing.
// Occluding segs should mark out areas of the screen that they cover, so that nothing behind draws.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool RV_IsOccludingSeg(const rvseg_t& seg, const sector_t& frontSector) noexcept {
    // One sided lines are always occluding
    if (!seg.backsector)
        return true;

    // Get the mid-wall gap between the front and back sectors
    sector_t& backSector = *seg.backsector;

    const fixed_t fty = frontSector.ceilingheight;
    const fixed_t fby = frontSector.floorheight;
    const fixed_t bty = backSector.ceilingheight;
    const fixed_t bby = backSector.floorheight;
    const fixed_t midTy = std::min(fty, bty);
    const fixed_t midBy = std::max(fby, bby);

    // If there is a gap then the seg cannot be occluding
    if (midTy > midBy)
        return false;

    // The seg is occluding if there is no upper wall, since lower walls are always opaque
    if (midBy >= fty)
        return true;

    // If the upper wall is a normal wall then it occludes
    if (backSector.ceilingpic >= 0)
        return true;

    // Otherwise if the upper wall is sky or void, only make it occlude if there is no lower wall
    return (midBy <= fby);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check if the bounding box for the specified node is visible
//------------------------------------------------------------------------------------------------------------------------------------------
static bool RV_NodeBBVisible(const fixed_t boxCoords[4]) noexcept {
    // Get the bounds of the box in normalized device coords to see what area of the screen it occupies
    const float nodeTy = RV_FixedToFloat(boxCoords[BOXTOP]);
    const float nodeBy = RV_FixedToFloat(boxCoords[BOXBOTTOM]);
    const float nodeLx = RV_FixedToFloat(boxCoords[BOXLEFT]);
    const float nodeRx = RV_FixedToFloat(boxCoords[BOXRIGHT]);

    float nodeMinX = +1.0f;
    float nodeMaxX = -1.0f;

    const auto addNodeLineToBounds = [&](const float x1, const float y1, const float x2, const float y2) noexcept {
        float lx = {};
        float rx = {};

        if (RV_GetLineNdcBounds(x1, y1, x2, y2, lx, rx)) {
            nodeMinX = std::min(lx, nodeMinX);
            nodeMaxX = std::max(rx, nodeMaxX);
        }
    };

    addNodeLineToBounds(nodeLx, nodeTy, nodeRx, nodeTy);
    addNodeLineToBounds(nodeRx, nodeTy, nodeRx, nodeBy);
    addNodeLineToBounds(nodeRx, nodeBy, nodeLx, nodeBy);
    addNodeLineToBounds(nodeLx, nodeBy, nodeLx, nodeTy);

    // Regard the node as visible if any of that area is unobscured
    return RV_IsRangeVisible(nodeMinX, nodeMaxX);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Visit a subsector during BSP traversal.
// Adds it to the list of subsectors to be drawn, and marks the areas that its fully solid walls occlude.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_VisitSubsec(const int32_t subsecIdx) noexcept {
    // Run through all of the segs for the subsector and mark out areas of the screen that they fully occlude.
    // Also determine whether each seg is visible and backfacing while we are at it.
    subsector_t& subsec = gpSubsectors[subsecIdx];
    sector_t& frontSector = *subsec.sector;

    rvseg_t* const pSegs = gpRvSegs.get() + subsec.firstseg;
    const uint32_t numSegs = subsec.numsegs;

    for (uint32_t segIdx = 0; segIdx < numSegs; ++segIdx) {
        // Firstly, clear the line segment flags
        rvseg_t& seg = pSegs[segIdx];
        seg.flags = 0;

        // Determine whether the segment is backfacing so we can re-use the result later
        const float p1f[2] = { seg.v1x, seg.v1y };
        const float p2f[2] = { seg.v2x, seg.v2y };

        {
            const float viewDx = gViewXf - p1f[0];
            const float viewDy = gViewYf - p1f[1];
            const float edgeDx = p2f[0] - p1f[0];
            const float edgeDy = p2f[1] - p1f[1];

            if (edgeDx * viewDy > edgeDy * viewDx) {
                seg.flags |= SGF_BACKFACING;
            }
        }

        // Determine if the seg is visible, ignoring whether it is backfacing or not.
        // First get the area of the screen that the seg covers in normalized device coords, if it's onscreen.
        // Then check if that range is actually visible if it is onscreen.
        float segLx = {};
        float segRx = {};

        if (RV_GetLineNdcBounds(p1f[0], p1f[1], p2f[0], p2f[1], segLx, segRx)) {
            if (RV_IsRangeVisible(segLx, segRx)) {
                seg.flags |= SGF_VISIBLE_COLS;
            }
        }

        // Make the seg occlude if it's the type of seg that occludes, it's not backfacing (so we can see via noclip) and if it's visible
        const bool bMakeSegOcclude = (
            ((seg.flags & SGF_BACKFACING) == 0) &&
            (seg.flags & SGF_VISIBLE_COLS) &&
            RV_IsOccludingSeg(seg, frontSector)
        );

        if (bMakeSegOcclude) {
            RV_OccludeRange(segLx, segRx);
        }
    }

    // Add the subsector to the draw list and set it's draw index.
    // If the sector has a sky then also mark the sky as visible.
    subsec.vkDrawSubsecIdx = (int32_t) gRvDrawSubsecs.size();
    gRvDrawSubsecs.push_back(&subsec);

    if (frontSector.ceilingpic < 0) {
        gbIsSkyVisible = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does recursive traversal of the BSP tree to prepare a list of subsectors to draw, starting at the given node
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_VisitBspNode(const int32_t nodeIdx) noexcept {
    // Is this node number a subsector?
    // If so then process it for potential later rendering:
    if (nodeIdx & NF_SUBSECTOR) {
        // Note: this strange check is in the PC engine too...
        // Under what circumstances can the node number be '-1'?
        if (nodeIdx == -1) {
            RV_VisitSubsec(0);
        } else {
            RV_VisitSubsec(nodeIdx & (~NF_SUBSECTOR));
        }
    } else {
        // This is not a subsector, continue traversing the BSP tree.
        // Only stop when a particular node is determined to be not visible.
        node_t& node = gpBspNodes[nodeIdx];

        // Compute which side of the line the point is on using the cross product.
        // This is pretty much the same code found in 'R_PointOnSide':
        const float dx = gViewXf - RV_FixedToFloat(node.line.x);
        const float dy = gViewYf - RV_FixedToFloat(node.line.y);
        const float lprod = RV_FixedToFloat(node.line.dx) * dy;
        const float rprod = RV_FixedToFloat(node.line.dy) * dx;

        // Depending on which side of the halfspace we are on, reverse the traversal order:
        if (lprod < rprod) {
            if (RV_NodeBBVisible(node.bbox[0])) {
                RV_VisitBspNode(node.children[0]);
            }

            if (RV_NodeBBVisible(node.bbox[1])) {
                RV_VisitBspNode(node.children[1]);
            }
        } else {
            if (RV_NodeBBVisible(node.bbox[1])) {
                RV_VisitBspNode(node.children[1]);
            }

            if (RV_NodeBBVisible(node.bbox[0])) {
                RV_VisitBspNode(node.children[0]);
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Start traversing the BSP tree from the root using the current viewpoint and find what subsectors are to be drawn
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_BuildDrawSubsecList() noexcept {
    // Prepare the draw subsectors list and prealloc enough memory
    gRvDrawSubsecs.clear();
    gRvDrawSubsecs.reserve(gNumSubsectors);

    // Initially assume the sky is not visible
    gbIsSkyVisible = false;

    // Traverse the BSP tree, starting at the root
    const int32_t bspRootNodeIdx = gNumBspNodes - 1;
    RV_VisitBspNode(bspRootNodeIdx);
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
