//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing code for the new native Vulkan renderer: walls
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
#include "rv_sky.h"
#include "rv_utils.h"

#include <cmath>

static int32_t gNextSkyWallDrawSubsecIdx;       // Index of the next draw subsector to have its sky walls drawn

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw a wall (upper, mid, lower) for a seg
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawWall(
    // Wall extents
    const float x1,
    const float z1,
    const float x2,
    const float z2,
    const float yt,
    const float yb,
    // Texture UV coords
    const float u1,
    const float u2,
    const float vt,
    const float vb,
    // Texture and shading details
    const uint8_t colR,
    const uint8_t colG,
    const uint8_t colB,
    texture_t& tex,
    const bool bBlend
) noexcept {
    // Upload the texture to VRAM if required
    RV_UploadDirtyTex(tex);

    // Get the texture page location for this texture
    uint16_t texWinX, texWinY;
    uint16_t texWinW, texWinH;
    RV_GetTexWinXyWh(tex, texWinX, texWinY, texWinW, texWinH);

    // Decide light diminishing mode depending on whether view lighting is disabled or not (disabled for visor powerup)
    const VLightDimMode lightDimMode = (gbDoViewLighting) ? VLightDimMode::Walls : VLightDimMode::None;

    // Draw the wall triangles.
    // Note: assuming the correct draw pipeline has been already set.
    const uint8_t alpha = (bBlend) ? 64 : 128;

    VDrawing::addWorldQuad(
        x1, yb, z1, u1, vb,
        x1, yt, z1, u1, vt,
        x2, yt, z2, u2, vt,
        x2, yb, z2, u2, vb,
        colR, colG, colB,
        gClutX, gClutY,
        texWinX, texWinY, texWinW, texWinH,
        lightDimMode,
        128, 128, 128, alpha
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the fully opaque upper, lower and mid walls for a seg.
// Also marks the wall as viewed for the automap, if it's visible.
//
// Note: unlike the original PSX renderer 'R_DrawWalls' there is no height limitation placed on wall textures here.
// Therefore no stretching will occur when uv coords exceed 256 pixel limit, and this may result in rendering differences in a few places.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawSegSolid(
    const rvseg_t& seg,
    const subsector_t& subsec,
    const uint8_t colR,
    const uint8_t colG,
    const uint8_t colB
) noexcept {
    // Skip the line segment if it's not front facing or visible
    if (seg.flags & SGF_BACKFACING)
        return;

    if ((seg.flags & SGF_VISIBLE_COLS) == 0)
        return;

    // This line is now viewed by the player: show in the automap if the line is viewable there
    const side_t& side = *seg.sidedef;
    line_t& line = *seg.linedef;
    line.flags |= ML_MAPPED;

    // Get the xz positions of the seg endpoints and the seg length
    const float x1 = seg.v1x;
    const float z1 = seg.v1y;
    const float x2 = seg.v2x;
    const float z2 = seg.v2y;
    const float segLen = seg.length;

    // Get the top and bottom y values of the front sector.
    //
    // Note: use the subsector passed into this function rather than the seg's reference to it - the subsector passed in is more reliable.
    // Some maps in PSX Final Doom (MAP04, 'Combine' for example) have not all segs in a subsector pointing to that same subsector, as expected.
    // This inconsitency causes problems such as bad wall heights, due to querying the wrong sector for height.
    const sector_t& frontSec = *subsec.sector;
    const float fty = RV_FixedToFloat(frontSec.ceilingheight);
    const float fby = RV_FixedToFloat(frontSec.floorDrawHeight);

    // Get the upper/mid/lower textures for the seg.
    // Note that these array indexes are always guaranteed to be in range by the level setup code.
    texture_t& tex_u = gpTextures[gpTextureTranslation[side.toptexture]];
    texture_t& tex_m = gpTextures[gpTextureTranslation[side.midtexture]];
    texture_t& tex_l = gpTextures[gpTextureTranslation[side.bottomtexture]];

    // Get u and v offsets for the seg and the u1/u2 coordinates
    const float uOffset = RV_FixedToFloat(side.textureoffset) + seg.uOffset;
    const float vOffset = RV_FixedToFloat(side.rowoffset);
    const float u1 = uOffset;
    const float u2 = uOffset + segLen;

    // These are the y values for the top and bottom of the mid wall
    float midTy = fty;
    float midBy = fby;

    // Do we draw these walls transparent? (x-ray cheat)
    const bool bDrawTransparent = (gpViewPlayer->cheats & CF_XRAYVISION);

    // See if the seg's line is two sided or not, may need to draw upper/lower walls if two-sided
    const bool bTwoSidedWall = seg.backsector;

    if (bTwoSidedWall) {
        // Get the bottom and top y values of the back sector
        const sector_t& backSec = *seg.backsector;
        const float bty = RV_FixedToFloat(backSec.ceilingheight);
        const float bby = RV_FixedToFloat(backSec.floorDrawHeight);

        // Adjust mid wall size so that it only occupies the gap between the upper and lower walls
        midTy = std::min(midTy, bty);
        midBy = std::max(midBy, bby);

        // Figure out whether there are upper and lower walls and whether the upper and lower walls should be treated as a sky walls
        const bool bIsUpperSkyWall = (backSec.ceilingpic == -1);
        const bool bIsLowerSkyWall = (backSec.floorpic == -1);
        const bool bHasUpperWall = (bty < fty);
        const bool bHasLowerWall = (bby > fby);

        // Draw the upper wall if existing not a sky wall
        if (bHasUpperWall && (!bIsUpperSkyWall)) {
            // Compute the top and bottom v coordinate and then draw
            const float wallBy = std::max(bty, fby);
            const float unclippedWallH = fty - bty;         // Upper wall may be clipped against the floor, but some texcoords are easier to calculate from unclipped height
            const float clippedWallH = fty - wallBy;
            float vt;
            float vb;

            if (line.flags & ML_DONTPEGTOP) {
                // Top of texture is at top of upper wall
                vt = vOffset;
                vb = vOffset + clippedWallH;
            } else {
                // Bottom of texture is at bottom of upper wall
                vt = vOffset - unclippedWallH;
                vb = vt + clippedWallH;
            }

            VDrawing::setDrawPipeline(gOpaqueGeomPipeline);
            RV_DrawWall(x1, z1, x2, z2, fty, wallBy, u1, u2, vt, vb, colR, colG, colB, tex_u, bDrawTransparent);
        }

        // Draw the lower wall if existing not a sky wall
        if (bHasLowerWall && (!bIsLowerSkyWall)) {
            // Compute the top and bottom v coordinate and then draw
            const float heightToLower = fty - bby;
            const float wallTy = std::min(bby, fty);
            const float unclippedWallH = bby - fby;     // Lower wall may be clipped against the ceiling, but some texcoords are easier to calculate from unclipped height
            const float clippedWallH = wallTy - fby;
            float vt;
            float vb;

            if (line.flags & ML_DONTPEGBOTTOM) {
                // Don't anchor lower wall texture to the floor
                vb = vOffset + heightToLower + unclippedWallH;
                vt = vb - clippedWallH;
            } else {
                // Anchor lower wall texture to the floor
                vb = vOffset + unclippedWallH;
                vt = vb - clippedWallH;
            }

            VDrawing::setDrawPipeline(gOpaqueGeomPipeline);
            RV_DrawWall(x1, z1, x2, z2, wallTy, fby, u1, u2, vt, vb, colR, colG, colB, tex_l, bDrawTransparent);
        }
    }

    // Draw the mid wall if the line is one sided
    if (!bTwoSidedWall) {
        // Compute the top and bottom v coordinate
        const float wallH = midTy - midBy;
        float vt;
        float vb;

        // Note: I broke with the original strange PSX wrapping behavior here (see 'R_DrawWalls' for that) for simplicity.
        // It may result in some different behavior in a few spots but it actually fixes a few texture related issues, and makes the levels look as they were intended.
        // I think the original PSX code was mostly trying to work around the limitations of 8-bit texcoords, so wound up with a few strange bugs.
        // We don't have to live with that limitation anymore for the Vulkan renderer...
        if (line.flags & ML_DONTPEGBOTTOM) {
            // Bottom of texture is at bottom of mid wall
            vb = vOffset;
            vt = vOffset - wallH;
        } else {
            // Top of texture is at top of mid wall
            vt = vOffset;
            vb = vOffset + wallH;
        }

        // Draw the wall
        VDrawing::setDrawPipeline(gOpaqueGeomPipeline);
        RV_DrawWall(x1, z1, x2, z2, midTy, midBy, u1, u2, vt, vb, colR, colG, colB, tex_m, bDrawTransparent);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the translucent or masked mid wall of a seg, if it has that.
//
// Note: unlike the original PSX renderer 'R_DrawWalls' there is no height limitation placed on wall textures here.
// Therefore no stretching will occur when uv coords exceed 256 pixel limit, and this may result in rendering differences in a few places.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawSegBlended(
    const rvseg_t& seg,
    const subsector_t& subsec,
    const uint8_t colR,
    const uint8_t colG,
    const uint8_t colB
) noexcept {
    // Skip the line segment if it's backfacing or not visible
    if (seg.flags & SGF_BACKFACING)
        return;

    if ((seg.flags & SGF_VISIBLE_COLS) == 0)
        return;

    // Must have a back sector and be translucent or masked
    line_t& line = *seg.linedef;
    const bool bCanDraw = ((seg.backsector) && (line.flags & (ML_MIDTRANSLUCENT | ML_MIDMASKED)));

    if (!bCanDraw)
        return;

    // Get the xz positions of the seg endpoints and the seg length
    const float x1 = seg.v1x;
    const float z1 = seg.v1y;
    const float x2 = seg.v2x;
    const float z2 = seg.v2y;
    const float segLen = seg.length;

    // Get the top and bottom y values of the front sector.
    //
    // Note: use the subsector passed into this function rather than the seg's reference to it - the subsector passed in is more reliable.
    // Some maps in PSX Final Doom (MAP04, 'Combine' for example) have not all segs in a subsector pointing to that same subsector, as expected.
    // This inconsitency causes problems such as bad wall heights, due to querying the wrong sector for height.
    const sector_t& frontSec = *subsec.sector;
    const float fty = RV_FixedToFloat(frontSec.ceilingheight);
    const float fby = RV_FixedToFloat(frontSec.floorDrawHeight);

    // Get the mid texture for the seg.
    // Note that this array index is always guaranteed to be in range by the level setup code.
    const side_t& side = *seg.sidedef;
    texture_t& tex_m = gpTextures[gpTextureTranslation[side.midtexture]];

    // Get u and v offsets for the seg and the u1/u2 coordinates
    const float uOffset = RV_FixedToFloat(side.textureoffset) + seg.uOffset;
    const float vOffset = RV_FixedToFloat(side.rowoffset);
    const float u1 = uOffset;
    const float u2 = uOffset + segLen;

    // Get the floor and ceiling heights for the back sector
    const sector_t& backSec = *seg.backsector;
    const float bty = RV_FixedToFloat(backSec.ceilingheight);
    const float bby = RV_FixedToFloat(backSec.floorDrawHeight);

    // These are the y values for the top and bottom of the mid wall.
    // It occupies the gap between the front and back sectors.
    float midTy = std::min(fty, bty);
    float midBy = std::max(fby, bby);

    // Final Doom: force the mid wall to be 128 units in height if this flag is specified.
    // This is used for masked fences and such, to stop them from repeating vertically - MAP23 (BALLISTYX) is a good example of this.
    // Note for PsyDoom this is restricted to two sided linedefs only, for more on that see 'R_DrawWalls' in the original renderer.
    if (line.flags & ML_MIDHEIGHT_128) {
        midTy = midBy + 128.0f;
    }

    // Compute the top and bottom v coordinate
    const float wallH = midTy - midBy;
    float vt;
    float vb;

    // Note: I broke with the original strange PSX wrapping behavior here (see 'R_DrawWalls' for that) for simplicity.
    // It may result in some different behavior in a few spots but it actually fixes a few texture related issues, and makes the levels look as they were intended.
    // I think the original PSX code was mostly trying to work around the limitations of 8-bit texcoords, so wound up with a few strange bugs.
    // We don't have to live with that limitation anymore for the Vulkan renderer...
    if (line.flags & ML_DONTPEGBOTTOM) {
        // Bottom of texture is at bottom of mid wall
        vb = vOffset;
        vt = vOffset - wallH;
    } else {
        // Top of texture is at top of mid wall
        vt = vOffset;
        vb = vOffset + wallH;
    }

    // Should this wall be drawn with 50% alpha or not?
    const bool bBlend = (line.flags & ML_MIDTRANSLUCENT);

    // Draw the wall and make sure we are on the correct alpha blended pipeline before we draw
    VDrawing::setDrawPipeline(VPipelineType::World_GeomAlpha);
    RV_DrawWall(x1, z1, x2, z2, midTy, midBy, u1, u2, vt, vb, colR, colG, colB, tex_m, bBlend);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws any 'infinite' sky walls for the specified seg.
// These include both upper and lower sky walls.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawSegSkyWalls(const rvseg_t& seg, const subsector_t& subsec) noexcept {
    // Skip the line segment if it's not visible at all or if it's back facing
    if ((seg.flags & SGF_VISIBLE_COLS) == 0)
        return;

    if (seg.flags & SGF_BACKFACING)
        return;

    // Don't draw sky walls for the line segment if it's marked as having see-through voids:
    const int32_t lineFlags = seg.linedef->flags;

    if (lineFlags & ML_VOID)
        return;

    // Does the floor or ceiling have a sky? Must have one or the other to render sky walls:
    const sector_t& frontSec = *subsec.sector;
    const bool bFrontSecHasSkyFloor = (frontSec.floorpic == -1);
    const bool bFrontSecHasSkyCeil = (frontSec.ceilingpic == -1);

    if ((!bFrontSecHasSkyFloor) && (!bFrontSecHasSkyCeil))
        return;

    // Get the xz positions of the seg endpoints
    const float x1 = seg.v1x;
    const float z1 = seg.v1y;
    const float x2 = seg.v2x;
    const float z2 = seg.v2y;

    // Get the top and bottom y values of the front sector.
    //
    // Note: use the subsector passed into this function rather than the seg's reference to it - the subsector passed in is more reliable.
    // Some maps in PSX Final Doom (MAP04, 'Combine' for example) have not all segs in a subsector pointing to that same subsector, as expected.
    // This inconsitency causes problems such as bad wall heights, due to querying the wrong sector for height.
    const float fty = RV_FixedToFloat(frontSec.ceilingheight);
    const float fby = RV_FixedToFloat(frontSec.floorDrawHeight);

    // See if the seg's line is two sided or not, have to check the back sector sky status if two sided
    if (seg.backsector) {
        // Get the bottom and top y values of the back sector
        const sector_t& backSec = *seg.backsector;
        const float bty = RV_FixedToFloat(backSec.ceilingheight);
        const float bby = RV_FixedToFloat(backSec.floorDrawHeight);

        // Get the mid wall size so that it only occupies the gap between the upper and lower walls
        const float midTy = std::min(fty, bty);
        const float midBy = std::max(fby, bby);
        const bool bHasNoOpening = (midTy <= midBy);

        // See if the back sector has a non-sky floor or ceiling
        const bool bBackSecHasFloor = (backSec.floorpic != -1);
        const bool bBackSecHasCeil = (backSec.ceilingpic != -1);

        // Can only draw an upper sky wall if there is a sky ceiling
        if (bFrontSecHasSkyCeil) {
            // Only draw a sky wall if there is a change in sky status for the back sector, or if there is no opening (treat as 1-sided line then)
            if (bBackSecHasCeil || bHasNoOpening) {
                // In certain situations treat the sky wall as a void (not to be rendered) to allow floating ceiling effects.
                // If there is a higher surrounding sky ceiling then take that as an indication that this is not the true sky level and treat as a void.
                // In the "GEC Master Edition" for example this has been used to create effects like floating cubes.
                // Only do this however if the map is not explicit about wanting a sky wall...
                const bool bRenderSkyWall = (bHasNoOpening || (lineFlags & ML_ADD_SKY_WALL_HINT) || (!R_HasHigherSurroundingSkyCeiling(frontSec)));
            
                if (bRenderSkyWall) {
                    RV_AddInfiniteSkyWall(x1, z1, x2, z2, fty, true);
                }
            }
        }

        // Can only draw a lower sky wall if there is a sky floor
        if (bFrontSecHasSkyFloor) {
            // Only draw a sky wall if there is a change in sky status for the back sector, or if there is no opening (treat as 1-sided line then)
            if (bBackSecHasFloor || bHasNoOpening) {
                // In certain situations treat the sky wall as a void (not to be rendered) to allow floating ceiling effects.
                // If there is a lower surrounding sky floor then take that as an indication that this is not the true sky level and treat as a void.
                // Only do this however if the map is not explicit about wanting a sky wall...
                const bool bRenderSkyWall = (bHasNoOpening || (lineFlags & ML_ADD_SKY_WALL_HINT) || (!R_HasLowerSurroundingSkyFloor(frontSec)));

                if (bRenderSkyWall) {
                    RV_AddInfiniteSkyWall(x1, z1, x2, z2, fby, false);
                }
            }
        }
    }
    else {
        // One sided line: always draw any required sky ceilings or floors
        if (bFrontSecHasSkyCeil) {
            RV_AddInfiniteSkyWall(x1, z1, x2, z2, fty, true);
        }

        if (bFrontSecHasSkyFloor) {
            RV_AddInfiniteSkyWall(x1, z1, x2, z2, fby, false);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Must be called before drawing sky walls.
// Initializes which subsector is to have sky walls drawn next.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_InitNextDrawSkyWalls() noexcept {
    gNextSkyWallDrawSubsecIdx = (int32_t) gRvDrawSubsecs.size() - 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw all fully opaque upper, lower and mid walls for the given subsector
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSubsecOpaqueWalls(subsector_t& subsec, const uint8_t secR, const uint8_t secG, const uint8_t secB) noexcept {
    // Draw all opaque segs in the subsector
    const rvseg_t* const pBegSeg = gpRvSegs.get() + subsec.firstseg;
    const rvseg_t* const pEndSeg = pBegSeg + subsec.numsegs;

    for (const rvseg_t* pSeg = pBegSeg; pSeg < pEndSeg; ++pSeg) {
        RV_DrawSegSolid(*pSeg, subsec, secR, secG, secB);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws all blended and masked mid walls for the specified subsector
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSubsecBlendedWalls(subsector_t& subsec, const uint8_t secR, const uint8_t secG, const uint8_t secB) noexcept {
    // Draw all blended and masked segs in the subsector
    const rvseg_t* const pBegSeg = gpRvSegs.get() + subsec.firstseg;
    const rvseg_t* const pEndSeg = pBegSeg + subsec.numsegs;

    for (const rvseg_t* pSeg = pBegSeg; pSeg < pEndSeg; ++pSeg) {
        RV_DrawSegBlended(*pSeg, subsec, secR, secG, secB);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws sky walls starting at the given draw subsector index.
// Tries to batch together as many sky walls with the same ceiling height as possible, so the number of draw calls can be reduced.
// Sky walls use a different draw pipeline to all other level geometry and sprites, therefore it's best if we can group them where possible.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSubsecSkyWalls(const int32_t fromDrawSubsecIdx) noexcept {
    ASSERT((fromDrawSubsecIdx >= 0) && (fromDrawSubsecIdx < (int32_t) gRvDrawSubsecs.size()));

    // If we've already drawn the sky walls for this subsector then we are done
    if (gNextSkyWallDrawSubsecIdx != fromDrawSubsecIdx)
        return;

    while (true) {
        // Draw any sky walls required for all segs in this subsector
        const subsector_t& subsec = *gRvDrawSubsecs[gNextSkyWallDrawSubsecIdx];
        const sector_t& sector = *subsec.sector;

        {
            const rvseg_t* const pBegSeg = gpRvSegs.get() + subsec.firstseg;
            const rvseg_t* const pEndSeg = pBegSeg + subsec.numsegs;

            for (const rvseg_t* pSeg = pBegSeg; pSeg < pEndSeg; ++pSeg) {
                RV_DrawSegSkyWalls(*pSeg, subsec);
            }
        }

        // Should we end the draw batch here? Stop if there is no next draw sector, or if the sky status or ceiling height changes
        gNextSkyWallDrawSubsecIdx--;

        if (gNextSkyWallDrawSubsecIdx < 0)
            break;

        const sector_t& nextSector = *gRvDrawSubsecs[gNextSkyWallDrawSubsecIdx]->sector;

        if (nextSector.ceilingheight != sector.ceilingheight)
            break;

        const bool bHasSkyCeil = (sector.ceilingpic == -1);
        const bool bHasSkyFloor = (sector.floorpic == -1);
        const bool bNextHasSkyCeil = (nextSector.ceilingpic == -1);
        const bool bNextHasSkyFloor = (nextSector.floorpic == -1);

        if (bHasSkyCeil != bNextHasSkyCeil)
            break;

        if (bHasSkyFloor != bNextHasSkyFloor)
            break;
    }
}

#endif
