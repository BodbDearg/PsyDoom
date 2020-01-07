#include "pw_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_password.h"
#include "Doom/Game/p_tick.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGPU.h"

void START_PasswordScreen() noexcept {
    sp -= 0x18;
    a0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x10);
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F44);                               // Load from: gTicButtons[0] (80077F44)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F48);                               // Load from: gTicButtons[1] (80077F48)
    sw(0, gp + 0xA4C);                                  // Store to: gInvalidPasswordFlashTicsLeft (8007802C)
    sw(0, gp + 0xB94);                                  // Store to: gCurPasswordCharIdx (80078174)
    *gVBlanksUntilMenuMove = 0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7DEC);                                // Store to: gOldTicButtons[0] (80078214)
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7DE8);                                // Store to: gOldTicButtons[1] (80078218)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void STOP_PasswordScreen() noexcept {
    sp -= 0x18;
    a0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x10);
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = 0x20;                                          // Result = 00000020
    sw(v0, gp + 0xB94);                                 // Store to: gCurPasswordCharIdx (80078174)
    DRAW_PasswordScreen();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void TIC_PasswordScreen() noexcept {
    a0 = lw(gp + 0xA4C);                                // Load from: gInvalidPasswordFlashTicsLeft (8007802C)
    sp -= 0x28;
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    if (a0 == 0) goto loc_80036EF4;
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = a0 - 1;
        if (bJump) goto loc_80036EF4;
    }
    sw(v0, gp + 0xA4C);                                 // Store to: gInvalidPasswordFlashTicsLeft (8007802C)
    v0 &= 7;
    v1 = 4;                                             // Result = 00000004
    a0 = 0;                                             // Result = 00000000
    if (v0 != v1) goto loc_80036EF4;
    a1 = 0x18;                                          // Result = 00000018
    S_StartSound();
loc_80036EF4:
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x7F44);                               // Load from: gTicButtons[0] (80077F44)
    s1 = 0x80080000;                                    // Result = 80080000
    s1 = lw(s1 - 0x7DEC);                               // Load from: gOldTicButtons[0] (80078214)
    v0 = s0 & 0xF000;
    {
        const bool bJump = (v0 != 0);
        v0 = s0 & 0x900;
        if (bJump) goto loc_80036F1C;
    }
    *gVBlanksUntilMenuMove = 0;
    goto loc_8003700C;
loc_80036F1C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7EF8;                                       // Result = gVBlanksUntilMenuMove (80077EF8)
    v0 = *gVBlanksUntilMenuMove;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FBC);                               // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v0 -= v1;
    *gVBlanksUntilMenuMove = v0;
    if (i32(v0) > 0) goto loc_80037008;
    *gVBlanksUntilMenuMove = MENU_MOVE_VBLANK_DELAY;
    v0 = s0 & 0x1000;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x4000;
        if (bJump) goto loc_80036F70;
    }
    v1 = lw(gp + 0xB94);                                // Load from: gCurPasswordCharIdx (80078174)
    v0 = (i32(v1) < 8);
    {
        const bool bJump = (v0 != 0);
        v0 = s0 & 0x8000;
        if (bJump) goto loc_80036FA0;
    }
    v0 = v1 - 8;
    goto loc_80036F8C;
loc_80036F70:
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x8000;
        if (bJump) goto loc_80036FA0;
    }
    v1 = lw(gp + 0xB94);                                // Load from: gCurPasswordCharIdx (80078174)
    v0 = (i32(v1) < 0x18);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 + 8;
        if (bJump) goto loc_80036F9C;
    }
loc_80036F8C:
    sw(v0, gp + 0xB94);                                 // Store to: gCurPasswordCharIdx (80078174)
    a0 = 0;                                             // Result = 00000000
    a1 = 0x12;                                          // Result = 00000012
    S_StartSound();
loc_80036F9C:
    v0 = s0 & 0x8000;
loc_80036FA0:
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x2000;
        if (bJump) goto loc_80036FCC;
    }
    v0 = lw(gp + 0xB94);                                // Load from: gCurPasswordCharIdx (80078174)
    v0--;
    sw(v0, gp + 0xB94);                                 // Store to: gCurPasswordCharIdx (80078174)
    a0 = 0;                                             // Result = 00000000
    if (i32(v0) >= 0) goto loc_80037000;
    sw(0, gp + 0xB94);                                  // Store to: gCurPasswordCharIdx (80078174)
    v0 = s0 & 0x900;
    goto loc_8003700C;
