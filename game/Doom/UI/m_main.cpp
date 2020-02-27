#include "m_main.h"

#include "Doom/Base/i_crossfade.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "o_main.h"
#include "PcPsx/Finally.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"

const VmPtr<texture_t>              gTex_BACK(0x80097A10);                  // The background texture for the main menu
const VmPtr<int32_t[MAXPLAYERS]>    gCursorPos(0x80078000);                 // Which of the menu options each player's cursor is over (see 'menu_t')
const VmPtr<int32_t>                gCursorFrame(0x800781D8);               // Current frame that the menu cursor is displaying
const VmPtr<int32_t>                gMenuTimeoutStartTicCon(0x80077F0C);    // Tick that we start checking for menu timeouts from

// Main menu options
enum menu_t : int32_t {
    gamemode,
    level,
    difficulty,
    options,
    NUMMENUITEMS
};

// The position of each main menu option
static const int16_t gMenuYPos[NUMMENUITEMS] = {
    91,     // gamemode
    133,    // level
    158,    // difficulty
    200     // options
};

// Game mode names and skill names
static const char gGameTypeNames[NUMGAMETYPES][16] = {
    "Single Player",
    "Cooperative",
    "Deathmatch"
};

static const char gSkillNames[NUMSKILLS][16] = {
    "I am a Wimp",
    "Not Too Rough",
    "Hurt Me Plenty",
    "Ultra Violence",
#if PC_PSX_DOOM_MODS    // PC-PSX: exposing the unused 'Nightmare' mode
    "Nightmare!"
#endif
};

// The texture for the DOOM logo
static const VmPtr<texture_t> gTex_DOOM(0x80097A50);

// Restricts what maps or episodes the player can pick
static const VmPtr<int32_t> gMaxStartEpisodeOrMap(0x8007817C);

