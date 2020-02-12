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
    "Nightmare"
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
        if (MiniLoop(M_Start, _thunk_M_Stop, M_Ticker, M_Drawer) == ga_timeout)
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
    *gVBlanksUntilMenuMove = 0;
    
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

void _thunk_M_Stop() noexcept {
    M_Stop((gameaction_t) a0);
}

void M_Ticker() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x7F44);                               // Load from: gTicButtons[0] (80077F44)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7DEC);                               // Load from: gOldTicButtons[0] (80078214)
    sw(ra, sp + 0x14);
    if (s0 == 0) goto loc_80035EF4;
    v0 = *gTicCon;
    *gMenuTimeoutStartTicCon = v0;
loc_80035EF4:
    v0 = *gTicCon;
    v1 = *gMenuTimeoutStartTicCon;
    v0 -= v1;
    v0 = (i32(v0) < 0x708);
    {
        const bool bJump = (v0 == 0);
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_80036244;
    }
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 3;
        if (bJump) goto loc_80035F4C;
    }
    if (v0 != 0) goto loc_80035F4C;
    v0 = lw(gp + 0xBF8);                                // Load from: gCursorFrame (800781D8)
    v0 ^= 1;
    sw(v0, gp + 0xBF8);                                 // Store to: gCursorFrame (800781D8)
loc_80035F4C:
    v0 = s0 & 0x800;
    if (s0 == a0) goto loc_80035FCC;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0xF0;
        if (bJump) goto loc_80035F64;
    }
    v0 = 9;                                             // Result = 00000009
    goto loc_80036244;
loc_80035F64:
    if (v0 == 0) goto loc_80035FCC;
    v1 = gCursorPos[0];
    v0 = (i32(v1) < 3);
    if (i32(v1) < 0) goto loc_80035FCC;
    {
        const bool bJump = (v0 != 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_80036244;
    }
    v0 = 3;                                             // Result = 00000003
    {
        const bool bJump = (v1 != v0);
        v0 = s0 & 0xF000;
        if (bJump) goto loc_80035FD0;
    }

    v0 = MiniLoop(O_Init, O_Shutdown, O_Control, O_Drawer);
    v1 = 4;

    {
        const bool bJump = (v0 != v1);
        v0 = s0 & 0xF000;
        if (bJump) goto loc_80035FD0;
    }
    v0 = 4;                                             // Result = 00000004
    goto loc_80036244;
loc_80035FCC:
    v0 = s0 & 0xF000;
loc_80035FD0:
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80035FE4;
    }
    *gVBlanksUntilMenuMove = 0;
    goto loc_80036244;
loc_80035FE4:
    v0 = *gVBlanksUntilMenuMove;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FBC);                               // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v0 -= v1;
    *gVBlanksUntilMenuMove = v0;
    if (i32(v0) > 0) goto loc_80036240;
    *gVBlanksUntilMenuMove = MENU_MOVE_VBLANK_DELAY;
    v0 = s0 & 0x4000;
    v1 = 4;                                             // Result = 00000004
    if (v0 == 0) goto loc_80036040;
    a0 = gCursorPos;
    v0 = gCursorPos[0];
    v0++;
    sw(v0, a0);                                         // Store to: gCursorPos (80078000)
    if (v0 != v1) goto loc_80036070;
    sw(0, a0);                                          // Store to: gCursorPos (80078000)
    goto loc_80036070;
loc_80036040:
    v0 = s0 & 0x1000;
    v1 = -1;                                            // Result = FFFFFFFF
    if (v0 == 0) goto loc_8003607C;
    a0 = gCursorPos;
    v0 = gCursorPos[0];
    v0--;
    sw(v0, a0);                                         // Store to: gCursorPos (80078000)
    if (v0 != v1) goto loc_80036070;
    v0 = 3;                                             // Result = 00000003
    sw(v0, a0);                                         // Store to: gCursorPos (80078000)
loc_80036070:
    a0 = 0;
    a1 = sfx_pstop;
    S_StartSound();
