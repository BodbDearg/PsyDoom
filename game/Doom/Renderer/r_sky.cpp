#include "r_sky.h"

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

// The CLUT to use for the sky
uint16_t gPaletteClutId_CurMapSky;

#if PSYDOOM_LIMIT_REMOVING
//------------------------------------------------------------------------------------------------------------------------------------------
// New for PsyDoom: draws sky wall columns for the given leaf edge starting from the specified world space z value.
// The sky is drawn to either the top or bottom of the screen. The leaf edge is assumed to be front facing.
//------------------------------------------------------------------------------------------------------------------------------------------
static void R_AddFrontFacingInfiniteSkyWall(const leafedge_t& edge, const fixed_t z, const bool bUpperSkyWall) noexcept {
    // Firstly determine the x size of the leaf edge onscreen.
    // Note: assuming the sky wall is not back facing because we've already done that check in 'R_DrawSkyEdgeWalls'.
    const leafedge_t& nextEdge = (&edge)[1];
    const vertex_t& vert1 = *edge.vertex;
    const vertex_t& vert2 = *nextEdge.vertex;

    const int32_t x1 = vert1.screenx;
    const int32_t x2 = vert2.screenx;
    const int32_t dx = x2 - x1;

    ASSERT_LOG(dx > 0, "Edge must be front facing and not zero sized!");

    // Setup the texture window and texture page for drawing the sky
    texture_t& skytex = *gpSkyTexture;

    {
        // Note: only require the sky texture width to be a power of two.
        // Set the texture window height to be the maximum allowed (no wrapping restrictions) by using a window height of '0'.
        SRECT texWindow = { (int16_t) skytex.texPageCoordX, (int16_t) skytex.texPageCoordY, skytex.width, 0 };
        DR_MODE drawModePrim = {};
        LIBGPU_SetDrawMode(drawModePrim, false, false, skytex.texPageId, &texWindow);
        I_AddPrim(drawModePrim);
    }

    // Initialize parts of the sprite primitive used to draw the sky columns; these fields don't change from column to column:
    SPRT drawPrim = {};
    LIBGPU_SetSprt(drawPrim);
    LIBGPU_setRGB0(drawPrim, (uint8_t) 128, (uint8_t) 128, (uint8_t) 128);
    LIBGPU_SetDisableMasking(drawPrim, true);

    drawPrim.clut = gPaletteClutId_CurMapSky;
    drawPrim.y0 = 0;
    drawPrim.v0 = 0;
    drawPrim.w = 1;

    // Compute the 'U' texture coordinate offset based on the view angle.
    // Also get a mask to wrap to the texture boundaries (assumes the dimensions are powers of 2):
    const uint16_t uOffset = static_cast<uint16_t>(-(int16_t)(gViewAngle >> ANGLETOSKYSHIFT));
    const uint16_t uWrapMask = skytex.width - 1;

    // Compute the y coordinate step per sky wall column in screenspace, after transforming to inverted viewspace
    const int32_t iviewZ = -d_fixed_to_int(z - gViewZ);
    const fixed_t dy = (iviewZ * vert2.scale) - (iviewZ * vert1.scale);
    const fixed_t yStep = dy / dx;

    // Compute the start y value and bring into screenspace
    fixed_t yCur_frac = iviewZ * vert1.scale + HALF_VIEW_3D_H * FRACUNIT;

    // Adjust the starting column if the beginning of the seg is obscured: skip past the not visible columns
    const seg_t& seg = *edge.seg;
    int32_t xCur = x1;

    if (seg.visibleBegX > x1) {
        const int32_t numColsToSkip = seg.visibleBegX - x1;
        xCur = seg.visibleBegX;
        yCur_frac += numColsToSkip * yStep;
    }

    // Adjust the end column if the end of the seg is obscured: stop before the not visible columns
    const int32_t xEnd = std::min(x2, (int32_t) seg.visibleEndX);

    // Draw all of the visible sky wall columns.
    // Logic is slightly different depending on whether we are doing an upper or lower sky wall.
    if (bUpperSkyWall) {
        while (xCur < xEnd) {
            // Get the start 'y' value for this sky wall column
            const int16_t yCur = (int16_t) d_fixed_to_int(yCur_frac);

            // Ignore the column if it is completely offscreen
            if (yCur >= 0) {
                // Set the location and height of the column to draw.
                // Clip the height so it doesn't exceed the height of the sky texture also.
                drawPrim.x0 = (int16_t) xCur;
                drawPrim.h = (int16_t) std::min(yCur + 1, (int32_t) skytex.height);

                // Set the 'U' texture coordinate for the column and submit
                drawPrim.u0 = (LibGpuUV)((xCur + uOffset) & uWrapMask);
                I_AddPrim(drawPrim);
            }

            ++xCur;
            yCur_frac += yStep;
        }
    }
    else {
        // Drawing a lower sky wall.
        // This is the maximum y value (exclusive): can't exceed the screen or sky texture bounds
        const int16_t endY = std::min<int16_t>(skytex.height, SCREEN_H);

        while (xCur < xEnd) {
            // Get the start 'y' value for this sky wall column
            const int16_t yCur = (int16_t) d_fixed_to_int(yCur_frac);

            // Ignore the column if it is completely offscreen or past the end of the sky texture
            if (yCur < endY) {
                // Set the location and height of the column to draw
                drawPrim.x0 = (int16_t) xCur;
                drawPrim.y0 = yCur;
                drawPrim.h = (int16_t)(endY - yCur);

                // Set the texture coordinates for the column and submit
                drawPrim.u0 = (LibGpuUV)((xCur + uOffset) & uWrapMask);
                drawPrim.v0 = (LibGpuUV) yCur;
                I_AddPrim(drawPrim);
            }

            ++xCur;
            yCur_frac += yStep;
        }
    }
}
#endif  // #if PSYDOOM_LIMIT_REMOVING

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the sky sprite.
// Unlike most other versions of DOOM, the PSX version draws the sky as a single sprite rather than individual columns.
// This means that the cylindrical mapping that is normally present is not here in this version of the game.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawSky() noexcept {
    // Do we need to upload the fire sky texture? If so then upload it...
    // This code only executes for the fire sky - the regular sky is already in VRAM at this point.
    texture_t& skytex = *gpSkyTexture;

    if (skytex.uploadFrameNum == TEX_INVALID_UPLOAD_FRAME_NUM) {
        // PsyDoom: updates to work with the new WAD management code
        #if PSYDOOM_MODS
            const WadLump& wadLump = W_GetLump(skytex.lumpNum);
            const std::byte* const pLumpData = (const std::byte*) wadLump.pCachedData;
        #else
            const std::byte* const pLumpData = (const std::byte*) gpLumpCache[skytex.lumpNum];
        #endif

        const uint16_t* const pTexData = (const std::uint16_t*)(pLumpData + sizeof(texlump_header_t));
        SRECT vramRect = getTextureVramRect(skytex);

        LIBGPU_LoadImage(vramRect, pTexData);
        skytex.uploadFrameNum = gNumFramesDrawn;
    }

    // Set the draw mode firstly
    {
        // Note: only require the sky texture width to be a power of two.
        // Set the texture window height to be the maximum allowed (no wrapping restrictions) by using a window height of '0'.
        SRECT texWindow = { (int16_t) skytex.texPageCoordX, (int16_t) skytex.texPageCoordY, skytex.width, 0 };

        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        #if PSYDOOM_MODS
            DR_MODE drawMode = {};
        #else
            DR_MODE& drawMode = *(DR_MODE*) LIBETC_getScratchAddr(128);
        #endif

        LIBGPU_SetDrawMode(drawMode, false, false, skytex.texPageId, &texWindow);
        I_AddPrim(drawMode);
    }

    // Setup and draw the sky sprite
    #if PSYDOOM_MODS
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        SPRT spr = {};
    #else
        SPRT& spr = *(SPRT*) LIBETC_getScratchAddr(128);
    #endif

    LIBGPU_SetSprt(spr);
    LIBGPU_SetShadeTex(spr, true);
    LIBGPU_setXY0(spr, 0, 0);
    LIBGPU_setWH(spr, SCREEN_W, skytex.height);

    #if PSYDOOM_MODS
        // PsyDoom: I think the original UV calculations were not correct, because we've already set a texture window - coords should be relative to that.
        // It probably just always happened to work correctly in spite of this due to where the sky texture was located in VRAM.
        LIBGPU_setUV0(spr, (LibGpuUV)(-(gViewAngle >> ANGLETOSKYSHIFT)), 0);

        // PsyDoom: also disable masking for this sprite, skies should always be fully opaque.
        // This is required for the 'sky leak fix' to work correctly in some instances with the fire sky...
        LIBGPU_SetDisableMasking(spr, true);
    #else
        LIBGPU_setUV0(spr, (uint8_t)(skytex.texPageCoordX - (gViewAngle >> ANGLETOSKYSHIFT)), skytex.texPageCoordY);
    #endif

    spr.clut = gPaletteClutId_CurMapSky;
    I_AddPrim(spr);
}

