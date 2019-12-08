#include "d_main.h"

#include "PsxVm/PsxVm.h"

void D_DoomMain() noexcept {
loc_80012274:
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    I_PSXInit();
    a2 = 0x51EB0000;                                    // Result = 51EB0000
    v0 = lw(gp + 0x10);                                 // Load from: gOptionsSndVol (800775F0)
    a2 |= 0x851F;                                       // Result = 51EB851F
    v1 = v0 << 7;
    v1 -= v0;
    mult(v1, a2);
    a1 = lw(gp + 0x14);                                 // Load from: gOptionsMusVol (800775F4)
    a0 = hi;
    v0 = a1 << 7;
    v0 -= a1;
    mult(v0, a2);
    s0 = 9;                                             // Result = 00000009
    s1 = 7;                                             // Result = 00000007
    v1 = u32(i32(v1) >> 31);
    a0 = u32(i32(a0) >> 5);
    a0 -= v1;
    v0 = u32(i32(v0) >> 31);
    a2 = 0x800A0000;                                    // Result = 800A0000
    a2 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    a1 = hi;
    a1 = u32(i32(a1) >> 5);
    a1 -= v0;
    PsxSoundInit();
    Z_Init();
    I_Init();
    W_Init();
    R_Init();
    ST_Init();
    sw(0, gp + 0x9C4);                                  // Store to: gPrevGameTic (80077FA4)
    sw(0, gp + 0xA6C);                                  // Store to: gGameTic (8007804C)
    sw(0, gp + 0xCBC);                                  // Store to: gLastTgtGameTicCount (8007829C)
    sw(0, gp + 0xB6C);                                  // Store to: gTicCon (8007814C)
    sw(0, gp + 0x964);                                  // Store to: gPlayerPadButtons[0] (80077F44)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7F48);                                 // Store to: gPlayerPadButtons[1] (80077F48)
    sw(0, gp + 0xC34);                                  // Store to: gPlayerOldPadButtons[0] (80078214)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7DE8);                                 // Store to: gPlayerOldPadButtons[1] (80078218)
loc_80012334:
    RunTitle();
    if (v0 == s0) goto loc_80012374;
    a0 = 2;                                             // Result = 00000002
    RunDemo();
    if (v0 == s0) goto loc_80012374;
    RunCredits();
    if (v0 == s0) goto loc_80012374;
    a0 = 3;                                             // Result = 00000003
    RunDemo();
    if (v0 != s0) goto loc_80012334;
loc_80012374:
    RunMenu();
    if (v0 != s1) goto loc_80012374;
    goto loc_80012334;
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void RunLegals() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0x80030000;                                    // Result = 80030000
    a0 += 0x4F54;                                       // Result = START_Legals (80034F54)
    a1 = 0x80030000;                                    // Result = 80030000
    a1 += 0x4FA0;                                       // Result = STOP_Legals (80034FA0)
    a2 = 0x80030000;                                    // Result = 80030000
    a2 += 0x4FCC;                                       // Result = TIC_Legals (80034FCC)
    a3 = 0x80030000;                                    // Result = 80030000
    a3 += 0x504C;                                       // Result = DRAW_Legals (8003504C)
    MiniLoop();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void RunTitle() noexcept {
loc_800123E4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0x80030000;                                    // Result = 80030000
    a0 += 0x5098;                                       // Result = START_Title (80035098)
    a1 = 0x80030000;                                    // Result = 80030000
    a1 += 0x5268;                                       // Result = STOP_Title (80035268)
    a2 = 0x80030000;                                    // Result = 80030000
    a2 += 0x5294;                                       // Result = TIC_Title (80035294)
    a3 = 0x80030000;                                    // Result = 80030000
    a3 += 0x540C;                                       // Result = DRAW_Title (8003540C)
    MiniLoop();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void RunDemo() noexcept {
loc_80012424:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    a1 = 0x4000;                                        // Result = 00004000
    a2 = 1;                                             // Result = 00000001
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    sw(ra, sp + 0x14);
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2_b();
    sw(v0, gp + 0x8);                                   // Store to: gpDemoBuffer (800775E8)
    a0 = s0;
    OpenFile();
    s0 = v0;
    a0 = s0;
    a1 = lw(gp + 0x8);                                  // Load from: gpDemoBuffer (800775E8)
    a2 = 0x4000;                                        // Result = 00004000
    ReadFile();
    a0 = s0;
    CloseFile();
    G_PlayDemoPtr();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a1 = lw(gp + 0x8);                                  // Load from: gpDemoBuffer (800775E8)
    s0 = v0;
    Z_Free2();
    v0 = s0;
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void RunCredits() noexcept {
loc_800124A8:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0x80030000;                                    // Result = 80030000
    a0 += 0x6BD8;                                       // Result = START_Credits (80036BD8)
    a1 = 0x80030000;                                    // Result = 80030000
    a1 += 0x6CA0;                                       // Result = STOP_Credits (80036CA0)
    a2 = 0x80030000;                                    // Result = 80030000
    a2 += 0x6CC0;                                       // Result = TIC_Credits (80036CC0)
    a3 = 0x80030000;                                    // Result = 80030000
    a3 += 0x6D58;                                       // Result = DRAW_Credits (80036D58)
    MiniLoop();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void I_SetDebugDrawStringPos() noexcept {
    sw(a0, gp + 0xA50);                                 // Store to: gDebugDrawStringXPos (80078030)
    sw(a1, gp + 0xA5C);                                 // Store to: gDebugDrawStringYPos (8007803C)
    return;
}

void I_DebugDrawString() noexcept {
    sw(a0, sp);
    sw(a1, sp + 0x4);
    sw(a2, sp + 0x8);
    sw(a3, sp + 0xC);
    sp -= 0x120;
    sw(s0, sp + 0x118);
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    sw(a0, sp + 0x120);
    a0 = s0;                                            // Result = 1F800200
    a1 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lhu(a3 - 0x6B0E);                              // Load from: gTexInfo_STATUS[2] (800A94F2)
    a2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x11C);
    sw(0, sp + 0x10);
    LIBGPU_SetDrawMode();
    s0 += 4;                                            // Result = 1F800204
    t3 = 0xFF0000;                                      // Result = 00FF0000
    t3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t7 = 0x80080000;                                    // Result = 80080000
    t7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    t8 = t7 & t3;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_80012578:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0)
        v0 = t1 + a0;
        if (bJump) goto loc_800125E0;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0)
        v0 = t2 + a0;
        if (bJump) goto loc_800126A4;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(t7, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= t8;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_800125E0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80012694;
    if (v1 == a0) goto loc_80012578;
loc_80012604:
    v0 = lw(gp + 0x4);                                  // Load from: GPU_REG_GP1 (800775E4)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_80012578;
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
    if (a1 == t4) goto loc_80012670;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80012654:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp);                                        // Load from: GPU_REG_GP0 (800775E0)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_80012654;
loc_80012670:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80012578;
    goto loc_80012604;
loc_80012694:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_800126A4:
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
    if (t0 == v0) goto loc_80012704;
    v1 = -1;                                            // Result = FFFFFFFF
loc_800126EC:
    v0 = lw(s0);
    s0 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_800126EC;
loc_80012704:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_800127B8;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_80012730:
    v0 = lw(gp + 0x4);                                  // Load from: GPU_REG_GP1 (800775E4)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_800127B8;
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
    if (a1 == t0) goto loc_8001279C;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80012780:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp);                                        // Load from: GPU_REG_GP0 (800775E0)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80012780;
loc_8001279C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80012730;
loc_800127B8:
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    a0 = s0;                                            // Result = 1F800200
    LIBGPU_SetSprt();
    a0 = s0;                                            // Result = 1F800200
    a1 = 0;                                             // Result = 00000000
    LIBGPU_SetSemiTrans();
    a0 = s0;                                            // Result = 1F800200
    a1 = 0;                                             // Result = 00000000
    LIBGPU_SetShadeTex();
    a0 = sp + 0x18;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lhu(v1 - 0x6F7C);                              // Load from: gPaletteClutId_Main (800A9084)
    v0 = 0x80;                                          // Result = 00000080
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x204);                                 // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x205);                                 // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x206);                                 // Store to: 1F800206
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x20E);                                 // Store to: 1F80020E
    a1 = lw(sp + 0x120);
    a2 = sp + 0x124;
    D_vsprintf();
    a0 = lw(gp + 0xA50);                                // Load from: gDebugDrawStringXPos (80078030)
    a1 = lw(gp + 0xA5C);                                // Load from: gDebugDrawStringYPos (8007803C)
    a2 = sp + 0x18;
    I_DrawStringSmall();
    v0 = lw(gp + 0xA5C);                                // Load from: gDebugDrawStringYPos (8007803C)
    v0 += 8;
    sw(v0, gp + 0xA5C);                                 // Store to: gDebugDrawStringYPos (8007803C)
    ra = lw(sp + 0x11C);
    s0 = lw(sp + 0x118);
    sp += 0x120;
    return;
}

void MiniLoop() noexcept {
loc_80012B78:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FA4);                               // Load from: gNetGame (8007805C)
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s3, sp + 0x1C);
    s3 = a1;
    sw(s1, sp + 0x14);
    s1 = a2;
    sw(s2, sp + 0x18);
    s2 = a3;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    if (v0 == 0) goto loc_80012BB8;
    I_NetHandshake();
loc_80012BB8:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7EB4);                                 // Store to: gGameAction (80077EB4)
    sw(0, gp + 0x9C4);                                  // Store to: gPrevGameTic (80077FA4)
    sw(0, gp + 0xA6C);                                  // Store to: gGameTic (8007804C)
    sw(0, gp + 0xB6C);                                  // Store to: gTicCon (8007814C)
    sw(0, gp + 0xCBC);                                  // Store to: gLastTgtGameTicCount (8007829C)
    pcall(s0);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7E44);                                 // Store to: gElapsedVBlanks (800781BC)
    a0 = -1;                                            // Result = FFFFFFFF
    LIBETC_VSync();
    s4 = 0x80070000;                                    // Result = 80070000
    s4 += 0x7F44;                                       // Result = gPlayerPadButtons[0] (80077F44)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7EEC);                                // Store to: gLastTotalVBlanks (80078114)
