#include "ct_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/d_main.h"
#include "PsxVm/PsxVm.h"

void START_ControlsScreen() noexcept {
    sp -= 0x18;
    a0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x10);
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7AD0;                                       // Result = gTexInfo_BUTTONS[0] (80097AD0)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C5C;                                       // Result = STR_LumpName_BUTTONS[0] (80077C5C)
    sw(0, gp + 0xBF8);                                  // Store to: gCursorFrame (800781D8)
    sw(0, gp + 0xA20);                                  // Store to: gCursorPos (80078000)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void STOP_ControlsScreen() noexcept {
    sp -= 0x18;
    a0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x10);
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = 3;                                             // Result = 00000003
    sw(v0, gp + 0xA20);                                 // Store to: gCursorPos (80078000)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void TIC_ControlsScreen() noexcept {
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = *gpGameTic;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7FA4);                               // Load from: gPrevGameTic (80077FA4)
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = (i32(v0) < i32(v1));
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_800379F0;
    v0 = v1 & 3;
    if (v0 != 0) goto loc_800379F0;
    v0 = lw(gp + 0xBF8);                                // Load from: gCursorFrame (800781D8)
    v0 ^= 1;
    sw(v0, gp + 0xBF8);                                 // Store to: gCursorFrame (800781D8)
loc_800379F0:
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x7F44);                               // Load from: gPlayerPadButtons[0] (80077F44)
    s1 = 0x80080000;                                    // Result = 80080000
    s1 = lw(s1 - 0x7DEC);                               // Load from: gPlayerOldPadButtons[0] (80078214)
    v0 = s0 & 0xF000;
    {
        const bool bJump = (v0 != 0);
        v0 = s0 & 0x900;
        if (bJump) goto loc_80037A18;
    }
    sw(0, gp + 0x918);                                  // Store to: gVBlanksUntilMenuMove (80077EF8)
    goto loc_80037AB8;
loc_80037A18:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7EF8;                                       // Result = gVBlanksUntilMenuMove (80077EF8)
    v0 = lw(a0);                                        // Load from: gVBlanksUntilMenuMove (80077EF8)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FBC);                               // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v0 -= v1;
    sw(v0, a0);                                         // Store to: gVBlanksUntilMenuMove (80077EF8)
    if (i32(v0) > 0) goto loc_80037AB4;
    v0 = 0xF;                                           // Result = 0000000F
    sw(v0, a0);                                         // Store to: gVBlanksUntilMenuMove (80077EF8)
    v0 = s0 & 0x4000;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x1000;
        if (bJump) goto loc_80037A7C;
    }
    v1 = 0x80080000;                                    // Result = 80080000
    v1 -= 0x8000;                                       // Result = gCursorPos (80078000)
    v0 = lw(v1);                                        // Load from: gCursorPos (80078000)
    v0++;
    sw(v0, v1);                                         // Store to: gCursorPos (80078000)
    v0 = (i32(v0) < 9);
    a0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80037AAC;
    sw(0, v1);                                          // Store to: gCursorPos (80078000)
    goto loc_80037AAC;
loc_80037A7C:
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x900;
        if (bJump) goto loc_80037AB8;
    }
    v1 = 0x80080000;                                    // Result = 80080000
    v1 -= 0x8000;                                       // Result = gCursorPos (80078000)
    v0 = lw(v1);                                        // Load from: gCursorPos (80078000)
    v0--;
    sw(v0, v1);                                         // Store to: gCursorPos (80078000)
    if (i32(v0) >= 0) goto loc_80037AA8;
    v0 = 8;                                             // Result = 00000008
    sw(v0, v1);                                         // Store to: gCursorPos (80078000)
loc_80037AA8:
    a0 = 0;                                             // Result = 00000000
loc_80037AAC:
    a1 = 0x12;                                          // Result = 00000012
    S_StartSound();
loc_80037AB4:
    v0 = s0 & 0x900;
loc_80037AB8:
    {
        const bool bJump = (v0 != 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_80037B6C;
    }
    v0 = 0;                                             // Result = 00000000
    if (s0 == s1) goto loc_80037B6C;
    v0 = lw(gp + 0xA20);                                // Load from: gCursorPos (80078000)
    v0 = (i32(v0) < 8);
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80037B34;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x3DEC;                                       // Result = gBtnSprite_Triangle_ButtonMask (80073DEC)
loc_80037AE4:
    a2 = lw(v1);
    v0 = s0 & a2;
    a0++;
    if (v0 != 0) goto loc_80037B0C;
    v0 = (i32(a0) < 8);
    v1 += 4;
    if (v0 != 0) goto loc_80037AE4;
    v0 = 0;                                             // Result = 00000000
    goto loc_80037B6C;
loc_80037B0C:
    a0 = 0;                                             // Result = 00000000
    v0 = lw(gp + 0xA20);                                // Load from: gCursorPos (80078000)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3E0C;                                       // Result = gBtnBinding_Attack (80073E0C)
    at += v0;
    sw(a2, at);
    a1 = 0x17;                                          // Result = 00000017
    goto loc_80037B60;
loc_80037B34:
    v0 = s0 & 0xF0;
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80037B6C;
    }
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x3E0C;                                       // Result = gBtnBinding_Attack (80073E0C)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x3E2C;                                       // Result = DefaultBtnBinding_Attack (80073E2C)
    a2 = 0x20;                                          // Result = 00000020
    D_memcpy();
    a0 = 0;                                             // Result = 00000000
    a1 = 0x17;                                          // Result = 00000017
