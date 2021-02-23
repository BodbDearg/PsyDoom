//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing code for the new native Vulkan renderer: floors and ceilings
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_VULKAN_RENDERER

#include "rv_walls.h"

#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VTypes.h"
#include "rv_bsp.h"
#include "rv_data.h"
#include "rv_main.h"
#include "rv_utils.h"

#include <cmath>

static int32_t gNextFloorDrawSubsecIdx;     // Index of the next draw subsector to have its floor drawn
static int32_t gNextCeilDrawSubsecIdx;      // Index of the next draw subsector to have its ceiling drawn

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: tells if the specified subsector has any visible masked or blended walls
//------------------------------------------------------------------------------------------------------------------------------------------
static bool RV_SubsecHasVisibleMaskedOrBlendedMidWalls(const subsector_t& subsec) noexcept {
    const rvseg_t* const pSegs = gpRvSegs.get() + subsec.firstseg;
    const int32_t numSegs = subsec.numsegs;

    for (int32_t segIdx = 0; segIdx < numSegs; ++segIdx) {
        const rvseg_t& seg = pSegs[segIdx];

        if (seg.flags & SGF_BACKFACING)
            continue;

        const uint32_t lineFlags = seg.linedef->flags;

        if (lineFlags & (ML_MIDMASKED | ML_MIDTRANSLUCENT))
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Figures out a 2D point (on the XZ plane) to act as the center of a triangle fan type arrangement for the subsector.
// The subsector is convex so we should be able to do a triangle fan from this point to every other subector edge, in order to fill it.
// N.B: assumes the subsector has at least 3 leaf edges!
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_CalcSubsecTriFanCenter(
    const rvleafedge_t* const pLeafEdges,
    float& triFanCenterX,
    float& triFanCenterZ
) noexcept {
    // Just use the first 3 points in the subsector, we don't need to average the whole lot...
    ASSERT(pLeafEdges);
    const rvleafedge_t& e1 = pLeafEdges[0];
    const rvleafedge_t& e2 = pLeafEdges[1];
    const rvleafedge_t& e3 = pLeafEdges[2];
    triFanCenterX = (e1.v1x + e2.v1x + e3.v1x) * (1.0f / 3.0f);
    triFanCenterZ = (e1.v1y + e2.v1y + e3.v1y) * (1.0f / 3.0f);
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
    const rvleafedge_t* const pLeafEdges = gpRvLeafEdges.get() + subsec.firstLeafEdge;
    const uint16_t numLeafEdges = subsec.numLeafEdges;
    ASSERT(numLeafEdges >= 3);

    float triFanCenterX;
    float triFanCenterZ;
    RV_CalcSubsecTriFanCenter(pLeafEdges, triFanCenterX, triFanCenterZ);
    
    // Decide light diminishing mode depending on whether view lighting is disabled or not (disabled for visor powerup)
    const VLightDimMode lightDimMode = (gbDoViewLighting) ? VLightDimMode::Flats : VLightDimMode::None;
 
    // Ensure we have the correct draw pipeline set
    VDrawing::setDrawPipeline(gOpaqueGeomPipeline);

    // Do all the triangles for the plane.
    // Note that all draw calls assume that the correct pipeline has already been set beforehand.
    for (uint16_t edgeIdx = 0; edgeIdx < numLeafEdges; ++edgeIdx) {
        // Get the edge coords
        const rvleafedge_t& e1 = pLeafEdges[edgeIdx];
        const rvleafedge_t& e2 = pLeafEdges[(edgeIdx + 1) % numLeafEdges];
        const float x1 = e1.v1x;
        const float z1 = e1.v1y;
        const float x2 = e2.v1x;
        const float z2 = e2.v1y;

        // Draw the triangle: note that UV coords are just the vertex coords (scaled in the case of U) - no offsetting to worry about here.
        // For ceilings as well reverse the winding order so backface culling works OK.
        if constexpr (IsFloor) {
            VDrawing::addWorldTriangle(
                x1, planeH, z1, x1, z1,
                x2, planeH, z2, x2, z2,
                triFanCenterX, planeH, triFanCenterZ, triFanCenterX, triFanCenterZ,
                colR, colG, colB,
                gClutX, gClutY,
                texWinX, texWinY, texWinW, texWinH,
                lightDimMode,
                128, 128, 128, 128
            );
        } else {
            VDrawing::addWorldTriangle(
                x1, planeH, z1, x1, z1,
                triFanCenterX, planeH, triFanCenterZ, triFanCenterX, triFanCenterZ,
                x2, planeH, z2, x2, z2,
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
        const subsector_t& subsec = *gRvDrawSubsecs[gNextFloorDrawSubsecIdx];
        const sector_t& sector = *subsec.sector;

        uint8_t secR;
        uint8_t secG;
        uint8_t secB;
        RV_GetSectorColor(sector, secR, secG, secB);

        // Draw the floor
        RV_DrawFlat(subsec, true, secR, secG, secB);
        gNextFloorDrawSubsecIdx--;

        // Should we end the draw batch here? Stop if there is no next draw sector or if the floor height changes.
        if (gNextFloorDrawSubsecIdx < 0)
            break;

        const subsector_t& nextSubsec = *gRvDrawSubsecs[gNextFloorDrawSubsecIdx];
        const sector_t& nextSector = *nextSubsec.sector;

        if (nextSector.floorheight != sector.floorheight)
            break;

        // Break the batch if the next subsector has visible masked or blended mid walls.
        // We can't batch across those without causing ordering issues for sprites behind them, which should be occluded by stuff in front:
        if (RV_SubsecHasVisibleMaskedOrBlendedMidWalls(nextSubsec))
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
        const subsector_t& subsec = *gRvDrawSubsecs[gNextCeilDrawSubsecIdx];
        const sector_t& sector = *subsec.sector;

        uint8_t secR;
        uint8_t secG;
        uint8_t secB;
        RV_GetSectorColor(sector, secR, secG, secB);

        // Draw the ceiling
        RV_DrawFlat(subsec, false, secR, secG, secB);
        gNextCeilDrawSubsecIdx--;

        // Should we end the draw batch here? Stop if there is no next draw sector or if the ceiling height changes.
        if (gNextCeilDrawSubsecIdx < 0)
            break;

        const subsector_t& nextSubsec = *gRvDrawSubsecs[gNextCeilDrawSubsecIdx];
        const sector_t& nextSector = *nextSubsec.sector;

        if (nextSector.ceilingheight != sector.ceilingheight)
            break;

        // Break the batch if the next subsector has visible masked or blended mid walls.
        // We can't batch across those without causing ordering issues for sprites behind them, which should be occluded by stuff in front:
        if (RV_SubsecHasVisibleMaskedOrBlendedMidWalls(nextSubsec))
            break;

        // Also break the batch if there is a change in sky or void ceiling status.
        // If we don't do this then sky walls can sometimes bleed through to other neighboring ceilings:
        const bool bIsSkyCeil = (sector.ceilingpic == -1);
        const bool bIsNextSkyCeil = (nextSector.ceilingpic == -1);
        const bool bIsVoidCeil = (sector.ceilingpic < -1);
        const bool bIsNextVoidCeil = (nextSector.ceilingpic < -1);

        if (bIsSkyCeil != bIsNextSkyCeil)
            break;

        if (bIsVoidCeil != bIsNextVoidCeil)
            break;
    }
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