loc_80012BF8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7E44);                               // Load from: gElapsedVBlanks (800781BC)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    sw(v1, at);
    v0 = lw(gp + 0x964);                                // Load from: gPlayerPadButtons[0] (80077F44)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F48);                               // Load from: gPlayerPadButtons[1] (80077F48)
    sw(v0, gp + 0xC34);                                 // Store to: gPlayerOldPadButtons[0] (80078214)
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7DE8);                                // Store to: gPlayerOldPadButtons[1] (80078218)
    I_ReadGamepad();
    a0 = v0;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FA4);                               // Load from: gNetGame (8007805C)
    v1 <<= 2;
    a1 = v1 + s4;
    sw(a0, a1);
    if (v0 == 0) goto loc_80012C80;
    I_NetUpdate();
    {
        const bool bJump = (v0 == 0)
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_80012D34;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7EB4);                                // Store to: gGameAction (80077EB4)
    s0 = 4;                                             // Result = 00000004
    goto loc_80012DBC;
loc_80012C80:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7E54);                               // Load from: gbDemoRecording (800781AC)
    if (v0 != 0) goto loc_80012CA8;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F80);                               // Load from: gbDemoPlayback (80078080)
    if (v0 == 0) goto loc_80012D34;
loc_80012CA8:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F80);                               // Load from: gbDemoPlayback (80078080)
    {
        const bool bJump = (v0 == 0)
        v0 = a0 & 0xF9FF;
        if (bJump) goto loc_80012CE0;
    }
    s0 = 9;                                             // Result = 00000009
    if (v0 != 0) goto loc_80012DBC;
    v1 = lw(gp + 0xC);                                  // Load from: gpDemo_p (800775EC)
    v0 = v1 + 4;
    sw(v0, gp + 0xC);                                   // Store to: gpDemo_p (800775EC)
    a0 = lw(v1);
    sw(a0, a1);
loc_80012CE0:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7E54);                               // Load from: gbDemoRecording (800781AC)
    {
        const bool bJump = (v0 == 0)
        v0 = a0 & 0x800;
        if (bJump) goto loc_80012D0C;
    }
    v1 = lw(gp + 0xC);                                  // Load from: gpDemo_p (800775EC)
    v0 = v1 + 4;
    sw(v0, gp + 0xC);                                   // Store to: gpDemo_p (800775EC)
    sw(a0, v1);
    v0 = a0 & 0x800;
loc_80012D0C:
    s0 = 5;                                             // Result = 00000005
    if (v0 != 0) goto loc_80012DBC;
    v0 = lw(gp + 0xC);                                  // Load from: gpDemo_p (800775EC)
    v1 = lw(gp + 0x8);                                  // Load from: gpDemoBuffer (800775E8)
    v0 -= v1;
    v0 = u32(i32(v0) >> 2);
    v0 = (i32(v0) < 0x4000);
    if (v0 == 0) goto loc_80012DBC;
loc_80012D34:
    v0 = lw(gp + 0xB6C);                                // Load from: gTicCon (8007814C)
    v1 = lw(gp + 0x9DC);                                // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v0 += v1;
    v1 = lw(gp + 0xCBC);                                // Load from: gLastTgtGameTicCount (8007829C)
    a0 = u32(i32(v0) >> 2);
    sw(v0, gp + 0xB6C);                                 // Store to: gTicCon (8007814C)
    v1 = (i32(v1) < i32(a0));
    if (v1 == 0) goto loc_80012D6C;
    v0 = lw(gp + 0xA6C);                                // Load from: gGameTic (8007804C)
    sw(a0, gp + 0xCBC);                                 // Store to: gLastTgtGameTicCount (8007829C)
    v0++;
    sw(v0, gp + 0xA6C);                                 // Store to: gGameTic (8007804C)
loc_80012D6C:
    pcall(s1);
    s0 = v0;
    if (s0 != 0) goto loc_80012DBC;
    pcall(s2);
    v1 = lw(gp + 0xA6C);                                // Load from: gGameTic (8007804C)
    v0 = lw(gp + 0x9C4);                                // Load from: gPrevGameTic (80077FA4)
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_80012DA8;
    S_UpdateSounds();
loc_80012DA8:
    v0 = lw(gp + 0xA6C);                                // Load from: gGameTic (8007804C)
    sw(v0, gp + 0x9C4);                                 // Store to: gPrevGameTic (80077FA4)
    goto loc_80012BF8;
loc_80012DBC:
    a0 = s0;
    pcall(s3);
    v1 = lw(gp + 0x964);                                // Load from: gPlayerPadButtons[0] (80077F44)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7F48);                               // Load from: gPlayerPadButtons[1] (80077F48)
    v0 = s0;
    sw(v1, gp + 0xC34);                                 // Store to: gPlayerOldPadButtons[0] (80078214)
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0x7DE8);                                // Store to: gPlayerOldPadButtons[1] (80078218)
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void Read_Vlq() noexcept {
loc_80047664:
    a2 = a0;
    v1 = lbu(a2);
    v0 = v1 & 0x80;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xE00);                                 // Store to: gWess_Read_Vlq_v (8007F200)
    a2++;
    if (v0 == 0) goto loc_800476C4;
    v0 = v1 & 0x7F;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE00);                                 // Store to: gWess_Read_Vlq_v (8007F200)
loc_80047690:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE00);                                // Load from: gWess_Read_Vlq_v (8007F200)
    a0 = lbu(a2);
    v0 <<= 7;
    v1 = a0 & 0x7F;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sb(a0, at - 0xDFC);                                 // Store to: gWess_Read_Vlq_c (8007F204)
    a0 &= 0x80;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE00);                                 // Store to: gWess_Read_Vlq_v (8007F200)
    a2++;
    if (a0 != 0) goto loc_80047690;
loc_800476C4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE00);                                // Load from: gWess_Read_Vlq_v (8007F200)
    sw(v0, a1);
    v0 = a2;
    return;
}

void Write_Vlq() noexcept {
    sp -= 0x10;
    v0 = a1 & 0x7F;
    sb(v0, sp);
    a1 >>= 7;
    v1 = sp + 1;
    if (a1 == 0) goto loc_80047708;
loc_800476F4:
    v0 = a1 | 0x80;
    sb(v0, v1);
    a1 >>= 7;
    v1++;
    if (a1 != 0) goto loc_800476F4;
loc_80047708:
    v1--;
    v0 = lbu(v1);
    sb(v0, a0);
    v0 = lbu(v1);
    v0 &= 0x80;
    a0++;
    if (v0 != 0) goto loc_80047708;
    v0 = a0;
    sp += 0x10;
    return;
}

void Len_Vlq() noexcept {
    sp -= 0x20;
    a1 = sp + 0x10;
    v0 = a0 & 0x7F;
    sb(v0, sp);
    a0 >>= 7;
    v1 = sp + 1;
    if (a0 == 0) goto loc_8004776C;
loc_80047758:
    v0 = a0 | 0x80;
    sb(v0, v1);
    a0 >>= 7;
    v1++;
    if (a0 != 0) goto loc_80047758;
loc_8004776C:
    v1--;
    v0 = lbu(v1);
    sb(v0, a1);
    v0 = lbu(v1);
    v0 &= 0x80;
    a1++;
    if (v0 != 0) goto loc_8004776C;
    v0 = sp + 0x10;
    v0 = a1 - v0;
    v0 &= 0xFF;
    sp += 0x20;
    return;
}

void Eng_DriverInit() noexcept {
loc_800477A8:
    v0 = lw(a0 + 0x28);
    v1 = lw(a0 + 0x20);
    a1 = lw(a0 + 0xC);
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5AC0);                                // Store to: gWess_SeqEngine_pm_stat (80075AC0)
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AB8);                                // Store to: 80075AB8
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5ABC);                                // Store to: gWess_Eng_piter (80075ABC)
    v0 = lbu(a1 + 0xC);
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x5AB4);                                // Store to: 80075AB4
    return;
}

void Eng_DriverExit() noexcept {
loc_800477E4:
    return;
}

void Eng_DriverEntry1() noexcept {
loc_800477EC:
    return;
}

void Eng_DriverEntry2() noexcept {
loc_800477F4:
    return;
}

void Eng_DriverEntry3() noexcept {
loc_800477FC:
    return;
}

void Eng_TrkOff() noexcept {
loc_80047804:
    a1 = a0;
    v1 = lbu(a1 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    a0 = lw(a1);
    v1 = lw(v1 + 0x20);
    v0 <<= 3;
    v1 += v0;
    v0 = a0 & 8;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDF8);                                 // Store to: 8007F208
    {
        const bool bJump = (v0 != 0)
        v0 = a0 | 8;
        if (bJump) goto loc_80047874;
    }
    sw(v0, a1);
    v0 = lbu(v1 + 0x5);
    v0--;
    sb(v0, v1 + 0x5);
    v0 &= 0xFF;
    if (v0 != 0) goto loc_80047874;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xDF8);                                // Load from: 8007F208
    sb(0, v0 + 0x1);
loc_80047874:
    v0 = lw(a1);
    v0 &= 4;
    if (v0 != 0) goto loc_8004799C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xDF8);                                // Load from: 8007F208
    v1 = lbu(v0 + 0x1C);
    v0 = lw(a0 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDF0);                                 // Store to: 8007F210
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDF4);                                 // Store to: 8007F20C
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDF0);                                 // Store to: 8007F210
    a2 = -1;                                            // Result = FFFFFFFF
    if (v1 == v0) goto loc_8004791C;
    a3 = 0xFF;                                          // Result = 000000FF
