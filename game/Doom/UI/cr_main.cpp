#include "cr_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/d_main.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/MapInfo/MapInfo.h"
#include "PsyDoom/Utils.h"
#include "ti_main.h"
#include "Wess/psxcd.h"

// Credits screen resources
static texture_t gTex_LEVCRED2;
static texture_t gTex_IDCRED1;
static texture_t gTex_IDCRED2;
static texture_t gTex_WMSCRED1;
static texture_t gTex_WMSCRED2;

// PsyDoom: credits screen resources for 'GEC Master Edition'
#if PSYDOOM_MODS
    static texture_t gTex_GEC;
    static texture_t gTex_GECCRED;
    static texture_t gTex_DWOLRD;
    static texture_t gTex_DWCRED;
#endif

// Current credits scroll position
static int32_t  gCreditsScrollYPos;
static int32_t  gCreditsPage;

// This controls the update rate
static int32_t  gVBlanksUntilCreditScreenUpdate;

//------------------------------------------------------------------------------------------------------------------------------------------
// Startup/init logic for the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
void START_Credits() noexcept {
    // Cache required textures and set initial screen state
    I_LoadAndCacheTexLump(gTex_IDCRED1, "IDCRED1", 0);
    I_LoadAndCacheTexLump(gTex_IDCRED2, "IDCRED2", 0);
    I_LoadAndCacheTexLump(gTex_WMSCRED1, "WMSCRED1", 0);
    I_LoadAndCacheTexLump(gTex_WMSCRED2, "WMSCRED2", 0);

    const CreditsScreenStyle screenStyle = MapInfo::getGameInfo().creditsScreenStyle;

    if (screenStyle != CreditsScreenStyle::Doom) {
        I_LoadAndCacheTexLump(gTex_TITLE, "TITLE", 0);
        I_LoadAndCacheTexLump(gTex_LEVCRED2, "LEVCRED2", 0);
    }

    #if PSYDOOM_MODS
        if (screenStyle == CreditsScreenStyle::GEC_ME) {
            I_LoadAndCacheTexLump(gTex_GEC, "GEC", 0);
            I_LoadAndCacheTexLump(gTex_GECCRED, "GECCRED", 0);
            I_LoadAndCacheTexLump(gTex_DWOLRD, "DWOLRD", 0);
            I_LoadAndCacheTexLump(gTex_DWCRED, "DWCRED", 0);
        }
    #endif

    gCreditsScrollYPos = SCREEN_H;
    gCreditsPage = 0;

    // Play the credits music
    psxcd_play_at_andloop(
        gCDTrackNum[cdmusic_credits_demo],
        gCdMusicVol,
        0,
        0,
        gCDTrackNum[cdmusic_credits_demo],
        gCdMusicVol,
        0,
        0
    );

    // Wait until some cd audio has been read
    Utils::waitForCdAudioPlaybackStart();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown logic for the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
void STOP_Credits([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // PsyDoom: if quitting the app then exit immediately
    #if PSYDOOM_MODS
        if (Input::isQuitRequested())
            return;
    #endif

    psxcd_stop();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t TIC_Credits() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0) {
            gbKeepInputEvents = true;   // Don't consume 'key pressed' etc. events yet, not ticking...
            return ga_nothing;
        }
    #endif

    // If any controller buttons are pressed then exit the credits.
    // PsyDoom: just accept certain menu inputs for this.
    #if PSYDOOM_MODS
        if (gTickInputs[0].fMenuOk() || gTickInputs[0].fMenuStart() || gTickInputs[0].fMenuBack())
            return ga_exit;
    #else
        if (gTicButtons[0] != 0)
            return ga_exit;
    #endif

    // We only update/scroll this screen periodically, see if it is time
    gVBlanksUntilCreditScreenUpdate -= gPlayersElapsedVBlanks[0];

    if (gVBlanksUntilCreditScreenUpdate > 0)
        return ga_nothing;

    gVBlanksUntilCreditScreenUpdate = 2;
    gCreditsScrollYPos -= 1;

    // Move onto the next page or exit if we have scrolled far enough
    const CreditsScreenStyle screenStyle = MapInfo::getGameInfo().creditsScreenStyle;

    if (screenStyle == CreditsScreenStyle::Doom) {
        // Regular Doom: just 2 credits screen pages
        if (gCreditsPage == 0) {
            if (gCreditsScrollYPos < -182) {
                gCreditsScrollYPos = SCREEN_H;
                gCreditsPage = 1;
            }
        }
        else if (gCreditsPage == 1) {
            if (gCreditsScrollYPos < -228) {
                return ga_exitdemo;
            }
        }
    }
    else if (screenStyle == CreditsScreenStyle::FinalDoom) {
        // Final Doom: 3 credit screen pages
        if (gCreditsPage < 2) {
            if (gCreditsScrollYPos < -256) {
                gCreditsScrollYPos = SCREEN_H;
                gCreditsPage += 1;
            }
        }
        else if (gCreditsScrollYPos < -256) {
            return ga_exitdemo;
        }
    }
#if PSYDOOM_MODS
    else if (screenStyle == CreditsScreenStyle::GEC_ME) {
        // GEC Master Edition: 5 credit screen pages
        if (gCreditsPage < 4) {
            if (gCreditsScrollYPos < -256) {
                gCreditsScrollYPos = SCREEN_H;
                gCreditsPage += 1;
            }
        }
        else if (gCreditsScrollYPos < -256) {
            return ga_exitdemo;
        }
    }
    else {
        I_Error("TIC_Credits: unhandled credits screen type!");
    }
#endif  // #if PSYDOOM_MODS

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
void DRAW_Credits() noexcept {
    // Increment the frame count for the texture cache's management purposes
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    // Draw the background and scrolling credits text for whatever credits page we are on
    const MapInfo::GameInfo& gameInfo = MapInfo::getGameInfo();
    const int16_t xpos_IDCRED2 = gameInfo.creditsXPos_IDCRED2;
    const int16_t xpos_WMSCRED2 = gameInfo.creditsXPos_WMSCRED2;
    const int16_t xpos_LEVCRED2 = gameInfo.creditsXPos_LEVCRED2;
    const int16_t xpos_GECCRED = gameInfo.creditsXPos_GECCRED;
    const int16_t xpos_DWCRED = gameInfo.creditsXPos_DWCRED;

    const CreditsScreenStyle screenStyle = MapInfo::getGameInfo().creditsScreenStyle;

    if (screenStyle == CreditsScreenStyle::Doom) {
        // Regular Doom: there are 2 pages: ID and Williams credits
        if (gCreditsPage == 0) {
            I_CacheAndDrawBackgroundSprite(gTex_IDCRED1, Game::getTexPalette_IDCRED1());
            I_CacheAndDrawSprite(gTex_IDCRED2, xpos_IDCRED2, (int16_t) gCreditsScrollYPos, Game::getTexPalette_IDCRED2());
        }
        else if (gCreditsPage == 1) {
            I_CacheAndDrawBackgroundSprite(gTex_WMSCRED1, Game::getTexPalette_WMSCRED1());
            I_CacheAndDrawSprite(gTex_WMSCRED2, xpos_WMSCRED2, (int16_t) gCreditsScrollYPos, Game::getTexPalette_WMSCRED2());
        }
    }
    else if (screenStyle == CreditsScreenStyle::FinalDoom) {
        // Final Doom: there are 3 pages: level, ID and Williams credits
        if (gCreditsPage == 0) {
            I_CacheAndDrawBackgroundSprite(gTex_TITLE, Game::getTexPalette_TITLE());
            I_CacheAndDrawSprite(gTex_LEVCRED2, xpos_LEVCRED2, (int16_t) gCreditsScrollYPos, Game::getTexPalette_LEVCRED2());
        }
        else if (gCreditsPage == 1) {
            I_CacheAndDrawBackgroundSprite(gTex_WMSCRED1, Game::getTexPalette_WMSCRED1());
            I_CacheAndDrawSprite(gTex_WMSCRED2, xpos_WMSCRED2, (int16_t) gCreditsScrollYPos, Game::getTexPalette_WMSCRED2());
        }
        else if (gCreditsPage == 2) {
            I_CacheAndDrawBackgroundSprite(gTex_IDCRED1, Game::getTexPalette_IDCRED1());
            I_CacheAndDrawSprite(gTex_IDCRED2, xpos_IDCRED2, (int16_t) gCreditsScrollYPos, Game::getTexPalette_IDCRED2());
        }
    }
#if PSYDOOM_MODS
    // TODO: GEC ME BETA 4: work here needed!
    else if (screenStyle == CreditsScreenStyle::GEC_ME) {
        // GEC Master Edition: there are 5 pages: GEC, Doomworld, Final Doom level, ID and Williams credits
        if (gCreditsPage == 0) {
            I_CacheAndDrawBackgroundSprite(gTex_GEC, Game::getTexPalette_GEC());
            I_CacheAndDrawSprite(gTex_GECCRED, xpos_GECCRED, (int16_t) gCreditsScrollYPos, Game::getTexPalette_GECCRED());
        }
        else if (gCreditsPage == 1) {
            I_CacheAndDrawBackgroundSprite(gTex_DWOLRD, Game::getTexPalette_DWOLRD());
            I_CacheAndDrawSprite(gTex_DWCRED, xpos_DWCRED, (int16_t) gCreditsScrollYPos, Game::getTexPalette_DWCRED());
        }
        else if (gCreditsPage == 2) {
            I_CacheAndDrawBackgroundSprite(gTex_TITLE, Game::getTexPalette_TITLE());
            I_CacheAndDrawSprite(gTex_LEVCRED2, xpos_LEVCRED2, (int16_t) gCreditsScrollYPos, Game::getTexPalette_LEVCRED2());
        }
        else if (gCreditsPage == 3) {
            I_CacheAndDrawBackgroundSprite(gTex_WMSCRED1, Game::getTexPalette_WMSCRED1());
            I_CacheAndDrawSprite(gTex_WMSCRED2, xpos_WMSCRED2, (int16_t) gCreditsScrollYPos, Game::getTexPalette_WMSCRED2());
        }
        else if (gCreditsPage == 4) {
            I_CacheAndDrawBackgroundSprite(gTex_IDCRED1, Game::getTexPalette_IDCRED1());
            I_CacheAndDrawSprite(gTex_IDCRED2, xpos_IDCRED2, (int16_t) gCreditsScrollYPos, Game::getTexPalette_IDCRED2());
        }
    }
    else {
        I_Error("DRAW_Credits: unhandled credits screen type!");
    }
#endif  // #if PSYDOOM_MODS

    // PsyDoom: draw any enabled performance counters
    #if PSYDOOM_MODS
        I_DrawEnabledPerfCounters();
    #endif

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}
