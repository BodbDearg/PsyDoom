//------------------------------------------------------------------------------------------------------------------------------------------
// This is an entirely new menu added for PsyDoom.
// It simply shows a network error message with a single 'ok' option.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS

#include "neterror_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "m_main.h"
#include "o_main.h"
#include "PcPsx/Controls.h"
#include "PcPsx/Game.h"
#include "PcPsx/Input.h"
#include "PcPsx/Utils.h"

#include <sstream>
#include <string>
#include <vector>

// The error message the UI is initialized with
const char* gNetErrorMenuMsg = "";

// Individual lines in the network error message
static std::vector<std::string> gErrorLines;

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the cursor at the specified position
//------------------------------------------------------------------------------------------------------------------------------------------
static void DrawCursor(const int16_t cursorX, const int16_t cursorY) noexcept {
    I_DrawSprite(
        gTex_STATUS.texPageId,
        gPaletteClutIds[UIPAL],
        (int16_t) cursorX - 24,
        (int16_t) cursorY - 2,
        // PsyDoom: the STATUS texture atlas might not be at UV 0,0 anymore! (if limit removing, but always offset to be safe)
        #if PSYDOOM_MODS
            (int16_t)(gTex_STATUS.texPageCoordX + M_SKULL_TEX_U + (uint8_t) gCursorFrame * M_SKULL_W),
            (int16_t)(gTex_STATUS.texPageCoordY + M_SKULL_TEX_V),
        #else
            M_SKULL_TEX_U + (uint8_t) gCursorFrame * M_SKULL_W,
            M_SKULL_TEX_V,
        #endif
        M_SKULL_W,
        M_SKULL_H
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Show the network error menu for various different types of errors
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunNetErrorMenu_FailedToConnect() noexcept {
    return RunNetErrorMenu("Failed to connect!\nCheck network status.\nCheck network settings.");
}

gameaction_t RunNetErrorMenu_GameTypeOrVersionMismatch() noexcept {
    return RunNetErrorMenu("Failed to connect!\nGame version or game\ntype are mismatched.\nPsyDoom versions and\ngame discs must match!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convenience function that runs the network error menu with the given message
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunNetErrorMenu(const char* const msg) noexcept {
    gNetErrorMenuMsg = msg;
    return MiniLoop(NetError_Init, NetError_Shutdown, NetError_Update, NetError_Draw);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the network error menu
//------------------------------------------------------------------------------------------------------------------------------------------
void NetError_Init() noexcept {
    Input::consumeEvents();
    S_StartSound(nullptr, sfx_firxpl);
    gCursorFrame = 0;

    // Split up the error message into lines
    {
        gErrorLines.clear();
        std::stringstream errorMsgStream(gNetErrorMenuMsg);
        std::string errorLine;

        while (std::getline(errorMsgStream, errorLine, '\n')) {
            gErrorLines.push_back(errorLine);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the network error menu
//------------------------------------------------------------------------------------------------------------------------------------------
void NetError_Shutdown([[maybe_unused]] const gameaction_t exitAction) noexcept {
    gErrorLines.clear();
    gErrorLines.shrink_to_fit();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs update logic for the network error menu: does menu controls
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t NetError_Update() noexcept {
    // Gather menu inputs and exit if the back button has just been pressed
    const bool bMenuBack = Controls::isJustPressed(Controls::Binding::Menu_Back);
    const bool bMenuOk = Controls::isJustPressed(Controls::Binding::Menu_Ok);

    if (bMenuBack || bMenuOk) {
        S_StartSound(nullptr, sfx_pistol);
        return ga_exit;
    }

    // Only want to animate the skull cursor if ticks have elapsed: restricts the cursor to ticking at 30 Hz
    if (gPlayersElapsedVBlanks[0] <= 0) {
        gbKeepInputEvents = true;   // Don't consume 'key pressed' etc. events yet, not ticking...
        return ga_nothing;
    }

    // Animate the skull cursor
    if ((gGameTic > gPrevGameTic) && ((gGameTic & 3) == 0)) {
        gCursorFrame ^= 1;
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the network error menu
//------------------------------------------------------------------------------------------------------------------------------------------
void NetError_Draw() noexcept {
    // Increment the frame count for the texture cache and draw the background using the 'MARB01' sprite
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    {
        const uint16_t bgPaletteClutId = Game::getTexPalette_OptionsBg();

        for (int16_t y = 0; y < 4; ++y) {
            for (int16_t x = 0; x < 4; ++x) {
                I_CacheAndDrawSprite(gTex_OptionsBg, x * 64, y * 64, bgPaletteClutId);
            }
        }
    }

    // Don't do any rendering if we are about to exit the menu
    if (gGameAction == ga_nothing) {
        // Menu title
        I_DrawString(-1, 20, "Network Error!");

        // Draw the error message lines
        int32_t yPos = 60;

        for (const std::string& msgLine : gErrorLines) {
            I_DrawString(-1, yPos, msgLine.c_str());
            yPos += 24;
        }

        // Draw the exit option and skull cursor
        I_DrawString(116, 200, "Ok");
        DrawCursor(116, 200);
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

#endif  // // #if PSYDOOM_MODS