loc_800478CC:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xDF4);                                // Load from: 8007F20C
    v0 = lbu(a1 + 0x1);
    v1 = lbu(a0);
    {
        const bool bJump = (v1 != v0)
        v0 = a0 + 1;
        if (bJump) goto loc_800478F8;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDF4);                                 // Store to: 8007F20C
    sb(a3, a0);
    goto loc_8004791C;
loc_800478F8:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xDF0);                                // Load from: 8007F210
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDF4);                                 // Store to: 8007F20C
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDF0);                                 // Store to: 8007F210
    if (v1 != a2) goto loc_800478CC;
loc_8004791C:
    a2 = -2;                                            // Result = FFFFFFFE
    v0 = lw(a1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v0 &= a2;
    sw(v0, a1);
    v0 = lbu(v1 + 0x5);
    v0--;
    sb(v0, v1 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xDF8);                                // Load from: 8007F208
    v0 = lbu(v1 + 0x4);
    v0--;
    sb(v0, v1 + 0x4);
    v0 &= 0xFF;
    if (v0 != 0) goto loc_8004799C;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xDF8);                                // Load from: 8007F208
    v0 = lw(v1);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v0 &= a2;
    sw(v0, v1);
    v0 = lbu(a0 + 0x4);
    v0--;
    sb(v0, a0 + 0x4);
loc_8004799C:
    v0 = lw(a1);
    v1 = -0x11;                                         // Result = FFFFFFEF
    v0 &= v1;
    sw(v0, a1);
    return;
}

void Eng_TrkMute() noexcept {
loc_800479B0:
    return;
}

void Eng_PatchChg() noexcept {
loc_800479B8:
    v0 = lw(a0 + 0x34);
    v0 = lbu(v0 + 0x1);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDEC);                                 // Store to: gWess_Eng_thepatch (8007F214)
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xDEC);                               // Load from: gWess_Eng_thepatch (8007F214)
    sh(v0, a0 + 0xA);
    return;
}

void Eng_PatchMod() noexcept {
loc_800479E0:
    return;
}

void Eng_PitchMod() noexcept {
loc_800479E8:
    v1 = lw(a0 + 0x34);
    v0 = lbu(v1 + 0x2);
    v1 = lbu(v1 + 0x1);
    v0 <<= 8;
    v1 |= v0;
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at - 0xDE8);                                 // Store to: gWess_Eng_thepitchmod (8007F218)
    sh(v1, a0 + 0xE);
    return;
}

void Eng_ZeroMod() noexcept {
loc_80047A10:
    return;
}

void Eng_ModuMod() noexcept {
loc_80047A18:
    return;
}

void Eng_VolumeMod() noexcept {
loc_80047A20:
    v0 = lw(a0 + 0x34);
    v0 = lbu(v0 + 0x1);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDE4);                                 // Store to: gWess_Eng_thevolume (8007F21C)
    sb(v0, a0 + 0xC);
    return;
}

void Eng_PanMod() noexcept {
loc_80047A40:
    v0 = lw(a0 + 0x34);
    v0 = lbu(v0 + 0x1);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDE0);                                 // Store to: gWess_Eng_thepan (8007F220)
    sb(v0, a0 + 0xD);
    return;
}

void Eng_PedalMod() noexcept {
loc_80047A60:
    return;
}

void Eng_ReverbMod() noexcept {
loc_80047A68:
    return;
}

void Eng_ChorusMod() noexcept {
loc_80047A70:
    return;
}

void Eng_NoteOn() noexcept {
loc_80047A78:
    return;
}

void Eng_NoteOff() noexcept {
loc_80047A80:
    return;
}

void Eng_StatusMark() noexcept {
loc_80047A88:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lbu(v1 + 0xA);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDD8);                                 // Store to: 8007F228
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xDD8);                               // Load from: 8007F228
    if (v0 == 0) goto loc_80047B94;
    v0 = lw(v1 + 0x10);
    v1 = lw(v1 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDD4);                                 // Store to: 8007F22C
    v1 = lbu(v1 + 0xF);
    v0 = v1 + 0xFF;
    goto loc_80047B84;
loc_80047AD8:
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0xDD4);                                // Load from: 8007F22C
    v0 = lbu(a2);
    if (v0 == 0) goto loc_80047B64;
    a1 = lw(a0 + 0x34);
    v1 = lbu(a2 + 0x1);
    v0 = lbu(a1 + 0x1);
    if (v1 != v0) goto loc_80047B40;
    v0 = lbu(a1 + 0x3);
    a1 = lbu(a1 + 0x2);
    a0 = lbu(a2 + 0x1);
    v0 <<= 8;
    a1 |= v0;
    sh(a1, a2 + 0x2);
    a1 <<= 16;
    v0 = lw(a2 + 0x4);
    a1 = u32(i32(a1) >> 16);
    pcall(v0);
    goto loc_80047B94;
loc_80047B40:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xDD8);                               // Load from: 8007F228
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDD8);                                 // Store to: 8007F228
    v0 &= 0xFF;
    if (v0 == 0) goto loc_80047B94;
loc_80047B64:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xDD4);                                // Load from: 8007F22C
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xDDC);                               // Load from: 8007F224
    v0 += 8;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDD4);                                 // Store to: 8007F22C
    v0 = v1 + 0xFF;
loc_80047B84:
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDDC);                                 // Store to: 8007F224
    if (v1 != 0) goto loc_80047AD8;
loc_80047B94:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Eng_GateJump() noexcept {
loc_80047BA4:
    a1 = a0;
    v1 = lbu(a1 + 0x2);
    a0 = lw(a1 + 0x34);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v1 = lbu(a0 + 0x1);
    v0 = lw(v0 + 0x10);
    v1 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDC8);                                 // Store to: 8007F238
    a0 = lbu(v1);
    v0 = 0xFF;                                          // Result = 000000FF
    if (a0 == 0) goto loc_80047C7C;
    if (a0 != v0) goto loc_80047C0C;
    v0 = lw(a1 + 0x34);
    v0 = lbu(v0 + 0x2);
    sb(v0, v1);
loc_80047C0C:
    v0 = lw(a1 + 0x34);
    v1 = lbu(v0 + 0x4);
    v0 = lbu(v0 + 0x3);
    v1 <<= 8;
    v0 |= v1;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xDD0);                                 // Store to: 8007F230
    v0 <<= 16;
    a0 = u32(i32(v0) >> 16);
    if (i32(a0) < 0) goto loc_80047C7C;
    v0 = lh(a1 + 0x18);
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 == 0)
        v0 = a0 << 2;
        if (bJump) goto loc_80047C7C;
    }
    v1 = lw(a1 + 0x38);
    v0 += v1;
    v0 = lw(v0);
    v1 = lw(a1 + 0x30);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDCC);                                 // Store to: 8007F234
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDCC);                                 // Store to: 8007F234
    sw(v0, a1 + 0x34);
loc_80047C7C:
    v0 = lw(a1);
    v0 |= 0x40;
    sw(v0, a1);
    return;
}

void Eng_IterJump() noexcept {
loc_80047C90:
    a1 = a0;
    v1 = lbu(a1 + 0x2);
    a0 = lw(a1 + 0x34);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v1 = lbu(a0 + 0x1);
    v0 = lw(v0 + 0x14);
    v1 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDBC);                                 // Store to: 8007F244
    a0 = lbu(v1);
    v0 = 0xFF;                                          // Result = 000000FF
    if (a0 == 0) goto loc_80047D78;
    if (a0 != v0) goto loc_80047CF8;
    v0 = lw(a1 + 0x34);
    v0 = lbu(v0 + 0x2);
    sb(v0, v1);
    goto loc_80047D08;
loc_80047CF8:
    v0 = lbu(v1);
    v0--;
    sb(v0, v1);
loc_80047D08:
    v0 = lw(a1 + 0x34);
    v1 = lbu(v0 + 0x4);
    v0 = lbu(v0 + 0x3);
    v1 <<= 8;
    v0 |= v1;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xDC4);                                 // Store to: 8007F23C
    v0 <<= 16;
    a0 = u32(i32(v0) >> 16);
    if (i32(a0) < 0) goto loc_80047D78;
    v0 = lh(a1 + 0x18);
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 == 0)
        v0 = a0 << 2;
        if (bJump) goto loc_80047D78;
    }
    v1 = lw(a1 + 0x38);
    v0 += v1;
    v0 = lw(v0);
    v1 = lw(a1 + 0x30);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDC0);                                 // Store to: 8007F240
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDC0);                                 // Store to: 8007F240
    sw(v0, a1 + 0x34);
loc_80047D78:
    v0 = lw(a1);
    v0 |= 0x40;
    sw(v0, a1);
    return;
}

void Eng_ResetGates() noexcept {
loc_80047D8C:
    v1 = a0;
    v0 = lw(v1 + 0x34);
    a0 = lbu(v0 + 0x1);
    v0 = 0xFF;                                          // Result = 000000FF
    if (a0 != v0) goto loc_80047E50;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v0 = lw(v0 + 0xC);
    a0 = lbu(v0 + 0xD);
    at = 0x80080000;                                    // Result = 80080000
    sb(a0, at - 0xDB8);                                 // Store to: 8007F248
    v1 = lbu(v1 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v1 = lw(v0 + 0x10);
    v0 = a0 + 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDB8);                                 // Store to: 8007F248
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDB4);                                 // Store to: 8007F24C
    {
        const bool bJump = (a0 == 0)
        a0 = 0xFF;                                      // Result = 000000FF
        if (bJump) goto loc_80047E88;
    }
