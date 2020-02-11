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
#include "PsxVm/PsxVm.h"
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

void RunMenu() noexcept {
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = gPaletteClutIds;
    sw(s1, sp + 0x14);
    s1 = gTex_BACK;
    sw(s2, sp + 0x18);
    s2 = gTex_DOOM;
    sw(ra, sp + 0x1C);

loc_80035B4C:
    v0 = MiniLoop(M_Start, M_Stop, M_Ticker, M_Drawer);
    v1 = 7;
    {
        const bool bJump = (v0 == v1);
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_80035C78;
    }
    I_IncDrawnFrameCount();

    a0 = gTex_BACK;
    a1 = 0;                                             // Result = 00000000
    a3 = lh(s0);                                        // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_CacheAndDrawSprite();

    a0 = gTex_DOOM;
    a1 = 0x4B;                                          // Result = 0000004B
    a3 = lh(s0 + 0x22);                                 // Load from: gPaletteClutId_Title (800A90A6)
    a2 = 0x14;                                          // Result = 00000014
    _thunk_I_CacheAndDrawSprite();
    
    I_SubmitGpuCmds();
    I_DrawPresent();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7604);                               // Load from: gStartGameType (80077604)
    a0 = s1 + 0x100;                                    // Result = gTex_CONNECT[0] (80097B10)
    if (v0 == 0) goto loc_80035C4C;
    
    a1 = 0x36;                                          // Result = 00000036
    a3 = lh(s0);                                        // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0x67;                                          // Result = 00000067
    _thunk_I_DrawLoadingPlaque();

    I_NetSetup();
    I_IncDrawnFrameCount();
    
    a0 = gTex_BACK;
    a1 = 0;                                             // Result = 00000000
    a3 = lh(s0);                                        // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_CacheAndDrawSprite();

    a0 = gTex_DOOM;
    a1 = 0x4B;                                          // Result = 0000004B
    a3 = lh(s0 + 0x22);                                 // Load from: gPaletteClutId_Title (800A90A6)
    a2 = 0x14;                                          // Result = 00000014
    _thunk_I_CacheAndDrawSprite();
    
    I_SubmitGpuCmds();
    I_DrawPresent();
    a0 = 0;
    a1 = sfx_pistol;
    S_StartSound();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C0C);                               // Load from: gbDidAbortGame (80077C0C)
    if (v0 != 0) goto loc_80035B4C;
loc_80035C4C:
    G_InitNew(*gStartSkill, *gStartMapOrEpisode, *gStartGameType);
    G_RunGame();

    v0 = 0;
loc_80035C78:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
}

void M_Start() noexcept {
    sp -= 0x28;
    v0 = 1;
    sw(ra, sp + 0x24);
    sw(s0, sp + 0x20);

    *gNetGame = gt_single;
    *gCurPlayerIndex = 0;
    gbPlayerInGame[0] = true;
    gbPlayerInGame[1] = false;
    I_PurgeTexCache();
    
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7A90;                                       // Result = gTex_LOADING[0] (80097A90)
    a0 = s0;                                            // Result = gTex_LOADING[0] (80097A90)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C4C;                                       // Result = STR_LumpName_LOADING[0] (80077C4C)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    
    a0 = s0;                                            // Result = gTex_LOADING[0] (80097A90)
    a1 = 0x5F;                                          // Result = 0000005F
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    a2 = 0x6D;                                          // Result = 0000006D
    _thunk_I_DrawLoadingPlaque();
    
    a0 = 0;                                             // Result = 00000000
    S_LoadSoundAndMusic();
    
    a0 = gTex_BACK;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C8C;                                       // Result = STR_LumpName_BACK[0] (80077C8C)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();

    a0 = gTex_DOOM;                                     // Result = gTex_DOOM[0] (80097A50)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C94;                                       // Result = STR_LumpName_DOOM[0] (80077C94)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    
    a0 = s0 + 0x80;                                     // Result = gTex_CONNECT[0] (80097B10)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C9C;                                       // Result = STR_LumpName_CONNECT[0] (80077C9C)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7604);                               // Load from: gStartGameType (80077604)
    sw(0, gp + 0xBF8);                                  // Store to: gCursorFrame (800781D8)
    *gCursorPos = 0;
    *gVBlanksUntilMenuMove = 0;
    {
        const bool bJump = (v0 != 0);
        v0 = 0x36;                                      // Result = 00000036
        if (bJump) goto loc_80035D64;
    }
    v0 = 2;                                             // Result = 00000002
loc_80035D64:
    sw(v0, gp + 0xB9C);                                 // Store to: gMaxStartEpisodeOrMap (8007817C)
    v1 = *gStartMapOrEpisode;
    v0 = lw(gp + 0xB9C);                                // Load from: gMaxStartEpisodeOrMap (8007817C)
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80035D8C;
    }
    v0 = 2;                                             // Result = 00000002
    if (i32(v1) >= 0) goto loc_80035D94;
loc_80035D8C:
    at = 0x80070000;                                    // Result = 80070000
    *gStartMapOrEpisode = v0;
loc_80035D94:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3E50;                                       // Result = CDTrackNum_MainMenu (80073E50)
    a0 = lw(v0);                                        // Load from: CDTrackNum_MainMenu (80073E50)
    a1 = *gCdMusicVol;
    a2 = 0;                                             // Result = 00000000
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    v0 = lw(v0);                                        // Load from: CDTrackNum_MainMenu (80073E50)
    a3 = 0;                                             // Result = 00000000
    sw(v0, sp + 0x10);
    sw(a1, sp + 0x14);
    psxcd_play_at_andloop();
loc_80035DC8:
    psxcd_elapsed_sectors();
    if (v0 == 0) goto loc_80035DC8;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F08);                               // Load from: gCurDrawDispBufferIdx (800780F8)
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6F3C;                                       // Result = gDrawEnv1[C] (800A90C4)
    sb(0, s0);                                          // Store to: gDrawEnv1[C] (800A90C4)
    at = 0x800B0000;                                    // Result = 800B0000
    sb(0, at - 0x6EE0);                                 // Store to: gDrawEnv2[C] (800A9120)
    v0 ^= 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7F08);                                // Store to: gCurDrawDispBufferIdx (800780F8)
    M_Drawer();
    I_CrossFadeFrameBuffers();
    v1 = *gTicCon;
    v0 = 1;
    sb(v0, s0);                                         // Store to: gDrawEnv1[C] (800A90C4)
    at = 0x800B0000;                                    // Result = 800B0000
    sb(v0, at - 0x6EE0);                                // Store to: gDrawEnv2[C] (800A9120)
    *gMenuTimeoutStartTicCon = v1;
    ra = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x28;
    return;
}

void M_Stop() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    a0 = 0;
    sw(ra, sp + 0x14);
    a1 = sfx_pistol;
    S_StartSound();
    psxcd_stop();
    v0 = 9;
    if (s0 != v0) goto loc_80035EB0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7604);                               // Load from: gStartGameType (80077604)
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80035EB0;
    }
    v1 = *gStartMapOrEpisode;
    {
        const bool bJump = (v1 != v0);
        v0 = 0x1F;                                      // Result = 0000001F
        if (bJump) goto loc_80035EA8;
    }
    *gStartMapOrEpisode = v1;
    goto loc_80035EB0;
loc_80035EA8:
    *gStartMapOrEpisode = v0;
loc_80035EB0:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
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
