#include "r_draw.h"

#include "Asserts.h"
#include "Doom/Base/i_main.h"
#include "Doom/Game/p_setup.h"
#include "PsyDoom/Config.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGTE.h"
#include "r_local.h"
#include "r_main.h"
#include "r_plane.h"
#include "r_segs.h"
#include "r_sky.h"
#include "r_things.h"

#include <vector>

// The maximum number of new vertices that can be added to leafs by clipping operations.
// If we happen to emit more than this then engine will fail with an error.
// PsyDoom: this static limit no longer applies in the limit removing engine.
#if !PSYDOOM_LIMIT_REMOVING
    static constexpr int32_t MAX_NEW_CLIP_VERTS = 32;
#endif

// How many new vertices were generated by clipping operations and a list of those new vertices.
// PsyDoom: this list can now be expanded as required at runtime, for the limit removing engine.
static int32_t gNumNewClipVerts;

#if PSYDOOM_LIMIT_REMOVING
    static std::vector<vertex_t> gNewClipVerts;
#else
    static vertex_t gNewClipVerts[MAX_NEW_CLIP_VERTS];
#endif

#if PSYDOOM_MODS
    // PsyDoom: two leafs that can be alternated between for clipping operations.
    // These were previously in the 1 KiB scratchpad, but I'm moving them out so we can extend the max number of allowed edges.
    static leaf_t gLeafs[2];

    // PsyDoom: whether each leaf edge point is on the inside of a test plane.
    // Note: for limit removing builds I use 'uint8_t' instead of 'bool' because 'std::vector<bool>' is often bit packed and does not behave like a normal vector.
    #if PSYDOOM_LIMIT_REMOVING
        static std::vector<uint8_t> gbPointsOnOutside;
    #else
        static bool gbPointsOnOutside[MAX_LEAF_EDGES + 1];
    #endif
#endif

