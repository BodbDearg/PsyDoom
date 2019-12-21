#include "p_tick.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/am_main.h"
#include "Doom/UI/o_main.h"
#include "Doom/UI/st_main.h"
#include "g_game.h"
#include "p_base.h"
#include "p_mobj.h"
#include "p_sight.h"
#include "p_spec.h"
#include "p_user.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"
#include "Wess/psxspu.h"
#include "Wess/wessapi.h"

void P_AddThinker() noexcept {
loc_80028C38:
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x6550);                               // Load from: gThinkerCap[0] (80096550)
    sw(a0, v0 + 0x4);
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    sw(v0, a0 + 0x4);
    v1 = lw(v0);                                        // Load from: gThinkerCap[0] (80096550)
    sw(v1, a0);
    sw(a0, v0);                                         // Store to: gThinkerCap[0] (80096550)
    return;
}

void P_RemoveThinker() noexcept {
loc_80028C68:
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, a0 + 0x8);
    return;
}

void P_RunThinkers() noexcept {
    sp -= 0x20;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6554;                                       // Result = gThinkerCap[1] (80096554)
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    s0 = lw(v0);                                        // Load from: gThinkerCap[1] (80096554)
    v0 -= 4;                                            // Result = gThinkerCap[0] (80096550)
    sw(0, gp + 0xD14);                                  // Store to: gNumActiveThinkers (800782F4)
    s2 = -1;                                            // Result = FFFFFFFF
    if (s0 == v0) goto loc_80028D14;
    s1 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_80028CA8:
    v0 = lw(s0 + 0x8);
    a1 = s0;
    if (v0 != s2) goto loc_80028CE4;
    v1 = lw(s0 + 0x4);
    v0 = lw(s0);
    a0 = *gpMainMemZone;
    sw(v0, v1);
    v1 = lw(s0);
    v0 = lw(s0 + 0x4);
    sw(v0, v1 + 0x4);
    _thunk_Z_Free2();
    goto loc_80028D04;
loc_80028CE4:
    if (v0 == 0) goto loc_80028CF4;
    a0 = s0;
    ptr_call(v0);
loc_80028CF4:
    v0 = lw(gp + 0xD14);                                // Load from: gNumActiveThinkers (800782F4)
    v0++;
    sw(v0, gp + 0xD14);                                 // Store to: gNumActiveThinkers (800782F4)
loc_80028D04:
    s0 = lw(s0 + 0x4);
    if (s0 != s1) goto loc_80028CA8;
loc_80028D14:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_RunMobjLate() noexcept {
    sp -= 0x20;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    a0 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 -= 0x14;                                         // Result = gMObjHead[0] (800A8E90)
    s1 = v0;                                            // Result = gMObjHead[0] (800A8E90)
    if (a0 == v0) goto loc_80028D7C;
loc_80028D58:
    v0 = lw(a0 + 0x18);
    s0 = lw(a0 + 0x14);
    if (v0 == 0) goto loc_80028D70;
    ptr_call(v0);
loc_80028D70:
    a0 = s0;
    if (a0 != s1) goto loc_80028D58;
loc_80028D7C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_CheckCheats() noexcept {
loc_80028D94:
    sp -= 0x40;
    sw(s0, sp + 0x20);
    s0 = 1;                                             // Result = 00000001
    sw(s5, sp + 0x34);
    s5 = 0x800B0000;                                    // Result = 800B0000
    s5 -= 0x7754;                                       // Result = gPlayer1[30] (800A88AC)
    sw(s6, sp + 0x38);
    s6 = -0x31;                                         // Result = FFFFFFCF
    sw(s3, sp + 0x2C);
    s3 = 0x12C;                                         // Result = 0000012C
    sw(s1, sp + 0x24);
    s1 = 4;                                             // Result = 00000004
    sw(ra, sp + 0x3C);
    sw(s4, sp + 0x30);
    sw(s2, sp + 0x28);
loc_80028DD0:
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7F54;                                       // Result = gbPlayerInGame[0] (800780AC)
    at += s1;
    v0 = lw(at);
    if (v0 == 0) goto loc_80028F34;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gPlayerPadButtons[0] (80077F44)
    at += s1;
    s2 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gPlayerOldPadButtons[0] (80078214)
    at += s1;
    s4 = lw(at);
    v0 = s2 & 0x800;
    {
        const bool bJump = (v0 == 0);
        v0 = s4 & 0x800;
        if (bJump) goto loc_80028E90;
    }
    if (v0 != 0) goto loc_80028E90;
    v0 = lw(gp + 0x8E0);                                // Load from: gbGamePaused (80077EC0)
    v0 ^= 1;
    sw(v0, gp + 0x8E0);                                 // Store to: gbGamePaused (80077EC0)
    if (v0 != 0) goto loc_800290B0;
    a0 = 0;                                             // Result = 00000000
    psxcd_restart();
loc_80028E40:
    psxcd_seeking_for_play();
    if (v0 != 0) goto loc_80028E40;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75F8);                               // Load from: gCdMusicVol (800775F8)
    a0 = 0x1F4;                                         // Result = 000001F4
    psxspu_start_cd_fade();
    S_Resume();
    v0 = lw(s5);                                        // Load from: gPlayer1[30] (800A88AC)
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D28);                               // Load from: gTicConOnPause (800782D8)
    v0 &= s6;
    at = 0x80080000;                                    // Result = 80080000
    *gTicCon = v1;
    v1 = u32(i32(v1) >> 2);
    sw(v0, s5);                                         // Store to: gPlayer1[30] (800A88AC)
    at = 0x80080000;                                    // Result = 80080000
    *gLastTgtGameTicCount = v1;
loc_80028E90:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    a3 = s3 + v0;
    v0 = s2 & 0x100;
    {
        const bool bJump = (v0 == 0);
        v0 = s4 & 0x100;
        if (bJump) goto loc_80028F34;
    }
    if (v0 != 0) goto loc_80028F34;
    v0 = lw(gp + 0x8E0);                                // Load from: gbGamePaused (80077EC0)
    if (v0 == 0) goto loc_80028F34;
    v0 = lw(a3 + 0xC0);
    v0 &= s6;
    sw(v0, a3 + 0xC0);
    I_DrawPresent();

    v1 = MiniLoop(O_Init, O_Shutdown, O_Control, O_Drawer);        
    v0 = 9;

    {
        const bool bJump = (v1 == v0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_800293E8;
    }
    *gGameAction = (gameaction_t) v1;
    {
        const bool bJump = (v1 == v0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_80028F24;
    }
    if (v1 != v0) goto loc_800293E8;
loc_80028F24:
    O_Drawer();
    goto loc_800293E8;
loc_80028F34:
    s3 -= 0x12C;
    s0--;
    s1 -= 4;
    if (i32(s0) >= 0) goto loc_80028DD0;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FA4);                               // Load from: gNetGame (8007805C)
    if (v0 != 0) goto loc_800293E8;
    if (s2 != 0) goto loc_80028F68;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7EF8);                                 // Store to: gVBlanksUntilMenuMove (80077EF8)
loc_80028F68:
    v1 = lw(a3 + 0xC0);
    v0 = v1 & 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 0x10;
        if (bJump) goto loc_80029044;
    }
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7EF8;                                       // Result = gVBlanksUntilMenuMove (80077EF8)
    v0 = lw(a0);                                        // Load from: gVBlanksUntilMenuMove (80077EF8)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FBC);                               // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v0 -= v1;
    sw(v0, a0);                                         // Store to: gVBlanksUntilMenuMove (80077EF8)
    if (i32(v0) > 0) goto loc_80029000;
    v0 = s2 & 0x8000;
    {
        const bool bJump = (v0 == 0);
        v0 = s2 & 0x2000;
        if (bJump) goto loc_80028FCC;
    }
    v0 = lw(gp + 0xC90);                                // Load from: gMapNumToCheatWarpTo (80078270)
    v0--;
    sw(v0, gp + 0xC90);                                 // Store to: gMapNumToCheatWarpTo (80078270)
    {
        const bool bJump = (i32(v0) > 0);
        v0 = 0xF;                                       // Result = 0000000F
        if (bJump) goto loc_80028FFC;
    }
    v0 = 1;                                             // Result = 00000001
    goto loc_80028FF4;
