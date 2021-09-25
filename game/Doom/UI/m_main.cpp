#include "m_main.h"

#include "Doom/Base/i_crossfade.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/i_texcache.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "o_main.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/MapInfo.h"
#include "PsyDoom/Network.h"
#include "PsyDoom/PsxPadButtons.h"
#include "PsyDoom/Utils.h"
#include "PsyQ/LIBGPU.h"
#include "SmallString.h"
#include "Wess/psxcd.h"

#include <algorithm>

// PsyDoom: move this up slightly to make room for the 'quit' option
#if PSYDOOM_MODS
    static constexpr int32_t DOOM_LOGO_YPOS = 10;
#else
    static constexpr int32_t DOOM_LOGO_YPOS = 20;
#endif

texture_t   gTex_BACK;                  // The background texture for the main menu
int32_t     gCursorPos[MAXPLAYERS];     // Which of the menu options each player's cursor is over (see 'menu_t')
int32_t     gCursorFrame;               // Current frame that the menu cursor is displaying
int32_t     gMenuTimeoutStartTicCon;    // Tick that we start checking for menu timeouts from

// Main menu options
enum menu_t : int32_t {
    gamemode,
    level,
    difficulty,
    options,
#if PSYDOOM_MODS        // PsyDoom: add a quit option to the main menu
    menu_quit,
#endif
    NUMMENUITEMS
};

// The position of each main menu option.
// PsyDoom: had to move everything up to make room for 'quit'
#if PSYDOOM_MODS
    static const int16_t gMenuYPos[NUMMENUITEMS] = {
        81,     // gamemode
        123,    // level
        145,    // difficulty
        187,    // options
        209     // quit
    };
#else
    static const int16_t gMenuYPos[NUMMENUITEMS] = {
        91,     // gamemode
        133,    // level
        158,    // difficulty
        200     // options
    };
#endif

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
#if PSYDOOM_MODS        // PsyDoom: exposing the unused 'Nightmare' mode
    "Nightmare!"
#endif
};

static texture_t    gTex_DOOM;                  // The texture for the DOOM logo
static int32_t      gMaxStartEpisodeOrMap;      // Restricts what maps or episodes the player can pick

