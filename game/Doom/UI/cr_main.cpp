#include "cr_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/d_main.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "PsxVm/PsxVm.h"
#include "Wess/psxcd.h"

// Credits screen resources
static const VmPtr<texture_t>   gTex_IDCRED1(0x80097B30);
static const VmPtr<texture_t>   gTex_IDCRED2(0x80097B50);
static const VmPtr<texture_t>   gTex_WMSCRED1(0x80097B70);
static const VmPtr<texture_t>   gTex_WMSCRED2(0x80097B90);

// Current credits scroll position
static const VmPtr<int32_t> gCreditsScrollYPos(0x8007825C);
static const VmPtr<int32_t> gCreditsPage(0x800781A4);

// This controls the update rate
static const VmPtr<int32_t> gVBlanksUntilCreditScreenUpdate(0x80077CCC);

//------------------------------------------------------------------------------------------------------------------------------------------
// Startup/init logic for the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
void START_Credits() noexcept {
    sp -= 0x28;
    
    // Cache required textures and set initial screen state
    I_LoadAndCacheTexLump(*gTex_IDCRED1, "IDCRED1", 0);
    I_LoadAndCacheTexLump(*gTex_IDCRED2, "IDCRED2", 0);
    I_LoadAndCacheTexLump(*gTex_WMSCRED1, "WMSCRED1", 0);
    I_LoadAndCacheTexLump(*gTex_WMSCRED2, "WMSCRED2", 0);
    
    *gCreditsScrollYPos = SCREEN_H;
    *gCreditsPage = 0;
    
    // Play the credits music
    a0 = gCDTrackNum[cdmusic_credits_demo];
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = *gCdMusicVol;
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    sw(gCDTrackNum[cdmusic_credits_demo], sp + 0x10);
    sw(*gCdMusicVol, sp + 0x14);
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    psxcd_play_at_andloop();
    
    // TODO: comment on elapsed sector stuff here
    do {
        psxcd_elapsed_sectors();
    } while (v0 == 0);
    
    sp += 0x28;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown logic for the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
void STOP_Credits() noexcept {
    psxcd_stop();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t TIC_Credits() noexcept {
    // If any controller buttons are pressed then exit the credits
    if (gTicButtons[0] != 0)
        return ga_exit;
    
    // We only update/scroll this screen periodically, see if it is time
    *gVBlanksUntilCreditScreenUpdate -= gPlayersElapsedVBlanks[0];

    if (*gVBlanksUntilCreditScreenUpdate > 0) 
        return ga_nothing;
    
    *gVBlanksUntilCreditScreenUpdate = 2;
    *gCreditsScrollYPos -= 1;

    // Move onto the next page or exit if we have scrolled far enough
    if (*gCreditsPage == 0) {
        if (*gCreditsScrollYPos < -182) {
            *gCreditsScrollYPos = SCREEN_H;
            *gCreditsPage = 1;
        }
    }
    else if (*gCreditsPage == 1) {
        if (*gCreditsScrollYPos < -228) {
            return ga_exitdemo;
        }
    }

    return ga_nothing;
}

void _thunk_TIC_Credits() noexcept {
    v0 = TIC_Credits();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the credits screen
//------------------------------------------------------------------------------------------------------------------------------------------
void DRAW_Credits() noexcept {
    // Increment the frame count for the texture cache's management purposes
    I_IncDrawnFrameCount();
    
    // Draw the background and scrolling credits text for whatever credits page we are on.
    // There are two pages, ID and Williams credits:
    if (*gCreditsPage == 0) {
        I_CacheAndDrawSprite(*gTex_IDCRED1, 0, 0, gPaletteClutIds[IDCREDITS1PAL]);
        I_CacheAndDrawSprite(*gTex_IDCRED2, 9, (int16_t) *gCreditsScrollYPos, gPaletteClutIds[UIPAL]);
    } 
    else if (*gCreditsPage == 1) {
        I_CacheAndDrawSprite(*gTex_WMSCRED1, 0, 0, gPaletteClutIds[WCREDITS1PAL]);
        I_CacheAndDrawSprite(*gTex_WMSCRED2, 7, (int16_t) *gCreditsScrollYPos, gPaletteClutIds[UIPAL]);
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}