loc_80028FCC:
    if (v0 == 0) goto loc_80029000;
    v0 = lw(gp + 0xC90);                                // Load from: gMapNumToCheatWarpTo (80078270)
    v0++;
    sw(v0, gp + 0xC90);                                 // Store to: gMapNumToCheatWarpTo (80078270)
    v0 = (i32(v0) < 0x37);
    {
        const bool bJump = (v0 != 0);
        v0 = 0xF;                                       // Result = 0000000F
        if (bJump) goto loc_80028FFC;
    }
    v0 = 0x36;                                          // Result = 00000036
loc_80028FF4:
    sw(v0, gp + 0xC90);                                 // Store to: gMapNumToCheatWarpTo (80078270)
    v0 = 0xF;                                           // Result = 0000000F
loc_80028FFC:
    sw(v0, a0);                                         // Store to: gVBlanksUntilMenuMove (80077EF8)
loc_80029000:
    v0 = s2 & 0xF0;
    if (s2 == s4) goto loc_800293E8;
    {
        const bool bJump = (v0 == 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_800293E8;
    }
    a0 = -0x21;                                         // Result = FFFFFFDF
    *gGameAction = (gameaction_t) v0;
    v0 = lw(a3 + 0xC0);
    v1 = lw(gp + 0xC90);                                // Load from: gMapNumToCheatWarpTo (80078270)
    v0 &= a0;
    sw(v0, a3 + 0xC0);
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
    *gGameMap = v1;
    goto loc_800293E8;
loc_80029044:
    if (v0 == 0) goto loc_800290EC;
    v0 = s2 & 0x8000;
    if (s2 == s4) goto loc_800293E8;
    {
        const bool bJump = (v0 == 0);
        v0 = s2 & 0x2000;
        if (bJump) goto loc_80029080;
    }
    v0 = lw(gp + 0x8F4);                                // Load from: gVramViewerTexPage (80077ED4)
    v0--;
    sw(v0, gp + 0x8F4);                                 // Store to: gVramViewerTexPage (80077ED4)
    if (i32(v0) >= 0) goto loc_800293E8;
    sw(0, gp + 0x8F4);                                  // Store to: gVramViewerTexPage (80077ED4)
    goto loc_800293E8;
loc_80029080:
    if (v0 == 0) goto loc_800293E8;
    v0 = lw(gp + 0x8F4);                                // Load from: gVramViewerTexPage (80077ED4)
    v0++;
    sw(v0, gp + 0x8F4);                                 // Store to: gVramViewerTexPage (80077ED4)
    v0 = (i32(v0) < 0xB);
    {
        const bool bJump = (v0 != 0);
        v0 = 0xA;                                       // Result = 0000000A
        if (bJump) goto loc_800293E8;
    }
    sw(v0, gp + 0x8F4);                                 // Store to: gVramViewerTexPage (80077ED4)
    goto loc_800293E8;
loc_800290B0:
    psxcd_pause();
    a0 = 0xD;                                           // Result = 0000000D
    wess_seq_stop();
    a0 = 0xE;                                           // Result = 0000000E
    wess_seq_stop();
    S_Pause();
    v0 = *gTicCon;
    sw(0, gp + 0xA04);                                  // Store to: gCurCheatBtnSequenceIdx (80077FE4)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7D28);                                // Store to: gTicConOnPause (800782D8)
    goto loc_800293E8;
loc_800290EC:
    v0 = lw(gp + 0x8E0);                                // Load from: gbGamePaused (80077EC0)
    if (v0 == 0) goto loc_800293E8;
    if (s2 == 0) goto loc_800293E8;
    s0 = 0;                                             // Result = 00000000
    if (s2 == s4) goto loc_800293E8;
    t6 = 0x800B0000;                                    // Result = 800B0000
    t6 -= 0x6E5C;                                       // Result = gCheatSequenceBtns[0] (800A91A4)
    t3 = 0x800A0000;                                    // Result = 800A0000
    t3 -= 0x78C0;                                       // Result = gpStatusBarMsgStr (80098740)
    t4 = t3 + 4;                                        // Result = gStatusBarMsgTicsLeft (80098744)
    t2 = 1;                                             // Result = 00000001
    t5 = 0x80010000;                                    // Result = 80010000
    t5 += 0x1120;                                       // Result = JumpTable_P_CheckCheats_2[0] (80011120)
    v0 = lw(gp + 0xA04);                                // Load from: gCurCheatBtnSequenceIdx (80077FE4)
    t1 = 0x80060000;                                    // Result = 80060000
    t1 += 0x7778;                                       // Result = CheatSequenceButtons[0] (80067778)
    v1 = v0 + 1;
    v0 <<= 1;
    sw(v1, gp + 0xA04);                                 // Store to: gCurCheatBtnSequenceIdx (80077FE4)
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6E5C;                                       // Result = gCheatSequenceBtns[0] (800A91A4)
    at += v0;
    sh(s2, at);
loc_80029154:
    t0 = lw(gp + 0xA04);                                // Load from: gCurCheatBtnSequenceIdx (80077FE4)
    a0 = 0;                                             // Result = 00000000
    if (i32(t0) <= 0) goto loc_80029194;
    a2 = t1;
    a1 = t6;                                            // Result = gCheatSequenceBtns[0] (800A91A4)
loc_8002916C:
    v1 = lh(a1);
    v0 = lh(a2);
    {
        const bool bJump = (v1 != v0);
        v0 = (i32(a0) < 8);
        if (bJump) goto loc_80029198;
    }
    a2 += 2;
    a0++;
    v0 = (i32(a0) < i32(t0));
    a1 += 2;
    if (v0 != 0) goto loc_8002916C;
loc_80029194:
    v0 = (i32(a0) < 8);
loc_80029198:
    {
        const bool bJump = (v0 != 0);
        v0 = (s0 < 0xA);
        if (bJump) goto loc_800293B4;
    }
    if (v0 == 0) goto loc_800293C4;
    v0 = s0 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += 0x10F8;                                       // Result = JumpTable_P_CheckCheats_1[0] (800110F8)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x800291C8: goto loc_800291C8;
        case 0x80029204: goto loc_80029204;
        case 0x80029240: goto loc_80029240;
        case 0x80029290: goto loc_80029290;
        case 0x800293C4: goto loc_800293C4;
        case 0x80029370: goto loc_80029370;
        case 0x800293A0: goto loc_800293A0;
        default: jump_table_err(); break;
    }