loc_80047E0C:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xDB4);                                // Load from: 8007F24C
    v0 = v1 + 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDB4);                                 // Store to: 8007F24C
    sb(a0, v1);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xDB8);                               // Load from: 8007F248
    v0 = v1 + 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDB8);                                 // Store to: 8007F248
    if (v1 == 0) goto loc_80047E88;
    goto loc_80047E0C;
loc_80047E50:
    v1 = lbu(v1 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v0 = lw(v0 + 0x10);
    v1 = 0xFF;                                          // Result = 000000FF
    v0 += a0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDB4);                                 // Store to: 8007F24C
    sb(v1, v0);
loc_80047E88:
    return;
}

void Eng_ResetIters() noexcept {
loc_80047E90:
    v1 = a0;
    v0 = lw(v1 + 0x34);
    a0 = lbu(v0 + 0x1);
    v0 = 0xFF;                                          // Result = 000000FF
    if (a0 != v0) goto loc_80047F54;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v0 = lw(v0 + 0xC);
    a0 = lbu(v0 + 0xE);
    at = 0x80080000;                                    // Result = 80080000
    sb(a0, at - 0xDB0);                                 // Store to: 8007F250
    v1 = lbu(v1 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v1 = lw(v0 + 0x14);
    v0 = a0 + 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDB0);                                 // Store to: 8007F250
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDAC);                                 // Store to: 8007F254
    {
        const bool bJump = (a0 == 0)
        a0 = 0xFF;                                      // Result = 000000FF
        if (bJump) goto loc_80047F8C;
    }
loc_80047F10:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xDAC);                                // Load from: 8007F254
    v0 = v1 + 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDAC);                                 // Store to: 8007F254
    sb(a0, v1);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xDB0);                               // Load from: 8007F250
    v0 = v1 + 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xDB0);                                 // Store to: 8007F250
    if (v1 == 0) goto loc_80047F8C;
    goto loc_80047F10;
loc_80047F54:
    v1 = lbu(v1 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v0 = lw(v0 + 0x14);
    v1 = 0xFF;                                          // Result = 000000FF
    v0 += a0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xDAC);                                 // Store to: 8007F254
    sb(v1, v0);
loc_80047F8C:
    return;
}

void Eng_WriteIterBox() noexcept {
loc_80047F94:
    v1 = lbu(a0 + 0x2);
    a1 = lw(a0 + 0x34);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    v0 <<= 3;
    v0 += v1;
    v1 = lbu(a1 + 0x1);
    v0 = lw(v0 + 0x14);
    a0 = lw(a0 + 0x34);
    v1 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xDA8);                                 // Store to: 8007F258
    v0 = lbu(a0 + 0x2);
    sb(v0, v1);
    return;
}

void Eng_SeqTempo() noexcept {
loc_80047FD8:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v0 = lbu(s0 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    a0 = lw(a0 + 0xC);
    v1 += v0;
    a1 = lh(v1 + 0x2);
    a0 = lw(a0 + 0x10);
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    v0 += a0;
    v0 = lhu(v0);
    a0 = lw(v1 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD94);                                 // Store to: 8007F26C
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xDA4);                                 // Store to: 8007F25C
    v1 = lbu(v1 + 0x4);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xDA4);                                 // Store to: 8007F25C
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD9C);                                 // Store to: 8007F264
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xDA0);                                 // Store to: 8007F260
    v1 = -1;                                            // Result = FFFFFFFF
    if (v0 == v1) goto loc_80048144;
loc_80048074:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xD9C);                                // Load from: 8007F264
    v1 = lbu(a0);
    v0 = 0xFF;                                          // Result = 000000FF
    {
        const bool bJump = (v1 == v0)
        v0 = a0 + 1;
        if (bJump) goto loc_8004811C;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD9C);                                 // Store to: 8007F264
    v0 = lbu(a0);
    v1 = lw(s0 + 0x34);
    a0 = v0 << 2;
    a0 += v0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AB8);                               // Load from: 80075AB8
    a0 <<= 4;
    a0 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD98);                                 // Store to: 8007F268
    v0 = lbu(v1 + 0x2);
    v1 = lbu(v1 + 0x1);
    v0 <<= 8;
    v1 |= v0;
    sh(v1, a0 + 0x16);
    GetIntsPerSec();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD98);                                // Load from: 8007F268
    v0 <<= 16;
    a1 = lh(v1 + 0x14);
    a2 = lh(v1 + 0x16);
    a0 = u32(i32(v0) >> 16);
    CalcPartsPerInt();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xDA0);                               // Load from: 8007F260
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xD98);                                // Load from: 8007F268
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xDA0);                                 // Store to: 8007F260
    v1 &= 0xFF;
    sw(v0, a0 + 0x1C);
    if (v1 == 0) goto loc_80048144;
loc_8004811C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xDA4);                               // Load from: 8007F25C
    v1 = -1;                                            // Result = FFFFFFFF
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xDA4);                                 // Store to: 8007F25C
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    if (v0 != v1) goto loc_80048074;
loc_80048144:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Eng_SeqGosub() noexcept {
loc_80048158:
    a2 = a0;
    v0 = lw(a2 + 0x34);
    v1 = lbu(v0 + 0x2);
    v0 = lbu(v0 + 0x1);
    v1 <<= 8;
    v0 |= v1;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD90);                                 // Store to: 8007F270
    v0 <<= 16;
    v1 = u32(i32(v0) >> 16);
    if (i32(v1) < 0) goto loc_80048320;
    v0 = lh(a2 + 0x18);
    v0 = (i32(v1) < i32(v0));
    if (v0 == 0) goto loc_80048320;
    v0 = lbu(a2 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    a0 = lw(a0 + 0xC);
    v1 += v0;
    a1 = lh(v1 + 0x2);
    a0 = lw(a0 + 0x10);
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    v0 += a0;
    v0 = lhu(v0);
    a0 = lw(v1 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD78);                                 // Store to: 8007F288
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD8C);                                 // Store to: 8007F274
    v1 = lbu(v1 + 0x4);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD8C);                                 // Store to: 8007F274
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD80);                                 // Store to: 8007F280
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xD84);                                 // Store to: 8007F27C
    v1 = -1;                                            // Result = FFFFFFFF
    if (v0 == v1) goto loc_80048320;
    t0 = 0xFF;                                          // Result = 000000FF
    a3 = 0x80070000;                                    // Result = 80070000
    a3 += 0x5B1A;                                       // Result = gWess_CmdLength[1A] (80075B1A)
loc_80048238:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD80);                                // Load from: 8007F280
    v0 = lbu(v1);
    {
        const bool bJump = (v0 == t0)
        v0 = v1 + 1;
        if (bJump) goto loc_800482F8;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD80);                                 // Store to: 8007F280
    v1 = lbu(v1);
    v0 = v1 << 2;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AB8);                               // Load from: 80075AB8
    v0 <<= 4;
    v0 += v1;
    a1 = lw(v0 + 0x40);
    a0 = lw(v0 + 0x34);
    v1 = a1 + 4;
    sw(v1, v0 + 0x40);
    v1 = lbu(a3);                                       // Load from: gWess_CmdLength[1A] (80075B1A)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD7C);                                 // Store to: 8007F284
    v1 += a0;
    sw(v1, a1);
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0xD7C);                                // Load from: 8007F284
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lh(v0 - 0xD90);                                // Load from: 8007F270
    v1 = lw(a1 + 0x38);
    v0 <<= 2;
    v0 += v1;
    v1 = lw(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xD84);                               // Load from: 8007F27C
    a0 = lw(a1 + 0x30);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD88);                                 // Store to: 8007F278
    v1 += a0;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xD84);                                 // Store to: 8007F27C
    v0 &= 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD88);                                 // Store to: 8007F278
    sw(v1, a1 + 0x34);
    if (v0 == 0) goto loc_80048320;
loc_800482F8:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xD8C);                               // Load from: 8007F274
    v1 = -1;                                            // Result = FFFFFFFF
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD8C);                                 // Store to: 8007F274
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    if (v0 != v1) goto loc_80048238;
loc_80048320:
    v0 = lw(a2);
    v0 |= 0x40;
    sw(v0, a2);
    return;
}

void Eng_SeqJump() noexcept {
loc_80048334:
    a3 = a0;
    v0 = lw(a3 + 0x34);
    v1 = lbu(v0 + 0x2);
    v0 = lbu(v0 + 0x1);
    v1 <<= 8;
    v0 |= v1;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD74);                                 // Store to: 8007F28C
    v0 <<= 16;
    a2 = u32(i32(v0) >> 16);
    if (i32(a2) < 0) goto loc_800484C8;
    v0 = lh(a3 + 0x18);
    v0 = (i32(a2) < i32(v0));
    if (v0 == 0) goto loc_800484C8;
    v0 = lbu(a3 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    a0 = lw(a0 + 0xC);
    v1 += v0;
    a1 = lh(v1 + 0x2);
    a0 = lw(a0 + 0x10);
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    v0 += a0;
    v0 = lhu(v0);
    a0 = lw(v1 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD5C);                                 // Store to: 8007F2A4
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD70);                                 // Store to: 8007F290
    v1 = lbu(v1 + 0x4);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD70);                                 // Store to: 8007F290
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD64);                                 // Store to: 8007F29C
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xD68);                                 // Store to: 8007F298
    v1 = -1;                                            // Result = FFFFFFFF
    if (v0 == v1) goto loc_800484C8;
    t1 = 0xFF;                                          // Result = 000000FF
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x5AB8);                               // Load from: 80075AB8
    a2 <<= 2;
