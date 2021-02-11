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

#include <cmath>

std::unique_ptr<rvseg_t[]>          gpRvSegs;           // The Vulkan renderer version of segs: same count as in 'p_setup.cpp'
std::unique_ptr<rvleafedge_t[]>     gpRvLeafEdges;      // The Vulkan renderer version of leaf edges: same count as in 'p_setup.cpp'

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
