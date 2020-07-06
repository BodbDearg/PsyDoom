#include "cr_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/d_main.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "PcPsx/Game.h"
#include "PcPsx/Utils.h"
#include "ti_main.h"
#include "Wess/psxcd.h"

// Credits screen resources
static texture_t gTex_LEVCRED2;
static texture_t gTex_IDCRED1;
static texture_t gTex_IDCRED2;
static texture_t gTex_WMSCRED1;
static texture_t gTex_WMSCRED2;

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

    if (Game::isFinalDoom()) {
        I_LoadAndCacheTexLump(gTex_TITLE, "TITLE", 0);
        I_LoadAndCacheTexLump(gTex_LEVCRED2, "LEVCRED2", 0);
    }
    
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
    psxcd_stop();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t TIC_Credits() noexcept {
    // PC-PSX: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PC_PSX_DOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0)
            return ga_nothing;
    #endif

    // If any controller buttons are pressed then exit the credits.
    // PC-PSX: just accept certain menu inputs for this.
    #if PC_PSX_DOOM_MODS
        if (gTickInputs[0].bMenuOk || gTickInputs[0].bMenuStart || gTickInputs[0].bMenuBack)
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
    if (Game::isFinalDoom()) {
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
    else {
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

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
void DRAW_Credits() noexcept {
    // Increment the frame count for the texture cache's management purposes
    I_IncDrawnFrameCount();
    
    // Draw the background and scrolling credits text for whatever credits page we are on
    if (Game::isFinalDoom()) {
        // Final Doom: there are 3 pages: level, ID and Williams credits
        if (gCreditsPage == 0) {
            I_CacheAndDrawSprite(gTex_TITLE, 0, 0, gPaletteClutIds[TITLEPAL]);
            I_CacheAndDrawSprite(gTex_LEVCRED2, 11, (int16_t) gCreditsScrollYPos, gPaletteClutIds[WCREDITS1PAL]);
        }
        else if (gCreditsPage == 1) {
            I_CacheAndDrawSprite(gTex_WMSCRED1, 0, 0, gPaletteClutIds[WCREDITS1PAL]);
            I_CacheAndDrawSprite(gTex_WMSCRED2, 5, (int16_t) gCreditsScrollYPos, gPaletteClutIds[WCREDITS1PAL]);
        }
        else if (gCreditsPage == 2) {
            I_CacheAndDrawSprite(gTex_IDCRED1, 0, 0, gPaletteClutIds[IDCREDITS1PAL]);
            I_CacheAndDrawSprite(gTex_IDCRED2, 9, (int16_t) gCreditsScrollYPos, gPaletteClutIds[WCREDITS1PAL]);
        }
    }
    else {
        // Regular Doom: there are 2 pages: ID and Williams credits
        if (gCreditsPage == 0) {
            I_CacheAndDrawSprite(gTex_IDCRED1, 0, 0, gPaletteClutIds[IDCREDITS1PAL]);
            I_CacheAndDrawSprite(gTex_IDCRED2, 9, (int16_t) gCreditsScrollYPos, gPaletteClutIds[UIPAL]);
        }
        else if (gCreditsPage == 1) {
            I_CacheAndDrawSprite(gTex_WMSCRED1, 0, 0, gPaletteClutIds[WCREDITS1PAL]);
            I_CacheAndDrawSprite(gTex_WMSCRED2, 7, (int16_t) gCreditsScrollYPos, gPaletteClutIds[UIPAL]);
        }
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}
