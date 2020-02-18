#include "f_finale.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/info.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "m_main.h"
#include "PcPsx/Finally.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"

// Win text for Doom 1 and 2
static const char gDoom1WinText[][24 + 1] = {
    { "you have won!"               },
    { "your victory enabled"        },
    { "humankind to evacuate"       },
    { "earth and escape the"        },
    { "nightmare."                  },
    { "but then earth control"      },
    { "pinpoints the source"        },
    { "of the alien invasion."      },
    { "you are their only hope."    },
    { "you painfully get up"        },
    { "and return to the fray."     }
};

static const char gDoom2WinText[][24 + 1] = {
    { "you did it!"                 },
    { "by turning the evil of"      },
    { "the horrors of hell in"      },
    { "upon itself you have"        },
    { "destroyed the power of"      },
    { "the demons."                 },
    { "their dreadful invasion"     },
    { "has been stopped cold!"      },
    { "now you can retire to"       },
    { "a lifetime of frivolity."    },
    { "congratulations!"            }
};

// The cast of characters to display
struct castinfo_t {
    const char*     name;
    mobjtype_t      type;
};

static const castinfo_t gCastOrder[] = {
    { "Zombieman",              MT_POSSESSED    },
    { "Shotgun Guy",            MT_SHOTGUY      },
    { "Heavy Weapon Dude",      MT_CHAINGUY     },
    { "Imp",                    MT_TROOP        },
    { "Demon",                  MT_SERGEANT     },
    { "Lost Soul",              MT_SKULL        },
    { "Cacodemon",              MT_HEAD         },
    { "Hell Knight",            MT_KNIGHT       },
    { "Baron Of Hell",          MT_BRUISER      },
    { "Arachnotron",            MT_BABY         },
    { "Pain Elemental",         MT_PAIN         },
    { "Revenant",               MT_UNDEAD       },
    { "Mancubus",               MT_FATSO        },
    { "The Spider Mastermind",  MT_SPIDER       },
    { "The Cyberdemon",         MT_CYBORG       },
    { "Our Hero",               MT_PLAYER       },
    { nullptr,                  MT_PLAYER       }       // Null marker
};

// Used for the cast finale - what stage we are at
enum finalestage_t : int32_t {
    F_STAGE_TEXT,
    F_STAGE_SCROLLTEXT,
    F_STAGE_CAST,
};

static const VmPtr<finalestage_t> gFinaleStage(0x8007816C);

