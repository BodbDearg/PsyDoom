#include "o_main.h"

#include "controls_main.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "m_main.h"
#include "PcPsx/Game.h"
#include "PcPsx/PlayerPrefs.h"
#include "PcPsx/PsxPadButtons.h"
#include "PcPsx/Utils.h"
#include "pw_main.h"
#include "Wess/psxspu.h"

// Available options and their names
enum option_t : int32_t {
    opt_music,
    opt_sound,
    opt_password,
// PsyDoom: Removing the psx controller configuration menu and replacing with 'controls'
#if PSYDOOM_MODS
    opt_controls,
#else
    opt_config,
#endif
    opt_main_menu,
    opt_restart
};

const char gOptionNames[][16] = {
    { "Music Volume"    },
    { "Sound Volume"    },
    { "Password"        },
// PsyDoom: Removing the psx controller configuration menu and replacing with 'controls'
#if PSYDOOM_MODS
    { "Controls"   },
#else
    { "Configuration"   },
#endif
    { "Main Menu"       },
    { "Restart Level"   }
};

// The layout for the options menu: outside of gameplay, in gameplay (single player) and in gameplay (multiplayer)
struct menuitem_t {
    option_t    option;
    int32_t     x;
    int32_t     y;
};

static const menuitem_t gOptMenuItems_MainMenu[] = {
    { opt_music,        62, 65  },
    { opt_sound,        62, 105 },
    { opt_password,     62, 145 },
// PsyDoom: Removing the psx controller configuration menu and replacing with 'controls'
#if PSYDOOM_MODS
    { opt_controls,     62, 170 },
#else
    { opt_config,       62, 170 },
#endif
    { opt_main_menu,    62, 195 },

};

static const menuitem_t gOptMenuItems_Single[] = {
    { opt_music,        62, 50  },
    { opt_sound,        62, 90  },
    { opt_password,     62, 130 },
// PsyDoom: Removing the psx controller configuration menu
#if PSYDOOM_MODS
    { opt_controls,     62, 155 },
#else
    { opt_config,       62, 155 },
#endif
    { opt_main_menu,    62, 180 },
    { opt_restart,      62, 205 },
};

static const menuitem_t gOptMenuItems_NetGame[] = {
    { opt_music,        62, 70  },
    { opt_sound,        62, 110 },
    { opt_main_menu,    62, 150 },
    { opt_restart,      62, 175 },
};

// Currently in-use options menu layout: items list and size
static int32_t              gOptionsMenuSize;
static const menuitem_t*    gpOptionsMenuItems;

// Texture used as a background for the options menu
texture_t gTex_OptionsBg;

