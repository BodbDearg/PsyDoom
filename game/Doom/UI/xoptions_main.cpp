//------------------------------------------------------------------------------------------------------------------------------------------
// This is an entirely new menu added for PsyDoom.
// It provides extra options for turn sensitivity, autorun and renderer etc.
// It is not available in multiplayer, similar to other nested menus in the options screen.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS

#include "xoptions_main.h"

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
#include "PsyDoom/Game.h"
#include "PsyDoom/PlayerPrefs.h"
#include "PsyDoom/Utils.h"
#include "PsyDoom/Video.h"
#include "PsyDoom/Vulkan/VRenderer.h"

#include <cstdio>

// The available menu items
enum MenuItem : uint32_t {
    menu_turn_speed,
    menu_always_run,
    menu_stat_display,
    menu_renderer,
    menu_exit,
    num_menu_items
};

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
// Initializes the menu
//------------------------------------------------------------------------------------------------------------------------------------------
void XOptions_Init() noexcept {
    S_StartSound(nullptr, sfx_pistol);

    // Initialize cursor position and vblanks until move
    gCursorFrame = 0;
    gCursorPos[gCurPlayerIndex] = 0;
    gVBlanksUntilMenuMove[gCurPlayerIndex] = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the menu
//------------------------------------------------------------------------------------------------------------------------------------------
void XOptions_Shutdown([[maybe_unused]] const gameaction_t exitAction) noexcept {
    gCursorPos[gCurPlayerIndex] = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs update logic for the menu: does menu controls
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t XOptions_Update() noexcept {
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
    const bool bMenuLeft = inputs.bMenuLeft;
    const bool bMenuRight = inputs.bMenuRight;
    const bool bMenuMove = (bMenuUp || bMenuDown || bMenuLeft || bMenuRight);

    if (bMenuBack) {
        S_StartSound(nullptr, sfx_pistol);
        return ga_exit;
    }

    // Check for up/down movement
    if (!bMenuMove) {
        // If there are no direction buttons pressed then the next move is allowed instantly
        gVBlanksUntilMenuMove[playerIdx] = 0;
    } else {
        // Direction buttons pressed or held down, check to see if we can an up/down move now
        gVBlanksUntilMenuMove[playerIdx] -= gPlayersElapsedVBlanks[playerIdx];

        if (gVBlanksUntilMenuMove[playerIdx] <= 0) {
            gVBlanksUntilMenuMove[playerIdx] = 15;

            if (bMenuDown) {
                gCursorPos[playerIdx]++;

                if (gCursorPos[playerIdx] >= num_menu_items) {
                    gCursorPos[playerIdx] = 0;
                }

                S_StartSound(nullptr, sfx_pstop);
            }
            else if (bMenuUp) {
                gCursorPos[playerIdx]--;

                if (gCursorPos[playerIdx] < 0) {
                    gCursorPos[playerIdx] = num_menu_items - 1;
                }

                S_StartSound(nullptr, sfx_pstop);
            }
        }
    }

    // Handle option actions and adjustment
    switch ((MenuItem) gCursorPos[playerIdx]) {
        // Adjust turn speed
        case menu_turn_speed: {
            // Only process audio updates for this player
            if (bMenuRight) {
                PlayerPrefs::gTurnSpeedMult100++;

                if (PlayerPrefs::gTurnSpeedMult100 > PlayerPrefs::TURN_SPEED_MULT_MAX) {
                    PlayerPrefs::gTurnSpeedMult100 = PlayerPrefs::TURN_SPEED_MULT_MAX;
                } else {
                    if ((PlayerPrefs::gTurnSpeedMult100 / 4) & 1) {
                        S_StartSound(nullptr, sfx_stnmov);
                    }
                }
            }
            else if (bMenuLeft) {
                if (PlayerPrefs::gTurnSpeedMult100 > PlayerPrefs::TURN_SPEED_MULT_MIN) {
                    PlayerPrefs::gTurnSpeedMult100--;;

                    if ((PlayerPrefs::gTurnSpeedMult100 / 4) & 1) {
                        S_StartSound(nullptr, sfx_stnmov);
                    }
                } else {
                    PlayerPrefs::gTurnSpeedMult100 = PlayerPrefs::TURN_SPEED_MULT_MIN;
                }
            }
        }   break;

        // Turn on/off always run
        case menu_always_run: {
            if (bMenuLeft && PlayerPrefs::gbAlwaysRun) {
                PlayerPrefs::gbAlwaysRun = false;
                S_StartSound(nullptr, sfx_swtchx);
            }
            else if (bMenuRight && (!PlayerPrefs::gbAlwaysRun)) {
                PlayerPrefs::gbAlwaysRun = true;
                S_StartSound(nullptr, sfx_swtchx);
            }
        }   break;

        // Stat display setting
        case menu_stat_display: {
            if (bMenuLeft && (!oldInputs.bMenuLeft) && (PlayerPrefs::gStatDisplayMode > StatDisplayMode::None)) {
                PlayerPrefs::gStatDisplayMode = (StatDisplayMode)((int32_t) PlayerPrefs::gStatDisplayMode - 1);
                S_StartSound(nullptr, sfx_swtchx);
            }
            else if (bMenuRight && (!oldInputs.bMenuRight) && (PlayerPrefs::gStatDisplayMode < StatDisplayMode::KillsSecretsAndItems)) {
                PlayerPrefs::gStatDisplayMode = (StatDisplayMode)((int32_t) PlayerPrefs::gStatDisplayMode + 1);
                S_StartSound(nullptr, sfx_swtchx);
            }
        }   break;

        // Renderer toggle
        case menu_renderer: {
            const bool bCanSwitchRenderers = (Video::gBackendType == Video::BackendType::Vulkan);

            #if PSYDOOM_VULKAN_RENDERER
                if (bCanSwitchRenderers) {
                    if (bMenuLeft && (!Video::isUsingVulkanRenderPath())) {
                        VRenderer::switchToMainVulkanRenderPath();
                        S_StartSound(nullptr, sfx_swtchx);
                    }
                    else if (bMenuRight && Video::isUsingVulkanRenderPath()) {
                        VRenderer::switchToPsxRenderPath();
                        S_StartSound(nullptr, sfx_swtchx);
                    }
                }
            #endif

            // If renderer switch is not possible and an attempt was made to do so then play this sound
            if (!bCanSwitchRenderers) {
                if ((bMenuLeft && (!oldInputs.bMenuLeft)) || (bMenuRight && (!oldInputs.bMenuRight))) {
                    S_StartSound(nullptr, sfx_itemup);
                }
            }
        }   break;

        // Exit to the options menu
        case menu_exit: {
            if (bMenuOk) {
                S_StartSound(nullptr, sfx_pistol);
                return ga_exit;
            }
        } break;

        default:
            break;
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the menu
//------------------------------------------------------------------------------------------------------------------------------------------
void XOptions_Draw() noexcept {
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

    // Don't do any rendering if we are about to exit the menu
    if (gGameAction == ga_nothing) {
        // Menu title
        I_DrawString(-1, 20, "Extra Options");

        // Draw the turn speed slider
        int16_t cursorX = 62;
        int16_t cursorY = 50;

        {
            // Draw the label for the slider
            const int16_t menuItemX = 62;
            const int16_t menuItemY = 50;

            int32_t turnSpeed = PlayerPrefs::gTurnSpeedMult100;
            char turnSpeedLabel[32];

            std::snprintf(
                turnSpeedLabel,
                sizeof(turnSpeedLabel),
                "Turn Speed %d.%02d",
                turnSpeed / 100,
                turnSpeed % 100
            );

            I_DrawString(menuItemX, menuItemY, turnSpeedLabel);

            // Draw the slider background
            I_DrawSprite(
                gTex_STATUS.texPageId,
                gPaletteClutIds[UIPAL],
                (int16_t)(menuItemX + 13),
                (int16_t)(menuItemY + 20),
                // PsyDoom: the STATUS texture atlas might not be at UV 0,0 anymore! (if limit removing, but always offset to be safe)
                #if PSYDOOM_MODS
                    (int16_t)(gTex_STATUS.texPageCoordX + 0),
                    (int16_t)(gTex_STATUS.texPageCoordY + 184),
                #else
                    0,
                    184,
                #endif
                108,
                11
            );

            // Draw the slider handle
            const int16_t sliderVal = (int16_t)(PlayerPrefs::gTurnSpeedMult100 / 5);

            I_DrawSprite(
                gTex_STATUS.texPageId,
                gPaletteClutIds[UIPAL],
                (int16_t)(menuItemX + 14 + sliderVal),
                (int16_t)(menuItemY + 20),
                // PsyDoom: the STATUS texture atlas might not be at UV 0,0 anymore! (if limit removing, but always offset to be safe)
                #if PSYDOOM_MODS
                    (int16_t)(gTex_STATUS.texPageCoordX + 108),
                    (int16_t)(gTex_STATUS.texPageCoordY + 184),
                #else
                    108,
                    184,
                #endif
                6,
                11
            );
        }

        // Draw the always run option
        I_DrawString(62, 90, (PlayerPrefs::gbAlwaysRun) ? "Always Run On" : "Always Run Off");

        if (gCursorPos[gCurPlayerIndex] == menu_always_run) {
            cursorY = 90;
        }

        // Draw the stats display option
        const char* statDisplayStr = "Stat Display Off";

        if (PlayerPrefs::gStatDisplayMode >= StatDisplayMode::KillsSecretsAndItems) {
            statDisplayStr = "Stat Display KSI";
        } else if (PlayerPrefs::gStatDisplayMode >= StatDisplayMode::KillsAndSecrets) {
            statDisplayStr = "Stat Display KS";
        } else if (PlayerPrefs::gStatDisplayMode >= StatDisplayMode::Kills) {
            statDisplayStr = "Stat Display K";
        }

        I_DrawString(62, 115, statDisplayStr);

        if (gCursorPos[gCurPlayerIndex] == menu_stat_display) {
            cursorY = 115;
        }

        // Draw the renderer option
        const bool bIsUsingVulkan = Video::isUsingVulkanRenderPath();
        I_DrawString(62, 140, (bIsUsingVulkan) ? "Vulkan Renderer" : "Classic Renderer");

        if (gCursorPos[gCurPlayerIndex] == menu_renderer) {
            cursorY = 140;
        }

        // Draw the exit option
        I_DrawString(62, 205, "Exit");

        if (gCursorPos[gCurPlayerIndex] == menu_exit) {
            cursorY = 205;
        }

        // Draw the skull cursor
        DrawCursor(cursorX, cursorY);
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

#endif  // // #if PSYDOOM_MODS
