//------------------------------------------------------------------------------------------------------------------------------------------
// Logic for drawing floors and ceilings
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_VULKAN_RENDERER

#include "rv_walls.h"

#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "rv_main.h"
#include "rv_utils.h"

#include <cmath>

//------------------------------------------------------------------------------------------------------------------------------------------
// Figures out a 2D point (on the XZ plane) to act as the center of a triangle fan type arrangement for the subsector.
// The subsector is convex so we should be able to do a triangle fan from this point to every other subector edge, in order to fill it.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_CalcSubsecTriFanCenter(
    const leafedge_t* const pLeafEdges,
    const uint16_t numLeafEdges,
    float& triFanCenterX,
    float& triFanCenterZ
) noexcept {
    ASSERT(pLeafEdges);
    ASSERT(numLeafEdges >= 3);

    // Just use the first 3 points in the subsector, we don't need to average the whole lot...
    const vertex_t& v1 = *pLeafEdges[0].vertex;
    const vertex_t& v2 = *pLeafEdges[1].vertex;
    const vertex_t& v3 = *pLeafEdges[2].vertex;
    triFanCenterX = RV_FixedToFloat(v1.x + v2.x + v3.x) * (1.0f / 3.0f);
    triFanCenterZ = RV_FixedToFloat(v1.y + v2.y + v3.y) * (1.0f / 3.0f);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw a floor plane for the given subsector
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawFloorPlane(
    // Edges for the convex plane section
    const leafedge_t* const pLeafEdges,
    const uint16_t numLeafEdges,
    // Height to draw the plane at and triangle fan center
    const float planeH,
    const float triFanCenterX,
    const float triFanCenterZ,
    // Texture and shading details
    const uint8_t colR,
    const uint8_t colG,
    const uint8_t colB,
    const texture_t& tex
) noexcept {
    // Get the texture page location for this texture
    uint16_t texWinX, texWinY;
    uint16_t texWinW, texWinH;
    RV_GetTexWinXyWh(tex, texWinX, texWinY, texWinW, texWinH);

    // Do all the triangles for the plane
    for (uint16_t edgeIdx = 0; edgeIdx < numLeafEdges; ++edgeIdx) {
        // Get the edge coords
        const vertex_t& v1 = *pLeafEdges[edgeIdx].vertex;
        const vertex_t& v2 = *pLeafEdges[(edgeIdx + 1) % numLeafEdges].vertex;
        const float x1 = RV_FixedToFloat(v1.x);
        const float z1 = RV_FixedToFloat(v1.y);
        const float x2 = RV_FixedToFloat(v2.x);
        const float z2 = RV_FixedToFloat(v2.y);

        // Note that the U texture coordinates must be halved due to 8bpp textures - VRAM coords are in terms of 16bpp pixels
        const float u1 = x1 * 0.5f;
        const float u2 = x2 * 0.5f;
        const float u3 = triFanCenterX * 0.5f;

        // Draw the triangle: note that UV coords are just the vertex coords (scaled in the case of U) - no offsetting to worry about here
        VDrawing::add3dViewTriangle(
            x1, planeH, z1, u1, z1,
            x2, planeH, z2, u2, z2,
            triFanCenterX, planeH, triFanCenterZ, u3, triFanCenterZ,
            colR, colG, colB, 128,
            gClutX, gClutY,
            texWinX, texWinY, texWinW, texWinH,
            false
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the floors and ceilings for the given subsector
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawFlats(const subsector_t& subsec, const uint8_t colR, const uint8_t colG, const uint8_t colB) noexcept {
    // Ignore degenerate subsectors: I don't think this should ever be the case but add for safety
    if (subsec.numLeafEdges <= 2)
        return;

    // Get the xz point to use as the center of a triangle fan for the subsector
    const leafedge_t* const pLeafEdges = gpLeafEdges + subsec.firstLeafEdge;
    const uint16_t numLeafEdges = subsec.numLeafEdges;

    float triFanCenterX;
    float triFanCenterY;
    RV_CalcSubsecTriFanCenter(pLeafEdges, numLeafEdges, triFanCenterX, triFanCenterY);

    // Get the floor and ceiling plane height
    const sector_t& sector = *subsec.sector;
    const float floorH = RV_FixedToFloat(sector.floorheight);
    const float ceilH = RV_FixedToFloat(sector.ceilingheight);

    // Draw the floor plane if above it
    if (gViewZf > floorH) {
        texture_t& floorTex = gpFlatTextures[sector.floorpic];
        RV_DrawFloorPlane(pLeafEdges, numLeafEdges, floorH, triFanCenterX, triFanCenterY, colR, colG, colB, floorTex);
    }

    // Draw the ceiling plane if below it and not a sky (invalid ceiling pic)
    if ((gViewZf < ceilH) && (sector.ceilingpic >= 0)) {
        texture_t& ceilingTex = gpFlatTextures[sector.ceilingpic];
        RV_DrawFloorPlane(pLeafEdges, numLeafEdges, ceilH, triFanCenterX, triFanCenterY, colR, colG, colB, ceilingTex);
    }
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