loc_800291C8:
    v0 = lw(a3 + 0xC0);
    v0 ^= 4;
    sw(v0, a3 + 0xC0);
    v0 &= 4;
    if (v0 == 0) goto loc_800291F4;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x1060;                                       // Result = STR_Cheat_AllMapLines_On[0] (80011060)
    sw(v0, t3);
    goto loc_80029288;
loc_800291F4:
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x1074;                                       // Result = STR_Cheat_AllMapLines_Off[0] (80011074)
    sw(v0, t3);
    goto loc_80029288;
loc_80029204:
    v0 = lw(a3 + 0xC0);
    v0 ^= 8;
    sw(v0, a3 + 0xC0);
    v0 &= 8;
    if (v0 == 0) goto loc_80029230;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x1088;                                       // Result = STR_Cheat_AllMapThings_On[0] (80011088)
    sw(v0, t3);
    goto loc_80029288;
loc_80029230:
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x109C;                                       // Result = STR_Cheat_AllMapThings_Off[0] (8001109C)
    sw(v0, t3);
    goto loc_80029288;
loc_80029240:
    v0 = lw(a3 + 0xC0);
    v0 ^= 2;
    sw(v0, a3 + 0xC0);
    v0 &= 2;
    if (v0 == 0) goto loc_8002927C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x10B0;                                       // Result = STR_Cheat_GodMode_On[0] (800110B0)
    sw(v0, t3);
    v1 = lw(a3);
    v0 = 0x64;                                          // Result = 00000064
    sw(v0, a3 + 0x24);
    sw(v0, v1 + 0x68);
    goto loc_80029288;
