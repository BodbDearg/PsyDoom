//------------------------------------------------------------------------------------------------------------------------------------------
// Update and drawing logic for the credits screen.
// This is rewritten for PsyDoom to support a flexible list of credit screens defined via MAPINFO.
// To see the original logic, check out the version of this in the 'Old' code folder.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "cr_main.h"

#if PSYDOOM_MODS

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_texcache.h"
#include "Doom/Base/s_sound.h"
#include "Doom/d_main.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/MapInfo/MapInfo.h"
#include "PsyDoom/Utils.h"
#include "Wess/psxcd.h"

// Holds resources and settings for one credits screen
struct CrPageData {
    MapInfo::CreditsPage    info;       // MAPINFO for this credits screen
    texture_t               bgTex;      // The texture for the background
    texture_t               fgTex;      // The texture for the foreground/text
};

static std::vector<CrPageData>      gPageDatas;                             // A list of all the credits screen pages and their textures
static int32_t                      gCreditsScrollYPos;                     // Current credits scroll position
static uint32_t                     gCreditsPage;                           // Which page the credits screen we are on currently
static int32_t                      gVBlanksUntilCreditScreenUpdate;        // This controls the update rate

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: builds the list of 'CrPageData' structures, one for each credits page.
// Note: this is only done once since the list must live for the duration of the app's lifetime (due to texture cache back references).
//------------------------------------------------------------------------------------------------------------------------------------------
static void Init_CreditPageDatas() noexcept {
    // Already built the page list?
    // Can't touch the list once it's built, since the texture cache may reference entries within the list.
    if (!gPageDatas.empty())
        return;

    for (const MapInfo::CreditsPage& creditsPageInfo : MapInfo::getCredits()) {
        CrPageData& pageData = gPageDatas.emplace_back();
        pageData.info = creditsPageInfo;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Startup/init logic for the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
void START_Credits() noexcept {
    // Build the list of credit pages if not already done
    Init_CreditPageDatas();

    // Cache required textures
    for (CrPageData& pageData : gPageDatas) {
        I_LoadAndCacheTexLump(pageData.bgTex, pageData.info.bgPic);
        I_LoadAndCacheTexLump(pageData.fgTex, pageData.info.fgPic);
    }

    // Set initial screen state
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
    if (Input::isQuitRequested())
        return;

    psxcd_stop();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t TIC_Credits() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    if (gPlayersElapsedVBlanks[0] <= 0) {
        gbKeepInputEvents = true;   // Don't consume 'key pressed' etc. events yet, not ticking...
        return ga_nothing;
    }

    // If any controller buttons are pressed then exit the credits.
    // PsyDoom: modified this to just accept certain menu inputs only.
    if (gTickInputs[0].fMenuOk() || gTickInputs[0].fMenuStart() || gTickInputs[0].fMenuBack())
        return ga_exit;

    // We only update/scroll this screen periodically, see if it is time:
    gVBlanksUntilCreditScreenUpdate -= gPlayersElapsedVBlanks[0];

    if (gVBlanksUntilCreditScreenUpdate > 0)
        return ga_nothing;

    gVBlanksUntilCreditScreenUpdate = 2;
    gCreditsScrollYPos -= 1;

    // Bail if the credits page is not valid!
    if (gCreditsPage >= gPageDatas.size())
        return ga_exitdemo;

    // Is it time to go onto a new credits page?
    CrPageData& pageData = gPageDatas[gCreditsPage];

    if (gCreditsScrollYPos < -pageData.info.maxScroll) {
        if (gCreditsPage + 1 < gPageDatas.size()) {
            // Move onto the next page
            gCreditsPage++;
            gCreditsScrollYPos = SCREEN_H;
        }
        else {
            // No more credit pages, finish up!
            return ga_exitdemo;
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

    // PsyDoom: UI drawing setup for the new Vulkan renderer
    Utils::onBeginUIDrawing();

    // Don't draw anything if the credits page is not valid!
    if (gCreditsPage < gPageDatas.size()) {
        // Draw the current credits page background
        CrPageData& pageData = gPageDatas[gCreditsPage];
        I_CacheAndDrawBackgroundSprite(pageData.bgTex, R_GetPaletteClutId(pageData.info.bgPal));

        // Draw the scrolling credits text for this page.
        // Note: this can potentially be drawn additive blended.
        const uint16_t semiTransTPageBits = (pageData.info.bFgAdditive) ? LIBGPU_GetTPageSemiTransBits(1) : 0;

        I_CacheTex(pageData.fgTex);
        I_DrawColoredSprite(
            pageData.fgTex.texPageId | semiTransTPageBits,
            R_GetPaletteClutId(pageData.info.fgPal),
            pageData.info.fgXPos,
            (int16_t) gCreditsScrollYPos,
            pageData.fgTex.texPageCoordX,
            pageData.fgTex.texPageCoordY,
            pageData.fgTex.width,
            pageData.fgTex.height,
            128,
            128,
            128,
            pageData.info.bFgAdditive
        );
    }

    // PsyDoom: draw any enabled performance counters
    I_DrawEnabledPerfCounters();

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

#endif  // #if PSYDOOM_MODS