loc_80048418:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD64);                                // Load from: 8007F29C
    v0 = lbu(v1);
    {
        const bool bJump = (v0 == t1)
        v0 = v1 + 1;
        if (bJump) goto loc_800484A0;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD64);                                 // Store to: 8007F29C
    v0 = lbu(v1);
    v1 = v0 << 2;
    v1 += v0;
    v1 <<= 4;
    v1 += t0;
    v0 = lw(v1 + 0x38);
    a1 = lw(v1 + 0x30);
    v0 += a2;
    a0 = lw(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xD68);                               // Load from: 8007F298
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD60);                                 // Store to: 8007F2A0
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD6C);                                 // Store to: 8007F294
    a0 += a1;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xD68);                                 // Store to: 8007F298
    v0 &= 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD6C);                                 // Store to: 8007F294
    sw(a0, v1 + 0x34);
    if (v0 == 0) goto loc_800484C8;
loc_800484A0:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xD70);                               // Load from: 8007F290
    v1 = -1;                                            // Result = FFFFFFFF
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD70);                                 // Store to: 8007F290
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    if (v0 != v1) goto loc_80048418;
loc_800484C8:
    v0 = lw(a3);
    v0 |= 0x40;
    sw(v0, a3);
    return;
}

void Eng_SeqRet() noexcept {
loc_800484DC:
    a2 = a0;
    v0 = lbu(a2 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5ABC);                               // Load from: gWess_Eng_piter (80075ABC)
    a0 = lw(a0 + 0xC);
    v1 += v0;
    a1 = lh(v1 + 0x2);
    a0 = lw(a0 + 0x10);
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    v0 += a0;
    v0 = lhu(v0);
    a0 = lw(v1 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD48);                                 // Store to: 8007F2B8
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD58);                                 // Store to: 8007F2A8
    v1 = lbu(v1 + 0x4);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD58);                                 // Store to: 8007F2A8
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD50);                                 // Store to: 8007F2B0
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xD54);                                 // Store to: 8007F2AC
    v1 = -1;                                            // Result = FFFFFFFF
    if (v0 == v1) goto loc_80048618;
    t0 = 0xFF;                                          // Result = 000000FF
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5AB8);                               // Load from: 80075AB8
    a3 = -1;                                            // Result = FFFFFFFF
loc_8004857C:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD50);                                // Load from: 8007F2B0
    v0 = lbu(v1);
    {
        const bool bJump = (v0 == t0)
        v0 = v1 + 1;
        if (bJump) goto loc_800485F0;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD50);                                 // Store to: 8007F2B0
    v0 = lbu(v1);
    v1 = v0 << 2;
    v1 += v0;
    v1 <<= 4;
    v1 += a1;
    a0 = lw(v1 + 0x40);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD4C);                                 // Store to: 8007F2B4
    v0 = a0 - 4;
    sw(v0, v1 + 0x40);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xD54);                               // Load from: 8007F2AC
    a0 = lw(a0 - 0x4);
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xD54);                                 // Store to: 8007F2AC
    v0 &= 0xFF;
    sw(a0, v1 + 0x34);
    if (v0 == 0) goto loc_80048618;
loc_800485F0:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xD58);                               // Load from: 8007F2A8
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xD58);                                 // Store to: 8007F2A8
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    if (v0 != a3) goto loc_8004857C;
loc_80048618:
    v0 = lw(a2);
    v0 |= 0x40;
    sw(v0, a2);
    return;
}

void Eng_SeqEnd() noexcept {
loc_8004862C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0);
    v0 &= 4;
    if (v0 != 0) goto loc_80048784;
    v1 = lbu(s0 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v0 = v1 << 1;
    v0 += v1;
    v1 = lw(a0 + 0x20);
    v0 <<= 3;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD44);                                 // Store to: 8007F2BC
    v1 = lbu(v0 + 0x4);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD38);                                 // Store to: 8007F2C8
    v1 = lw(v0 + 0xC);
    v0 = lbu(a0 + 0x1C);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD34);                                 // Store to: 8007F2CC
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD3C);                                 // Store to: 8007F2C4
    v1 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD34);                                 // Store to: 8007F2CC
    if (v0 == v1) goto loc_800488BC;
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
loc_800486C8:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD3C);                                // Load from: 8007F2C4
    v1 = lbu(v0);
    v0 = 0xFF;                                          // Result = 000000FF
    a0 = v1 << 2;
    if (v1 == v0) goto loc_80048748;
    a0 += v1;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AB8);                               // Load from: 80075AB8
    a0 <<= 4;
    a0 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD40);                                 // Store to: 8007F2C0
    v0 = lbu(a0 + 0x3);
    v0 <<= 2;
    v0 += s0;
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    pcall(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD38);                                // Load from: 8007F2C8
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD38);                                 // Store to: 8007F2C8
    if (v0 == 0) goto loc_800488BC;
loc_80048748:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD3C);                                // Load from: 8007F2C4
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD34);                                // Load from: 8007F2CC
    v0++;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD3C);                                 // Store to: 8007F2C4
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD34);                                 // Store to: 8007F2CC
    if (v1 == v0) goto loc_800488BC;
    goto loc_800486C8;
loc_80048784:
    v1 = lbu(s0 + 0x2);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    v0 = v1 << 1;
    v0 += v1;
    v1 = lw(a0 + 0x20);
    v0 <<= 3;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD44);                                 // Store to: 8007F2BC
    v1 = lbu(v0 + 0x4);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD38);                                 // Store to: 8007F2C8
    v1 = lw(v0 + 0xC);
    v0 = lbu(a0 + 0x1C);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD34);                                 // Store to: 8007F2CC
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD3C);                                 // Store to: 8007F2C4
    v1 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD34);                                 // Store to: 8007F2CC
    if (v0 == v1) goto loc_800488AC;
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
loc_800487F8:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD3C);                                // Load from: 8007F2C4
    v1 = lbu(v0);
    v0 = 0xFF;                                          // Result = 000000FF
    a0 = v1 << 2;
    if (v1 == v0) goto loc_80048878;
    a0 += v1;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AB8);                               // Load from: 80075AB8
    a0 <<= 4;
    a0 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD40);                                 // Store to: 8007F2C0
    v0 = lbu(a0 + 0x3);
    v0 <<= 2;
    v0 += s1;
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    pcall(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD38);                                // Load from: 8007F2C8
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD38);                                 // Store to: 8007F2C8
    if (v0 == 0) goto loc_800488AC;
loc_80048878:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD3C);                                // Load from: 8007F2C4
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD34);                                // Load from: 8007F2CC
    v0++;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD3C);                                 // Store to: 8007F2C4
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD34);                                 // Store to: 8007F2CC
    if (v1 != v0) goto loc_800487F8;
loc_800488AC:
    v0 = lw(s0);
    v0 |= 0x40;
    sw(v0, s0);
loc_800488BC:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void Eng_TrkTempo() noexcept {
loc_800488D4:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lw(s0 + 0x34);
    v0 = lbu(v1 + 0x2);
    v1 = lbu(v1 + 0x1);
    v0 <<= 8;
    v1 |= v0;
    sh(v1, s0 + 0x16);
    GetIntsPerSec();
    v0 <<= 16;
    a1 = lh(s0 + 0x14);
    a2 = lh(s0 + 0x16);
    a0 = u32(i32(v0) >> 16);
    CalcPartsPerInt();
    sw(v0, s0 + 0x1C);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Eng_TrkGosub() noexcept {
loc_80048930:
    a1 = a0;
    v0 = lw(a1 + 0x34);
    v1 = lbu(v0 + 0x2);
    v0 = lbu(v0 + 0x1);
    v1 <<= 8;
    v0 |= v1;
    v0 <<= 16;
    a2 = u32(i32(v0) >> 16);
    if (i32(a2) < 0) goto loc_800489BC;
    v0 = lh(a1 + 0x18);
    v0 = (i32(a2) < i32(v0));
    if (v0 == 0) goto loc_800489BC;
    a0 = lw(a1 + 0x40);
    v1 = lw(a1 + 0x34);
    v0 = a0 + 4;
    sw(v0, a1 + 0x40);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5B1A);                              // Load from: gWess_CmdLength[1A] (80075B1A)
    v0 += v1;
    sw(v0, a0);
    v1 = lw(a1 + 0x38);
    v0 = a2 << 2;
    v0 += v1;
    v1 = lw(v0);
    v0 = lw(a1);
    a0 = lw(a1 + 0x30);
    v0 |= 0x40;
    v1 += a0;
    sw(v0, a1);
    sw(v1, a1 + 0x34);
loc_800489BC:
    return;
}

void Eng_TrkJump() noexcept {
loc_800489C4:
    a1 = a0;
    v0 = lw(a1 + 0x34);
    v1 = lbu(v0 + 0x2);
    v0 = lbu(v0 + 0x1);
    v1 <<= 8;
    v0 |= v1;
    v0 <<= 16;
    a0 = u32(i32(v0) >> 16);
    if (i32(a0) < 0) goto loc_80048A2C;
    v0 = lh(a1 + 0x18);
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 == 0)
        v0 = a0 << 2;
        if (bJump) goto loc_80048A2C;
    }
    v1 = lw(a1 + 0x38);
    a0 = lw(a1 + 0x30);
    v0 += v1;
    v1 = lw(v0);
    v0 = lw(a1);
    sw(0, a1 + 0x4);
    v0 |= 0x40;
    v1 += a0;
    sw(v0, a1);
    sw(v1, a1 + 0x34);
loc_80048A2C:
    return;
}