static const VmPtr<int32_t>         gFinTextYPos(0x80077F10);           // Current y position of the top finale line
static const VmPtr<char[28]>        gFinIncomingLine(0x800A9048);       // Text for the incoming line
static const VmPtr<int32_t>         gFinIncomingLineLen(0x80077F58);    // How many characters are being displayed for the incomming text line
static const VmPtr<int32_t>         gFinLinesDone(0x80078110);          // How many full lines we are displaying
static const VmPtr<int32_t>         gCastNum(0x80078288);               // Which of the cast characters (specified by index) we are showing
static const VmPtr<VmPtr<state_t>>  gpCastState(0x80077FA8);            // Current state being displayed for the cast character
static const VmPtr<texture_t>       gTex_DEMON(0x80097BB0);             // The demon (icon of sin) background for the DOOM II finale

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the Ultimate DOOM finale screen
//------------------------------------------------------------------------------------------------------------------------------------------
void F1_Start() noexcept {
    // TODO: remove this eventually when no more VM stack is being used
    sp -= 0x28;
    auto cleanupStackFrame = finally([](){ sp += 0x28; });

    // Draw the loading plaque, purge the texture cache and load up the background needed
    I_DrawLoadingPlaque(*gTex_LOADING, 95, 109, gPaletteClutIds[UIPAL]);
    I_PurgeTexCache();
    I_CacheTex(*gTex_BACK);

    // Init finale 
    *gFinLinesDone = 0;
    *gFinIncomingLineLen = 0;
    gFinIncomingLine[0] = 0;

    // Play the finale cd track
    a0 = gCDTrackNum[cdmusic_finale_doom1];
    a1 = *gCdMusicVol;
    a2 = 0;
    a3 = 0;
    sw(gCDTrackNum[cdmusic_credits_demo], sp + 0x10);
    sw(*gCdMusicVol, sp + 0x14);
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    psxcd_play_at_andloop();

    // TODO: comment on elapsed sector stuff here
    do {
        psxcd_elapsed_sectors();
    } while (v0 == 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called to shut down the Ultimate DOOM finale screen
//------------------------------------------------------------------------------------------------------------------------------------------
void F1_Stop() noexcept {
    *gbGamePaused = false;
    psxcd_stop();
}

void F1_Ticker() noexcept {
    v0 = *gCurPlayerIndex;
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    *gGameAction = ga_nothing;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += v0;
    s0 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gOldTicButtons[0] (80078214)
    at += v0;
    s1 = lw(at);
    P_CheckCheats();
    v0 = *gbGamePaused;
    if (v0 == 0) goto loc_8003D80C;
    v0 = *gGameAction;
    goto loc_8003D8D8;
loc_8003D80C:
    a0 = *gFinLinesDone;
    v0 = (i32(a0) < 0xB);
    if (v0 == 0) goto loc_8003D8C4;
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 1;
        if (bJump) goto loc_8003D8D4;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003D8D8;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x483C;                                       // Result = STR_Doom1_WinText_1[0] (8007483C)
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    v0 += a0;
    a2 = *gFinIncomingLineLen;
    a1 = v0 + v1;
    v0 = a1 + a2;
    v0 = lbu(v0);
    {
        const bool bJump = (v0 != 0);
        v0 = a0 + 1;
        if (bJump) goto loc_8003D88C;
    }
    *gFinIncomingLineLen = 0;
    *gFinLinesDone = v0;
    goto loc_8003D89C;
loc_8003D88C:
    a0 = gFinIncomingLine;
    D_strncpy(vmAddrToPtr<char>(a0), vmAddrToPtr<const char>(a1), a2);
loc_8003D89C:
    v1 = *gFinIncomingLineLen;
    v0 = v1 + 1;
    *gFinIncomingLineLen = v0;
    at = gFinIncomingLine;
    at += v1;
    sb(0, at);
    v0 = 0;                                             // Result = 00000000
    goto loc_8003D8D8;
loc_8003D8C4:
    v0 = s0 & 0xF0;
    if (s0 == s1) goto loc_8003D8D4;
    {
        const bool bJump = (v0 != 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_8003D8D8;
    }
loc_8003D8D4:
    v0 = 0;                                             // Result = 00000000
loc_8003D8D8:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the rendering for the Doom I finale screen (text popping up gradually)
//------------------------------------------------------------------------------------------------------------------------------------------
void F1_Drawer() noexcept {
    // Increment the frame count (for the texture cache) and draw the background
    I_IncDrawnFrameCount();
    I_CacheAndDrawSprite(*gTex_BACK, 0, 0, gPaletteClutIds[MAINPAL]);

    // Show both the incoming and fully displayed text lines
    int32_t ypos = 45;
    
    for (int32_t lineIdx = 0; lineIdx < *gFinLinesDone; ++lineIdx) {
        I_DrawString(-1, ypos, gDoom1WinText[lineIdx]);
        ypos += 14;
    }
    
    I_DrawString(-1, ypos, gFinIncomingLine.get());

    // Not sure why the finale screen would be 'paused'?
    if (*gbGamePaused) {
        I_DrawPausedOverlay();
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

void F2_Start() noexcept {
    sp -= 0x68;
    sw(s0, sp + 0x60);
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7A90;                                       // Result = gTex_LOADING[0] (80097A90)
    a0 = s0;                                            // Result = gTex_LOADING[0] (80097A90)
    a1 = 0x5F;                                          // Result = 0000005F
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    sw(ra, sp + 0x64);
    a2 = 0x6D;                                          // Result = 0000006D
    _thunk_I_DrawLoadingPlaque();
    I_PurgeTexCache();
    a0 = s0 + 0x120;                                    // Result = gTex_DEMON[0] (80097BB0)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7D44;                                       // Result = STR_LumpName_DEMON[0] (80077D44)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    P_LoadBlocks(CdMapTbl_File::MAPSPR60_IMG);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x4A68);                               // Load from: CastInfo_1_ZombieMan[1] (80074A68)
    gFinIncomingLine[0] = 0;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1FB8;                                       // Result = MObjInfo_MT_PLAYER[3] (8005E048)
    at += v0;
    v0 = lw(at);
    *gFinaleStage = F_STAGE_TEXT;  // TODO: give proper enum
    *gFinLinesDone = 0;
    *gFinIncomingLineLen = 0;
    *gCastNum = 0;
    sw(0, gp + 0x998);                                  // Store to: 80077F78
    sw(0, gp + 0xAA8);                                  // Store to: 80078088
    sw(0, gp + 0xB88);                                  // Store to: 80078168
    v1 = v0 << 3;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v1 += v0;
    a1 = lw(v1 + 0x8);
    v0 = 0x2D;                                          // Result = 0000002D
    *gFinTextYPos = v0;
    *gpCastState = v1;
    sw(a1, gp + 0x8F0);                                 // Store to: 80077ED0
    a0 = 0x3C;                                          // Result = 0000003C
    S_LoadSoundAndMusic();
    a2 = 0;                                             // Result = 00000000
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x3E64);                               // Load from: CDTrackNum_Finale_Doom2 (80073E64)
    a1 = *gCdMusicVol;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x3E54);                               // Load from: CDTrackNum_Credits_Demo (80073E54)
    a3 = 0;                                             // Result = 00000000
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    sw(v0, sp + 0x10);
    sw(a1, sp + 0x14);
    psxcd_play_at_andloop();
loc_8003DACC:
    psxcd_elapsed_sectors();
    if (v0 == 0) goto loc_8003DACC;
    ra = lw(sp + 0x64);
    s0 = lw(sp + 0x60);
    sp += 0x68;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called to finish up the DOOM II finale
//------------------------------------------------------------------------------------------------------------------------------------------
void F2_Stop() noexcept {
    *gbGamePaused = false;
    psxcd_stop();
}

void F2_Ticker() noexcept {
    v0 = *gCurPlayerIndex;
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    *gGameAction = ga_nothing;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += v0;
    s0 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gOldTicButtons[0] (80078214)
    at += v0;
    s1 = lw(at);
    P_CheckCheats();
    v0 = *gbGamePaused;
    s2 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8003DB8C;
    v0 = *gGameAction;
    goto loc_8003E30C;
loc_8003DB8C:
    v1 = *gFinaleStage;
    v0 = (i32(v1) < F_STAGE_CAST);
    if (v1 == s2) goto loc_8003DC80;
    if (v0 == 0) goto loc_8003DBB4;
    v0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_8003DBC8;
    goto loc_8003E30C;
loc_8003DBB4:
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (v1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003DCA8;
    }
    goto loc_8003E30C;
loc_8003DBC8:
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 1;
        if (bJump) goto loc_8003E308;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003E30C;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x4950;                                       // Result = STR_Doom2_WinText_1[0] (80074950)
    a0 = *gFinLinesDone;
    a2 = *gFinIncomingLineLen;
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    v0 += a0;
    a1 = v0 + v1;
    v0 = a1 + a2;
    v0 = lbu(v0);
    {
        const bool bJump = (v0 != 0);
        v0 = a0 + 1;
        if (bJump) goto loc_8003DC48;
    }
    *gFinLinesDone = v0;
    v0 = (i32(v0) < 0xB);
    *gFinIncomingLineLen = 0;
    if (v0 != 0) goto loc_8003DC58;
    *gFinaleStage = (finalestage_t) s2;
    goto loc_8003DC58;
loc_8003DC48:
    a0 = gFinIncomingLine;
    D_strncpy(vmAddrToPtr<char>(a0), vmAddrToPtr<const char>(a1), a2);
loc_8003DC58:
    v1 = *gFinIncomingLineLen;
    v0 = v1 + 1;
    *gFinIncomingLineLen = v0;
    at = gFinIncomingLine;
    at += v1;
    sb(0, at);
    v0 = 0;                                             // Result = 00000000
    goto loc_8003E30C;
loc_8003DC80:
    v0 = *gFinTextYPos;
    v0--;
    *gFinTextYPos = v0;
    v0 = (i32(v0) < -0xC8);
    {
        const bool bJump = (v0 == 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8003E308;
    }
    *gFinaleStage = (finalestage_t) v0;
    v0 = 0;                                             // Result = 00000000
    goto loc_8003E30C;
loc_8003DCA8:
    v0 = lw(gp + 0x998);                                // Load from: 80077F78
    if (v0 != 0) goto loc_8003DDA0;
    v0 = s0 & 0xF0;
    if (s0 == s1) goto loc_8003DDA0;
    if (v0 == 0) goto loc_8003DDA0;
    a0 = 0;
    a1 = sfx_shotgn;
    S_StartSound();
    v0 = *gCastNum;
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F8C;                                       // Result = MObjInfo_MT_PLAYER[E] (8005E074)
    at += v0;
    a1 = lw(at);
    if (a1 == 0) goto loc_8003DD2C;
    a0 = 0;
    S_StartSound();
loc_8003DD2C:
    v0 = *gCastNum;
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F94;                                       // Result = MObjInfo_MT_PLAYER[C] (8005E06C)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    v1 = lw(v0 + 0x8);
    sw(s2, gp + 0x998);                                 // Store to: 80077F78
    sw(0, gp + 0xAA8);                                  // Store to: 80078088
    *gpCastState = v0;
    sw(v1, gp + 0x8F0);                                 // Store to: 80077ED0
loc_8003DDA0:
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003E30C;
    }
    v0 = lw(gp + 0x998);                                // Load from: 80077F78
    if (v0 == 0) goto loc_8003DEC0;
    v0 = *gpCastState;
    v0 = lw(v0 + 0x10);
    if (v0 != 0) goto loc_8003DEC0;
    v0 = *gCastNum;
    v0++;
    v1 = v0 << 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A64;                                       // Result = CastInfo_1_ZombieMan[0] (80074A64)
    at += v1;
    v1 = lw(at);
    sw(0, gp + 0x998);                                  // Store to: 80077F78
    *gCastNum = v0;
    if (v1 != 0) goto loc_8003DE1C;
    *gCastNum = 0;
loc_8003DE1C:
    a0 = *gCastNum;
    a0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += a0;
    v1 = lw(at);
    a1 = 0x80060000;                                    // Result = 80060000
    a1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    sw(0, gp + 0xAA8);                                  // Store to: 80078088
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1FB8;                                       // Result = MObjInfo_MT_PLAYER[3] (8005E048)
    at += v0;
    v0 = lw(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += a0;
    a0 = lw(at);
    v1 = v0 << 3;
    v1 -= v0;
    v1 <<= 2;
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 2;
    v0 -= a0;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1FB4;                                       // Result = MObjInfo_MT_PLAYER[4] (8005E04C)
    at += v0;
    v0 = lw(at);
    v1 += a1;
    *gpCastState = v1;
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8003E098;
    a1 = v0;
    goto loc_8003E090;
loc_8003DEC0:
    v0 = lw(gp + 0x8F0);                                // Load from: 80077ED0
    v0--;
    sw(v0, gp + 0x8F0);                                 // Store to: 80077ED0
    {
        const bool bJump = (i32(v0) > 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003E30C;
    }
    v0 = *gpCastState;
    v1 = lw(v0 + 0x10);
    v0 = 0x167;                                         // Result = 00000167
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0x168);
        if (bJump) goto loc_8003E054;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x10A;                                     // Result = 0000010A
        if (bJump) goto loc_8003DF94;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0x10B);
        if (bJump) goto loc_8003E04C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0xDB;                                      // Result = 000000DB
        if (bJump) goto loc_8003DF4C;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0xDC);
        if (bJump) goto loc_8003E03C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0xA0;                                      // Result = 000000A0
        if (bJump) goto loc_8003DF30;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 0xBE;                                      // Result = 000000BE
        if (bJump) goto loc_8003E034;
    }
    a1 = 7;                                             // Result = 00000007
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003DF30:
    v0 = 0x106;                                         // Result = 00000106
    {
        const bool bJump = (v1 == v0);
        v0 = 0x108;                                     // Result = 00000108
        if (bJump) goto loc_8003E044;
    }
    a1 = 0x50;                                          // Result = 00000050
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003DF4C:
    v0 = 0x12F;                                         // Result = 0000012F
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0x130);
        if (bJump) goto loc_8003E064;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x129;                                     // Result = 00000129
        if (bJump) goto loc_8003DF78;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 0x12C;                                     // Result = 0000012C
        if (bJump) goto loc_8003E064;
    }
    a1 = 0x4A;                                          // Result = 0000004A
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003DF78:
    v0 = (v1 < 0x14C);
    {
        const bool bJump = (v0 == 0);
        v0 = (v1 < 0x149);
        if (bJump) goto loc_8003E084;
    }
    a1 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_8003E088;
    a1 = 7;                                             // Result = 00000007
    goto loc_8003E088;
loc_8003DF94:
    v0 = (v1 < 0x1E7);
    {
        const bool bJump = (v0 == 0);
        v0 = (v1 < 0x1E5);
        if (bJump) goto loc_8003DFEC;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x1A5;                                     // Result = 000001A5
        if (bJump) goto loc_8003E06C;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0x1A6);
        if (bJump) goto loc_8003E064;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x182;                                     // Result = 00000182
        if (bJump) goto loc_8003DFD0;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 0x18F;                                     // Result = 0000018F
        if (bJump) goto loc_8003E05C;
    }
    a1 = 0x4A;                                          // Result = 0000004A
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003DFD0:
    v0 = 0x1BB;                                         // Result = 000001BB
    {
        const bool bJump = (v1 == v0);
        v0 = 0x1CB;                                     // Result = 000001CB
        if (bJump) goto loc_8003E064;
    }
    a1 = 0x3B;                                          // Result = 0000003B
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003DFEC:
    v0 = 0x225;                                         // Result = 00000225
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0x226);
        if (bJump) goto loc_8003E07C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x205;                                     // Result = 00000205
        if (bJump) goto loc_8003E018;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 0x223;                                     // Result = 00000223
        if (bJump) goto loc_8003E074;
    }
    a1 = 0xF;                                           // Result = 0000000F
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003E018:
    v0 = 0x227;                                         // Result = 00000227
    {
        const bool bJump = (v1 == v0);
        v0 = 0x23C;                                     // Result = 0000023C
        if (bJump) goto loc_8003E07C;
    }
    a1 = 0x3B;                                          // Result = 0000003B
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003E034:
    a1 = 0x1D;                                          // Result = 0000001D
    goto loc_8003E088;