//------------------------------------------------------------------------------------------------------------------------------------------
// Starts up the main menu and returns the action to do on exit
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunMenu() noexcept {
    do {
        // Run the menu: abort to the title screen & demos if the menu timed out
        if (MiniLoop(M_Start, M_Stop, M_Ticker, M_Drawer) == ga_timeout)
            return ga_timeout;

        // If we're not timing out draw the background and DOOM logo to prep for a 'loading' or 'connecting' plaque being drawn
        I_IncDrawnFrameCount();    
        I_CacheAndDrawSprite(*gTex_BACK, 0, 0, gPaletteClutIds[MAINPAL]);
        I_CacheAndDrawSprite(*gTex_DOOM, 75, 20, gPaletteClutIds[TITLEPAL]);
        I_SubmitGpuCmds();    
        I_DrawPresent();

        // If the game type is singleplayer then we can just go straight to initializing and running the game.
        // Otherwise for a net game, we need to establish a connection and show the 'connecting' plaque...
        if (*gStartGameType == gt_single)
            break;
        
        I_DrawLoadingPlaque(*gTex_CONNECT, 54, 103, gPaletteClutIds[MAINPAL]);
        I_NetSetup();

        // Once the net connection has been established, re-draw the background in prep for a loading or error plaque
        I_IncDrawnFrameCount();
        I_CacheAndDrawSprite(*gTex_BACK, 0, 0, gPaletteClutIds[MAINPAL]);
        I_CacheAndDrawSprite(*gTex_DOOM, 75, 20, gPaletteClutIds[TITLEPAL]);    
        I_SubmitGpuCmds();
        I_DrawPresent();
        
        // Play a sound to acknowledge that the connecting process has ended
        a0 = 0;
        a1 = sfx_pistol;
        S_StartSound();

    } while (*gbDidAbortGame);

    // Startup the game!
    G_InitNew(*gStartSkill, *gStartMapOrEpisode, *gStartGameType);
    G_RunGame();

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Setup/init logic for the main menu
//------------------------------------------------------------------------------------------------------------------------------------------
void M_Start() noexcept {
    // TODO: remove once there is no more VM stack use!
    sp -= 0x28;
    auto cleanupStackFrame = finally([]{ sp += 0x28; });

    // Assume no networked game initially
    *gNetGame = gt_single;
    *gCurPlayerIndex = 0;
    gbPlayerInGame[0] = true;
    gbPlayerInGame[1] = false;

    // Clear out any textures that can be unloaded
    I_PurgeTexCache();
    
    // Show the loading plaque
    I_LoadAndCacheTexLump(*gTex_LOADING, "LOADING", 0);
    I_DrawLoadingPlaque(*gTex_LOADING, 95, 109, gPaletteClutIds[UIPAL]);
    
    // TODO
    a0 = 0;
    S_LoadSoundAndMusic();
    
    // Load and cache some commonly used UI textures
    I_LoadAndCacheTexLump(*gTex_BACK, "BACK", 0);
    I_LoadAndCacheTexLump(*gTex_DOOM, "DOOM", 0);
    I_LoadAndCacheTexLump(*gTex_CONNECT, "CONNECT", 0);
    
    // Some basic menu setup
    *gCursorFrame = 0;
    *gCursorPos = 0;
    gVBlanksUntilMenuMove[0] = 0;
    
    if (*gStartGameType == gt_single) {
        *gMaxStartEpisodeOrMap = MAX_EPISODE;
    } else {
        *gMaxStartEpisodeOrMap = NUM_REGULAR_MAPS;      // For multiplayer any of the normal (non secret) maps can be selected
    }
    
    if (*gStartMapOrEpisode > *gMaxStartEpisodeOrMap) {
        // Wrap back around if we have to...
        *gStartMapOrEpisode = 1;
    }
    else if (*gStartMapOrEpisode < 0) {
        // Start map or episode will be set to '< 0' when the Doom I is finished.
        // This implies we want to point the user to Doom II:
        *gStartMapOrEpisode = 2;
    }

    // Play the main menu music
    a0 = gCDTrackNum[cdmusic_main_menu];
    a1 = *gCdMusicVol;
    a2 = 0;
    a3 = 0;
    sw(gCDTrackNum[cdmusic_main_menu], sp + 0x10);
    sw(*gCdMusicVol, sp + 0x14);
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    psxcd_play_at_andloop();

    // TODO: comment on elapsed sector stuff here
    do {
        psxcd_elapsed_sectors();
    } while (v0 == 0);

    // Don't clear the screen when setting a new draw environment.
    // Need to preserve the screen contents for the cross fade:
    gDrawEnvs[0].isbg = false;
    gDrawEnvs[1].isbg = false;

    // Draw the menu to the other framebuffer and do the cross fade
    *gCurDispBufferIdx ^= 1;
    M_Drawer();
    I_CrossFadeFrameBuffers();

    // Restore background clearing for both draw envs
    gDrawEnvs[0].isbg = true;
    gDrawEnvs[1].isbg = true;

    // Begin counting for menu timeouts
    *gMenuTimeoutStartTicCon = *gTicCon;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Teardown/cleanup logic for the main menu
//------------------------------------------------------------------------------------------------------------------------------------------
void M_Stop(const gameaction_t exitAction) noexcept {
    // Play the pistol sound and stop the current cd music track
    a0 = 0;
    a1 = sfx_pistol;
    S_StartSound();

    psxcd_stop();

    // Single player: adjust the start map for the episode that was selected
    if ((exitAction == ga_exit) && (*gStartGameType == gt_single)) {
        // If DOOM II is selected: point to the first map of DOOM II
        if (*gStartMapOrEpisode != episode_doom1) {
            *gStartMapOrEpisode = 31;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update/ticker logic for the main menu
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t M_Ticker() noexcept {
    // Reset the menu timeout if buttons are being pressed
    const padbuttons_t ticButtons = gTicButtons[0];

    if (ticButtons != 0) {
        *gMenuTimeoutStartTicCon = *gTicCon;
    }

    // Exit the menu if timed out
    if (*gTicCon - *gMenuTimeoutStartTicCon >= 1800)
        return ga_timeout;

    // Animate the skull cursor
    if ((*gGameTic > *gPrevGameTic) && ((*gGameTic & 3) == 0)) {
        *gCursorFrame ^= 1;
    }
    
    // Check for game start or options open actions
    if (ticButtons != gOldTicButtons[0]) {
        // If start is pressed then begin a game, no matter what menu option is picked
        if (ticButtons & PAD_START)
            return ga_exit;
    
        // If an action button is pressed then we can either start a map or open the options
        if ((ticButtons & PAD_ACTION_BTNS) && (gCursorPos[0] >= gamemode)) {
            if (gCursorPos[0] < options)
                return ga_exit;

            if (gCursorPos[0] == options) {
                // Options entry pressed: run the options menu.
                // Note that if a level password is entered correctly there, we exit with 'ga_warped' as the action.
                if (MiniLoop(O_Init, O_Shutdown, O_Control, O_Drawer) == ga_warped)
                    return ga_warped;
            }
        }
    }

    // Check for movement with the DPAD direction buttons.
    // If there is none then we can just stop here:    
    if ((ticButtons & PAD_DIRECTION_BTNS) == 0) {
        gVBlanksUntilMenuMove[0] = 0;
        return ga_nothing;
    }

    // Movement repeat rate limiting for when movement buttons are held down
    gVBlanksUntilMenuMove[0] -= gPlayersElapsedVBlanks[0];

    if (gVBlanksUntilMenuMove[0] > 0)
        return ga_nothing;

    gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;

    // Do menu up/down movements
    if (ticButtons & PAD_DOWN) {
        *gCursorPos += 1;
        
        if (*gCursorPos == NUMMENUITEMS) {
            *gCursorPos = (menu_t) 0;
        }

        a0 = 0;
        a1 = sfx_pstop;
        S_StartSound();
    }
    else if (ticButtons & PAD_UP) {
        *gCursorPos -= 1;

        if (*gCursorPos == -1) {
            *gCursorPos = NUMMENUITEMS - 1;
        }

        a0 = 0;
        a1 = sfx_pstop;
        S_StartSound();
    }

    if (gCursorPos[0] == gamemode) {
        // Menu left/right movements: game mode
        if (ticButtons & PAD_RIGHT) {
            if (*gStartGameType < gt_deathmatch) {
                *gStartGameType = (gametype_t)((uint32_t) *gStartGameType + 1);
                
                if (*gStartGameType == gt_coop) {
                    *gStartMapOrEpisode = 1;
                }

                a0 = 0;
                a1 = 0x17;
                S_StartSound();
            }
        } 
        else if (ticButtons & PAD_LEFT) {
            if (*gStartGameType != gt_single) {
                *gStartGameType = (gametype_t)((uint32_t) *gStartGameType -1);

                if (*gStartGameType == gt_single) {
                    *gStartMapOrEpisode = 1;
                }

                a0 = 0;
                a1 = 0x17;
                S_StartSound();
            }
        }

        if (*gStartGameType == gt_single) {
            *gMaxStartEpisodeOrMap = MAX_EPISODE;
        } else {
            *gMaxStartEpisodeOrMap = NUM_REGULAR_MAPS;
        }

        if (*gStartMapOrEpisode > *gMaxStartEpisodeOrMap) {
            *gStartMapOrEpisode = 1;
        }

        return ga_nothing;
    }    
    else if (gCursorPos[0] == level) {
        // Menu left/right movements: level/episode select
        if (ticButtons & PAD_RIGHT) {
            *gStartMapOrEpisode += 1;
            
            if (*gStartMapOrEpisode <= *gMaxStartEpisodeOrMap) {
                a0 = 0;
                a1 = sfx_swtchx;
                S_StartSound();
            } else {
                *gStartMapOrEpisode = *gMaxStartEpisodeOrMap;
            }
        } 
        else if (ticButtons & PAD_LEFT) {
            *gStartMapOrEpisode -= 1;
            
            if (*gStartMapOrEpisode > 0) {
                a0 = 0;
                a1 = sfx_swtchx;
                S_StartSound();
            } else {
                *gStartMapOrEpisode = 1;
            }
        }

        return ga_nothing;
    }
    else if (gCursorPos[0] == difficulty) {
        // Menu left/right movements: difficulty select
        if (ticButtons & PAD_RIGHT) {
            // PC-PSX: allow the previously hidden 'Nightmare' skill to be selected
            #if PC_PSX_DOOM_MODS
                constexpr skill_t MAX_ALLOWED_SKILL = sk_nightmare;
            #else
                constexpr skill_t MAX_ALLOWED_SKILL = sk_hard;
            #endif

            if (*gStartSkill < MAX_ALLOWED_SKILL) {
                *gStartSkill = (skill_t)((uint32_t) *gStartSkill + 1);
                a0 = 0;
                a1 = sfx_swtchx;
                S_StartSound();                
            }
        } 
        else if (ticButtons & PAD_LEFT) {
            if (*gStartSkill != sk_baby) {
                *gStartSkill = (skill_t)((uint32_t) *gStartSkill - 1);
                a0 = 0;
                a1 = sfx_swtchx;
                S_StartSound();
            }
        }
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the main menu
//------------------------------------------------------------------------------------------------------------------------------------------
void M_Drawer() noexcept {
    // Draw the menu background and increment frame count for the texture cache
    I_IncDrawnFrameCount();
    I_CacheAndDrawSprite(*gTex_BACK, 0, 0, gPaletteClutIds[MAINPAL]);

    // Draw the DOOM logo
    I_CacheAndDrawSprite(*gTex_DOOM, 75, 20, gPaletteClutIds[TITLEPAL]);

    // Draw the skull cursor
    I_DrawSprite(
        gTex_STATUS->texPageId,
        gPaletteClutIds[UIPAL],
        50,
        gMenuYPos[gCursorPos[0]] - 2,
        M_SKULL_TEX_U + (uint8_t)(*gCursorFrame) * M_SKULL_W,
        M_SKULL_TEX_V,
        M_SKULL_W,
        M_SKULL_H
    );

    // Draw the text for the various menu entries
    I_DrawString(74, gMenuYPos[gamemode], "Game Mode");
    I_DrawString(90, gMenuYPos[gamemode] + 20, gGameTypeNames[*gStartGameType]);

    if (*gStartGameType == gt_single) {
        if (*gStartMapOrEpisode == 1) {
            I_DrawString(74, gMenuYPos[level], "Ultimate Doom");
        } else {
            I_DrawString(74, gMenuYPos[level], "Doom II");
        }
    } else {
        // Coop or deathmatch game, draw the level number rather than episode
        I_DrawString(74, gMenuYPos[level], "Level");

        const int32_t xpos = (*gStartMapOrEpisode >= 10) ? 148 : 136;
        I_DrawNumber(xpos, gMenuYPos[level], *gStartMapOrEpisode);
    }

    I_DrawString(74, gMenuYPos[difficulty], "Difficulty");
    I_DrawString(90, gMenuYPos[difficulty] + 20, gSkillNames[*gStartSkill]);
    I_DrawString(74, gMenuYPos[options], "Options");
    
    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}
