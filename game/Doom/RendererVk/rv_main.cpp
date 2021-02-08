//------------------------------------------------------------------------------------------------------------------------------------------
// Various high level logic and utility functions for the new Vulkan world renderer.
// Note that this renderer re-uses some of the global state and functionality from the original 'r_' renderer modules.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_VULKAN_RENDERER

#include "rv_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_user.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/Renderer/r_sky.h"
#include "Doom/Renderer/r_things.h"
#include "PcPsx/Config.h"
#include "PcPsx/Utils.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VRenderer.h"
#include "PcPsx/Vulkan/VTypes.h"
#include "PsyQ/LIBGPU.h"
#include "rv_bsp.h"
#include "rv_flats.h"
#include "rv_occlusion.h"
#include "rv_sprites.h"
#include "rv_utils.h"
#include "rv_walls.h"

float           gViewXf, gViewYf, gViewZf;      // View position in floating point format
float           gViewAnglef;                    // View angle in radians (float)
float           gViewCosf, gViewSinf;           // Sin and cosine for view angle
uint16_t        gClutX, gClutY;                 // X and Y position in VRAM for the current CLUT (16-bit pixel coords)
Matrix4f        gSpriteBillboardMatrix;         // A transform matrix containing the axis vectors used for sprite billboarding
Matrix4f        gViewProjMatrix;                // The combined view and projection transform matrix for the scene
VPipelineType   gOpaqueGeomPipeline;            // The pipeline to use for drawing opaque geometry

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the sky in 'sky hole' regions of the screen that have been cut-out and marked with alpha '0'
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DrawSky() noexcept {
    // Switch back to a UI transform matrix and set the draw pipeline
    VDrawing::setDrawPipeline(VPipelineType::World_Sky);
    VDrawing::setDrawTransformMatrix(VDrawing::computeTransformMatrixForUI());

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

    // Get the texture page coordinates for the sky texture, and the CLUT coordinates
    Gpu::TexFmt texFmt = {};
    uint16_t texPageX = {};
    uint16_t texPageY = {};
    Gpu::BlendMode blendMode = {};
    RV_TexPageIdToTexParams(skytex.texPageId, texFmt, texPageX, texPageY, blendMode);

    uint16_t clutX = {};
    uint16_t clutY = {};
    RV_ClutIdToClutXy(gPaletteClutId_CurMapSky, clutX, clutY);

    // Compute the 'u' coordinate for the sky
    // TODO: support sub pixel precision for sky rotation
    const uint8_t texU = (uint8_t)(skytex.texPageCoordX - (gViewAngle >> ANGLETOSKYSHIFT));

    // Draw the sky quad itself
    // TODO: support widescreen here
    VDrawing::addDrawUISprite(
        0, 0, 256, skytex.height,
        texU, skytex.texPageCoordY,
        128, 128, 128, 128,
        clutX, clutY,
        texPageX, texPageY,
        skytex.width / 2, skytex.height
    );

    // End this draw batch and switch back to the projection matrix
    VDrawing::endCurrentDrawBatch();
    VDrawing::setDrawTransformMatrix(gViewProjMatrix);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determine various parameters affecting the draw, including view position, projection matrix and so on
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_DetermineDrawParams() noexcept {
    // Which player's viewpoint is the world being drawn from?
    player_t& player = gPlayers[gCurPlayerIndex];
    gpViewPlayer = &player;

    // Determine the view x/y/z position and the angle in fixed point terms first.
    // If we are doing an uncapped frame-rate then use interpolation to determine these parameters.
    // Note that unlike the original renderer I am not discarding the fractional component of the player's position here.
    const bool bInterpolateFrame = Config::gbUncapFramerate;
    mobj_t& playerMobj = *player.mo;

    if (bInterpolateFrame) {
        const fixed_t newViewX = playerMobj.x;
        const fixed_t newViewY = playerMobj.y;
        const fixed_t newViewZ = player.viewz;
        const angle_t newViewAngle = playerMobj.angle;
        const fixed_t lerp = R_CalcLerpFactor();

        if (gbSnapViewZInterpolation) {
            gOldViewZ = newViewZ;
            gbSnapViewZInterpolation = false;
        }

        gViewX = R_LerpCoord(gOldViewX, newViewX, lerp);
        gViewY = R_LerpCoord(gOldViewY, newViewY, lerp);
        gViewZ = R_LerpCoord(gOldViewZ, newViewZ, lerp);
        
        // View angle is not interpolated (except in demos) since turning movements are now completely framerate uncapped
        if (gbDemoPlayback) {
            gViewAngle = R_LerpAngle(gOldViewAngle, newViewAngle, lerp);
        } else {
            // Normal gameplay: take into consideration how much turning movement we haven't committed to the player object yet here.
            // For net games, we must use the view angle we said we would use NEXT as that is the most up-to-date angle.
            if (gNetGame == gt_single) {
                gViewAngle = playerMobj.angle + gPlayerUncommittedAxisTurning + gPlayerUncommittedMouseTurning;
            } else {
                gViewAngle = gPlayerNextTickViewAngle + gPlayerUncommittedAxisTurning + gPlayerUncommittedMouseTurning;
            }
        }
    }
    else {
        // Originally this is all that happened
        gViewX = playerMobj.x;
        gViewY = playerMobj.y;
        gViewZ = player.viewz;
        gViewAngle = playerMobj.angle;
    }

    // Determine view angle sine and cosine
    gViewCos = gFineCosine[gViewAngle >> ANGLETOFINESHIFT];
    gViewSin = gFineSine[gViewAngle >> ANGLETOFINESHIFT];

    // Convert the view position and angle to float for the new renderer
    gViewXf = RV_FixedToFloat(gViewX);
    gViewYf = RV_FixedToFloat(gViewY);
    gViewZf = RV_FixedToFloat(gViewZ);
    gViewAnglef = RV_AngleToFloat(gViewAngle - ANG90);
    gViewCosf = std::cosf(gViewAnglef);
    gViewSinf = std::sinf(gViewAnglef);

    // Determine the view rotation matrix used for sprite billboarding
    gSpriteBillboardMatrix = Matrix4f::rotateY(-gViewAnglef);

    // Compute the view projection matrix to use
    gViewProjMatrix = VDrawing::computeTransformMatrixFor3D(gViewXf, gViewZf, gViewYf, gViewAnglef);

    // Save the VRAM coordinate of the current palette CLUT
    RV_ClutIdToClutXy(g3dViewPaletteClutId, gClutX, gClutY);

    // Determine the pipeline to use for drawing fully opaque geometry
    if (gpViewPlayer->cheats & CF_XRAYVISION) {
        gOpaqueGeomPipeline = VPipelineType::World_SolidGeomXray;
    } else {
        gOpaqueGeomPipeline = VPipelineType::World_SolidGeom;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the player's view.
// Some of the high level logic here is copied from the original renderer's 'R_RenderPlayerView'.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_RenderPlayerView() noexcept {
    // Do nothing if drawing is currently not allowed
    if (!VRenderer::canSubmitDrawCmds())
        return;

    // Determine various draw settings and clear x-axis occlusion info to start with.
    // Then traverse the BSP tree to determine what needs to be drawn and in what order
    RV_DetermineDrawParams();
    RV_ClearOcclussion();
    RV_BuildDrawSubsecList();

    // Build the list of sprite fragments to be drawn for each subsector and set which floors/ceilings are to be drawn next
    RV_BuildSpriteFragLists();
    RV_InitNextDrawFlats();

    // Finish up any UI drawing batches first - so we don't disturb their current transform matrix.
    // Then set a compatible drawing pipeline (so we can push shader constants) and then the projection matrix to use.
    VDrawing::endCurrentDrawBatch();
    VDrawing::setDrawPipeline(gOpaqueGeomPipeline);
    VDrawing::setDrawTransformMatrix(gViewProjMatrix);

    // Draw all of the subsectors back to front
    const int32_t numDrawSubsecs = (int32_t) gRvDrawSubsecs.size();

    for (int32_t drawSubsecIdx = numDrawSubsecs - 1; drawSubsecIdx >= 0; --drawSubsecIdx) {
        // Get the light/color value for the sector
        subsector_t& subsec = *gRvDrawSubsecs[drawSubsecIdx];
        sector_t& sector = *subsec.sector;

        uint8_t secR;
        uint8_t secG;
        uint8_t secB;
        RV_GetSectorColor(sector, secR, secG, secB);

        // Draw all subsector opaque elements in the same batch (using the pipeline set above)
        RV_DrawSubsecOpaqueWalls(subsec, secR, secG, secB);
        RV_DrawSubsecFloors(drawSubsecIdx);
        RV_DrawSubsecCeilings(drawSubsecIdx);

        // Draw all subsector blended elements (this may change the draw pipeline)
        RV_DrawSubsecBlendedWalls(subsec, secR, secG, secB);
        RV_DrawSubsecSpriteFrags(drawSubsecIdx);
    }

    // Cleanup after drawing the world: need to clear the draw order for each drawn subsector
    RV_ClearSubsecDrawIndexes();

    // Draw the sky if showing, this is drawn through 'sky hole' regions of the screen that are marked and cut-out with alpha '0'
    if (gbIsSkyVisible) {
        RV_DrawSky();
    }

    // Switch back to UI renderng
    Utils::onBeginUIDrawing();

    // Set the global current light value.
    // In the old renderer this was written to constantly but here we'll just set it once for the player gun, based on the player's sector.
    // TODO: remove this when weapon drawing is implemented natively for Vulkan.
    {
        // Set the light used
        const sector_t& playerSector = *gpViewPlayer->mo->subsector->sector;
        gpCurLight = &gpLightsLump[playerSector.colorid];

        // Set the current light value
        uint8_t sectorR;
        uint8_t sectorG;
        uint8_t sectorB;
        RV_GetSectorColor(playerSector, sectorR, sectorG, sectorB);
        gCurLightValR = sectorR;
        gCurLightValG = sectorG;
        gCurLightValB = sectorB;
    }

    // TODO: implement this natively eventually for Vulkan so we can interpolate at a higher precision
    R_DrawWeapon();
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
