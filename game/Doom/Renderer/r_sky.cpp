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
// New for PsyDoom: draws sky wall columns for the given leaf edge starting from the specified world space z value to the top of the screen.
// The leaf edge is assumed to be front facing.
//------------------------------------------------------------------------------------------------------------------------------------------
static void R_AddFrontFacingInfiniteSkyWall(const leafedge_t& edge, const fixed_t zb) noexcept {
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
        RECT texWindow = { skytex.texPageCoordX, skytex.texPageCoordY, skytex.width, skytex.height };
        DR_MODE drawModePrim = {};
        LIBGPU_SetDrawMode(drawModePrim, false, false, skytex.texPageId, &texWindow);
        I_AddPrim(drawModePrim);
    }

    // Initialize parts of the sprite primitive used to draw the sky columns; these fields don't change from column to column:
    SPRT drawPrim = {};
    LIBGPU_SetSprt(drawPrim);
    LIBGPU_setRGB0(drawPrim, (uint8_t) 128, (uint8_t) 128, (uint8_t) 128);

    drawPrim.clut =  gPaletteClutId_CurMapSky;
    drawPrim.y0 = 0;
    drawPrim.v0 = 0;
    drawPrim.w = 1;

    // Compute the 'U' texture coordinate offset based on the view angle.
    // Also get a mask to wrap to the texture boundaries (assumes the dimensions are powers of 2):
    const uint16_t uOffset = static_cast<uint16_t>(-(int16_t)(gViewAngle >> ANGLETOSKYSHIFT));
    const uint16_t uWrapMask = skytex.width - 1;

    // Compute the bottom y coordinate step per sky wall column in screenspace, after transforming to inverted viewspace
    const int32_t iviewZb = -d_fixed_to_int(zb - gViewZ);
    const fixed_t dyb = (iviewZb * vert2.scale) - (iviewZb * vert1.scale);
    const fixed_t ybStep = dyb / dx;

    // Compute the start bottom y value and bring into screenspace
    fixed_t ybCur_frac = iviewZb * vert1.scale + HALF_VIEW_3D_H * FRACUNIT;

    // Adjust the starting column if the beginning of the seg is obscured: skip past the not visible columns
    const seg_t& seg = *edge.seg;
    int32_t xCur = x1;

    if (seg.visibleBegX > x1) {
        const int32_t numColsToSkip = seg.visibleBegX - x1;
        xCur = seg.visibleBegX;
        ybCur_frac += numColsToSkip * ybStep;
    }

    // Adjust the end column if the end of the seg is obscured: stop before the not visible columns
    const int32_t xEnd = std::min(x2, (int32_t) seg.visibleEndX);

    // Draw all of the visible sky wall columns
    while (xCur < xEnd) {
        // Get the bottom 'y' value for this sky wall column
        const int16_t ybCur = (int16_t) d_fixed_to_int(ybCur_frac);

        // Ignore the column if it is completely offscreen
        if (ybCur >= 0) {
            // Set the location and height of the column to draw.
            // Clip the height so it doesn't exceed the height of the sky texture also.
            drawPrim.x0 = (int16_t) xCur;
            drawPrim.h = (int16_t) std::min(ybCur + 1, (int32_t) skytex.height);

            // Set the 'U' texture coordinate for the column and submit
            drawPrim.u0 = (LibGpuUV)((xCur + uOffset) & uWrapMask);
            I_AddPrim(drawPrim);
        }

        ++xCur;
        ybCur_frac += ybStep;
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
        const std::byte* const pLumpData = (const std::byte*) gpLumpCache[skytex.lumpNum];
        const uint16_t* const pTexData = (const std::uint16_t*)(pLumpData + sizeof(texlump_header_t));
        RECT vramRect = getTextureVramRect(skytex);

        LIBGPU_LoadImage(vramRect, pTexData);
        skytex.uploadFrameNum = gNumFramesDrawn;
    }
    
    // Set the draw mode firstly
    {
        RECT texWindow = { skytex.texPageCoordX, skytex.texPageCoordY, skytex.width, skytex.height };

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
        LIBGPU_setUV0(spr, (uint8_t)(-(gViewAngle >> ANGLETOSKYSHIFT)), 0);
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
void R_DrawSkySegWalls(const subsector_t& subsec, const leafedge_t& edge) noexcept {
    // This must only be called if the leaf edge is in a sky sector, and also only if the leaf edge has a seg!
    ASSERT(subsec.sector->ceilingpic == -1);
    ASSERT(edge.seg);

    // If the seg for this leaf edge has no visible columns then just ignore
    const seg_t& seg = *edge.seg;

    if ((seg.flags & SGF_VISIBLE_COLS) == 0)
        return;

    // If the leaf edge is back facing then don't draw sky walls; use the screenspace delta between the vertices to detect this
    const leafedge_t& nextEdge = (&edge)[1];
    const vertex_t& v1 = *edge.vertex;
    const vertex_t& v2 = *nextEdge.vertex;

    if (v2.screenx - v1.screenx <= 0)
        return;

    // Get the top and bottom z values of the front sector
    const sector_t& frontSec = *subsec.sector;
    const fixed_t ftz = frontSec.ceilingheight;
    const fixed_t fbz = frontSec.floorheight;

    // See if the seg's line is two sided or not, may need to draw upper sky walls if two-sided
    if (seg.backsector) {
        // Get the bottom and top z values of the back sector
        const sector_t& backSec = *seg.backsector;
        const fixed_t btz = backSec.ceilingheight;
        const fixed_t bbz = backSec.floorheight;

        // Get the mid wall size so that it only occupies the gap between the upper and lower walls
        const fixed_t midTz = std::min(ftz, btz);
        const fixed_t midBz = std::max(fbz, bbz);

        // Draw a sky wall if the back sector has a normal ceiling or if there is no opening
        const bool bBackSecHasCeil = (backSec.ceilingpic >= 0);
        const bool bHasNoOpening = (midTz <= midBz);

        if (bBackSecHasCeil || bHasNoOpening) {
            // Hack special effect: treat the sky wall as a void (not to be rendered) to allow floating ceiling effects in certain situations.
            // If there is a higher surrounding sky or void ceiling then take that as an indication that this is not the true sky level and treat as a void.
            // In the "GEC Master Edition" this can be used to create things like floating cubes.
            const bool bTreatAsVoidWall = R_HasHigherSurroundingSkyOrVoidCeiling(frontSec);
            
            if (!bTreatAsVoidWall) {
                R_AddFrontFacingInfiniteSkyWall(edge, ftz);
            }
        }
    }
    else {
        // One sided line with a sky ceiling: just draw it
        R_AddFrontFacingInfiniteSkyWall(edge, ftz);
    }
}

#endif  // #if PSYDOOM_LIMIT_REMOVING