loc_8003E03C:
    a1 = 8;                                             // Result = 00000008
    goto loc_8003E088;
loc_8003E044:
    a1 = 0x4F;                                          // Result = 0000004F
    goto loc_8003E088;
loc_8003E04C:
    a1 = 0x4E;                                          // Result = 0000004E
    goto loc_8003E088;
loc_8003E054:
    a1 = 0x2E;                                          // Result = 0000002E
    goto loc_8003E088;
loc_8003E05C:
    a1 = 0x35;                                          // Result = 00000035
    goto loc_8003E088;
loc_8003E064:
    a1 = 0x4A;                                          // Result = 0000004A
    goto loc_8003E088;
loc_8003E06C:
    a1 = 7;                                             // Result = 00000007
    goto loc_8003E088;
loc_8003E074:
    a1 = 9;                                             // Result = 00000009
    goto loc_8003E088;
loc_8003E07C:
    a1 = 0xF;                                           // Result = 0000000F
    goto loc_8003E088;
loc_8003E084:
    a1 = 0;                                             // Result = 00000000
loc_8003E088:
    a0 = 0;                                             // Result = 00000000
    if (a1 == 0) goto loc_8003E098;
loc_8003E090:
    S_StartSound();
loc_8003E098:
    v0 = *gpCastState;
    v1 = lw(gp + 0xAA8);                                // Load from: 80078088
    a0 = lw(v0 + 0x10);
    v1++;
    sw(v1, gp + 0xAA8);                                 // Store to: 80078088
    v0 = a0 << 3;
    v0 -= a0;
    v0 <<= 2;
    a0 = 0x80060000;                                    // Result = 80060000
    a0 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += a0;
    *gpCastState = v0;
    v0 = 0xC;                                           // Result = 0000000C
    if (v1 != v0) goto loc_8003E25C;
    v0 = lw(gp + 0xB88);                                // Load from: 80078168
    if (v0 == 0) goto loc_8003E130;
    v0 = *gCastNum;
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F9C;                                       // Result = MObjInfo_MT_PLAYER[A] (8005E064)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 3;
    goto loc_8003E17C;