loc_8003607C:
    v1 = gCursorPos[0];
    a0 = 1;
    v0 = (i32(v1) < 2);
    if (v1 == a0) goto loc_80036178;
    if (v0 == 0) goto loc_800360A4;
    v0 = s0 & 0x2000;
    if (v1 == 0) goto loc_800360B8;
    v0 = 0;                                             // Result = 00000000
    goto loc_80036244;
loc_800360A4:
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (v1 == v0);
        v0 = s0 & 0x2000;
        if (bJump) goto loc_800361E8;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_80036244;
loc_800360B8:
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x8000;
        if (bJump) goto loc_800360F0;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7604);                               // Load from: gStartGameType (80077604)
    v0 = (v1 < 2);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 + 1;
        if (bJump) goto loc_80036130;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7604);                                // Store to: gStartGameType (80077604)
    if (v0 == a0) goto loc_8003611C;
    a0 = 0;                                             // Result = 00000000
    goto loc_80036128;
loc_800360F0:
    if (v0 == 0) goto loc_80036130;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7604);                               // Load from: gStartGameType (80077604)
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_80036144;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7604);                                // Store to: gStartGameType (80077604)
    if (v0 != 0) goto loc_80036124;
loc_8003611C:
    *gStartMapOrEpisode = a0;
loc_80036124:
    a0 = 0;                                             // Result = 00000000
loc_80036128:
    a1 = 0x17;                                          // Result = 00000017
    S_StartSound();
loc_80036130:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7604);                               // Load from: gStartGameType (80077604)
    {
        const bool bJump = (v0 != 0);
        v0 = 0x36;                                      // Result = 00000036
        if (bJump) goto loc_80036148;
    }
loc_80036144:
    v0 = 2;                                             // Result = 00000002
loc_80036148:
    sw(v0, gp + 0xB9C);                                 // Store to: gMaxStartEpisodeOrMap (8007817C)
    v1 = *gStartMapOrEpisode;
    v0 = lw(gp + 0xB9C);                                // Load from: gMaxStartEpisodeOrMap (8007817C)
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80036240;
    }
    *gStartMapOrEpisode = v0;
    v0 = 0;                                             // Result = 00000000
    goto loc_80036244;
loc_80036178:
    v0 = s0 & 0x2000;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x8000;
        if (bJump) goto loc_800361B0;
    }
    v0 = *gStartMapOrEpisode;
    v1 = lw(gp + 0xB9C);                                // Load from: gMaxStartEpisodeOrMap (8007817C)
    v0++;
    *gStartMapOrEpisode = v0;
    v0 = (i32(v1) < i32(v0));
    a0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_800361D8;
    goto loc_80036238;
loc_800361B0:
    {
        const bool bJump = (v0 == 0);
        v0 = 0;
        if (bJump) goto loc_80036244;
    }
    v0 = *gStartMapOrEpisode;
    v0--;
    *gStartMapOrEpisode = v0;
    a0 = 0;
    if (i32(v0) > 0) goto loc_80036238;
loc_800361D8:
    *gStartMapOrEpisode = v1;
    v0 = 0;
    goto loc_80036244;
loc_800361E8:
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x8000;
        if (bJump) goto loc_80036210;
    }
    v1 = *gStartSkill;
// PC-PSX: allow nightmare to be selected
#if PC_PSX_DOOM_MODS
    v0 = (v1 < sk_nightmare);
#else
    v0 = (v1 < 3);
#endif
    {
        const bool bJump = (v0 == 0);
        v0 = v1 + 1;
        if (bJump) goto loc_80036240;
    }
    goto loc_8003622C;
loc_80036210:
    {
        const bool bJump = (v0 == 0);
        v0 = 0;
        if (bJump) goto loc_80036244;
    }
    v0 = *gStartSkill;
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_80036240;
    }
loc_8003622C:
    *gStartSkill = (skill_t) v0;
    a0 = 0;
loc_80036238:
    a1 = sfx_swtchx;
    S_StartSound();
loc_80036240:
    v0 = 0;
loc_80036244:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
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
