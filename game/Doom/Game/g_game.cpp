#include "g_game.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/doomdef.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/f_finale.h"
#include "Doom/UI/in_main.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_setup.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"
#include "Wess/wessapi.h"

// Helper global holding the result of executing a gameloop via 'MiniLoop'.
// Sometimes this is used in preference to the return action, and sometimes it is used temporarily to hold the return action.
const VmPtr<gameaction_t> gGameAction(0x80077EB4);

// The current skill level and game map
const VmPtr<skill_t> gGameSkill(0x80078258);
const VmPtr<int32_t> gGameMap(0x80078048);

// State for each player and whether they are in the game
const VmPtr<player_t[MAXPLAYERS]> gPlayers(0x800A87EC);
const VmPtr<bool32_t[MAXPLAYERS]> gbPlayerInGame(0x800780AC);

// Current and previous game tick count (15 Hz ticks)
const VmPtr<int32_t> gGameTic(0x8007804C);
const VmPtr<int32_t> gPrevGameTic(0x80077FA4);

// The last tick count we wanted to be at (15 Hz ticks).
// On the PSX if the game was running slow, then we might not have reached this amount.
const VmPtr<int32_t> gLastTgtGameTicCount(0x8007829C);

// Are we playing back or recording a demo?
const VmPtr<bool32_t> gbDemoPlayback(0x80078080);
const VmPtr<bool32_t> gbDemoRecording(0x800781AC);

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays a loading message then loads the current map
//------------------------------------------------------------------------------------------------------------------------------------------
void G_DoLoadLevel() noexcept {
    // Draw the loading plaque
    a0 = 0x80097A90;                // Result = gTexInfo_LOADING[0] (80097A90)
    a1 = 95;
    a2 = 109;
    a3 = lh(0x800A90A4);            // Load from: gPaletteClutId_UI (800A90A4)    
    I_DrawPlaque();

    // TODO: what is this waiting on?
    do {
        a0 = 5;
        wess_seq_status();
        if (v0 == 3) continue;
    
        a0 = 7;
        wess_seq_status();    
        if (v0 == 3) continue;

    } while (false);

    // Loading sound and music
    a0 = *gGameMap;
    S_LoadSoundAndMusic();

    // Initialize the state of each player if required
    {
        const gameaction_t gameAction = *gGameAction;

        player_t* pPlayer = gPlayers.get();
        player_t* const pEndPlayer = pPlayer + MAXPLAYERS;
        bool32_t* pbPlayerInGame = gbPlayerInGame.get();

        do {
            if (*pbPlayerInGame) {
                if ((gameAction == ga_number8) || (gameAction == ga_number4) || (pPlayer->playerstate == PST_DEAD)) {
                    pPlayer->playerstate = PST_REBORN;
                }
            }

            ++pPlayer;
            ++pbPlayerInGame;
        } while (pPlayer < pEndPlayer);
    }

    // Load level data and do level setup
    a0 = *gGameMap;
    a1 = *gGameSkill;
    P_SetupLevel();
    
    // Verify heap integrity
    a0 = lw(0x80078198);        // Load from: gpMainMemZone (80078198)
    Z_CheckHeap();

    *gGameAction = ga_nothing;
}

