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

    VDrawing::addDrawWorldTriangle(
        x1, yb, z1, u1, vb,
        x1, yt, z1, u1, vt,
        x2, yt, z2, u2, vt,
        colR, colG, colB,
        gClutX, gClutY,
        texWinX, texWinY, texWinW, texWinH,
        lightDimMode,
        128, 128, 128, alpha
    );

    VDrawing::addDrawWorldTriangle(
        x2, yt, z2, u2, vt,
        x2, yb, z2, u2, vb,
        x1, yb, z1, u1, vb,
        colR, colG, colB,
        gClutX, gClutY,
        texWinX, texWinY, texWinW, texWinH,
        lightDimMode,
        128, 128, 128, alpha
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the upper, lower and mid walls for a seg.
//
// Note: unlike the original PSX renderer 'R_DrawWalls' there is no height limitation placed on wall textures here.
// Therefore no stretching will occur when uv coords exceed 256 pixel limit, and this may result in rendering differences in a few places.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSeg(
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

    // This line is now viewed by the player: show in the automap if the line is viewable there
    const side_t& side = *seg.sidedef;
    line_t& line = *seg.linedef;
    line.flags |= ML_MAPPED;

    // Get the xz positions of the seg endpoints in floating point format and compute the seg length.
    // TODO: consider precomputing, unclear if it's worth it though.
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

    // See if the seg's line is two sided or not
    if (seg.backsector) {
        // Figure out the bottom and top y values of the back sector and whether we are to draw the top and bottom walls.
        // We draw the top/bottom walls when there is a front face visible and if the ceiling is not a sky.
        const sector_t& backSec = *seg.backsector;
        const float bty = RV_FixedToFloat(backSec.ceilingheight);
        const float bby = RV_FixedToFloat(backSec.floorheight);

        const bool bBackSecHasSkyCeil = (backSec.ceilingpic == -1);
        const bool bDrawUpperWall = ((bty < fty) && (!bBackSecHasSkyCeil));
        const bool bDrawLowerWall = (bby > fby);

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

            const bool bDrawTransparent = (gpViewPlayer->cheats & CF_XRAYVISION);
            RV_DrawWall(x1, z1, x2, z2, fty, bty, u1, u2, vt, vb, colR, colG, colB, tex_u, bDrawTransparent);
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

            const bool bDrawTransparent = (gpViewPlayer->cheats & CF_XRAYVISION);
            RV_DrawWall(x1, z1, x2, z2, bby, fby, u1, u2, vt, vb, colR, colG, colB, tex_l, bDrawTransparent);
        }

        // Adjust mid wall size so that it only occupies the gap between the upper and lower walls
        midTy = std::min(midTy, bty);
        midBy = std::max(midBy, bby);
    }

    // Draw the mid wall, but only in these scenarios:
    //  (1) Line is 1 sided
    //  (2) There is a masked texture (like a fence)
    //  (3) The texture is blended (PSX specific new effect)
    const bool bDrawMidWall = (
        (!seg.backsector) ||
        (line.flags & ML_MIDMASKED) ||
        (line.flags & ML_MIDTRANSLUCENT)
    );

    if (bDrawMidWall) {
        // Final Doom: force the mid wall to be 128 units in height if this flag is specified.
        // This is used for masked fences and such, to stop them from repeating vertically - MAP23 (BALLISTYX) is a good example of this.
        if (line.flags & ML_MIDHEIGHT_128) {
            // PsyDoom: restricting this flag to two sided linedefs only.
            //
            // For some strange reason one of the maps in the original PSX Doom (MAP15, Spawning Vats) has this flag set on some of the one
            // sided wall linedefs, which causes them to be clipped without this modification. Perhaps the flag meant something else temporarily
            // during the development of the original PSX Doom? I don't think it makes sense for this flag to be used on anything other than 2
            // sided lines anyway so this change should be OK to apply without condition to both Doom and Final Doom:
            if (line.flags & ML_TWOSIDED) {
                midTy = midBy + 128.0f;
            }
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

        // Decide whether to draw the wall transparent and then draw
        const bool bDrawTransparent = (
            (line.flags & ML_MIDTRANSLUCENT) ||
            (gpViewPlayer->cheats & CF_XRAYVISION)
        );

        RV_DrawWall(x1, z1, x2, z2, midTy, midBy, u1, u2, vt, vb, colR, colG, colB, tex_m, bDrawTransparent);
    }
}

#endif
