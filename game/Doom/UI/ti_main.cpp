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
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_sky.h"
#include "m_main.h"
#include "o_main.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"

// Current position of the DOOM logo.
// Also adopted by the 'legals' screen for the same purpose.
const VmPtr<int32_t> gTitleScreenSpriteY(0x80078190);

// The DOOM logo texture
static const VmPtr<texture_t> gTex_TITLE(0x80097A30);

// Controls how frequently the title sprite and fire sky update and move
static const VmPtr<int32_t> gVBlanksUntilTitleSprMove(0x80077C84);
static const VmPtr<int32_t> gVBlanksUntilTitleFireMove(0x80077C88);

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
void STOP_Title([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // Play a barrel explode noise
    a0 = 0;
    a1 = sfx_barexp;
    S_StartSound();

    // Stop the current audio track
    psxcd_stop();
}

void _thunk_STOP_Title() noexcept {
    STOP_Title((gameaction_t) a0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the main title screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t TIC_Title() noexcept {
    // End the title screen if any buttons are pressed
    if (gTicButtons[0] != 0)
        return ga_exit;
    
    // Decrement the time until the title sprite moves
    const int32_t elapsedVBlanks = gPlayersElapsedVBlanks[*gCurPlayerIndex];
    *gVBlanksUntilTitleSprMove -= elapsedVBlanks;
    
    // If it is time to move the title sprite then do that.
    // Stop the title sprite also once it reaches the top of the screen and begin the menu timeout counter.
    if (*gVBlanksUntilTitleSprMove <= 0) {
        *gVBlanksUntilTitleSprMove = 2;

        if (*gTitleScreenSpriteY != 0) {
            *gTitleScreenSpriteY -= 1;

            if (*gTitleScreenSpriteY == 0) {
                *gMenuTimeoutStartTicCon = *gTicCon;    // Start the timeout process...
            }
        }
    }

    // Update the fire sky if it is time to do that
    *gVBlanksUntilTitleFireMove -= elapsedVBlanks;
    
    if (*gVBlanksUntilTitleFireMove <= 0) {        
        *gVBlanksUntilTitleFireMove = 2;

        // Once the title screen sprite is above a certain height, begin to fizzle out the fire.
        // Do this for every even y position that the title screen sprite is at.
        // Eventually it will settle at position '0' so we will do this all the time then and completely put out the fire.
        if ((*gTitleScreenSpriteY < 50) && ((*gTitleScreenSpriteY & 1) == 0)) {
            // Get the pixels for the last row in the fire sky
            texture_t& skyTex = **gpSkyTexture;
            uint8_t* const pLumpData = (uint8_t*)(*gpLumpCache)[skyTex.lumpNum].get();
            uint8_t* const pFSkyRows = pLumpData + 8;   // TODO: comment what is being skipped
            uint8_t* const pFSkyLastRow = pFSkyRows + FIRESKY_W * (FIRESKY_H - 1);

            // Sample the first pixel of the last row (the fire generator row) and decrease its 'temperature' by 1.
            // We will apply this temperature to the entire last row of the fire sky.
            uint8_t newFireTemp = pFSkyLastRow[0];

            if (newFireTemp > 0) {
                --newFireTemp;
            }

            // Apply the new temperature to the generator row of the fire to decrease it's intensity
            for (int32_t x = 0; x < FIRESKY_W; ++x) {
                pFSkyLastRow[x] = newFireTemp;
            }
        }

        // Run the fire sky simulation
        P_UpdateFireSky(**gpSkyTexture);
    }
    
    // If the title sprite has not reached the top of the screen then we don't ever timeout
    if (*gTitleScreenSpriteY != 0)
        return ga_nothing;
    
    // Once the title sprite reaches the top of the screen, timeout after a while...
    const bool bHasTimedOut = (*gTicCon - *gMenuTimeoutStartTicCon >= 1800);
    return (bHasTimedOut) ? ga_timeout : ga_nothing;
}

void _thunk_TIC_Title() noexcept {
    v0 = TIC_Title();
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