// Current options music and sound volume
int32_t gOptionsSndVol = S_SND_DEFAULT_VOL;
int32_t gOptionsMusVol = S_MUS_DEFAULT_VOL;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the options menu
//------------------------------------------------------------------------------------------------------------------------------------------
void O_Init() noexcept {
    S_StartSound(nullptr, sfx_pistol);      // Bam!

    // Initialize cursor position and vblanks until move for all players
    gCursorFrame = 0;

    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        gCursorPos[playerIdx] = 0;
        gVBlanksUntilMenuMove[playerIdx] = 0;
    }

    // Set what menu layout to use
    if (gNetGame != gt_single) {
        gpOptionsMenuItems = gOptMenuItems_NetGame;
        gOptionsMenuSize = C_ARRAY_SIZE(gOptMenuItems_NetGame);
    }
    else if (gbGamePaused) {
        gpOptionsMenuItems = gOptMenuItems_Single;
        gOptionsMenuSize = C_ARRAY_SIZE(gOptMenuItems_Single);
    }
    else {
        gpOptionsMenuItems = gOptMenuItems_MainMenu;
        gOptionsMenuSize = C_ARRAY_SIZE(gOptMenuItems_MainMenu);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the options menu
//------------------------------------------------------------------------------------------------------------------------------------------
void O_Shutdown([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // Reset the cursor position for all players
    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        gCursorPos[playerIdx] = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Runs update logic for the options menu: does menu controls
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t O_Control() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0)
            return ga_nothing;
    #endif

    // Animate the skull cursor
    if ((gGameTic > gPrevGameTic) && ((gGameTic & 3) == 0)) {
        gCursorFrame ^= 1;
    }

    // Do options menu controls for all players
    for (int32_t playerIdx = MAXPLAYERS - 1; playerIdx >= 0; --playerIdx) {
        // Ignore this player if not in the game
        if (!gbPlayerInGame[playerIdx])
            continue;

        // Exit the menu if start or select is pressed.
        // Note: checking for buttons newly pressed (as opposed to just pressed) here is a change incorporated from Final Doom.
        // It's required due to some changes made in other UIs like the password screen.
        #if PSYDOOM_MODS
            const TickInputs& inputs = gTickInputs[playerIdx];
            const TickInputs& oldInputs = gOldTickInputs[playerIdx];

            const bool bMenuStart = (inputs.bMenuStart && (!oldInputs.bMenuStart));
            const bool bMenuBack = (inputs.bMenuBack && (!oldInputs.bMenuBack));
            const bool bMenuOk = (inputs.bMenuOk && (!oldInputs.bMenuOk));
            const bool bMenuUp = inputs.bMenuUp;
            const bool bMenuDown = inputs.bMenuDown;
            const bool bMenuLeft = inputs.bMenuLeft;
            const bool bMenuRight = inputs.bMenuRight;
            const bool bMenuMove = (bMenuUp || bMenuDown || bMenuLeft || bMenuRight);
        #else
            const padbuttons_t ticButtons = gTicButtons[playerIdx];
            const padbuttons_t oldTicButtons = gOldTicButtons[playerIdx];

            const bool bMenuStart = ((ticButtons != oldTicButtons) && (ticButtons & PAD_START));
            const bool bMenuBack = ((ticButtons != oldTicButtons) && (ticButtons & PAD_SELECT));
            const bool bMenuOk = (ticButtons & PAD_ACTION_BTNS);
            const bool bMenuUp = (ticButtons & PAD_UP);
            const bool bMenuDown = (ticButtons & PAD_DOWN);
            const bool bMenuLeft = (ticButtons & PAD_LEFT);
            const bool bMenuRight = (ticButtons & PAD_RIGHT);
            const bool bMenuMove = (ticButtons & PAD_DIRECTION_BTNS);
        #endif

        // Allow the start or select buttons to close the menu
        if (bMenuStart || bMenuBack) {
            S_StartSound(nullptr, sfx_pistol);
            return ga_exit;
        }
        
        // Check for up/down movement
        if (!bMenuMove) {
            // If there are no direction buttons pressed then the next move is allowed instantly
            gVBlanksUntilMenuMove[playerIdx] = 0;
        } else {
            // Direction buttons pressed or held down, check to see if we can an up/down move now
            gVBlanksUntilMenuMove[playerIdx] -= gPlayersElapsedVBlanks[0];

            if (gVBlanksUntilMenuMove[playerIdx] <= 0) {
                gVBlanksUntilMenuMove[playerIdx] = 15;
                
                if (bMenuDown) {
                    gCursorPos[playerIdx]++;

                    if (gCursorPos[playerIdx] >= gOptionsMenuSize) {
                        gCursorPos[playerIdx] = 0;
                    }

                    // Note: only play sound for this user's player!
                    if (playerIdx == gCurPlayerIndex) {
                        S_StartSound(nullptr, sfx_pstop);
                    }
                }
                else if (bMenuUp) {
                    gCursorPos[playerIdx]--;

                    if (gCursorPos[playerIdx] < 0) {
                        gCursorPos[playerIdx] = gOptionsMenuSize - 1;
                    }

                    // Note: only play sound for this user's player!
                    if (playerIdx == gCurPlayerIndex) {
                        S_StartSound(nullptr, sfx_pstop);
                    }
                }
            }
        }

        // Handle option actions and adjustment
        const option_t option = gpOptionsMenuItems[gCursorPos[playerIdx]].option;

        switch (option) {
            // Change music volume
            case opt_music: {
                // Only process audio updates for this player
                if (playerIdx == gCurPlayerIndex) {
                    if (bMenuRight) {
                        gOptionsMusVol++;
                        
                        if (gOptionsMusVol > S_MAX_VOL) {
                            gOptionsMusVol = S_MAX_VOL;
                        } else {
                            S_SetMusicVolume(doomToWessVol(gOptionsMusVol));

                            if (gOptionsMusVol & 1) {
                                S_StartSound(nullptr, sfx_stnmov);
                            }
                        }
                        
                        gCdMusicVol = (gOptionsMusVol * PSXSPU_MAX_CD_VOL) / S_MAX_VOL;
                    }
                    else if (bMenuLeft) {
                        gOptionsMusVol--;

                        if (gOptionsMusVol < 0) {
                            gOptionsMusVol = 0;
                        } else {
                            S_SetMusicVolume(doomToWessVol(gOptionsMusVol));

                            if (gOptionsMusVol & 1) {
                                S_StartSound(nullptr, sfx_stnmov);
                            }
                        }
                        
                        gCdMusicVol = (gOptionsMusVol * PSXSPU_MAX_CD_VOL) / S_MAX_VOL;
                    }
                }

                // PsyDoom: update the volume levels in the preferences module also, whenever they are edited in the options menu.
                // Want to save the correct values upon exiting the game.
                #if PSYDOOM_MODS
                    PlayerPrefs::pullSoundAndMusicPrefs();
                #endif
            }   break;

            // Change sound volume
            case opt_sound: {
                // Only process audio updates for this player
                if (playerIdx == gCurPlayerIndex) {
                    if (bMenuRight) {
                        gOptionsSndVol++;

                        if (gOptionsSndVol > S_MAX_VOL) {
                            gOptionsSndVol = S_MAX_VOL;
                        } else {
                            S_SetSfxVolume(doomToWessVol(gOptionsSndVol));
                            
                            if (gOptionsSndVol & 1) {
                                S_StartSound(nullptr, sfx_stnmov);
                            }
                        }
                    }
                    else if (bMenuLeft) {
                        gOptionsSndVol--;

                        if (gOptionsSndVol < 0) {
                            gOptionsSndVol = 0;
                        } else {
                            S_SetSfxVolume(doomToWessVol(gOptionsSndVol));
                            
                            if (gOptionsSndVol & 1) {
                                S_StartSound(nullptr, sfx_stnmov);
                            }
                        }
                    }
                }

                // PsyDoom: update the volume levels in the preferences module also, whenever they are edited in the options menu.
                // Want to save the correct values upon exiting the game.
                #if PSYDOOM_MODS
                    PlayerPrefs::pullSoundAndMusicPrefs();
                #endif
            }   break;

            // Password entry
            case opt_password: {
                if (bMenuOk) {
                    if (MiniLoop(START_PasswordScreen, STOP_PasswordScreen, TIC_PasswordScreen, DRAW_PasswordScreen) == ga_warped)
                        return ga_warped;
                }
            }   break;

            // PsyDoom: Adding a new 'controls' menu
            case opt_controls: {
                if (bMenuOk) {
                    MiniLoop(Controls_Init, Controls_Shutdown, Controls_Update, Controls_Draw);
                }
            }   break;

        // PsyDoom: Removing the psx controller configuration menu
        #if !PSYDOOM_MODS
            // Controller configuration
            case opt_config: {
                if (bMenuOk) {
                    MiniLoop(START_ControlsScreen, STOP_ControlsScreen, TIC_ControlsScreen, DRAW_ControlsScreen);
                }
            }   break;
        #endif

            // Main menu option
            case opt_main_menu: {
                if (bMenuOk) {
                    S_StartSound(nullptr, sfx_pistol);
                    return ga_exitdemo;
                }
            }   break;

            // Restart option
            case opt_restart: {
                if (bMenuOk) {
                    S_StartSound(nullptr, sfx_pistol);
                    return ga_restart;
                }
            }   break;
        }
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the options menu
//------------------------------------------------------------------------------------------------------------------------------------------
void O_Drawer() noexcept {
    // Increment the frame count for the texture cache and draw the background using the 'MARB01' sprite
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    // Draw the options screen background
    {
        const uint16_t bgPaletteClutId = Game::getTexPalette_OptionsBg();
        texture_t& bgTex = gTex_OptionsBg;

        for (int16_t y = 0; y < 4; ++y) {
            for (int16_t x = 0; x < 4; ++x) {
                // PsyDoom: restrict these tiles to being drawn at a maximum size of 64x64.
                // This helps to prevent slight seams between the tiles when using MSAA with the Final Doom options menu.
                //
                // The Final Doom background tile unfortunately has some black/empty rows and columns at the edges of the texture,
                // and when MSAA is enabled these tend to work their way into the rendered image as seams. Restricting the draw area
                // of the sprite to a maximum of 64x64 avoids including these unwanted pixels when MSAA is on.
                #if PSYDOOM_MODS
                    I_CacheTex(bgTex);
                    I_DrawSprite(
                        bgTex.texPageId,
                        bgPaletteClutId,
                        x * 64,
                        y * 64,
                        bgTex.texPageCoordX,
                        bgTex.texPageCoordY,
                        std::min((uint16_t) bgTex.width, uint16_t(64)),
                        std::min((uint16_t) bgTex.height, uint16_t(64))
                    );
                #else
                    I_CacheAndDrawSprite(bgTex, x * 64, y * 64, bgPaletteClutId);
                #endif
            }
        }
    }

    // Don't do any rendering if we are about to exit the menu
    if (gGameAction == ga_nothing) {
        // Menu title
        I_DrawString(-1, 20, "Options");

        // Draw each menu item for the current options screen layout.
        // The available options will vary depending on game mode.
        const menuitem_t* pMenuItem = gpOptionsMenuItems;
        
        for (int32_t optIdx = 0; optIdx < gOptionsMenuSize; ++optIdx, ++pMenuItem) {
            // Draw the option label
            I_DrawString(pMenuItem->x, pMenuItem->y, gOptionNames[pMenuItem->option]);

            // If the option has a slider associated with it, draw that too
            if (pMenuItem->option <= opt_sound) {
                // Draw the slider backing/container
                I_DrawSprite(
                    gTex_STATUS.texPageId,
                    gPaletteClutIds[UIPAL],
                    (int16_t) pMenuItem->x + 13,
                    (int16_t) pMenuItem->y + 20,
                    0,
                    184,
                    108,
                    11
                );

                // Draw the slider handle
                const int32_t sliderVal = (pMenuItem->option == opt_sound) ? gOptionsSndVol : gOptionsMusVol;

                I_DrawSprite(
                    gTex_STATUS.texPageId,
                    gPaletteClutIds[UIPAL],
                    (int16_t)(pMenuItem->x + 14 + sliderVal),
                    (int16_t)(pMenuItem->y + 20),
                    108,
                    184,
                    6,
                    11
                );
            }
        }

        // Draw the skull cursor
        const int32_t cursorPos = gCursorPos[gCurPlayerIndex];
        const menuitem_t& menuItem = gpOptionsMenuItems[cursorPos];

        I_DrawSprite(
            gTex_STATUS.texPageId,
            gPaletteClutIds[UIPAL],
            (int16_t) menuItem.x - 24,
            (int16_t) menuItem.y - 2,
            M_SKULL_TEX_U + (uint8_t) gCursorFrame * M_SKULL_W,
            M_SKULL_TEX_V,
            M_SKULL_W,
            M_SKULL_H
        );
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}