void G_PlayerFinishLevel() noexcept {
loc_80012F00:
    sp -= 0x18;
    v0 = a0 << 2;
    v0 += a0;
    sw(s0, sp + 0x10);
    s0 = v0 << 4;
    s0 -= v0;
    s0 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    s0 += v0;
    a0 = s0 + 0x30;
    a1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x14);
    a2 = 0x18;                                          // Result = 00000018
    _thunk_D_memset();
    a0 = s0 + 0x48;
    a1 = 0;                                             // Result = 00000000
    a2 = 0x18;                                          // Result = 00000018
    _thunk_D_memset();
    a0 = lw(s0);
    v1 = 0x8FFF0000;                                    // Result = 8FFF0000
    v0 = lw(a0 + 0x64);
    v1 |= 0xFFFF;                                       // Result = 8FFFFFFF
    v0 &= v1;
    sw(v0, a0 + 0x64);
    sw(0, s0 + 0xE4);
    sw(0, s0 + 0xE8);
    sw(0, s0 + 0xD8);
    sw(0, s0 + 0xDC);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void G_PlayerReborn() noexcept {
loc_80012F88:
    sp -= 0x28;
    v0 = a0 << 2;
    v0 += a0;
    sw(s0, sp + 0x10);
    s0 = v0 << 4;
    s0 -= v0;
    s0 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    s0 += v0;
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    s1 = lw(s0 + 0x64);
    s2 = lw(s0 + 0xC8);
    s3 = lw(s0 + 0xCC);
    s4 = lw(s0 + 0xD0);
    a2 = 0x12C;                                         // Result = 0000012C
    _thunk_D_memset();
    a1 = 0;                                             // Result = 00000000
    a0 = 0x80060000;                                    // Result = 80060000
    a0 += 0x70D4;                                       // Result = gMaxAmmo[0] (800670D4)
    v1 = 1;                                             // Result = 00000001
    v0 = 0x64;                                          // Result = 00000064
    sw(v0, s0 + 0x24);
    v0 = 0x32;                                          // Result = 00000032
    sw(v1, s0 + 0xB8);
    sw(v1, s0 + 0xBC);
    sw(0, s0 + 0x4);
    sw(v1, s0 + 0x70);
    sw(v1, s0 + 0x6C);
    sw(v1, s0 + 0x74);
    sw(v1, s0 + 0x78);
    sw(v0, s0 + 0x98);
    sw(s1, s0 + 0x64);
    sw(s2, s0 + 0xC8);
    sw(s3, s0 + 0xCC);
    sw(s4, s0 + 0xD0);
loc_80013030:
    v0 = lw(a0);
    a0 += 4;
    a1++;
    sw(v0, s0 + 0xA8);
    v0 = (i32(a1) < 4);
    s0 += 4;
    if (v0 != 0) goto loc_80013030;
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void G_DoReborn() noexcept {
loc_80013070:
    v0 = lw(gp + 0xA7C);                                // Load from: gNetGame (8007805C)
    sp -= 0x30;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(ra, sp + 0x28);
    sw(s5, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    if (v0 != 0) goto loc_800130AC;
    *gGameAction = ga_died;
    goto loc_8001335C;
loc_800130AC:
    v0 = s3 << 2;
    a0 = v0 + s3;
    v0 = a0 << 4;
    v0 -= a0;
    s2 = v0 << 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    at += s2;
    v1 = lw(at);
    v0 = lw(v1 + 0x80);
    if (v0 == 0) goto loc_800130E8;
    sw(0, v1 + 0x80);
loc_800130E8:
    v1 = lw(gp + 0xA7C);                                // Load from: gNetGame (8007805C)
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (v1 != v0);
        v1 = a0 << 1;
        if (bJump) goto loc_800131E4;
    }
    s0 = 0;                                             // Result = 00000000
    s4 = s2;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7FA0);                               // Load from: gpDeathmatchP (80078060)
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 -= 0x7F94;                                       // Result = gDeathmatchStarts[0] (8009806C)
    v1 -= v0;
    v0 = v1 << 1;
    v0 += v1;
    v1 = v0 << 4;
    v0 += v1;
    v1 = v0 << 8;
    v0 += v1;
    v1 = v0 << 16;
    v0 += v1;
    v0 = -v0;
    s2 = u32(i32(v0) >> 1);
loc_8001313C:
    P_Random();
    div(v0, s2);
    if (s2 != 0) goto loc_80013154;
    _break(0x1C00);
loc_80013154:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s2 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001316C;
    }
    if (v0 != at) goto loc_8001316C;
    tge(zero, zero, 0x5D);
loc_8001316C:
    v1 = hi;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    at += s4;
    a0 = lw(at);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 1;
    v1 = 0x800A0000;                                    // Result = 800A0000
    v1 -= 0x7F94;                                       // Result = gDeathmatchStarts[0] (8009806C)
    s1 = v0 + v1;
    a1 = lh(s1);
    a2 = lh(s1 + 0x2);
    a1 <<= 16;
    a2 <<= 16;
    P_CheckPosition();
    s0++;
    if (v0 != 0) goto loc_800131D8;
    v0 = (i32(s0) < 0x10);
    {
        const bool bJump = (v0 != 0);
        v0 = s3 << 2;
        if (bJump) goto loc_8001313C;
    }
    v0 += s3;
    v0 <<= 1;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7184;                                       // Result = gPlayer1MapThing[0] (800A8E7C)
    s1 = v0 + v1;
    goto loc_80013278;