loc_80036FCC:
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x900;
        if (bJump) goto loc_8003700C;
    }
    v0 = lw(gp + 0xB94);                                // Load from: gCurPasswordCharIdx (80078174)
    v0++;
    sw(v0, gp + 0xB94);                                 // Store to: gCurPasswordCharIdx (80078174)
    v0 = (i32(v0) < 0x20);
    a0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80037000;
    v0 = 0x1F;                                          // Result = 0000001F
    sw(v0, gp + 0xB94);                                 // Store to: gCurPasswordCharIdx (80078174)
    v0 = s0 & 0x900;
    goto loc_8003700C;
loc_80037000:
    a1 = 0x12;                                          // Result = 00000012
    S_StartSound();
loc_80037008:
    v0 = s0 & 0x900;
loc_8003700C:
    {
        const bool bJump = (v0 != 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_8003711C;
    }
    v0 = s0 & 0xE0;
    if (s0 == s1) goto loc_80037118;
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_800370D0;
    a1 = 0x17;                                          // Result = 00000017
    S_StartSound();
    a0 = lw(gp + 0x660);                                // Load from: gNumPasswordCharsEntered (80077C40)
    v0 = (i32(a0) < 0xA);
    a2 = sp + 0x14;
    if (v0 == 0) goto loc_80037070;
    v1 = lbu(gp + 0xB94);                               // Load from: gCurPasswordCharIdx (80078174)
    v0 = a0 + 1;
    sw(v0, gp + 0x660);                                 // Store to: gNumPasswordCharsEntered (80077C40)
    at = 0x80090000;                                    // Result = 80090000
    at += 0x6560;                                       // Result = gPasswordChars[0] (80096560)
    at += a0;
    sb(v1, at);
    v0 = lw(gp + 0x660);                                // Load from: gNumPasswordCharsEntered (80077C40)
    v0 = (i32(v0) < 0xA);
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003711C;
    }
loc_80037070:
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x6560;                                       // Result = gPasswordChars[0] (80096560)
    a1 = sp + 0x10;
    a3 = 0;                                             // Result = 00000000
    P_ProcessPassword();
    a0 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_800370C0;
    v1 = lw(sp + 0x10);
    a1 = lw(sp + 0x14);
    sw(a0, gp + 0x65C);                                 // Store to: gbUsingAPassword (80077C3C)
    *gGameMap = v1;
    *gStartMapOrEpisode = v1;
    *gGameSkill = (skill_t) a1;
    *gStartSkill = (skill_t) a1;
    v0 = 4;
    goto loc_8003711C;
loc_800370C0:
    v0 = 0x10;                                          // Result = 00000010
    sw(v0, gp + 0xA4C);                                 // Store to: gInvalidPasswordFlashTicsLeft (8007802C)
    v0 = 0;                                             // Result = 00000000
    goto loc_8003711C;
loc_800370D0:
    v0 = s0 & 0x10;
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003711C;
    }
    a1 = 0x17;                                          // Result = 00000017
    S_StartSound();
    v0 = lw(gp + 0x660);                                // Load from: gNumPasswordCharsEntered (80077C40)
    v0--;
    sw(v0, gp + 0x660);                                 // Store to: gNumPasswordCharsEntered (80077C40)
    if (i32(v0) >= 0) goto loc_80037100;
    sw(0, gp + 0x660);                                  // Store to: gNumPasswordCharsEntered (80077C40)
loc_80037100:
    v0 = lw(gp + 0x660);                                // Load from: gNumPasswordCharsEntered (80077C40)
    at = 0x80090000;                                    // Result = 80090000
    at += 0x6560;                                       // Result = gPasswordChars[0] (80096560)
    at += v0;
    sb(0, at);
loc_80037118:
    v0 = 0;                                             // Result = 00000000
