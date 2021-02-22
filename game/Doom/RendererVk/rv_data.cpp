//------------------------------------------------------------------------------------------------------------------------------------------
// This module is responsible for initializing data-structures used by the new Vulkan renderer.
// These are more optimal versions of the regular render data-structures for the new renderer.
// The module is initialized on starting a level and frees resources on ending a level.
// 
// Note: if the Vulkan renderer is not supported then initializing VK level data is a no-op.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "rv_data.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Renderer/r_local.h"
#include "PcPsx/Video.h"
#include "rv_utils.h"

#include <algorithm>
#include <cmath>

std::unique_ptr<rvseg_t[]>          gpRvSegs;           // The Vulkan renderer version of segs: same count as in 'p_setup.cpp'
std::unique_ptr<rvleafedge_t[]>     gpRvLeafEdges;      // The Vulkan renderer version of leaf edges: same count as in 'p_setup.cpp'

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a seg or leaf edge vertex which is a candidate for welding
//------------------------------------------------------------------------------------------------------------------------------------------
struct WeldVertex {
    float       x, y;               // The 2d position of the vertex
    int32_t     weldPrevIdx;        // Index of the previous vertex in this vertex weld list (-1 if none)
    int32_t     weldNextIdx;        // Index of the next vertex in this vertex weld list (-1 if none)
    bool        bIsSegVertex;       // Is the vertex for a seg or leaf edge?
    bool        bIsSegV1;           // If the vertex is for a seg, is it for vertex 1 or 2?
    bool        bDidSave;           // Did we save the vertices in this weld group already?
    uint32_t    fromListIdx;        // The index of the seg or leaf edge that the vertex came from
};

static std::unique_ptr<WeldVertex>  gpWeldVerts;        // A list of vertices to try and weld together
static int32_t                      gNumWeldVerts;      // How many weld vertices there are