void Eng_TrkRet() noexcept {
loc_80048A34:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lw(s0 + 0x40);
    v0 = v1 - 4;
    sw(v0, s0 + 0x40);
    a0 = lw(v1 - 0x4);
    a1 = s0 + 4;
    sw(a0, s0 + 0x34);
    Read_Vlq();
    v1 = lw(s0);
    sw(v0, s0 + 0x34);
    v1 |= 0x40;
    sw(v1, s0);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Eng_TrkEnd() noexcept {
loc_80048A88:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lw(s0);
    v0 = v1 & 4;
    {
        const bool bJump = (v0 != 0)
        v0 = v1 & 0x20;
        if (bJump) goto loc_80048B00;
    }
    if (v0 == 0) goto loc_80048AC8;
    v0 = lw(s0 + 0x28);
    v0 = (v0 < 0x10);
    a1 = s0 + 4;
    if (v0 == 0) goto loc_80048B1C;
loc_80048AC8:
    v0 = lbu(s0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x14);
    a0 = s0;
    pcall(v0);
    goto loc_80048B78;
loc_80048B00:
    if (v0 == 0) goto loc_80048B38;
    v0 = lw(s0 + 0x28);
    v0 = (v0 < 0x10);
    a1 = s0 + 4;
    if (v0 != 0) goto loc_80048B38;
loc_80048B1C:
    a0 = lw(s0 + 0x30);
    v0 = v1 | 0x40;
    sw(v0, s0);
    sw(a0, s0 + 0x34);
    Read_Vlq();
    sw(v0, s0 + 0x34);
    goto loc_80048B78;
loc_80048B38:
    v0 = lbu(s0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x14);
    a0 = s0;
    pcall(v0);
    v0 = lw(s0);
    v0 |= 0x40;
    sw(v0, s0);
loc_80048B78:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Eng_NullEvent() noexcept {
loc_80048B8C:
    return;
}

void SeqEngine() noexcept {
loc_80048B94:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AC0);                               // Load from: gWess_SeqEngine_pm_stat (80075AC0)
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lbu(v0 + 0x5);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xD2C);                                 // Store to: gWess_SeqEngine_tmpNumTracks (8007F2D4)
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xD2C);                               // Load from: gWess_SeqEngine_tmpNumTracks (8007F2D4)
    if (v0 == 0) goto loc_80048E8C;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x5AB4);                              // Load from: 80075AB4
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AB8);                               // Load from: 80075AB8
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD30);                                 // Store to: 8007F2D0
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD28);                                 // Store to: 8007F2D8
    if (v1 == v0) goto loc_80048E8C;
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x5B00;                                       // Result = gWess_CmdLength[0] (80075B00)
loc_80048C14:
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0xD30);                                // Load from: 8007F2D0
    v1 = lw(a2);
    v0 = v1 & 1;
    {
        const bool bJump = (v0 == 0)
        v0 = v1 & 8;
        if (bJump) goto loc_80048E58;
    }
    if (v0 != 0) goto loc_80048E34;
    v0 = lw(a2 + 0x20);
    v1 = lw(a2 + 0x1C);
    a0 = lw(a2 + 0x24);
    v0 += v1;
    sw(v0, a2 + 0x20);
    v0 >>= 16;
    v1 = lw(a2 + 0x28);
    a1 = lhu(a2 + 0x20);
    v0 += v1;
    sw(v0, a2 + 0x28);
    v0 = lhu(a2 + 0x22);
    v1 = lw(a2);
    sw(a1, a2 + 0x20);
    v0 += a0;
    v1 &= 0x10;
    sw(v0, a2 + 0x24);
    if (v1 == 0) goto loc_80048E10;
    v1 = lw(a2 + 0x2C);
    v0 = lw(a2 + 0x28);
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80048E10;
    v0 = lbu(a2 + 0x3);
    v0 <<= 2;
    v0 += s2;
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    a0 = a2;
    pcall(v0);
    goto loc_80048E34;
loc_80048CC8:
    v0 = lw(a1);
    s0 = v0 & 9;
    v0 = 1;                                             // Result = 00000001
    if (s0 != v0) goto loc_80048E34;
    v0 = lw(a1 + 0x24);
    v1 = lw(a1 + 0x4);
    a0 = lw(a1 + 0x34);
    v0 -= v1;
    sw(v0, a1 + 0x24);
    a0 = lbu(a0);
    v0 = a0 - 7;
    v0 = (v0 < 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xD24);                                 // Store to: 8007F2DC
    {
        const bool bJump = (v0 == 0)
        v0 = a0 - 0x13;
        if (bJump) goto loc_80048D6C;
    }
    v0 = lbu(a1 + 0x3);
    v0 <<= 2;
    v0 += s2;
    v1 = lw(v0);
    v0 = a0 << 2;
    v0 += v1;
    v0 = lw(v0);
    a0 = a1;
    pcall(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD24);                                // Load from: 8007F2DC
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD30);                                // Load from: 8007F2D0
    v0 += s1;
    a0 = lbu(v0);
    v0 = lw(v1 + 0x34);
    a1 = v1 + 4;
    a0 += v0;
    sw(a0, v1 + 0x34);
    goto loc_80048DD8;
loc_80048D6C:
    v0 = (v0 < 0x11);
    {
        const bool bJump = (v0 == 0)
        v0 = a0 << 2;
        if (bJump) goto loc_80048DFC;
    }
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5A20;                                       // Result = gWess_DrvFunctions[0] (80075A20)
    at += v0;
    v0 = lw(at);
    a0 = a1;
    pcall(v0);
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0xD30);                                // Load from: 8007F2D0
    v1 = lw(a2);
    v0 = v1 & 0x41;
    {
        const bool bJump = (v0 != s0)
        v0 = -0x41;                                     // Result = FFFFFFBF
        if (bJump) goto loc_80048DF0;
    }
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD24);                                // Load from: 8007F2DC
    v0 += s1;
    a0 = lbu(v0);
    v0 = lw(a2 + 0x34);
    a1 = a2 + 4;
    a0 += v0;
    sw(a0, a2 + 0x34);
loc_80048DD8:
    Read_Vlq();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD30);                                // Load from: 8007F2D0
    sw(v0, v1 + 0x34);
    goto loc_80048E10;
loc_80048DF0:
    v0 &= v1;
    sw(v0, a2);
    goto loc_80048E10;
loc_80048DFC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A94);                               // Load from: gWess_SeqFunctions[A] (80075A94)
    a0 = a1;
    pcall(v0);
loc_80048E10:
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0xD30);                                // Load from: 8007F2D0
    v1 = lw(a1 + 0x4);
    v0 = lw(a1 + 0x24);
    v0 = (v0 < v1);
    if (v0 == 0) goto loc_80048CC8;
loc_80048E34:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xD2C);                               // Load from: gWess_SeqEngine_tmpNumTracks (8007F2D4)
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xD2C);                                 // Store to: gWess_SeqEngine_tmpNumTracks (8007F2D4)
    v0 &= 0xFF;
    if (v0 == 0) goto loc_80048E8C;
loc_80048E58:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xD30);                                // Load from: 8007F2D0
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xD28);                                // Load from: 8007F2D8
    v0 += 0x50;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xD30);                                 // Store to: 8007F2D0
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xD28);                                 // Store to: 8007F2D8
    if (v1 != v0) goto loc_80048C14;
loc_80048E8C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5AB8);                               // Load from: 80075AB8
    v0 = lbu(a0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x8);
    pcall(v0);
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_dig_lcd_loader_init() noexcept {
loc_80048EE4:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5AC4);                                 // Store to: 80075AC4
    a2 = 0;                                             // Result = 00000000
    if (a0 != 0) goto loc_80048F0C;
    v0 = 0;                                             // Result = 00000000
    goto loc_80048FC4;
loc_80048EFC:
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5AD8);                                // Store to: 80075AD8
    goto loc_80048F50;
loc_80048F0C:
    v0 = lw(a0 + 0xC);
    a1 = lbu(v0 + 0xA);
    v0 = (i32(a2) < i32(a1));
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80048F50;
    a3 = 1;                                             // Result = 00000001
    a0 = lw(a0 + 0x18);
loc_80048F30:
    v0 = lbu(a0 + 0x4);
    v1++;
    if (v0 == a3) goto loc_80048EFC;
    v0 = (i32(v1) < i32(a1));
    a0 += 0x54;
    if (v0 != 0) goto loc_80048F30;
loc_80048F50:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5AD8);                               // Load from: 80075AD8
    v0 = a2;                                            // Result = 00000000
    if (a1 == 0) goto loc_80048FC4;
    a0 = lw(a1 + 0x1C);
    v0 = lh(a1 + 0x8);
    v1 = lh(a1 + 0xC);
    a2 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(a2, at + 0x5AC4);                                // Store to: 80075AC4
    v0 <<= 2;
    v0 += a0;
    v1 <<= 4;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5AC8);                                // Store to: 80075AC8
    a0 = lh(a1 + 0x10);
    v1 += v0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5ACC);                                // Store to: 80075ACC
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5AD0);                                // Store to: 80075AD0
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 2;
    v0 += v1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AD4);                                // Store to: 80075AD4
    v0 = a2;                                            // Result = 00000001
loc_80048FC4:
    return;
}

void wess_dig_set_sample_position() noexcept {
loc_80048FCC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AD0);                               // Load from: 80075AD0
    v0 = a0 << 1;
    if (v1 == 0) goto loc_80048FF0;
    v0 += a0;
    v0 <<= 2;
    v0 += v1;
    sw(a1, v0 + 0x8);
loc_80048FF0:
    return;
}

