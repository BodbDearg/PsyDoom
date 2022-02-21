#include "ti_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_texcache.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/Game/doomdata.h"
#include "Doom/Game/p_firesky.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_sky.h"
#include "m_main.h"
#include "o_main.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/MapInfo.h"
#include "PsyDoom/Utils.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"

// Doom: current position of the DOOM logo; also adopted by the 'legals' screen for the same purpose.
// Final Doom: this is repurposed as the current fade/color-multiplier for the title screen image.
int32_t gTitleScreenSpriteY;

// The DOOM logo texture for 'Doom' and 'Final Doom' style title screens
texture_t gTex_TITLE;

// The 'Final DOOM' logo for the 'GEC Master Edition' style title screen
#if PSYDOOM_MODS
    static texture_t gTex_FINAL;
#endif

// Controls how frequently the title sprite and fire sky update and move
static int32_t gVBlanksUntilTitleSprMove;
static int32_t gVBlanksUntilTitleFireMove;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialization logic for the main title screen
//------------------------------------------------------------------------------------------------------------------------------------------
void START_Title() noexcept {
    // Cleanup the texture cache and remove anything we can
    I_PurgeTexCache();

    // Show the loading plaque
    W_CacheLumpName("LOADING", PU_STATIC, false);
    I_LoadAndCacheTexLump(gTex_LOADING, "LOADING", 0);
    I_DrawLoadingPlaque(gTex_LOADING, 95, 109, Game::getTexPalette_LOADING());

    // Load sounds for the menu
    S_LoadMapSoundAndMusic(0);

    // Cache commonly used UI lumps for fast access and upload them to VRAM
    W_CacheLumpName(Game::getTexLumpName_OptionsBg(), PU_STATIC, false);
    W_CacheLumpName("NETERR", PU_STATIC, false);
    W_CacheLumpName("PAUSE", PU_STATIC, false);

    I_LoadAndCacheTexLump(gTex_OptionsBg, Game::getTexLumpName_OptionsBg().c_str().data(), 0);
    I_LoadAndCacheTexLump(gTex_NETERR, "NETERR", 0);
    I_LoadAndCacheTexLump(gTex_PAUSE, "PAUSE", 0);

    // PsyDoom: if we are doing the 'GEC Master Edition' style title screen then different lumps are used
    const TitleScreenStyle screenStyle = MapInfo::getGameInfo().titleScreenStyle;

    #if PSYDOOM_MODS
        if (screenStyle == TitleScreenStyle::GEC_ME) {
            I_LoadAndCacheTexLump(gTex_DOOM, "DOOM", 0);
            I_LoadAndCacheTexLump(gTex_FINAL, "FINAL", 0);
            I_LoadAndCacheTexLump(gTex_DATA, "DATA", 0);
        } else {
            I_LoadAndCacheTexLump(gTex_TITLE, "TITLE", 0);
        }
    #else
        I_LoadAndCacheTexLump(gTex_TITLE, "TITLE", 0);
    #endif

    // PsyDoom: no longer required to be cached since we don't have the controls screen
    #if !PSYDOOM_MODS
        W_CacheLumpName("BUTTONS", PU_STATIC, false);
        I_LoadAndCacheTexLump(gTex_BUTTONS, "BUTTONS", 0);
    #endif

    // Cache the fire sky texture used in the title screen and save it's reference
    {
        // PsyDoom: this texture MUST exist, otherwise issue a fatal error
        #if PSYDOOM_MODS
            const int32_t skyTexIdx = R_TextureNumForName("SKY09", true);
        #else
            const int32_t skyTexIdx = R_TextureNumForName("SKY09");
        #endif

        texture_t& skyTex = gpTextures[skyTexIdx];
        gpSkyTexture = &skyTex;
        gPaletteClutId_CurMapSky = gPaletteClutIds[FIRESKYPAL];

        W_CacheLumpNum(skyTex.lumpNum, PU_CACHE, true);
        I_CacheTex(skyTex);
    }

    // Doom: initially the DOOM logo is offscreen.
    // Final Doom & others: it doesn't move and covers the fire.
    if (screenStyle == TitleScreenStyle::Doom) {
        gTitleScreenSpriteY = SCREEN_H + 10;
    } else {
        gTitleScreenSpriteY = 0;
    }

    // Play the music for the title screen
    psxcd_play(gCDTrackNum[cdmusic_title_screen], gCdMusicVol);

    // Wait until some cd audio has been read
    Utils::waitForCdAudioPlaybackStart();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown/cleanup logic for the main title screen
//------------------------------------------------------------------------------------------------------------------------------------------
void STOP_Title([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // Play a barrel explode noise and fade out the current cd audio track.
    // PsyDoom: only do this if not quitting the app, otherwise exit immediately.
    #if PSYDOOM_MODS
        if (Input::isQuitRequested())
            return;
    #endif

    S_StartSound(nullptr, sfx_barexp);

    #if PSYDOOM_MODS
        // PsyDoom: update sounds so that the barrel explosion plays while the music fades out.
        // Need to do this now since sounds don't play immediately and are queued.
        S_UpdateSounds();
    #endif

    psxcd_stop();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the main title screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t TIC_Title() noexcept {
    // End the title screen if any buttons are pressed.
    // PsyDoom: just accept menu start/ok/back but require them to be just released:
    #if PSYDOOM_MODS
        const bool bExitJustReleased = (
            ((!gTickInputs[0].fMenuOk())    && gOldTickInputs[0].fMenuOk()) ||
            ((!gTickInputs[0].fMenuStart()) && gOldTickInputs[0].fMenuStart()) ||
            ((!gTickInputs[0].fMenuBack())  && gOldTickInputs[0].fMenuBack())
        );

        if (bExitJustReleased)
            return ga_exit;
    #else
        if (gTicButtons[0] != 0)
            return ga_exit;
    #endif

    // Decrement the time until the title sprite moves
    const int32_t elapsedVBlanks = gPlayersElapsedVBlanks[gCurPlayerIndex];
    gVBlanksUntilTitleSprMove -= elapsedVBlanks;

    // For 'Final Doom' and 'GEC Master Edition' the scroll value is actually an RGB color multiply for the title image.
    // The logic to update it is different in that case.
    if (MapInfo::getGameInfo().titleScreenStyle == TitleScreenStyle::Doom) {
        // Regular Doom: if it is time to move the title sprite then do that.
        // Stop the title sprite also once it reaches the top of the screen and begin the menu timeout counter.
        if (gVBlanksUntilTitleSprMove <= 0) {
            gVBlanksUntilTitleSprMove = 2;

            if (gTitleScreenSpriteY != 0) {
                gTitleScreenSpriteY -= 1;

                if (gTitleScreenSpriteY == 0) {
                    gMenuTimeoutStartTicCon = gTicCon;      // Start the timeout process...
                }
            }
        }

        // Update the fire sky if it is time to do that
        gVBlanksUntilTitleFireMove -= elapsedVBlanks;

        if (gVBlanksUntilTitleFireMove <= 0) {
            gVBlanksUntilTitleFireMove = 2;

            // Once the title screen sprite is above a certain height, begin to fizzle out the fire.
            // Do this for every even y position that the title screen sprite is at.
            // Eventually it will settle at position '0' so we will do this all the time then and completely put out the fire.
            if ((gTitleScreenSpriteY < 50) && ((gTitleScreenSpriteY & 1) == 0)) {
                // Get the pixels for the last row in the fire sky.
                // PsyDoom: added updates here to work with the new WAD management code.
                texture_t& skyTex = *gpSkyTexture;

                #if PSYDOOM_MODS
                    const WadLump& fireSkyLump = W_GetLump(skyTex.lumpNum);
                    uint8_t* const pLumpData = (uint8_t*) fireSkyLump.pCachedData;
                #else
                    uint8_t* const pLumpData = (uint8_t*) gpLumpCache[skyTex.lumpNum];
                #endif

                uint8_t* const pFSkyRows = pLumpData + sizeof(texlump_header_t);
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
            P_UpdateFireSky(*gpSkyTexture);
        }

        // Once the title sprite reaches the top of the screen, timeout after a while...
        if (gTitleScreenSpriteY == 0) {
            const bool bHasTimedOut = (gTicCon - gMenuTimeoutStartTicCon >= 1800);
            return (bHasTimedOut) ? ga_timeout : ga_nothing;
        }
    }
    else {
        // 'Final Doom' and 'GEC Master Edition' style title screen
        #if PSYDOOM_MODS
            // PsyDoom: don't run this too fast!
            if (gPlayersElapsedVBlanks[gCurPlayerIndex] <= 0)
                return ga_nothing;
        #endif

        // Update the fire sky after a certain number of ticks have elapsed
        if ((gVBlanksUntilTitleSprMove <= 0) && (gTitleScreenSpriteY >= 128)) {
            gVBlanksUntilTitleSprMove = 2;
            P_UpdateFireSky(*gpSkyTexture);
        }

        // Do the fade in of the title screen
        if (gTitleScreenSpriteY <= 128) {
            gTitleScreenSpriteY += 4;
        }

        // Once the title is full faded in, timeout after a while...
        if (gTitleScreenSpriteY >= 128) {
            const bool bHasTimedOut = (gTicCon - gMenuTimeoutStartTicCon >= 1800);
            return (bHasTimedOut) ? ga_timeout : ga_nothing;
        }
    }

    // Continue running the screen
    return ga_nothing;
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the main title screen: the scrolling DOOM logo and fire sky.
//
// PsyDoom: this funtion has been rewritten to eliminate issues with gaps at the edge of the screen caused by using polygons rather than
// sprites to render. According to the NO$PSX docs "Polygons are displayed up to <excluding> their lower-right coordinates.".
// I also fixed issues with gaps at the side of the screen due to bad spacing of the firesky tiles.
//------------------------------------------------------------------------------------------------------------------------------------------
void DRAW_Title() noexcept {
    I_IncDrawnFrameCount();
    
    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    // Draw the title logo for 'Doom'.
    // Note that for 'Final Doom' this is drawn on top of everything (at the end) instead.
    const TitleScreenStyle screenStyle = MapInfo::getGameInfo().titleScreenStyle;

    if (screenStyle == TitleScreenStyle::Doom) {
        const int16_t titleY = (int16_t) gTitleScreenSpriteY;

        // PsyDoom: the TITLE logo might not be at UV 0,0 anymore! (if limit removing, but always offset to be safe)
        #if PSYDOOM_MODS
            I_DrawSprite(
                gTex_TITLE.texPageId,
                Game::getTexPalette_TITLE(),
                0,
                titleY,
                gTex_TITLE.texPageCoordX,
                gTex_TITLE.texPageCoordY,
                SCREEN_W,
                SCREEN_H
            );
        #else
            I_DrawSprite(gTex_TITLE.texPageId, gPaletteClutIds[TITLEPAL], 0, titleY, 0, 0, SCREEN_W, SCREEN_H);
        #endif
    }

    // Upload the firesky texture if VRAM if required.
    // This will happen constantly for the duration of the title screen, as the fire is constantly changing.
    texture_t& skyTex = *gpSkyTexture;

    if (skyTex.uploadFrameNum == TEX_INVALID_UPLOAD_FRAME_NUM) {
        // Figure out where the texture is in VRAM coords and upload it.
        // PsyDoom: added updates here to work with the new WAD management code and to ensure texture metrics are up-to-date.
        const SRECT vramRect = getTextureVramRect(skyTex);

        #if PSYDOOM_MODS
            const WadLump& fireSkyLump = W_GetLump(skyTex.lumpNum);
            const std::byte* const pSkyTexData = (const std::byte*) fireSkyLump.pCachedData;
            R_UpdateTexMetricsFromData(skyTex, pSkyTexData, fireSkyLump.uncompressedSize);
        #else
            const std::byte* const pSkyTexData = (const std::byte*) gpLumpCache[skytex.lumpNum];
        #endif

        LIBGPU_LoadImage(vramRect, (const uint16_t*)(pSkyTexData + sizeof(texlump_header_t)));

        // Mark this as uploaded now
        skyTex.uploadFrameNum = gNumFramesDrawn;
    }

    // Draw the fire sky pieces.
    // Note: slightly tweaked positions for Doom vs Final Doom.
    {
        constexpr uint8_t SKY_W = 64;       // PsyDoom: fix a 4 pixel gap at the right side of the screen with the fire ('64' rather than than the original '63')
        constexpr uint8_t SKY_H = 128;

        const auto texU = skyTex.texPageCoordX;
        const auto texV = skyTex.texPageCoordY;

        int16_t x = 0;
        const int16_t y = (screenStyle == TitleScreenStyle::FinalDoom) ? 112 : 116;

        for (int32_t i = 0; i < 4; ++i) {
            I_DrawSprite(skyTex.texPageId, gPaletteClutId_CurMapSky, x, y, texU, texV, SKY_W, SKY_H);
            x += SKY_W;
        }
    }

    // Draw the title logo for Final Doom (on top of everything):
    if (screenStyle == TitleScreenStyle::FinalDoom) {
        const uint8_t rgb = (uint8_t) gTitleScreenSpriteY;

        // PsyDoom: the TITLE logo might not be at UV 0,0 anymore! (if limit removing, but always offset to be safe)
        #if PSYDOOM_MODS
            I_DrawColoredSprite(
                gTex_TITLE.texPageId,
                Game::getTexPalette_TITLE(),
                0,
                0,
                gTex_TITLE.texPageCoordX,
                gTex_TITLE.texPageCoordY,
                SCREEN_W,
                SCREEN_H,
                rgb,
                rgb,
                rgb,
                false
            );
        #else
            I_DrawColoredSprite(gTex_TITLE.texPageId, gPaletteClutIds[TITLEPAL], 0, 0, 0, 0, SCREEN_W, SCREEN_H, rgb, rgb, rgb, false);
        #endif
    }

    // GEC Master Edition: draw the 'DOOM', 'Final DOOM' and 'Master Edition' logos
    #if PSYDOOM_MODS
        if (screenStyle == TitleScreenStyle::GEC_ME) {
            const uint8_t rgb = (uint8_t) gTitleScreenSpriteY;

            // Decide on the x and y position of the 'DOOM' and 'Final DOOM' logos: put them side by side and roughly in the center of the screen
            const int32_t freeXSpace = std::max(SCREEN_W - gTex_DOOM.width - gTex_FINAL.width, 0);
            const int32_t leftRightXPad = freeXSpace / 3;
            const int32_t middleXSpacing = freeXSpace - leftRightXPad * 2;
            const int32_t doomLogoX = leftRightXPad;
            const int32_t doomLogoY = (SCREEN_H - skyTex.height / 3 - gTex_DOOM.height) / 2;
            const int32_t finalLogoX = leftRightXPad + gTex_DOOM.width + middleXSpacing;
            const int32_t finalLogoY = (SCREEN_H - skyTex.height / 3 - gTex_FINAL.height) / 2;

            // Draw the 'DOOM' logo
            I_DrawColoredSprite(
                gTex_DOOM.texPageId,
                Game::getTexPalette_DOOM(),
                (int16_t) doomLogoX,
                (int16_t) doomLogoY,
                gTex_DOOM.texPageCoordX,
                gTex_DOOM.texPageCoordY,
                gTex_DOOM.width,
                gTex_DOOM.height,
                rgb,
                rgb,
                rgb,
                false
            );

            // Draw the 'Final DOOM' logo
            I_DrawColoredSprite(
                gTex_FINAL.texPageId,
                Game::getTexPalette_FINAL(),
                (int16_t) finalLogoX,
                (int16_t) finalLogoY,
                gTex_FINAL.texPageCoordX,
                gTex_FINAL.texPageCoordY,
                gTex_FINAL.width,
                gTex_FINAL.height,
                rgb,
                rgb,
                rgb,
                false
            );

            // Draw the 'Master Edition' logo
            I_DrawColoredSprite(
                gTex_DATA.texPageId,
                Game::getTexPalette_DATA(),
                49,
                12,
                gTex_DATA.texPageCoordX + 1,
                gTex_DATA.texPageCoordY + 1,
                157,
                8,
                rgb,
                rgb,
                rgb,
                false
            );
        }
    #endif

    I_SubmitGpuCmds();
    I_DrawPresent();
}

#endif  // #if PSYDOOM_MODS