loc_8002927C:
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x10C8;                                       // Result = STR_Cheat_GodMode_Off[0] (800110C8)
    sw(v0, t3);
loc_80029288:
    sw(t2, t4);
    goto loc_800293C4;
loc_80029290:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    a0 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 -= 0x14;                                         // Result = gMObjHead[0] (800A8E90)
    if (a0 == v0) goto loc_80029314;
    a1 = v0;                                            // Result = gMObjHead[0] (800A8E90)
loc_800292AC:
    v0 = lw(a0 + 0x54);
    v1 = v0 - 0x25;
    v0 = (v1 < 6);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80029304;
    }
    v0 += t5;
    v0 = lw(v0);
    switch (v0) {
        case 0x800292D8: goto loc_800292D8;
        case 0x800292E8: goto loc_800292E8;
        case 0x800292E0: goto loc_800292E0;
        case 0x800292F8: goto loc_800292F8;
        case 0x80029300: goto loc_80029300;
        case 0x800292F0: goto loc_800292F0;
        default: jump_table_err(); break;
    }
loc_800292D8:
    sw(t2, a3 + 0x4C);
    goto loc_80029304;
loc_800292E0:
    sw(t2, a3 + 0x50);
    goto loc_80029304;
loc_800292E8:
    sw(t2, a3 + 0x48);
    goto loc_80029304;
