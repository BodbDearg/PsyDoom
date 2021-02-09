//------------------------------------------------------------------------------------------------------------------------------------------
// A module that handles submitting primitives to draw the sky
//------------------------------------------------------------------------------------------------------------------------------------------
#include "rv_sky.h"

#if PSYDOOM_VULKAN_RENDERER

#include "Asserts.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_sky.h"
#include "Gpu.h"
#include "PcPsx/Vulkan/VDrawing.h"
#include "PsyQ/LIBGPU.h"
#include "rv_main.h"
#include "rv_utils.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the texture parameters for the sky texture (texture window and CLUT position).
// Note that 8-bit color is always assumed.
//------------------------------------------------------------------------------------------------------------------------------------------
static void RV_GetSkyTexParams(
    uint16_t& texWinX,
    uint16_t& texWinY,
    uint16_t& texWinW,
    uint16_t& texWinH,
    uint16_t& clutX,
    uint16_t& clutY
) noexcept {
    texture_t& skytex = *gpSkyTexture;

    // Get the texture window location
    Gpu::TexFmt texFmt = {};
    Gpu::BlendMode blendMode = {};
    RV_TexPageIdToTexParams(skytex.texPageId, texFmt, texWinX, texWinY, blendMode);
    ASSERT(texFmt == Gpu::TexFmt::Bpp8);

    // Set the texture window size - note that x is halved because the format is 8bpp and not 16bpp (VRAM coords are 16bpp)
    texWinW = skytex.width / 2;
    texWinH = skytex.height;

    // Get the CLUT location
    RV_ClutIdToClutXy(gPaletteClutId_CurMapSky, clutX, clutY);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Computes the current 'U' texture coordinate offset for the sky based on player rotation
//------------------------------------------------------------------------------------------------------------------------------------------
static float RV_GetSkyUCoordOffset() noexcept {
    // One full revolution is equal to 1024 texel units.
    // When the sky texture is 256 pixels wide, this means 4 wrappings.
    const float rotatePercent = -gViewAnglef * (1.0f / RV_2PI<float>);
    return rotatePercent * 1024;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Uploads the latest frame of the sky texture to VRAM if required; should be called at least once a frame.
// Won't do any work for normal skies since they are always precached after level start, but should do work periodically for the fire sky.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_CacheSkyTex() noexcept {
    // Texture already up to date in VRAM?
    texture_t& skytex = *gpSkyTexture;

    if (skytex.uploadFrameNum != TEX_INVALID_UPLOAD_FRAME_NUM)
        return;

    // Need to upload the texture to VRAM, do that now:
    const std::byte* const pLumpData = (const std::byte*) gpLumpCache[skytex.lumpNum];
    const uint16_t* const pTexData = (const std::uint16_t*)(pLumpData + sizeof(texlump_header_t));
    RECT vramRect = getTextureVramRect(skytex);

    LIBGPU_LoadImage(vramRect, pTexData);
    skytex.uploadFrameNum = gNumFramesDrawn;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a quad for the sky at the specified world position.
// Note: assumes the correct draw pipeline has been set beforehand and that the sky texture is cached.
//------------------------------------------------------------------------------------------------------------------------------------------
void RV_AddSkyQuad(
    const float x1,
    const float y1,
    const float z1,
    const float x2,
    const float y2,
    const float z2,
    const float x3,
    const float y3,
    const float z3,
    const float x4,
    const float y4,
    const float z4
) noexcept {
    // Get the basic texture params for the sky
    uint16_t texWinX, texWinY;
    uint16_t texWinW, texWinH;
    uint16_t clutX, clutY;
    RV_GetSkyTexParams(texWinX, texWinY, texWinW, texWinH, clutX, clutY);

    // Get the sky 'U' texture coordinate and add the sky triangle
    const float uOffset = RV_GetSkyUCoordOffset();
    VDrawing::addDrawWorldSkyQuad(x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, uOffset, clutX, clutY, texWinX, texWinY, texWinW, texWinH);
}

#endif  // #if PSYDOOM_VULKAN_RENDERER