loc_8003E130:
    v0 = *gCastNum;
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F98;                                       // Result = MObjInfo_MT_PLAYER[B] (8005E068)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 3;
loc_8003E17C:
    v0 -= v1;
    v0 <<= 2;
    v0 += a0;
    *gpCastState = v0;
    v0 = lw(gp + 0xB88);                                // Load from: 80078168
    a0 = *gpCastState;
    v1 = v0 ^ 1;
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    sw(v1, gp + 0xB88);                                 // Store to: 80078168
    if (a0 != v0) goto loc_8003E25C;
    if (v1 == 0) goto loc_8003E200;
    v0 = *gCastNum;
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F9C;                                       // Result = MObjInfo_MT_PLAYER[A] (8005E064)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 3;
    goto loc_8003E24C;
loc_8003E200:
    v0 = *gCastNum;
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F98;                                       // Result = MObjInfo_MT_PLAYER[B] (8005E068)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 3;
loc_8003E24C:
    v0 -= v1;
    v0 <<= 2;
    v0 += a0;
    *gpCastState = v0;
loc_8003E25C:
    v1 = lw(gp + 0xAA8);                                // Load from: 80078088
    v0 = 0x18;                                          // Result = 00000018
    if (v1 == v0) goto loc_8003E280;
    v1 = *gpCastState;
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x619C;                                       // Result = State_S_PLAY[0] (80059E64)
    if (v1 != v0) goto loc_8003E2E4;
