//------------------------------------------------------------------------------------------------------------------------------------------
// Various miscellaneous utility functions for the new Vulkan world renderer
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_VULKAN_RENDERER

#include "rv_utils.h"

#include "Asserts.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_user.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Gpu.h"
#include "PsyQ/LIBGPU.h"
#include "rv_main.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

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
// Unpack a CLUT 'id' into an x, y coordinate for the CLUT
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_ClutIdToClutXy(const uint16_t clutId, uint16_t& clutX, uint16_t& clutY) noexcept {
    clutX = (clutId & 0x3Fu) << 4;      // Max coord: 1023, restricted to being on 16 pixel boundaries on the x-axis
    clutY = (clutId >> 6) & 0x3FFu;     // Max coord: 1023
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unpack various texturing parameters from the texture page id.
// Extracts the texture format, texture page position, and blending mode.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_TexPageIdToTexParams(
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
// Get the x/y and width/height of the given texture's texture window in VRAM coords (in terms of 16-bit pixels).
// Assumes the texture format is 8-bits per pixel, which will be the case for all textures used in the 3D world.
//
// Note: I had an assert which checked the 8bpp texture assumption but there are some cases with levels where invalid textures sometimes
// causes it the assertion to fail. Because of this I'm removing the assertion.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_GetTexWinXyWh(const texture_t& tex, uint16_t& texWinX, uint16_t& texWinY, uint16_t& texWinW, uint16_t& texWinH) noexcept {
    const uint16_t texPageId = tex.texPageId;
    const uint16_t texPageX = ((texPageId & 0x000Fu) >> 0) * 64u;
    const uint16_t texPageY = ((texPageId & 0x0010u) >> 4) * 256u;

    texWinX = texPageX + tex.texPageCoordX / 2;     // Divide by 2 because the format is 8bpp and VRAM coords are 16bpp
    texWinY = texPageY + tex.texPageCoordY;
    texWinW = tex.width / 2;                        // Divide by 2 because the format is 8bpp and VRAM coords are 16bpp
    texWinH = tex.height;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Uploads the given texture to VRAM if it needs to be uploaded, otherwise does nothing.
// Assumes the texture has already been added to the texture cache and has an area of VRAM assigned to itself.
// Used for updating animated floor, wall and sky textures.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_UploadDirtyTex(texture_t& tex) noexcept {
    // Is the texture already uploaded?
    if (tex.uploadFrameNum != TEX_INVALID_UPLOAD_FRAME_NUM)
        return;

    // Decompress the lump data to the temporary buffer if required
    const std::byte* pLumpData;
    const bool bIsUncompressedLump = gpbIsUncompressedLump[tex.lumpNum];

    if (bIsUncompressedLump) {
        pLumpData = (const std::byte*) gpLumpCache[tex.lumpNum];
    } else {
        const void* pCompressedLumpData = gpLumpCache[tex.lumpNum];
        ASSERT(getDecodedSize(pCompressedLumpData) <= TMP_BUFFER_SIZE);
        decode(pCompressedLumpData, gTmpBuffer);
        pLumpData = gTmpBuffer;
    }

    // Load the decompressed texture to the required part of VRAM and mark as loaded
    const RECT vramRect = getTextureVramRect(tex);
    LIBGPU_LoadImage(vramRect, (uint16_t*)(pLumpData + sizeof(texlump_header_t)));
    tex.uploadFrameNum = gNumFramesDrawn;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the left x and right x coordinates of the given line in normalized device coords.
// Returns 'false' if the line was offscreen, in which case the bounds are UNDEFINED and not set.
//
// Notes:
//  (1) I treat Doom's 'y' coordinate as 'z' for all calculations. In most literature and code on the subject 'z' is actually depth so
//      interpreting 'y' as this keeps things more familiar and allows me to just plug the values into existing equations.
//  (2) For 'xc', 'yc', and 'zc' clipspace coordinates, the point will be inside the clip cube if the
//      following comparisons against clipspace w ('wc') hold:
//          -wc <= xc && xc <= wc
//          -wc <= yc && yc <= wc
//          -wc <= zc && zc <= wc
//  (3) I don't bother clipping the line against any of the planes, it's not needed for what this function is used for.
//      The returned coords may be outside the usual -1 to +1 range.
//------------------------------------------------------------------------------------------------------------------------------------------
bool RV_GetLineNdcBounds(
    const float p1x,
    const float p1y,
    const float p2x,
    const float p2y,
    float& lx,
    float& rx
) noexcept {
    // Convert the points to clip space by assuming they are the homogenous vectors (x, 0, y, 1) and
    // multiplying by the view projection transform matrix. Save only the xzw components.
    const float mR0C0 = gViewProjMatrix.e[0][0];
    const float mR0C2 = gViewProjMatrix.e[0][2];
    const float mR0C3 = gViewProjMatrix.e[0][3];
    const float mR2C0 = gViewProjMatrix.e[2][0];
    const float mR2C2 = gViewProjMatrix.e[2][2];
    const float mR2C3 = gViewProjMatrix.e[2][3];
    const float mR3C0 = gViewProjMatrix.e[3][0];
    const float mR3C2 = gViewProjMatrix.e[3][2];
    const float mR3C3 = gViewProjMatrix.e[3][3];

    const float p1_clip[3] = {
        p1x * mR0C0 + p1y * mR2C0 + mR3C0,  // x
        p1x * mR0C2 + p1y * mR2C2 + mR3C2,  // z (Doom y)
        p1x * mR0C3 + p1y * mR2C3 + mR3C3   // w
    };

    const float p2_clip[3] = {
        p2x * mR0C0 + p2y * mR2C0 + mR3C0,  // x
        p2x * mR0C2 + p2y * mR2C2 + mR3C2,  // z (Doom y)
        p2x * mR0C3 + p2y * mR2C3 + mR3C3   // w
    };

    // Throw out the line if it's on the wrong side of the left, right or front viewing planes
    const float wc1 = p1_clip[2];
    const float wc2 = p2_clip[2];

    const bool bCullLine = (
        ((p1_clip[0] < -wc1) && (p2_clip[0] < -wc2)) ||     // Line outside the left view frustrum plane?
        ((p1_clip[0] > +wc1) && (p2_clip[0] > +wc2)) ||     // Line outside the right view frustrum plane?
        ((p1_clip[1] < -wc1) && (p2_clip[1] < -wc2))        // Line outside the front view frustrum plane?
    );

    if (bCullLine)
        return false;

    // Compute the normalized device coords for the two line endpoints and save
    float x1 = p1_clip[0] / wc1;
    float x2 = p2_clip[0] / wc2;

    if (x1 > x2) {
        std::swap(x1, x2);
    }

    lx = x1;
    rx = x2;
    return true;
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