loc_80037B60:
    S_StartSound();
    v0 = 0;                                             // Result = 00000000
loc_80037B6C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void DRAW_ControlsScreen() noexcept {
    sp -= 0x38;
    sw(ra, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    I_IncDrawnFrameCount();
    s1 = 0;                                             // Result = 00000000
    s0 = 0;                                             // Result = 00000000
loc_80037BAC:
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7AB0;                                       // Result = gTexInfo_MARB01[0] (80097AB0)
    a1 = s0 << 6;
    a2 = s1 << 6;
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F7C);                               // Load from: gPaletteClutId_Main (800A9084)
    s0++;
    I_CacheAndDrawSprite();
    v0 = (i32(s0) < 4);
    if (v0 != 0) goto loc_80037BAC;
    s1++;                                               // Result = 00000001
    v0 = (i32(s1) < 4);                                 // Result = 00000001
    s0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80037BAC;
    a0 = -1;                                            // Result = FFFFFFFF
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x15C8;                                       // Result = STR_Configuration[0] (800115C8)
    a1 = 0x14;                                          // Result = 00000014
    I_DrawString();
    a2 = 0xC;                                           // Result = 0000000C
    s2 = 0;                                             // Result = 00000000
    s4 = 0x80070000;                                    // Result = 80070000
    s4 += 0x3DEC;                                       // Result = gBtnSprite_Triangle_ButtonMask (80073DEC)
    s3 = 0x80090000;                                    // Result = 80090000
    s3 += 0x7ADA;                                       // Result = gTexInfo_BUTTONS[2] (80097ADA)
    s1 = 0x2D;                                          // Result = 0000002D
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x3E0C;                                       // Result = gBtnBinding_Attack (80073E0C)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lhu(a0 - 0x6B0E);                              // Load from: gTexInfo_STATUS[2] (800A94F2)
    v0 = 0xC0;                                          // Result = 000000C0
    sw(v0, sp + 0x14);
    v0 = 0x10;                                          // Result = 00000010
    sw(v0, sp + 0x18);
    v0 = 0x12;                                          // Result = 00000012
    sw(v0, sp + 0x1C);
    v0 = lw(gp + 0xA20);                                // Load from: gCursorPos (80078000)
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lh(a1 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    a3 = v0 << 2;
    a3 += v0;
    a3 <<= 2;
    v0 = lw(gp + 0xBF8);                                // Load from: gCursorFrame (800781D8)
    a3 += 0x2B;
    v0 <<= 4;
    v0 += 0x84;
    sw(v0, sp + 0x10);
    I_DrawSprite();
loc_80037C70:
    v1 = 0;                                             // Result = 00000000
    a1 = lw(s0);
    a0 = s4;                                            // Result = gBtnSprite_Triangle_ButtonMask (80073DEC)
loc_80037C7C:
    v0 = lw(a0);
    if (a1 == v0) goto loc_80037C9C;
    v1++;                                               // Result = 00000001
    v0 = (i32(v1) < 8);                                 // Result = 00000001
    a0 += 4;                                            // Result = gBtnSprite_Circle_ButtonMask (80073DF0)
    if (v0 != 0) goto loc_80037C7C;
loc_80037C9C:
    v0 = lw(gp + 0xA20);                                // Load from: gCursorPos (80078000)
    a2 = 0x26;                                          // Result = 00000026
    if (v0 != s2) goto loc_80037CC4;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    v0 &= 8;
    if (v0 != 0) goto loc_80037CFC;
loc_80037CC4:
    a3 = s1;
    v0 = 0x10;                                          // Result = 00000010
    v1 <<= 4;                                           // Result = 00000000
    sw(v0, sp + 0x18);
    sw(v0, sp + 0x1C);
    a0 = lhu(s3);                                       // Load from: gTexInfo_BUTTONS[2] (80097ADA)
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lh(a1 - 0x6F7C);                               // Load from: gPaletteClutId_Main (800A9084)
    v0 = lbu(s3 - 0x2);                                 // Load from: gTexInfo_BUTTONS[2] (80097AD8)
    t0 = lbu(s3 - 0x1);                                 // Load from: gTexInfo_BUTTONS[2] (80097AD9)
    v0 += v1;
    sw(v0, sp + 0x10);
    sw(t0, sp + 0x14);
    I_DrawSprite();
loc_80037CFC:
    s1 += 0x14;
    s2++;
    v0 = (i32(s2) < 8);
    s0 += 4;
    if (v0 != 0) goto loc_80037C70;
    s2 = 0;                                             // Result = 00000000
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x3D6C;                                       // Result = STR_MenuOpt_Attack[0] (80073D6C)
    s0 = 0x2D;                                          // Result = 0000002D
loc_80037D20:
    a0 = 0x41;                                          // Result = 00000041
    a1 = s0;
    a2 = s1;
    I_DrawString();
    s1 += 0x10;
    s2++;
    v0 = (i32(s2) < 8);
    s0 += 0x14;
    if (v0 != 0) goto loc_80037D20;
    v0 = lw(gp + 0xA20);                                // Load from: gCursorPos (80078000)
    a0 = 0x41;                                          // Result = 00000041
    if (v0 != s2) goto loc_80037D6C;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    v0 &= 8;
    if (v0 != 0) goto loc_80037D88;
loc_80037D6C:
    a1 = s2 << 2;
    a1 += s2;
    a1 <<= 2;
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7CD4;                                       // Result = STR_Default[0] (80077CD4)
    a1 += 0x2D;
    I_DrawString();
loc_80037D88:
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x38;
    return;
}
