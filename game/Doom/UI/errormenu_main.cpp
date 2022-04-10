//------------------------------------------------------------------------------------------------------------------------------------------
// This is an entirely new menu added for PsyDoom.
// It simply shows a network error message with a single 'ok' option.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS

#include "errormenu_main.h"

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
#include "PsyDoom/Controls.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/ProgArgs.h"
#include "PsyDoom/Utils.h"

#include <sstream>
#include <string>
#include <vector>

// The title and error message the UI is initialized with
static std::string gErrorMenuTitle;
static std::string gErrorMenuMsg;

// Individual lines in the error message
static std::vector<std::string> gErrorLines;

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the cursor at the specified position
//------------------------------------------------------------------------------------------------------------------------------------------
static void DrawCursor(const int16_t cursorX, const int16_t cursorY) noexcept {
    I_DrawSprite(
        gTex_STATUS.texPageId,
        Game::getTexPalette_STATUS(),
        (int16_t) cursorX - 24,
        (int16_t) cursorY - 2,
        (int16_t)(gTex_STATUS.texPageCoordX + M_SKULL_TEX_U + (uint8_t) gCursorFrame * M_SKULL_W),
        (int16_t)(gTex_STATUS.texPageCoordY + M_SKULL_TEX_V),
        M_SKULL_W,
        M_SKULL_H
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Show a network error menu for various different types of errors
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunNetErrorMenu(const char* const msg) noexcept {
    gErrorMenuTitle = "Network Error!";
    gErrorMenuMsg = msg;
    return MiniLoop(ErrorMenu_Init, ErrorMenu_Shutdown, ErrorMenu_Update, ErrorMenu_Draw);
}

gameaction_t RunNetErrorMenu_FailedToConnect() noexcept {
    return RunNetErrorMenu("Failed to connect!\nCheck network status.\nCheck network settings.");
}

gameaction_t RunNetErrorMenu_GameTypeOrVersionMismatch() noexcept {
    return RunNetErrorMenu("Failed to connect!\nGame version or game\ntype are mismatched.\nPsyDoom versions and\ngame discs must match!");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Show an error menu for various different types of 'load game' errors
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunLoadGameErrorMenu(const char* const msg) noexcept {
    gErrorMenuTitle = "Load Failed!";
    gErrorMenuMsg = msg;
    return MiniLoop(ErrorMenu_Init, ErrorMenu_Shutdown, ErrorMenu_Update, ErrorMenu_Draw);
}

gameaction_t RunLoadGameErrorMenu_BadFileId() noexcept {
    return RunLoadGameErrorMenu("Bad save file ID!\nNot a valid PsyDoom\nsave!");
}

gameaction_t RunLoadGameErrorMenu_BadFileVersion() noexcept {
    return RunLoadGameErrorMenu("Bad save file version!\nSave is from an\nincompatible version\nof PsyDoom!");
}

gameaction_t RunLoadGameErrorMenu_BadMapNum() noexcept {
    return RunLoadGameErrorMenu("Bad save file map num!\nMap does not exist\nin this game!");
}

gameaction_t RunLoadGameErrorMenu_IOError() noexcept {
    return RunLoadGameErrorMenu("An IO error occurred!\nData may be corrupt or\ntruncated or disk\nmay be faulty.");
}

gameaction_t RunLoadGameErrorMenu_BadMapHash() noexcept {
    RunLoadGameErrorMenu("Bad map hash!\nThe map data has\nchanged since the\nsave was made.");

    // Restart the map as the game is broken at this point (map only partially setup).
    // At least this way the player will get to play the save file map using a pistol start...
    return ga_restart;
}

gameaction_t RunLoadGameErrorMenu_BadMapData() noexcept {
    RunLoadGameErrorMenu("Bad save data!\nSome parts of the\nsave data failed\nvalidation checks.");

    // Restart the map as the game is broken at this point (map only partially setup).
    // At least this way the player will get to play the save file map using a pistol start...
    return ga_restart;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Show an error menu for various different types of demo playback errors.
// Note that if the game is playing back a demo in headless mode then the calls to run these menus will be ignored.
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunDemoErrorMenu(const char* const msg) noexcept {
    if (!ProgArgs::gbHeadlessMode) {
        gErrorMenuTitle = "Demo Error!";
        gErrorMenuMsg = msg;
        return MiniLoop(ErrorMenu_Init, ErrorMenu_Shutdown, ErrorMenu_Update, ErrorMenu_Draw);
    } else {
        return ga_exit;
    }
}

gameaction_t RunDemoErrorMenu_UnexpectedEOF() noexcept {
    return RunDemoErrorMenu("Unexpected EOF!\nCorrupt demo file!");
}

gameaction_t RunDemoErrorMenu_InvalidDemoVersion() noexcept {
    return RunDemoErrorMenu("Wrong demo format!\nThis demo file is\nfor another\nversion of PsyDoom!");
}

gameaction_t RunDemoErrorMenu_InvalidSkill() noexcept {
    return RunDemoErrorMenu("Invalid skill!");
}

gameaction_t RunDemoErrorMenu_InvalidMapNumber() noexcept {
    return RunDemoErrorMenu("Invalid map!");
}

gameaction_t RunDemoErrorMenu_InvalidGameType() noexcept {
    return RunDemoErrorMenu("Bad game type!");
}

gameaction_t RunDemoErrorMenu_InvalidPlayerNum() noexcept {
    return RunDemoErrorMenu("Bad player num!");
}

gameaction_t RunDemoErrorMenu_BadMapHash() noexcept {
    return RunLoadGameErrorMenu("Bad map hash!\nThe map data has\nchanged since the\ndemo was made.");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the error menu
//------------------------------------------------------------------------------------------------------------------------------------------
void ErrorMenu_Init() noexcept {
    // Consume all input events, setup the cursor and play the intro sound (fireball explode)
    Input::consumeEvents();
    S_StartSound(nullptr, sfx_firxpl);
    gCursorFrame = 0;

    // This lump needs to be cached for the error menu!
    I_LoadAndCacheTexLump(gTex_OptionsBg, Game::getTexLumpName_OptionsBg().c_str().data(), 0);

    // Split up the error message into lines
    {
        gErrorLines.clear();
        std::stringstream errorMsgStream(gErrorMenuMsg);
        std::string errorLine;

        while (std::getline(errorMsgStream, errorLine, '\n')) {
            gErrorLines.push_back(errorLine);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the error menu
//------------------------------------------------------------------------------------------------------------------------------------------
void ErrorMenu_Shutdown([[maybe_unused]] const gameaction_t exitAction) noexcept {
    gErrorLines.clear();
    gErrorLines.shrink_to_fit();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs update logic for the error menu: does menu controls
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t ErrorMenu_Update() noexcept {
    // Gather menu inputs and exit if the back button has just been pressed
    const bool bMenuBack = Controls::isJustPressed(Controls::Binding::Menu_Back);
    const bool bMenuOk = Controls::isJustPressed(Controls::Binding::Menu_Ok);

    if (bMenuBack || bMenuOk) {
        S_StartSound(nullptr, sfx_itmbk);
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
// Draws the error menu
//------------------------------------------------------------------------------------------------------------------------------------------
void ErrorMenu_Draw() noexcept {
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
        I_DrawString(-1, 20, gErrorMenuTitle.c_str());

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

    // PsyDoom: draw any enabled performance counters
    #if PSYDOOM_MODS
        I_DrawEnabledPerfCounters();
    #endif

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

#endif  // #if PSYDOOM_MODS
