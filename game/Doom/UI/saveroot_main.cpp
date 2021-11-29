//------------------------------------------------------------------------------------------------------------------------------------------
// This is an entirely new menu added for PsyDoom.
// It is a root menu for loading, saving and password entry.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS

#include "saveroot_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "loadsave_main.h"
#include "m_main.h"
#include "o_main.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/Utils.h"
#include "pw_main.h"

// The available menu items
enum MenuItemType : uint32_t {
    menu_load,
    menu_save,
    menu_password,
    menu_back,
    num_menu_item_types
};

// Represents single item on the menu
struct MenuItem {
    MenuItemType    type;
    int16_t         x;
    int16_t         y;
    const char*     label;
};

// Menu layouts: main menu and in-game
static const MenuItem gMenuItems_MainMenu[] = {
    { menu_load,        62, 75,  "Load"     },
    { menu_password,    62, 100, "Password" },
    { menu_back,        62, 205, "Back"     },
};

static const MenuItem gMenuItems_InGame[] = {
    { menu_load,        62, 75,  "Load"     },
    { menu_save,        62, 100, "Save"     },
    { menu_password,    62, 125, "Password" },
    { menu_back,        62, 205, "Back"     },
};

// Currently in-use options menu layout: items list and size
static int32_t          gMenuSize;
static const MenuItem*  gpMenuItems;

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the cursor at the specified position
//------------------------------------------------------------------------------------------------------------------------------------------
static void DrawCursor(const int16_t cursorX, const int16_t cursorY) noexcept {
    I_DrawSprite(
        gTex_STATUS.texPageId,
        gPaletteClutIds[UIPAL],
        (int16_t) cursorX - 24,
        (int16_t) cursorY - 2,
        (int16_t)(gTex_STATUS.texPageCoordX + M_SKULL_TEX_U + (uint8_t) gCursorFrame * M_SKULL_W),
        (int16_t)(gTex_STATUS.texPageCoordY + M_SKULL_TEX_V),
        M_SKULL_W,
        M_SKULL_H
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the menu
//------------------------------------------------------------------------------------------------------------------------------------------
void SaveRoot_Init() noexcept {
    S_StartSound(nullptr, sfx_pistol);

    // Initialize cursor position and vblanks until move
    gCursorFrame = 0;
    gCursorPos[gCurPlayerIndex] = 0;
    gVBlanksUntilMenuMove[gCurPlayerIndex] = 0;

    // Set what menu layout to use
    if (gbGamePaused) {
        gpMenuItems = gMenuItems_InGame;
        gMenuSize = C_ARRAY_SIZE(gMenuItems_InGame);
    } else {
        gpMenuItems = gMenuItems_MainMenu;
        gMenuSize = C_ARRAY_SIZE(gMenuItems_MainMenu);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the menu
//------------------------------------------------------------------------------------------------------------------------------------------
void SaveRoot_Shutdown([[maybe_unused]] const gameaction_t exitAction) noexcept {
    gCursorPos[gCurPlayerIndex] = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs update logic for the menu: does menu controls
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t SaveRoot_Update() noexcept {
    // PsyDoom: in all UIs tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    const uint32_t playerIdx = gCurPlayerIndex;

    if (gPlayersElapsedVBlanks[playerIdx] <= 0) {
        gbKeepInputEvents = true;   // Don't consume 'key pressed' etc. events yet, not ticking...
        return ga_nothing;
    }

    // Animate the skull cursor
    if ((gGameTic > gPrevGameTic) && ((gGameTic & 3) == 0)) {
        gCursorFrame ^= 1;
    }

    // Gather menu inputs and exit if the back button has just been pressed
    const TickInputs& inputs = gTickInputs[playerIdx];
    const TickInputs& oldInputs = gOldTickInputs[playerIdx];

    const bool bMenuBack = (inputs.bMenuBack && (!oldInputs.bMenuBack));
    const bool bMenuOk = (inputs.bMenuOk && (!oldInputs.bMenuOk));
    const bool bMenuUp = inputs.bMenuUp;
    const bool bMenuDown = inputs.bMenuDown;
    const bool bMenuMove = (bMenuUp || bMenuDown);

    if (bMenuBack) {
        S_StartSound(nullptr, sfx_pistol);
        return ga_exit;
    }

    // Check for up/down movement
    if (!bMenuMove) {
        // If there are no direction buttons pressed then the next move is allowed instantly
        gVBlanksUntilMenuMove[playerIdx] = 0;
    } else {
        // Direction buttons pressed or held down, check to see if we can move up/down now
        gVBlanksUntilMenuMove[playerIdx] -= gPlayersElapsedVBlanks[playerIdx];

        if (gVBlanksUntilMenuMove[playerIdx] <= 0) {
            gVBlanksUntilMenuMove[playerIdx] = 15;

            if (bMenuDown) {
                gCursorPos[playerIdx]++;

                if (gCursorPos[playerIdx] >= gMenuSize) {
                    gCursorPos[playerIdx] = 0;
                }

                S_StartSound(nullptr, sfx_pstop);
            }
            else if (bMenuUp) {
                gCursorPos[playerIdx]--;

                if (gCursorPos[playerIdx] < 0) {
                    gCursorPos[playerIdx] = gMenuSize - 1;
                }

                S_StartSound(nullptr, sfx_pstop);
            }
        }
    }

    // Handle selecting menu items
    if (bMenuOk) {
        const int32_t cursorPos = gCursorPos[playerIdx];
        const MenuItem& menuItem = gpMenuItems[cursorPos];

        switch (menuItem.type) {
            // Load a save file
            case menu_load: {
                LoadSave_SetMode(LoadSaveMenuMode::Load);
                const gameaction_t action = MiniLoop(LoadSave_Init, LoadSave_Shutdown, LoadSave_Update, LoadSave_Draw);

                // Handle exiting all menus or warping to a new level
                if ((action == ga_warped) || (action == ga_exitmenus)) {
                    return action;
                }
            }   break;

            // Save to a save file and exit all menus if we do
            case menu_save: {
                LoadSave_SetMode(LoadSaveMenuMode::Save);

                if (MiniLoop(LoadSave_Init, LoadSave_Shutdown, LoadSave_Update, LoadSave_Draw) == ga_exitmenus) {
                    return ga_exitmenus;
                }
            }   break;

            // Password entry
            case menu_password: {
                if (MiniLoop(START_PasswordScreen, STOP_PasswordScreen, TIC_PasswordScreen, DRAW_PasswordScreen) == ga_warped) {
                    return ga_warped;
                }
            }   break;

            // Exit to the options menu
            case menu_back:
                S_StartSound(nullptr, sfx_pistol);
                return ga_exit;

            default:
                break;
        }
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the menu
//------------------------------------------------------------------------------------------------------------------------------------------
void SaveRoot_Draw() noexcept {
    // Increment the frame count for the texture cache and draw the background using the 'MARB01' sprite
    I_IncDrawnFrameCount();
    Utils::onBeginUIDrawing();      // PsyDoom: UI drawing setup for the new Vulkan renderer

    {
        const uint16_t bgPaletteClutId = Game::getTexPalette_OptionsBg();

        for (int16_t y = 0; y < 4; ++y) {
            for (int16_t x = 0; x < 4; ++x) {
                I_CacheAndDrawSprite(gTex_OptionsBg, x * 64, y * 64, bgPaletteClutId);
            }
        }
    }

    // Draw the menu title
    I_DrawString(-1, 20, "Load And Save");

    // Draw all the menu items
    for (int32_t i = 0; i < gMenuSize; ++i) {
        const MenuItem& menuItem = gpMenuItems[i];
        I_DrawString(menuItem.x, menuItem.y, menuItem.label);
    }

    // Draw the skull cursor
    {
        const int32_t cursorPos = gCursorPos[gCurPlayerIndex];
        const MenuItem& menuItem = gpMenuItems[cursorPos];
        DrawCursor(menuItem.x, menuItem.y);
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

#endif  // #if PSYDOOM_MODS
