//------------------------------------------------------------------------------------------------------------------------------------------
// Drawing code for the new native Vulkan renderer: walls
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_VULKAN_RENDERER

#include "rv_walls.h"

#include "Doom/Game/doomdata.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VTypes.h"
#include "rv_main.h"
#include "rv_utils.h"

#include <cmath>

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw a wall (upper, mid, lower) for a seg.
// UV coordinates are in terms of 8bpp texels, not VRAM's 16bpp texels.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawWall(
    // Wall extents
    const float x1,
    const float z1,
    const float x2,
    const float z2,
    const float yt,
    const float yb,
    // Sort point for blended/masked walls for testing against occlusion planes
    const float sortPtX,
    const float sortPtZ,
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
        sortPtX, sortPtZ,
        colR, colG, colB,
        gClutX, gClutY,
        texWinX, texWinY, texWinW, texWinH,
        lightDimMode,
        128, 128, 128, alpha
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the fully opaque upper, lower and mid walls for a seg.
// Also add occlusion planes to that need to be drawn to that pass.
// Also marks the wall as viewed for the automap, if it's visible.
//
// Note: unlike the original PSX renderer 'R_DrawWalls' there is no height limitation placed on wall textures here.
// Therefore no stretching will occur when uv coords exceed 256 pixel limit, and this may result in rendering differences in a few places.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSegSolid(
    const seg_t& seg,
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

    // Get the xz positions of the seg endpoints in floating point format and compute the seg length
    vertex_t& v1 = *seg.vertex1;
    vertex_t& v2 = *seg.vertex2;
    const float x1 = RV_FixedToFloat(v1.x);
    const float z1 = RV_FixedToFloat(v1.y);
    const float x2 = RV_FixedToFloat(v2.x);
    const float z2 = RV_FixedToFloat(v2.y);
    const float dx = x2 - x1;
    const float dz = z2 - z1;
    const float segLen = std::sqrtf(dx * dx + dz * dz);

    // Figure out the top and bottom y values of the front sector.
    //
    // Note: use the subsector passed into this function rather than the seg's reference to it - the subsector passed in is more reliable.
    // Some maps in PSX Final Doom (MAP04, 'Combine' for example) have not all segs in a subsector pointing to that same subsector, as expected.
    // This inconsitency causes problems such as bad wall heights, due to querying the wrong sector for height.
    const sector_t& frontSec = *subsec.sector;
    const float fty = RV_FixedToFloat(frontSec.ceilingheight);
    const float fby = RV_FixedToFloat(frontSec.floorheight);

    // Compute the plane offset and angle to use for occlusion plane rendering (angle of the plane normal).
    //
    // Notes:
    //  (1) If the seg is backfacing then we need to flip the normal to get the correct result.
    //  (2) The plane offset is just a 16-bit integer quantity and in theory it could overflow.
    //      In practice it should not though, since Doom maps are limited to +/- 32767 units.
    //
    constexpr uint32_t TO_OCC_ANGLE_SHIFT = 32 - VVertex_OccPlane::ANGLE_BITS;

    const bool bSegIsBackFacing = (seg.flags & SGF_BACKFACING);
    const angle_t planeAngle = seg.angle + ANG90 + (bSegIsBackFacing ? ANG180 : 0);
    const uint16_t planeAngle16 = (uint16_t)((planeAngle >> TO_OCC_ANGLE_SHIFT) | VVertex_OccPlane::IS_PLANE_BIT);
    float planeNormalX = (+dz) / segLen;
    float planeNormalZ = (-dx) / segLen;

    float planeOffsetF = (-planeNormalX * x1 + -planeNormalZ * z1) * (bSegIsBackFacing ? -1.0f : +1.0f);
    const int16_t planeOffset = (int16_t) planeOffsetF;

    // Y values to use for stretching the occlusion planes to 'infinity' at the top and bottom of the screen
    const float OCC_PLANE_INF_TY = +65536.0f;
    const float OCC_PLANE_INF_BY = -65536.0f;

    // Get the upper/mid/lower textures for the seg.
    // Note that these array indexes are always guaranteed to be in range by the level setup code.
    texture_t& tex_u = gpTextures[gpTextureTranslation[side.toptexture]];
    texture_t& tex_m = gpTextures[gpTextureTranslation[side.midtexture]];
    texture_t& tex_l = gpTextures[gpTextureTranslation[side.bottomtexture]];

    // Get u and v offsets for the seg and compute the u1 and u2 coordinates.
    // Note that the 'u' coordinate must be divided by '2' as our texture format is always 8bpp and VRAM coordinates are in terms of 16bpp pixels.
    const float uOffset = RV_FixedToFloat(side.textureoffset + seg.offset);
    const float vOffset = RV_FixedToFloat(side.rowoffset);
    const float u1 = (uOffset) * 0.5f;
    const float u2 = (uOffset + segLen) * 0.5f;

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

        // Figure out whether we are to draw the top and bottom walls as well as the top and bottom occlusion planes.
        // We draw the top/bottom walls when there is a front face visible and if the wall is not a sky wall.
        // We draw occlusion planes for front facing walls that the eye line hits, or back facing walls that the eye line DOESN'T hit.
        const bool bDrawTransparent = (gpViewPlayer->cheats & CF_XRAYVISION);
        const bool bIsNotSkyWall = (backSec.ceilingpic != -1);
        const bool bHasUpperWall = (bty < fty);
        const bool bHasLowerWall = (bby > fby);
        const bool bEyeHitsUpperWall = (viewZ >= bty);
        const bool bEyeHitsLowerWall = (viewZ <= bby);

        const bool bDrawUpperWall = (bHasUpperWall && bSegIsFrontFacing && bIsNotSkyWall);
        const bool bDrawLowerWall = (bHasLowerWall && bSegIsFrontFacing);
        const bool bDrawUpperWallOccPlane = (bHasUpperWall && (bSegIsFrontFacing == bEyeHitsUpperWall) && bIsNotSkyWall);
        const bool bDrawLowerWallOccPlane = (bHasLowerWall && (bSegIsFrontFacing == bEyeHitsLowerWall));

        // Draw the upper wall
        if (bDrawUpperWall) {
            // Compute the top and bottom v coordinate and then draw
            const float wallH = fty - bty;
            float vt;
            float vb;

            if (line.flags & ML_DONTPEGTOP) {
                // Top of texture is at top of upper wall
                vt = vOffset;
                vb = vOffset + wallH;
            } else {
                // Bottom of texture is at bottom of upper wall
                vb = vOffset;
                vt = vOffset - wallH;
            }

            RV_DrawWall(x1, z1, x2, z2, fty, bty, {}, {}, u1, u2, vt, vb, colR, colG, colB, tex_u, bDrawTransparent);
        }

        // Draw the upper wall occlusion plane
        if (bDrawUpperWallOccPlane) {
            VDrawing::addOccPlaneWorldQuad(
                x1, OCC_PLANE_INF_TY, z1,
                x2, OCC_PLANE_INF_TY, z2,
                x2, bty, z2,
                x1, bty, z1,
                planeAngle16,
                planeOffset
            );
        }

        // Draw the lower wall
        if (bDrawLowerWall) {
            // Compute the top and bottom v coordinate and then draw
            const float heightToLower = fty - bby;
            const float wallH = bby - fby;
            float vt;
            float vb;

            if (line.flags & ML_DONTPEGBOTTOM) {
                // Don't anchor lower wall texture to the floor
                vt = vOffset + heightToLower;
                vb = vOffset + heightToLower + wallH;
            } else {
                // Anchor lower wall texture to the floor
                vt = vOffset;
                vb = vOffset + wallH;
            }

            RV_DrawWall(x1, z1, x2, z2, bby, fby, {}, {}, u1, u2, vt, vb, colR, colG, colB, tex_l, bDrawTransparent);
        }

        // Draw the lower wall occlusion plane
        if (bDrawLowerWallOccPlane) {
            VDrawing::addOccPlaneWorldQuad(
                x1, bby, z1,
                x2, bby, z2,
                x2, OCC_PLANE_INF_BY, z2,
                x1, OCC_PLANE_INF_BY, z1,
                planeAngle16,
                planeOffset
            );
        }

        // Adjust mid wall size so that it only occupies the gap between the upper and lower walls
        midTy = std::min(midTy, bty);
        midBy = std::max(midBy, bby);
    }

    // Draw the mid wall and the mid wall occlusion plane
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

        RV_DrawWall(x1, z1, x2, z2, midTy, midBy, {}, {}, u1, u2, vt, vb, colR, colG, colB, tex_m, bDrawTransparent);

        // Draw the occlusion plane
        VDrawing::addOccPlaneWorldQuad(
            x1, OCC_PLANE_INF_TY, z1,
            x2, OCC_PLANE_INF_TY, z2,
            x2, OCC_PLANE_INF_BY, z2,
            x1, OCC_PLANE_INF_BY, z1,
            planeAngle16,
            planeOffset
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the translucent or masked mid wall of a seg, if it has that.
//
// Note: unlike the original PSX renderer 'R_DrawWalls' there is no height limitation placed on wall textures here.
// Therefore no stretching will occur when uv coords exceed 256 pixel limit, and this may result in rendering differences in a few places.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSegBlended(
    const seg_t& seg,
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

    // Get the xz positions of the seg endpoints in floating point format and compute the seg length
    vertex_t& v1 = *seg.vertex1;
    vertex_t& v2 = *seg.vertex2;
    const float x1 = RV_FixedToFloat(v1.x);
    const float z1 = RV_FixedToFloat(v1.y);
    const float x2 = RV_FixedToFloat(v2.x);
    const float z2 = RV_FixedToFloat(v2.y);
    const float dx = x2 - x1;
    const float dz = z2 - z1;
    const float segLen = std::sqrtf(dx * dx + dz * dz);

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

    // Get u and v offsets for the seg and compute the u1 and u2 coordinates.
    // Note that the 'u' coordinate must be divided by '2' as our texture format is always 8bpp and VRAM coordinates are in terms of 16bpp pixels.
    const float uOffset = RV_FixedToFloat(side.textureoffset + seg.offset);
    const float vOffset = RV_FixedToFloat(side.rowoffset);
    const float u1 = (uOffset) * 0.5f;
    const float u2 = (uOffset + segLen) * 0.5f;

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

    // This is the sort point for testing against occlusion planes.
    // Use the middle of the seg for this purpose:
    const float sortPtX = (x1 + x2) * 0.5f;
    const float sortPtZ = (z1 + z2) * 0.5f;

    // Decide whether to draw the wall transparent and then draw.
    // Also make sure we are on the correct alpha blended pipeline before we draw.
    const bool bDrawTransparent = (
        (line.flags & ML_MIDTRANSLUCENT) ||
        (gpViewPlayer->cheats & CF_XRAYVISION)
    );

    VDrawing::setDrawPipeline(VPipelineType::World_AlphaGeom);
    RV_DrawWall(x1, z1, x2, z2, midTy, midBy, sortPtX, sortPtZ, u1, u2, vt, vb, colR, colG, colB, tex_m, bDrawTransparent);
}

#endif
