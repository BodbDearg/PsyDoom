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
#include "info.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_setup.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"
#include "Wess/wessapi.h"

// Helper global holding the result of executing a gameloop via 'MiniLoop'.
// Sometimes this is used in preference to the return action, and sometimes it is used temporarily to hold the return action.
const VmPtr<gameaction_t> gGameAction(0x80077EB4);

// The current skill level. game type, game map, and the next/upcoming map
const VmPtr<skill_t>        gGameSkill(0x80078258);
const VmPtr<gametype_t>     gNetGame(0x8007805C);
const VmPtr<int32_t>        gGameMap(0x80078048);
const VmPtr<int32_t>        gNextMap(0x80078098);

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

// Is the level being restarted?
const VmPtr<bool32_t> gbIsLevelBeingRestarted(0x80077FF4);

// An empty map object initially assigned to players during network game setup, for net consistency checks.
// This is all zeroed out initially.
const VmPtr<mobj_t> gEmptyMObj(0x800A9E30);

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

    // And and setup the level, then verify the heap after all that is done
    P_SetupLevel(*gGameMap, *gGameSkill);
    Z_CheckHeap(**gpMainMemZone);

    // No action set upon starting a level
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
    v0 = *gNetGame;
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
    v1 = *gNetGame;
    v0 = 2;
    {
        const bool bJump = (v1 != v0);
        v1 = a0 << 1;
        if (bJump) goto loc_800131E4;
    }
    s0 = 0;
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
    _thunk_P_Random();
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Common game setup logic for both demos and regular gameplay
//------------------------------------------------------------------------------------------------------------------------------------------
void G_InitNew(const skill_t skill, const int32_t mapNum, const gametype_t gameType) noexcept {
    // Resetting memory management related stuff and RNGs
    *gbIsLevelBeingRestarted = false;
    *gLockedTexPagesMask &= 1;
    I_ResetTexCache();    

    Z_FreeTags(**gpMainMemZone, PU_CACHE|PU_ANIMATION|PU_LEVSPEC|PU_LEVEL);
    M_ClearRandom();

    // Save game params: note that in multiplayer these might be overriden later
    *gGameMap = mapNum;
    *gGameSkill = skill;
    *gNetGame = gameType;

    // Mark all players as reborn
    {
        player_t* pPlayer = &gPlayers[MAXPLAYERS - 1];
        player_t* const pFirstPlayer = &gPlayers[0];

        while (pPlayer >= pFirstPlayer) {
            pPlayer->playerstate = PST_REBORN;
            --pPlayer;
        }
    }

    // Clear the empty map object and assign to both players initially.
    // This is used for network consistency checks:
    D_memset(gEmptyMObj.get(), (std::byte) 0, sizeof(mobj_t));
    gPlayers[0].mo = gEmptyMObj;
    gPlayers[1].mo = gEmptyMObj;

    // Set some player status flags and controls stuff
    gbPlayerInGame[0] = true;

    if (gameType == gt_single) {
        gpPlayerBtnBindings[0] = gBtnBindings;
        gbPlayerInGame[1] = false;
    } 
    else if (gameType == gt_deathmatch || gameType == gt_coop) {
        gbPlayerInGame[2] = true;
    }

    // Not recording or playing back a demo (yet)
    *gbDemoRecording = false;
    *gbDemoPlayback = false;

    // Patching some monster states depending on difficulty
    if (skill == sk_nightmare) {
        gStates[S_SARG_ATK1].tics = 2;
        gStates[S_SARG_ATK2].tics = 2;
        gStates[S_SARG_ATK3].tics = 2;        
        gMObjInfo[MT_SERGEANT].speed = 15;
        gMObjInfo[MT_BRUISERSHOT].speed = 40 * FRACUNIT;
        gMObjInfo[MT_HEADSHOT].speed = 40 * FRACUNIT;
        gMObjInfo[MT_TROOPSHOT].speed = 40 * FRACUNIT;
    } else {
        gStates[S_SARG_ATK1].tics = 4;
        gStates[S_SARG_ATK2].tics = 4;
        gStates[S_SARG_ATK3].tics = 4;
        gMObjInfo[MT_SERGEANT].speed = 10;
        gMObjInfo[MT_BRUISERSHOT].speed = 30 * FRACUNIT;
        gMObjInfo[MT_HEADSHOT].speed = 20 * FRACUNIT;
        gMObjInfo[MT_TROOPSHOT].speed = 20 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Run the actual in-game (3d) portions of the game.
// Also do intermission and finale screens following a level.
// This is used to run the game for non-demo gameplay.
//------------------------------------------------------------------------------------------------------------------------------------------
void G_RunGame() noexcept {
    while (true) {
        // Load the level and run the game
        G_DoLoadLevel();
        MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);
    
        *gbIsLevelBeingRestarted = false;
    
        if (*gGameAction == ga_number6) {
            // Who knows what this function originally did...
            empty_func1();
        }
    
        if (*gGameAction == ga_number4)
            continue;
    
        if (*gGameAction == ga_died || *gGameAction == ga_number8) {
            *gbIsLevelBeingRestarted = true;
            continue;
        }
        
        // Cleanup after the level is done
        *gLockedTexPagesMask &= 1;
        Z_FreeTags(**gpMainMemZone, PU_ANIMATION);
        
        if (*gGameAction == ga_number5)
            break;
        
        // Do the intermission
        MiniLoop(IN_Start, IN_Stop, IN_Ticker, IN_Drawer);

        // Should we do the Ultimate DOOM finale?
        if (*gNetGame == gt_single && *gGameMap == 30 && *gNextMap == 31) {    
            MiniLoop(F1_Start, F1_Stop, F1_Ticker, F1_Drawer);

            if (*gGameAction == ga_number4 || *gGameAction == ga_number8)
                continue;
    
            if (*gGameAction == ga_number5)
                break;

            *gStartMapOrEpisode = -2;
            break;
        }

        // If there is a next map go onto it, otherwise show the DOOM II finale
        if (*gNextMap < 60) {
            *gGameMap = *gNextMap;
            continue;
        }

        MiniLoop(F2_Start, F2_Stop, F2_Ticker, F2_Drawer);

        if (*gGameAction != ga_number4 && *gGameAction != ga_number8)
            break;
    }
}

void G_PlayDemoPtr() noexcept {
loc_80013714:
    sp -= 0x40;
    a0 = sp + 0x10;
    sw(s1, sp + 0x34);
    s1 = gBtnBindings;
    a1 = s1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = *gpDemoBuffer;
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
    a0 = s1;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75EC);                               // Load from: gpDemo_p (800775EC)
    a2 = 0x20;                                          // Result = 00000020
    _thunk_D_memcpy();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x75EC);                               // Load from: gpDemo_p (800775EC)
    v0 += 0x20;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x75EC);                                // Store to: gpDemo_p (800775EC)
    G_InitNew((skill_t) s2, (int32_t) s0, gt_single);
    G_DoLoadLevel();

    *gbDemoPlayback = true;
    s0 = MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);
    *gbDemoPlayback = false;

    a0 = s1;
    a1 = sp + 0x10;
    a2 = 0x20;
    _thunk_D_memcpy();

    *gLockedTexPagesMask &= 1;
    Z_FreeTags(**gpMainMemZone, PU_CACHE|PU_ANIMATION|PU_LEVSPEC|PU_LEVEL);

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