loc_800131D8:
    v0 = s3 + 1;
    sh(v0, s1 + 0x6);
    goto loc_80013278;
loc_800131E4:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7184;                                       // Result = gPlayer1MapThing[0] (800A8E7C)
    s1 = v1 + v0;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    at += s2;
    a0 = lw(at);
    a1 = lh(s1);
    a2 = lh(s1 + 0x2);
    a1 <<= 16;
    a2 <<= 16;
    P_CheckPosition();
    s0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80013278;
    s5 = s2;
    s2 = 0;                                             // Result = 00000000
loc_80013224:
    s4 = 0x800B0000;                                    // Result = 800B0000
    s4 -= 0x7184;                                       // Result = gPlayer1MapThing[0] (800A8E7C)
    s1 = s2 + s4;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    at += s5;
    a0 = lw(at);
    a1 = lh(s1);
    a2 = lh(s1 + 0x2);
    a1 <<= 16;
    a2 <<= 16;
    P_CheckPosition();
    s0++;
    if (v0 != 0) goto loc_800131D8;
    v0 = (i32(s0) < 2);
    s2 += 0xA;
    if (v0 != 0) goto loc_80013224;
    v0 = s3 << 2;
    v0 += s3;
    v0 <<= 1;
    s1 = v0 + s4;
loc_80013278:
    a0 = s1;
    P_SpawnPlayer();
    s0 = 0;                                             // Result = 00000000
    v0 = s0 << 2;                                       // Result = 00000000
loc_80013288:
    v0 += s0;
    v0 <<= 1;
    v1 = s0 + 1;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x717E;                                       // Result = gPlayer1MapThing[3] (800A8E82)
    at += v0;
    sh(v1, at);
    s0 = v1;
    v0 = (i32(s0) < 2);
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 2;
        if (bJump) goto loc_80013288;
    }
    a0 = lh(s1);
    a1 = lh(s1 + 0x2);
    a0 <<= 16;
    a1 <<= 16;
    R_PointInSubsector();
    v1 = 0x6C160000;                                    // Result = 6C160000
    a0 = lh(s1 + 0x4);
    v1 |= 0xC16D;                                       // Result = 6C16C16D
    multu(a0, v1);
    a3 = 0x1D;                                          // Result = 0000001D
    a1 = lh(s1);
    v0 = lw(v0);
    a1 <<= 16;
    a2 = lw(v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 = hi;
    a0 -= v1;
    a0 >>= 1;
    v1 += a0;
    v1 <<= 7;
    v1 &= 0x7000;
    v0 += v1;
    v0 = lw(v0);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v1;
    v1 = lw(at);
    a0 = v0 << 2;
    a0 += v0;
    a0 <<= 2;
    a0 += a1;
    a1 = v1 << 2;
    a1 += v1;
    v0 = lh(s1 + 0x2);
    a1 <<= 2;
    v0 <<= 16;
    a1 += v0;
    P_SpawnMObj();
    a0 = v0;
    a1 = 0x1B;                                          // Result = 0000001B
    S_StartSound();
loc_8001335C:
    ra = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x30;
    return;
}

void G_SetGameComplete() noexcept {
    *gGameAction = ga_number2;
}

void G_InitNew() noexcept {
loc_80013394:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C08);                               // Load from: gLockedTexPagesMask (80077C08)
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(s1, sp + 0x14);
    sw(ra, sp + 0x1C);
    sw(0, gp + 0xA14);                                  // Store to: gbIsLevelBeingRestarted (80077FF4)
    v0 &= 1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C08);                                // Store to: gLockedTexPagesMask (80077C08)
    s1 = a2;
    I_ResetTexCache();
    a0 = *gpMainMemZone;
    a1 = 0x2E;                                          // Result = 0000002E
    Z_FreeTags();
    M_ClearRandom();
    v1 = 2;                                             // Result = 00000002
    v0 = 0x12C;                                         // Result = 0000012C
    *gGameMap = s0;
    *gGameSkill = (skill_t) s2;
    sw(s1, gp + 0xA7C);                                 // Store to: gNetGame (8007805C)
