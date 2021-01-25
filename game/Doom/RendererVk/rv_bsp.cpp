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
#include "rv_main.h"
#include "rv_occlusion.h"
#include "rv_utils.h"

// This is the list of subsectors to be drawn by the Vulkan renderer, in front to back order
std::vector<subsector_t*> gRVDrawSubsecs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given line segment is occluding for the purposes of visibility testing.
// Occluding segs should mark out areas of the screen that they cover, so that nothing behind draws.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool RV_IsOccludingSeg(const seg_t& seg, const sector_t& frontSector) noexcept {
    // One sided lines are always occluding
    if (!seg.backsector)
        return true;

    // If there is a zero sized gap between the sector in front of the line and behind it then count as occluding.
    // This will happen most typically for closed doors:
    sector_t& backSector = *seg.backsector;

    return (
        (frontSector.floorheight >= backSector.ceilingheight) ||
        (frontSector.ceilingheight <= backSector.floorheight)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check if the specified subsector is visible according to occlusion and view frustrum culling
//------------------------------------------------------------------------------------------------------------------------------------------
static bool RV_IsSubsecVisible(const int32_t subsecIdx) noexcept {
    // This is the 'x' coordinate min/max range of the subsector, in normalized device coordinates.
    float subsecXMin = +1.0f;
    float subsecXMax = -1.0f;

    // Run through all leaf edges for the subsector and see which ones are on-screen at least somewhat.
    // Add these onscreen edges to the x coordinate range for the subsector.
    subsector_t& subsec = gpSubsectors[subsecIdx];
    const leafedge_t* const pLeafEdges = gpLeafEdges + subsec.firstLeafEdge;
    const uint32_t numLeafEdges = subsec.numLeafEdges;
    
    for (uint32_t edgeIdx = 0; edgeIdx < numLeafEdges; ++edgeIdx) {
        // Get the edge points in float format
        const leafedge_t& edge1 = pLeafEdges[edgeIdx];
        const leafedge_t& edge2 = pLeafEdges[(edgeIdx + 1) % numLeafEdges];
        const vertex_t& p1 = *edge1.vertex;
        const vertex_t& p2 = *edge2.vertex;
        const float p1f[2] = { RV_FixedToFloat(p1.x), RV_FixedToFloat(p1.y) };
        const float p2f[2] = { RV_FixedToFloat(p2.x), RV_FixedToFloat(p2.y) };

        // Get the normalized device x bounds of the segment and skip if offscreen
        float edgeLx = {};
        float edgeRx = {};

        if (!RV_GetLineNdcBounds(p1f[0], p1f[1], p2f[0], p2f[1], edgeLx, edgeRx))
            continue;

        // Edge is onscreen, add to the subsector NDC bounds
        subsecXMin = std::min(subsecXMin, edgeLx);
        subsecXMax = std::max(subsecXMax, edgeRx);
    }

    // Now that we have the bounds for the subsector, determine if it is visible
    return RV_IsRangeVisible(subsecXMin, subsecXMax);
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
    // Skip drawing the subsector if it is not visible
    if (!RV_IsSubsecVisible(subsecIdx))
        return;

    // Run through all of the segs for the subsector and mark out areas of the screen that they fully occlude
    subsector_t& subsec = gpSubsectors[subsecIdx];
    sector_t& frontSector = *subsec.sector;

    const seg_t* const pSegs = gpSegs + subsec.firstseg;
    const uint32_t numSegs = subsec.numsegs;
    
    for (uint32_t segIdx = 0; segIdx < numSegs; ++segIdx) {
        // Skip the seg if it's not something that fully occludes stuff behind it
        const seg_t& seg = pSegs[segIdx];

        if (!RV_IsOccludingSeg(seg, frontSector))
            continue;

        // Ignore the seg if it's backfacing, don't have it occlude in that situation.
        // This enables the back-face of walls to be seen through when no-clipping.
        const vertex_t& p1 = *seg.vertex1;
        const vertex_t& p2 = *seg.vertex2;
        const float p1f[2] = { RV_FixedToFloat(p1.x), RV_FixedToFloat(p1.y) };
        const float p2f[2] = { RV_FixedToFloat(p2.x), RV_FixedToFloat(p2.y) };
        const float viewDx = gViewXf - p1f[0];
        const float viewDy = gViewYf - p1f[1];
        const float edgeDx = p2f[0] - p1f[0];
        const float edgeDy = p2f[1] - p1f[1];

        if (edgeDx * viewDy > edgeDy * viewDx)
            continue;

        // Get the area of the screen that the seg occludes, in normalized device coords.
        // If the seg is offscreen then skip it.
        float segLx = {};
        float segRx = {};

        if (!RV_GetLineNdcBounds(p1f[0], p1f[1], p2f[0], p2f[1], segLx, segRx))
            continue;

        // Mark this part of the screen as occluded
        RV_OccludeRange(segLx, segRx);
    }

    // Add the subsector to the draw list.
    // If the sector has a sky then also mark the sky as visible.
    gRVDrawSubsecs.push_back(&subsec);

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
    gRVDrawSubsecs.clear();
    gRVDrawSubsecs.reserve(gNumSubsectors);

    // Initially assume the sky is not visible
    gbIsSkyVisible = false;

    // Traverse the BSP tree, starting at the root
    const int32_t bspRootNodeIdx = gNumBspNodes - 1;
    RV_VisitBspNode(bspRootNodeIdx);
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