#if PSYDOOM_LIMIT_REMOVING
//------------------------------------------------------------------------------------------------------------------------------------------
// New for PsyDoom: draw 'sky wall' columns for the specified leaf edge that is contained in a sky sector and which also has a seg.
// These sky columns extend up from the top of the subsector to the top of the screen and are used to fix rooms beyond the sky sometimes
// poking through when the ceiling in the room beyond is higher than the sky.
// This is considered an engine limit removing feature and the logic here largely mirrors 'RV_DrawSkySegSkyWalls' in the Vulkan renderer.
//------------------------------------------------------------------------------------------------------------------------------------------
void R_DrawSegSkyWalls(const subsector_t& subsec, const leafedge_t& edge) noexcept {
    // If the seg for this leaf edge has no visible columns then just ignore
    ASSERT(edge.seg);
    const seg_t& seg = *edge.seg;

    if ((seg.flags & SGF_VISIBLE_COLS) == 0)
        return;

    // If the leaf edge is back facing then don't draw sky walls; use the screenspace delta between the vertices to detect this
    const leafedge_t& nextEdge = (&edge)[1];
    const vertex_t& v1 = *edge.vertex;
    const vertex_t& v2 = *nextEdge.vertex;

    if (v2.screenx - v1.screenx <= 0)
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

    // Get the top and bottom z values of the front sector
    const fixed_t ftz = frontSec.ceilingheight;
    const fixed_t fbz = frontSec.floorDrawHeight;

    // See if the seg's line is two sided or not, have to check the back sector sky status if two sided
    if (seg.backsector) {
        // Get the bottom and top z values of the back sector
        const sector_t& backSec = *seg.backsector;
        const fixed_t btz = backSec.ceilingheight;
        const fixed_t bbz = backSec.floorDrawHeight;

        // Get the mid wall size so that it only occupies the gap between the upper and lower walls
        const fixed_t midTz = std::min(ftz, btz);
        const fixed_t midBz = std::max(fbz, bbz);
        const bool bHasNoOpening = (midTz <= midBz);

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
                    R_AddFrontFacingInfiniteSkyWall(edge, ftz, true);
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
                    R_AddFrontFacingInfiniteSkyWall(edge, fbz, false);
                }
            }
        }
    }
    else {
        // One sided line: always draw any required sky ceilings or floors
        if (bFrontSecHasSkyCeil) {
            R_AddFrontFacingInfiniteSkyWall(edge, ftz, true);
        }

        if (bFrontSecHasSkyFloor) {
            R_AddFrontFacingInfiniteSkyWall(edge, fbz, false);
        }
    }
}
#endif  // #if PSYDOOM_LIMIT_REMOVING