loc_8003711C:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void DRAW_PasswordScreen() noexcept {
loc_80037134:
    sp -= 0x40;
    sw(ra, sp + 0x38);
    sw(s3, sp + 0x34);
    sw(s2, sp + 0x30);
    sw(s1, sp + 0x2C);
    sw(s0, sp + 0x28);
    I_IncDrawnFrameCount();
    s1 = 0;                                             // Result = 00000000
    s0 = 0;                                             // Result = 00000000
loc_80037158:
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7AB0;                                       // Result = gTexInfo_MARB01[0] (80097AB0)
    a1 = s0 << 6;
    a2 = s1 << 6;
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F7C);                               // Load from: gPaletteClutId_Main (800A9084)
    s0++;
    I_CacheAndDrawSprite();
    v0 = (i32(s0) < 4);
    if (v0 != 0) goto loc_80037158;
    s1++;                                               // Result = 00000001
    v0 = (i32(s1) < 4);                                 // Result = 00000001
    s0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80037158;
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    a0 = s0;                                            // Result = 1F800200
    a1 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lhu(a3 - 0x6B0E);                              // Load from: gTexInfo_STATUS[2] (800A94F2)
    a2 = 0;                                             // Result = 00000000
    sw(0, sp + 0x10);
    LIBGPU_SetDrawMode();
    s0 += 4;                                            // Result = 1F800204
    t3 = 0xFF0000;                                      // Result = 00FF0000
    t3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t7 = 0x80080000;                                    // Result = 80080000
    t7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = t7 & t3;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_800371F4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8003725C;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_80037320;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(t7, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003725C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80037310;
    if (v1 == a0) goto loc_800371F4;
loc_80037280:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_800371F4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t3;
    v0 |= t5;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_800372EC;
    a3 = -1;                                            // Result = FFFFFFFF
loc_800372D0:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_800372D0;
loc_800372EC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800371F4;
    goto loc_80037280;
loc_80037310:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_80037320:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= a1;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a2 += 4;
    if (t0 == v0) goto loc_80037380;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80037368:
    v0 = lw(s0);
    s0 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80037368;
loc_80037380:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    s2 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_80037438;
    t2 = 0x4000000;                                     // Result = 04000000
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_800373B0:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t2;
    s2 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80037438;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= a3;
    v0 |= t1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t0) goto loc_8003741C;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80037400:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80037400;
loc_8003741C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    s2 = 0;                                             // Result = 00000000
    if (v1 != v0) goto loc_800373B0;
loc_80037438:
    s3 = 0x1F800000;                                    // Result = 1F800000
    s3 += 0x200;                                        // Result = 1F800200
    t4 = 0xFF0000;                                      // Result = 00FF0000
    t4 |= 0xFFFF;                                       // Result = 00FFFFFF
    t9 = 0x80080000;                                    // Result = 80080000
    t9 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    t8 = t9 & t4;                                       // Result = 00086550
    t7 = 0x4000000;                                     // Result = 04000000
    t6 = 0x80000000;                                    // Result = 80000000
    t5 = -1;                                            // Result = FFFFFFFF
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lhu(v1 - 0x6F5C);                              // Load from: gPaletteClutId_UI (800A90A4)
    v0 = 4;                                             // Result = 00000004
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v0 = 0x64;                                          // Result = 00000064
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    v0 = 0x80;                                          // Result = 00000080
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x204);                                 // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x205);                                 // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x206);                                 // Store to: 1F800206
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x20E);                                 // Store to: 1F80020E
loc_800374A4:
    a0 = s2;
    if (i32(s2) >= 0) goto loc_800374B0;
    a0 = s2 + 7;
loc_800374B0:
    a0 = u32(i32(a0) >> 3);
    v1 = a0 << 3;
    v1 = s2 - v1;
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    s0 = v0 + 0x30;
    v0 = a0 << 2;
    v0 += a0;
    a0 = v0 << 2;
    v0 = lw(gp + 0xB94);                                // Load from: gCurPasswordCharIdx (80078174)
    s1 = a0 + 0x3C;
    if (v0 != s2) goto loc_80037518;
    v0 = *gTicCon;
    v0 &= 4;
    {
        const bool bJump = (v0 != 0);
        v0 = 0xFF;                                      // Result = 000000FF
        if (bJump) goto loc_80037828;
    }
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x204);                                 // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x205);                                  // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x206);                                  // Store to: 1F800206
loc_80037518:
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3D4C;                                       // Result = STR_PasswordChars[0] (80073D4C)
    at += s2;
    v1 = lbu(at);
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 - 0x39;
        if (bJump) goto loc_80037548;
    }
    v1 = v0 << 2;
    s1 = a0 + 0x3F;
    goto loc_80037578;
loc_80037548:
    a0 = v1 - 0x30;
    v0 = (a0 < 0xA);
    {
        const bool bJump = (v0 == 0);
        v0 = 0x21;                                      // Result = 00000021
        if (bJump) goto loc_8003756C;
    }
    v1 = a0 << 2;
    goto loc_80037578;
loc_80037560:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_8003772C;
loc_8003756C:
    if (v1 != v0) goto loc_80037578;
    v1 = 0x30;                                          // Result = 00000030
