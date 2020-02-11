#include "ti_main.h"

#include "cn_main.h"
#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/Game/p_firesky.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_sky.h"
#include "m_main.h"
#include "o_main.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"

// Current position of the DOOM logo.
// Also adopted by the unused scrolling 'legals' screen for the same purpose.
const VmPtr<int32_t> gTitleScreenSpriteY(0x80078190);

// The DOOM logo texture
const VmPtr<texture_t> gTex_TITLE(0x80097A30);

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialization logic for the main title screen
//------------------------------------------------------------------------------------------------------------------------------------------
void START_Title() noexcept {
    // Cleanup the texture cache and remove anything we can
    I_PurgeTexCache();

    // Show the loading plaque
    W_CacheLumpName("LOADING", PU_STATIC, false);
    I_LoadAndCacheTexLump(*gTex_LOADING, "LOADING", 0);
    I_DrawLoadingPlaque(*gTex_LOADING, 95, 109, gPaletteClutIds[UIPAL]);
    
    // TODO
    a0 = 0;
    S_LoadSoundAndMusic();
    
    // Cache commonly used UI lumps for fast access and upload them to VRAM
    W_CacheLumpName("MARB01", PU_STATIC, false);
    W_CacheLumpName("BUTTONS", PU_STATIC, false);
    W_CacheLumpName("NETERR", PU_STATIC, false);
    W_CacheLumpName("PAUSE", PU_STATIC, false);

    I_LoadAndCacheTexLump(*gTex_MARB01, "MARB01", 0);
    I_LoadAndCacheTexLump(*gTex_BUTTONS, "BUTTONS", 0);
    I_LoadAndCacheTexLump(*gTex_NETERR, "NETERR", 0);
    I_LoadAndCacheTexLump(*gTex_PAUSE, "PAUSE", 0);
    I_LoadAndCacheTexLump(*gTex_TITLE, "TITLE", 0);
    
    // Cache the fire sky texture used in the title screen and save it's reference
    {
        const int32_t skyTexLumpNum = R_TextureNumForName("SKY09");
        texture_t& skyTex = (*gpTextures)[skyTexLumpNum];

        *gpSkyTexture = &skyTex;
        *gPaletteClutId_CurMapSky = gPaletteClutIds[FIRESKYPAL];

        W_CacheLumpNum(skyTex.lumpNum, PU_CACHE, true);
        I_CacheTex(skyTex);
    }
    
    // Initially the DOOM logo is offscreen
    *gTitleScreenSpriteY = SCREEN_H + 10;

    // Play the music for the title screen
    a0 = gCDTrackNum[cdmusic_title_screen];
    a1 = *gCdMusicVol;
    psxcd_play();
    
    // TODO: comment on elapsed sector stuff here
    do {
        psxcd_elapsed_sectors();
    } while (v0 == 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown/cleanup logic for the main title screen
//------------------------------------------------------------------------------------------------------------------------------------------
void STOP_Title() noexcept {
    // Play a barrel explode noise
    a0 = 0;
    a1 = sfx_barexp;
    S_StartSound();

    // Stop the current audio track
    psxcd_stop();
}

void TIC_Title() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F44);                               // Load from: gTicButtons[0] (80077F44)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_800352B0;
    v0 = 9;                                             // Result = 00000009
    goto loc_800353FC;
loc_800352B0:
    v0 = *gCurPlayerIndex;
    v1 = lw(gp + 0x6A4);                                // Load from: gVBlanksUntilTitleSprMove (80077C84)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    v1 -= v0;
    sw(v1, gp + 0x6A4);                                 // Store to: gVBlanksUntilTitleSprMove (80077C84)
    v0 = 2;                                             // Result = 00000002
    if (i32(v1) > 0) goto loc_80035310;
    v1 = *gTitleScreenSpriteY;
    sw(v0, gp + 0x6A4);                                 // Store to: gVBlanksUntilTitleSprMove (80077C84)
    v0 = v1 - 1;
    if (v1 == 0) goto loc_80035310;
    *gTitleScreenSpriteY = v0;
    if (v0 != 0) goto loc_80035310;
    v0 = *gTicCon;
    *gMenuTimeoutStartTicCon = v0;
loc_80035310:
    v0 = *gCurPlayerIndex;
    v1 = lw(gp + 0x6A8);                                // Load from: gVBlanksUntilTitleFireMove (80077C88)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    v1 -= v0;
    sw(v1, gp + 0x6A8);                                 // Store to: gVBlanksUntilTitleFireMove (80077C88)
    {
        const bool bJump = (i32(v1) > 0);
        v1 = 2;                                         // Result = 00000002
        if (bJump) goto loc_800353C8;
    }
    v0 = *gTitleScreenSpriteY;
    sw(v1, gp + 0x6A8);                                 // Store to: gVBlanksUntilTitleFireMove (80077C88)
    v1 = (i32(v0) < 0x32);
    v0 ^= 1;
    v0 &= 1;
    v1 &= v0;
    if (v1 == 0) goto loc_800353B8;
    v0 = *gpSkyTexture;
    v0 = lh(v0 + 0x10);
    v1 = *gpLumpCache;
    v0 <<= 2;
    v0 += v1;
    a1 = lw(v0);
    v0 = lbu(a1 + 0x1FC8);
    a0 = v0 - 1;
    v1 = 0x3F;                                          // Result = 0000003F
    if (i32(a0) >= 0) goto loc_800353A4;
    a0 = 0;                                             // Result = 00000000
loc_800353A4:
    v0 = a1 + 0x2007;
loc_800353A8:
    sb(a0, v0);
    v1--;
    v0--;
    if (i32(v1) >= 0) goto loc_800353A8;
loc_800353B8:
    P_UpdateFireSky(**gpSkyTexture);
loc_800353C8:
    v1 = *gTitleScreenSpriteY;
    v0 = 0;                                             // Result = 00000000
    if (v1 != 0) goto loc_800353FC;
    v0 = *gTicCon;
    v1 = *gMenuTimeoutStartTicCon;
    v0 -= v1;
    v0 = (i32(v0) < 0x708);
    v0 ^= 1;
    v0 = -v0;
    v0 &= 7;
loc_800353FC:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the main title screen: the scrolling DOOM logo and fire sky
//------------------------------------------------------------------------------------------------------------------------------------------
void DRAW_Title() noexcept {
    I_IncDrawnFrameCount();

    // Draw the title logo
    {
        POLY_FT4 polyPrim;
        LIBGPU_SetPolyFT4(polyPrim);
        LIBGPU_setRGB0(polyPrim, 128, 128, 128);

        LIBGPU_setUV4(polyPrim,
            0,      0,
            255,    0,
            0,      239,
            255,    239
        );

        const int16_t titleY = (int16_t) *gTitleScreenSpriteY;

        LIBGPU_setXY4(polyPrim,
            0,      titleY,
            255,    titleY,
            0,      239 + titleY,
            255,    239 + titleY
        );

        polyPrim.tpage = gTex_TITLE->texPageId;
        polyPrim.clut = gPaletteClutIds[TITLEPAL];

        I_AddPrim(&polyPrim);
    }

    // Upload the firesky texture if VRAM if required.
    // This will happen constantly for the duration of the title screen, as the fire is constantly changing.
    texture_t& skytex = **gpSkyTexture;

    if (skytex.uploadFrameNum == TEX_INVALID_UPLOAD_FRAME_NUM) {
        // Figure out where the texture is in VRAM coords and upload it
        const RECT vramRect = getTextureVramRect(skytex);
        const void* const pSkyTexData = (*gpLumpCache)[skytex.lumpNum].get();
        LIBGPU_LoadImage(vramRect, (const uint32_t*) pSkyTexData + 2);              // TODO: what 8 bytes is this skipping?

        // Mark this as uploaded now
        skytex.uploadFrameNum = *gNumFramesDrawn;
    }

    // Setup a polygon primitive for drawing fire sky pieces
    POLY_FT4 polyPrim;

    LIBGPU_SetPolyFT4(polyPrim);
    LIBGPU_setRGB0(polyPrim, 128, 128, 128);
    
    const uint8_t texU = skytex.texPageCoordX;
    const uint8_t texV = skytex.texPageCoordY;

    #if PC_PSX_DOOM_MODS
        constexpr uint8_t SKY_W = 64;   // PC-PSX: fix a 4 pixel gap at the right side of the screen with the fire
    #else
        constexpr uint8_t SKY_W = 63;
    #endif

    LIBGPU_setUV4(polyPrim,
        texU,           texV,
        texU + SKY_W,   texV,
        texU,           texV + 127,
        texU + SKY_W,   texV + 127
    );

    LIBGPU_setXY4(polyPrim,
        0,      116,
        SKY_W,  116,
        0,      243,
        SKY_W,  243
    );

    polyPrim.tpage = skytex.texPageId;
    polyPrim.clut =  *gPaletteClutId_CurMapSky;

    // Draw the fire sky pieces
    for (int32_t i = 0; i < 4; ++i) {
        I_AddPrim(&polyPrim);

        polyPrim.x0 += SKY_W;
        polyPrim.x1 += SKY_W;
        polyPrim.x2 += SKY_W;
        polyPrim.x3 += SKY_W;
    }

    I_SubmitGpuCmds();
    I_DrawPresent();
}
