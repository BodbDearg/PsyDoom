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

// TODO: remove LIBGTE use here eventually
#include "PsyQ/LIBGTE.h"

#include <cmath>

// The current view projection matrix for the scene
static Matrix4f gViewProjMatrix;

float   gViewXf;
float   gViewYf;
float   gViewZf;
float   gViewAnglef;
float   gViewCosf;
float   gViewSinf;

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpack a CLUT 'id' into an x, y coordinate for the CLUT
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_ClutIdToClutXy(const uint16_t clutId, uint16_t& clutX, uint16_t& clutY) noexcept {
    clutX = (clutId & 0x3Fu) << 4;      // Max coord: 1023, restricted to being on 16 pixel boundaries on the x-axis
    clutY = (clutId >> 6) & 0x3FFu;     // Max coord: 1023
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpack various texturing parameters from the texture page id.
// Extracts the texture format, texture page position, and blending mode.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_TexPageIdToTexParams(
    const uint16_t texPageId,
    Gpu::TexFmt& texFmt,
    uint16_t& texPageX,
    uint16_t& texPageY,
    Gpu::BlendMode& blendMode
) noexcept {
    texFmt = (Gpu::TexFmt)((texPageId >> 7) & 0x0003u);
    texPageX = ((texPageId & 0x000Fu) >> 0) * 64u;
    texPageY = ((texPageId & 0x0010u) >> 4) * 256u;
    blendMode = (Gpu::BlendMode)((texPageId >> 5) & 0x0003u);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a 16.16 fixed point number to float
//------------------------------------------------------------------------------------------------------------------------------------------
float RV_FixedToFloat(const fixed_t num) noexcept {
    return (float)(num * (1.0f / 65536.0f));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a floating point number to 16.16 fixed
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t RV_FloatToFixed(const float num) noexcept {
    return (fixed_t)(num * 65536.0f);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert an angle to a float (radians)
//------------------------------------------------------------------------------------------------------------------------------------------
float RV_AngleToFloat(const angle_t angle) noexcept {
    const double normalized = (double) angle / (double(UINT32_MAX) + 1.0);
    const double twoPiRange = normalized * RV_2PI<double>;
    return (float) twoPiRange;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a radians angle to Doom's 32-bit binary angle format
//------------------------------------------------------------------------------------------------------------------------------------------
angle_t RV_FloatToAngle(const float angle) noexcept {
    const double twoPiRange = std::fmod((double) angle, RV_2PI<double>);
    const double normalized = twoPiRange / RV_2PI<double>;
    const double intRange = normalized * (double(UINT32_MAX) + 1.0);
    return (angle_t) intRange;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the RGB color value to apply to shade the sector.
// A value of '128' for a component means full brightness, and values over that are over-bright.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_GetSectorColor(const sector_t& sec, uint8_t& r, uint8_t& g, uint8_t& b) noexcept {
    if (gbDoViewLighting) {
        // Compute basic light values
        const light_t& light = gpLightsLump[sec.colorid];
        uint32_t r32 = ((uint32_t) sec.lightlevel * (uint32_t) light.r) >> 8;
        uint32_t g32 = ((uint32_t) sec.lightlevel * (uint32_t) light.g) >> 8;
        uint32_t b32 = ((uint32_t) sec.lightlevel * (uint32_t) light.b) >> 8;

        // Contribute the player muzzle flash to the light
        player_t& player = gPlayers[gCurPlayerIndex];

        if (player.extralight != 0) {
            r32 += player.extralight;
            g32 += player.extralight;
            b32 += player.extralight;
        }

        // Return the saturated light value
        r = (uint8_t) std::min(r32, 255u);
        g = (uint8_t) std::min(g32, 255u);
        b = (uint8_t) std::min(b32, 255u);
    } else {
        // No lighting - render full bright!
        r = 128;
        g = 128;
        b = 128;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// TODO: temp code to draw a subsector - this needs a lot more work.
// TODO: move this elsewhere.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_DrawSubsector(subsector_t& subsec) noexcept {
    // Get the light/color value for the sector
    uint8_t secR;
    uint8_t secG;
    uint8_t secB;
    RV_GetSectorColor(*subsec.sector, secR, secG, secB);

    // Get the current CLUT id and get it's VRAM position
    const uint16_t clutId = gPaletteClutIds[1];
    uint16_t clutX = {};
    uint16_t clutY = {};
    RV_ClutIdToClutXy(clutId, clutX, clutY);

    // Draw all segs in the subsector
    for (int32_t segIdx = 0; segIdx < subsec.numsegs; ++segIdx) {
        const seg_t& seg = gpSegs[subsec.firstseg + segIdx];

        // TODO: handle two sided linedefs
        if (seg.backsector)
            continue;

        // TODO: a lot of this should be precomputed
        const float x1 = RV_FixedToFloat(seg.vertex1->x);
        const float z1 = RV_FixedToFloat(seg.vertex1->y);
        const float x2 = RV_FixedToFloat(seg.vertex2->x);
        const float z2 = RV_FixedToFloat(seg.vertex2->y);
        const float fyb = RV_FixedToFloat(seg.frontsector->floorheight);
        const float fyt = RV_FixedToFloat(seg.frontsector->ceilingheight);

        // Get the offset and base uv for the current texture
        // TODO: handle texture alignment flags
        const texture_t& tex = gpTextures[seg.sidedef->midtexture];
        const float uOffset = RV_FixedToFloat(seg.sidedef->textureoffset);
        const float vOffset = RV_FixedToFloat(seg.sidedef->rowoffset);

        // Get the texture page details for the current texture
        Gpu::TexFmt texFmt;
        uint16_t texPageX;
        uint16_t texPageY;
        Gpu::BlendMode blendMode;
        RV_TexPageIdToTexParams(tex.texPageId, texFmt, texPageX, texPageY, blendMode);
        
        ASSERT_LOG(texFmt == Gpu::TexFmt::Bpp8, "Only 8-bit textures are supported currently for the 3d view!");

        // TODO: a lot of this should be precomputed
        const float dx = x2 - x1;
        const float dy = fyt - fyb;
        const float dz = z2 - z1;
        const float lineDist = std::sqrtf(dx * dx + dz * dz);

        const float ul = uOffset;
        const float ur = uOffset + lineDist;    // TODO: this doesn't work correctly for BSP split linedefs
        const float vt = vOffset - dy;
        const float vb = vOffset;

        VDrawing::add3dViewTriangle(
            x1, fyb, z1, ul, vb,
            x1, fyt, z1, ul, vt,
            x2, fyt, z2, ur, vt,
            secR, secG, secB, 128,
            clutX, clutY,
            texPageX + tex.texPageCoordX / 2, texPageY + tex.texPageCoordY, // TODO: shift divide to VDrawing
            tex.width, tex.height,
            false   // TODO: do blending if required
        );

        VDrawing::add3dViewTriangle(
            x2, fyt, z2, ur, vt,
            x2, fyb, z2, ur, vb,
            x1, fyb, z1, ul, vb,
            secR, secG, secB, 128,
            clutX, clutY,
            texPageX + tex.texPageCoordX / 2, texPageY + tex.texPageCoordY, // TODO: shift divide to VDrawing
            tex.width, tex.height,
            false   // TODO: do blending if required
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the player's view.
// Some of the high level logic here is copied from the original renderer's 'R_RenderPlayerView'.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_RenderPlayerView() noexcept {
    // Store view parameters before drawing
    player_t& player = gPlayers[gCurPlayerIndex];
    gpViewPlayer = &player;
    
    // PsyDoom: use interpolation to update the actual view if doing an uncapped framerate.
    // Determine the view x/y/z position and the angle in fixed point terms first.
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

    // Compute the view projection matrix to use and set for upcoming drawing
    gViewProjMatrix = VDrawing::computeTransformMatrixFor3D(gViewXf, gViewZf, gViewYf, gViewAnglef);
    VDrawing::setTransformMatrix(gViewProjMatrix);

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
    
    // Draw all subsectors emitted during BSP traversal.
    // Draw them in back to front order.
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
