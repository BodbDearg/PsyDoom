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
#include "rv_data.h"
#include "rv_main.h"
#include "rv_sky.h"
#include "rv_utils.h"

#include <cmath>

//------------------------------------------------------------------------------------------------------------------------------------------
// Check for a sector surrounding the given sector which has both a sky or void ceiling and is higher
//------------------------------------------------------------------------------------------------------------------------------------------
static bool RV_HasHigherSurroundingSkyOrVoidCeiling(const sector_t& sector) noexcept {
    const int32_t numLines = sector.linecount;
    const line_t* const* const pLines = sector.lines;

    for (int32_t lineIdx = 0; lineIdx < numLines; ++lineIdx) {
        const line_t& line = *pLines[lineIdx];
        const sector_t* pNextSector = (line.frontsector == &sector) ? line.backsector : line.frontsector;

        if (pNextSector && (pNextSector->ceilingpic < 0)) {
            if (pNextSector->ceilingheight > sector.ceilingheight)
                return true;
        }
    }

    return false;
}

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

    VDrawing::addDrawWorldQuad(
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
    // Skip the line segment if it's not visible at all
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
    const float dx = x2 - x1;
    const float dz = z2 - z1;
    const float segLen = seg.length;

    // Figure out the top and bottom y values of the front sector.
    //
    // Note: use the subsector passed into this function rather than the seg's reference to it - the subsector passed in is more reliable.
    // Some maps in PSX Final Doom (MAP04, 'Combine' for example) have not all segs in a subsector pointing to that same subsector, as expected.
    // This inconsitency causes problems such as bad wall heights, due to querying the wrong sector for height.
    const sector_t& frontSec = *subsec.sector;
    const float fty = RV_FixedToFloat(frontSec.ceilingheight);
    const float fby = RV_FixedToFloat(frontSec.floorheight);

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

    // Grabbing some useful info: view height and front-facing status
    const float viewZ = gViewZf;
    const bool bSegIsFrontFacing = ((seg.flags & SGF_BACKFACING) == 0);

    // See if the seg's line is two sided or not, may need to draw upper/lower walls if two-sided
    if (seg.backsector) {
        // Figure out the bottom and top y values of the back sector and whether we are to draw the top and bottom walls
        const sector_t& backSec = *seg.backsector;
        const float bty = RV_FixedToFloat(backSec.ceilingheight);
        const float bby = RV_FixedToFloat(backSec.floorheight);

        // Adjust mid wall size so that it only occupies the gap between the upper and lower walls
        midTy = std::min(midTy, bty);
        midBy = std::max(midBy, bby);

        // Figure out whether we are to draw the top and bottom walls.
        // We draw the top/bottom walls when there is a front face visible and if the wall is not a sky wall.
        const bool bDrawTransparent = (gpViewPlayer->cheats & CF_XRAYVISION);
        const bool bIsSkyOrVoidWall = (backSec.ceilingpic < 0);
        const bool bHasUpperWall = (bty < fty);
        const bool bHasLowerWall = (bby > fby);
        const bool bDrawUpperWall = (bHasUpperWall && bSegIsFrontFacing && (!bIsSkyOrVoidWall));
        const bool bDrawLowerWall = (bHasLowerWall && bSegIsFrontFacing);

        // Draw the upper wall
        if (bDrawUpperWall) {
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

        // Draw a sky wall if there is a sky
        const bool bHasNoOpening = (midTy <= midBy);

        if ((frontSec.ceilingpic == -1) && ((backSec.ceilingpic >= 0) || bHasNoOpening) && bSegIsFrontFacing) {
            // Hack special effect: treat the sky wall plane as a void (not to be rendered) and allow floating ceiling effects in certain situations.
            // If the ceiling is a sky and the next highest sky or void ceiling is higher then treat the sky ceiling as if it were a void ceiling too.
            // In the "GEC Master Edition" this can be used to create things like floating cubes, and in "Ballistyx" the altar top appears to be lower than surrounding building walls.
            const bool bTreatAsVoidCeiling = RV_HasHigherSurroundingSkyOrVoidCeiling(frontSec);
            const float skyBy = (backSec.ceilingpic == -1) ? std::max(fby, bby) : fty;

            if (!bTreatAsVoidCeiling) {
                RV_AddInfiniteSkyWall(x1, z1, x2, z2, skyBy);
            }
        }

        // Draw the lower wall
        if (bDrawLowerWall) {
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

    // Draw the mid wall
    const bool bDrawMidWall = (bSegIsFrontFacing && (!seg.backsector));

    if (bDrawMidWall) {
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

        // Decide whether to draw the wall transparent and then draw
        const bool bDrawTransparent = (
            (line.flags & ML_MIDTRANSLUCENT) ||
            (gpViewPlayer->cheats & CF_XRAYVISION)
        );

        VDrawing::setDrawPipeline(gOpaqueGeomPipeline);
        RV_DrawWall(x1, z1, x2, z2, midTy, midBy, u1, u2, vt, vb, colR, colG, colB, tex_m, bDrawTransparent);

        // Draw a sky wall if there is a sky
        if (frontSec.ceilingpic == -1) {
            RV_AddInfiniteSkyWall(x1, z1, x2, z2, fty);
        }
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

    // Must have a back sector and masked or translucent elements
    line_t& line = *seg.linedef;

    const bool bCanDraw = (
        (seg.backsector) &&
        (line.flags & (ML_MIDMASKED | ML_MIDTRANSLUCENT))
    );

    if (!bCanDraw)
        return;

    // Get the xz positions of the seg endpoints and the seg length
    const float x1 = seg.v1x;
    const float z1 = seg.v1y;
    const float x2 = seg.v2x;
    const float z2 = seg.v2y;
    const float dx = x2 - x1;
    const float dz = z2 - z1;
    const float segLen = seg.length;

    // Figure out the top and bottom y values of the front sector.
    //
    // Note: use the subsector passed into this function rather than the seg's reference to it - the subsector passed in is more reliable.
    // Some maps in PSX Final Doom (MAP04, 'Combine' for example) have not all segs in a subsector pointing to that same subsector, as expected.
    // This inconsitency causes problems such as bad wall heights, due to querying the wrong sector for height.
    const sector_t& frontSec = *subsec.sector;
    const float fty = RV_FixedToFloat(frontSec.ceilingheight);
    const float fby = RV_FixedToFloat(frontSec.floorheight);

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
    const float bby = RV_FixedToFloat(backSec.floorheight);

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

    // Decide whether to draw the wall transparent and then draw.
    // Also make sure we are on the correct alpha blended pipeline before we draw.
    const bool bDrawTransparent = (
        (line.flags & ML_MIDTRANSLUCENT) ||
        (gpViewPlayer->cheats & CF_XRAYVISION)
    );

    VDrawing::setDrawPipeline(VPipelineType::World_AlphaGeom);
    RV_DrawWall(x1, z1, x2, z2, midTy, midBy, u1, u2, vt, vb, colR, colG, colB, tex_m, bDrawTransparent);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw all fully opaque walls for the given subsector
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
// Draws all blended walls for the specified subsector
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSubsecBlendedWalls(subsector_t& subsec, const uint8_t secR, const uint8_t secG, const uint8_t secB) noexcept {
    // Draw all blended and masked segs in the subsector
    const rvseg_t* const pBegSeg = gpRvSegs.get() + subsec.firstseg;
    const rvseg_t* const pEndSeg = pBegSeg + subsec.numsegs;

    for (const rvseg_t* pSeg = pBegSeg; pSeg < pEndSeg; ++pSeg) {
        RV_DrawSegBlended(*pSeg, subsec, secR, secG, secB);
    }
}

#endif
