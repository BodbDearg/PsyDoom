#include "o_main.h"

#include "cn_main.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "m_main.h"
#include "pw_main.h"
#include "Wess/psxspu.h"

// Available options and their names
enum option_t : int32_t {
    opt_music,
    opt_sound,
    opt_password,
    opt_config,
    opt_main_menu,
    opt_restart
};

const char gOptionNames[][16] = {
    { "Music Volume"    },
    { "Sound Volume"    },
    { "Password"        },
    { "Configuration"   },
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
    { opt_config,       62, 170 },
    { opt_main_menu,    62, 195 },
};

static const menuitem_t gOptMenuItems_Single[] = {
    { opt_music,        62, 50  },
    { opt_sound,        62, 90  },
    { opt_password,     62, 130 },
    { opt_config,       62, 155 },
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

// The marble floor texture used as a background for the options menu
const VmPtr<texture_t> gTex_MARB01(0x80097AB0);

// Current options music and sound volume
int32_t     gOptionsSndVol = 100;           // TODO: make constant
int32_t     gOptionsMusVol = 100;           // TODO: make constant

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the options menu
//------------------------------------------------------------------------------------------------------------------------------------------
void O_Init() noexcept {
    // BAM
    S_StartSound(nullptr, sfx_pistol);

    // Initialize cursor position and vblanks until move for all players
    gCursorFrame = 0;

    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        gCursorPos[playerIdx] = 0;
        gVBlanksUntilMenuMove[playerIdx] = 0;
    }

    // Set what menu layout to use
    if (gNetGame != gt_single) {
        gpOptionsMenuItems = gOptMenuItems_NetGame;
        gOptionsMenuSize = 4;
    }
    else if (gbGamePaused) {
        gpOptionsMenuItems = gOptMenuItems_Single;
        gOptionsMenuSize = 6;
    }
    else {
        gpOptionsMenuItems = gOptMenuItems_MainMenu;
        gOptionsMenuSize = 5;
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
    // Animate the skull cursor
    if ((gGameTic > gPrevGameTic) && ((gGameTic & 3) == 0)) {
        gCursorFrame ^= 1;
    }

    // Do options menu controls for all players
    for (int32_t playerIdx = MAXPLAYERS - 1; playerIdx >= 0; --playerIdx) {
        // Ignore this player if not in the game
        if (!gbPlayerInGame[playerIdx])
            continue;

        // Exit the menu if start or select is pressed
        const padbuttons_t ticButtons = gTicButtons[playerIdx];
        const padbuttons_t oldTicButtons = gOldTicButtons[playerIdx];

        // Allow the start or select buttons to close the menu
        if ((ticButtons != oldTicButtons) && (ticButtons & (PAD_START | PAD_SELECT))) {
            S_StartSound(nullptr, sfx_pistol);
            return ga_exit;
        }
        
        // Check for up/down movement
        if ((ticButtons & PAD_DIRECTION_BTNS) == 0) {
            // If there are no direction buttons pressed then the next move is allowed instantly
            gVBlanksUntilMenuMove[playerIdx] = 0;
        } else {
            // Direction buttons pressed or held down, check to see if we can an up/down move now
            gVBlanksUntilMenuMove[playerIdx] -= gPlayersElapsedVBlanks[0];

            if (gVBlanksUntilMenuMove[playerIdx] <= 0) {
                gVBlanksUntilMenuMove[playerIdx] = 15;
                
                if (ticButtons & PAD_DOWN) {
                    gCursorPos[playerIdx]++;

                    if (gCursorPos[playerIdx] >= gOptionsMenuSize) {
                        gCursorPos[playerIdx] = 0;
                    }

                    // Note: only play sound for this user's player!
                    if (playerIdx == gCurPlayerIndex) {
                        S_StartSound(nullptr, sfx_pstop);
                    }
                }
                else if (ticButtons & PAD_UP) {
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
                    if (ticButtons & PAD_RIGHT) {
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
                    else if (ticButtons & PAD_LEFT) {
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
            }   break;

            // Change sound volume
            case opt_sound: {
                // Only process audio updates for this player
                if (playerIdx == gCurPlayerIndex) {
                    if (ticButtons & PAD_RIGHT) {
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
                    else if (ticButtons & PAD_LEFT) {
                        gOptionsSndVol -= 1;

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
            }   break;

            // Password entry
            case opt_password: {
                if (ticButtons & PAD_ACTION_BTNS) {
                    if (MiniLoop(START_PasswordScreen, STOP_PasswordScreen, TIC_PasswordScreen, DRAW_PasswordScreen) == ga_warped)
                        return ga_warped;
                }
            }   break;

            // Controller configuration
            case opt_config: {
                if (ticButtons & PAD_ACTION_BTNS) {
                    MiniLoop(START_ControlsScreen, STOP_ControlsScreen, TIC_ControlsScreen, DRAW_ControlsScreen);
                }
            }   break;

            // Main menu option
            case opt_main_menu: {
                if (ticButtons & PAD_ACTION_BTNS) {
                    S_StartSound(nullptr, sfx_pistol);
                    return ga_exitdemo;
                }
            }   break;

            // Restart option
            case opt_restart: {
                if (ticButtons & PAD_ACTION_BTNS) {
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

    for (int16_t y = 0; y < 4; ++y) {
        for (int16_t x = 0; x < 4; ++x) {
            I_CacheAndDrawSprite(*gTex_MARB01, x * 64, y * 64, gPaletteClutIds[MAINPAL]);
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
                    gTex_STATUS->texPageId,
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
                    gTex_STATUS->texPageId,
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
            gTex_STATUS->texPageId,
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
