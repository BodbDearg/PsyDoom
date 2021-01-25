//------------------------------------------------------------------------------------------------------------------------------------------
// Various high level logic and utility functions for the new Vulkan world renderer.
// Note that this renderer re-uses some of the global state and functionality from the original 'r_' renderer modules.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_VULKAN_RENDERER

#include "rv_main.h"

#include "Asserts.h"
#include "Doom/Base/i_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_user.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/Renderer/r_sky.h"
#include "Doom/Renderer/r_things.h"
#include "Gpu.h"
#include "PcPsx/Config.h"
#include "PcPsx/Utils.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VRenderer.h"
#include "PcPsx/Vulkan/VTypes.h"
#include "rv_bsp.h"
#include "rv_flats.h"
#include "rv_occlusion.h"
#include "rv_sprites.h"
#include "rv_utils.h"
#include "rv_walls.h"

float       gViewXf, gViewYf, gViewZf;      // View position in floating point format
float       gViewAnglef;                    // View angle in radians (float)
float       gViewCosf, gViewSinf;           // Sin and cosine for view angle
uint16_t    gClutX, gClutY;                 // X and Y position in VRAM for the current CLUT (16-bit pixel coords)
Matrix4f    gSpriteBillboardMatrix;         // A transform matrix containing the axis vectors used for sprite billboarding
Matrix4f    gViewProjMatrix;                // The combined view and projection transform matrix for the scene

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the given subsector and all of it's contained objects
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSubsector(subsector_t& subsec) noexcept {
    // Get the light/color value for the sector
    uint8_t secR;
    uint8_t secG;
    uint8_t secB;
    RV_GetSectorColor(*subsec.sector, secR, secG, secB);
    
    // Draw all segs in the subsector
    const seg_t* const pBegSeg = gpSegs + subsec.firstseg;
    const seg_t* const pEndSeg = pBegSeg + subsec.numsegs;

    for (const seg_t* pSeg = pBegSeg; pSeg < pEndSeg; ++pSeg) {
        RV_DrawSeg(*pSeg, subsec, secR, secG, secB);
    }

    // Draw all flats in the subsector
    RV_DrawFlats(subsec, secR, secG, secB);

    // Draw sprites in the subsector:
    // TODO, do sprite and masked drawing AFTER all other solid geom.
    RV_DrawSubsectorSprites(subsec, secR, secG, secB);
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
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the player's view.
// Some of the high level logic here is copied from the original renderer's 'R_RenderPlayerView'.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_RenderPlayerView() noexcept {
    // Do nothing if drawing is currently not allowed
    if (!VRenderer::canSubmitDrawCmds())
        return;

    // Determine various draw settings and clear screen occlusion info to start with.
    // Then traverse the BSP tree to determine what needs to be drawn and in what order
    RV_DetermineDrawParams();
    RV_ClearOcclussion();
    RV_BuildDrawSubsecList();

    // Stat tracking: how many subsectors will we draw?
    gNumDrawSubsectors = (int32_t)(gppEndDrawSubsector - gpDrawSubsectors);

    // Draw the sky if showing
    if (gbIsSkyVisible) {
        // TODO: need a native Vulkan Sky Drawing routine, so we can do widescreen etc.
        Utils::onBeginUIDrawing();
        R_DrawSky();
    }

    // Set the projection matrix to use.
    // Make sure we have ended the draw batch first so the previous drawing is unaffected.
    // Also make sure we are on a compatible pipeline to accept the new transform matrix.
    VDrawing::endCurrentDrawBatch();
    VDrawing::setPipeline(VPipelineType::View_Alpha);
    VDrawing::setTransformMatrix(gViewProjMatrix);

    // Draw the sectors, back to front
    const int32_t numDrawSubsecs = (int32_t) gRVDrawSubsecs.size();

    for (int32_t i = numDrawSubsecs - 1; i >= 0; --i) {
        RV_DrawSubsector(*gRVDrawSubsecs[i]);
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