loc_800133FC:
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7810;                                       // Result = gPlayer1[1] (800A87F0)
    at += v0;
    sw(v1, at);
    v0 -= 0x12C;
    a1 = 0;                                             // Result = 00000000
    if (i32(v0) >= 0) goto loc_800133FC;
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x61D0;                                       // Result = gUnusedGameBuffer[0] (800A9E30)
    a0 = s0;                                            // Result = gUnusedGameBuffer[0] (800A9E30)
    a2 = 0x94;                                          // Result = 00000094
    _thunk_D_memset();
    v1 = 1;                                             // Result = 00000001
    at = 0x800B0000;                                    // Result = 800B0000
    sw(s0, at - 0x76E8);                                // Store to: gPlayer2[0] (800A8918)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(s0, at - 0x7814);                                // Store to: gPlayer1[0] (800A87EC)
    sw(v1, gp + 0xACC);                                 // Store to: gbPlayerInGame[0] (800780AC)
    v0 = (s1 < 3);
    if (s1 != 0) goto loc_8001346C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3E0C;                                       // Result = gBtnBinding_Attack (80073E0C)
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7FC8);                                // Store to: MAYBE_gpButtonBindings_Player1 (80077FC8)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7F50);                                 // Store to: gbPlayerInGame[1] (800780B0)
    v0 = 4;                                             // Result = 00000004
    goto loc_8001347C;
loc_8001346C:
    {
        const bool bJump = (v0 == 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_8001347C;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7F50);                                // Store to: gbPlayerInGame[1] (800780B0)
loc_8001347C:
    *gbDemoRecording = false;
    *gbDemoPlayback = false;
    if (s2 != v0) goto loc_800134C8;
    v0 = 2;                                             // Result = 00000002
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x4850);                                // Store to: State_S_SARG_ATK1[2] (8005B7B0)
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x4834);                                // Store to: State_S_SARG_ATK2[2] (8005B7CC)
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x4818);                                // Store to: State_S_SARG_ATK3[2] (8005B7E8)
    v0 = 0xF;                                           // Result = 0000000F
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x1C18);                                // Store to: MObjInfo_MT_SERGEANT[F] (8005E3E8)
    v0 = 0x280000;                                      // Result = 00280000
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x17F8);                                // Store to: MObjInfo_MT_BRUISERSHOT[F] (8005E808)
    goto loc_800134FC;
loc_800134C8:
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x4850);                                // Store to: State_S_SARG_ATK1[2] (8005B7B0)
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x4834);                                // Store to: State_S_SARG_ATK2[2] (8005B7CC)
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x4818);                                // Store to: State_S_SARG_ATK3[2] (8005B7E8)
    v0 = 0xA;                                           // Result = 0000000A
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x1C18);                                // Store to: MObjInfo_MT_SERGEANT[F] (8005E3E8)
    v0 = 0x1E0000;                                      // Result = 001E0000
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x17F8);                                // Store to: MObjInfo_MT_BRUISERSHOT[F] (8005E808)
    v0 = 0x140000;                                      // Result = 00140000
loc_800134FC:
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x1850);                                // Store to: MObjInfo_MT_HEADSHOT[F] (8005E7B0)
    at = 0x80060000;                                    // Result = 80060000
    sw(v0, at - 0x18A8);                                // Store to: MObjInfo_MT_TROOPSHOT[F] (8005E758)
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void G_RunGame() noexcept {
loc_80013528:
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);

loc_8001353C:
    G_DoLoadLevel();
    MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);

    v1 = *gGameAction;
    v0 = 6;                                             // Result = 00000006
    sw(0, gp + 0xA14);                                  // Store to: gbIsLevelBeingRestarted (80077FF4)
    s2 = 4;                                             // Result = 00000004
    if (v1 != v0) goto loc_80013588;

    // Who knows what this function originally did...
    empty_func1();

loc_80013588:
    v0 = *gGameAction;
    v1 = 1;                                             // Result = 00000001
    if (v0 == s2) goto loc_8001353C;
    s1 = 8;                                             // Result = 00000008
    if (v0 == v1) goto loc_800135A8;
    if (v0 != s1) goto loc_800135B4;

loc_800135A8:
    sw(v1, gp + 0xA14);                                 // Store to: gbIsLevelBeingRestarted (80077FF4)
    goto loc_8001353C;