loc_800292F0:
    sw(t2, a3 + 0x58);
    goto loc_80029304;
loc_800292F8:
    sw(t2, a3 + 0x5C);
    goto loc_80029304;
loc_80029300:
    sw(t2, a3 + 0x54);
loc_80029304:
    a0 = lw(a0 + 0x14);
    if (a0 != a1) goto loc_800292AC;
loc_80029314:
    a0 = 8;                                             // Result = 00000008
    v1 = a3 + 0x20;
    v0 = 0xC8;                                          // Result = 000000C8
    sw(v0, a3 + 0x28);
    v0 = 2;                                             // Result = 00000002
    sw(v0, a3 + 0x2C);
loc_8002932C:
    sw(t2, v1 + 0x74);
    a0--;
    v1 -= 4;
    if (i32(a0) >= 0) goto loc_8002932C;
    a0 = 0;                                             // Result = 00000000
    v1 = a3;
loc_80029344:
    v0 = lw(v1 + 0xA8);
    a0++;
    sw(v0, v1 + 0x98);
    v0 = (i32(a0) < 4);
    v1 += 4;
    if (v0 != 0) goto loc_80029344;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x10E0;                                       // Result = STR_Cheat_LotsOfGoodies[0] (800110E0)
    sw(v0, t3);
    sw(t2, t3 + 0x4);
    goto loc_800293C4;
loc_80029370:
    v0 = lw(a3 + 0xC0);
    v1 = *gGameMap;
    v0 |= 0x20;
    sw(v1, gp + 0xC90);                                 // Store to: gMapNumToCheatWarpTo (80078270)
    v1 = (i32(v1) < 0x37);
    sw(v0, a3 + 0xC0);
    if (v1 != 0) goto loc_800293C4;
    v0 = 0x36;                                          // Result = 00000036
    sw(v0, gp + 0xC90);                                 // Store to: gMapNumToCheatWarpTo (80078270)
    goto loc_800293C4;
loc_800293A0:
    v0 = lw(a3 + 0xC0);
    v0 ^= 0x80;
    sw(v0, a3 + 0xC0);
    goto loc_800293C4;
loc_800293B4:
    s0++;
    v0 = (s0 < 0xC);
    t1 += 0x10;
    if (v0 != 0) goto loc_80029154;
loc_800293C4:
    v1 = lw(gp + 0xA04);                                // Load from: gCurCheatBtnSequenceIdx (80077FE4)
    v0 = v1;
    if (i32(v1) >= 0) goto loc_800293D8;
    v0 = v1 + 7;
loc_800293D8:
    v0 = u32(i32(v0) >> 3);
    v0 <<= 3;
    v0 = v1 - v0;
    sw(v0, gp + 0xA04);                                 // Store to: gCurCheatBtnSequenceIdx (80077FE4)
loc_800293E8:
    ra = lw(sp + 0x3C);
    s6 = lw(sp + 0x38);
    s5 = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x40;
    return;
}

void P_Ticker() noexcept {
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    *gGameAction = ga_nothing;
    P_CheckCheats();
    v0 = lw(gp + 0x8E0);                                // Load from: gbGamePaused (80077EC0)
    if (v0 != 0) goto loc_8002955C;
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002955C;
    s0 = 0x80090000;                                    // Result = 80090000
    s0 = lw(s0 + 0x6554);                               // Load from: gThinkerCap[1] (80096554)
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    sw(0, gp + 0xD14);                                  // Store to: gNumActiveThinkers (800782F4)
    if (s0 == v0) goto loc_800294F8;
    s2 = -1;                                            // Result = FFFFFFFF
    s1 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_8002948C:
    v0 = lw(s0 + 0x8);
    a1 = s0;
    if (v0 != s2) goto loc_800294C8;
    v1 = lw(s0 + 0x4);
    v0 = lw(s0);
    a0 = *gpMainMemZone;
    sw(v0, v1);
    v1 = lw(s0);
    v0 = lw(s0 + 0x4);
    sw(v0, v1 + 0x4);
    _thunk_Z_Free2();
    goto loc_800294E8;
loc_800294C8:
    if (v0 == 0) goto loc_800294D8;
    a0 = s0;
    ptr_call(v0);
loc_800294D8:
    v0 = lw(gp + 0xD14);                                // Load from: gNumActiveThinkers (800782F4)
    v0++;
    sw(v0, gp + 0xD14);                                 // Store to: gNumActiveThinkers (800782F4)
loc_800294E8:
    s0 = lw(s0 + 0x4);
    if (s0 != s1) goto loc_8002948C;
loc_800294F8:
    P_CheckSights();
    P_RunMobjBase();
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x715C);                               // Load from: gMObjHead[5] (800A8EA4)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    s1 = v0;                                            // Result = gMObjHead[0] (800A8E90)
    if (a0 == v0) goto loc_80029544;
