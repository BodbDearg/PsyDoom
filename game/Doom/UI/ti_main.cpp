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
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/MapInfo/MapInfo.h"
#include "PsyDoom/Utils.h"
#include "PsyDoom/Video.h"
#include "PsyDoom/Vulkan/VRenderer.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"

#include <algorithm>
#include <cmath>

// Doom: current position of the TITLE (Doom) logo; also adopted by the 'legals' screen for the same purpose.
// Even when the logo is not moving it can still be used for timing purposes.
int32_t gTitleScreenSpriteY;

// Final Doom: The current fade/color-multiplier for the TITLE screen image.
static int32_t gTitleScreenFadeRgb;

// The DOOM logo texture for 'Doom' and 'Final Doom' style title screens
texture_t gTex_TITLE;

#if PSYDOOM_MODS
    static texture_t gTex_FINAL;        // The 'Final DOOM' logo for 'GEC Master Edition Beta 3' style title screens
    static texture_t gTex_TITLE2;       // A second title screen texture used for 'GEC Master Edition Beta 4' style title screens
#endif

// Controls how frequently the title sprite and fire sky update and move
static int32_t gVBlanksUntilTitleSprMove;
static int32_t gVBlanksUntilTitleFireMove;

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws a sprite that is assumed to be full size of the texture.
// Draws it with the specified shade and semi transparency setting.
//------------------------------------------------------------------------------------------------------------------------------------------
static void TI_DrawSpriteTexture(
    const texture_t& tex,
    const uint16_t paletteClut,
    const int32_t posX,
    const int32_t posY,
    const uint8_t rgb = 128,
    const bool bSemiTrans = false,
    const uint8_t semiTransMode = 0
) noexcept {
    const uint16_t semiTransTPageBits = (bSemiTrans) ? LIBGPU_GetTPageSemiTransBits(semiTransMode) : 0;

    I_DrawColoredSprite(
        tex.texPageId | semiTransTPageBits,
        paletteClut,
        (int16_t) posX,
        (int16_t) posY,
        tex.texPageCoordX,
        tex.texPageCoordY,
        tex.width,
        tex.height,
        rgb,
        rgb,
        rgb,
        bSemiTrans
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the fire for the title screen
//------------------------------------------------------------------------------------------------------------------------------------------
static void TI_DrawFire(const bool bUseFinalDoomFirePos, const bool bAdditiveBlend) noexcept {
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
        int32_t numSkyPieces = 4;

        #if PSYDOOM_VULKAN_RENDERER
            // PsyDoom: make the fire sky support widescreen if this game or mod allows it:
            if (Video::isUsingVulkanRenderPath() && Config::gbVulkanWidescreenEnabled && MapInfo::getGameInfo().bAllowWideTitleScreenFire) {
                const int32_t extraSpaceAtSides = (int32_t) std::max(std::floor(VRenderer::gPsxCoordsFbX), 0.0f);
                const int32_t numExtraPiecesAtSides = std::min((extraSpaceAtSides + SKY_W - 1) / SKY_W, 64);
                x -= (int16_t)(SKY_W * numExtraPiecesAtSides);
                numSkyPieces += numExtraPiecesAtSides * 2;
            }
        #endif

        const int16_t y = (bUseFinalDoomFirePos) ? 112 : 116;

        for (int32_t i = 0; i < numSkyPieces; ++i) {
            #if PSYDOOM_MODS
                const uint16_t semiTransTPageBits = (bAdditiveBlend) ? LIBGPU_GetTPageSemiTransBits(1) : 0;

                I_DrawColoredSprite(
                    skyTex.texPageId | semiTransTPageBits,
                    gPaletteClutId_CurMapSky,
                    x,
                    y,
                    texU,
                    texV,
                    SKY_W,
                    SKY_H,
                    128,
                    128,
                    128,
                    bAdditiveBlend
                );
            #else
                I_DrawSprite(skyTex.texPageId, gPaletteClutId_CurMapSky, x, y, texU, texV, SKY_W, SKY_H);
            #endif

            x += SKY_W;
        }
    }
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the title screen logos for 'GEC Master Edition Beta 3'
//------------------------------------------------------------------------------------------------------------------------------------------
static void TI_DrawGecMeBeta3Logos() noexcept {
    texture_t& skyTex = *gpSkyTexture;
    const uint8_t rgb = (uint8_t) gTitleScreenFadeRgb;

    // Decide on the x and y position of the 'DOOM' and 'Final DOOM' logos: put them side by side and roughly in the center of the screen
    const int32_t freeXSpace = std::max(SCREEN_W - gTex_DOOM.width - gTex_FINAL.width, 0);
    const int32_t leftRightXPad = freeXSpace / 3;
    const int32_t middleXSpacing = freeXSpace - leftRightXPad * 2;
    const int32_t doomLogoX = leftRightXPad;
    const int32_t doomLogoY = (SCREEN_H - skyTex.height / 3 - gTex_DOOM.height) / 2;
    const int32_t finalLogoX = leftRightXPad + gTex_DOOM.width + middleXSpacing;
    const int32_t finalLogoY = (SCREEN_H - skyTex.height / 3 - gTex_FINAL.height) / 2;

    // Draw the 'DOOM' and 'FINAL DOOM' logos
    TI_DrawSpriteTexture(gTex_DOOM,  Game::getTexClut_DOOM(), doomLogoX, doomLogoY, rgb);
    TI_DrawSpriteTexture(gTex_FINAL, Game::getTexClut_FINAL(), finalLogoX, finalLogoY, rgb);

    // Draw the 'Master Edition' logo: this one is more complex since the texcoords need to be specified manually
    I_DrawColoredSprite(
        gTex_DATA.texPageId,
        Game::getTexClut_DATA(),
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
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Main title screen rendering for various games
//------------------------------------------------------------------------------------------------------------------------------------------
static void DRAW_Title_Doom() noexcept {
    // Draw the logo first
    const int16_t titleY = (int16_t) gTitleScreenSpriteY;

    #if PSYDOOM_MODS
        // PsyDoom: the TITLE logo might not be at UV 0,0 anymore! (if limit removing, but always offset to be safe).
        // PsyDoom: also ensure the TITLE logo is centered horizontally - enables widescreen assets to be used.
        TI_DrawSpriteTexture(gTex_TITLE, Game::getTexClut_TITLE(), I_GetCenteredDrawPos_X(gTex_TITLE), titleY);
    #else
        I_DrawSprite(gTex_TITLE.texPageId, gPaletteClutIds[TITLEPAL], 0, titleY, 0, 0, SCREEN_W, SCREEN_H);
    #endif

    // Draw the fire: use original 'Doom' fire positioning and no additive blending
    TI_DrawFire(false, false);
}

static void DRAW_Title_FinalDoom() noexcept {
    // Draw the fire: use 'Final Doom' fire positioning and no additive blending
    TI_DrawFire(true, false);

    // Draw the title logo.
    const uint8_t rgb = (uint8_t) gTitleScreenFadeRgb;

    #if PSYDOOM_MODS
        // PsyDoom: the TITLE logo might not be at UV 0,0 anymore! (if limit removing, but always offset to be safe).
        // PsyDoom: also ensure the TITLE logo is centered horizontally and vertically - enables widescreen assets to be used.
        TI_DrawSpriteTexture(
            gTex_TITLE,
            Game::getTexClut_TITLE(),
            I_GetCenteredDrawPos_X(gTex_TITLE),
            I_GetCenteredDrawPos_Y(gTex_TITLE),
            rgb
        );
    #else
        I_DrawColoredSprite(gTex_TITLE.texPageId, gPaletteClutIds[TITLEPAL], 0, 0, 0, 0, SCREEN_W, SCREEN_H, rgb, rgb, rgb, false);
    #endif
}

#if PSYDOOM_MODS
static void DRAW_Title_GecMeBeta3() noexcept {
    TI_DrawFire(false, false);
    TI_DrawGecMeBeta3Logos();
}

static void DRAW_Title_GecMeBeta4() noexcept {
    // Fade in the background and logos and draw semi transparent fire (additive blending) in between them
    const int32_t elapsedVBlanks = gTicCon - gMenuTimeoutStartTicCon;
    const uint8_t title1Rgb = (uint8_t) std::clamp(elapsedVBlanks - 60, 0, 128);
    const uint8_t title2Rgb = (uint8_t) std::clamp(elapsedVBlanks - 180, 0, 128);

    TI_DrawSpriteTexture(gTex_TITLE, Game::getTexClut_TITLE(), I_GetCenteredDrawPos_X(gTex_TITLE2), 0, title1Rgb);
    TI_DrawFire(false, true);
    TI_DrawSpriteTexture(gTex_TITLE2, Game::getTexClut_TITLE2(), I_GetCenteredDrawPos_X(gTex_TITLE2), 0, title2Rgb, true, 1);
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Moves the TITLE sprite and optionally resets the timeout when it reaches the top of the screen
//------------------------------------------------------------------------------------------------------------------------------------------
static void TI_MoveTitleSprite(const int32_t elapsedVBlanks, const bool bResetTimeoutOnEnd) noexcept {
    // If it is time to move the title sprite then do that.
    // Stop the title sprite also once it reaches the top of the screen and begin the menu timeout counter (if specified).
    gVBlanksUntilTitleSprMove -= elapsedVBlanks;

    if (gVBlanksUntilTitleSprMove <= 0) {
        gVBlanksUntilTitleSprMove = 2;

        if (gTitleScreenSpriteY != 0) {
            gTitleScreenSpriteY -= 1;

            if ((gTitleScreenSpriteY == 0) && bResetTimeoutOnEnd) {
                // Restart the menu timeout counter
                gMenuTimeoutStartTicCon = gTicCon;
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Updates the title screen fire (Doom style)
//------------------------------------------------------------------------------------------------------------------------------------------
static void TI_UpdateFire_Doom(const int32_t elapsedVBlanks) noexcept {
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
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Updates the title screen fire (Final Doom style)
//------------------------------------------------------------------------------------------------------------------------------------------
static void TI_UpdateFire_FinalDoom(const int32_t elapsedVBlanks) noexcept {
    // Update the fire sky after a certain number of ticks have elapsed
    gVBlanksUntilTitleFireMove -= elapsedVBlanks;

    if ((gVBlanksUntilTitleFireMove <= 0) && (gTitleScreenFadeRgb >= 128)) {
        gVBlanksUntilTitleFireMove = 2;
        P_UpdateFireSky(*gpSkyTexture);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs the RGB fade in used by Final Doom
//------------------------------------------------------------------------------------------------------------------------------------------
static void TI_UpdateFadeRgb(const int32_t elapsedVBlanks) noexcept {
    // PsyDoom: added logic to make sure this doesn't run too fast for uncapped framerates!
    #if PSYDOOM_MODS
        if (elapsedVBlanks <= 0)
            return;
    #endif

    // Do the fade in of the title screen
    if (gTitleScreenFadeRgb <= 128) {
        gTitleScreenFadeRgb += 4;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update helper: checks for title screen timeout and returns what game action should be taken by the update function
//------------------------------------------------------------------------------------------------------------------------------------------
static gameaction_t TI_CheckTimeOut() noexcept {
    const bool bHasTimedOut = (gTicCon - gMenuTimeoutStartTicCon >= 1800);
    return (bHasTimedOut) ? ga_timeout : ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performing title screen updates for various games.
// Each function returns what game action should be taken ('ga_nothing' if the screen should continue updating).
//------------------------------------------------------------------------------------------------------------------------------------------
static gameaction_t Tic_Title_Doom(const int32_t elapsedVBlanks) noexcept {
    TI_MoveTitleSprite(elapsedVBlanks, true);
    TI_UpdateFire_Doom(elapsedVBlanks);

    // Once the title sprite reaches the top of the screen, timeout after a while:
    return (gTitleScreenSpriteY == 0) ? TI_CheckTimeOut() : ga_nothing;
}

static gameaction_t Tic_Title_FinalDoom(const int32_t elapsedVBlanks) noexcept {
    TI_UpdateFire_FinalDoom(elapsedVBlanks);
    TI_UpdateFadeRgb(elapsedVBlanks);

    // Once the title is full faded in, timeout after a while:
    return (gTitleScreenFadeRgb >= 128) ? TI_CheckTimeOut() : ga_nothing;
}

#if PSYDOOM_MODS
static gameaction_t Tic_Title_GecMeBeta3(const int32_t elapsedVBlanks) noexcept {
    // This just needs the same logic as Final Doom
    return Tic_Title_FinalDoom(elapsedVBlanks);
}

static gameaction_t Tic_Title_GecMeBeta4(const int32_t elapsedVBlanks) noexcept {
    TI_MoveTitleSprite(elapsedVBlanks, false);
    TI_UpdateFire_Doom(elapsedVBlanks);
    TI_UpdateFadeRgb(elapsedVBlanks);

    // Once the title is full faded in, timeout after a while:
    return (gTitleScreenFadeRgb >= 128) ? TI_CheckTimeOut() : ga_nothing;
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialization logic for the main title screen
//------------------------------------------------------------------------------------------------------------------------------------------
void START_Title() noexcept {
    // PsyDoom: initialize these for good measure
    #if PSYDOOM_MODS
        gVBlanksUntilTitleSprMove = 0;
        gVBlanksUntilTitleFireMove = 0;
    #endif
    
    // Cleanup the texture cache and remove anything we can
    I_PurgeTexCache();

    // Show the loading plaque
    W_CacheLumpName("LOADING", PU_STATIC, false);
    I_LoadAndCache_LOADING_TexLump(gTex_LOADING);
    I_DrawLoadingPlaque(gTex_LOADING, 95, 109, Game::getTexClut_LOADING());

    // Load sounds for the menu
    S_LoadMapSoundAndMusic(0);

    // Cache commonly used UI lumps for fast access and upload them to VRAM
    W_CacheLumpName(Game::getTexLumpName_OptionsBg(), PU_STATIC, false);
    W_CacheLumpName("NETERR", PU_STATIC, false);
    W_CacheLumpName("PAUSE", PU_STATIC, false);

    I_LoadAndCacheTexLump(gTex_OptionsBg, Game::getTexLumpName_OptionsBg());
    I_LoadAndCacheTexLump(gTex_NETERR, "NETERR", 0);
    I_LoadAndCacheTexLump(gTex_PAUSE, "PAUSE", 0);

    // PsyDoom: if we are doing the 'GEC Master Edition' style title screen then different lumps are used
    const TitleScreenStyle screenStyle = MapInfo::getGameInfo().titleScreenStyle;

    #if PSYDOOM_MODS
        if (screenStyle == TitleScreenStyle::GEC_ME_BETA4) {
            I_LoadAndCacheTexLump(gTex_TITLE, Game::getTexLumpName_TITLE());
            I_LoadAndCacheTexLump(gTex_TITLE2, Game::getTexLumpName_TITLE2());
        } 
        else if (screenStyle == TitleScreenStyle::GEC_ME_BETA3) {
            I_LoadAndCacheTexLump(gTex_DOOM, "DOOM", 0);
            I_LoadAndCacheTexLump(gTex_FINAL, "FINAL", 0);
            I_LoadAndCacheTexLump(gTex_DATA, "DATA", 0);
        } 
        else {
            I_LoadAndCacheTexLump(gTex_TITLE, Game::getTexLumpName_TITLE());
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
        gPaletteClutId_CurMapSky = Game::getTexClut_TitleScreenFire();

        W_CacheLumpNum(skyTex.lumpNum, PU_CACHE, true);
        I_CacheTex(skyTex);
    }

    // Doom: initially the DOOM logo is offscreen.
    // Final Doom: begin fading in the graphics.
    gTitleScreenSpriteY = SCREEN_H + 10;
    gTitleScreenFadeRgb = 0;

    // Play the music for the title screen
    #if PSYDOOM_MODS
        const int32_t cdTrackOverride = MapInfo::getGameInfo().titleScreenCdTrackOverride;
        psxcd_play((cdTrackOverride >= 2) ? cdTrackOverride : gCDTrackNum[cdmusic_title_screen], gCdMusicVol);
    #else
        psxcd_play(gCDTrackNum[cdmusic_title_screen], gCdMusicVol);
    #endif

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

    // Update the title screen in the appropriate manner
    const int32_t elapsedVBlanks = gPlayersElapsedVBlanks[gCurPlayerIndex];
    const TitleScreenStyle screenStyle = MapInfo::getGameInfo().titleScreenStyle;

    switch (screenStyle) {
        case TitleScreenStyle::Doom:            return Tic_Title_Doom(elapsedVBlanks);
        case TitleScreenStyle::FinalDoom:       return Tic_Title_FinalDoom(elapsedVBlanks);

    #if PSYDOOM_MODS
        case TitleScreenStyle::GEC_ME_BETA3:    return Tic_Title_GecMeBeta3(elapsedVBlanks);
        case TitleScreenStyle::GEC_ME_BETA4:    return Tic_Title_GecMeBeta4(elapsedVBlanks);
    #endif

        default:
            FatalErrors::raiseF("Unknown/unhandled title screen style %d!", (int) screenStyle);
            break;
    }

    // Should never get to here!
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

    // PsyDoom: UI drawing setup for the new Vulkan renderer
    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();
    #endif

    // Draw the title screen in the appropriate style
    const TitleScreenStyle screenStyle = MapInfo::getGameInfo().titleScreenStyle;

    switch (screenStyle) {
        case TitleScreenStyle::Doom:            DRAW_Title_Doom();          break;
        case TitleScreenStyle::FinalDoom:       DRAW_Title_FinalDoom();     break;

    #if PSYDOOM_MODS
        case TitleScreenStyle::GEC_ME_BETA3:    DRAW_Title_GecMeBeta3();    break;
        case TitleScreenStyle::GEC_ME_BETA4:    DRAW_Title_GecMeBeta4();    break;
    #endif

        default:
            FatalErrors::raiseF("Unknown/unhandled title screen style %d!", (int) screenStyle);
            break;
    }

    // PsyDoom: draw any enabled performance counters
    #if PSYDOOM_MODS
        I_DrawEnabledPerfCounters();
    #endif

    // Finish up the render
    I_SubmitGpuCmds();
    I_DrawPresent();
}
#endif  // #if PSYDOOM_MODS