// How close vertices must be together on the x/y axes before they are added into the current weld group or a new weld group (if starting a new one)
static constexpr float WELD_DIST = 0.5f;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of segs for the Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_InitSegs() noexcept {
    const int32_t numSegs = gNumSegs;
    gpRvSegs.reset(new rvseg_t[numSegs]);

    for (int32_t i = 0; i < numSegs; ++i) {
        const seg_t& srcSeg = gpSegs[i];
        const vertex_t& srcV1 = *srcSeg.vertex1;
        const vertex_t& srcV2 = *srcSeg.vertex2;
        
        rvseg_t& dstSeg = gpRvSegs[i];
        dstSeg.v1x = RV_FixedToFloat(srcV1.x);
        dstSeg.v1y = RV_FixedToFloat(srcV1.y);
        dstSeg.v2x = RV_FixedToFloat(srcV2.x);
        dstSeg.v2y = RV_FixedToFloat(srcV2.y);
        
        const double edgeDx = (double) dstSeg.v2x - dstSeg.v1x;
        const double edgeDy = (double) dstSeg.v2y - dstSeg.v1y;
        dstSeg.length = (float) std::sqrt(edgeDx * edgeDx + edgeDy * edgeDy);

        dstSeg.uOffset = RV_FixedToFloat(srcSeg.offset);
        dstSeg.angle = srcSeg.angle;
        dstSeg.flags = srcSeg.flags;
        dstSeg.sidedef = srcSeg.sidedef;
        dstSeg.linedef = srcSeg.linedef;
        dstSeg.frontsector = srcSeg.frontsector;
        dstSeg.backsector = srcSeg.backsector;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the list of leaf edges for the Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_InitLeafEdges() noexcept {
    const int32_t numEdges = gTotalNumLeafEdges;
    gpRvLeafEdges.reset(new rvleafedge_t[numEdges]);

    for (int32_t i = 0; i < numEdges; ++i) {
        const leafedge_t& srcEdge = gpLeafEdges[i];
        const vertex_t& srcVertex = *srcEdge.vertex;

        rvleafedge_t& dstEdge = gpRvLeafEdges[i];
        dstEdge.v1x = RV_FixedToFloat(srcVertex.x);
        dstEdge.v1y = RV_FixedToFloat(srcVertex.y);

        if (srcEdge.seg) {
            const int32_t srcSegIdx = (int32_t)(srcEdge.seg - gpSegs);
            ASSERT(srcSegIdx < gNumSegs);
            dstEdge.seg = gpRvSegs.get() + srcSegIdx;
        } else {
            dstEdge.seg = nullptr;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Saves the given updated coordinate for the specified weld vertex
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_SaveWeldedVertex(const WeldVertex& origVertex, const float newX, const float newY) noexcept {
    if (origVertex.bIsSegVertex) {
        rvseg_t& seg = gpRvSegs[origVertex.fromListIdx];

        if (origVertex.bIsSegV1) {
            seg.v1x = newX;
            seg.v1y = newY;
        } else {
            seg.v2x = newX;
            seg.v2y = newY;
        }
    } else {
        rvleafedge_t& leafEdge = gpRvLeafEdges[origVertex.fromListIdx];
        leafEdge.v1x = newX;
        leafEdge.v1y = newY;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Build the initial list of of candidate vertices to be welded and sort them along the x-axis
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_InitWeldVertsList() noexcept {
    // Set the total amount and alloc the list
    const int32_t numSegs = gNumSegs;
    const int32_t numLeafEdges = gTotalNumLeafEdges;
    const int32_t numWeldVerts = (numSegs * 2) + numLeafEdges;

    gNumWeldVerts = numWeldVerts;
    gpWeldVerts.reset(new WeldVertex[numWeldVerts]);

    // Populate all of the vertices
    WeldVertex* pWeldVertex = gpWeldVerts.get();
    const rvseg_t* const pSegs = gpRvSegs.get();

    for (int32_t i = 0; i < numSegs; ++i) {
        WeldVertex& dstVert1 = *pWeldVertex++;
        WeldVertex& dstVert2 = *pWeldVertex++;
        const rvseg_t& srcSeg = pSegs[i];

        dstVert1.x = srcSeg.v1x;
        dstVert1.y = srcSeg.v1y;
        dstVert1.weldPrevIdx = -1;
        dstVert1.weldNextIdx = -1;
        dstVert1.bIsSegVertex = true;
        dstVert1.bIsSegV1 = true;
        dstVert1.bDidSave = false;
        dstVert1.fromListIdx = i;

        dstVert2.x = srcSeg.v2x;
        dstVert2.y = srcSeg.v2y;
        dstVert2.weldPrevIdx = -1;
        dstVert2.weldNextIdx = -1;
        dstVert2.bIsSegVertex = true;
        dstVert2.bIsSegV1 = false;
        dstVert2.bDidSave = false;
        dstVert2.fromListIdx = i;
    }

    const rvleafedge_t* const pLeafEdges = gpRvLeafEdges.get();

    for (int32_t i = 0; i < numLeafEdges; ++i) {
        WeldVertex& dstVert = *pWeldVertex++;
        const rvleafedge_t& srcEdge = pLeafEdges[i];

        dstVert.x = srcEdge.v1x;
        dstVert.y = srcEdge.v1y;
        dstVert.weldPrevIdx = -1;
        dstVert.weldNextIdx = -1;
        dstVert.bIsSegVertex = false;
        dstVert.bIsSegV1 = false;
        dstVert.bDidSave = false;
        dstVert.fromListIdx = i;
    }

    // Next sort the weld vertices along the x-axis
    std::sort(
        gpWeldVerts.get(),
        gpWeldVerts.get() + numWeldVerts,
        [](const WeldVertex& v1, const WeldVertex& v2) noexcept {
            return (v1.x < v2.x);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determine how many vertices are in a weld cluster, starting at the specified vertex index. A weld cluster is a group of vertices that
// are close on the x-axis, where the maximum distance between each is the maximum allowed welding distance.
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t RV_DetermineWeldClusterSize(const int32_t startVertIdx) noexcept {
    ASSERT(startVertIdx < gNumWeldVerts);

    const WeldVertex* const pWeldVerts = gpWeldVerts.get();
    const int32_t numWeldVerts = gNumWeldVerts;
    int32_t endVertexIdx = startVertIdx + 1;

    while (endVertexIdx < numWeldVerts) {
        const WeldVertex& v1 = pWeldVerts[endVertexIdx - 1];
        const WeldVertex& v2 = pWeldVerts[endVertexIdx];

        // Is the x-axis distance between this pair too great?
        // If so then the cluster stops here:
        if (v2.x - v1.x > WELD_DIST)
            break;

        endVertexIdx++;
    }

    return endVertexIdx - startVertIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sort the specified cluster of weld vertices on the y-axis.
// This enables us to do ordered comparisons on the y-axis, after the x-axis broad phase grouping is done.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_SortWeldCluster(const int32_t startVertIdx, const int32_t endVertIdx) noexcept {
    ASSERT(startVertIdx <= endVertIdx);
    ASSERT(endVertIdx <= gNumWeldVerts);

    WeldVertex* const pWeldVerts = gpWeldVerts.get();
    std::sort(
        pWeldVerts + startVertIdx,
        pWeldVerts + endVertIdx,
        [](const WeldVertex& v1, const WeldVertex& v2) noexcept {
            return (v1.y < v2.y);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the start vertex index in the specified group/linked-list of vertices to be welded together
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t RV_GetWeldGroupStart(const int32_t inputVertIdx) noexcept {
    const WeldVertex* const pWeldVerts = gpWeldVerts.get();
    int32_t curVertIdx = inputVertIdx;

    while (true) {
        ASSERT(curVertIdx < gNumWeldVerts);
        const WeldVertex& vert = pWeldVerts[curVertIdx];
        const int32_t prevWeldVertIdx = vert.weldPrevIdx;

        if (prevWeldVertIdx < 0)
            break;

        curVertIdx = prevWeldVertIdx;
    }

    return curVertIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the last vertex index in the specified group/linked-list of vertices to be welded together
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t RV_GetWeldGroupEnd(const int32_t inputVertIdx) noexcept {
    const WeldVertex* const pWeldVerts = gpWeldVerts.get();
    int32_t curVertIdx = inputVertIdx;

    while (true) {
        ASSERT(curVertIdx < gNumWeldVerts);
        const WeldVertex& vert = pWeldVerts[curVertIdx];
        const int32_t nextWeldVertIdx = vert.weldNextIdx;

        if (nextWeldVertIdx < 0)
            break;

        curVertIdx = nextWeldVertIdx;
    }

    return curVertIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Marks the specified vertices as needing to be welded together.
// Specifically it will join the weld lists for the two vertices, joining the list for vertex 2 onto the end of the list for vertex 1.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_JoinVertexWeldLists(const int32_t vertIdx1, const int32_t vertIdx2) noexcept {
    // Get the start of both weld lists, if they are the same list then don't join - already joined
    const int32_t list1BegIdx = RV_GetWeldGroupStart(vertIdx1);
    const int32_t list2BegIdx = RV_GetWeldGroupStart(vertIdx2);
    ASSERT(list1BegIdx < gNumWeldVerts);

    if (list1BegIdx == list2BegIdx)
        return;

    ASSERT(list2BegIdx < gNumWeldVerts);

    // Get the end of the 1st weld list, this is where we will join the 2nd list
    const int32_t list1EndIdx = RV_GetWeldGroupEnd(vertIdx1);
    ASSERT(list1EndIdx < gNumWeldVerts);

    // Join the vertex weld lists!
    WeldVertex* const pWeldVerts = gpWeldVerts.get();
    WeldVertex& endVert = pWeldVerts[list1EndIdx];
    WeldVertex& begVert = pWeldVerts[list2BegIdx];
    endVert.weldNextIdx = list2BegIdx;
    begVert.weldPrevIdx = list1EndIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determine the averaged position for a weld group, starting at the specified vertex index.
// Also returns the beginning vertex index in the group, so it can be re-used.
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t RV_WeldGroupCoords(const int32_t weldVertIdx, float& xOut, float& yOut) noexcept {
    // Get the group start vertex
    const int32_t begVertIdx = RV_GetWeldGroupStart(weldVertIdx);

    // Run through all vertices and sum their coordinates
    const WeldVertex* const pWeldVerts = gpWeldVerts.get();

    double totalX = 0;
    double totalY = 0;
    uint32_t groupSize = 0;

    for (int32_t curVertIdx = begVertIdx; curVertIdx >= 0;) {
        ASSERT(curVertIdx < gNumWeldVerts);
        const WeldVertex& vert = pWeldVerts[curVertIdx];
        totalX += vert.x;
        totalY += vert.y;
        groupSize++;
        curVertIdx = vert.weldNextIdx;
    }

    // Average the result and give back the start of the list
    xOut = (float)(totalX / (double) groupSize);
    yOut = (float)(totalY / (double) groupSize);
    return begVertIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Saves the specified welded coordinates for the a weld group starting at the given vertex index
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_SaveWeldedGroupCoords(const int32_t groupStartVertIdx, const float x, const float y) noexcept {
    WeldVertex* const pWeldVerts = gpWeldVerts.get();

    for (int32_t curVertIdx = groupStartVertIdx; curVertIdx >= 0;) {
        ASSERT(curVertIdx < gNumWeldVerts);
        WeldVertex& vert = pWeldVerts[curVertIdx];
        RV_SaveWeldedVertex(vert, x, y);
        vert.bDidSave = true;
        curVertIdx = vert.weldNextIdx;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to weld together the specified cluster of vertices that are close together on the x-axis.
// Saves the welded vertices back out when done.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_WeldCluster(const int32_t clustBegIdx, const int32_t clustEndIdx) noexcept {
    // Sort the vertices on the y-axis first
    RV_SortWeldCluster(clustBegIdx, clustEndIdx);

    // Run through all the vertices in the cluster and try to weld
    WeldVertex* const pWeldVerts = gpWeldVerts.get();

    for (int32_t i = clustBegIdx; i < clustEndIdx; ++i) {
        WeldVertex& v1 = pWeldVerts[i];

        for (int32_t j = i + 1; j < clustEndIdx; ++j) {
            // Are these vertices too far away on the y-axis?
            // If so then we can stop the inner loop here, further vertices will be only further away:
            WeldVertex& v2 = pWeldVerts[j];

            if (v2.y - v1.y > WELD_DIST)
                break;

            // Are the vertices close enough on the x-axis?
            if (std::abs(v2.x - v1.x) > WELD_DIST)
                continue;

            // These vertices can be welded, weld them together!
            RV_JoinVertexWeldLists(i, j);
        }
    }

    // Next get the averaged coordinate for all weld groups and save each one
    for (int32_t i = clustBegIdx; i < clustEndIdx; ++i) {
        // Did we already save out this group?
        const WeldVertex& vert = pWeldVerts[i];

        if (vert.bDidSave)
            continue;

        // Didn't save the group, get the averaged coord and save
        float weldedX = {};
        float weldedY = {};
        const int32_t weldGroupBegIdx = RV_WeldGroupCoords(i, weldedX, weldedY);
        RV_SaveWeldedGroupCoords(weldGroupBegIdx, weldedX, weldedY);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to weld vertices that are close together the level to fix minor precision issues and seams
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_WeldVertices() noexcept {
    // Initialize the weld vertex list
    RV_InitWeldVertsList();

    // Group the vertices into clusters of vertices that are close on the x-axis and weld those clusters
    const int32_t numWeldVerts = gNumWeldVerts;
    int32_t curVertIdx = 0;

    while (curVertIdx < numWeldVerts) {
        const int32_t clusterSize = RV_DetermineWeldClusterSize(curVertIdx);
        const int32_t clusterEnd = curVertIdx + clusterSize;
        RV_WeldCluster(curVertIdx, clusterEnd);
        curVertIdx = clusterEnd;
    }

    // Cleanup after welding
    gpWeldVerts.reset();
    gNumWeldVerts = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize data-structures used by the Vulkan renderer on level startup
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_InitLevelData() noexcept {
    ASSERT(gpSegs);
    ASSERT(gpLeafEdges);

    // If Vulkan is not supported then this is a no-op
    if (Video::gBackendType != Video::BackendType::Vulkan)
        return;

    // Initialize basic data structures
    RV_InitSegs();
    RV_InitLeafEdges();

    // Weld nearby vertices together to try and fix minor seams in some levels due to precision issues
    RV_WeldVertices();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Free data-structures used by the Vulkan renderer on level shutdown
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_FreeLevelData() noexcept {
    // If Vulkan is not supported then this is a no-op
    if (Video::gBackendType != Video::BackendType::Vulkan)
        return;

    gpRvLeafEdges.reset();
    gpRvSegs.reset();
}

#endif