loc_80029520:
    v0 = lw(a0 + 0x18);
    s0 = lw(a0 + 0x14);
    if (v0 == 0) goto loc_80029538;
    ptr_call(v0);
loc_80029538:
    a0 = s0;
    if (a0 != s1) goto loc_80029520;
loc_80029544:
    P_UpdateSpecials();
    P_RespawnSpecials();
    ST_Ticker();
loc_8002955C:
    sw(0, gp + 0xD0C);                                  // Store to: gPlayerNum (800782EC)
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    s2 = 0x80080000;                                    // Result = 80080000
    s2 -= 0x7F54;                                       // Result = gbPlayerInGame[0] (800780AC)
    s1 = 2;                                             // Result = 00000002
loc_80029574:
    a0 = lw(gp + 0xD0C);                                // Load from: gPlayerNum (800782EC)
    v0 = a0 << 2;
    v0 += s2;
    v0 = lw(v0);
    if (v0 == 0) goto loc_800295BC;
    v0 = lw(s0 + 0x4);
    if (v0 != s1) goto loc_800295AC;
    G_DoReborn();
loc_800295AC:
    a0 = s0;
    AM_Control();
    a0 = s0;
    P_PlayerThink();
loc_800295BC:
    v0 = lw(gp + 0xD0C);                                // Load from: gPlayerNum (800782EC)
    v0++;
    sw(v0, gp + 0xD0C);                                 // Store to: gPlayerNum (800782EC)
    v0 = (i32(v0) < 2);
    s0 += 0x12C;
    if (v0 != 0) goto loc_80029574;
    v0 = *gGameAction;
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_Drawer() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    I_IncDrawnFrameCount();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x76F0;                                       // Result = gPlayer1[49] (800A8910)
    at += v0;
    v0 = lw(at);
    v0 &= 1;
    if (v0 == 0) goto loc_8002965C;
    AM_Drawer();
    goto loc_80029664;
loc_8002965C:
    R_RenderPlayerView();
loc_80029664:
    ST_Drawer();
    I_SubmitGpuCmds();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_Start() noexcept {
    sp -= 0x28;
    sw(s0, sp + 0x20);
    s0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x24);
    sw(0, gp + 0x8E0);                                  // Store to: gbGamePaused (80077EC0)
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    AM_Start();
    M_ClearRandom();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = *gbDemoPlayback;
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x7BE4);                                // Store to: gbIsLevelDataCached (80077BE4)
    a2 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80029700;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x3E54;                                       // Result = CDTrackNum_Credits_Demo (80073E54)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75F8);                               // Load from: gCdMusicVol (800775F8)
    v0 = lw(v1);                                        // Load from: CDTrackNum_Credits_Demo (80073E54)
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    sw(v0, sp + 0x10);
    sw(a1, sp + 0x14);
    a0 = lw(v1);                                        // Load from: CDTrackNum_Credits_Demo (80073E54)
    a3 = 0;                                             // Result = 00000000
    psxcd_play_at_andloop();
    goto loc_80029708;
loc_80029700:
    S_StartMusicSequence();
loc_80029708:
    ra = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x28;
    return;
}

void P_Stop() noexcept {
    sp -= 0x20;
    a0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    LIBGPU_DrawSync();
    s0 = 0;                                             // Result = 00000000
    S_Clear();
    psxcd_stop();
    S_StopMusicSequence();
    s1 = 0x80080000;                                    // Result = 80080000
    s1 -= 0x7F54;                                       // Result = gbPlayerInGame[0] (800780AC)
    sw(0, gp + 0x8E0);                                  // Store to: gbGamePaused (80077EC0)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7BE4);                                 // Store to: gbIsLevelDataCached (80077BE4)
loc_80029760:
    v0 = lw(s1);
    s1 += 4;
    if (v0 == 0) goto loc_80029778;
    a0 = s0;
    G_PlayerFinishLevel();
loc_80029778:
    s0++;
    v0 = (i32(s0) < 2);
    if (v0 != 0) goto loc_80029760;
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