void lcd_open() noexcept {
loc_80048FF8:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_open();
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0xD20;                                        // Result = 8007F2E0
    a2 = v0;
    t0 = a2 + 0x20;
loc_80049018:
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    a0 = lw(a2 + 0x8);
    a1 = lw(a2 + 0xC);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    sw(a0, a3 + 0x8);
    sw(a1, a3 + 0xC);
    a2 += 0x10;
    a3 += 0x10;
    if (a2 != t0) goto loc_80049018;
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0xD20;                                        // Result = 8007F2E0
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void lcd_upload_spu_samples() noexcept {
loc_8004906C:
    sp -= 0x30;
    sw(s6, sp + 0x28);
    s6 = a0;
    sw(s4, sp + 0x20);
    s4 = a1;
    sw(s3, sp + 0x1C);
    s3 = a2;
    sw(s5, sp + 0x24);
    s5 = a3;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(s2, sp + 0x18);
    s2 = 0;                                             // Result = 00000000
    sw(s0, sp + 0x10);
    s0 = 0x800;                                         // Result = 00000800
    sw(ra, sp + 0x2C);
loc_800490AC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AE4);                               // Load from: 80075AE4
    {
        const bool bJump = (v0 != 0)
        v0 = (i32(v0) < i32(s0));
        if (bJump) goto loc_80049234;
    }
loc_800490C0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AE0);                               // Load from: 80075AE0
    {
        const bool bJump = (i32(v0) <= 0)
        v0 <<= 1;
        if (bJump) goto loc_80049178;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AEC);                               // Load from: 80075AEC
    a1 = v0 + v1;
    v1 = lhu(a1);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5AD0);                               // Load from: 80075AD0
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v1 = v0 + a2;
    v0 = lw(v1 + 0x8);
    if (v0 == 0) goto loc_80049118;
    if (s5 == 0) goto loc_80049178;
loc_80049118:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5ADC);                               // Load from: 80075ADC
    sw(v0, v1 + 0x8);
    if (s3 == 0) goto loc_80049178;
    v0 = lhu(s3);
    v1 = lhu(a1);
    v0 <<= 1;
    v0 += s3;
    sh(v1, v0 + 0x2);
    a0 = lhu(s3);
    v1 = lhu(a1);
    a0 <<= 1;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 += a2;
    v0 = lw(v0 + 0x8);
    a0 += s3;
    v0 >>= 3;
    sh(v0, a0 + 0xCA);
    v0 = lhu(s3);
    v0++;
    sh(v0, s3);
loc_80049178:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lhu(v0 + 0x5AE8);                              // Load from: 80075AE8
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5AE0);                               // Load from: 80075AE0
    v0 = (i32(a1) < i32(v0));
    {
        const bool bJump = (v0 == 0)
        v0 = s2;
        if (bJump) goto loc_80049380;
    }
    a0 = a1 + 1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AEC);                               // Load from: 80075AEC
    v0 = a0 << 1;
    v0 += v1;
    v1 = lhu(v0);
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5AE0);                                // Store to: 80075AE0
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AD0);                               // Load from: 80075AD0
    v0 <<= 2;
    v0 += v1;
    a0 = lw(v0 + 0x4);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x598C);                               // Load from: 8007598C
    v1 = s4 + s1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5ADC);                                // Store to: 80075ADC
    v0 -= s4;
    v0 += s1;
    v0 = (v0 < a0);
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5AE4);                                // Store to: 80075AE4
    {
        const bool bJump = (v0 == 0)
        v0 = s2;
        if (bJump) goto loc_8004921C;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x5AE0);                                // Store to: 80075AE0
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5AE4);                                 // Store to: 80075AE4
    goto loc_80049380;
loc_8004921C:
    if (s0 == 0) goto loc_80049380;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AE4);                               // Load from: 80075AE4
    v0 = (i32(v0) < i32(s0));
loc_80049234:
    if (v0 != 0) goto loc_800492CC;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AE0);                               // Load from: 80075AE0
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AEC);                               // Load from: 80075AEC
    v0 <<= 1;
    v0 += v1;
    v1 = lhu(v0);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AD0);                               // Load from: 80075AD0
    v0 <<= 2;
    v0 += v1;
    v0 = lw(v0 + 0x8);
    if (v0 == 0) goto loc_8004928C;
    if (s5 == 0) goto loc_800492AC;
loc_8004928C:
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    a0 = s4 + s1;
    LIBSPU_SpuSetTransferStartAddr();
    a0 = s6 + s1;
    a1 = s0;
    LIBSPU_SpuWrite();
    s2 += s0;
loc_800492AC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AE4);                               // Load from: 80075AE4
    s1 += s0;
    v0 -= s0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AE4);                                // Store to: 80075AE4
    s0 = 0;                                             // Result = 00000000
    goto loc_80049364;
loc_800492CC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AE0);                               // Load from: 80075AE0
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AEC);                               // Load from: 80075AEC
    v0 <<= 1;
    v0 += v1;
    v1 = lhu(v0);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AD0);                               // Load from: 80075AD0
    v0 <<= 2;
    v0 += v1;
    v0 = lw(v0 + 0x8);
    if (v0 == 0) goto loc_8004931C;
    if (s5 == 0) goto loc_8004934C;
loc_8004931C:
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    a0 = s4 + s1;
    LIBSPU_SpuSetTransferStartAddr();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5AE4);                               // Load from: 80075AE4
    a0 = s6 + s1;
    LIBSPU_SpuWrite();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AE4);                               // Load from: 80075AE4
    s2 += v0;
loc_8004934C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AE4);                               // Load from: 80075AE4
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5AE4);                                 // Store to: 80075AE4
    s1 += v0;
    s0 -= v0;
loc_80049364:
    if (s0 != 0) goto loc_800490AC;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AE4);                               // Load from: 80075AE4
    {
        const bool bJump = (v0 == 0)
        v0 = s2;
        if (bJump) goto loc_800490C0;
    }
loc_80049380:
    ra = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x30;
    return;
}

void lcd_close() noexcept {
loc_800493AC:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    s0 += 0x1F40;
    v0 = (v0 < s0);
    sw(s1, sp + 0x14);
    if (v0 == 0) goto loc_80049434;
    s2 = 5;                                             // Result = 00000005
    s1 = 2;                                             // Result = 00000002
loc_800493E4:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x5AF8;                                       // Result = 80075AF8
    a0 = 1;                                             // Result = 00000001
    LIBCD_CdSync();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AF4);                                // Store to: 80075AF4
    if (v0 != s2) goto loc_80049414;
    LIBCD_CdFlush();
    v0 = 1;                                             // Result = 00000001
    goto loc_80049438;
loc_80049414:
    {
        const bool bJump = (v0 == s1)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80049438;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    v0 = (v0 < s0);
    if (v0 != 0) goto loc_800493E4;
loc_80049434:
    v0 = 1;                                             // Result = 00000001
loc_80049438:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_dig_lcd_load() noexcept {
loc_80049454:
    sp -= 0x40;
    sw(fp, sp + 0x38);
    fp = a0;
    sw(s6, sp + 0x30);
    s6 = a2;
    sw(s7, sp + 0x34);
    s7 = a3;
    sw(ra, sp + 0x3C);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    sw(a1, sp + 0x10);
    psxcd_disable_callbacks();
loc_80049494:
    s5 = 1;                                             // Result = 00000001
    s4 = 5;                                             // Result = 00000005
loc_8004949C:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5AE0);                                 // Store to: 80075AE0
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5AE4);                                 // Store to: 80075AE4
    s3 = 0;                                             // Result = 00000000
    psxcd_init_pos();
    psxcd_set_data_mode();
    a0 = fp;
    lcd_open();
    s0 = v0;
    v0 = lw(s0);
    s2 = lw(sp + 0x10);
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800497A4;
    }
    s1 = lw(s0 + 0x4);
    a0 = s0;
    LIBCD_CdPosToInt();
    a0 = v0;
    s0 += 0x18;
    a1 = s0;
    LIBCD_CdIntToPos();
    a0 = 2;                                             // Result = 00000002
    a1 = s0;
    a2 = 0;                                             // Result = 00000000
    LIBCD_CdControl();
    a0 = 6;                                             // Result = 00000006
    a1 = s0;
    a2 = 0;                                             // Result = 00000000
    LIBCD_CdControl();
    a0 = 1;                                             // Result = 00000001
loc_80049518:
    a1 = 0;                                             // Result = 00000000
    LIBCD_CdReady();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AF0);                                // Store to: 80075AF0
    if (v0 == s5) goto loc_80049540;
    a0 = 1;                                             // Result = 00000001
    if (v0 != s4) goto loc_80049518;
    LIBCD_CdFlush();
loc_80049540:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AF0);                               // Load from: 80075AF0
    if (v0 == s4) goto loc_8004949C;
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6AE8;                                       // Result = gPSXCD_sectorbuf[0] (800A9518)
    a0 = s0;                                            // Result = gPSXCD_sectorbuf[0] (800A9518)
    a1 = 0x200;                                         // Result = 00000200
    LIBCD_CdGetSector();
    v0 = lhu(s0);                                       // Load from: gPSXCD_sectorbuf[0] (800A9518)
    at = 0x80070000;                                    // Result = 80070000
    sh(v0, at + 0x5AE8);                                // Store to: 80075AE8
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lhu(v0 + 0x5AE8);                              // Load from: 80075AE8
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5AEC);                                // Store to: 80075AEC
    v0 = (v0 < 0x65);
    {
        const bool bJump = (v0 == 0)
        v0 = (i32(s1) < 0x800);
        if (bJump) goto loc_8004949C;
    }
    s1 -= 0x800;
    if (v0 == 0) goto loc_800495A0;
    s1 = 0;                                             // Result = 00000000
loc_800495A0:
    s0 = 1;                                             // Result = 00000001
    v0 = 1;                                             // Result = 00000001
    if (s1 == 0) goto loc_80049760;
loc_800495AC:
    a0 = 1;                                             // Result = 00000001
    if (s0 == 0) goto loc_8004963C;
loc_800495B4:
    a1 = 0;                                             // Result = 00000000
    LIBCD_CdReady();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AF0);                                // Store to: 80075AF0
    if (v0 == s5) goto loc_800495DC;
    a0 = 1;                                             // Result = 00000001
    if (v0 != s4) goto loc_800495B4;
    LIBCD_CdFlush();
loc_800495DC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AF0);                               // Load from: 80075AF0
    if (v0 == s4) goto loc_80049494;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = 0x200;                                         // Result = 00000200
    LIBCD_CdGetSector();
    v0 = (i32(s1) < 0x800);
    s1 -= 0x800;
    if (v0 == 0) goto loc_80049610;
    s1 = 0;                                             // Result = 00000000