loc_800135B4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C08);                               // Load from: gLockedTexPagesMask (80077C08)
    a0 = *gpMainMemZone;
    v0 &= 1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C08);                                // Store to: gLockedTexPagesMask (80077C08)
    a1 = 8;                                             // Result = 00000008
    Z_FreeTags();
    v0 = *gGameAction;
    s0 = 5;                                             // Result = 00000005
    if (v0 == s0) goto loc_800136F8;
    
    MiniLoop(IN_Start, IN_Stop, IN_Ticker, IN_Drawer);

    v0 = lw(gp + 0xA7C);                                // Load from: gNetGame (8007805C)
    {
        const bool bJump = (v0 != 0);
        v0 = 0x1E;                                      // Result = 0000001E
        if (bJump) goto loc_80013698;
    }
    v1 = *gGameMap;
    {
        const bool bJump = (v1 != v0);
        v0 = 0x1F;                                      // Result = 0000001F
        if (bJump) goto loc_80013698;
    }
    v1 = lw(gp + 0xAB8);                                // Load from: gNextMap (80078098)
    {
        const bool bJump = (v1 != v0);
        v0 = (i32(v1) < 0x3C);
        if (bJump) goto loc_800136A4;
    }
    
    MiniLoop(F1_Start, F1_Stop, F1_Ticker, F1_Drawer);

    v0 = *gGameAction;
    if (v0 == s2) goto loc_8001353C;
    if (v0 == s1) goto loc_8001353C;
    {
        const bool bJump = (v0 == s0);
        v0 = -2;                                        // Result = FFFFFFFE
        if (bJump) goto loc_800136F8;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
    goto loc_800136F8;

loc_80013698:
    v1 = lw(gp + 0xAB8);                                // Load from: gNextMap (80078098)
    v0 = (i32(v1) < 0x3C);

loc_800136A4:
    if (v0 == 0) goto loc_800136B8;
    *gGameMap = v1;
    goto loc_8001353C;

loc_800136B8:
    MiniLoop(F2_Start, F2_Stop, F2_Ticker, F2_Drawer);

    v1 = *gGameAction;
    v0 = 4;                                             // Result = 00000004
    {
        const bool bJump = (v1 == v0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8001353C;
    }
    if (v1 == v0) goto loc_8001353C;

loc_800136F8:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void G_PlayDemoPtr() noexcept {
loc_80013714:
    sp -= 0x40;
    a0 = sp + 0x10;
    sw(s1, sp + 0x34);
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x3E0C;                                       // Result = gBtnBinding_Attack (80073E0C)
    a1 = s1;                                            // Result = gBtnBinding_Attack (80073E0C)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x75E8);                               // Load from: gpDemoBuffer (800775E8)
    sw(ra, sp + 0x3C);
    sw(s2, sp + 0x38);
    sw(s0, sp + 0x30);
    v0 = v1 + 4;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x75EC);                                // Store to: gpDemo_p (800775EC)
    s2 = lw(v1);
    v0 = v1 + 8;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x75EC);                                // Store to: gpDemo_p (800775EC)
    s0 = lw(v1 + 0x4);
    a2 = 0x20;                                          // Result = 00000020
    _thunk_D_memcpy();
    a0 = s1;                                            // Result = gBtnBinding_Attack (80073E0C)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75EC);                               // Load from: gpDemo_p (800775EC)
    a2 = 0x20;                                          // Result = 00000020
    _thunk_D_memcpy();
    a0 = s2;
    a1 = s0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x75EC);                               // Load from: gpDemo_p (800775EC)
    v0 += 0x20;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x75EC);                                // Store to: gpDemo_p (800775EC)
    a2 = 0;                                             // Result = 00000000
    G_InitNew();
    G_DoLoadLevel();

    *gbDemoPlayback = true;
    s0 = MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);

    a0 = s1;                    // Result = gBtnBinding_Attack (80073E0C)
    a1 = sp + 0x10;
    a2 = 0x20;
    *gbDemoPlayback = false;

    _thunk_D_memcpy();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C08);                               // Load from: gLockedTexPagesMask (80077C08)
    a0 = *gpMainMemZone;
    v0 &= 1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C08);                                // Store to: gLockedTexPagesMask (80077C08)
    a1 = 0x2E;                                          // Result = 0000002E
    Z_FreeTags();
    v0 = s0;
    ra = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x40;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// An unknown function called prior to the level being restarted.
// Since it's contents are empty in the retail .EXE we cannot deduce what it originally did.
// It's likely some sort of debug functionality that got compiled out of the retail build.
//------------------------------------------------------------------------------------------------------------------------------------------
void empty_func1() noexcept {
}