//------------------------------------------------------------------------------------------------------------------------------------------
// Starts up the main menu and returns the action to do on exit
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t RunMenu() noexcept {
    do {
        // Run the menu: abort to the title screen & demos if the menu timed out.
        // PsyDoom: also quit if app quit is requested.
        const gameaction_t menuResult = MiniLoop(M_Start, M_Stop, M_Ticker, M_Drawer);

        #if PSYDOOM_MODS
            if ((menuResult == ga_timeout) || (menuResult == ga_quitapp))
                return menuResult;
        #else
            if (menuResult == ga_timeout)
                return menuResult;
        #endif

        // If we're not timing out draw the background and DOOM logo to prep for a 'loading' or 'connecting' plaque being drawn
        #if PSYDOOM_MODS
            Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
        #endif

        I_IncDrawnFrameCount();
        I_CacheAndDrawSprite(gTex_BACK, 0, 0, Game::getTexPalette_BACK());
        I_CacheAndDrawSprite(gTex_DOOM, 75, DOOM_LOGO_YPOS, Game::getTexPalette_DOOM());
        I_SubmitGpuCmds();
        I_DrawPresent();

        // If the game type is singleplayer then we can just go straight to initializing and running the game.
        // Otherwise for a net game, we need to establish a connection and show the 'connecting' plaque...
        if (gStartGameType == gt_single)
            break;

        I_DrawLoadingPlaque(gTex_CONNECT, 54, 103, Game::getTexPalette_CONNECT());
        I_NetSetup();

        // PsyDoom: abort if app quit was requested
        #if PSYDOOM_MODS
            if (Input::isQuitRequested())
                break;
        #endif

        // Once the net connection has been established, re-draw the background in prep for a loading or error plaque
        #if PSYDOOM_MODS
            Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
        #endif

        I_IncDrawnFrameCount();
        I_CacheAndDrawSprite(gTex_BACK, 0, 0, Game::getTexPalette_BACK());
        I_CacheAndDrawSprite(gTex_DOOM, 75, DOOM_LOGO_YPOS, Game::getTexPalette_DOOM());
        I_SubmitGpuCmds();
        I_DrawPresent();

        // Play a sound to acknowledge that the connecting process has ended
        S_StartSound(nullptr, sfx_pistol);

    } while (gbDidAbortGame);

    // Startup the game!
    // PsyDoom: don't do this if app quit was requested.
    {
        #if PSYDOOM_MODS
            const bool bStartGame = (!Input::isQuitRequested());
        #else 
            const bool bStartGame = true;
        #endif

        if (bStartGame) {
            G_InitNew(gStartSkill, gStartMapOrEpisode, gStartGameType);
            G_RunGame();
        }
    }

    // PsyDoom: cleanup any network connections if we were doing a net game
    #if PSYDOOM_MODS
        Network::shutdown();
    #endif

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Setup/init logic for the main menu
//------------------------------------------------------------------------------------------------------------------------------------------
void M_Start() noexcept {
    // Assume no networked game initially
    gNetGame = gt_single;
    gCurPlayerIndex = 0;
    gbPlayerInGame[0] = true;
    gbPlayerInGame[1] = false;

    // Clear out any textures that can be unloaded
    I_PurgeTexCache();

    // Show the loading plaque
    I_LoadAndCacheTexLump(gTex_LOADING, "LOADING", 0);
    I_DrawLoadingPlaque(gTex_LOADING, 95, 109, Game::getTexPalette_LOADING());

    // Load sounds for the menu
    S_LoadMapSoundAndMusic(0);

    // Load and cache some commonly used UI textures
    I_LoadAndCacheTexLump(gTex_BACK, "BACK", 0);
    I_LoadAndCacheTexLump(gTex_DOOM, "DOOM", 0);
    I_LoadAndCacheTexLump(gTex_CONNECT, "CONNECT", 0);

    // Some basic menu setup
    gCursorFrame = 0;
    gCursorPos[0] = 0;
    gVBlanksUntilMenuMove[0] = 0;

    if (gStartGameType == gt_single) {
        gMaxStartEpisodeOrMap = Game::getNumEpisodes();
    } else {
        gMaxStartEpisodeOrMap = Game::getNumRegularMaps();  // For multiplayer any of the normal (non secret) maps can be selected
    }

    if (gStartMapOrEpisode > gMaxStartEpisodeOrMap) {
        // Wrap back around if we have to...
        gStartMapOrEpisode = 1;
    }
    else if (gStartMapOrEpisode < 0) {
        // Start map or episode will be set to '-NextEpisodeNum' when an episode like 'Ultimate Doom' is finished, which has a next episode.
        // This means we want to point the user to the next episode automatically, upon entering the main menu.
        gStartMapOrEpisode = -gStartMapOrEpisode;

        // PsyDoom: an added check, just in case...
        #if PSYDOOM_MODS
            gStartMapOrEpisode = std::min(gStartMapOrEpisode, gMaxStartEpisodeOrMap);
        #endif
    }

    // Play the main menu music
    psxcd_play_at_andloop(
        gCDTrackNum[cdmusic_main_menu],
        gCdMusicVol,
        0,
        0,
        gCDTrackNum[cdmusic_main_menu],
        gCdMusicVol,
        0,
        0
    );

    // Wait until some cd audio has been read
    Utils::waitForCdAudioPlaybackStart();

    // Don't clear the screen when setting a new draw environment.
    // Need to preserve the screen contents for the cross fade:
    gDrawEnvs[0].isbg = false;
    gDrawEnvs[1].isbg = false;

    // Draw the menu to the other framebuffer and do the cross fade
    gCurDispBufferIdx ^= 1;

    #if PSYDOOM_MODS
        I_PreCrossfade();
    #endif

    M_Drawer();
    I_CrossfadeFrameBuffers();

    // Restore background clearing for both draw envs
    gDrawEnvs[0].isbg = true;
    gDrawEnvs[1].isbg = true;

    // Begin counting for menu timeouts
    gMenuTimeoutStartTicCon = gTicCon;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Teardown/cleanup logic for the main menu
//------------------------------------------------------------------------------------------------------------------------------------------
void M_Stop(const gameaction_t exitAction) noexcept {
    // PsyDoom: if quitting the app then exit immediately, don't play any sounds and fade cd audio etc.
    #if PSYDOOM_MODS
        if (Input::isQuitRequested() || (exitAction == ga_quitapp))
            return;
    #endif

    // Play the pistol sound and stop the current cd music track
    S_StartSound(nullptr, sfx_pistol);

    #if PSYDOOM_MODS
        // PsyDoom: update sounds so that the barrel explosion plays while the music fades out.
        // Need to do this now since sounds don't play immediately and are queued.
        S_UpdateSounds();
    #endif

    psxcd_stop();

    // Single player: adjust the start map for the episode that was selected
    if ((exitAction == ga_exit) && (gStartGameType == gt_single)) {
        gStartMapOrEpisode = Game::getEpisodeStartMap(gStartMapOrEpisode);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update/ticker logic for the main menu
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t M_Ticker() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0) {
            gbKeepInputEvents = true;   // Don't consume 'key pressed' etc. events yet, not ticking...
            return ga_nothing;
        }
    #endif

    // Reset the menu timeout if buttons are being pressed.
    // PsyDoom: just restrict that to valid menu inputs only.
    #if PSYDOOM_MODS
        const TickInputs& inputs = gTickInputs[0];
        const TickInputs& oldInputs = gOldTickInputs[0];

        const bool bMenuStart = (inputs.bMenuStart && (inputs.bMenuStart != oldInputs.bMenuStart));
        const bool bMenuOk = (inputs.bMenuOk && (!oldInputs.bMenuOk));
        const bool bMenuUp = inputs.bMenuUp;
        const bool bMenuDown = inputs.bMenuDown;
        const bool bMenuLeft = inputs.bMenuLeft;
        const bool bMenuRight = inputs.bMenuRight;
        const bool bMenuMove = (bMenuUp || bMenuDown || bMenuLeft || bMenuRight);
        const bool bMenuAnyInput = (bMenuMove || bMenuOk || bMenuStart || inputs.bMenuBack);
    #else
        const padbuttons_t ticButtons = gTicButtons[0];

        const bool bMenuStart = ((ticButtons != gOldTicButtons[0]) && (ticButtons & PAD_START));
        const bool bMenuOk = ((ticButtons != gOldTicButtons[0]) && (ticButtons & PAD_ACTION_BTNS));
        const bool bMenuUp = (ticButtons & PAD_UP);
        const bool bMenuDown = (ticButtons & PAD_DOWN);
        const bool bMenuLeft = (ticButtons & PAD_LEFT);
        const bool bMenuRight = (ticButtons & PAD_RIGHT);
        const bool bMenuMove = (ticButtons & PAD_DIRECTION_BTNS);
        const bool bMenuAnyInput = (ticButtons != 0);
    #endif

    if (bMenuAnyInput) {
        gMenuTimeoutStartTicCon = gTicCon;
    }

    // Exit the menu if timed out
    if (gTicCon - gMenuTimeoutStartTicCon >= 1800)
        return ga_timeout;

    // Animate the skull cursor
    if ((gGameTic > gPrevGameTic) && ((gGameTic & 3) == 0)) {
        gCursorFrame ^= 1;
    }

    // If start is pressed then begin a game, no matter what menu option is picked
    if (bMenuStart)
        return ga_exit;

    // If an 'ok' button is pressed then we can either start a map or open the options
    if (bMenuOk && (gCursorPos[0] >= gamemode)) {
        if (gCursorPos[0] < options)
            return ga_exit;

        if (gCursorPos[0] == options) {
            // Options entry pressed: run the options menu.
            // Note that if a level password is entered correctly there, we exit with 'ga_warped' as the action.
            if (MiniLoop(O_Init, O_Shutdown, O_Control, O_Drawer) == ga_warped)
                return ga_warped;
        }

        // PsyDoom: quit the game if that option is chosen
        #if PSYDOOM_MODS
            if (gCursorPos[0] == menu_quit)
                return ga_quitapp;
        #endif
    }

    // Check for movement with the DPAD direction buttons.
    // If there is none then we can just stop here:
    if (!bMenuMove) {
        gVBlanksUntilMenuMove[0] = 0;
        return ga_nothing;
    }

    // Movement repeat rate limiting for when movement buttons are held down
    gVBlanksUntilMenuMove[0] -= gPlayersElapsedVBlanks[0];

    if (gVBlanksUntilMenuMove[0] > 0)
        return ga_nothing;

    gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;

    // Do menu up/down movements
    if (bMenuDown) {
        gCursorPos[0]++;

        if (gCursorPos[0] == NUMMENUITEMS) {
            gCursorPos[0] = (menu_t) 0;
        }

        S_StartSound(nullptr, sfx_pstop);
    }
    else if (bMenuUp) {
        gCursorPos[0]--;

        if (gCursorPos[0] == -1) {
            gCursorPos[0] = NUMMENUITEMS - 1;
        }

        S_StartSound(nullptr, sfx_pstop);
    }

    if (gCursorPos[0] == gamemode) {
        // Menu left/right movements: game mode.
        // PsyDoom: multiplayer can now be disabled by mods that don't want to support it or which can't support it.
        #if PSYDOOM_MODS
            const bool bAllowMultiplayer = (!MapInfo::getGameInfo().bDisableMultiplayer);
        #else
            const bool bAllowMultiplayer = true;
        #endif

        if (bAllowMultiplayer) {
            if (bMenuRight) {
                if (gStartGameType < gt_deathmatch) {
                    gStartGameType = (gametype_t)((uint32_t) gStartGameType + 1);

                    if (gStartGameType == gt_coop) {
                        gStartMapOrEpisode = 1;
                    }

                    S_StartSound(nullptr, sfx_swtchx);
                }
            }
            else if (bMenuLeft) {
                if (gStartGameType != gt_single) {
                    gStartGameType = (gametype_t)((uint32_t) gStartGameType -1);

                    if (gStartGameType == gt_single) {
                        gStartMapOrEpisode = 1;
                    }

                    S_StartSound(nullptr, sfx_swtchx);
                }
            }
        } else {
            #if PSYDOOM_MODS
                if (bMenuRight || bMenuLeft) {
                    // PsyDoom: play a sound when trying to switch to multiplayer when it is not allowed
                    S_StartSound(nullptr, sfx_itemup);
                }
            #endif
        }

        if (gStartGameType == gt_single) {
            gMaxStartEpisodeOrMap = Game::getNumEpisodes();
        } else {
            gMaxStartEpisodeOrMap = Game::getNumRegularMaps();
        }

        if (gStartMapOrEpisode > gMaxStartEpisodeOrMap) {
            gStartMapOrEpisode = 1;
        }

        return ga_nothing;
    }
    else if (gCursorPos[0] == level) {
        // Menu left/right movements: level/episode select
        if (bMenuRight) {
            gStartMapOrEpisode += 1;

            if (gStartMapOrEpisode <= gMaxStartEpisodeOrMap) {
                S_StartSound(nullptr, sfx_swtchx);
            } else {
                gStartMapOrEpisode = gMaxStartEpisodeOrMap;
            }
        }
        else if (bMenuLeft) {
            gStartMapOrEpisode -= 1;

            if (gStartMapOrEpisode > 0) {
                S_StartSound(nullptr, sfx_swtchx);
            } else {
                gStartMapOrEpisode = 1;
            }
        }

        return ga_nothing;
    }
    else if (gCursorPos[0] == difficulty) {
        // Menu left/right movements: difficulty select
        if (bMenuRight) {
            // PsyDoom: allow the previously hidden 'Nightmare' skill to be selected
            #if PSYDOOM_MODS
                constexpr skill_t MAX_ALLOWED_SKILL = sk_nightmare;
            #else
                constexpr skill_t MAX_ALLOWED_SKILL = sk_hard;
            #endif

            if (gStartSkill < MAX_ALLOWED_SKILL) {
                gStartSkill = (skill_t)((uint32_t) gStartSkill + 1);
                S_StartSound(nullptr, sfx_swtchx);
            }
        }
        else if (bMenuLeft) {
            if (gStartSkill != sk_baby) {
                gStartSkill = (skill_t)((uint32_t) gStartSkill - 1);
                S_StartSound(nullptr, sfx_swtchx);
            }
        }
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the main menu
//------------------------------------------------------------------------------------------------------------------------------------------
void M_Drawer() noexcept {
    // Draw the menu background and increment frame count for the texture cache.
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    I_CacheAndDrawSprite(gTex_BACK, 0, 0, Game::getTexPalette_BACK());

    // Draw the DOOM logo
    I_CacheAndDrawSprite(gTex_DOOM, 75, DOOM_LOGO_YPOS, Game::getTexPalette_DOOM());

    // Draw the skull cursor
    I_DrawSprite(
        gTex_STATUS.texPageId,
        gPaletteClutIds[UIPAL],
        50,
        gMenuYPos[gCursorPos[0]] - 2,
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

    // Draw the text for the various menu entries
    I_DrawString(74, gMenuYPos[gamemode], "Game Mode");
    I_DrawString(90, gMenuYPos[gamemode] + 20, gGameTypeNames[gStartGameType]);

    if (gStartGameType == gt_single) {
        I_DrawString(74, gMenuYPos[level], Game::getEpisodeName(gStartMapOrEpisode).c_str().data());
    } else {
        // Coop or deathmatch game, draw the level number rather than episode
        I_DrawString(74, gMenuYPos[level], "Level");

        const int32_t xpos = (gStartMapOrEpisode >= 10) ? 148 : 136;
        I_DrawNumber(xpos, gMenuYPos[level], gStartMapOrEpisode);
    }

    I_DrawString(74, gMenuYPos[difficulty], "Difficulty");
    I_DrawString(90, gMenuYPos[difficulty] + 20, gSkillNames[gStartSkill]);
    I_DrawString(74, gMenuYPos[options], "Options");

    #if PSYDOOM_MODS
        I_DrawString(74, gMenuYPos[menu_quit], "Quit");
    #endif

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the display when the game is waiting for a network connection.
// Used by the Network module to re-draw the display during network connection for the Vulkan renderer.
// This is required if the user resizes the view while we are connecting, when we need to refresh the display.
//------------------------------------------------------------------------------------------------------------------------------------------
void M_DrawNetworkConnectDisplay() noexcept {
    I_IncDrawnFrameCount();
    Utils::onBeginUIDrawing();

    I_CacheAndDrawSprite(gTex_BACK, 0, 0, Game::getTexPalette_BACK());
    I_CacheAndDrawSprite(gTex_DOOM, 75, DOOM_LOGO_YPOS, Game::getTexPalette_DOOM());
    I_CacheAndDrawSprite(gTex_CONNECT, 54, 103, Game::getTexPalette_CONNECT());

    I_SubmitGpuCmds();
    I_DrawPresent();
}
#endif  // #if PSYDOOM_MODS
