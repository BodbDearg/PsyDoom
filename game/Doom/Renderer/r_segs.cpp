#include "r_segs.h"

#include "Asserts.h"
#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Game/doomdata.h"
#include "PsyQ/LIBGPU.h"
#include "r_data.h"
#include "r_local.h"
#include "r_main.h"

#include <algorithm>

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the upper, lower and middle walls for the given leaf edge
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawWalls(leafedge_t& edge) noexcept {
    // Grabbing stuff being drawn
    sector_t& frontSec = *gpCurDrawSector;
    seg_t& seg = *edge.seg;
    side_t& side = *seg.sidedef;
    line_t& line = *seg.linedef;

    // This line is now viewed by the player: show in the automap if the line is viewable there
    line.flags |= ML_MAPPED;

    // Compute the top and bottom y values for the front sector in texture space, relative to the viewpoint.
    // Note: texture space y coords run in the opposite direction to viewspace z, hence this calculation is
    // inverted from what it would normally be in viewspace.
    // 
    // PsyDoom: floors and ceilings might be drawn at a different height to their real height due to interpolation etc.
    #if PSYDOOM_MODS 
        const fixed_t frontFloorH = frontSec.floorDrawH;
        const fixed_t frontCeilH = frontSec.ceilingDrawH;
    #else
        const fixed_t frontFloorH = frontSec.floorheight;
        const fixed_t frontCeilH = frontSec.ceilingheight;
    #endif

    const int32_t fsec_ty = d_fixed_to_int(gViewZ - frontCeilH);
    const int32_t fsec_by = d_fixed_to_int(gViewZ - frontFloorH);

    // Initially the mid wall texture space y coords are that of the sector.
    // Adjust as we go along if we are drawing a two sided line:
    int32_t mid_ty = fsec_ty;
    int32_t mid_by = fsec_by;

    // Draw upper and lower walls if the line is two sided (has a back sector)
    sector_t* const pBackSec = seg.backsector;

    if (pBackSec) {
        // Get the top and bottom y values for the back sector in texture space.
        // PsyDoom: floors and ceilings might be drawn at a different height to their real height due to interpolation etc.
        // PsyDoom: texture indexes can now be '-1' to indicate a wall has no texture (and therefore is not drawn).
        #if PSYDOOM_MODS
            const fixed_t backFloorH = pBackSec->floorDrawH;
            const fixed_t backCeilH = pBackSec->ceilingDrawH;
            const bool bUpperWallHasTex = (side.toptexture >= 0);
        #else
            const fixed_t backFloorH = pBackSec->floorheight;
            const fixed_t backCeilH = pBackSec->ceilingheight;
            constexpr bool bUpperWallHasTex = true;
        #endif

        const int32_t bsec_ty = d_fixed_to_int(gViewZ - backCeilH);
        const int32_t bsec_by = d_fixed_to_int(gViewZ - backFloorH);

        // Do we need to render the upper wall?
        // Do so if the ceiling lowers, and if the following texture is not sky and there is an upper wall texture (PsyDoom):
        if ((frontSec.ceilingheight > pBackSec->ceilingheight) && (pBackSec->ceilingpic != -1) && bUpperWallHasTex) {
            // Update mid texture top bound: anything above this is upper wall
            mid_ty = bsec_ty;

            // Texture display height: clamp if we exceed h/w limits, so at least we stretch in more a sensible way.
            // PsyDoom: removed this restriction because 16-bit UV coord support has been added to the GPU; this allows us to do tall walls greater than 256 units.
            int32_t tex_h = bsec_ty - fsec_ty;

            #if !PSYDOOM_LIMIT_REMOVING
                if (tex_h > TEXCOORD_MAX) {
                    tex_h = TEXCOORD_MAX;
                }
            #endif

            // Compute top and bottom texture 'v' coordinates
            int32_t vt, vb;

            if (line.flags & ML_DONTPEGTOP) {
                // Top of texture is at top of upper wall
                vt = d_fixed_to_int(side.rowoffset);
                vb = vt + tex_h;
            } else {
                // Bottom of texture is at bottom of upper wall
                vb = d_fixed_to_int(side.rowoffset) + TEXCOORD_MAX;
                vt = vb - tex_h;
            }

            // Draw the columns of wall piece
            texture_t& tex = gpTextures[gpTextureTranslation[side.toptexture]];
            R_DrawWallPiece(edge, tex, fsec_ty, bsec_ty, vt, vb, false);
        }

        // Do we need to render the lower wall? Do so if the floor raises.
        // PsyDoom: also require that the lower wall not be a sky wall, since floors can now be skies too.
        // PsyDoom: texture indexes can now be '-1' to indicate a wall has no texture (and therefore is not drawn).
        #if PSYDOOM_MODS
            const bool bIsLowerSkyWall = (pBackSec->floorpic == -1);
            const bool bLowerWallHasTex = (side.bottomtexture >= 0);
        #else
            constexpr bool bIsLowerSkyWall = false;
            constexpr bool bLowerWallHasTex = true;
        #endif

        if ((frontFloorH < backFloorH) && (!bIsLowerSkyWall) && bLowerWallHasTex) {
            // Update mid texture lower bound: anything below this is lower wall
            mid_by = bsec_by;

            // Texture display height: clamp if we exceed h/w limits, so at least we stretch in more a sensible way.
            // PsyDoom: removed this restriction because 16-bit UV coord support has been added to the GPU; this allows us to do tall walls greater than 256 units.
            int32_t tex_h = fsec_by - bsec_by;

            #if !PSYDOOM_LIMIT_REMOVING
                if (tex_h > TEXCOORD_MAX) {
                    tex_h = TEXCOORD_MAX;
                }
            #endif

            // Compute top and bottom texture 'v' coordinates
            int32_t vt, vb;

            if (line.flags & ML_DONTPEGBOTTOM) {
                // Don't anchor lower wall texture to the floor.
                // This seems to do a weird wrapping as well every 128 units - not sure why that is, probably related to the limitations of  8-bit uv coords.
                // This could maybe cause some weirdness in some cases with lower walls!
                const int32_t wall_h = bsec_by - fsec_ty;

                // PsyDoom: eliminate the weird wrapping as it's not needed anymore now with 16-bit UV coord support and could cause problems
                #if PSYDOOM_LIMIT_REMOVING
                    vt = d_fixed_to_int(side.rowoffset) + wall_h;
                #else
                    vt = (d_fixed_to_int(side.rowoffset) + wall_h) & (~128);
                #endif
            } else {
                // Anchor lower wall texture to the floor
                vt = d_fixed_to_int(side.rowoffset);
            }

            vb = vt + tex_h;

            // Draw the columns of wall piece
            texture_t& tex = gpTextures[gpTextureTranslation[side.bottomtexture]];
            R_DrawWallPiece(edge, tex, bsec_by, fsec_by, vt, vb, false);
        }

        // Only draw the mid wall for a 2 sided line if there is a masked texture or translucent wall to draw
        if ((line.flags & (ML_MIDMASKED | ML_MIDTRANSLUCENT)) == 0)
            return;
    }

    // PsyDoom: texture indexes can now be '-1' to indicate a wall has no texture (and therefore is not drawn).
    #if PSYDOOM_MODS
        const bool bMidWallHasTex = (side.midtexture >= 0);
    #else
        constexpr bool bMidWallHasTex = true;
    #endif

    // Final Doom: force the mid wall to be 128 units in height (127 for PsyDoom) if this flag is specified.
    // This is used for masked fences and such, to stop them from repeating vertically; MAP23 (BALLISTYX) is a good example of this.
    // 
    // PsyDoom: use the texture height instead of a hardcoded '128' or '127', so we can have fences shorter than 128 units.
    if (line.flags & ML_MID_FIXED_HEIGHT) {
        // PsyDoom: restricting this flag to two sided linedefs only, and only for sides that have a valid texture.
        //
        // For some strange reason one of the maps in the original PSX Doom (MAP15, Spawning Vats) has this flag set on some of the one
        // sided wall linedefs, which causes them to be clipped without this modification. Perhaps the flag meant something else temporarily
        // during the development of the original PSX Doom? I don't think it makes sense for this flag to be used on anything other than 2
        // sided lines anyway so this change should be OK to apply without condition to both Doom and Final Doom:
        #if PSYDOOM_MODS
            const bool bApplyFixedWallHeightFlag = (bMidWallHasTex && (line.flags & ML_TWOSIDED));
        #else 
            const bool bApplyFixedWallHeightFlag = true;
        #endif

        if (bApplyFixedWallHeightFlag) {
            // Note again, since texture space coords are opposite to view space coords, we do a -128 instead of +128 you might expect.
            //
            // PsyDoom: use the texture height as the fixed height, so we can have fences shorter than 128 units.
            // PsyDoom: tweak the height to be 'tex.height - 1' rather than 'tex.height' to avoid stuff wrapping around and causing artifacts on some fences.
            // I would make the adjustment smaller but we are just dealing with integer coordinates here.
            #if PSYDOOM_MODS
                texture_t& tex = gpTextures[gpTextureTranslation[side.midtexture]];
                mid_ty = mid_by - (tex.height - 1);
            #else
                mid_ty = mid_by - 128;
            #endif
        }
    }

    // Drawing the mid wall
    if (bMidWallHasTex) {
        // Texture display height: clamp if we exceed h/w limits, so at least we stretch in more a sensible way.
        // PsyDoom: removed this restriction because 16-bit UV coord support has been added to the GPU; this allows us to do tall walls greater than 256 units.
        int32_t tex_h = mid_by - mid_ty;

        #if !PSYDOOM_LIMIT_REMOVING
            if (tex_h > TEXCOORD_MAX) {
                tex_h = TEXCOORD_MAX;
            }
        #endif

        // Compute top and bottom texture 'v' coordinates
        int32_t vt, vb;

        if (line.flags & ML_DONTPEGBOTTOM) {
            // Bottom of texture is at bottom of mid wall
            vb = d_fixed_to_int(side.rowoffset) + TEXCOORD_MAX;
            vt = vb - tex_h;
        } else {
            // Top of texture is at top of mid wall
            vt = d_fixed_to_int(side.rowoffset);
            vb = vt + tex_h;
        }

        // Draw the columns of wall piece.
        // PSX specific: draw translucent as well if the linedef has that special flag.
        const bool bDrawTransparent = (line.flags & ML_MIDTRANSLUCENT);
        texture_t& tex = gpTextures[gpTextureTranslation[side.midtexture]];

        R_DrawWallPiece(edge, tex, mid_ty, mid_by, vt, vb, bDrawTransparent);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the given upper, lower or middle wall piece for a leaf edge.
// Draws all of the columns in the wall.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawWallPiece(
    const leafedge_t& edge,
    texture_t& tex,
    const int32_t yt,
    const int32_t yb,
    const int32_t vt,
    const int32_t vb,
    bool bTransparent
) noexcept {
    // Firstly determine the x size of the leaf edge onscreen: if zero or negative sized (back facing?) then ignore
    const leafedge_t& nextEdge = (&edge)[1];
    const vertex_t& vert1 = *edge.vertex;
    const vertex_t& vert2 = *nextEdge.vertex;

    const int32_t x1 = vert1.screenx;
    const int32_t x2 = vert2.screenx;
    const int32_t dx = x2 - x1;

    if (dx <= 0)
        return;

    // PsyDoom: chuck out walls that are completely offscreen to the top or bottom to avoid numeric overflows in some cases.
    // Use 64-bit arithmetic as well to do these calculations, to avoid overflows.
    #if PSYDOOM_MODS
    {
        const int64_t v1ty = ((int64_t) yt * (int64_t) vert1.scale) >> FRACBITS;
        const int64_t v1by = ((int64_t) yb * (int64_t) vert1.scale) >> FRACBITS;
        const int64_t v2ty = ((int64_t) yt * (int64_t) vert2.scale) >> FRACBITS;
        const int64_t v2by = ((int64_t) yb * (int64_t) vert2.scale) >> FRACBITS;

        // Completely offscreen at the top?
        if ((v1by < -HALF_VIEW_3D_H) && (v2by < -HALF_VIEW_3D_H))
            return;

        // Completely offscreen at the bottom?
        if ((v1ty >= HALF_VIEW_3D_H) && (v2ty >= HALF_VIEW_3D_H))
            return;
    }
    #endif

    // Force the wall to be transparent if the X-Ray vision cheat is on
    player_t& player = *gpViewPlayer;

    if (player.cheats & CF_XRAYVISION) {
        bTransparent = true;
    }

    // Decompress and upload the wall texture to VRAM if we haven't already done so.
    // This code gets invoked constantly for animated textures like the blood or slime fall textures.
    // Only one frame of the animated texture stays in VRAM at a time!
    if (tex.uploadFrameNum == TEX_INVALID_UPLOAD_FRAME_NUM) {
        // Decompress (PsyDoom: if required) and get a pointer to the texture data.
        // PsyDoom: also made updates here to work with the new WAD management code.
        #if PSYDOOM_MODS
            const WadLump& texLump = W_GetLump(tex.lumpNum);
            const void* const pLumpData = texLump.pCachedData;
        #else
            const void* const pLumpData = gpLumpCache[tex.lumpNum];
        #endif

        const std::byte* pTexData;

        #if PSYDOOM_LIMIT_REMOVING
            if (texLump.bIsUncompressed) {
                pTexData = (const std::byte*) pLumpData;
            } else {
                gTmpBuffer.ensureSize(texLump.uncompressedSize);
                decode(pLumpData, gTmpBuffer.bytes());
                pTexData = gTmpBuffer.bytes();
            }
        #else
            // PsyDoom: check for buffer overflows and issue an error if we exceed the limits
            #if PSYDOOM_MODS
                if (getDecodedSize(pLumpData) > TMP_BUFFER_SIZE) {
                    I_Error("R_DrawWallPiece: lump %d size > 64 KiB!", tex.lumpNum);
                }
            #endif

            decode(pLumpData, gTmpBuffer);
            pTexData = gTmpBuffer;
        #endif

        // Upload to the GPU and mark the texture as loaded this frame.
        // PsyDoom: also ensure texture metrics are up-to-date.
        #if PSYDOOM_MODS
            R_UpdateTexMetricsFromData(tex, pTexData, texLump.uncompressedSize);
        #endif

        const SRECT texRect = getTextureVramRect(tex);
        LIBGPU_LoadImage(texRect, (const uint16_t*)(pTexData + sizeof(texlump_header_t)));
        tex.uploadFrameNum = gNumFramesDrawn;
    }

    // Set the texture window - the area of VRAM used for texturing
    {
        SRECT texRect;
        LIBGPU_setRECT(texRect, tex.texPageCoordX, tex.texPageCoordY, tex.width, tex.height);

        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        #if PSYDOOM_MODS
            DR_TWIN texWinPrim = {};
        #else
            DR_TWIN& texWinPrim = *(DR_TWIN*) LIBETC_getScratchAddr(128);
        #endif

        LIBGPU_SetTexWindow(texWinPrim, texRect);
        I_AddPrim(texWinPrim);
    }

    // Initialization of the flat shaded textured triangle drawing primitive
    #if PSYDOOM_MODS
        // PsyDoom: switched this to the new 'wall column' drawing primitive supported by PsyDoom's enhanced GPU - this performs better and with similar results.
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state.
        WALLCOL_GT drawPrim = {};
        LIBGPU_SetWallColGT(drawPrim);
    #else
        POLY_FT3& drawPrim = *(POLY_FT3*) LIBETC_getScratchAddr(128);
        LIBGPU_SetPolyFT3(drawPrim);
    #endif

    if (bTransparent) {
        LIBGPU_SetSemiTrans(drawPrim, true);
    }

    drawPrim.clut = g3dViewPaletteClutId;
    drawPrim.tpage = tex.texPageId;

    // PsyDoom: figure out the top and bottom colors for the wall, accounting for dual colored lighting.
    // Note that these calculations might look odd because the coordinates have been inverted from worldspace at this point.
    // Getting back to worldspace Z values involves jumping through a few hoops...
    #if PSYDOOM_MODS
        uint8_t wtColR, wtColG, wtColB;
        uint8_t wbColR, wbColG, wbColB;

        const fixed_t viewZ = gViewZ;
        const sector_t& sector = *gpCurDrawSector;

        R_GetSectorDrawColor(sector, ((-yt) << FRACBITS) + viewZ, wtColR, wtColG, wtColB);
        R_GetSectorDrawColor(sector, ((-yb) << FRACBITS) + viewZ, wbColR, wbColG, wbColB);
    #endif

    // Compute scale and y coordinate step per wall column
    const fixed_t dscale = vert2.scale - vert1.scale;
    const fixed_t scaleStep = dscale / dx;

    const fixed_t dyt = (yt * vert2.scale) - (yt * vert1.scale);
    const fixed_t ytStep = dyt / dx;

    const fixed_t dyb = (yb * vert2.scale) - (yb * vert1.scale);
    const fixed_t ybStep = dyb / dx;

    // Compute some basic seg stuff, sin/cos of seg angle and relative angles and vectors.
    // Note that a lot of these amounts are shifted down to 24.8 format - presumably so fixed point math can be can done
    // in a very simplified way within the bounds of a 32-bit number without handling separate result parts etc.
    seg_t& seg = *edge.seg;
    vertex_t& segv1 = *seg.vertex1;

    const uint32_t segFineAngle = seg.angle >> ANGLETOFINESHIFT;
    const uint32_t segViewFineAngle = (seg.angle - gViewAngle + ANG90) >> ANGLETOFINESHIFT;

    const fixed_t segSin = d_rshift<8>(gFineSine[segFineAngle]);
    const fixed_t segCos = d_rshift<8>(gFineCosine[segFineAngle]);
    const fixed_t segViewSin = d_rshift<8>(gFineSine[segViewFineAngle]);
    const fixed_t segViewCos = d_rshift<8>(gFineCosine[segViewFineAngle]);
    const fixed_t segP1ViewX = d_rshift<8>(segv1.x - gViewX);
    const fixed_t segP1ViewY = d_rshift<8>(segv1.y - gViewY);

    // Compute perpendicular distance to the line segment.
    // Should be negative if we are on the inside of the line segment:
    const fixed_t segViewDot = d_rshift<8>(segP1ViewX * segSin - segP1ViewY * segCos);

    //--------------------------------------------------------------------------------------------------------------------------------------
    // 'U' texture coordinate calculation stuff:
    //
    // This stuff is a little hard to understand, and I can't 100% explain the calculations yet - needs further analysis...
    // But from examining so far it seems that what is going on here is that the column drawing loop is performing a series
    // of intersections of a line against another line in order to determine the 'u' texture coordinate for each column.
    // In the loop, one of the lines is advanced along the wall and intersected against another line, which presumably has
    // some sort of relation to the front view plane.
    //
    // Basically given the equations of two lines:
    //      a1x + b1y = d1
    //      a2x + b2y = d2
    //
    // An intersection (x,y) point can be determined (via Cramer's Rule) as follows:
    //      x = (b2d1 - b1d2)/(a1b2 - a2b1)
    //      y = (a1d2 - a2d1)/(a1b2 - a2b1)
    //
    // In the case of 'x' if we say the following:
    //      x = isectNum / isectDiv
    //
    // Filling in the blanks, the game defines the dividend and divisor as follows:
    //      isectNum = (x1 - HALF_SCREEN_W) * isectNumStep + (segViewSin * 8 * segViewDot)
    //      isectDiv = (x1 - HALF_SCREEN_W) * isectDivStep - (segViewCos * 8)
    //
    // Where:
    //      isectNumStep = (segViewDot * segViewCos) >> 4
    //      isectDivStep = segViewSin >> 4
    //
    // Decomposing further, the line equations are as follows:
    //      a1 = isectDivStep
    //      b1 = 8
    //      d1 = isectNumStep
    //
    //      a2 = segViewCos
    //      b2 = x1 - HALF_SCREEN_W
    //      d2 = -segViewDot * segViewSin
    //
    // I'm not sure what those mean exactly, but it's clear one line is being advanced and intersected against another.
    //--------------------------------------------------------------------------------------------------------------------------------------

    // Compute the stepping values used in the loop to compute u coords and also the initial components of the interesection equation
    const fixed_t isectNumStep = d_rshift<4>(segViewDot * segViewCos);
    const fixed_t isectDivStep = d_rshift<4>(segViewSin);

    fixed_t isectNum = (x1 - HALF_SCREEN_W) * isectNumStep + (segViewSin * 8 * segViewDot);
    fixed_t isectDiv = (x1 - HALF_SCREEN_W) * isectDivStep - (segViewCos * 8);

    // Compute the constant 'u' texture offset for the seg, in pixels.
    // This offset is based on the texture offset, as well as the viewpoint's distance along the seg.
    const int32_t segStartU = d_rshift<8>(
        (seg.offset + FRACUNIT + seg.sidedef->textureoffset) -
        (segP1ViewX * segCos + segP1ViewY * segSin)
    );

    // Compute the start top and bottom y values and bring into screenspace
    fixed_t ybCur_frac = yb * vert1.scale + HALF_VIEW_3D_H * FRACUNIT;
    fixed_t ytCur_frac = yt * vert1.scale + HALF_VIEW_3D_H * FRACUNIT;

    // Adjust the starting column if the beginning of the seg is obscured: skip past the not visible columns
    int32_t xCur = x1;

    if (seg.visibleBegX > x1) {
        const int32_t numColsToSkip = seg.visibleBegX - x1;
        xCur = seg.visibleBegX;

        ytCur_frac += numColsToSkip * ytStep;
        ybCur_frac += numColsToSkip * ybStep;
        isectNum += numColsToSkip * isectNumStep;
        isectDiv += numColsToSkip * isectDivStep;
    }

    // Adjust the end column if the end of the seg is obscured: stop before the not visible columns
    int32_t xEnd = x2;

    if (seg.visibleEndX < x2) {
        xEnd = seg.visibleEndX;
    }

    // Draw all of the visible wall columns
    fixed_t scaleCur = vert1.scale;

    while (xCur < xEnd) {
        // Get column pixel/integer y bounds
        int16_t ytCur = (int16_t) d_fixed_to_int(ytCur_frac);
        int16_t ybCur = (int16_t) d_fixed_to_int(ybCur_frac);

        // Ignore the column if it is completely offscreen
        if ((ytCur <= VIEW_3D_H) && (ybCur >= 0)) {
            // Compute the 'U' texture coordinate for the column
            const uint8_t uCur = (isectDiv != 0) ? (uint8_t) d_rshift<8>(isectNum / isectDiv + segStartU) : 0;

            // Some hacky-ish code to clamp the column height to the screen bounds if it gets too big, and to adjust the 'v' texture coords.
            // For the most part the engine relies on the hardware to do it's clipping for vertical columns, but if the offscreen distance
            // becomes too great then problems can start occurring and columns don't render. This is a PSX hardware limitation, where the
            // maximum bounds of primitives can't exceed around 1023x511.
            //
            // Note: the value '510' appears to be the maximum we can here use without encountering issues.
            // If you reduce this any more, then sometimes columns will not draw when you get close enough.
            //
            const int32_t colHeight = ybCur - ytCur;

            #if PSYDOOM_MODS
                // PsyDoom: if clipping then we also need to adjust the dual colored lighting parameters
                uint8_t clipWtColR = wtColR;    uint8_t clipWbColR = wbColR;
                uint8_t clipWtColG = wtColG;    uint8_t clipWbColG = wbColG;
                uint8_t clipWtColB = wtColB;    uint8_t clipWbColB = wbColB;
            #endif

            int32_t vtCur = vt;
            int32_t vbCur = vb;

            if (colHeight >= 510) {
                // Compute the amount of 'v' coordinate from the top of the column to the center of the screen.
                // PsyDoom: extend this to do similar clipping and calculations on the two wall column colors also (for dual color lighting).
                const int32_t vHeight = vbCur - vtCur;
                const fixed_t yAboveCenterPercent = d_int_to_fixed(HALF_VIEW_3D_H - ytCur) / colHeight;
                const int32_t vTopToCenter = d_fixed_to_int(yAboveCenterPercent * vHeight);

                #if PSYDOOM_MODS
                    const int32_t dColR = wbColR - wtColR;
                    const int32_t dColG = wbColG - wtColG;
                    const int32_t dColB = wbColB - wtColB;
                    const int32_t rTopToCenter = d_fixed_to_int(yAboveCenterPercent * dColR);
                    const int32_t gTopToCenter = d_fixed_to_int(yAboveCenterPercent * dColG);
                    const int32_t bTopToCenter = d_fixed_to_int(yAboveCenterPercent * dColB);
                #endif

                // Compute the amount of 'v' coordinate for half of the screen.
                // PsyDoom: extend this to do similar clipping and calculations on the two wall column colors also (for dual color lighting).
                const fixed_t halfScreenFrac = d_int_to_fixed(HALF_VIEW_3D_H) / colHeight;
                const int32_t vHalfScreen = d_fixed_to_int(halfScreenFrac * vHeight);

                #if PSYDOOM_MODS
                    const fixed_t rHalfScreen = d_fixed_to_int(halfScreenFrac * dColR);
                    const fixed_t gHalfScreen = d_fixed_to_int(halfScreenFrac * dColG);
                    const fixed_t bHalfScreen = d_fixed_to_int(halfScreenFrac * dColB);
                #endif

                // Clamp the render coordinates to the top and bottom of the screen if required
                const int32_t vtOrig = vtCur;

                if (ytCur < 0) {
                    // Offscreen to the top: advance the v coordinate by the amount offscreen
                    ytCur = 0;
                    vtCur += vTopToCenter - vHalfScreen;

                    #if PSYDOOM_MODS
                        // Note: need to guard against underflow due to rounding of potential negative numbers (color value deltas) working like a 'floor()' operation
                        clipWtColR = (uint8_t) std::max(wtColR + rTopToCenter - rHalfScreen, 0);
                        clipWtColG = (uint8_t) std::max(wtColG + gTopToCenter - gHalfScreen, 0);
                        clipWtColB = (uint8_t) std::max(wtColB + bTopToCenter - bHalfScreen, 0);
                    #endif
                }

                if (ybCur > VIEW_3D_H) {
                    // Offscreen to the bottom: stop the v coordinate at the bottom of the screen
                    ybCur = VIEW_3D_H;
                    vbCur = vtOrig + vTopToCenter + vHalfScreen;

                    #if PSYDOOM_MODS
                        // Note: need to guard against underflow due to rounding of potential negative numbers (color value deltas) working like a 'floor()' operation
                        clipWbColR = (uint8_t) std::max(wtColR + rTopToCenter + rHalfScreen, 0);
                        clipWbColG = (uint8_t) std::max(wtColG + gTopToCenter + gHalfScreen, 0);
                        clipWbColB = (uint8_t) std::max(wtColB + bTopToCenter + bHalfScreen, 0);
                    #endif
                }
            }

            // Decide on rgb color values to render the column with.
            // PsyDoom: added changes here to account for dual colored lighting.
            #if PSYDOOM_MODS
                int32_t topR, topG, topB;
                int32_t botR, botG, botB;
            #else
                int32_t r, g, b;
            #endif

            if (gbDoViewLighting) {
                int32_t lightIntensity = d_rshift<8>(scaleCur);

                if (lightIntensity < LIGHT_INTENSTIY_MIN) {
                    lightIntensity = LIGHT_INTENSTIY_MIN;
                }
                else if (lightIntensity > LIGHT_INTENSTIY_MAX) {
                    lightIntensity = LIGHT_INTENSTIY_MAX;
                }

                // PsyDoom: changes to account for dual colored lighting
                #if PSYDOOM_MODS
                    topR = ((uint32_t) lightIntensity * clipWtColR) >> 7;
                    topG = ((uint32_t) lightIntensity * clipWtColG) >> 7;
                    topB = ((uint32_t) lightIntensity * clipWtColB) >> 7;
                    botR = ((uint32_t) lightIntensity * clipWbColR) >> 7;
                    botG = ((uint32_t) lightIntensity * clipWbColG) >> 7;
                    botB = ((uint32_t) lightIntensity * clipWbColB) >> 7;
                    topR = std::min(topR, 255);
                    topG = std::min(topG, 255);
                    topB = std::min(topB, 255);
                    botR = std::min(botR, 255);
                    botG = std::min(botG, 255);
                    botB = std::min(botB, 255);
                #else
                    r = ((uint32_t) lightIntensity * gCurLightValR) >> 7;
                    g = ((uint32_t) lightIntensity * gCurLightValG) >> 7;
                    b = ((uint32_t) lightIntensity * gCurLightValB) >> 7;
                    if (r > 255) { r = 255; }
                    if (g > 255) { g = 255; }
                    if (b > 255) { b = 255; }
                #endif
            } else {
                // PsyDoom: changes to account for dual colored lighting
                #if PSYDOOM_MODS
                    topR = clipWtColR;
                    topG = clipWtColG;
                    topB = clipWtColB;
                    botR = clipWbColR;
                    botG = clipWbColG;
                    botB = clipWbColB;
                #else
                    r = gCurLightValR;
                    g = gCurLightValG;
                    b = gCurLightValB;
                #endif
            }

            // Finally populate the triangle for the wall column and draw.
            //
            // PsyDoom: now drawing this with the new GPU 'wall column' primitive instead of a polygon for better performance.
            // This new primitive also supports gouraud shading if we are using dual colored lighting.
            #if PSYDOOM_MODS
                drawPrim.u0 = (LibGpuUV) uCur;
                drawPrim.v0 = (LibGpuUV) vtCur;
                drawPrim.v1 = (LibGpuUV) vbCur;
                drawPrim.x0 = (int16_t)(xCur);
                drawPrim.y0 = (int16_t)(ytCur - 1);
                drawPrim.y1 = (int16_t)(ybCur + 1);
                drawPrim.r0 = (uint8_t) topR;
                drawPrim.g0 = (uint8_t) topG;
                drawPrim.b0 = (uint8_t) topB;
                drawPrim.r1 = (uint8_t) botR;
                drawPrim.g1 = (uint8_t) botG;
                drawPrim.b1 = (uint8_t) botB;
            #else
                LIBGPU_setRGB0(drawPrim, (uint8_t) r, (uint8_t) g, (uint8_t) b);
                LIBGPU_setUV3(drawPrim,
                    (LibGpuUV) uCur, (LibGpuUV) vtCur,
                    (LibGpuUV) uCur, (LibGpuUV) vbCur,
                    (LibGpuUV) uCur, (LibGpuUV) vbCur
                );

                LIBGPU_setXY3(drawPrim,
                    (int16_t)(xCur),     (int16_t)(ytCur - 1),
                    (int16_t)(xCur + 1), (int16_t)(ybCur + 1),
                    (int16_t)(xCur),     (int16_t)(ybCur + 1)
                );
            #endif

            I_AddPrim(drawPrim);
        }

        ++xCur;
        ytCur_frac += ytStep;
        ybCur_frac += ybStep;
        scaleCur += scaleStep;
        isectNum += isectNumStep;
        isectDiv += isectDivStep;
    }
}
