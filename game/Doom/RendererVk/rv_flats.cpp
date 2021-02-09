//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing code for the new native Vulkan renderer: floors and ceilings
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_VULKAN_RENDERER

#include "rv_walls.h"

#include "Doom/Game/p_setup.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VTypes.h"
#include "rv_bsp.h"
#include "rv_main.h"
#include "rv_utils.h"

#include <cmath>

static int32_t gNextFloorDrawSubsecIdx;     // Index of the next draw subsector to have its floor drawn
static int32_t gNextCeilDrawSubsecIdx;      // Index of the next draw subsector to have its ceiling drawn

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
// Draw a floor or ceiling plane for the given subsector
//------------------------------------------------------------------------------------------------------------------------------------------
template <bool IsFloor>
static void RV_DrawPlane(
    // Plane details
    const subsector_t& subsec,
    const float planeH,
    // Texture and shading details
    const uint8_t colR,
    const uint8_t colG,
    const uint8_t colB,
    texture_t& tex
) noexcept {
    // Upload the texture to VRAM if required
    RV_UploadDirtyTex(tex);

    // Get the texture page location for this texture
    uint16_t texWinX, texWinY;
    uint16_t texWinW, texWinH;
    RV_GetTexWinXyWh(tex, texWinX, texWinY, texWinW, texWinH);

    // Get the xz point to use as the center of a triangle fan for the subsector
    const leafedge_t* const pLeafEdges = gpLeafEdges + subsec.firstLeafEdge;
    const uint16_t numLeafEdges = subsec.numLeafEdges;

    float triFanCenterX;
    float triFanCenterZ;
    RV_CalcSubsecTriFanCenter(pLeafEdges, numLeafEdges, triFanCenterX, triFanCenterZ);
    
    // Decide light diminishing mode depending on whether view lighting is disabled or not (disabled for visor powerup)
    const VLightDimMode lightDimMode = (gbDoViewLighting) ? VLightDimMode::Flats : VLightDimMode::None;
 
    // Ensure we have the correct draw pipeline set
    VDrawing::setDrawPipeline(gOpaqueGeomPipeline);

    // Do all the triangles for the plane.
    // Note that all draw calls assume that the correct pipeline has already been set beforehand.
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

        // Draw the triangle: note that UV coords are just the vertex coords (scaled in the case of U) - no offsetting to worry about here.
        // For ceilings as well reverse the winding order so backface culling works OK.
        if constexpr (IsFloor) {
            VDrawing::addDrawWorldTriangle(
                x1, planeH, z1, u1, z1,
                x2, planeH, z2, u2, z2,
                triFanCenterX, planeH, triFanCenterZ, u3, triFanCenterZ,
                colR, colG, colB,
                gClutX, gClutY,
                texWinX, texWinY, texWinW, texWinH,
                lightDimMode,
                128, 128, 128, 128
            );
        } else {
            VDrawing::addDrawWorldTriangle(
                x1, planeH, z1, u1, z1,
                triFanCenterX, planeH, triFanCenterZ, u3, triFanCenterZ,
                x2, planeH, z2, u2, z2,
                colR, colG, colB,
                gClutX, gClutY,
                texWinX, texWinY, texWinW, texWinH,
                lightDimMode,
                128, 128, 128, 128
            );
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the floor or ceiling plane for the specified subsector.
// Assumes the correct drawing pipeline has been set beforehand.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawFlat(const subsector_t& subsec, const bool bDrawFloor, const uint8_t colR, const uint8_t colG, const uint8_t colB) noexcept {
    // Ignore degenerate subsectors: I don't think this should ever be the case but add for safety
    if (subsec.numLeafEdges <= 2)
        return;

    // Drawing the floor or ceiling?
    const sector_t& sector = *subsec.sector;

    if (bDrawFloor) {
        // Draw the floor plane if the view is above it
        const float floorH = RV_FixedToFloat(sector.floorheight);

        if (gViewZf > floorH) {
            texture_t& floorTex = gpFlatTextures[gpFlatTranslation[sector.floorpic]];
            RV_DrawPlane<true>(subsec, floorH, colR, colG, colB, floorTex);
        }
    }
    else {
        // Draw the ceiling plane if the view is below it and if it's a normal ceiling and not a sky or void ceiling
        const float ceilH = RV_FixedToFloat(sector.ceilingheight);

        if (gViewZf < ceilH) {
            if (sector.ceilingpic >= 0) {
                texture_t& ceilingTex = gpFlatTextures[gpFlatTranslation[sector.ceilingpic]];
                RV_DrawPlane<false>(subsec, ceilH, colR, colG, colB, ceilingTex);
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Must be called before drawing subsectors and their flats.
// Initializes which subsector floor and ceiling is to be drawn next.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_InitNextDrawFlats() noexcept {
    const int32_t lastSubsecIdx = (int32_t) gRvDrawSubsecs.size() - 1;
    gNextFloorDrawSubsecIdx = lastSubsecIdx;
    gNextCeilDrawSubsecIdx = lastSubsecIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws floors starting at the given draw subsector index. Tries to batch together as many similar floors (same height) as possible.
// We draw flats in this way to try and avoid artifacts where sprites that extend into the floor/ceiling get cut off by neighboring flats.
// Sprites which exhibit this problem are explosions and fireballs. Merging flat planes helps avoid it.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSubsecFloors(const int32_t fromDrawSubsecIdx) noexcept {
    ASSERT((fromDrawSubsecIdx >= 0) && (fromDrawSubsecIdx < (int32_t) gRvDrawSubsecs.size()));

    // If we've already drawn the floors for this subsector then we are done
    if (gNextFloorDrawSubsecIdx != fromDrawSubsecIdx)
        return;

    while (true) {
        // Get the light/color value for the sector
        subsector_t& subsec = *gRvDrawSubsecs[gNextFloorDrawSubsecIdx];
        sector_t& sector = *subsec.sector;

        uint8_t secR;
        uint8_t secG;
        uint8_t secB;
        RV_GetSectorColor(sector, secR, secG, secB);

        // Draw the floor
        RV_DrawFlat(subsec, true, secR, secG, secB);
        gNextFloorDrawSubsecIdx--;

        // Should we end the draw batch here?
        if (gNextFloorDrawSubsecIdx < 0)
            break;

        if (gRvDrawSubsecs[gNextFloorDrawSubsecIdx]->sector->floorheight != sector.floorheight)
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws ceilings starting at the given draw subsector index. Tries to batch together as many similar ceilings (same height) as possible.
// We draw flats in this way to try and avoid artifacts where sprites that extend into the floor/ceiling get cut off by neighboring flats.
// Sprites which exhibit this problem are explosions and fireballs. Merging flat planes helps avoid it.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSubsecCeilings(const int32_t fromDrawSubsecIdx) noexcept {
    ASSERT((fromDrawSubsecIdx >= 0) && (fromDrawSubsecIdx < (int32_t) gRvDrawSubsecs.size()));

    // If we've already drawn the ceilings for this subsector then we are done
    if (gNextCeilDrawSubsecIdx != fromDrawSubsecIdx)
        return;

    while (true) {
        // Get the light/color value for the sector
        subsector_t& subsec = *gRvDrawSubsecs[gNextCeilDrawSubsecIdx];
        sector_t& sector = *subsec.sector;

        uint8_t secR;
        uint8_t secG;
        uint8_t secB;
        RV_GetSectorColor(sector, secR, secG, secB);

        // Draw the ceiling
        RV_DrawFlat(subsec, false, secR, secG, secB);
        gNextCeilDrawSubsecIdx--;

        // Should we end the draw batch here?
        if (gNextCeilDrawSubsecIdx < 0)
            break;

        if (gRvDrawSubsecs[gNextCeilDrawSubsecIdx]->sector->ceilingheight != sector.ceilingheight)
            break;
    }
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