loc_80037578:
    at = 0x1F800000;                                    // Result = 1F800000
    sh(s0, at + 0x208);                                 // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(s1, at + 0x20A);                                 // Store to: 1F80020A
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F84;                                       // Result = BigFontTexcoords_0[0] (80073F84)
    at += v1;
    v0 = lbu(at);
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F85;                                       // Result = BigFontTexcoords_0[1] (80073F85)
    at += v1;
    v0 = lbu(at);
    t2 = s3 + 4;                                        // Result = 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20D);                                 // Store to: 1F80020D
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F86;                                       // Result = BigFontTexcoords_0[2] (80073F86)
    at += v1;
    v0 = lbu(at);
    t1 = t0 << 2;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F87;                                       // Result = BigFontTexcoords_0[3] (80073F87)
    at += v1;
    v0 = lbu(at);
    t3 = t1 + 4;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
loc_80037604:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80037668;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_80037560;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(t9, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= t8;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80037668:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003771C;
    if (v1 == a0) goto loc_80037604;
loc_8003768C:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t7;
    if (v0 == 0) goto loc_80037604;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t6;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800376F8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800376DC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800376DC;
loc_800376F8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80037604;
    goto loc_8003768C;
loc_8003771C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t3;
loc_8003772C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a3);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t4;
    v1 |= v0;
    sw(v1, a3);
    sb(t0, a3 + 0x3);
    t0--;
    a3 += 4;
    if (t0 == t5) goto loc_800377F4;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80037768:
    v0 = lw(t2);
    t2 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_80037768;
    goto loc_800377F4;
loc_80037788:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t7;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x80;                                      // Result = 00000080
        if (bJump) goto loc_80037810;
    }
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t6;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800377F4;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800377D8:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800377D8;
loc_800377F4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    {
        const bool bJump = (v1 != v0);
        v0 = 0x80;                                      // Result = 00000080
        if (bJump) goto loc_80037788;
    }
loc_80037810:
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x204);                                 // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x205);                                 // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x206);                                 // Store to: 1F800206
loc_80037828:
    s2++;
    v0 = (i32(s2) < 0x20);
    if (v0 != 0) goto loc_800374A4;
    a0 = -1;                                            // Result = FFFFFFFF
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x15A8;                                       // Result = STR_Password[0] (800115A8)
    a1 = 0x14;                                          // Result = 00000014
    I_DrawString();
    v0 = lw(gp + 0x660);                                // Load from: gNumPasswordCharsEntered (80077C40)
    s2 = 0;                                             // Result = 00000000
    sb(0, sp + 0x19);
    if (i32(v0) <= 0) goto loc_800378AC;
    s0 = 0x3A;                                          // Result = 0000003A
    a0 = s0;                                            // Result = 0000003A
loc_80037864:
    a1 = 0xA0;                                          // Result = 000000A0
    at = 0x80090000;                                    // Result = 80090000
    at += 0x6560;                                       // Result = gPasswordChars[0] (80096560)
    at += s2;
    v0 = lbu(at);
    a2 = sp + 0x18;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3D4C;                                       // Result = STR_PasswordChars[0] (80073D4C)
    at += v0;
    v0 = lbu(at);
    s0 += 0xE;
    sb(v0, sp + 0x18);
    I_DrawString();
    v0 = lw(gp + 0x660);                                // Load from: gNumPasswordCharsEntered (80077C40)
    s2++;
    v0 = (i32(s2) < i32(v0));
    a0 = s0;
    if (v0 != 0) goto loc_80037864;
loc_800378AC:
    v0 = (i32(s2) < 0xA);
    {
        const bool bJump = (v0 == 0);
        v0 = s2 << 3;
        if (bJump) goto loc_800378E8;
    }
    v0 -= s2;
    v0 <<= 1;
    s0 = v0 + 0x3A;
loc_800378C4:
    a0 = s0;
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7CD0;                                       // Result = STR_EmptyPasswordChar[0] (80077CD0)
    a1 = 0xA0;                                          // Result = 000000A0
    I_DrawString();
    s2++;
    v0 = (i32(s2) < 0xA);
    s0 += 0xE;
    if (v0 != 0) goto loc_800378C4;
loc_800378E8:
    v0 = lw(gp + 0xA4C);                                // Load from: gInvalidPasswordFlashTicsLeft (8007802C)
    v0 &= 4;
    a0 = -1;                                            // Result = FFFFFFFF
    if (v0 == 0) goto loc_8003790C;
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x15B4;                                       // Result = STR_InvalidPassword[0] (800115B4)
    a1 = 0xC8;                                          // Result = 000000C8
    I_DrawString();
loc_8003790C:
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x40;
    return;
}
