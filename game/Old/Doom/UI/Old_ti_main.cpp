#include "Doom/UI/ti_main.h"

#if !PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the main title screen: the scrolling DOOM logo and fire sky
//------------------------------------------------------------------------------------------------------------------------------------------
void DRAW_Title() noexcept {
    I_IncDrawnFrameCount();

    // Draw the title logo.
    // Final Doom: this is drawn on top of everything instead.
    if (!Game::isFinalDoom()) {
        POLY_FT4 polyPrim;
        LIBGPU_SetPolyFT4(polyPrim);
        LIBGPU_setRGB0(polyPrim, 128, 128, 128);

        LIBGPU_setUV4(polyPrim,
            0,      0,
            255,    0,
            0,      239,
            255,    239
        );

        const int16_t titleY = (int16_t) gTitleScreenSpriteY;

        LIBGPU_setXY4(polyPrim,
            0,      titleY,
            255,    titleY,
            0,      titleY + 239,
            255,    titleY + 239
        );

        polyPrim.tpage = gTex_TITLE.texPageId;
        polyPrim.clut = gPaletteClutIds[TITLEPAL];

        I_AddPrim(&polyPrim);
    }

    // Upload the firesky texture if VRAM if required.
    // This will happen constantly for the duration of the title screen, as the fire is constantly changing.
    texture_t& skytex = *gpSkyTexture;

    if (skytex.uploadFrameNum == TEX_INVALID_UPLOAD_FRAME_NUM) {
        // Figure out where the texture is in VRAM coords and upload it
        const SRECT vramRect = getTextureVramRect(skytex);
        const std::byte* const pSkyTexData = (const std::byte*) gpLumpCache[skytex.lumpNum];
        LIBGPU_LoadImage(vramRect, (const uint16_t*)(pSkyTexData + sizeof(texlump_header_t)));

        // Mark this as uploaded now
        skytex.uploadFrameNum = gNumFramesDrawn;
    }

    // Setup a polygon primitive for drawing fire sky pieces
    POLY_FT4 polyPrim;

    LIBGPU_SetPolyFT4(polyPrim);
    LIBGPU_setRGB0(polyPrim, 128, 128, 128);

    const uint8_t texU = skytex.texPageCoordX;
    const uint8_t texV = skytex.texPageCoordY;
    constexpr uint8_t SKY_W = 63;

    LIBGPU_setUV4(polyPrim,
        texU,           texV,
        texU + SKY_W,   texV,
        texU,           texV + 127,
        texU + SKY_W,   texV + 127
    );

    if (Game::isFinalDoom()) {
        // Slightly tweaked positions for Final Doom
        LIBGPU_setXY4(polyPrim,
            0,      112,
            SKY_W,  112,
            0,      239,
            SKY_W,  239
        );
    } else {
        LIBGPU_setXY4(polyPrim,
            0,      116,
            SKY_W,  116,
            0,      243,
            SKY_W,  243
        );
    }

    polyPrim.tpage = skytex.texPageId;
    polyPrim.clut = gPaletteClutId_CurMapSky;

    // Draw the fire sky pieces
    for (int32_t i = 0; i < 4; ++i) {
        I_AddPrim(&polyPrim);

        polyPrim.x0 += SKY_W;
        polyPrim.x1 += SKY_W;
        polyPrim.x2 += SKY_W;
        polyPrim.x3 += SKY_W;
    }

    // Draw the title logo for Final Doom (on top of everything):
    if (Game::isFinalDoom()) {
        const uint8_t rgbMul = (uint8_t) gTitleScreenSpriteY;

        LIBGPU_SetPolyFT4(polyPrim);
        LIBGPU_setUV4(polyPrim,
            0,      0,
            255,    0,
            0,      239,
            255,    239
        );

        LIBGPU_setXY4(polyPrim,
            0,      0,
            255,    0,
            0,      239,
            255,    239
        );

        LIBGPU_setRGB0(polyPrim, rgbMul, rgbMul, rgbMul);
        polyPrim.tpage = gTex_TITLE.texPageId;
        polyPrim.clut = gPaletteClutIds[TITLEPAL];

        I_AddPrim(&polyPrim);
    }

    I_SubmitGpuCmds();
    I_DrawPresent();
}

#endif
