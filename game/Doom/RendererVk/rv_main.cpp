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
#include "Doom/Renderer/r_bsp.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/Renderer/r_sky.h"
#include "Doom/Renderer/r_things.h"
#include "Gpu.h"
#include "PcPsx/Config.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PcPsx/Vulkan/VTypes.h"
#include "rv_utils.h"
#include "rv_walls.h"

// TODO: remove LIBGTE use here eventually
#include "PsyQ/LIBGTE.h"

// The current view projection matrix for the scene
static Matrix4f gViewProjMatrix;

float       gViewXf;
float       gViewYf;
float       gViewZf;
float       gViewAnglef;
float       gViewCosf;
float       gViewSinf;
uint16_t    gClutX;
uint16_t    gClutY;

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
        RV_DrawSeg(*pSeg, secR, secG, secB);
    }
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
    
    // Set the draw matrix and upload to the GTE.
    // TODO: remove LIBGTE use here eventually
    gDrawMatrix.m[0][0] = (int16_t) d_rshift<GTE_ROTFRAC_SHIFT>( gViewSin);
    gDrawMatrix.m[0][2] = (int16_t) d_rshift<GTE_ROTFRAC_SHIFT>(-gViewCos);
    gDrawMatrix.m[2][0] = (int16_t) d_rshift<GTE_ROTFRAC_SHIFT>( gViewCos);
    gDrawMatrix.m[2][2] = (int16_t) d_rshift<GTE_ROTFRAC_SHIFT>( gViewSin);
    LIBGTE_SetRotMatrix(gDrawMatrix);

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
    // Determine various draw settings
    RV_DetermineDrawParams();

    // Traverse the BSP tree to determine what needs to be drawn and in what order.
    // TODO: remove old BSP here eventually, won't work with expanded frustrum.
    R_BSP();
    
    // Stat tracking: how many subsectors will we draw?
    gNumDrawSubsectors = (int32_t)(gppEndDrawSubsector - gpDrawSubsectors);

    if (gbIsSkyVisible) {
        R_DrawSky();
    }

    // TODO: HACK workaround until we get a dedicated 3D shader. Need to end the batch.
    VDrawing::setPipeline(VPipelineType::Lines);
    VDrawing::setPipeline(VPipelineType::UI_8bpp);
    
    // Set the projection matrix to use and then draw all subsectors emitted during BSP traversal in back to front order
    VDrawing::setTransformMatrix(gViewProjMatrix);

    while (gppEndDrawSubsector > gpDrawSubsectors) {
        --gppEndDrawSubsector;
        subsector_t& subsec = **gppEndDrawSubsector;
        RV_DrawSubsector(subsec);
    }

    // TODO: HACK workaround until we get a dedicated 3D shader. Need to end the batch.
    VDrawing::setPipeline(VPipelineType::Lines);
    VDrawing::setPipeline(VPipelineType::UI_8bpp);

    // Draw any player sprites/weapons
    VDrawing::setTransformMatrix(VDrawing::computeTransformMatrixForUI());
    R_DrawWeapon();
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