loc_8003E280:
    v0 = *gCastNum;
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1FB8;                                       // Result = MObjInfo_MT_PLAYER[3] (8005E048)
    at += v0;
    v1 = lw(at);
    sw(0, gp + 0xAA8);                                  // Store to: 80078088
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    *gpCastState = v0;
loc_8003E2E4:
    v0 = *gpCastState;
    v1 = lw(v0 + 0x8);
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v1, gp + 0x8F0);                                 // Store to: 80077ED0
    {
        const bool bJump = (v1 != v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003E30C;
    }
    v0 = 0xF;                                           // Result = 0000000F
    sw(v0, gp + 0x8F0);                                 // Store to: 80077ED0
loc_8003E308:
    v0 = 0;                                             // Result = 00000000
loc_8003E30C:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the rendering for the Doom II finale screen (text, then cast of characters)
//------------------------------------------------------------------------------------------------------------------------------------------
void F2_Drawer() noexcept {
    // Draw the icon of sin background and increment frame count for the texture cache
    I_IncDrawnFrameCount();
    I_CacheAndDrawSprite(*gTex_DEMON, 0, 0, gPaletteClutIds[MAINPAL]);

    // See whether we are drawing the text or the cast of characters
    if (*gFinaleStage >= F_STAGE_TEXT && *gFinaleStage <= F_STAGE_SCROLLTEXT) {
        // Still showing the text, show both the incoming and fully displayed text lines
        int32_t ypos = *gFinTextYPos;

        for (int32_t lineIdx = 0; lineIdx < *gFinLinesDone; ++lineIdx) {
            I_DrawString(-1, ypos, gDoom2WinText[lineIdx]);
            ypos += 14;
        }
        
        I_DrawString(-1, ypos, gFinIncomingLine.get());
    }
    else if (*gFinaleStage == F_STAGE_CAST) {
        // Showing the cast of character, get the texture for the current sprite to show and cache it
        const state_t& state = **gpCastState;
        const spritedef_t& spriteDef = gSprites[state.sprite];
        const spriteframe_t& spriteFrame = spriteDef.spriteframes[state.frame & FF_FRAMEMASK];

        texture_t& spriteTex = (*gpSpriteTextures)[spriteFrame.lump[0] - *gFirstSpriteLumpNum];
        I_CacheTex(spriteTex);

        // Setup and draw the sprite for the cast character
        POLY_FT4& polyPrim = *(POLY_FT4*) getScratchAddr(128);

        LIBGPU_SetPolyFT4(polyPrim);
        LIBGPU_SetShadeTex(&polyPrim, true);
        polyPrim.clut = gPaletteClutIds[MAINPAL];
        polyPrim.tpage = spriteTex.texPageId;

        const int16_t ypos = 180 - spriteTex.offsetY;
        int16_t xpos;

        if (!spriteFrame.flip[0]) {
            polyPrim.tu0 = spriteTex.texPageCoordX;
            polyPrim.tu1 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;
            polyPrim.tu2 = spriteTex.texPageCoordX;            
            polyPrim.tu3 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;

            xpos = HALF_SCREEN_W - spriteTex.offsetX;
        } else {
            polyPrim.tu0 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;
            polyPrim.tu1 = spriteTex.texPageCoordX;
            polyPrim.tu2 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;            
            polyPrim.tu3 = spriteTex.texPageCoordX;
            
            xpos = HALF_SCREEN_W + spriteTex.offsetX - spriteTex.width;
        }

        LIBGPU_setXY4(polyPrim,
            xpos,                       ypos,
            spriteTex.width + xpos,     ypos,
            xpos,                       spriteTex.height + ypos,
            spriteTex.width + xpos,     spriteTex.height + ypos
        );
        
        polyPrim.tv0 = spriteTex.texPageCoordY;
        polyPrim.tv1 = spriteTex.texPageCoordY;
        polyPrim.tv2 = spriteTex.texPageCoordY + (uint8_t) spriteTex.height - 1;
        polyPrim.tv3 = spriteTex.texPageCoordY + (uint8_t) spriteTex.height - 1;

        I_AddPrim(&polyPrim);

        // Draw screen title and current character name
        I_DrawString(-1, 20, "Cast Of Characters");
        I_DrawString(-1, 208, gCastOrder[*gCastNum].name);
    }

    // Not sure why the finale screen would be 'paused'?
    if (*gbGamePaused) {
        I_DrawPausedOverlay();
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}