loc_80049610:
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = s2;
    a2 = s6;
    a3 = s7;
    lcd_upload_spu_samples();
    s3 += v0;
    s2 += v0;
    v0 = 1;                                             // Result = 00000001
    s0 = 0;                                             // Result = 00000000
    goto loc_80049758;
loc_8004963C:
    if (v0 == 0) goto loc_800496D0;
loc_80049644:
    a1 = 0;                                             // Result = 00000000
    LIBCD_CdReady();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AF0);                                // Store to: 80075AF0
    if (v0 == s5) goto loc_8004966C;
    a0 = 1;                                             // Result = 00000001
    if (v0 != s4) goto loc_80049644;
    LIBCD_CdFlush();
loc_8004966C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AF0);                               // Load from: 80075AF0
    if (v0 == s4) goto loc_80049494;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x6D7C;                                       // Result = gWess_data_read_chunk2[0] (80096D7C)
    a1 = 0x200;                                         // Result = 00000200
    LIBCD_CdGetSector();
    v0 = (i32(s1) < 0x800);
    s1 -= 0x800;
    if (v0 == 0) goto loc_800496A0;
    s1 = 0;                                             // Result = 00000000
loc_800496A0:
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x6D7C;                                       // Result = gWess_data_read_chunk2[0] (80096D7C)
    a1 = s2;
    a2 = s6;
    a3 = s7;
    lcd_upload_spu_samples();
    s3 += v0;
    s2 += v0;
    v0 = 0;                                             // Result = 00000000
    goto loc_80049758;
loc_800496D0:
    a1 = 0;                                             // Result = 00000000
    LIBCD_CdReady();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AF0);                                // Store to: 80075AF0
    if (v0 == s5) goto loc_800496F8;
    a0 = 1;                                             // Result = 00000001
    if (v0 != s4) goto loc_800496D0;
    LIBCD_CdFlush();
loc_800496F8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AF0);                               // Load from: 80075AF0
    if (v0 == s4) goto loc_80049494;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = 0x200;                                         // Result = 00000200
    LIBCD_CdGetSector();
    v0 = (i32(s1) < 0x800);
    s1 -= 0x800;
    if (v0 == 0) goto loc_8004972C;
    s1 = 0;                                             // Result = 00000000
loc_8004972C:
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = s2;
    a2 = s6;
    a3 = s7;
    lcd_upload_spu_samples();
    s3 += v0;
    s2 += v0;
    v0 = 1;                                             // Result = 00000001
loc_80049758:
    if (s1 != 0) goto loc_800495AC;
loc_80049760:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AF0);                               // Load from: 80075AF0
    a0 = 9;                                             // Result = 00000009
    if (v0 == s4) goto loc_8004949C;
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    LIBCD_CdControl();
    lcd_close();
    if (v0 != 0) goto loc_8004949C;
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    psxcd_enable_callbacks();
    v0 = s3;
loc_800497A4:
    ra = lw(sp + 0x3C);
    fp = lw(sp + 0x38);
    s7 = lw(sp + 0x34);
    s6 = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x40;
    return;
}

void wess_master_sfx_volume_get() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    Is_Module_Loaded();
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800497F8;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A04);                              // Load from: gWess_master_sfx_volume (80075A04)
loc_800497F8:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_master_mus_volume_get() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    Is_Module_Loaded();
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80049828;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A05);                              // Load from: gWess_master_mus_volume (80075A05)
loc_80049828:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_master_sfx_vol_set() noexcept {
loc_80049838:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    Is_Module_Loaded();
    if (v0 == 0) goto loc_8004985C;
    at = 0x80070000;                                    // Result = 80070000
    sb(s0, at + 0x5A04);                                // Store to: gWess_master_sfx_volume (80075A04)
loc_8004985C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_master_mus_vol_set() noexcept {
loc_80049870:
    sp -= 0x48;
    sw(s0, sp + 0x20);
    s0 = a0;
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s6, sp + 0x38);
    sw(s5, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    Is_Module_Loaded();
    if (v0 == 0) goto loc_800499FC;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sb(s0, at + 0x5A05);                                // Store to: gWess_master_mus_volume (80075A05)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    fp = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s7 = lw(v0 + 0x20);
    s6 = lbu(v1 + 0xB);
    v0 = s6;
    if (fp == 0) goto loc_800499F0;
    v0 &= 0xFF;
    s6--;
    if (v0 == 0) goto loc_800499F0;
    a1 = -1;                                            // Result = FFFFFFFF
    s5 = s7 + 0xC;
loc_800498F0:
    v0 = lw(s7);
    v0 &= 1;
    if (v0 == 0) goto loc_800499D8;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s4 = lbu(s5 - 0x8);
    s2 = lbu(v0 + 0x1C);
    s3 = lw(s5);
    s2--;
    if (s2 == a1) goto loc_800499C8;
loc_80049924:
    a0 = lbu(s3);
    v0 = 0xFF;                                          // Result = 000000FF
    {
        const bool bJump = (a0 == v0)
        v0 = a0 << 2;
        if (bJump) goto loc_800499BC;
    }
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += a0;
    v1 = lw(v1 + 0x28);
    v0 <<= 4;
    s1 = v0 + v1;
    v1 = lbu(s1 + 0x13);
    v0 = 1;                                             // Result = 00000001
    s4--;
    if (v1 != v0) goto loc_800499B4;
    s0 = lw(s1 + 0x34);
    v0 = sp + 0x10;
    sw(v0, s1 + 0x34);
    v0 = 0xC;                                           // Result = 0000000C
    sb(v0, sp + 0x10);
    v1 = lw(s1 + 0x34);
    v0 = lbu(s1 + 0xC);
    sb(v0, v1 + 0x1);
    v0 = lbu(s1 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    v0 <<= 2;
    v0 += a2;
    v0 = lw(v0);
    v0 = lw(v0 + 0x30);
    a0 = s1;
    sw(a1, sp + 0x18);
    pcall(v0);
    sw(s0, s1 + 0x34);
    a1 = lw(sp + 0x18);
loc_800499B4:
    if (s4 == 0) goto loc_800499C8;
loc_800499BC:
    s2--;
    s3++;
    if (s2 != a1) goto loc_80049924;
loc_800499C8:
    fp--;
    v0 = fp & 0xFF;
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800499F4;
    }
loc_800499D8:
    s5 += 0x18;
    s7 += 0x18;
    v0 = s6;
    v0 &= 0xFF;
    s6--;
    if (v0 != 0) goto loc_800498F0;
loc_800499F0:
    v0 = 1;                                             // Result = 00000001
loc_800499F4:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_800499FC:
    ra = lw(sp + 0x44);
    fp = lw(sp + 0x40);
    s7 = lw(sp + 0x3C);
    s6 = lw(sp + 0x38);
    s5 = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x48;
    return;
}

void wess_pan_mode_get() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    return;
}

void wess_pan_mode_set() noexcept {
    at = 0x80070000;                                    // Result = 80070000
    sb(a0, at + 0x5A06);                                // Store to: gWess_pan_status (80075A06)
    return;
}

void wess_seq_range_sizeof() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(s2, sp + 0x18);
    s2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s1, sp + 0x14);
    if (v0 == 0) goto loc_80049AB8;
    s1 = a0;
    if (s0 != 0) goto loc_80049A8C;
    v0 = 0;                                             // Result = 00000000
    goto loc_80049ABC;
loc_80049A8C:
    s0--;
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s0 == v0)
        v0 = s2;                                        // Result = 00000000
        if (bJump) goto loc_80049ABC;
    }
    s3 = -1;                                            // Result = FFFFFFFF
loc_80049AA0:
    a0 = s1;
    wess_seq_sizeof();
    s2 += v0;
    s0--;
    s1++;
    if (s0 != s3) goto loc_80049AA0;
loc_80049AB8:
    v0 = s2;
loc_80049ABC:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void wess_seq_range_load() noexcept {
loc_80049ADC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x28;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(s4, sp + 0x20);
    s4 = a2;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x24);
    sw(s3, sp + 0x1C);
    if (v0 == 0) goto loc_80049B68;
    open_sequence_data();
    if (v0 == 0) goto loc_80049B2C;
    {
        const bool bJump = (s0 != 0)
        s0--;
        if (bJump) goto loc_80049B34;
    }
loc_80049B2C:
    v0 = 0;                                             // Result = 00000000
    goto loc_80049B6C;
loc_80049B34:
    v0 = -1;                                            // Result = FFFFFFFF
    if (s0 == v0) goto loc_80049B60;
    s3 = -1;                                            // Result = FFFFFFFF
loc_80049B44:
    a0 = s2;
    a1 = s4 + s1;
    wess_seq_load();
    s1 += v0;
    s0--;
    s2++;
    if (s0 != s3) goto loc_80049B44;
loc_80049B60:
    close_sequence_data();
loc_80049B68:
    v0 = s1;
loc_80049B6C:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void wess_seq_range_free() noexcept {
loc_80049B90:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(s3, sp + 0x1C);
    s3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x20);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    if (v0 == 0) goto loc_80049BF8;
    s1 = a0;
    if (s0 != 0) goto loc_80049BCC;
    v0 = 0;                                             // Result = 00000000
    goto loc_80049BFC;
loc_80049BCC:
    s0--;
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s0 == v0)
        v0 = s3;                                        // Result = 00000000
        if (bJump) goto loc_80049BFC;
    }
    s3 = 1;                                             // Result = 00000001
    s2 = -1;                                            // Result = FFFFFFFF
loc_80049BE4:
    a0 = s1;
    wess_seq_free();
    s0--;
    s1++;
    if (s0 != s2) goto loc_80049BE4;
loc_80049BF8:
    v0 = s3;
loc_80049BFC:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}