#if PSYDOOM_LIMIT_REMOVING
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom limit removing addition: preallocs memory for some resizable draw buffers
//------------------------------------------------------------------------------------------------------------------------------------------
void R_InitDrawBuffers() noexcept {
    // These defaults should be plenty for all but the most insanely complex scenes
    gpDrawSubsectors.reserve(1024 * 8);
    gNewClipVerts.resize(2048);
    gbPointsOnOutside.reserve(2048);

    for (leaf_t& leaf : gLeafs) {
        leaf.edges.reserve(2048);
    }
}
#endif  // #if PSYDOOM_LIMIT_REMOVING

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws everything in the subsector: floors, ceilings, walls and things
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawSubsector(subsector_t& subsec) noexcept {
    // The PSX scratchpad is used to store 2 leafs, grab that memory here.
    // The code below ping-pongs between both leafs, using them as either input or output leafs for each clipping operation
    // I don't know why this particular address is used though...
    //
    // PsyDoom: use global memory instead, can hold a lot more!
    #if PSYDOOM_MODS
        leaf_t* const pLeafs = gLeafs;
    #else
        leaf_t* const pLeafs = (leaf_t*) LIBETC_getScratchAddr(42);
    #endif

    leaf_t& leaf1 = pLeafs[0];

    #if PSYDOOM_LIMIT_REMOVING
        leaf1.edges.clear();
    #endif

    // Cache the entire leaf for the subsector to the scratchpad.
    // Also transform any leaf vertices that were not yet transformed up until this point.
    // PsyDoom: we no longer use the scratchpad anymore, but the logic remains mostly the same.
    {
        const leafedge_t* pSrcEdge = gpLeafEdges + subsec.firstLeafEdge;

        #if !PSYDOOM_LIMIT_REMOVING
            leafedge_t* pDstEdge = leaf1.edges;
        #endif

        for (int32_t edgeIdx = 0; edgeIdx < subsec.numLeafEdges; ++edgeIdx, ++pSrcEdge) {
            // Cache the leaf edge
            vertex_t& vert = *pSrcEdge->vertex;

            #if PSYDOOM_LIMIT_REMOVING
                leafedge_t* const pDstEdge = &leaf1.edges.emplace_back();
            #endif

            pDstEdge->vertex = &vert;
            pDstEdge->seg = pSrcEdge->seg;

            // Transform this leaf edge's vertexes if they need to be transformed
            if (vert.frameUpdated != gNumFramesDrawn) {
                const SVECTOR viewToPt = {
                    (int16_t) d_fixed_to_int(vert.x - gViewX),
                    0,
                    (int16_t) d_fixed_to_int(vert.y - gViewY)
                };

                VECTOR viewVec;
                int32_t rotFlags;
                LIBGTE_RotTrans(viewToPt, viewVec, rotFlags);

                vert.viewx = viewVec.vx;
                vert.viewy = viewVec.vz;

                if (viewVec.vz > 3) {
                    vert.scale = (HALF_SCREEN_W * FRACUNIT) / viewVec.vz;
                    vert.screenx = d_fixed_to_int(vert.scale * vert.viewx) + HALF_SCREEN_W;
                }

                vert.frameUpdated = gNumFramesDrawn;
            }

            #if !PSYDOOM_LIMIT_REMOVING
                ++pDstEdge;
            #endif
        }

        #if !PSYDOOM_LIMIT_REMOVING
            leaf1.numEdges = subsec.numLeafEdges;
        #endif
    }

    // Begin the process of clipping the leaf: ping pong between the two leaf buffers for input and output.
    // 
    // PsyDoom: the new vertices buffer can now accomodate any number of vertices - ensure it is big enough here.
    // In the absolute worst case scenario every edge in leaf would produce an extra vertex every time it is clipped (2x the vertices) and we clip the leaf 3 times.
    // Therefore having 8x (2^3) as many vertices should cover any possible scenario.
    uint32_t curLeafIdx = 0;

    #if PSYDOOM_LIMIT_REMOVING
        const size_t maxNewClipVerts = subsec.numLeafEdges * 8;
        
        if (maxNewClipVerts > gNewClipVerts.size()) {
            gNewClipVerts.resize(maxNewClipVerts);
        }
    #endif

    gNumNewClipVerts = 0;

    // Clip the leaf against the front plane if required
    {
        #if PSYDOOM_LIMIT_REMOVING
            leafedge_t* pEdge = leaf1.edges.data();
        #else
            leafedge_t* pEdge = leaf1.edges;
        #endif

        for (int32_t edgeIdx = 0; edgeIdx < subsec.numLeafEdges; ++edgeIdx, ++pEdge) {
            if (pEdge->vertex->viewy <= NEAR_CLIP_DIST + 1) {
                R_FrontZClip(pLeafs[curLeafIdx], pLeafs[curLeafIdx ^ 1]);
                curLeafIdx ^= 1;
                break;
            }
        }
    }

    // Check to see what side of the left view frustrum plane the leaf's points are on.
    // Clip the leaf if required, or discard if all the points are offscreen.
    const int32_t leftPlaneSide = R_CheckLeafSide(false, pLeafs[curLeafIdx]);

    if (leftPlaneSide < 0)
        return;

    if (leftPlaneSide > 0) {
        const int32_t numOutputEdges = R_LeftEdgeClip(pLeafs[curLeafIdx], pLeafs[curLeafIdx ^ 1]);
        curLeafIdx ^= 1;

        if (numOutputEdges < 3)     // If there is not a triangle left then discard the subsector
            return;
    }

    // Check to see what side of the right view frustrum plane the leaf's points are on.
    // Clip the leaf if required, or discard if all the points are offscreen.
    const int32_t rightPlaneSide = R_CheckLeafSide(true, pLeafs[curLeafIdx]);

    if (rightPlaneSide < 0)
        return;

    // Clip the leaf against the right view frustrum plane if required
    if (rightPlaneSide > 0) {
        const int32_t numOutputEdges = R_RightEdgeClip(pLeafs[curLeafIdx], pLeafs[curLeafIdx ^ 1]);
        curLeafIdx ^= 1;

        if (numOutputEdges < 3)     // If there is not a triangle left then discard the subsector
            return;
    }

    // Terminate the list of leaf edges by putting the first edge past the end of the list.
    // This allows the renderer to implicitly wraparound to the beginning of the list when accessing 1 past the end.
    // This is useful for when working with edges as it saves checks!
    leaf_t& drawleaf = pLeafs[curLeafIdx];

    #if PSYDOOM_LIMIT_REMOVING
    {
        const leafedge_t firstEdge = drawleaf.edges[0];     // Deliberately copying, so 'push_back' isn't getting a reference to an invalidated element
        drawleaf.edges.push_back(firstEdge);
    }
    #else
        drawleaf.edges[drawleaf.numEdges] = drawleaf.edges[0];
    #endif

    // PsyDoom limit removing Classic renderer extension.
    // Draw sky walls for leaf edges with associated segs where appropriate if the 'sky leak fix' is enabled.
    #if PSYDOOM_LIMIT_REMOVING
        if (Config::gbSkyLeakFix) {
            leafedge_t* pEdge = drawleaf.edges.data();
            const int32_t numEdges = (int32_t) drawleaf.edges.size() - 1;   // N.B: last edge is a dummy one for fast wraparound!

            for (int32_t edgeIdx = 0; edgeIdx < numEdges; ++edgeIdx, ++pEdge) {
                seg_t* const pSeg = pEdge->seg;

                if (pSeg) {
                    R_DrawSegSkyWalls(subsec, *pEdge);
                }
            }
        }
    #endif

    // Draw the walls for all visible edges in the leaf
    {
        #if PSYDOOM_LIMIT_REMOVING
            leafedge_t* pEdge = drawleaf.edges.data();
            const int32_t numEdges = (int32_t) drawleaf.edges.size() - 1;   // N.B: last edge is a dummy one for fast wraparound!
        #else
            leafedge_t* pEdge = drawleaf.edges;
            const int32_t numEdges = drawleaf.numEdges;
        #endif

        for (int32_t edgeIdx = 0; edgeIdx < numEdges; ++edgeIdx, ++pEdge) {
            seg_t* const pSeg = pEdge->seg;

            // Only draw walls for this leaf edge if its seg has visible columns
            if (pSeg && (pSeg->flags & SGF_VISIBLE_COLS)) {
                R_DrawWalls(*pEdge);
            }
        }
    }

    // Draw the floor if above it.
    // PsyDoom: floors can now have skies and draw floor height might be different to real height.
    sector_t& drawsec = *gpCurDrawSector;

    #if PSYDOOM_MODS
        const bool bHasSkyFloor = (drawsec.floorpic == -1);
        const fixed_t drawSecFloorH = drawsec.floorDrawHeight;
    #else
        constexpr bool bHasSkyFloor = false;
        const fixed_t drawSecFloorH = drawsec.floorheight;
    #endif

    if ((!bHasSkyFloor) && (gViewZ > drawSecFloorH)) {
        R_DrawSubsectorFlat(drawleaf, false);
    }

    // Draw the ceiling if below it and it is not a sky ceiling
    if ((drawsec.ceilingpic != -1) && (gViewZ < drawsec.ceilingheight)) {
        R_DrawSubsectorFlat(drawleaf, true);
    }

    // Draw all sprites in the subsector
    R_DrawSubsectorSprites(subsec);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clip the given input leaf against the front view frustrum plane.
// Returns the result in the given output leaf.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_FrontZClip(const leaf_t& inLeaf, leaf_t& outLeaf) noexcept {
    // Front plane clipping of leafs is much more aggressive than earlier stages of the rendering pipeline - 2x the distance used elsewhere.
    // This might have been done to work around a numerical overflow when computing the vertex 'screenY' value in 'R_DrawFlatSpans',
    // which PsyDoom now fixes. Without PsyDoom's fix the original issue is greatly reduced if the clipping distance here is increased...
    constexpr int32_t CLIP_DIST = NEAR_CLIP_DIST * 2;

    // Run through all the source edges in the given leaf and see if each edge needs to be clipped, skipped or stored as-is
    #if PSYDOOM_LIMIT_REMOVING
        const leafedge_t* pSrcEdge = inLeaf.edges.data();
        const int32_t numSrcEdges = (int32_t) inLeaf.edges.size();

        outLeaf.edges.clear();
    #else
        const leafedge_t* pSrcEdge = inLeaf.edges;
        const int32_t numSrcEdges = inLeaf.numEdges;

        leafedge_t* pDstEdge = outLeaf.edges;
        int32_t numDstEdges = 0;
    #endif

    for (int32_t srcEdgeIdx = 0; srcEdgeIdx < numSrcEdges; ++srcEdgeIdx, ++pSrcEdge) {
        // Grab the next edge after this and wraparound if required
        const leafedge_t* pNextSrcEdge;

        if (srcEdgeIdx < numSrcEdges - 1) {
            pNextSrcEdge = pSrcEdge + 1;
        } else {
            #if PSYDOOM_LIMIT_REMOVING
                pNextSrcEdge = inLeaf.edges.data();
            #else
                pNextSrcEdge = inLeaf.edges;
            #endif
        }

        // Get the 2 points in this edge and their signed distance to the clipping plane
        const vertex_t& srcVert1 = *pSrcEdge->vertex;
        const vertex_t& srcVert2 = *pNextSrcEdge->vertex;

        const int32_t planeDist1 = CLIP_DIST - srcVert1.viewy;
        const int32_t planeDist2 = CLIP_DIST - srcVert2.viewy;

        // See if we need to clip or not.
        // Generate a new edge and vertex if required.
        if (planeDist1 == 0) {
            // Rare case: if the 1st point is exactly on the plane then just emit the edge as-is and don't clip
            #if PSYDOOM_LIMIT_REMOVING
                leafedge_t* const pDstEdge = &outLeaf.edges.emplace_back();
            #endif

            pDstEdge->vertex = pSrcEdge->vertex;
            pDstEdge->seg = pSrcEdge->seg;
        } else {
            // If the 1st point is on the inside of the clipping plane, emit this edge as-is to begin with
            if (planeDist1 < 0) {
                #if PSYDOOM_LIMIT_REMOVING
                    leafedge_t* const pDstEdge = &outLeaf.edges.emplace_back();
                #endif

                pDstEdge->vertex = pSrcEdge->vertex;
                pDstEdge->seg = pSrcEdge->seg;

                #if !PSYDOOM_LIMIT_REMOVING
                    ++numDstEdges;
                    ++pDstEdge;

                    if (numDstEdges > MAX_LEAF_EDGES) {
                        I_Error("FrontZClip: Point Overflow");
                    }
                #endif
            }

            // Rare case: if the 2nd point is exactly on the plane then do not clip, will emit it's edge in the next iteration
            if (planeDist2 == 0)
                continue;

            // If both points are on the same side of the clipping plane then we do not clip
            if ((planeDist1 < 0) == (planeDist2 < 0))
                continue;

            // Clipping required: will make a new vertex because of the clipping operation:
            #if PSYDOOM_LIMIT_REMOVING
                ASSERT((size_t) gNumNewClipVerts < gNewClipVerts.size());    // Sanity check: buffer should always be big enough
            #endif

            vertex_t& newVert = gNewClipVerts[gNumNewClipVerts];
            gNumNewClipVerts++;

            #if !PSYDOOM_LIMIT_REMOVING
                if (gNumNewClipVerts >= MAX_NEW_CLIP_VERTS) {
                    // This check seems like it was slightly incorrect - should have been done BEFORE the vertex count increment perhaps?
                    // With this code it can trigger if you are at the maximum amount, but not exceeding the max...
                    I_Error("FrontZClip: exceeded max new vertexes\n");
                }
            #endif

            // Compute the intersection time of the edge against the plane.
            // Use the same method described in more detail in 'R_CheckBBox':
            fixed_t intersectT;

            {
                const int32_t a = planeDist1;
                const int32_t b = -planeDist2;
                intersectT = d_int_to_fixed(a) / (a + b);
            }

            // Compute & set the view x/y values for the clipped edge vertex
            {
                const int32_t dviewx = srcVert2.viewx - srcVert1.viewx;
                newVert.viewx = d_fixed_to_int(dviewx * intersectT) + srcVert1.viewx;
                newVert.viewy = CLIP_DIST;
            }

            // Compute the world x/y values for the clipped edge vertex
            {
                const int32_t dx = d_fixed_to_int(srcVert2.x - srcVert1.x);
                const int32_t dy = d_fixed_to_int(srcVert2.y - srcVert1.y);
                newVert.x = dx * intersectT + srcVert1.x;
                newVert.y = dy * intersectT + srcVert1.y;
            }

            // Re-do perspective projection to compute screen x and scale for the vertex.
            // PsyDoom: fix an occasional overflow here by using 64-bit arithmetic.
            newVert.scale = (HALF_SCREEN_W * FRACUNIT) / newVert.viewy;

            #if PSYDOOM_MODS
                newVert.screenx = (int32_t) d_rshift<FRACBITS>((int64_t) newVert.viewx * newVert.scale) + HALF_SCREEN_W;
            #else
                newVert.screenx = d_fixed_to_int(newVert.viewx * newVert.scale) + HALF_SCREEN_W;
            #endif

            // Mark the new vertex as having up-to-date transforms and populate the new edge created.
            // Note that the new edge will only have a seg it doesn't run along the clip plane.
            newVert.frameUpdated = gNumFramesDrawn;

            #if PSYDOOM_LIMIT_REMOVING
                leafedge_t* const pDstEdge = &outLeaf.edges.emplace_back();
            #endif

            pDstEdge->vertex = &newVert;

            if ((planeDist1 > 0) && (planeDist2 < 0)) {
                pDstEdge->seg = pSrcEdge->seg;
            } else {
                pDstEdge->seg = nullptr;    // New edge will run along the clip plane, this is not associated with any seg
            }
        }

        // If we get to here then we stored an edge, move along to the next edge
        #if !PSYDOOM_LIMIT_REMOVING
            ++numDstEdges;
            ++pDstEdge;

            if (numDstEdges > MAX_LEAF_EDGES) {
                I_Error("FrontZClip: Point Overflow");
            }
        #endif
    }

    // Before we finish up, save the new number of edges after clipping.
    // PsyDoom: not needed if limit removing, since the std::vector has that count already.
    #if !PSYDOOM_LIMIT_REMOVING
        outLeaf.numEdges = numDstEdges;
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check to see what side of left or right view frustrum plane all points in the leaf are on.
// Also stores what side each point is on at the start of scratchpad memory as a bool32_t.
//
// Returns:
//      -1  : If all leaf points are on the back side of the plane (leaf can be discarded completely)
//       0  : If all leaf points are on the front side of the plane (leaf does not need view frustrum clipping)
//      +1  : If some leaf points are on the inside and some on the outside of the plane (leaf needs to be clipped)
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t R_CheckLeafSide(const bool bRightViewPlane, const leaf_t& leaf) noexcept {
    // Loop vars for iterating through the leaf
    #if PSYDOOM_LIMIT_REMOVING
        const leafedge_t* pLeafEdge = leaf.edges.data();
        const int32_t numLeafEdges = (int32_t) leaf.edges.size();
    #else
        const leafedge_t* pLeafEdge = leaf.edges;
        const int32_t numLeafEdges = leaf.numEdges;
    #endif

    // Store whether each point is on the inside or outside of the leaf in scratchpad memory.
    // PsyDoom: use global memory instead, can hold a lot more! Also use normal bool.
    #if PSYDOOM_MODS
        #if PSYDOOM_LIMIT_REMOVING
            // For limit removing builds ensure the std::vector is big enough and oversize deliberately to service future allocations
            const size_t numLeafPoints = numLeafEdges + 1;

            if (gbPointsOnOutside.size() < numLeafPoints) {
                gbPointsOnOutside.resize(numLeafPoints * 2);
            }

            uint8_t* const pbPointsOnOutside = gbPointsOnOutside.data();
            uint8_t* pbPointOnOutside = gbPointsOnOutside.data();
        #else
            bool* const pbPointsOnOutside = gbPointsOnOutside;
            bool* pbPointOnOutside = gbPointsOnOutside;
        #endif
    #else
        bool32_t* const pbPointsOnOutside = (bool32_t*) LIBETC_getScratchAddr(0);
        bool32_t* pbPointOnOutside = pbPointsOnOutside;
    #endif

    // Track how many points are on the inside or outside of the plane here.
    // For each point on the inside, increment - otherwise decrement.
    int32_t insideOutsideCount = 0;

    // See which plane we are checking against, left or right view frustrum plane
    if (!bRightViewPlane) {
        for (int32_t edgeIdx = 0; edgeIdx < numLeafEdges; ++edgeIdx, ++pLeafEdge, ++pbPointOnOutside) {
            vertex_t& vert = *pLeafEdge->vertex;

            if (-vert.viewx > vert.viewy) {
                *pbPointOnOutside = true;
                --insideOutsideCount;
            } else {
                *pbPointOnOutside = false;
                ++insideOutsideCount;
            }
        }
    } else {
        for (int32_t edgeIdx = 0; edgeIdx < numLeafEdges; ++edgeIdx, ++pLeafEdge, ++pbPointOnOutside) {
            vertex_t& vert = *pLeafEdge->vertex;

            if (vert.viewx > vert.viewy) {
                *pbPointOnOutside = true;
                --insideOutsideCount;
            } else {
                *pbPointOnOutside = false;
                ++insideOutsideCount;
            }
        }
    }

    // Terminate the list of whether each leaf point is on the front side of the plane or not by duplicating
    // the first entry in the list at the end. This allows the renderer to wraparound automatically to the
    // beginning of the list when accessing 1 past the end. This saves on checks when working with edges!
    *pbPointOnOutside = pbPointsOnOutside[0];

    // Return what the renderer should do with the leaf
    if (insideOutsideCount == numLeafEdges) {
        // All points are on the inside, no clipping required
        return 0;
    } else if (insideOutsideCount == -numLeafEdges) {
        // All points are on the outside, leaf should be completely discarded
        return -1;
    } else {
        // Some points are on the inside, some on the outside - clipping is required
        return 1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clip the given input leaf against the left view frustrum plane.
// Returns the result in the given output leaf, and also the number of edges in the output leaf.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t R_LeftEdgeClip(const leaf_t& inLeaf, leaf_t& outLeaf) noexcept {
    // This clipping operation reuses the plane side checking results generated by 'R_CheckLeafSide'.
    // These results are cached at the start of the PSX scratchpad memory.
    //
    // PsyDoom: use global memory instead, can hold a lot more!
    #if PSYDOOM_MODS
        #if PSYDOOM_LIMIT_REMOVING
            const uint8_t* pbPointOnOutside = gbPointsOnOutside.data();
        #else
            const bool* pbPointOnOutside = gbPointsOnOutside;
        #endif
    #else
        const bool32_t* pbPointOnOutside = (const bool32_t*) LIBETC_getScratchAddr(0);
    #endif

    // Run through all the source edges in the given leaf and see if each edge needs to be clipped, skipped or stored as-is
    #if PSYDOOM_LIMIT_REMOVING
        const leafedge_t* pSrcEdge = inLeaf.edges.data();
        const int32_t numSrcEdges = (int32_t) inLeaf.edges.size();

        outLeaf.edges.clear();
    #else
        const leafedge_t* pSrcEdge = inLeaf.edges;
        const int32_t numSrcEdges = inLeaf.numEdges;

        leafedge_t* pDstEdge = outLeaf.edges;
        int32_t numDstEdges = 0;
    #endif

    for (int32_t srcEdgeIdx = 0; srcEdgeIdx < numSrcEdges; ++srcEdgeIdx, ++pSrcEdge, ++pbPointOnOutside) {
        // Get whether this edge point and the next are on the inside of the plane
        const bool bP1Inside = (!pbPointOnOutside[0]);
        const bool bP2Inside = (!pbPointOnOutside[1]);

        // If the 1st point is on the inside of the clipping plane, emit this edge as-is to begin with
        if (bP1Inside) {
            #if PSYDOOM_LIMIT_REMOVING
                leafedge_t* const pDstEdge = &outLeaf.edges.emplace_back();
            #endif

            pDstEdge->vertex = pSrcEdge->vertex;
            pDstEdge->seg = pSrcEdge->seg;

            #if !PSYDOOM_LIMIT_REMOVING
                ++pDstEdge;
                ++numDstEdges;
            #endif
        }

        // If both points are on the same side of the clipping plane then we do not clip
        if (bP1Inside == bP2Inside)
            continue;

        // Clipping required: grab the next edge and the vertex to output to
        const leafedge_t* pNextSrcEdge;

        if (srcEdgeIdx < numSrcEdges - 1) {
            pNextSrcEdge = pSrcEdge + 1;
        } else {
            #if PSYDOOM_LIMIT_REMOVING
                pNextSrcEdge = inLeaf.edges.data();
            #else
                pNextSrcEdge = inLeaf.edges;
            #endif
        }

        #if PSYDOOM_LIMIT_REMOVING
            ASSERT((size_t) gNumNewClipVerts < gNewClipVerts.size());   // Sanity check: buffer should always be big enough
        #endif

        vertex_t& newVert = gNewClipVerts[gNumNewClipVerts];
        gNumNewClipVerts++;

        #if !PSYDOOM_LIMIT_REMOVING
            if (gNumNewClipVerts >= MAX_NEW_CLIP_VERTS) {
                // This check seems like it was slightly incorrect - should have been done BEFORE the vertex count increment perhaps?
                // With this code it can trigger if you are at the maximum amount, but not exceeding the max...
                I_Error("LeftEdgeClip: exceeded max new vertexes\n");
            }
        #endif

        // Get the 2 points in this edge
        const vertex_t& srcVert1 = *pSrcEdge->vertex;
        const vertex_t& srcVert2 = *pNextSrcEdge->vertex;

        // Compute the intersection time of the edge against the plane.
        // Use the same method described in more detail in 'R_CheckBBox':
        fixed_t intersectT;

        {
            const int32_t a = srcVert1.viewx + srcVert1.viewy;
            const int32_t b = -srcVert2.viewx - srcVert2.viewy;
            intersectT = d_int_to_fixed(a) / (a + b);
        }

        // Compute & set the view x/y values for the clipped edge vertex
        {
            const int32_t dviewy = (srcVert2.viewy - srcVert1.viewy);
            newVert.viewy = d_fixed_to_int(dviewy * intersectT) + srcVert1.viewy;
            newVert.viewx = -newVert.viewy;
        }

        // Compute the world x/y values for the clipped edge vertex
        {
            const int32_t dx = d_fixed_to_int(srcVert2.x - srcVert1.x);
            const int32_t dy = d_fixed_to_int(srcVert2.y - srcVert1.y);
            newVert.x = dx * intersectT + srcVert1.x;
            newVert.y = dy * intersectT + srcVert1.y;
        }

        // Re-do perspective projection to compute screen x and scale for the vertex
        newVert.scale = (HALF_SCREEN_W * FRACUNIT) / newVert.viewy;
        newVert.screenx = d_fixed_to_int(newVert.viewx * newVert.scale) + HALF_SCREEN_W;

        // Mark the new vertex as having up-to-date transforms
        newVert.frameUpdated = gNumFramesDrawn;

        // If we get to here then we stored an edge so move along to the next edge and save the vertex/seg for the edge
        #if PSYDOOM_LIMIT_REMOVING
            leafedge_t* const pDstEdge = &outLeaf.edges.emplace_back();
        #endif

        pDstEdge->vertex = &newVert;
        pDstEdge->seg = pSrcEdge->seg;

        #if !PSYDOOM_LIMIT_REMOVING
            ++numDstEdges;
            ++pDstEdge;

            if (numDstEdges > MAX_LEAF_EDGES) {
                I_Error("LeftEdgeClip: Point Overflow");
            }
        #endif
    }

    // Before we finish up, save the new number of edges after clipping
    // PsyDoom: not needed if limit removing, since the std::vector has that count already.
    #if PSYDOOM_LIMIT_REMOVING
        return (int32_t) outLeaf.edges.size();
    #else
        outLeaf.numEdges = numDstEdges;
        return numDstEdges;
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clip the given input leaf against the right view frustrum plane.
// Returns the result in the given output leaf, and also the number of edges in the output leaf.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t R_RightEdgeClip(const leaf_t& inLeaf, leaf_t& outLeaf) noexcept {
    // This clipping operation reuses the plane side checking results generated by 'R_CheckLeafSide'.
    // These results are cached at the start of the PSX scratchpad memory:
    //
    // PsyDoom: use global memory instead, can hold a lot more!
    #if PSYDOOM_MODS
        #if PSYDOOM_LIMIT_REMOVING
            const uint8_t* pbPointOnOutside = gbPointsOnOutside.data();
        #else
            const bool* pbPointOnOutside = gbPointsOnOutside;
        #endif
    #else
        const bool32_t* pbPointOnOutside = (const bool32_t*) LIBETC_getScratchAddr(0);
    #endif

    // Run through all the source edges in the given leaf and see if each edge needs to be clipped, skipped or stored as-is
    #if PSYDOOM_LIMIT_REMOVING
        const leafedge_t* pSrcEdge = inLeaf.edges.data();
        const int32_t numSrcEdges = (int32_t) inLeaf.edges.size();

        outLeaf.edges.clear();
    #else
        const leafedge_t* pSrcEdge = inLeaf.edges;
        const int32_t numSrcEdges = inLeaf.numEdges;

        leafedge_t* pDstEdge = outLeaf.edges;
        int32_t numDstEdges = 0;
    #endif

    for (int32_t srcEdgeIdx = 0; srcEdgeIdx < numSrcEdges; ++srcEdgeIdx, ++pSrcEdge, ++pbPointOnOutside) {
        // Get whether this edge point and the next are on the inside of the plane
        const bool bP1Inside = (!pbPointOnOutside[0]);
        const bool bP2Inside = (!pbPointOnOutside[1]);

        // If the 1st point is on the inside of the clipping plane, emit this edge as-is to begin with
        if (bP1Inside) {
            #if PSYDOOM_LIMIT_REMOVING
                leafedge_t* const pDstEdge = &outLeaf.edges.emplace_back();
            #endif

            pDstEdge->vertex = pSrcEdge->vertex;
            pDstEdge->seg = pSrcEdge->seg;

            #if !PSYDOOM_LIMIT_REMOVING
                ++pDstEdge;
                ++numDstEdges;
            #endif
        }

        // If both points are on the same side of the clipping plane then we do not clip
        if (bP1Inside == bP2Inside)
            continue;

        // Clipping required: grab the next edge and the vertex to output to
        const leafedge_t* pNextSrcEdge;

        if (srcEdgeIdx < numSrcEdges - 1) {
            pNextSrcEdge = pSrcEdge + 1;
        } else {
            #if PSYDOOM_LIMIT_REMOVING
                pNextSrcEdge = inLeaf.edges.data();
            #else
                pNextSrcEdge = inLeaf.edges;
            #endif
        }

        #if PSYDOOM_LIMIT_REMOVING
            ASSERT((size_t) gNumNewClipVerts < gNewClipVerts.size());   // Sanity check: buffer should always be big enough
        #endif

        vertex_t& newVert = gNewClipVerts[gNumNewClipVerts];
        gNumNewClipVerts++;

        #if !PSYDOOM_LIMIT_REMOVING
            if (gNumNewClipVerts >= MAX_NEW_CLIP_VERTS) {
                // This check seems like it was slightly incorrect - should have been done BEFORE the vertex count increment perhaps?
                // With this code it can trigger if you are at the maximum amount, but not exceeding the max...
                I_Error("RightEdgeClip: exceeded max new vertexes\n");
            }
        #endif

        // Get the 2 points in this edge
        const vertex_t& srcVert1 = *pSrcEdge->vertex;
        const vertex_t& srcVert2 = *pNextSrcEdge->vertex;

        // Compute the intersection time of the edge against the plane.
        // Use the same method described in more detail in 'R_CheckBBox':
        fixed_t intersectT;

        {
            const int32_t a = srcVert1.viewx - srcVert1.viewy;
            const int32_t b = -srcVert2.viewx + srcVert2.viewy;
            intersectT = d_int_to_fixed(a) / (a + b);
        }

        // Compute & set the view x/y values for the clipped edge vertex
        {
            const int32_t dviewy = (srcVert2.viewy - srcVert1.viewy);
            newVert.viewy = d_fixed_to_int(dviewy * intersectT) + srcVert1.viewy;
            newVert.viewx = newVert.viewy;
        }

        // Compute the world x/y values for the clipped edge vertex
        {
            const int32_t dx = d_fixed_to_int(srcVert2.x - srcVert1.x);
            const int32_t dy = d_fixed_to_int(srcVert2.y - srcVert1.y);
            newVert.x = dx * intersectT + srcVert1.x;
            newVert.y = dy * intersectT + srcVert1.y;
        }

        // Re-do perspective projection to compute screen x and scale for the vertex.
        //
        // The +1 here appears to be a hack to nudge the clipped seg over by 1 pixel unit.
        // If I remove this adjustment then sometimes gaps appear for walls at the right side of the view.
        newVert.scale = ((HALF_SCREEN_W * FRACUNIT) / newVert.viewy) + 1;
        newVert.screenx = d_fixed_to_int(newVert.viewx * newVert.scale) + HALF_SCREEN_W;

        // Mark the new vertex as having up-to-date transforms
        newVert.frameUpdated = gNumFramesDrawn;

        // If we get to here then we stored an edge so move along to the next edge and save the vertex/seg for the edge
        #if PSYDOOM_LIMIT_REMOVING
            leafedge_t* const pDstEdge = &outLeaf.edges.emplace_back();
        #endif

        pDstEdge->vertex = &newVert;
        pDstEdge->seg = pSrcEdge->seg;

        #if !PSYDOOM_LIMIT_REMOVING
            ++numDstEdges;
            ++pDstEdge;

            if (numDstEdges > MAX_LEAF_EDGES) {
                I_Error("RightEdgeClip: Point Overflow");
            }
        #endif
    }

    // Before we finish up, save the new number of edges after clipping
    // PsyDoom: not needed if limit removing, since the std::vector has that count already.
    #if PSYDOOM_LIMIT_REMOVING
        return (int32_t) outLeaf.edges.size();
    #else
        outLeaf.numEdges = numDstEdges;
        return numDstEdges;
    #endif
}
