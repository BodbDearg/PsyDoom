//------------------------------------------------------------------------------------------------------------------------------------------
// Code for the 'legals' screen.
// This screen is unused in the retail version of the game, it was only used in demo builds of DOOM.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "le_main.h"

#include "Doom/Base/i_crossfade.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_texcache.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "m_main.h"
#include "PcPsx/Input.h"
#include "PcPsx/Utils.h"
#include "ti_main.h"

// Texture for the legals screen text
static texture_t gTex_LEGALS;

//------------------------------------------------------------------------------------------------------------------------------------------
// Startup/init logic for the 'legals' screen
//------------------------------------------------------------------------------------------------------------------------------------------
void START_Legals() noexcept {
    I_PurgeTexCache();
    I_LoadAndCacheTexLump(gTex_LEGALS, "LEGALS", 0);

    S_StartSound(nullptr, sfx_sgcock);
    gTitleScreenSpriteY = SCREEN_H;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown logic for the 'legals' screen
//------------------------------------------------------------------------------------------------------------------------------------------
void STOP_Legals([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // PsyDoom: if quitting the app then exit immediately, don't play any sounds etc.
    #if PSYDOOM_MODS
        if (Input::isQuitRequested())
            return;
    #endif

    S_StartSound(nullptr, sfx_barexp);

    #if PSYDOOM_MODS
        I_PreCrossfade();
    #endif

    I_CrossfadeFrameBuffers();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the 'legals' screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t TIC_Legals() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0) {
            gbKeepInputEvents = true;   // Don't consume 'key pressed' etc. events yet, not ticking...
            return ga_nothing;
        }
    #endif

    // Scroll the legal text, otherwise check for timeout
    if (gTitleScreenSpriteY > 0) {
        gTitleScreenSpriteY--;

        if (gTitleScreenSpriteY == 0) {
            gMenuTimeoutStartTicCon = gTicCon;
        }
    } else {
        // Must hold the legals text for a small amount of time before allowing skip (via a button press) or timeout
        const int32_t waitTicsElapsed = gTicCon - gMenuTimeoutStartTicCon;

        if (waitTicsElapsed > 120) {
            if (waitTicsElapsed >= 180)
                return ga_timeout;

            // PsyDoom: only accept main menu buttons to skip
            #if PSYDOOM_MODS
                if (gTickInputs[0].bMenuOk || gTickInputs[0].bMenuStart || gTickInputs[0].bMenuBack)
                    return ga_exit;
            #else
                if (gTicButtons[0] != 0)
                    return ga_exit;
            #endif
        }
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does drawing for the 'legals' screen - very simple, just a single sprite
//------------------------------------------------------------------------------------------------------------------------------------------
void DRAW_Legals() noexcept {
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    I_CacheAndDrawSprite(gTex_LEGALS, 0, (int16_t) gTitleScreenSpriteY, gPaletteClutIds[UIPAL]);

    I_SubmitGpuCmds();
    I_DrawPresent();
}
