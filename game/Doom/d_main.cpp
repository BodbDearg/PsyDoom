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

void D_mystrlen() noexcept {
    v1 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_800310BC;
    v0 = lbu(a0);
    a0++;
    if (v0 == 0) goto loc_800310C0;
loc_800310A0:
    v1++;
    v0 = lbu(a0);
    a0++;
    if (v0 != 0) goto loc_800310A0;
    goto loc_800310C0;
loc_800310BC:
    v1 = -1;                                            // Result = FFFFFFFF
loc_800310C0:
    v0 = v1;
    return;
}

void D_vsprintf() noexcept {
loc_800310C8:
    v0 = lbu(a1);
    t6 = a0;
    if (v0 == 0) goto loc_80031384;
    t5 = 0x75;                                          // Result = 00000075
loc_800310DC:
    v1 = lbu(a1);
    v0 = 0x25;                                          // Result = 00000025
    {
        const bool bJump = (v1 == v0)
        v0 = 0x30;                                      // Result = 00000030
        if (bJump) goto loc_800310F8;
    }
    v0 = lbu(a1);
    a1++;
    goto loc_80031180;
loc_800310F8:
    a1++;
    v1 = lbu(a1);
    t4 = 0x20;                                          // Result = 00000020
    if (v1 != v0) goto loc_80031114;
    t4 = 0x30;                                          // Result = 00000030
    a1++;
loc_80031114:
    v0 = lbu(a1);
    t2 = 0;                                             // Result = 00000000
    goto loc_8003113C;
loc_80031120:
    v0 = lbu(a1);
    a1++;
    v1 += t2;
    v1 <<= 1;
    v1 += v0;
    v0 = lbu(a1);
    t2 = v1 - 0x30;
loc_8003113C:
    v0 -= 0x30;
    v0 = (v0 < 0xA);
    v1 = t2 << 2;
    if (v0 != 0) goto loc_80031120;
    v1 = lbu(a1);
    v0 = 0x6C;                                          // Result = 0000006C
    a3 = 0;                                             // Result = 00000000
    if (v1 != v0) goto loc_80031168;
    a3 = 1;                                             // Result = 00000001
    a1++;
    v1 = lbu(a1);
loc_80031168:
    v0 = 0x63;                                          // Result = 00000063
    {
        const bool bJump = (v1 != v0)
        v0 = 0x73;                                      // Result = 00000073
        if (bJump) goto loc_8003118C;
    }
    v0 = lbu(a2);
    a2 += 4;
    a1++;
loc_80031180:
    sb(v0, a0);
    a0++;
    goto loc_80031374;
loc_8003118C:
    if (v1 != v0) goto loc_80031228;
    t1 = lw(a2);
    a2 += 4;
    a3 = 0;                                             // Result = 00000000
    if (t1 == 0) goto loc_800311D0;
    v0 = lbu(t1);
    v1 = t1 + 1;
    if (v0 == 0) goto loc_800311D4;
loc_800311B4:
    a3++;
    v0 = lbu(v1);
    v1++;
    if (v0 != 0) goto loc_800311B4;
    t0 = a3;
    goto loc_800311E4;
loc_800311D0:
    a3 = -1;                                            // Result = FFFFFFFF
loc_800311D4:
    t0 = a3;
    goto loc_800311E4;
loc_800311DC:
    sb(t4, a0);
    a0++;
loc_800311E4:
    v0 = t2;
    v0 = (i32(t0) < i32(v0));
    t2--;
    if (v0 != 0) goto loc_800311DC;
    v0 = lbu(t1);
    if (v0 == 0) goto loc_80031370;
loc_80031204:
    v0 = lbu(t1);
    t1++;
    sb(v0, a0);
    v0 = lbu(t1);
    a0++;
    if (v0 != 0) goto loc_80031204;
    a1++;
    goto loc_80031374;
loc_80031228:
    v0 = 0x6F;                                          // Result = 0000006F
    {
        const bool bJump = (v1 != v0)
        v0 = 0x78;                                      // Result = 00000078
        if (bJump) goto loc_8003123C;
    }
    t3 = 8;                                             // Result = 00000008
    goto loc_80031250;
loc_8003123C:
    t3 = 0x10;                                          // Result = 00000010
    if (v1 == v0) goto loc_80031250;
    v0 = 0x58;                                          // Result = 00000058
    {
        const bool bJump = (v1 != v0)
        v0 = 0x69;                                      // Result = 00000069
        if (bJump) goto loc_8003125C;
    }
loc_80031250:
    a3 = lw(a2);
    a2 += 4;
    goto loc_800312B4;
loc_8003125C:
    t3 = 0xA;                                           // Result = 0000000A
    if (v1 == v0) goto loc_80031278;
    v0 = 0x64;                                          // Result = 00000064
    if (v1 == v0) goto loc_80031278;
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 != t5) goto loc_8003138C;
loc_80031278:
    v1 = lw(a2);
    a2 += 4;
    if (i32(v1) >= 0) goto loc_800312B0;
    v0 = lbu(a1);
    {
        const bool bJump = (v0 == t5)
        v0 = 0x2D;                                      // Result = 0000002D
        if (bJump) goto loc_800312B0;
    }
    sb(v0, a0);
    a0++;
    a3 = -v1;
    if (t2 == 0) goto loc_800312B4;
    t2--;
    goto loc_800312B4;
loc_800312B0:
    a3 = v1;
loc_800312B4:
    t0 = 0;                                             // Result = 00000000
loc_800312B8:
    v1 = t0 + a0;
    if (t0 == 0) goto loc_800312FC;
    t1 = a0;
loc_800312C4:
    v0 = lbu(v1 - 0x1);
    sb(v0, v1);
    v1--;
    if (v1 != t1) goto loc_800312C4;
    if (t0 == 0) goto loc_800312FC;
    if (t2 == 0) goto loc_800312FC;
    if (a3 != 0) goto loc_800312FC;
    sb(t4, a0);
    goto loc_80031348;
loc_800312FC:
    divu(a3, t3);
    if (t3 != 0) goto loc_8003130C;
    _break(0x1C00);
loc_8003130C:
    v0 = hi;
    sb(v0, a0);
    v1 = lbu(a0);
    v0 = (v1 < 0xA);
    {
        const bool bJump = (v0 != 0)
        v0 = v1 + 0x30;
        if (bJump) goto loc_80031330;
    }
    v0 = v1 + 0x37;
loc_80031330:
    sb(v0, a0);
    divu(a3, t3);
    if (t3 != 0) goto loc_80031344;
    _break(0x1C00);
loc_80031344:
    a3 = lo;
loc_80031348:
    t0++;
    if (t2 == 0) goto loc_80031354;
    t2--;
loc_80031354:
    if (a3 != 0) goto loc_800312B8;
    if (t2 != 0) goto loc_800312B8;
    if (t0 == 0) goto loc_800312B8;
    a0 += t0;
loc_80031370:
    a1++;
loc_80031374:
    v0 = lbu(a1);
    if (v0 != 0) goto loc_800310DC;
loc_80031384:
    sb(0, a0);
    v0 = t6 - a0;
loc_8003138C:
    return;
}

void START_Legals() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    I_ResetTexCache();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7BD0;                                       // Result = gTexInfo_LEGALS[0] (80097BD0)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C44;                                       // Result = STR_LumpName_LEGALS[0] (80077C44)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = 0;                                             // Result = 00000000
    a1 = 1;                                             // Result = 00000001
    S_StartSound();
    v0 = 0xF0;                                          // Result = 000000F0
    sw(v0, gp + 0xBB0);                                 // Store to: gTitleScreenSpriteY (80078190)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void STOP_Legals() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0;                                             // Result = 00000000
    a1 = 5;                                             // Result = 00000005
    S_StartSound();
    I_CrossFadeFrameBuffers();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void TIC_Legals() noexcept {
    v0 = lw(gp + 0xBB0);                                // Load from: gTitleScreenSpriteY (80078190)
    {
        const bool bJump = (i32(v0) <= 0)
        v0--;
        if (bJump) goto loc_80035000;
    }
    sw(v0, gp + 0xBB0);                                 // Store to: gTitleScreenSpriteY (80078190)
    {
        const bool bJump = (v0 != 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80035044;
    }
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    sw(v0, gp + 0x92C);                                 // Store to: gMenuTimeoutStartTicCon (80077F0C)
    v0 = 0;                                             // Result = 00000000
    goto loc_80035044;
loc_80035000:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    v1 = lw(gp + 0x92C);                                // Load from: gMenuTimeoutStartTicCon (80077F0C)
    v1 = v0 - v1;
    v0 = (i32(v1) < 0x79);
    {
        const bool bJump = (v0 != 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80035044;
    }
    v0 = (i32(v1) < 0xB4);
    {
        const bool bJump = (v0 == 0)
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_80035044;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F44);                               // Load from: gPlayerPadButtons[0] (80077F44)
    v0 = (v0 > 0);
    v0 = -v0;
    v0 &= 9;
loc_80035044:
    return;
}

void DRAW_Legals() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    I_IncDrawnFrameCount();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7BD0;                                       // Result = gTexInfo_LEGALS[0] (80097BD0)
    a2 = lw(gp + 0xBB0);                                // Load from: gTitleScreenSpriteY (80078190)
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    a1 = 0;                                             // Result = 00000000
    I_CacheAndDrawSprite();
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void START_Title() noexcept {
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    I_ResetTexCache();
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x7C4C;                                       // Result = STR_LumpName_LOADING[0] (80077C4C)
    a0 = s1;                                            // Result = STR_LumpName_LOADING[0] (80077C4C)
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    W_CacheLumpName();
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7A90;                                       // Result = gTexInfo_LOADING[0] (80097A90)
    a0 = s0;                                            // Result = gTexInfo_LOADING[0] (80097A90)
    a1 = s1;                                            // Result = STR_LumpName_LOADING[0] (80077C4C)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = s0;                                            // Result = gTexInfo_LOADING[0] (80097A90)
    a1 = 0x5F;                                          // Result = 0000005F
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    a2 = 0x6D;                                          // Result = 0000006D
    I_DrawPlaque();
    a0 = 0;                                             // Result = 00000000
    S_LoadSoundAndMusic();
    s4 = 0x80070000;                                    // Result = 80070000
    s4 += 0x7C54;                                       // Result = STR_LumpName_MARB01[0] (80077C54)
    a0 = s4;                                            // Result = STR_LumpName_MARB01[0] (80077C54)
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    W_CacheLumpName();
    s3 = 0x80070000;                                    // Result = 80070000
    s3 += 0x7C5C;                                       // Result = STR_LumpName_BUTTONS[0] (80077C5C)
    a0 = s3;                                            // Result = STR_LumpName_BUTTONS[0] (80077C5C)
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    W_CacheLumpName();
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x7C64;                                       // Result = STR_LumpName_NETERR[0] (80077C64)
    a0 = s2;                                            // Result = STR_LumpName_NETERR[0] (80077C64)
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    W_CacheLumpName();
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x7C6C;                                       // Result = STR_LumpName_PAUSE[0] (80077C6C)
    a0 = s1;                                            // Result = STR_LumpName_PAUSE[0] (80077C6C)
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    W_CacheLumpName();
    a0 = s0 + 0x20;                                     // Result = gTexInfo_MARB01[0] (80097AB0)
    a1 = s4;                                            // Result = STR_LumpName_MARB01[0] (80077C54)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = s0 + 0x40;                                     // Result = gTexInfo_BUTTONS[0] (80097AD0)
    a1 = s3;                                            // Result = STR_LumpName_BUTTONS[0] (80077C5C)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = s0 + 0x60;                                     // Result = gTexInfo_NETERR[0] (80097AF0)
    a1 = s2;                                            // Result = STR_LumpName_NETERR[0] (80077C64)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = s0 - 0x20;                                     // Result = gTexInfo_PAUSE[0] (80097A70)
    a1 = s1;                                            // Result = STR_LumpName_PAUSE[0] (80077C6C)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = s0 - 0x60;                                     // Result = gTexInfo_TITLE[0] (80097A30)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C74;                                       // Result = STR_LumpName_TITLE[0] (80077C74)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7C7C;                                       // Result = STR_LumpName_SKY09[0] (80077C7C)
    R_TextureNumForName();
    a1 = 0x20;                                          // Result = 00000020
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7ED8);                               // Load from: gpTextures (80078128)
    v0 <<= 5;
    v0 += v1;
    a0 = lh(v0 + 0x10);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7FB0);                                // Store to: gpSkyTexture (80078050)
    a2 = 1;                                             // Result = 00000001
    W_CacheLumpNum();
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lhu(v0 - 0x6F5E);                              // Load from: gPaletteClutId_Sky (800A90A2)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0x7D34);                                // Store to: gPaletteClutId_CurMapSky (800782CC)
    I_CacheTex();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x3E4C);                               // Load from: CDTrackNum_TitleScreen (80073E4C)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75F8);                               // Load from: gCdMusicVol (800775F8)
    v0 = 0xFA;                                          // Result = 000000FA
    sw(v0, gp + 0xBB0);                                 // Store to: gTitleScreenSpriteY (80078190)
    psxcd_play();
loc_80035234:
    psxcd_elapsed_sectors();
    if (v0 == 0) goto loc_80035234;
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void STOP_Title() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0;                                             // Result = 00000000
    a1 = 5;                                             // Result = 00000005
    S_StartSound();
    psxcd_stop();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void TIC_Title() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F44);                               // Load from: gPlayerPadButtons[0] (80077F44)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_800352B0;
    v0 = 9;                                             // Result = 00000009
    goto loc_800353FC;
loc_800352B0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    v1 = lw(gp + 0x6A4);                                // Load from: gVBlanksUntilTitleSprMove (80077C84)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    v1 -= v0;
    sw(v1, gp + 0x6A4);                                 // Store to: gVBlanksUntilTitleSprMove (80077C84)
    v0 = 2;                                             // Result = 00000002
    if (i32(v1) > 0) goto loc_80035310;
    v1 = lw(gp + 0xBB0);                                // Load from: gTitleScreenSpriteY (80078190)
    sw(v0, gp + 0x6A4);                                 // Store to: gVBlanksUntilTitleSprMove (80077C84)
    v0 = v1 - 1;
    if (v1 == 0) goto loc_80035310;
    sw(v0, gp + 0xBB0);                                 // Store to: gTitleScreenSpriteY (80078190)
    if (v0 != 0) goto loc_80035310;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    sw(v0, gp + 0x92C);                                 // Store to: gMenuTimeoutStartTicCon (80077F0C)
loc_80035310:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    v1 = lw(gp + 0x6A8);                                // Load from: gVBlanksUntilTitleFireMove (80077C88)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    v1 -= v0;
    sw(v1, gp + 0x6A8);                                 // Store to: gVBlanksUntilTitleFireMove (80077C88)
    {
        const bool bJump = (i32(v1) > 0)
        v1 = 2;                                         // Result = 00000002
        if (bJump) goto loc_800353C8;
    }
    v0 = lw(gp + 0xBB0);                                // Load from: gTitleScreenSpriteY (80078190)
    sw(v1, gp + 0x6A8);                                 // Store to: gVBlanksUntilTitleFireMove (80077C88)
    v1 = (i32(v0) < 0x32);
    v0 ^= 1;
    v0 &= 1;
    v1 &= v0;
    if (v1 == 0) goto loc_800353B8;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    v0 = lh(v0 + 0x10);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7DC4);                               // Load from: gpLumpCache (8007823C)
    v0 <<= 2;
    v0 += v1;
    a1 = lw(v0);
    v0 = lbu(a1 + 0x1FC8);
    a0 = v0 - 1;
    v1 = 0x3F;                                          // Result = 0000003F
    if (i32(a0) >= 0) goto loc_800353A4;
    a0 = 0;                                             // Result = 00000000
loc_800353A4:
    v0 = a1 + 0x2007;
loc_800353A8:
    sb(a0, v0);
    v1--;
    v0--;
    if (i32(v1) >= 0) goto loc_800353A8;
loc_800353B8:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    P_UpdateFireSky();
loc_800353C8:
    v1 = lw(gp + 0xBB0);                                // Load from: gTitleScreenSpriteY (80078190)
    v0 = 0;                                             // Result = 00000000
    if (v1 != 0) goto loc_800353FC;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    v1 = lw(gp + 0x92C);                                // Load from: gMenuTimeoutStartTicCon (80077F0C)
    v0 -= v1;
    v0 = (i32(v0) < 0x708);
    v0 ^= 1;
    v0 = -v0;
    v0 &= 7;
loc_800353FC:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void DRAW_Title() noexcept {
    sp -= 0x48;
    sw(ra, sp + 0x44);
    sw(s0, sp + 0x40);
    I_IncDrawnFrameCount();
    t0 = 9;                                             // Result = 00000009
    t1 = sp + 0x14;
    t3 = 0x24;                                          // Result = 00000024
    t4 = 0x28;                                          // Result = 00000028
    t2 = 0xFF0000;                                      // Result = 00FF0000
    t2 |= 0xFFFF;                                       // Result = 00FFFFFF
    t8 = 0x80080000;                                    // Result = 80080000
    t8 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s0 = t8 & t2;                                       // Result = 00086550
    t7 = 0x4000000;                                     // Result = 04000000
    t6 = 0x80000000;                                    // Result = 80000000
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 = 0xFF;                                          // Result = 000000FF
    sb(v1, sp + 0x24);
    sb(v1, sp + 0x34);
    v1 = lhu(gp + 0xBB0);                               // Load from: gTitleScreenSpriteY (80078190)
    v0 = 9;                                             // Result = 00000009
    sb(v0, sp + 0x13);
    v0 = 0x2C;                                          // Result = 0000002C
    sb(v0, sp + 0x17);
    v0 = 0xFF;                                          // Result = 000000FF
    sh(v0, sp + 0x20);
    sh(v0, sp + 0x30);
    v0 = 0xEF;                                          // Result = 000000EF
    sb(v0, sp + 0x2D);
    sb(v0, sp + 0x35);
    v0 = 0x80;                                          // Result = 00000080
    sb(v0, sp + 0x14);
    sb(v0, sp + 0x15);
    sb(v0, sp + 0x16);
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lhu(v0 + 0x7A3A);                              // Load from: gTexInfo_TITLE[2] (80097A3A)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lhu(a0 - 0x6F5A);                              // Load from: gPaletteClutId_Title (800A90A6)
    t5 = -1;                                            // Result = FFFFFFFF
    sh(0, sp + 0x18);
    sh(0, sp + 0x28);
    sb(0, sp + 0x1C);
    sb(0, sp + 0x1D);
    sb(0, sp + 0x25);
    sb(0, sp + 0x2C);
    sh(v1, sp + 0x1A);
    sh(v1, sp + 0x22);
    v1 += 0xEF;
    sh(v1, sp + 0x2A);
    sh(v1, sp + 0x32);
    sh(v0, sp + 0x26);
    sh(a0, sp + 0x1E);
loc_800354E0:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0)
        v0 = t3 + a0;
        if (bJump) goto loc_80035548;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0)
        v0 = t4 + a0;
        if (bJump) goto loc_8003560C;
    }
    v0 = lw(a3);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(t8, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s0;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80035548:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t3 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_800355FC;
    if (v1 == a0) goto loc_800354E0;
loc_8003556C:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t7;
    if (v0 == 0) goto loc_800354E0;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t2;
    v0 |= t6;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800355D8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800355BC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800355BC;
loc_800355D8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800354E0;
    goto loc_8003556C;
loc_800355FC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t4;
loc_8003560C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a3);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= a1;
    v1 |= v0;
    sw(v1, a3);
    sb(t0, a3 + 0x3);
    t0--;                                               // Result = 00000008
    v0 = -1;                                            // Result = FFFFFFFF
    a3 += 4;
    if (t0 == v0) goto loc_8003566C;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80035654:
    v0 = lw(t1);
    t1 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_80035654;
loc_8003566C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80035724;
    t2 = 0x4000000;                                     // Result = 04000000
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003569C:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_80035724;
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
    if (a1 == t0) goto loc_80035708;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800356EC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800356EC;
loc_80035708:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003569C;
loc_80035724:
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    v1 = lw(a1 + 0x1C);
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != v0)
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_800357C4;
    }
    v1 = lbu(a1 + 0x8);
    v0 = lhu(a1 + 0xA);
    v1 >>= 1;
    v0 &= 0xF;
    v0 <<= 6;
    v1 += v0;
    sh(v1, sp + 0x38);
    v1 = lhu(a1 + 0xA);
    a0 = lbu(a1 + 0x9);
    v0 = 0x20;                                          // Result = 00000020
    sh(v0, sp + 0x3C);
    v0 = 0x80;                                          // Result = 00000080
    sh(v0, sp + 0x3E);
    v1 &= 0x10;
    v1 <<= 4;
    a0 += v1;
    sh(a0, sp + 0x3A);
    v0 = lh(a1 + 0x10);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7DC4);                               // Load from: gpLumpCache (8007823C)
    v0 <<= 2;
    v0 += v1;
    a1 = lw(v0);
    a0 = sp + 0x38;
    a1 += 8;
    LIBGPU_LoadImage();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C10);                               // Load from: gNumFramesDrawn (80077C10)
    sw(v0, v1 + 0x1C);
    v0 = 9;                                             // Result = 00000009
loc_800357C4:
    sb(v0, sp + 0x13);
    v0 = 0x2C;                                          // Result = 0000002C
    sb(v0, sp + 0x17);
    v0 = 0x74;                                          // Result = 00000074
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    v1 = 0x3F;                                          // Result = 0000003F
    sh(v0, sp + 0x1A);
    sh(v0, sp + 0x22);
    v0 = 0xF3;                                          // Result = 000000F3
    sh(0, sp + 0x18);
    sh(v1, sp + 0x20);
    sh(0, sp + 0x28);
    sh(v0, sp + 0x2A);
    sh(v1, sp + 0x30);
    sh(v0, sp + 0x32);
    v0 = lbu(a0 + 0x8);
    t6 = 0;                                             // Result = 00000000
    sb(v0, sp + 0x1C);
    v0 = lbu(a0 + 0x9);
    t4 = 0xFF0000;                                      // Result = 00FF0000
    sb(v0, sp + 0x1D);
    v0 = lbu(a0 + 0x8);
    t4 |= 0xFFFF;                                       // Result = 00FFFFFF
    v0 += 0x3F;
    sb(v0, sp + 0x24);
    v0 = lbu(a0 + 0x9);
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    sb(v0, sp + 0x25);
    v0 = lbu(a0 + 0x8);
    t9 = s0 & t4;                                       // Result = 00086550
    sb(v0, sp + 0x2C);
    v0 = lbu(a0 + 0x9);
    t8 = 0x4000000;                                     // Result = 04000000
    v0 += 0x7F;
    sb(v0, sp + 0x2D);
    v0 = lbu(a0 + 0x8);
    t7 = 0x80000000;                                    // Result = 80000000
    v0 += 0x3F;
    sb(v0, sp + 0x34);
    v1 = lbu(a0 + 0x9);
    v0 = 0x80;                                          // Result = 00000080
    sb(v0, sp + 0x14);
    sb(v0, sp + 0x15);
    sb(v0, sp + 0x16);
    v1 += 0x7F;
    sb(v1, sp + 0x35);
    v0 = lhu(a0 + 0xA);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0x7D34);                              // Load from: gPaletteClutId_CurMapSky (800782CC)
    t5 = -1;                                            // Result = FFFFFFFF
    sh(v0, sp + 0x26);
    sh(v1, sp + 0x1E);
loc_8003589C:
    t3 = sp + 0x14;
    t0 = lbu(sp + 0x13);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_800358B4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0)
        v0 = t1 + a0;
        if (bJump) goto loc_8003591C;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0)
        v0 = t2 + a0;
        if (bJump) goto loc_800359E0;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= t9;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003591C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_800359D0;
    if (v1 == a0) goto loc_800358B4;
loc_80035940:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t8;
    if (v0 == 0) goto loc_800358B4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t7;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800359AC;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80035990:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_80035990;
loc_800359AC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800358B4;
    goto loc_80035940;
loc_800359D0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_800359E0:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t4;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    a2 += 4;
    if (t0 == t5) goto loc_80035AA8;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80035A1C:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80035A1C;
    goto loc_80035AA8;
loc_80035A3C:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t8;
    if (v0 == 0) goto loc_80035AC4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t7;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_80035AA8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80035A8C:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80035A8C;
loc_80035AA8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80035A3C;
loc_80035AC4:
    t6++;
    v0 = lhu(sp + 0x18);
    v1 = lhu(sp + 0x20);
    v0 += 0x3F;
    sh(v0, sp + 0x18);
    v0 = lhu(sp + 0x28);
    v1 += 0x3F;
    sh(v1, sp + 0x20);
    v1 = lhu(sp + 0x30);
    v0 += 0x3F;
    v1 += 0x3F;
    sh(v0, sp + 0x28);
    v0 = (i32(t6) < 4);
    sh(v1, sp + 0x30);
    if (v0 != 0) goto loc_8003589C;
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x44);
    s0 = lw(sp + 0x40);
    sp += 0x48;
    return;
}

void RunMenu() noexcept {
loc_80035B24:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6F7C;                                       // Result = gPaletteClutId_Main (800A9084)
    sw(s1, sp + 0x14);
    s1 = 0x80090000;                                    // Result = 80090000
    s1 += 0x7A10;                                       // Result = gTexInfo_BACK[0] (80097A10)
    sw(s2, sp + 0x18);
    s2 = s1 + 0x40;                                     // Result = gTexInfo_DOOM[0] (80097A50)
    sw(ra, sp + 0x1C);
loc_80035B4C:
    a0 = 0x80030000;                                    // Result = 80030000
    a0 += 0x5C94;                                       // Result = M_Start (80035C94)
    a1 = 0x80030000;                                    // Result = 80030000
    a1 += 0x5E40;                                       // Result = M_Stop (80035E40)
    a2 = 0x80030000;                                    // Result = 80030000
    a2 += 0x5EC4;                                       // Result = M_Ticker (80035EC4)
    a3 = 0x80030000;                                    // Result = 80030000
    a3 += 0x6258;                                       // Result = M_Drawer (80036258)
    MiniLoop();
    v1 = 7;                                             // Result = 00000007
    {
        const bool bJump = (v0 == v1)
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_80035C78;
    }
    I_IncDrawnFrameCount();
    a0 = s1;                                            // Result = gTexInfo_BACK[0] (80097A10)
    a1 = 0;                                             // Result = 00000000
    a3 = lh(s0);                                        // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0;                                             // Result = 00000000
    I_CacheAndDrawSprite();
    a0 = s2;                                            // Result = gTexInfo_DOOM[0] (80097A50)
    a1 = 0x4B;                                          // Result = 0000004B
    a3 = lh(s0 + 0x22);                                 // Load from: gPaletteClutId_Title (800A90A6)
    a2 = 0x14;                                          // Result = 00000014
    I_CacheAndDrawSprite();
    I_SubmitGpuCmds();
    I_DrawPresent();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7604);                               // Load from: gStartGameType (80077604)
    a0 = s1 + 0x100;                                    // Result = gTexInfo_CONNECT[0] (80097B10)
    if (v0 == 0) goto loc_80035C4C;
    a1 = 0x36;                                          // Result = 00000036
    a3 = lh(s0);                                        // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0x67;                                          // Result = 00000067
    I_DrawPlaque();
    I_NetSetup();
    I_IncDrawnFrameCount();
    a0 = s1;                                            // Result = gTexInfo_BACK[0] (80097A10)
    a1 = 0;                                             // Result = 00000000
    a3 = lh(s0);                                        // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0;                                             // Result = 00000000
    I_CacheAndDrawSprite();
    a0 = s2;                                            // Result = gTexInfo_DOOM[0] (80097A50)
    a1 = 0x4B;                                          // Result = 0000004B
    a3 = lh(s0 + 0x22);                                 // Load from: gPaletteClutId_Title (800A90A6)
    a2 = 0x14;                                          // Result = 00000014
    I_CacheAndDrawSprite();
    I_SubmitGpuCmds();
    I_DrawPresent();
    a0 = 0;                                             // Result = 00000000
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C0C);                               // Load from: gbDidAbortGame (80077C0C)
    if (v0 != 0) goto loc_80035B4C;
loc_80035C4C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x75FC);                               // Load from: gStartSkill (800775FC)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7600);                               // Load from: gStartMapOrEpisode (80077600)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7604);                               // Load from: gStartGameType (80077604)
    G_InitNew();
    G_RunGame();
    v0 = 0;                                             // Result = 00000000
loc_80035C78:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void M_Start() noexcept {
    sp -= 0x28;
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x24);
    sw(s0, sp + 0x20);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7FA4);                                 // Store to: gNetGame (8007805C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7618);                                 // Store to: gCurPlayerIndex (80077618)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7F54);                                // Store to: gbPlayerInGame[0] (800780AC)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7F50);                                 // Store to: gbPlayerInGame[1] (800780B0)
    I_ResetTexCache();
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7A90;                                       // Result = gTexInfo_LOADING[0] (80097A90)
    a0 = s0;                                            // Result = gTexInfo_LOADING[0] (80097A90)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C4C;                                       // Result = STR_LumpName_LOADING[0] (80077C4C)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = s0;                                            // Result = gTexInfo_LOADING[0] (80097A90)
    a1 = 0x5F;                                          // Result = 0000005F
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    a2 = 0x6D;                                          // Result = 0000006D
    I_DrawPlaque();
    a0 = 0;                                             // Result = 00000000
    S_LoadSoundAndMusic();
    a0 = s0 - 0x80;                                     // Result = gTexInfo_BACK[0] (80097A10)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C8C;                                       // Result = STR_LumpName_BACK[0] (80077C8C)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = s0 - 0x40;                                     // Result = gTexInfo_DOOM[0] (80097A50)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C94;                                       // Result = STR_LumpName_DOOM[0] (80077C94)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = s0 + 0x80;                                     // Result = gTexInfo_CONNECT[0] (80097B10)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C9C;                                       // Result = STR_LumpName_CONNECT[0] (80077C9C)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7604);                               // Load from: gStartGameType (80077604)
    sw(0, gp + 0xBF8);                                  // Store to: gCursorFrame (800781D8)
    sw(0, gp + 0xA20);                                  // Store to: gCursorPos (80078000)
    sw(0, gp + 0x918);                                  // Store to: gVBlanksUntilMenuMove (80077EF8)
    {
        const bool bJump = (v0 != 0)
        v0 = 0x36;                                      // Result = 00000036
        if (bJump) goto loc_80035D64;
    }
    v0 = 2;                                             // Result = 00000002
loc_80035D64:
    sw(v0, gp + 0xB9C);                                 // Store to: gMaxStartEpisodeOrMap (8007817C)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7600);                               // Load from: gStartMapOrEpisode (80077600)
    v0 = lw(gp + 0xB9C);                                // Load from: gMaxStartEpisodeOrMap (8007817C)
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80035D8C;
    }
    v0 = 2;                                             // Result = 00000002
    if (i32(v1) >= 0) goto loc_80035D94;
loc_80035D8C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
loc_80035D94:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3E50;                                       // Result = CDTrackNum_MainMenu (80073E50)
    a0 = lw(v0);                                        // Load from: CDTrackNum_MainMenu (80073E50)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75F8);                               // Load from: gCdMusicVol (800775F8)
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
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    v0 = 1;                                             // Result = 00000001
    sb(v0, s0);                                         // Store to: gDrawEnv1[C] (800A90C4)
    at = 0x800B0000;                                    // Result = 800B0000
    sb(v0, at - 0x6EE0);                                // Store to: gDrawEnv2[C] (800A9120)
    sw(v1, gp + 0x92C);                                 // Store to: gMenuTimeoutStartTicCon (80077F0C)
    ra = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x28;
    return;
}

void M_Stop() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    a0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x14);
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    psxcd_stop();
    v0 = 9;                                             // Result = 00000009
    if (s0 != v0) goto loc_80035EB0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7604);                               // Load from: gStartGameType (80077604)
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80035EB0;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7600);                               // Load from: gStartMapOrEpisode (80077600)
    {
        const bool bJump = (v1 != v0)
        v0 = 0x1F;                                      // Result = 0000001F
        if (bJump) goto loc_80035EA8;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
    goto loc_80035EB0;
loc_80035EA8:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
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
    s0 = lw(s0 + 0x7F44);                               // Load from: gPlayerPadButtons[0] (80077F44)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7DEC);                               // Load from: gPlayerOldPadButtons[0] (80078214)
    sw(ra, sp + 0x14);
    if (s0 == 0) goto loc_80035EF4;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    sw(v0, gp + 0x92C);                                 // Store to: gMenuTimeoutStartTicCon (80077F0C)
loc_80035EF4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    v1 = lw(gp + 0x92C);                                // Load from: gMenuTimeoutStartTicCon (80077F0C)
    v0 -= v1;
    v0 = (i32(v0) < 0x708);
    {
        const bool bJump = (v0 == 0)
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_80036244;
    }
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7FB4);                               // Load from: gGameTic (8007804C)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7FA4);                               // Load from: gPrevGameTic (80077FA4)
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v0 == 0)
        v0 = s0 & 0xF0;
        if (bJump) goto loc_80035F64;
    }
    v0 = 9;                                             // Result = 00000009
    goto loc_80036244;
loc_80035F64:
    if (v0 == 0) goto loc_80035FCC;
    v1 = lw(gp + 0xA20);                                // Load from: gCursorPos (80078000)
    v0 = (i32(v1) < 3);
    if (i32(v1) < 0) goto loc_80035FCC;
    {
        const bool bJump = (v0 != 0)
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_80036244;
    }
    v0 = 3;                                             // Result = 00000003
    {
        const bool bJump = (v1 != v0)
        v0 = s0 & 0xF000;
        if (bJump) goto loc_80035FD0;
    }
    a0 = 0x80040000;                                    // Result = 80040000
    a0 -= 0x16F0;                                       // Result = O_Init (8003E910)
    a1 = 0x80040000;                                    // Result = 80040000
    a1 -= 0x1630;                                       // Result = O_Shutdown (8003E9D0)
    a2 = 0x80040000;                                    // Result = 80040000
    a2 -= 0x160C;                                       // Result = O_Control (8003E9F4)
    a3 = 0x80040000;                                    // Result = 80040000
    a3 -= 0x1138;                                       // Result = O_Drawer (8003EEC8)
    MiniLoop();
    v1 = 4;                                             // Result = 00000004
    {
        const bool bJump = (v0 != v1)
        v0 = s0 & 0xF000;
        if (bJump) goto loc_80035FD0;
    }
    v0 = 4;                                             // Result = 00000004
    goto loc_80036244;
loc_80035FCC:
    v0 = s0 & 0xF000;
loc_80035FD0:
    {
        const bool bJump = (v0 != 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80035FE4;
    }
    sw(0, gp + 0x918);                                  // Store to: gVBlanksUntilMenuMove (80077EF8)
    goto loc_80036244;
loc_80035FE4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7EF8;                                       // Result = gVBlanksUntilMenuMove (80077EF8)
    v0 = lw(a0);                                        // Load from: gVBlanksUntilMenuMove (80077EF8)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FBC);                               // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v0 -= v1;
    sw(v0, a0);                                         // Store to: gVBlanksUntilMenuMove (80077EF8)
    if (i32(v0) > 0) goto loc_80036240;
    v0 = 0xF;                                           // Result = 0000000F
    sw(v0, a0);                                         // Store to: gVBlanksUntilMenuMove (80077EF8)
    v0 = s0 & 0x4000;
    v1 = 4;                                             // Result = 00000004
    if (v0 == 0) goto loc_80036040;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x8000;                                       // Result = gCursorPos (80078000)
    v0 = lw(a0);                                        // Load from: gCursorPos (80078000)
    v0++;
    sw(v0, a0);                                         // Store to: gCursorPos (80078000)
    if (v0 != v1) goto loc_80036070;
    sw(0, a0);                                          // Store to: gCursorPos (80078000)
    goto loc_80036070;
loc_80036040:
    v0 = s0 & 0x1000;
    v1 = -1;                                            // Result = FFFFFFFF
    if (v0 == 0) goto loc_8003607C;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x8000;                                       // Result = gCursorPos (80078000)
    v0 = lw(a0);                                        // Load from: gCursorPos (80078000)
    v0--;
    sw(v0, a0);                                         // Store to: gCursorPos (80078000)
    if (v0 != v1) goto loc_80036070;
    v0 = 3;                                             // Result = 00000003
    sw(v0, a0);                                         // Store to: gCursorPos (80078000)
loc_80036070:
    a0 = 0;                                             // Result = 00000000
    a1 = 0x12;                                          // Result = 00000012
    S_StartSound();
loc_8003607C:
    v1 = lw(gp + 0xA20);                                // Load from: gCursorPos (80078000)
    a0 = 1;                                             // Result = 00000001
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
        const bool bJump = (v1 == v0)
        v0 = s0 & 0x2000;
        if (bJump) goto loc_800361E8;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_80036244;
loc_800360B8:
    {
        const bool bJump = (v0 == 0)
        v0 = s0 & 0x8000;
        if (bJump) goto loc_800360F0;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7604);                               // Load from: gStartGameType (80077604)
    v0 = (v1 < 2);
    {
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v0 == 0)
        v0--;
        if (bJump) goto loc_80036144;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7604);                                // Store to: gStartGameType (80077604)
    if (v0 != 0) goto loc_80036124;
loc_8003611C:
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
loc_80036124:
    a0 = 0;                                             // Result = 00000000
loc_80036128:
    a1 = 0x17;                                          // Result = 00000017
    S_StartSound();
loc_80036130:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7604);                               // Load from: gStartGameType (80077604)
    {
        const bool bJump = (v0 != 0)
        v0 = 0x36;                                      // Result = 00000036
        if (bJump) goto loc_80036148;
    }
loc_80036144:
    v0 = 2;                                             // Result = 00000002
loc_80036148:
    sw(v0, gp + 0xB9C);                                 // Store to: gMaxStartEpisodeOrMap (8007817C)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7600);                               // Load from: gStartMapOrEpisode (80077600)
    v0 = lw(gp + 0xB9C);                                // Load from: gMaxStartEpisodeOrMap (8007817C)
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80036240;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
    v0 = 0;                                             // Result = 00000000
    goto loc_80036244;
loc_80036178:
    v0 = s0 & 0x2000;
    {
        const bool bJump = (v0 == 0)
        v0 = s0 & 0x8000;
        if (bJump) goto loc_800361B0;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7600);                               // Load from: gStartMapOrEpisode (80077600)
    v1 = lw(gp + 0xB9C);                                // Load from: gMaxStartEpisodeOrMap (8007817C)
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
    v0 = (i32(v1) < i32(v0));
    a0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_800361D8;
    goto loc_80036238;
loc_800361B0:
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80036244;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7600);                               // Load from: gStartMapOrEpisode (80077600)
    v0--;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
    a0 = 0;                                             // Result = 00000000
    if (i32(v0) > 0) goto loc_80036238;
loc_800361D8:
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
    v0 = 0;                                             // Result = 00000000
    goto loc_80036244;
loc_800361E8:
    {
        const bool bJump = (v0 == 0)
        v0 = s0 & 0x8000;
        if (bJump) goto loc_80036210;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x75FC);                               // Load from: gStartSkill (800775FC)
    v0 = (v1 < 3);
    {
        const bool bJump = (v0 == 0)
        v0 = v1 + 1;
        if (bJump) goto loc_80036240;
    }
    goto loc_8003622C;
loc_80036210:
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80036244;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x75FC);                               // Load from: gStartSkill (800775FC)
    {
        const bool bJump = (v0 == 0)
        v0--;
        if (bJump) goto loc_80036240;
    }
loc_8003622C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x75FC);                                // Store to: gStartSkill (800775FC)
    a0 = 0;                                             // Result = 00000000
loc_80036238:
    a1 = 0x17;                                          // Result = 00000017
    S_StartSound();
loc_80036240:
    v0 = 0;                                             // Result = 00000000
loc_80036244:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void M_Drawer() noexcept {
loc_80036258:
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s0, sp + 0x20);
    I_IncDrawnFrameCount();
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7A10;                                       // Result = gTexInfo_BACK[0] (80097A10)
    a0 = s0;                                            // Result = gTexInfo_BACK[0] (80097A10)
    a1 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F7C);                               // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0;                                             // Result = 00000000
    I_CacheAndDrawSprite();
    a0 = s0 + 0x40;                                     // Result = gTexInfo_DOOM[0] (80097A50)
    a1 = 0x4B;                                          // Result = 0000004B
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5A);                               // Load from: gPaletteClutId_Title (800A90A6)
    a2 = 0x14;                                          // Result = 00000014
    I_CacheAndDrawSprite();
    a2 = 0x32;                                          // Result = 00000032
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lhu(a0 - 0x6B0E);                              // Load from: gTexInfo_STATUS[2] (800A94F2)
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lh(a1 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    v1 = lw(gp + 0xA20);                                // Load from: gCursorPos (80078000)
    v0 = lw(gp + 0xBF8);                                // Load from: gCursorFrame (800781D8)
    v1 <<= 1;
    v0 <<= 4;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7C34;                                       // Result = MainMenu_GameMode_YPos (80077C34)
    at += v1;
    a3 = lh(at);
    v0 += 0x84;
    sw(v0, sp + 0x10);
    v0 = 0xC0;                                          // Result = 000000C0
    sw(v0, sp + 0x14);
    v0 = 0x10;                                          // Result = 00000010
    sw(v0, sp + 0x18);
    v0 = 0x12;                                          // Result = 00000012
    sw(v0, sp + 0x1C);
    a3 -= 2;
    I_DrawSprite();
    a1 = lh(gp + 0x654);                                // Load from: MainMenu_GameMode_YPos (80077C34)
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1568;                                       // Result = STR_GameMode[0] (80011568)
    a0 = 0x4A;                                          // Result = 0000004A
    I_DrawString();
    a0 = 0x5A;                                          // Result = 0000005A
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3CDC;                                       // Result = STR_MenuOpt_SinglePlayer[0] (80073CDC)
    a1 = lh(gp + 0x654);                                // Load from: MainMenu_GameMode_YPos (80077C34)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7604);                               // Load from: gStartGameType (80077604)
    a1 += 0x14;
    a2 <<= 4;
    a2 += v0;
    I_DrawString();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7604);                               // Load from: gStartGameType (80077604)
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80036398;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7600);                               // Load from: gStartMapOrEpisode (80077600)
    if (v1 != v0) goto loc_8003637C;
    a1 = lh(gp + 0x656);                                // Load from: MainMenu_Episode_YPos (80077C36)
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1574;                                       // Result = STR_UltimateDoom[0] (80011574)
    a0 = 0x4A;                                          // Result = 0000004A
    I_DrawString();
    goto loc_800363D4;
loc_8003637C:
    a1 = lh(gp + 0x656);                                // Load from: MainMenu_Episode_YPos (80077C36)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7CA4;                                       // Result = STR_Doom2[0] (80077CA4)
    a0 = 0x4A;                                          // Result = 0000004A
    I_DrawString();
    goto loc_800363D4;
loc_80036398:
    a1 = lh(gp + 0x656);                                // Load from: MainMenu_Episode_YPos (80077C36)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7CAC;                                       // Result = STR_Level[0] (80077CAC)
    a0 = 0x4A;                                          // Result = 0000004A
    I_DrawString();
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7600);                               // Load from: gStartMapOrEpisode (80077600)
    v0 = (i32(a2) < 0xA);
    a0 = 0x88;                                          // Result = 00000088
    if (v0 != 0) goto loc_800363C8;
    a0 = 0x94;                                          // Result = 00000094
loc_800363C8:
    a1 = lh(gp + 0x656);                                // Load from: MainMenu_Episode_YPos (80077C36)
    I_DrawNumber();
loc_800363D4:
    a1 = lh(gp + 0x658);                                // Load from: MainMenu_Difficulty_YPos (80077C38)
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1584;                                       // Result = STR_Difficulty[0] (80011584)
    a0 = 0x4A;                                          // Result = 0000004A
    I_DrawString();
    a0 = 0x5A;                                          // Result = 0000005A
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3D0C;                                       // Result = STR_MenuOpt_IAmAWimp[0] (80073D0C)
    a1 = lh(gp + 0x658);                                // Load from: MainMenu_Difficulty_YPos (80077C38)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x75FC);                               // Load from: gStartSkill (800775FC)
    a1 += 0x14;
    a2 <<= 4;
    a2 += v0;
    I_DrawString();
    a1 = lh(gp + 0x65A);                                // Load from: MainMenu_Options_YPos (80077C3A)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7CB4;                                       // Result = STR_Options[0] (80077CB4)
    a0 = 0x4A;                                          // Result = 0000004A
    I_DrawString();
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x28;
    return;
}

void I_CrossFadeFrameBuffers() noexcept {
loc_80036448:
    sp -= 0x188;
    sw(ra, sp + 0x184);
    sw(s6, sp + 0x180);
    sw(s5, sp + 0x17C);
    sw(s4, sp + 0x178);
    sw(s3, sp + 0x174);
    sw(s2, sp + 0x170);
    sw(s1, sp + 0x16C);
    sw(s0, sp + 0x168);
    I_ResetTexCache();
    s3 = sp + 0x68;
    a0 = s3;
    a1 = 0x200;                                         // Result = 00000200
    a2 = 0x100;                                         // Result = 00000100
    a3 = 0x100;                                         // Result = 00000100
    s1 = 0xF0;                                          // Result = 000000F0
    sw(s1, sp + 0x10);
    LIBGPU_SetDefDrawEnv();
    a0 = sp + 0xC4;
    a1 = 0x300;                                         // Result = 00000300
    a2 = 0x100;                                         // Result = 00000100
    a3 = 0x100;                                         // Result = 00000100
    s0 = 1;                                             // Result = 00000001
    sb(s0, sp + 0x80);
    sb(0, sp + 0x7E);
    sb(s0, sp + 0x7F);
    sw(s1, sp + 0x10);
    LIBGPU_SetDefDrawEnv();
    s2 = sp + 0x120;
    a0 = s2;
    a1 = 0x300;                                         // Result = 00000300
    a2 = 0x100;                                         // Result = 00000100
    a3 = 0x100;                                         // Result = 00000100
    sb(s0, sp + 0xDC);
    sb(0, sp + 0xDA);
    sb(s0, sp + 0xDB);
    sw(s1, sp + 0x10);
    LIBGPU_SetDefDispEnv();
    a0 = sp + 0x134;
    a1 = 0x200;                                         // Result = 00000200
    a2 = 0x100;                                         // Result = 00000100
    a3 = 0x100;                                         // Result = 00000100
    sw(s1, sp + 0x10);
    LIBGPU_SetDefDispEnv();
    a1 = 0x300;                                         // Result = 00000300
    a2 = 0x100;                                         // Result = 00000100
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F08);                               // Load from: gCurDrawDispBufferIdx (800780F8)
    s4 = 0;                                             // Result = 00000000
    a0 = v0 << 2;
    a0 += v0;
    a0 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x6E9C;                                       // Result = gDispEnv1[0] (800A9164)
    a0 += v0;
    LIBGPU_MoveImage();
    a0 = 0;                                             // Result = 00000000
    LIBGPU_DrawSync();
    a0 = 0;                                             // Result = 00000000
    LIBETC_VSync();
    a0 = s3;
    LIBGPU_PutDrawEnv();
    a0 = s2;
    LIBGPU_PutDispEnv();
    v0 = 9;                                             // Result = 00000009
    sb(v0, sp + 0x1B);
    v0 = 0x2C;                                          // Result = 0000002C
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7F08);                               // Load from: gCurDrawDispBufferIdx (800780F8)
    v1 = 0xFF;                                          // Result = 000000FF
    sb(v0, sp + 0x1F);
    v0 = 0xEF;                                          // Result = 000000EF
    sh(v1, sp + 0x28);
    sh(v1, sp + 0x38);
    v1 = 0xFF;                                          // Result = 000000FF
    sh(v0, sp + 0x32);
    sh(v0, sp + 0x3A);
    v0 = 0xEF;                                          // Result = 000000EF
    sh(0, sp + 0x20);
    sh(0, sp + 0x22);
    sh(0, sp + 0x2A);
    sh(0, sp + 0x30);
    sb(0, sp + 0x24);
    sb(0, sp + 0x25);
    sb(v1, sp + 0x2C);
    sb(0, sp + 0x2D);
    sb(0, sp + 0x34);
    sb(v0, sp + 0x35);
    sb(v1, sp + 0x3C);
    sb(v0, sp + 0x3D);
    if (a0 != 0) goto loc_800365C4;
    a0 = 2;                                             // Result = 00000002
    a1 = 0;                                             // Result = 00000000
    a2 = 0x100;                                         // Result = 00000100
    goto loc_800365D0;
loc_800365C4:
    a0 = 2;                                             // Result = 00000002
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
loc_800365D0:
    a3 = 0;                                             // Result = 00000000
    LIBGPU_GetTPage();
    sh(v0, sp + 0x2E);
    v0 = 9;                                             // Result = 00000009
    sb(v0, sp + 0x43);
    v0 = 0x2E;                                          // Result = 0000002E
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7F08);                               // Load from: gCurDrawDispBufferIdx (800780F8)
    v1 = 0xFF;                                          // Result = 000000FF
    sb(v0, sp + 0x47);
    v0 = 0xEF;                                          // Result = 000000EF
    sh(v1, sp + 0x50);
    sh(v1, sp + 0x60);
    v1 = 0xFF;                                          // Result = 000000FF
    sh(v0, sp + 0x5A);
    sh(v0, sp + 0x62);
    v0 = 0xEF;                                          // Result = 000000EF
    sh(0, sp + 0x26);
    sh(0, sp + 0x48);
    sh(0, sp + 0x4A);
    sh(0, sp + 0x52);
    sh(0, sp + 0x58);
    sb(0, sp + 0x4C);
    sb(0, sp + 0x4D);
    sb(v1, sp + 0x54);
    sb(0, sp + 0x55);
    sb(0, sp + 0x5C);
    sb(v0, sp + 0x5D);
    sb(v1, sp + 0x64);
    sb(v0, sp + 0x65);
    if (a0 == 0) goto loc_8003665C;
    a0 = 2;                                             // Result = 00000002
    a1 = 0;                                             // Result = 00000000
    a2 = 0x100;                                         // Result = 00000100
    goto loc_80036668;
loc_8003665C:
    a0 = 2;                                             // Result = 00000002
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
loc_80036668:
    a3 = 0;                                             // Result = 00000000
    LIBGPU_GetTPage();
    sh(v0, sp + 0x56);
    sh(0, sp + 0x4E);
    s1 = 0xFF;                                          // Result = 000000FF
    s0 = 0xFF0000;                                      // Result = 00FF0000
    s0 |= 0xFFFF;                                       // Result = 00FFFFFF
    s6 = 0x80080000;                                    // Result = 80080000
    s6 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s5 = s6 & s0;                                       // Result = 00086550
    s3 = 0x4000000;                                     // Result = 04000000
    s2 = 0x80000000;                                    // Result = 80000000
    t3 = sp + 0x1C;
loc_8003669C:
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = lbu(sp + 0x1B);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = ~s1;
    sb(s1, sp + 0x1C);
    sb(s1, sp + 0x1D);
    sb(s1, sp + 0x1E);
    sb(v0, sp + 0x44);
    sb(v0, sp + 0x45);
    sb(v0, sp + 0x46);
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_800366D0:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0)
        v0 = t1 + a0;
        if (bJump) goto loc_80036738;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0)
        v0 = t2 + a0;
        if (bJump) goto loc_800367FC;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(s6, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s5;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80036738:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_800367EC;
    if (v1 == a0) goto loc_800366D0;
loc_8003675C:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= s3;
    if (v0 == 0) goto loc_800366D0;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= s0;
    v0 |= s2;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_800367C8;
    a3 = -1;                                            // Result = FFFFFFFF
loc_800367AC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_800367AC;
loc_800367C8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800366D0;
    goto loc_8003675C;
loc_800367EC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_800367FC:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= s0;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a2 += 4;
    if (t0 == v0) goto loc_80036854;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003683C:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_8003683C;
loc_80036854:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800368FC;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80036874:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= s3;
    if (v0 == 0) goto loc_800368FC;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= s0;
    v0 |= s2;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == a3) goto loc_800368E0;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800368C4:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800368C4;
loc_800368E0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80036874;
loc_800368FC:
    t3 = sp + 0x44;
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = lbu(sp + 0x43);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_80036918:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0)
        v0 = t1 + a0;
        if (bJump) goto loc_80036980;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0)
        v0 = t2 + a0;
        if (bJump) goto loc_80036A44;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(s6, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s5;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80036980:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80036A34;
    if (v1 == a0) goto loc_80036918;
loc_800369A4:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= s3;
    if (v0 == 0) goto loc_80036918;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= s0;
    v0 |= s2;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_80036A10;
    a3 = -1;                                            // Result = FFFFFFFF
loc_800369F4:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_800369F4;
loc_80036A10:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80036918;
    goto loc_800369A4;
loc_80036A34:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_80036A44:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= s0;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a2 += 4;
    if (t0 == v0) goto loc_80036A9C;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80036A84:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80036A84;
loc_80036A9C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a3 = -1;                                            // Result = FFFFFFFF
    if (v1 == v0) goto loc_80036B40;
loc_80036AB8:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= s3;
    if (v0 == 0) goto loc_80036B40;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= s0;
    v0 |= s2;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == a3) goto loc_80036B24;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80036B08:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80036B08;
loc_80036B24:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80036AB8;
loc_80036B40:
    s4 ^= 1;
    I_SubmitGpuCmds();
    a0 = 0;                                             // Result = 00000000
    LIBGPU_DrawSync();
    a0 = 0;                                             // Result = 00000000
    LIBETC_VSync();
    v0 = sp + 0x68;
    a0 = s4 << 1;
    a0 += s4;
    a0 <<= 3;
    a0 -= s4;
    a0 <<= 2;
    a0 += v0;
    LIBGPU_PutDrawEnv();
    v0 = sp + 0x120;
    a0 = s4 << 2;
    a0 += s4;
    a0 <<= 2;
    a0 += v0;
    LIBGPU_PutDispEnv();
    s1 -= 5;
    t3 = sp + 0x1C;
    if (i32(s1) >= 0) goto loc_8003669C;
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x184);
    s6 = lw(sp + 0x180);
    s5 = lw(sp + 0x17C);
    s4 = lw(sp + 0x178);
    s3 = lw(sp + 0x174);
    s2 = lw(sp + 0x170);
    s1 = lw(sp + 0x16C);
    s0 = lw(sp + 0x168);
    sp += 0x188;
    return;
}

void START_PasswordScreen() noexcept {
    sp -= 0x18;
    a0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x10);
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F44);                               // Load from: gPlayerPadButtons[0] (80077F44)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F48);                               // Load from: gPlayerPadButtons[1] (80077F48)
    sw(0, gp + 0xA4C);                                  // Store to: gInvalidPasswordFlashTicsLeft (8007802C)
    sw(0, gp + 0xB94);                                  // Store to: gCurPasswordCharIdx (80078174)
    sw(0, gp + 0x918);                                  // Store to: gVBlanksUntilMenuMove (80077EF8)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7DEC);                                // Store to: gPlayerOldPadButtons[0] (80078214)
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7DE8);                                // Store to: gPlayerOldPadButtons[1] (80078218)
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
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7FB4);                               // Load from: gGameTic (8007804C)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7FA4);                               // Load from: gPrevGameTic (80077FA4)
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0)
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
    s0 = lw(s0 + 0x7F44);                               // Load from: gPlayerPadButtons[0] (80077F44)
    s1 = 0x80080000;                                    // Result = 80080000
    s1 = lw(s1 - 0x7DEC);                               // Load from: gPlayerOldPadButtons[0] (80078214)
    v0 = s0 & 0xF000;
    {
        const bool bJump = (v0 != 0)
        v0 = s0 & 0x900;
        if (bJump) goto loc_80036F1C;
    }
    sw(0, gp + 0x918);                                  // Store to: gVBlanksUntilMenuMove (80077EF8)
    goto loc_8003700C;
loc_80036F1C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7EF8;                                       // Result = gVBlanksUntilMenuMove (80077EF8)
    v0 = lw(a0);                                        // Load from: gVBlanksUntilMenuMove (80077EF8)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FBC);                               // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v0 -= v1;
    sw(v0, a0);                                         // Store to: gVBlanksUntilMenuMove (80077EF8)
    if (i32(v0) > 0) goto loc_80037008;
    v0 = 0xF;                                           // Result = 0000000F
    sw(v0, a0);                                         // Store to: gVBlanksUntilMenuMove (80077EF8)
    v0 = s0 & 0x1000;
    {
        const bool bJump = (v0 == 0)
        v0 = s0 & 0x4000;
        if (bJump) goto loc_80036F70;
    }
    v1 = lw(gp + 0xB94);                                // Load from: gCurPasswordCharIdx (80078174)
    v0 = (i32(v1) < 8);
    {
        const bool bJump = (v0 != 0)
        v0 = s0 & 0x8000;
        if (bJump) goto loc_80036FA0;
    }
    v0 = v1 - 8;
    goto loc_80036F8C;
loc_80036F70:
    {
        const bool bJump = (v0 == 0)
        v0 = s0 & 0x8000;
        if (bJump) goto loc_80036FA0;
    }
    v1 = lw(gp + 0xB94);                                // Load from: gCurPasswordCharIdx (80078174)
    v0 = (i32(v1) < 0x18);
    {
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v0 != 0)
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
        const bool bJump = (v0 != 0)
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
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7FB8);                                // Store to: gGameMap (80078048)
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x7600);                                // Store to: gStartMapOrEpisode (80077600)
    at = 0x80080000;                                    // Result = 80080000
    sw(a1, at - 0x7DA8);                                // Store to: gGameSkill (80078258)
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x75FC);                                // Store to: gStartSkill (800775FC)
    v0 = 4;                                             // Result = 00000004
    goto loc_8003711C;
loc_800370C0:
    v0 = 0x10;                                          // Result = 00000010
    sw(v0, gp + 0xA4C);                                 // Store to: gInvalidPasswordFlashTicsLeft (8007802C)
    v0 = 0;                                             // Result = 00000000
    goto loc_8003711C;
loc_800370D0:
    v0 = s0 & 0x10;
    {
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v0 != 0)
        v0 = t1 + a0;
        if (bJump) goto loc_8003725C;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0)
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
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    v0 &= 4;
    {
        const bool bJump = (v0 != 0)
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
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v0 != 0)
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
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v1 != v0)
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
        const bool bJump = (v0 == 0)
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
    v1 = lw(v1 - 0x7FB4);                               // Load from: gGameTic (8007804C)
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
        const bool bJump = (v0 != 0)
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
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v0 == 0)
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
        const bool bJump = (v0 != 0)
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
        const bool bJump = (v0 == 0)
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

void P_ComputePassword() noexcept {
loc_80037DBC:
    sp -= 0x28;
    sw(s1, sp + 0x1C);
    s1 = a0;
    a0 = sp + 0x10;
    a1 = 0;                                             // Result = 00000000
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a2 = 8;                                             // Result = 00000008
    sw(ra, sp + 0x20);
    sw(s0, sp + 0x18);
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    s0 = v1 + v0;
    D_memset();
    a0 = 0;                                             // Result = 00000000
    a2 = 1;                                             // Result = 00000001
    a1 = s0;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0x7F68);                              // Load from: gNextMap (80078098)
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0x7DA8);                              // Load from: gGameSkill (80078258)
    v0 &= 0x3F;
    v0 <<= 2;
    v1 &= 3;
    sb(v0, sp + 0x10);
    v0 |= v1;
    sb(v0, sp + 0x10);
loc_80037E3C:
    v0 = lw(a1 + 0x7C);
    a1 += 4;
    if (v0 == 0) goto loc_80037E5C;
    v0 = lbu(sp + 0x11);
    v1 = a2 << a0;
    v0 |= v1;
    sb(v0, sp + 0x11);
loc_80037E5C:
    a0++;
    v0 = (i32(a0) < 7);
    if (v0 != 0) goto loc_80037E3C;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 = lw(v1 + 0x70D4);                               // Load from: gMaxAmmo[0] (800670D4)
    a0 = 0x80060000;                                    // Result = 80060000
    a0 = lw(a0 + 0x70D8);                               // Load from: gMaxAmmo[1] (800670D8)
    a2 = 0x80060000;                                    // Result = 80060000
    a2 = lw(a2 + 0x70DC);                               // Load from: gMaxAmmo[2] (800670DC)
    v0 = lw(s0 + 0x60);
    a3 = 0x80060000;                                    // Result = 80060000
    a3 = lw(a3 + 0x70E0);                               // Load from: gMaxAmmo[3] (800670E0)
    if (v0 == 0) goto loc_80037EB4;
    v1 <<= 1;
    a0 <<= 1;
    a2 <<= 1;
    v0 = lbu(sp + 0x11);
    a3 <<= 1;
    v0 |= 0x80;
    sb(v0, sp + 0x11);
loc_80037EB4:
    v0 = lw(s0 + 0x98);
    v0 <<= 3;
    div(v0, v1);
    if (v1 != 0) goto loc_80037ED0;
    _break(0x1C00);
loc_80037ED0:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at)
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80037EE8;
    }
    if (v0 != at) goto loc_80037EE8;
    tge(zero, zero, 0x5D);
loc_80037EE8:
    a1 = lo;
    v0 = hi;
    v1 = a1 << 4;
    if (v0 == 0) goto loc_80037F00;
    a1++;
    v1 = a1 << 4;
loc_80037F00:
    sb(v1, sp + 0x12);
    v0 = lw(s0 + 0x9C);
    v0 <<= 3;
    div(v0, a0);
    if (a0 != 0) goto loc_80037F20;
    _break(0x1C00);
loc_80037F20:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a0 != at)
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80037F38;
    }
    if (v0 != at) goto loc_80037F38;
    tge(zero, zero, 0x5D);
loc_80037F38:
    a1 = lo;
    v0 = hi;
    {
        const bool bJump = (v0 == 0)
        v0 = v1 | a1;
        if (bJump) goto loc_80037F50;
    }
    a1++;
    v0 = v1 | a1;
loc_80037F50:
    sb(v0, sp + 0x12);
    v0 = lw(s0 + 0xA0);
    v0 <<= 3;
    div(v0, a2);
    if (a2 != 0) goto loc_80037F70;
    _break(0x1C00);
loc_80037F70:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a2 != at)
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80037F88;
    }
    if (v0 != at) goto loc_80037F88;
    tge(zero, zero, 0x5D);
loc_80037F88:
    a1 = lo;
    v0 = hi;
    v1 = a1 << 4;
    if (v0 == 0) goto loc_80037FA0;
    a1++;
    v1 = a1 << 4;
loc_80037FA0:
    sb(v1, sp + 0x13);
    v0 = lw(s0 + 0xA4);
    v0 <<= 3;
    div(v0, a3);
    if (a3 != 0) goto loc_80037FC0;
    _break(0x1C00);
loc_80037FC0:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a3 != at)
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80037FD8;
    }
    if (v0 != at) goto loc_80037FD8;
    tge(zero, zero, 0x5D);
loc_80037FD8:
    a1 = lo;
    v0 = hi;
    a2 = 0x51EB0000;                                    // Result = 51EB0000
    if (v0 == 0) goto loc_80037FEC;
    a1++;
loc_80037FEC:
    v0 = v1 | a1;
    sb(v0, sp + 0x13);
    v1 = lw(s0 + 0x24);
    a2 |= 0x851F;                                       // Result = 51EB851F
    mult(v1, a2);
    v0 = hi;
    a0 = v1 << 3;
    mult(a0, a2);
    v1 = u32(i32(v1) >> 31);
    v0 = u32(i32(v0) >> 3);
    a1 = v0 - v1;
    v0 = u32(i32(a0) >> 31);
    v1 = hi;
    v1 = u32(i32(v1) >> 6);
    v1 -= v0;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += v1;
    v0 <<= 3;
    a3 = a1 << 4;
    if (a0 == v0) goto loc_8003804C;
    a1++;
    a3 = a1 << 4;
loc_8003804C:
    sb(a3, sp + 0x14);
    v0 = lw(s0 + 0x28);
    mult(v0, a2);
    v1 = hi;
    a0 = v0 << 3;
    mult(a0, a2);
    v0 = u32(i32(v0) >> 31);
    v1 = u32(i32(v1) >> 3);
    a1 = v1 - v0;
    v0 = u32(i32(a0) >> 31);
    v1 = hi;
    v1 = u32(i32(v1) >> 6);
    v1 -= v0;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += v1;
    v0 <<= 3;
    t0 = sp + 0x10;
    if (a0 == v0) goto loc_800380A4;
    a1++;
loc_800380A4:
    a0 = 0;                                             // Result = 00000000
    v0 = a3 | a1;
    sb(v0, sp + 0x14);
    v0 = lbu(s0 + 0x2C);
    t1 = 0x80;                                          // Result = 00000080
    v0 <<= 3;
    sb(v0, sp + 0x15);
    a3 = 0;                                             // Result = 00000000
loc_800380C4:
    a2 = 0x10;                                          // Result = 00000010
    a1 = 4;                                             // Result = 00000004
loc_800380CC:
    v0 = a0;
    if (i32(a0) >= 0) goto loc_800380D8;
    v0 = a0 + 7;
loc_800380D8:
    v0 = u32(i32(v0) >> 3);
    v1 = t0 + v0;
    v1 = lbu(v1);
    v0 <<= 3;
    v0 = a0 - v0;
    v0 = i32(t1) >> v0;
    v1 &= v0;
    a0++;
    if (v1 == 0) goto loc_80038100;
    a3 |= a2;
loc_80038100:
    a1--;
    a2 = u32(i32(a2) >> 1);
    if (i32(a1) >= 0) goto loc_800380CC;
    v0 = 0x66660000;                                    // Result = 66660000
    v0 |= 0x6667;                                       // Result = 66666667
    v1 = a0 - 1;
    mult(v1, v0);
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 = u32(i32(v0) >> 1);
    v0 -= v1;
    v0 += s1;
    sb(a3, v0);
    v0 = (i32(a0) < 0x2D);
    a3 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_800380C4;
    sb(0, s1 + 0x9);
    a0 = s1;
    a1 = s1 + 9;
loc_8003814C:
    v0 = lbu(s1 + 0x9);
    v1 = lbu(a0);
    a0++;
    v0 ^= v1;
    sb(v0, s1 + 0x9);
    v0 = (i32(a0) < i32(a1));
    if (v0 != 0) goto loc_8003814C;
    a0 = s1;
    a1 = s1 + 9;
loc_80038174:
    v0 = lbu(a0);
    v1 = lbu(s1 + 0x9);
    v0 ^= v1;
    sb(v0, a0);
    a0++;
    v0 = (i32(a0) < i32(a1));
    if (v0 != 0) goto loc_80038174;
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void P_ProcessPassword() noexcept {
loc_800381B0:
    sp -= 0x40;
    v0 = a0;
    sw(s2, sp + 0x30);
    s2 = a1;
    sw(s3, sp + 0x34);
    s3 = a2;
    sw(s1, sp + 0x2C);
    s1 = a3;
    sw(s0, sp + 0x28);
    s0 = sp + 0x18;
    a0 = s0;
    a1 = v0;
    sw(ra, sp + 0x38);
    a2 = 0xA;                                           // Result = 0000000A
    D_memcpy();
    a0 = sp + 0x21;
loc_800381F0:
    v0 = lbu(s0);
    v1 = lbu(sp + 0x21);
    v0 ^= v1;
    sb(v0, s0);
    s0++;
    v0 = (i32(s0) < i32(a0));
    v1 = sp + 0x18;
    if (v0 != 0) goto loc_800381F0;
    a0 = 0;                                             // Result = 00000000
    a1 = sp + 0x21;
loc_8003821C:
    v0 = lbu(v1);
    v1++;
    a0 ^= v0;
    v0 = (i32(v1) < i32(a1));
    if (v0 != 0) goto loc_8003821C;
    v0 = lbu(sp + 0x21);
    {
        const bool bJump = (a0 != v0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    a1 = 0;                                             // Result = 00000000
    t2 = 0x66660000;                                    // Result = 66660000
    t2 |= 0x6667;                                       // Result = 66666667
    t1 = sp + 0x18;
    t3 = 0x10;                                          // Result = 00000010
loc_80038258:
    t0 = 0;                                             // Result = 00000000
    a3 = 0x80;                                          // Result = 00000080
    a2 = 7;                                             // Result = 00000007
loc_80038264:
    mult(a1, t2);
    v0 = u32(i32(a1) >> 31);
    v1 = hi;
    v1 = u32(i32(v1) >> 1);
    v1 -= v0;
    v0 = t1 + v1;
    a0 = lbu(v0);
    v0 = v1 << 2;
    v0 += v1;
    v0 = a1 - v0;
    v0 = i32(t3) >> v0;
    a0 &= v0;
    a1++;
    if (a0 == 0) goto loc_800382A0;
    t0 |= a3;
loc_800382A0:
    a2--;
    a3 = u32(i32(a3) >> 1);
    if (i32(a2) >= 0) goto loc_80038264;
    v0 = a1 - 1;
    v1 = sp + 0x10;
    if (i32(v0) >= 0) goto loc_800382BC;
    v0 = a1 + 6;
loc_800382BC:
    v0 = u32(i32(v0) >> 3);
    v1 += v0;
    v0 = (i32(a1) < 0x30);
    sb(t0, v1);
    if (v0 != 0) goto loc_80038258;
    v0 = lbu(sp + 0x10);
    v0 >>= 2;
    sw(v0, s2);
    if (v0 == 0) goto loc_800383B0;
    v0 = (i32(v0) < 0x3C);
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x10);
    v0 &= 3;
    sw(v0, s3);
    v0 = lbu(sp + 0x12);
    v0 &= 0xF;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x12);
    v0 >>= 4;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x13);
    v0 &= 0xF;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x13);
    v0 >>= 4;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x14);
    v0 &= 0xF;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x14);
    v1 = v0 >> 4;
    v0 = (v1 < 9);
    if (v0 == 0) goto loc_800383B0;
    v0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80038538;
    v0 = lbu(sp + 0x15);
    v0 >>= 3;
    v0 = (v0 < 3);
    if (v0 != 0) goto loc_800383B8;
loc_800383B0:
    v0 = 0;                                             // Result = 00000000
    goto loc_80038538;
loc_800383B8:
    a1 = 0;                                             // Result = 00000000
    if (s1 != 0) goto loc_800383C8;
    v0 = 1;                                             // Result = 00000001
    goto loc_80038538;
loc_800383C8:
    a0 = 1;                                             // Result = 00000001
    v1 = s1;
loc_800383D0:
    v0 = lbu(sp + 0x11);
    v0 = i32(v0) >> a1;
    v0 &= 1;
    a1++;
    if (v0 == 0) goto loc_800383EC;
    sw(a0, v1 + 0x7C);
loc_800383EC:
    v0 = (i32(a1) < 7);
    v1 += 4;
    if (v0 != 0) goto loc_800383D0;
    v0 = lbu(sp + 0x11);
    v0 &= 0x80;
    if (v0 == 0) goto loc_80038444;
    v0 = lw(s1 + 0x60);
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80038444;
    }
    sw(v0, s1 + 0x60);
    a1 = 0;                                             // Result = 00000000
    v1 = s1;
loc_80038428:
    v0 = lw(v1 + 0xA8);
    a1++;
    v0 <<= 1;
    sw(v0, v1 + 0xA8);
    v0 = (i32(a1) < 4);
    v1 += 4;
    if (v0 != 0) goto loc_80038428;
loc_80038444:
    v0 = lbu(sp + 0x12);
    v1 = lw(s1 + 0xA8);
    v0 >>= 4;
    mult(v0, v1);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_80038464;
    v0 += 7;
loc_80038464:
    v0 = u32(i32(v0) >> 3);
    sw(v0, s1 + 0x98);
    v0 = lbu(sp + 0x12);
    v1 = lw(s1 + 0xAC);
    v0 &= 0xF;
    mult(v0, v1);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_8003848C;
    v0 += 7;
loc_8003848C:
    v0 = u32(i32(v0) >> 3);
    sw(v0, s1 + 0x9C);
    v0 = lbu(sp + 0x13);
    v1 = lw(s1 + 0xB0);
    v0 >>= 4;
    mult(v0, v1);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_800384B4;
    v0 += 7;
loc_800384B4:
    v0 = u32(i32(v0) >> 3);
    sw(v0, s1 + 0xA0);
    v0 = lbu(sp + 0x13);
    v1 = lw(s1 + 0xB4);
    v0 &= 0xF;
    mult(v0, v1);
    v0 = lo;
    if (i32(v0) >= 0) goto loc_800384DC;
    v0 += 7;
loc_800384DC:
    v0 = u32(i32(v0) >> 3);
    sw(v0, s1 + 0xA4);
    v1 = lbu(sp + 0x14);
    a1 = lw(s1);
    v1 >>= 4;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += v1;
    sw(v0, s1 + 0x24);
    a0 = lbu(sp + 0x14);
    v0 = 1;                                             // Result = 00000001
    a0 &= 0xF;
    v1 = a0 << 1;
    v1 += a0;
    v1 <<= 3;
    v1 += a0;
    sw(v1, s1 + 0x28);
    v1 = lbu(sp + 0x15);
    a0 = lw(s1 + 0x24);
    v1 >>= 3;
    sw(v1, s1 + 0x2C);
    sw(a0, a1 + 0x68);
loc_80038538:
    ra = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x40;
    return;
}

void PsxSoundInit() noexcept {
loc_800415EC:
    sp -= 0x28;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s4, sp + 0x20);
    s4 = a1;
    sw(s1, sp + 0x14);
    s1 = a2;
    sw(ra, sp + 0x24);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
    wess_init();
    psxcd_init();
    a0 = 0xC9;                                          // Result = 000000C9
    psxcd_open();
    a0 = s1;
    s0 = v0;
    a1 = lw(s0 + 0x4);
    a2 = s0;
    psxcd_read();
    a0 = s0;
    psxcd_close();
    s2 = 0x80080000;                                    // Result = 80080000
    s2 -= 0x1364;                                       // Result = gDoomSfxLoadedSamples[0] (8007EC9C)
    a0 = s2;                                            // Result = gDoomSfxLoadedSamples[0] (8007EC9C)
    ZeroHalfWord();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x11D0;                                       // Result = gMapMusSfxLoadedSamples[0] (8007EE30)
    ZeroHalfWord();
    a1 = 0x80080000;                                    // Result = 80080000
    a1 -= 0x7A78;                                       // Result = gPSXSND_wmdMemBuffer[0] (80078588)
    a2 = lw(gp + 0x824);                                // Load from: gPSXSND_maxWmdSize (80077E04)
    a3 = 0x80070000;                                    // Result = 80070000
    a3 += 0x7E08;                                       // Result = gPSXSND_soundSettingsLists[0] (80077E08)
    a0 = s1;
    wess_load_module();
    wess_get_master_status();
    a0 = v0;
    wess_dig_lcd_loader_init();
    wess_get_master_status();
    a0 = v0;
    a1 = 0xC9;                                          // Result = 000000C9
    a2 = 1;                                             // Result = 00000001
    wess_seq_loader_init();
    wess_get_wmd_end();
    a0 = 0;                                             // Result = 00000000
    a1 = 0x5A;                                          // Result = 0000005A
    a2 = v0;
    wess_seq_range_load();
    s0 = v0;
    wess_get_wmd_end();
    v0 += s0;
    sw(v0, gp + 0x83C);                                 // Store to: gpMusSequencesEnd (80077E1C)
    a0 = s3;
    S_SetSfxVolume();
    a0 = s4;
    S_SetMusicVolume();
    a0 = 0xC8;                                          // Result = 000000C8
    a1 = 0x1010;                                        // Result = 00001010
    a2 = s2;                                            // Result = gDoomSfxLoadedSamples[0] (8007EC9C)
    sw(0, gp + 0x840);                                  // Store to: gbDidLoadDoomSfxLcd (80077E20)
    a3 = 0;                                             // Result = 00000000
    wess_dig_lcd_load();
    v0 += 0x1010;
    sw(v0, gp + 0x838);                                 // Store to: gNextSoundUploadAddr (80077E18)
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x840);                                 // Store to: gbDidLoadDoomSfxLcd (80077E20)
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void PsxSoundExit() noexcept {
    return;
}

void trackstart() noexcept {
loc_80041734:
    v1 = lw(a0);
    v0 = v1 & 8;
    {
        const bool bJump = (v0 == 0)
        v0 = -9;                                        // Result = FFFFFFF7
        if (bJump) goto loc_80041770;
    }
    v0 &= v1;
    sw(v0, a0);
    v0 = lbu(a1 + 0x5);
    v0++;
    sb(v0, a1 + 0x5);
    v0 &= 0xFF;
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80041770;
    }
    sb(v0, a1 + 0x1);
loc_80041770:
    return;
}

void trackstop() noexcept {
loc_80041778:
    v1 = lw(a0);
    v0 = v1 & 8;
    {
        const bool bJump = (v0 != 0)
        v0 = v1 | 8;
        if (bJump) goto loc_800417B0;
    }
    sw(v0, a0);
    v0 = lbu(a1 + 0x5);
    v0--;
    sb(v0, a1 + 0x5);
    v0 &= 0xFF;
    if (v0 != 0) goto loc_800417B0;
    sb(0, a1 + 0x1);
loc_800417B0:
    return;
}

void queue_wess_seq_pause() noexcept {
    sp -= 0x48;
    sw(a0, sp + 0x10);
    a0 = lw(sp + 0x10);
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s6, sp + 0x38);
    sw(s5, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    sw(a1, sp + 0x18);
    Is_Seq_Num_Valid();
    if (v0 == 0) goto loc_8004192C;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s7 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s6 = lbu(v1 + 0xB);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s7 == 0) goto loc_80041920;
    s6--;
    {
        const bool bJump = (s6 == v0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80041924;
    }
    fp = -1;                                            // Result = FFFFFFFF
    s4 = s5 + 0xC;
loc_80041838:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_80041910;
    v0 = lh(s4 - 0xA);
    a2 = lw(sp + 0x10);
    if (v0 != a2) goto loc_80041904;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s3 = lbu(s4 - 0x8);
    s1 = lbu(v0 + 0x1C);
    s2 = lw(s4);
    s1--;
    if (s1 == fp) goto loc_80041904;
loc_80041880:
    a0 = lbu(s2);
    v0 = 0xFF;                                          // Result = 000000FF
    {
        const bool bJump = (a0 == v0)
        v0 = a0 << 2;
        if (bJump) goto loc_800418F8;
    }
    v0 += a0;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 <<= 4;
    v1 = lw(v1 + 0x28);
    a1 = s5;
    s0 = v0 + v1;
    a0 = s0;
    trackstop();
    a2 = lw(sp + 0x18);
    v0 = 1;                                             // Result = 00000001
    s3--;
    if (a2 != v0) goto loc_800418F0;
    v0 = lbu(s0 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    v0 <<= 2;
    v0 += a2;
    v0 = lw(v0);
    v0 = lw(v0 + 0x18);
    a0 = s0;
    pcall(v0);
loc_800418F0:
    if (s3 == 0) goto loc_80041904;
loc_800418F8:
    s1--;
    s2++;
    if (s1 != fp) goto loc_80041880;
loc_80041904:
    s7--;
    v0 = 1;                                             // Result = 00000001
    if (s7 == 0) goto loc_80041924;
loc_80041910:
    s4 += 0x18;
    s6--;
    s5 += 0x18;
    if (s6 != fp) goto loc_80041838;
loc_80041920:
    v0 = 1;                                             // Result = 00000001
loc_80041924:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_8004192C:
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

void queue_wess_seq_restart() noexcept {
    sp -= 0x40;
    sw(fp, sp + 0x38);
    fp = a0;
    sw(ra, sp + 0x3C);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    Is_Seq_Num_Valid();
    if (v0 == 0) goto loc_80041A98;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s4 = lw(v0 + 0x20);
    s5 = lbu(v1 + 0xB);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s6 == 0) goto loc_80041A8C;
    s5--;
    {
        const bool bJump = (s5 == v0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80041A90;
    }
    a2 = -1;                                            // Result = FFFFFFFF
    s3 = s4 + 0xC;
loc_800419D8:
    v0 = lw(s4);
    v0 &= 1;
    if (v0 == 0) goto loc_80041A7C;
    v0 = lh(s3 - 0xA);
    if (v0 != fp) goto loc_80041A70;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s0 = lbu(v0 + 0x1C);
    s1 = lw(s3);
    s0--;
    if (s0 == a2) goto loc_80041A70;
    s7 = -1;                                            // Result = FFFFFFFF
loc_80041A20:
    v1 = lbu(s1);
    a3 = 0xFF;                                          // Result = 000000FF
    a1 = s4;
    if (v1 == a3) goto loc_80041A64;
    s2--;
    a0 = v1 << 2;
    a0 += v1;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 <<= 4;
    v0 = lw(v0 + 0x28);
    sw(a2, sp + 0x10);
    a0 += v0;
    trackstart();
    a2 = lw(sp + 0x10);
    if (s2 == 0) goto loc_80041A70;
loc_80041A64:
    s0--;
    s1++;
    if (s0 != s7) goto loc_80041A20;
loc_80041A70:
    s6--;
    v0 = 1;                                             // Result = 00000001
    if (s6 == 0) goto loc_80041A90;
loc_80041A7C:
    s3 += 0x18;
    s5--;
    s4 += 0x18;
    if (s5 != a2) goto loc_800419D8;
loc_80041A8C:
    v0 = 1;                                             // Result = 00000001
loc_80041A90:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80041A98:
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

void queue_wess_seq_pauseall() noexcept {
loc_80041ACC:
    sp -= 0x40;
    sw(s0, sp + 0x18);
    s0 = a1;
    sw(ra, sp + 0x3C);
    sw(fp, sp + 0x38);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(a0, sp + 0x10);
    Is_Module_Loaded();
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80041C54;
    }
    a2 = lw(sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    if (a2 != v0) goto loc_80041B28;
    a0 = s0;
    start_record_music_mute();
loc_80041B28:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s7 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s4 = lw(v0 + 0x20);
    s6 = lbu(v1 + 0xB);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s7 == 0) goto loc_80041C34;
    s6--;
    if (s6 == v0) goto loc_80041C34;
    fp = -1;                                            // Result = FFFFFFFF
    s5 = s4 + 0xC;
loc_80041B60:
    v0 = lw(s4);
    v0 &= 1;
    if (v0 == 0) goto loc_80041C24;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s3 = lbu(s5 - 0x8);
    s1 = lbu(v0 + 0x1C);
    s2 = lw(s5);
    s1--;
    if (s1 == fp) goto loc_80041C18;
loc_80041B94:
    a0 = lbu(s2);
    v0 = 0xFF;                                          // Result = 000000FF
    {
        const bool bJump = (a0 == v0)
        v0 = a0 << 2;
        if (bJump) goto loc_80041C0C;
    }
    v0 += a0;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 <<= 4;
    v1 = lw(v1 + 0x28);
    a2 = lw(sp + 0x10);
    s0 = v0 + v1;
    v0 = 1;                                             // Result = 00000001
    s3--;
    if (a2 != v0) goto loc_80041BF8;
    v0 = lbu(s0 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    v0 <<= 2;
    v0 += a2;
    v0 = lw(v0);
    v0 = lw(v0 + 0x18);
    a0 = s0;
    pcall(v0);
loc_80041BF8:
    a0 = s0;
    a1 = s4;
    trackstop();
    if (s3 == 0) goto loc_80041C18;
loc_80041C0C:
    s1--;
    s2++;
    if (s1 != fp) goto loc_80041B94;
loc_80041C18:
    s7--;
    if (s7 == 0) goto loc_80041C34;
loc_80041C24:
    s5 += 0x18;
    s6--;
    s4 += 0x18;
    if (s6 != fp) goto loc_80041B60;
loc_80041C34:
    a2 = lw(sp + 0x10);
    s0 = 1;                                             // Result = 00000001
    if (a2 != s0) goto loc_80041C4C;
    end_record_music_mute();
loc_80041C4C:
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80041C54:
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

void queue_wess_seq_restartall() noexcept {
loc_80041C88:
    sp -= 0x58;
    sw(s5, sp + 0x44);
    s5 = a0;
    sw(ra, sp + 0x54);
    sw(fp, sp + 0x50);
    sw(s7, sp + 0x4C);
    sw(s6, sp + 0x48);
    sw(s4, sp + 0x40);
    sw(s3, sp + 0x3C);
    sw(s2, sp + 0x38);
    sw(s1, sp + 0x34);
    sw(s0, sp + 0x30);
    Is_Module_Loaded();
    if (v0 == 0) goto loc_80041E44;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    t1 = lbu(v0 + 0x4);
    sw(t1, sp + 0x18);
    v1 = lw(v0 + 0xC);
    fp = lw(v0 + 0x20);
    t0 = lbu(v1 + 0xB);
    v0 = -1;                                            // Result = FFFFFFFF
    if (t1 == 0) goto loc_80041E30;
    t0--;
    if (t0 == v0) goto loc_80041E30;
    s7 = fp + 2;
loc_80041D04:
    v0 = lw(fp);
    v0 &= 1;
    t1 = -1;                                            // Result = FFFFFFFF
    if (v0 == 0) goto loc_80041E1C;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s6 = lbu(s7 + 0x2);
    s2 = lbu(v0 + 0x1C);
    s4 = lw(s7 + 0xA);
    s2--;
    if (s2 == t1) goto loc_80041E08;
loc_80041D38:
    a0 = lbu(s4);
    v0 = 0xFF;                                          // Result = 000000FF
    a1 = fp;
    if (a0 == v0) goto loc_80041DF8;
    v0 = a0 << 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += a0;
    v1 = lw(v1 + 0x28);
    v0 <<= 4;
    sw(t0, sp + 0x28);
    s3 = v0 + v1;
    a0 = s3;
    trackstart();
    t0 = lw(sp + 0x28);
    if (s5 == 0) goto loc_80041DEC;
    v0 = lw(s5);
    s1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80041DEC;
    s0 = s5;
loc_80041D90:
    v1 = lbu(s4);
    v0 = lh(s0 + 0x6);
    s1++;
    if (v1 != v0) goto loc_80041DD8;
    v1 = lh(s7);
    v0 = lh(s0 + 0x4);
    a0 = s3;
    if (v1 != v0) goto loc_80041DD8;
    v0 = lbu(s0 + 0x9);
    a3 = lbu(s0 + 0x8);
    sw(v0, sp + 0x10);
    a1 = lw(s0 + 0xC);
    a2 = lw(s0 + 0x10);
    sw(t0, sp + 0x28);
    PSX_voicenote();
    t0 = lw(sp + 0x28);
loc_80041DD8:
    v0 = lw(s5);
    v0 = (i32(s1) < i32(v0));
    s0 += 0x10;
    if (v0 != 0) goto loc_80041D90;
loc_80041DEC:
    s6--;
    if (s6 == 0) goto loc_80041E08;
loc_80041DF8:
    s2--;
    t1 = -1;                                            // Result = FFFFFFFF
    s4++;
    if (s2 != t1) goto loc_80041D38;
loc_80041E08:
    t1 = lw(sp + 0x18);
    t1--;
    sw(t1, sp + 0x18);
    if (t1 == 0) goto loc_80041E30;
loc_80041E1C:
    s7 += 0x18;
    t0--;
    t1 = -1;                                            // Result = FFFFFFFF
    fp += 0x18;
    if (t0 != t1) goto loc_80041D04;
loc_80041E30:
    v0 = 1;                                             // Result = 00000001
    if (s5 == 0) goto loc_80041E3C;
    sw(0, s5);
loc_80041E3C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80041E44:
    ra = lw(sp + 0x54);
    fp = lw(sp + 0x50);
    s7 = lw(sp + 0x4C);
    s6 = lw(sp + 0x48);
    s5 = lw(sp + 0x44);
    s4 = lw(sp + 0x40);
    s3 = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x58;
    return;
}

void zeroset() noexcept {
loc_80041E78:
    sp -= 8;
    v0 = a1 - 1;
    if (a1 == 0) goto loc_80041E98;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80041E88:
    sb(0, a0);
    v0--;
    a0++;
    if (v0 != v1) goto loc_80041E88;
loc_80041E98:
    sp += 8;
    return;
}

void wess_install_error_handler() noexcept {
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5918);                                // Store to: gpWess_Error_func (80075918)
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x591C);                                // Store to: gpWess_Error_module (8007591C)
    return;
}

void wess_get_master_status() noexcept {
loc_80041EBC:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    return;
}

void Is_System_Active() noexcept {
loc_80041ECC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F4);                               // Load from: gbWess_sysinit (800758F4)
    v0 = (v0 > 0);
    return;
}

void Is_Module_Loaded() noexcept {
loc_80041EDC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    v0 = (v0 > 0);
    return;
}

void Is_Seq_Num_Valid() noexcept {
loc_80041EEC:
    v0 = 0;                                             // Result = 00000000
    if (i32(a0) < 0) goto loc_80041F40;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5900);                               // Load from: gWess_max_seq_num (80075900)
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 != 0)
        v0 = a0 << 2;
        if (bJump) goto loc_80041F14;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_80041F40;
loc_80041F14:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v1 = lw(v1 + 0xC);
    v0 += a0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    v0 = lw(v0 + 0x4);
    v0 = (v0 > 0);
loc_80041F40:
    return;
}

void Register_Early_Exit() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58FC);                               // Load from: gbWess_early_exit (800758FC)
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80041F64;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58FC);                                // Store to: gbWess_early_exit (800758FC)
loc_80041F64:
    return;
}

void wess_install_handler() noexcept {
loc_80041F6C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    init_WessTimer();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_restore_handler() noexcept {
loc_80041F8C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    exit_WessTimer();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_init() noexcept {
loc_80041FAC:
    sp -= 0x18;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F4);                               // Load from: gbWess_sysinit (800758F4)
    v1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x10);
    if (v0 != 0) goto loc_80041FFC;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x594C);                               // Load from: gbWess_WessTimerActive (8007594C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    if (v0 != 0) goto loc_80041FE4;
    wess_install_handler();
loc_80041FE4:
    wess_low_level_init();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58F4);                                // Store to: gbWess_sysinit (800758F4)
    v1 = 1;                                             // Result = 00000001
loc_80041FFC:
    v0 = v1;
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_exit() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    Is_System_Active();
    if (v0 == 0) goto loc_80042088;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F4);                               // Load from: gbWess_sysinit (800758F4)
    if (v0 == 0) goto loc_80042088;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    if (v0 == 0) goto loc_8004205C;
    wess_unload_module();
loc_8004205C:
    wess_low_level_exit();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x594C);                               // Load from: gbWess_WessTimerActive (8007594C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x58F4);                                 // Store to: gbWess_sysinit (800758F4)
    v0 |= s0;
    if (v0 == 0) goto loc_80042088;
    wess_restore_handler();
loc_80042088:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_get_wmd_start() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x590C);                               // Load from: gpWess_wmd_mem (8007590C)
    return;
}

void wess_get_wmd_end() noexcept {
loc_800420AC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5910);                               // Load from: gpWess_wmd_end (80075910)
    return;
}

void free_mem_if_mine() noexcept {
loc_800420BC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5908);                               // Load from: gbWess_wmd_mem_is_mine (80075908)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_800420FC;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x590C);                               // Load from: gpWess_wmd_mem (8007590C)
    if (a0 == 0) goto loc_800420F4;
    wess_free();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x590C);                                 // Store to: gpWess_wmd_mem (8007590C)
loc_800420F4:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5908);                                 // Store to: gbWess_wmd_mem_is_mine (80075908)
loc_800420FC:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_unload_module() noexcept {
loc_8004210C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    sp -= 0x30;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    if (v0 == 0) goto loc_800421F8;
    s0 = 0;                                             // Result = 00000000
    wess_seq_stopall();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5920);                               // Load from: gWess_CmdFuncArr[0] (80075920)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0 + 0x4);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    pcall(v0);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(v0 + 0x8);
    if (s2 == 0) goto loc_800421E8;
    s3 = 0x80070000;                                    // Result = 80070000
    s3 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    s1 = 0;                                             // Result = 00000000
loc_80042184:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a0 + 0x18);
    v1 = s1 + v0;
    v0 = lw(v1 + 0x50);
    v0 &= 7;
    s1 += 0x54;
    if (v0 == 0) goto loc_800421D8;
    v0 = lw(v1 + 0x4C);
    v0 <<= 2;
    v0 += s3;
    v0 = lw(v0);
    v0 = lw(v0 + 0x4);
    pcall(v0);
loc_800421D8:
    s0++;
    v0 = (i32(s0) < i32(s2));
    if (v0 != 0) goto loc_80042184;
loc_800421E8:
    free_mem_if_mine();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x58F8);                                 // Store to: gbWess_module_loaded (800758F8)
loc_800421F8:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void wess_memcpy() noexcept {
loc_80042218:
    sp -= 8;
    v1 = a2 - 1;
    if (a2 == 0) goto loc_80042240;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80042228:
    v0 = lbu(a1);
    a1++;
    v1--;
    sb(v0, a0);
    a0++;
    if (v1 != a2) goto loc_80042228;
loc_80042240:
    sp += 8;
    return;
}

void conditional_read() noexcept {
loc_8004224C:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s0, sp + 0x10);
    s0 = a2;
    sw(ra, sp + 0x18);
    if (a0 == 0) goto loc_800422B8;
    a0 = lw(s1);
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    wess_memcpy();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 += s0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 = lw(s1);
    v0 += s0;
    v1 = v0 & 1;
    v1 += v0;
    v0 = v1 & 2;
    v0 += v1;
    sw(v0, s1);
    goto loc_800422D0;
loc_800422B8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 += s0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
loc_800422D0:
    v0 = 1;                                             // Result = 00000001
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_load_module() noexcept {
loc_800422EC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    sp -= 0x80;
    sw(s2, sp + 0x60);
    s2 = a0;
    sw(s1, sp + 0x5C);
    s1 = a1;
    sw(s0, sp + 0x58);
    s0 = a2;
    sw(s3, sp + 0x64);
    sw(ra, sp + 0x7C);
    sw(fp, sp + 0x78);
    sw(s7, sp + 0x74);
    sw(s6, sp + 0x70);
    sw(s5, sp + 0x6C);
    sw(s4, sp + 0x68);
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5904);                                // Store to: gWess_mem_limit (80075904)
    s3 = a3;
    if (v0 == 0) goto loc_80042344;
    wess_unload_module();
loc_80042344:
    a0 = s3;
    get_num_Wess_Sound_Drivers();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E4);                                // Store to: gWess_num_sd (800758E4)
    v0 = 1;                                             // Result = 00000001
    if (s1 != 0) goto loc_8004238C;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5908);                                // Store to: gbWess_wmd_mem_is_mine (80075908)
    a0 = s0;
    wess_malloc();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x590C);                                // Store to: gpWess_wmd_mem (8007590C)
    if (v0 != 0) goto loc_8004239C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    goto loc_80043090;
loc_8004238C:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5908);                                 // Store to: gbWess_wmd_mem_is_mine (80075908)
    at = 0x80070000;                                    // Result = 80070000
    sw(s1, at + 0x590C);                                // Store to: gpWess_wmd_mem (8007590C)
loc_8004239C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x590C);                               // Load from: gpWess_wmd_mem (8007590C)
    a1 = s0;
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x5914);                                // Store to: gWess_wmd_size (80075914)
    zeroset();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5900);                                 // Store to: gWess_max_seq_num (80075900)
    Is_System_Active();
    if (v0 == 0) goto loc_800423D8;
    t3 = 4;                                             // Result = 00000004
    if (s2 != 0) goto loc_800423F0;
loc_800423D8:
    free_mem_if_mine();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    goto loc_80043090;
loc_800423F0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x590C);                               // Load from: gpWess_wmd_mem (8007590C)
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x58F0);                               // Load from: gpWess_fp_wmd_file (800758F0)
    a1 = s2;
    sb(t3, sp + 0x18);
    sb(0, sp + 0x20);
    v1 = v0 + 0x38;
    sw(v1, v0 + 0xC);
    a0 = lw(v0 + 0xC);
    a2 = 0x10;                                          // Result = 00000010
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x58EC);                                // Store to: gpWess_tmp_fp_wmd_file_2 (800758EC)
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    sw(v0, sp + 0x10);
    sw(v1, sp + 0x10);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x5954;                                       // Result = gWess_Millicount (80075954)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x78A8);                                // Store to: gpWess_pm_stat (800A8758)
    sw(a3, v0 + 0x34);
    sw(v1, v0);
    v0 += 0x4C;
    sw(v0, sp + 0x10);
    wess_memcpy();
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    sb(0, sp + 0x28);
    a2 = lw(a1 + 0xC);
    v1 = 0x80010000;                                    // Result = 80010000
    v1 = lw(v1 + 0x1760);                               // Load from: STR_SPSX[0] (80011760)
    a0 = lw(a2);
    v0 += 0x10;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 = 1;                                             // Result = 00000001
    if (a0 != v1) goto loc_800424A0;
    v1 = lw(a2 + 0x4);
    if (v1 == v0) goto loc_800424B0;
loc_800424A0:
    free_mem_if_mine();
loc_800424A8:
    v0 = 0;                                             // Result = 00000000
    goto loc_80043090;
loc_800424B0:
    v1 = lw(sp + 0x10);
    v0 = lw(a1 + 0xC);
    sw(v1, a1 + 0x20);
    a0 = lbu(v0 + 0xB);
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    a0 = lw(a1 + 0xC);
    v1 += v0;
    sw(v1, sp + 0x10);
    sw(v1, a1 + 0x28);
    a0 = lbu(a0 + 0xC);
    v0 = a0 << 2;
    v0 += a0;
    v0 <<= 4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lbu(a0 + 0x58E4);                              // Load from: gWess_num_sd (800758E4)
    v1 += v0;
    sw(v1, sp + 0x10);
    sb(a0, a1 + 0x8);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(sp + 0x10);
    v1 = lbu(a0 + 0x8);
    sw(v0, a0 + 0x14);
    v0 += v1;
    v1 = v0 & 1;
    v0 += v1;
    v1 = v0 & 2;
    v0 += v1;
    sw(v0, sp + 0x10);
    v0 = -1;                                            // Result = FFFFFFFF
    v1 = lbu(a0 + 0x8);
    a1 = lw(a0 + 0x14);
    v1--;
    a0 = 0x80;                                          // Result = 00000080
    if (v1 == v0) goto loc_8004255C;
loc_8004254C:
    sb(a0, a1);
    v1--;
    a1++;
    if (v1 != v0) goto loc_8004254C;
loc_8004255C:
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 = lw(a2 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 = lw(sp + 0x10);
    v1 = lbu(a2 + 0x8);
    sw(a0, a2 + 0x18);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 += v1;
    v0 <<= 2;
    a0 += v0;
    sw(a0, sp + 0x10);
    if (s3 == 0) goto loc_80042708;
    a1 = lbu(a2 + 0x8);
    v0 = -1;                                            // Result = FFFFFFFF
    a1--;
    {
        const bool bJump = (a1 == v0)
        v0 = a1 << 2;
        if (bJump) goto loc_80042708;
    }
    t2 = a2;
    t1 = v0 + s3;
    v0 += a1;
    v0 <<= 2;
    v0 += a1;
    t0 = v0 << 2;
loc_800425BC:
    v0 = lw(t1);
    v0 = lw(v0);
    if (v0 == 0) goto loc_800426F4;
    a3 = 0;                                             // Result = 00000000
loc_800425D8:
    v0 = lw(t2 + 0x18);
    v1 = lw(t1);
    v0 += t0;
    v1 += a3;
    v1 = lw(v1);
    v0 += a3;
    sw(v1, v0 + 0x24);
    v0 = lw(t2 + 0x18);
    v1 = lw(t1);
    v0 += t0;
    v1 += a3;
    v1 = lw(v1 + 0x4);
    v0 += a3;
    sw(v1, v0 + 0x28);
    v0 = lw(t2 + 0x18);
    a2 = t0 + v0;
    a0 = a3 + a2;
    v1 = lw(a0 + 0x24);
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0)
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8004263C;
    }
    v0 = lw(a0 + 0x28);
    sw(v0, a2 + 0x4C);
    goto loc_800426D0;
loc_8004263C:
    if (v1 != v0) goto loc_80042664;
    v1 = -2;                                            // Result = FFFFFFFE
    v0 = lw(a2 + 0x50);
    a0 = lw(a0 + 0x28);
    v1 &= v0;
    v0 &= 1;
    v0 |= a0;
    v0 &= 1;
    goto loc_800426C8;
loc_80042664:
    v0 = 3;                                             // Result = 00000003
    if (v1 != v0) goto loc_80042698;
    v1 = -3;                                            // Result = FFFFFFFD
    v0 = lw(a2 + 0x50);
    a0 = lw(a0 + 0x28);
    v1 &= v0;
    v0 >>= 1;
    v0 &= 1;
    v0 |= a0;
    v0 &= 1;
    v0 <<= 1;
    goto loc_800426C8;
loc_80042698:
    v0 = 4;                                             // Result = 00000004
    {
        const bool bJump = (v1 != v0)
        v0 = a1 << 2;
        if (bJump) goto loc_800426D4;
    }
    v1 = -5;                                            // Result = FFFFFFFB
    v0 = lw(a2 + 0x50);
    a0 = lw(a0 + 0x28);
    v1 &= v0;
    v0 >>= 2;
    v0 &= 1;
    v0 |= a0;
    v0 &= 1;
    v0 <<= 2;
loc_800426C8:
    v1 |= v0;
    sw(v1, a2 + 0x50);
loc_800426D0:
    v0 = a1 << 2;
loc_800426D4:
    v0 += s3;
    v0 = lw(v0);
    a3 += 8;
    v0 += a3;
    v0 = lw(v0);
    if (v0 != 0) goto loc_800425D8;
loc_800426F4:
    t1 -= 4;
    a1--;
    v0 = -1;                                            // Result = FFFFFFFF
    t0 -= 0x54;
    if (a1 != v0) goto loc_800425BC;
loc_80042708:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    sb(0, v0 + 0x7);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0 + 0xC);
    s1 = lbu(v0 + 0xA);
    v0 = -1;                                            // Result = FFFFFFFF
    s1--;
    if (s1 == v0) goto loc_80042934;
    s2 = -1;                                            // Result = FFFFFFFF
    s0 = 0x80080000;                                    // Result = 80080000
    s0 -= 0x1038;                                       // Result = 8007EFC8
    s3 = s0 - 4;                                        // Result = 8007EFC4
loc_80042750:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x103C;                                       // Result = 8007EFC4
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    a2 = 0x1C;                                          // Result = 0000001C
    wess_memcpy();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += 0x1C;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    a1 = lbu(v1 + 0x8);
    a1--;
    v0 = a1 << 2;
    if (a1 == s2) goto loc_80042928;
    v0 += a1;
    v0 <<= 2;
    v0 += a1;
    a3 = v0 << 2;
loc_800427A8:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0 + 0x18);
    v1 = lbu(s0);                                       // Load from: 8007EFC8
    a2 = a3 + v0;
    v0 = lw(a2 + 0x4C);
    a1--;
    if (v1 != v0) goto loc_80042920;
    v0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    v1 = lw(s0);                                        // Load from: 8007EFC8
    a0 = lw(s0 + 0x4);                                  // Load from: 8007EFCC
    a1 = lw(s0 + 0x8);                                  // Load from: 8007EFD0
    sw(v0, a2);
    sw(v1, a2 + 0x4);
    sw(a0, a2 + 0x8);
    sw(a1, a2 + 0xC);
    v0 = lw(s0 + 0xC);                                  // Load from: 8007EFD4
    v1 = lw(s0 + 0x10);                                 // Load from: 8007EFD8
    a0 = lw(s0 + 0x14);                                 // Load from: 8007EFDC
    sw(v0, a2 + 0x10);
    sw(v1, a2 + 0x14);
    sw(a0, a2 + 0x18);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v1 = lbu(s0 + 0x1);                                 // Load from: 8007EFC9
    v0 = lbu(a0 + 0x7);
    v0 += v1;
    sb(v0, a0 + 0x7);
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v1 = lw(sp + 0x10);
    v0 = lw(a1 + 0x18);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 += a3;
    sw(v1, v0 + 0x1C);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x58EC);                               // Load from: gpWess_tmp_fp_wmd_file_2 (800758EC)
    v0 = lw(a1 + 0x18);
    s6 = a0 - v1;
    v0 += a3;
    sw(s6, v0 + 0x20);
    v1 = lh(s0 + 0x4);                                  // Load from: 8007EFCC
    v0 = lh(s0 + 0x6);                                  // Load from: 8007EFCE
    mult(v1, v0);
    a0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    a1 = sp + 0x10;
    a2 = lo;
    a0 &= 1;
    conditional_read();
    a1 = sp + 0x10;
    if (v0 == 0) goto loc_800424A8;
    v0 = lh(s0 + 0x8);                                  // Load from: 8007EFD0
    v1 = lh(s0 + 0xA);                                  // Load from: 8007EFD2
    mult(v0, v1);
    a0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    a2 = lo;
    a0 &= 2;
    conditional_read();
    a1 = sp + 0x10;
    if (v0 == 0) goto loc_800424A8;
    v0 = lh(s0 + 0xC);                                  // Load from: 8007EFD4
    v1 = lh(s0 + 0xE);                                  // Load from: 8007EFD6
    mult(v0, v1);
    a0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    a2 = lo;
    a0 &= 4;
    conditional_read();
    a1 = sp + 0x10;
    if (v0 == 0) goto loc_800424A8;
    v0 = lh(s0 + 0x10);                                 // Load from: 8007EFD8
    v1 = lh(s0 + 0x12);                                 // Load from: 8007EFDA
    mult(v0, v1);
    a0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    a2 = lo;
    a0 &= 8;
    conditional_read();
    a1 = sp + 0x10;
    if (v0 == 0) goto loc_800424A8;
    a0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    a2 = lw(s3 + 0x18);                                 // Load from: 8007EFDC
    a0 &= 0x10;
    conditional_read();
    s1--;
    if (v0 != 0) goto loc_8004292C;
    v0 = 0;                                             // Result = 00000000
    goto loc_80043090;
loc_80042920:
    a3 -= 0x54;
    if (a1 != s2) goto loc_800427A8;
loc_80042928:
    s1--;
loc_8004292C:
    if (s1 != s2) goto loc_80042750;
loc_80042934:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lbu(a0 + 0x7);
    v1 = v0 << 1;
    v1 += v0;
    v0 = lw(sp + 0x10);
    v1 <<= 3;
    v1 += v0;
    sw(v1, sp + 0x10);
    v1 = lbu(a0 + 0x8);
    sw(v0, a0 + 0x30);
    if (v1 == 0) goto loc_80042A78;
    t1 = v1;
    a3 = 0;                                             // Result = 00000000
    v0 = lw(a0 + 0x18);
    v1 = lbu(a0 + 0x7);
    a1 = lbu(v0 + 0x5);
    s2 = 0;                                             // Result = 00000000
    if (i32(v1) <= 0) goto loc_80042A78;
    t2 = -1;                                            // Result = FFFFFFFF
    a2 = 0;                                             // Result = 00000000
    t0 = 0;                                             // Result = 00000000
loc_80042998:
    if (t1 == 0) goto loc_80042A58;
    a1--;
    if (a1 == t2) goto loc_800429F4;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v1 + 0x18);
    v1 = lw(v1 + 0x30);
    v0 += t0;
    v0 = lbu(v0 + 0x4);
    v1 += a2;
    sb(v0, v1 + 0x1);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0 + 0x30);
    v0 += a2;
    sb(a3, v0 + 0x2);
    a3++;                                               // Result = 00000001
    goto loc_80042A58;
loc_800429F4:
    t1--;
    t0 += 0x54;                                         // Result = 00000054
    if (t1 == 0) goto loc_80042A58;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a0 + 0x18);
    v1 = t0 + v0;
    a1 = lbu(v1 + 0x5);
    a1--;
    a3 = 0;                                             // Result = 00000000
    if (a1 == t2) goto loc_80042A58;
    v0 = lw(a0 + 0x30);
    v1 = lbu(v1 + 0x4);
    v0 += a2;
    sb(v1, v0 + 0x1);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0 + 0x30);
    a3 = 1;                                             // Result = 00000001
    v0 += a2;
    sb(0, v0 + 0x2);
loc_80042A58:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lbu(v0 + 0x7);
    s2++;
    v0 = (i32(s2) < i32(v0));
    a2 += 0x18;
    if (v0 != 0) goto loc_80042998;
loc_80042A78:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a1 = lw(sp + 0x10);
    v0 = lw(a0 + 0xC);
    sw(a1, v0 + 0x10);
    v0 = lw(a0 + 0xC);
    v1 = lh(v0 + 0x8);
    s2 = 0;                                             // Result = 00000000
    v0 = v1 << 2;
    v0 += v1;
    v1 = lw(a0 + 0xC);
    v0 <<= 2;
    v1 = lh(v1 + 0x8);
    v0 += a1;
    sw(v0, sp + 0x10);
    if (i32(v1) <= 0) goto loc_80042DB0;
    s7 = -1;                                            // Result = FFFFFFFF
    fp = 0x80080000;                                    // Result = 80080000
    fp -= 0x1020;                                       // Result = 8007EFE0
    s4 = fp + 0x12;                                     // Result = 8007EFF2
    s5 = 0;                                             // Result = 00000000
loc_80042AD4:
    a2 = 4;                                             // Result = 00000004
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 = lw(v0 + 0xC);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x58EC);                               // Load from: gpWess_tmp_fp_wmd_file_2 (800758EC)
    a0 = lw(v0 + 0x10);
    s6 = a1 - v1;
    a0 += s5;
    wess_memcpy();
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s3 = 0;                                             // Result = 00000000
    v0 = lw(v0 + 0xC);
    v1 = lw(v0 + 0x10);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v1 += s5;
    s1 = lh(v1);
    v0 += 4;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    s1--;
    s0 = 0;                                             // Result = 00000000
    if (s1 == s7) goto loc_80042D1C;
    a0 = fp;                                            // Result = 8007EFE0
loc_80042B48:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    a2 = 0x18;                                          // Result = 00000018
    wess_memcpy();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v1 = lbu(fp);                                       // Load from: 8007EFE0
    v0 += 0x18;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    t0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80042C60;
    v0 = 0x32;                                          // Result = 00000032
    if (v1 == v0) goto loc_80042C60;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a1 = lbu(v0 + 0x8);
    a1--;
    if (a1 == s7) goto loc_80042C78;
    t1 = v1;
    a3 = v0;
    v1 = lw(v0 + 0x18);
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    v0 += a1;
    a0 = v0 << 2;
    a2 = a0 + v1;
loc_80042BC8:
    v0 = lw(a2 + 0x4C);
    if (t1 != v0) goto loc_80042C68;
    v1 = lbu(fp + 0x4);                                 // Load from: 8007EFE4
    v0 = 3;                                             // Result = 00000003
    if (v1 == 0) goto loc_80042BF0;
    if (v1 != v0) goto loc_80042C04;
loc_80042BF0:
    v0 = lw(a2 + 0x50);
    v0 &= 1;
    if (v0 != 0) goto loc_80042C60;
loc_80042C04:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0x101C);                              // Load from: 8007EFE4
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0)
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80042C38;
    }
    v0 = lw(a3 + 0x18);
    v0 += a0;
    v0 = lw(v0 + 0x50);
    v0 &= 2;
    {
        const bool bJump = (v0 != 0)
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80042C60;
    }
loc_80042C38:
    if (v1 != v0) goto loc_80042C68;
    v0 = lw(a3 + 0x18);
    v0 += a0;
    v0 = lw(v0 + 0x50);
    v0 &= 4;
    if (v0 == 0) goto loc_80042C68;
loc_80042C60:
    t0 = 1;                                             // Result = 00000001
    goto loc_80042C78;
loc_80042C68:
    a0 -= 0x54;
    a1--;
    a2 -= 0x54;
    if (a1 != s7) goto loc_80042BC8;
loc_80042C78:
    if (t0 == 0) goto loc_80042CEC;
    v0 = lh(s4);                                        // Load from: 8007EFF2
    v1 = lw(s4 + 0x2);                                  // Load from: 8007EFF4
    t3 = lbu(sp + 0x20);
    v0 <<= 2;
    v1 += 0x20;
    v0 += v1;
    s0 += v0;
    v0 = s0 & 1;
    s0 += v0;
    v1 = s0 & 2;
    v0 = lbu(s4 - 0x11);                                // Load from: 8007EFE1
    v0 = (t3 < v0);
    s0 += v1;
    if (v0 == 0) goto loc_80042CC8;
    t3 = lbu(s4 - 0x11);                                // Load from: 8007EFE1
    sb(t3, sp + 0x20);
loc_80042CC8:
    v0 = lbu(s4 - 0x6);                                 // Load from: 8007EFEC
    t3 = lbu(sp + 0x28);
    v0 = (t3 < v0);
    s3++;                                               // Result = 00000001
    if (v0 == 0) goto loc_80042CEC;
    t3 = lbu(s4 - 0x6);                                 // Load from: 8007EFEC
    sb(t3, sp + 0x28);
loc_80042CEC:
    s1--;
    v0 = lh(s4);                                        // Load from: 8007EFF2
    v1 = lw(s4 + 0x2);                                  // Load from: 8007EFF4
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 <<= 2;
    v0 += v1;
    v0 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    a0 = fp;                                            // Result = 8007EFE0
    if (s1 != s7) goto loc_80042B48;
loc_80042D1C:
    v0 = lbu(sp + 0x18);
    v0 = (i32(v0) < i32(s3));
    if (v0 == 0) goto loc_80042D34;
    sb(s3, sp + 0x18);
loc_80042D34:
    s2++;
    if (s3 != 0) goto loc_80042D40;
    s0 = 0x24;                                          // Result = 00000024
loc_80042D40:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s5;
    sw(s3, v0 + 0x10);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s5;
    sw(s0, v0 + 0xC);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s5;
    sw(s6, v0 + 0x8);
    v0 = lw(v1 + 0xC);
    v0 = lh(v0 + 0x8);
    v0 = (i32(s2) < i32(v0));
    s5 += 0x14;
    if (v0 != 0) goto loc_80042AD4;
loc_80042DB0:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 = lw(sp + 0x10);
    v0 = lw(v1 + 0xC);
    sw(a0, v1 + 0x10);
    v0 = lbu(v0 + 0xF);
    t3 = lbu(sp + 0x18);
    v0 <<= 3;
    v0 += a0;
    sw(v0, sp + 0x10);
    sb(t3, v1 + 0x1C);
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 = lw(a2 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a2 + 0xC);
    v0 = lbu(v0 + 0xB);
    s2 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80042EE0;
    t0 = 0xFF;                                          // Result = 000000FF
    a3 = 0;                                             // Result = 00000000
loc_80042E08:
    v0 = lw(a2 + 0x20);
    v1 = lw(sp + 0x10);
    v0 += a3;
    sw(v1, v0 + 0x10);
    v0 = lw(a2 + 0xC);
    v0 = lbu(v0 + 0xD);
    v0 += v1;
    v1 = v0 & 1;
    v1 += v0;
    a0 = v1 & 2;
    v0 = lw(a2 + 0x20);
    a0 += v1;
    v0 += a3;
    sw(a0, v0 + 0x14);
    v0 = lw(a2 + 0xC);
    sw(a0, sp + 0x10);
    v0 = lbu(v0 + 0xE);
    s1 = lbu(sp + 0x18);
    v0 += a0;
    v1 = v0 & 1;
    v1 += v0;
    v0 = v1 & 2;
    v0 += v1;
    a1 = v0;
    a0 = s1 + a1;
    s1--;
    v1 = a0 & 1;
    v0 = lw(a2 + 0x20);
    v1 += a0;
    sw(a1, sp + 0x10);
    v0 += a3;
    sw(a1, v0 + 0xC);
    v0 = v1 & 2;
    v0 += v1;
    sw(v0, sp + 0x10);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s1 == v0) goto loc_80042EB8;
loc_80042EA8:
    sb(t0, a1);
    s1--;
    a1++;
    if (s1 != v0) goto loc_80042EA8;
loc_80042EB8:
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 = lw(a2 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a2 + 0xC);
    v0 = lbu(v0 + 0xB);
    s2++;
    v0 = (i32(s2) < i32(v0));
    a3 += 0x18;
    if (v0 != 0) goto loc_80042E08;
loc_80042EE0:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    t3 = lbu(sp + 0x20);
    sb(t3, v0 + 0x2C);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    t3 = lbu(sp + 0x28);
    sb(t3, v0 + 0x24);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a0 + 0xC);
    v0 = lbu(v0 + 0xC);
    s2 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80042F94;
    a2 = 0;                                             // Result = 00000000
loc_80042F30:
    v0 = lw(a0 + 0x28);
    v0 += a2;
    sb(s2, v0 + 0x1);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a0 + 0x28);
    a1 = lw(sp + 0x10);
    v0 += a2;
    sw(a1, v0 + 0x3C);
    v1 = lbu(a0 + 0x24);
    v0 = lw(a0 + 0x28);
    v1 <<= 2;
    v1 += a1;
    v0 += a2;
    sw(v1, v0 + 0x44);
    v0 = lw(a0 + 0xC);
    s2++;
    sw(v1, sp + 0x10);
    v0 = lbu(v0 + 0xC);
    v0 = (i32(s2) < i32(v0));
    a2 += 0x50;
    if (v0 != 0) goto loc_80042F30;
loc_80042F94:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5920);                               // Load from: gWess_CmdFuncArr[0] (80075920)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0);
    s2 = 0;                                             // Result = 00000000
    pcall(v0);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lbu(a0 + 0x8);
    {
        const bool bJump = (i32(v0) <= 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80043044;
    }
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    s0 = 0;                                             // Result = 00000000
loc_80042FDC:
    v0 = lw(a0 + 0x18);
    v1 = s0 + v0;
    v0 = lw(v1 + 0x50);
    v0 &= 7;
    s0 += 0x54;
    if (v0 == 0) goto loc_80043024;
    v0 = lw(v1 + 0x4C);
    v0 <<= 2;
    v0 += s1;
    v0 = lw(v0);
    v0 = lw(v0);
    pcall(v0);
loc_80043024:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lbu(a0 + 0x8);
    s2++;
    v0 = (i32(s2) < i32(v0));
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80042FDC;
    }
loc_80043044:
    a1 = lw(sp + 0x10);
    v1 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x58F8);                                // Store to: gbWess_module_loaded (800758F8)
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 = a1 & 1;
    a0 += a1;
    a1 = lw(v1 + 0xC);
    v1 = a0 & 2;
    a1 = lh(a1 + 0x8);
    v1 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5910);                                // Store to: gpWess_wmd_end (80075910)
    sw(v1, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x5900);                                // Store to: gWess_max_seq_num (80075900)
loc_80043090:
    ra = lw(sp + 0x7C);
    fp = lw(sp + 0x78);
    s7 = lw(sp + 0x74);
    s6 = lw(sp + 0x70);
    s5 = lw(sp + 0x6C);
    s4 = lw(sp + 0x68);
    s3 = lw(sp + 0x64);
    s2 = lw(sp + 0x60);
    s1 = lw(sp + 0x5C);
    s0 = lw(sp + 0x58);
    sp += 0x80;
    return;
}

void filltrackstat() noexcept {
loc_800430C4:
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s1, sp + 0x14);
    v0 = lw(s0);
    v1 = -0x11;                                         // Result = FFFFFFEF
    v0 |= 1;
    v0 &= v1;
    v1 = -0x21;                                         // Result = FFFFFFDF
    v0 &= v1;
    v1 = -0x41;                                         // Result = FFFFFFBF
    v0 &= v1;
    v1 = -0x81;                                         // Result = FFFFFF7F
    v0 &= v1;
    sw(v0, s0);
    v0 = lbu(s2);
    sb(v0, s0 + 0x3);
    v0 = lbu(s2 + 0x2);
    sb(v0, s0 + 0x8);
    v0 = lbu(s2 + 0x4);
    sb(0, s0 + 0x10);
    sb(v0, s0 + 0x13);
    v0 = lbu(s2 + 0x1);
    sb(v0, s0 + 0x11);
    v1 = lhu(s2 + 0xE);
    v0 = lw(s0 + 0x3C);
    sw(0, s0 + 0x20);
    sw(0, s0 + 0x24);
    sw(0, s0 + 0x28);
    sw(v0, s0 + 0x40);
    sh(v1, s0 + 0x14);
    v0 = lhu(s2 + 0x12);
    sh(v0, s0 + 0x18);
    v0 = lw(s2 + 0x14);
    sw(v0, s0 + 0x48);
    v0 = lbu(s2 + 0xD);
    s3 = a2;
    sb(v0, s0 + 0x12);
    if (s3 == 0) goto loc_80043194;
    v0 = lw(s3);
    s1 = v0;
    if (v0 != 0) goto loc_80043198;
loc_80043194:
    s1 = 0;                                             // Result = 00000000
loc_80043198:
    v0 = s1 & 1;
    if (v0 == 0) goto loc_800431B0;
    v0 = lbu(s3 + 0x4);
    sb(v0, s0 + 0xC);
    goto loc_800431BC;
loc_800431B0:
    v0 = lbu(s2 + 0xA);
    sb(v0, s0 + 0xC);
loc_800431BC:
    v0 = s1 & 2;
    if (v0 == 0) goto loc_800431D4;
    v0 = lbu(s3 + 0x5);
    sb(v0, s0 + 0xD);
    goto loc_800431E0;
loc_800431D4:
    v0 = lbu(s2 + 0xB);
    sb(v0, s0 + 0xD);
loc_800431E0:
    v0 = s1 & 4;
    if (v0 == 0) goto loc_800431F8;
    v0 = lhu(s3 + 0x6);
    sh(v0, s0 + 0xA);
    goto loc_80043204;
loc_800431F8:
    v0 = lhu(s2 + 0x6);
    sh(v0, s0 + 0xA);
loc_80043204:
    v0 = s1 & 8;                                        // Result = 00000000
    if (v0 == 0) goto loc_8004321C;
    v0 = lhu(s3 + 0x8);
    sh(v0, s0 + 0xE);
    goto loc_80043228;
loc_8004321C:
    v0 = lhu(s2 + 0x8);
    sh(v0, s0 + 0xE);
loc_80043228:
    v0 = s1 & 0x10;                                     // Result = 00000000
    if (v0 == 0) goto loc_8004325C;
    v0 = lbu(s0 + 0x12);
    v1 = lbu(s3 + 0xA);
    v0 = i32(v0) >> v1;
    v0 &= 1;
    if (v0 == 0) goto loc_8004325C;
    v0 = lw(s0);
    v0 |= 2;
    goto loc_80043268;
loc_8004325C:
    v0 = lw(s0);
    v1 = -3;                                            // Result = FFFFFFFD
    v0 &= v1;
loc_80043268:
    sw(v0, s0);
    v0 = s1 & 0x20;                                     // Result = 00000000
    if (v0 == 0) goto loc_80043284;
    v0 = lhu(s3 + 0xC);
    goto loc_80043288;
loc_80043284:
    v0 = lhu(s2 + 0x10);
loc_80043288:
    sh(v0, s0 + 0x16);
    GetIntsPerSec();
    v0 <<= 16;
    a1 = lh(s0 + 0x14);
    a2 = lh(s0 + 0x16);
    a0 = u32(i32(v0) >> 16);
    CalcPartsPerInt();
    sw(v0, s0 + 0x1C);
    v0 = s1 & 0x40;                                     // Result = 00000000
    v1 = -0x11;                                         // Result = FFFFFFEF
    if (v0 == 0) goto loc_800432D4;
    v0 = lw(s0 + 0x28);
    a0 = lw(s3 + 0x10);
    v1 = lw(s0);
    v0 += a0;
    v1 |= 0x10;
    sw(v0, s0 + 0x2C);
    sw(v1, s0);
    goto loc_800432E4;
loc_800432D4:
    v0 = lw(s0);
    v0 &= v1;
    sw(v0, s0);
loc_800432E4:
    v0 = s1 & 0x80;                                     // Result = 00000000
    v1 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_800432FC;
    v0 = lw(s0);
    v0 |= 0x20;
    goto loc_80043308;
loc_800432FC:
    v0 = lw(s0);
    v0 &= v1;
loc_80043308:
    sw(v0, s0);
    v0 = s1 & 0x100;                                    // Result = 00000000
    if (v0 == 0) goto loc_80043324;
    v0 = lbu(s3 + 0xB);
    sb(v0, s0 + 0x9);
    goto loc_80043330;
loc_80043324:
    v0 = lbu(s2 + 0x5);
    sb(v0, s0 + 0x9);
loc_80043330:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void assigntrackstat() noexcept {
loc_80043350:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x14);
    s0 = a0;
    sw(v0, s0 + 0x4C);
    v0 = lhu(s1 + 0x12);
    sh(v0, s0 + 0x1A);
    a0 = lw(s1 + 0x1C);
    a1 = s0 + 4;
    sw(a0, s0 + 0x30);
    Read_Vlq();
    sw(v0, s0 + 0x34);
    v0 = lw(s1 + 0x18);
    sw(v0, s0 + 0x38);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_seq_structrig() noexcept {
loc_800433B4:
    sp -= 0x58;
    sw(a0, sp + 0x10);
    sw(a1, sp + 0x18);
    a0 = lw(sp + 0x18);
    sw(ra, sp + 0x54);
    sw(fp, sp + 0x50);
    sw(s7, sp + 0x4C);
    sw(s6, sp + 0x48);
    sw(s5, sp + 0x44);
    sw(s4, sp + 0x40);
    sw(s3, sp + 0x3C);
    sw(s2, sp + 0x38);
    sw(s1, sp + 0x34);
    sw(s0, sp + 0x30);
    sw(a2, sp + 0x20);
    sw(a3, sp + 0x28);
    Is_Seq_Num_Valid();
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80043678;
    }
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v1 + 0xC);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s7 = lbu(v0 + 0xB);
    a0 = s7 & 0xFF;
    s4 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_8004346C;
    a1 = lw(v1 + 0x20);
    v0 = s4 & 0xFF;                                     // Result = 00000000
loc_80043434:
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v1 += a1;
    v0 = lw(v1);
    v0 &= 1;
    if (v0 == 0) goto loc_8004346C;
    s4++;
    v0 = s4 & 0xFF;
    v0 = (v0 < a0);
    {
        const bool bJump = (v0 != 0)
        v0 = s4 & 0xFF;
        if (bJump) goto loc_80043434;
    }
loc_8004346C:
    a0 = s4 & 0xFF;
    s5 = 0;                                             // Result = 00000000
    if (a0 != s7) goto loc_8004348C;
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80043484:
    v0 = 0;                                             // Result = 00000000
    goto loc_80043678;
loc_8004348C:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    t0 = lw(sp + 0x10);
    v0 = lw(v1 + 0xC);
    fp = lhu(t0);
    v1 = lw(v1 + 0x20);
    s7 = lbu(v0 + 0xC);
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    s2 = v0 + v1;
    s6 = lw(s2 + 0xC);
    s3 = 0;                                             // Result = 00000000
    if (s7 == 0) goto loc_800435C4;
    v1 = s3 & 0xFF;                                     // Result = 00000000
loc_800434C8:
    v0 = v1 << 2;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += v1;
    v1 = lw(a0 + 0x28);
    v0 <<= 4;
    s1 = v0 + v1;
    v0 = lw(s1);
    v0 &= 1;
    a0 = s1;
    if (v0 != 0) goto loc_800435B0;
    t0 = lw(sp + 0x10);
    s0 = s5 & 0xFF;                                     // Result = 00000000
    v0 = lw(t0 + 0x4);
    s0 <<= 5;                                           // Result = 00000000
    sb(s4, s1 + 0x2);
    a2 = lw(sp + 0x68);
    s0 += v0;
    a1 = s0;
    filltrackstat();
    a0 = s1;
    a1 = s0;
    assigntrackstat();
    t0 = lw(sp + 0x28);
    v0 = -5;                                            // Result = FFFFFFFB
    if (t0 == 0) goto loc_8004354C;
    v0 = lw(s1);
    v0 |= 0xC;
    sw(v0, s1);
    goto loc_80043574;
loc_8004354C:
    v1 = lw(s1);
    v1 &= v0;
    v0 = -9;                                            // Result = FFFFFFF7
    v1 &= v0;
    sw(v1, s1);
    v0 = lbu(s2 + 0x5);
    v0++;
    sb(v0, s2 + 0x5);
loc_80043574:
    s5++;                                               // Result = 00000001
    v0 = lbu(s2 + 0x4);
    v1 = fp - 1;
    v0++;
    sb(v0, s2 + 0x4);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    fp = v1;
    v0 = lbu(a0 + 0x5);
    v1 <<= 16;
    v0++;
    sb(v0, a0 + 0x5);
    sb(s3, s6);
    s6++;
    if (v1 == 0) goto loc_800435C4;
loc_800435B0:
    s3++;
    v0 = s3 & 0xFF;
    v0 = (v0 < s7);
    v1 = s3 & 0xFF;
    if (v0 != 0) goto loc_800434C8;
loc_800435C4:
    v0 = s5 & 0xFF;
    if (v0 == 0) goto loc_8004365C;
    t0 = lhu(sp + 0x18);
    sh(t0, s2 + 0x2);
    t0 = lw(sp + 0x20);
    sw(t0, s2 + 0x8);
    t0 = lw(sp + 0x28);
    v1 = -3;                                            // Result = FFFFFFFD
    if (t0 == 0) goto loc_80043610;
    v0 = lw(s2);
    v0 |= 2;
    sw(v0, s2);
    sb(0, s2 + 0x1);
    goto loc_80043628;
loc_80043610:
    v0 = lw(s2);
    v0 &= v1;
    sw(v0, s2);
    v0 = 1;                                             // Result = 00000001
    sb(v0, s2 + 0x1);
loc_80043628:
    v0 = 0x80;                                          // Result = 00000080
    sb(v0, s2 + 0x6);
    v0 = 0x40;                                          // Result = 00000040
    sb(v0, s2 + 0x7);
    v0 = lw(s2);
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 |= 1;
    sw(v0, s2);
    v0 = lbu(v1 + 0x4);
    v0++;
    sb(v0, v1 + 0x4);
loc_8004365C:
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
    v0 = s5 & 0xFF;
    {
        const bool bJump = (v0 == 0)
        v0 = s4 & 0xFF;                                 // Result = 00000000
        if (bJump) goto loc_80043484;
    }
    v0++;                                               // Result = 00000001
loc_80043678:
    ra = lw(sp + 0x54);
    fp = lw(sp + 0x50);
    s7 = lw(sp + 0x4C);
    s6 = lw(sp + 0x48);
    s5 = lw(sp + 0x44);
    s4 = lw(sp + 0x40);
    s3 = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x58;
    return;
}

void wess_seq_trigger() noexcept {
loc_800436AC:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = 0;                                             // Result = 00000000
    wess_seq_trigger_type();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_seq_trigger_special() noexcept {
    sp -= 0x20;
    v1 = a0;
    a0 = v1 << 2;
    a0 += v1;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 <<= 2;
    sw(ra, sp + 0x18);
    v0 = lw(v0 + 0xC);
    a2 = 0;                                             // Result = 00000000
    v0 = lw(v0 + 0x10);
    a3 = 0;                                             // Result = 00000000
    sw(a1, sp + 0x10);
    a1 = v1;
    a0 += v0;
    wess_seq_structrig();
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void wess_seq_status() noexcept {
loc_8004371C:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    Is_Seq_Num_Valid();
    a2 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_80043748;
    v0 = 0;                                             // Result = 00000000
    goto loc_800437DC;
loc_80043740:
    a2 = 2;                                             // Result = 00000002
    goto loc_800437D8;
loc_80043748:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v1 + 0xC);
    a0 = lbu(v0 + 0xB);
    a1 = lw(v1 + 0x20);
    v0 = a0;
    v0 &= 0xFF;
    a0--;
    if (v0 == 0) goto loc_800437D8;
    a3 = 1;                                             // Result = 00000001
    v1 = a1 + 1;
loc_8004377C:
    v0 = lw(a1);
    v0 &= 1;
    if (v0 == 0) goto loc_800437C0;
    v0 = lh(v1 + 0x1);
    if (v0 != s0) goto loc_800437C0;
    v0 = lbu(v1);
    if (v0 == 0) goto loc_80043740;
    {
        const bool bJump = (v0 != a3)
        v0 = a2;                                        // Result = 00000001
        if (bJump) goto loc_800437DC;
    }
    a2 = 3;                                             // Result = 00000003
    goto loc_800437D8;
loc_800437C0:
    v1 += 0x18;
    a1 += 0x18;
    v0 = a0;
    v0 &= 0xFF;
    a0--;
    if (v0 != 0) goto loc_8004377C;
loc_800437D8:
    v0 = a2;
loc_800437DC:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_seq_stop() noexcept {
loc_800437F0:
    sp -= 0x38;
    sw(s7, sp + 0x2C);
    s7 = a0;
    sw(ra, sp + 0x34);
    sw(fp, sp + 0x30);
    sw(s6, sp + 0x28);
    sw(s5, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    Is_Seq_Num_Valid();
    if (v0 == 0) goto loc_80043948;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s4 = lbu(v1 + 0xB);
    v0 = s4;
    if (s6 == 0) goto loc_8004393C;
    v0 &= 0xFF;
    s4--;
    if (v0 == 0) goto loc_8004393C;
    fp = -1;                                            // Result = FFFFFFFF
    s3 = s5 + 0xC;
loc_80043868:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_80043924;
    v0 = lh(s3 - 0xA);
    if (v0 != s7) goto loc_80043914;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s0 = lbu(v0 + 0x1C);
    s1 = lw(s3);
    s0--;
    if (s0 == fp) goto loc_80043914;
loc_800438AC:
    v1 = lbu(s1);
    a1 = 0xFF;                                          // Result = 000000FF
    a0 = v1 << 2;
    if (v1 == a1) goto loc_80043908;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 += v1;
    v0 = lw(v0 + 0x28);
    a0 <<= 4;
    a0 += v0;
    v0 = lbu(a0 + 0x3);
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    v0 <<= 2;
    v0 += a1;
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    s2--;
    pcall(v0);
    if (s2 == 0) goto loc_80043914;
loc_80043908:
    s0--;
    s1++;
    if (s0 != fp) goto loc_800438AC;
loc_80043914:
    s6--;
    v0 = s6 & 0xFF;
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80043940;
    }
loc_80043924:
    s3 += 0x18;
    s5 += 0x18;
    v0 = s4;
    v0 &= 0xFF;
    s4--;
    if (v0 != 0) goto loc_80043868;
loc_8004393C:
    v0 = 1;                                             // Result = 00000001
loc_80043940:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80043948:
    ra = lw(sp + 0x34);
    fp = lw(sp + 0x30);
    s7 = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x38;
    return;
}

void wess_seq_stopall() noexcept {
loc_8004397C:
    sp -= 0x38;
    sw(ra, sp + 0x34);
    sw(fp, sp + 0x30);
    sw(s7, sp + 0x2C);
    sw(s6, sp + 0x28);
    sw(s5, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    Is_Module_Loaded();
    if (v0 == 0) goto loc_80043AC4;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s4 = lbu(v1 + 0xB);
    v0 = s4;
    if (s6 == 0) goto loc_80043AB8;
    v0 &= 0xFF;
    s4--;
    if (v0 == 0) goto loc_80043AB8;
    s7 = -1;                                            // Result = FFFFFFFF
    fp = 0x80070000;                                    // Result = 80070000
    fp += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    s3 = s5 + 0xC;
loc_800439F8:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_80043AA0;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s0 = lbu(v0 + 0x1C);
    s1 = lw(s3);
    s0--;
    if (s0 == s7) goto loc_80043A90;
loc_80043A2C:
    v1 = lbu(s1);
    a1 = 0xFF;                                          // Result = 000000FF
    a0 = v1 << 2;
    if (v1 == a1) goto loc_80043A84;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 += v1;
    v0 = lw(v0 + 0x28);
    a0 <<= 4;
    a0 += v0;
    v0 = lbu(a0 + 0x3);
    v0 <<= 2;
    v0 += fp;
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    s2--;
    pcall(v0);
    if (s2 == 0) goto loc_80043A90;
loc_80043A84:
    s0--;
    s1++;
    if (s0 != s7) goto loc_80043A2C;
loc_80043A90:
    s6--;
    v0 = s6 & 0xFF;
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80043ABC;
    }
loc_80043AA0:
    s3 += 0x18;
    s5 += 0x18;
    v0 = s4;
    v0 &= 0xFF;
    s4--;
    if (v0 != 0) goto loc_800439F8;
loc_80043AB8:
    v0 = 1;                                             // Result = 00000001
loc_80043ABC:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80043AC4:
    ra = lw(sp + 0x34);
    fp = lw(sp + 0x30);
    s7 = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x38;
    return;
}

void wess_low_level_init() noexcept {
loc_80043AF8:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxspu_init();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_low_level_exit() noexcept {
loc_80043B18:
    return;
}

void wess_malloc() noexcept {
loc_80043B20:
    v0 = 0;                                             // Result = 00000000
    return;
}

void wess_free() noexcept {
loc_80043B28:
    return;
}

void GetIntsPerSec() noexcept {
loc_80043B30:
    v0 = 0x78;                                          // Result = 00000078
    return;
}

void CalcPartsPerInt() noexcept {
loc_80043B38:
    a2 <<= 16;
    a0 <<= 16;
    a0 = u32(i32(a0) >> 16);
    v0 = a0 << 4;
    v0 -= a0;
    v1 = v0 << 1;
    a2 += v1;
    a2 += 0x1E;
    v0 <<= 2;
    divu(a2, v0);
    if (v0 != 0) goto loc_80043B6C;
    _break(0x1C00);
loc_80043B6C:
    a2 = lo;
    a1 <<= 16;
    a1 = u32(i32(a1) >> 16);
    mult(a2, a1);
    v0 = lo;
    return;
}

void WessInterruptHandler() noexcept {
    sp -= 0x18;
    a0 = 0x80000;                                       // Result = 00080000
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5950);                               // Load from: gWess_T2counter (80075950)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5958);                               // Load from: 80075958
    a0 |= 0x5555;                                       // Result = 00085555
    sw(ra, sp + 0x10);
    v1 += a0;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5950);                                // Store to: gWess_T2counter (80075950)
    v0 = v1 >> 16;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5958);                                // Store to: 80075958
    v1 &= 0xFFFF;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5958);                                // Store to: 80075958
    v0 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5954);                                // Store to: gWess_Millicount (80075954)
    psxspu_fadeengine();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5948);                               // Load from: gbWess_SeqOn (80075948)
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80043C0C;
    }
    SeqEngine();
    v0 = 0;                                             // Result = 00000000
loc_80043C0C:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void init_WessTimer() noexcept {
loc_80043C1C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    LIBAPI_EnterCriticalSection();
    a0 = 0xF2000000;                                    // Result = F2000000
    a0 |= 2;                                            // Result = F2000002
    a1 = 2;                                             // Result = 00000002
    a3 = 0x80040000;                                    // Result = 80040000
    a3 += 0x3B88;                                       // Result = WessInterruptHandler (80043B88)
    a2 = 0x1000;                                        // Result = 00001000
    LIBAPI_OpenEvent();
    a0 = v0;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x595C);                                // Store to: gWess_EV2 (8007595C)
    LIBAPI_EnableEvent();
    a0 = 0xF2000000;                                    // Result = F2000000
    a0 |= 2;                                            // Result = F2000002
    a1 = 0x87A2;                                        // Result = 000087A2
    a2 = 0x1000;                                        // Result = 00001000
    LIBAPI_SetRCnt();
    a0 = 0xF2000000;                                    // Result = F2000000
    a0 |= 2;                                            // Result = F2000002
    LIBAPI_StartRCnt();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x594C);                                // Store to: gbWess_WessTimerActive (8007594C)
    LIBAPI_ExitCriticalSection();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void exit_WessTimer() noexcept {
loc_80043CA8:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxspu_get_master_vol();
    a0 = v0;
    psxspu_set_master_vol();
    psxspu_get_cd_vol();
    a0 = v0;
    psxspu_set_cd_vol();
    LIBAPI_EnterCriticalSection();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x595C);                               // Load from: gWess_EV2 (8007595C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x594C);                                 // Store to: gbWess_WessTimerActive (8007594C)
    LIBAPI_DisableEvent();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x595C);                               // Load from: gWess_EV2 (8007595C)
    LIBAPI_CloseEvent();
    LIBAPI_ExitCriticalSection();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Wess_init_for_LoadFileData() noexcept {
    v0 = 1;                                             // Result = 00000001
    return;
}

void module_open() noexcept {
loc_80043D20:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_open();
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0x1004;                                       // Result = gWess_module_fileref[0] (8007EFFC)
    a2 = v0;
    t0 = a2 + 0x20;
loc_80043D40:
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
    if (a2 != t0) goto loc_80043D40;
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0x1004;                                       // Result = gWess_module_fileref[0] (8007EFFC)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_read() noexcept {
loc_80043D94:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_read();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_seek() noexcept {
loc_80043DB4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_seek();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_tell() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_tell();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_close() noexcept {
loc_80043DF4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_close();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void get_num_Wess_Sound_Drivers() noexcept {
loc_80043E14:
    v0 = 1;                                             // Result = 00000001
    return;
}

void data_open() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_open();
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0xFDC;                                        // Result = gWess_data_fileref[0] (8007F024)
    a2 = v0;
    t0 = a2 + 0x20;
loc_80043E3C:
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
    if (a2 != t0) goto loc_80043E3C;
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0xFDC;                                        // Result = gWess_data_fileref[0] (8007F024)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void data_read_chunk() noexcept {
loc_80043E90:
    sp -= 0x20;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xFB4);                                // Load from: 8007F04C
    v1 = a0;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s2, sp + 0x18);
    s2 = a2;
    sw(ra, sp + 0x1C);
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_80043F00;
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a2 = v1;
    psxcd_read();
    a0 = s2;
    LIBSPU_SpuSetTransferStartAddr();
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = s1;
    LIBSPU_SpuWrite();
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x1008);                                // Store to: 8007EFF8
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xFB4);                                  // Store to: 8007F04C
    goto loc_80043F90;
loc_80043F00:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x1008);                               // Load from: 8007EFF8
    a1 = s1;
    if (v0 == 0) goto loc_80043F54;
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x6D7C;                                       // Result = gWess_data_read_chunk2[0] (80096D7C)
    a0 = s0;                                            // Result = gWess_data_read_chunk2[0] (80096D7C)
    a2 = v1;
    psxcd_read();
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    a0 = s2;
    LIBSPU_SpuSetTransferStartAddr();
    a0 = s0;                                            // Result = gWess_data_read_chunk2[0] (80096D7C)
    a1 = s1;
    LIBSPU_SpuWrite();
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x1008);                                 // Store to: 8007EFF8
    goto loc_80043F90;
loc_80043F54:
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a2 = v1;
    psxcd_read();
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    a0 = s2;
    LIBSPU_SpuSetTransferStartAddr();
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = s1;
    LIBSPU_SpuWrite();
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x1008);                                // Store to: 8007EFF8
loc_80043F90:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void data_read() noexcept {
    sp -= 0x28;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(s3, sp + 0x1C);
    s3 = a2;
    sw(s0, sp + 0x10);
    s0 = s3;
    sw(s1, sp + 0x14);
    s1 = a1;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x598C);                               // Load from: 8007598C
    a1 = a3;
    v0 -= s1;
    v0 = (v0 < s3);
    sw(ra, sp + 0x20);
    if (v0 == 0) goto loc_80043FF4;
    v0 = 0;                                             // Result = 00000000
    goto loc_80044058;
loc_80043FF4:
    a0 = s2;
    a2 = 0;                                             // Result = 00000000
    psxcd_seek();
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xFB4);                                 // Store to: 8007F04C
    v0 = (i32(s3) < 0x800);
    if (v0 != 0) goto loc_80044038;
loc_80044018:
    a0 = s2;
    a1 = 0x800;                                         // Result = 00000800
    a2 = s1;
    data_read_chunk();
    s0 -= 0x800;
    v0 = (i32(s0) < 0x800);
    s1 += 0x800;
    if (v0 == 0) goto loc_80044018;
loc_80044038:
    a0 = s2;
    if (s0 == 0) goto loc_8004404C;
    a1 = s0;
    a2 = s1;
    data_read_chunk();
loc_8004404C:
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    v0 = s3;
loc_80044058:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void data_close() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_close();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void updatetrackstat() noexcept {
loc_80044098:
    sp -= 0x30;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(s1, sp + 0x1C);
    s1 = a1;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    if (s1 == 0) goto loc_8004436C;
    v0 = lw(s1);
    s3 = v0;
    if (v0 == 0) goto loc_8004436C;
    a0 = s3 & 1;
    v1 = s3 & 2;
    if (a0 == 0) goto loc_800440E4;
    v0 = lbu(s1 + 0x4);
    sb(v0, s0 + 0xC);
loc_800440E4:
    v0 = s3 & 3;
    if (v1 == 0) goto loc_800440FC;
    v0 = lbu(s1 + 0x5);
    sb(v0, s0 + 0xD);
    v0 = s3 & 3;
loc_800440FC:
    if (v0 == 0) goto loc_80044164;
    v0 = lbu(s1 + 0x4);
    s2 = lw(s0 + 0x34);
    sb(v0, s0 + 0xC);
    v1 = lbu(s1 + 0x5);
    v0 = sp + 0x10;
    sw(v0, s0 + 0x34);
    v0 = 0xC;                                           // Result = 0000000C
    sb(v1, s0 + 0xD);
    sb(v0, sp + 0x10);
    v1 = lw(s0 + 0x34);
    v0 = lbu(s0 + 0xC);
    sb(v0, v1 + 0x1);
    v0 = lbu(s0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x30);
    goto loc_80044214;
loc_80044164:
    v0 = sp + 0x10;
    if (a0 == 0) goto loc_800441C0;
    s2 = lw(s0 + 0x34);
    v1 = lbu(s1 + 0x4);
    sw(v0, s0 + 0x34);
    v0 = 0xC;                                           // Result = 0000000C
    sb(v1, s0 + 0xC);
    sb(v0, sp + 0x10);
    v1 = lw(s0 + 0x34);
    v0 = lbu(s0 + 0xC);
    sb(v0, v1 + 0x1);
    v0 = lbu(s0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x30);
    goto loc_80044214;
loc_800441C0:
    if (v1 == 0) goto loc_80044224;
    s2 = lw(s0 + 0x34);
    v1 = lbu(s1 + 0x5);
    sw(v0, s0 + 0x34);
    v0 = 0xD;                                           // Result = 0000000D
    sb(v1, s0 + 0xD);
    sb(v0, sp + 0x10);
    v1 = lw(s0 + 0x34);
    v0 = lbu(s0 + 0xD);
    sb(v0, v1 + 0x1);
    v0 = lbu(s0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x34);
loc_80044214:
    a0 = s0;
    pcall(v0);
    sw(s2, s0 + 0x34);
loc_80044224:
    v0 = s3 & 4;
    {
        const bool bJump = (v0 == 0)
        v0 = s3 & 8;
        if (bJump) goto loc_80044240;
    }
    v0 = lhu(s1 + 0x6);
    sh(v0, s0 + 0xA);
    v0 = s3 & 8;
loc_80044240:
    {
        const bool bJump = (v0 == 0)
        v0 = sp + 0x10;
        if (bJump) goto loc_800442B4;
    }
    s2 = lw(s0 + 0x34);
    v1 = lhu(s1 + 0x8);
    sw(v0, s0 + 0x34);
    v0 = 9;                                             // Result = 00000009
    sh(v1, s0 + 0xE);
    sb(v0, sp + 0x10);
    v1 = lw(s0 + 0x34);
    v0 = lbu(s0 + 0xE);
    sb(v0, v1 + 0x1);
    v0 = lhu(s0 + 0xE);
    v1 = lw(s0 + 0x34);
    v0 >>= 8;
    sb(v0, v1 + 0x2);
    v0 = lbu(s0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x24);
    a0 = s0;
    pcall(v0);
    sw(s2, s0 + 0x34);
loc_800442B4:
    v0 = s3 & 0x10;
    {
        const bool bJump = (v0 == 0)
        v0 = s3 & 0x20;
        if (bJump) goto loc_800442FC;
    }
    v0 = lbu(s0 + 0x12);
    v1 = lbu(s1 + 0xA);
    v0 = i32(v0) >> v1;
    v0 &= 1;
    v1 = -3;                                            // Result = FFFFFFFD
    if (v0 == 0) goto loc_800442E8;
    v0 = lw(s0);
    v0 |= 2;
    goto loc_800442F4;
loc_800442E8:
    v0 = lw(s0);
    v0 &= v1;
loc_800442F4:
    sw(v0, s0);
    v0 = s3 & 0x20;
loc_800442FC:
    {
        const bool bJump = (v0 == 0)
        v0 = s3 & 0x40;
        if (bJump) goto loc_8004432C;
    }
    v0 = lhu(s1 + 0xC);
    sh(v0, s0 + 0x16);
    GetIntsPerSec();
    v0 <<= 16;
    a1 = lh(s0 + 0x14);
    a2 = lh(s0 + 0x16);
    a0 = u32(i32(v0) >> 16);
    CalcPartsPerInt();
    sw(v0, s0 + 0x1C);
    v0 = s3 & 0x40;
loc_8004432C:
    {
        const bool bJump = (v0 == 0)
        v0 = s3 & 0x80;
        if (bJump) goto loc_80044354;
    }
    v0 = lw(s0 + 0x28);
    a0 = lw(s1 + 0x10);
    v1 = lw(s0);
    v0 += a0;
    v1 |= 0x10;
    sw(v0, s0 + 0x2C);
    sw(v1, s0);
    v0 = s3 & 0x80;
loc_80044354:
    if (v0 == 0) goto loc_8004436C;
    v0 = lw(s0);
    v0 |= 0x20;
    sw(v0, s0);
loc_8004436C:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void wess_seq_trigger_type() noexcept {
loc_8004438C:
    sp -= 0x20;
    v1 = a0;
    a2 = a1;
    a0 = v1 << 2;
    a0 += v1;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 <<= 2;
    sw(ra, sp + 0x18);
    v0 = lw(v0 + 0xC);
    a1 = v1;
    v0 = lw(v0 + 0x10);
    a3 = 0;                                             // Result = 00000000
    sw(0, sp + 0x10);
    a0 += v0;
    wess_seq_structrig();
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void wess_seq_trigger_type_special() noexcept {
loc_800443DC:
    sp -= 0x20;
    v1 = a0;
    a3 = a1;
    a0 = v1 << 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 += v1;
    sw(ra, sp + 0x18);
    v0 = lw(v0 + 0xC);
    a0 <<= 2;
    v0 = lw(v0 + 0x10);
    a1 = v1;
    sw(a2, sp + 0x10);
    a2 = a3;
    a3 = 0;                                             // Result = 00000000
    a0 += v0;
    wess_seq_structrig();
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void queue_wess_seq_update_type_special() noexcept {
    sp -= 0x40;
    sw(fp, sp + 0x38);
    fp = a1;
    sw(ra, sp + 0x3C);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    sw(a0, sp + 0x10);
    Is_Module_Loaded();
    if (v0 == 0) goto loc_80044578;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s4 = lbu(v1 + 0xB);
    v0 = s4;
    if (s6 == 0) goto loc_8004456C;
    v0 &= 0xFF;
    s4--;
    if (v0 == 0) goto loc_8004456C;
    s3 = s5 + 0xC;
loc_800444A8:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_80044554;
    v0 = lw(s3 - 0x4);
    a2 = lw(sp + 0x10);
    if (v0 != a2) goto loc_80044544;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s1 = lw(s3);
    s0 = lbu(v0 + 0x1C);
    v0 = -1;                                            // Result = FFFFFFFF
    s0--;
    if (s0 == v0) goto loc_80044544;
    s7 = -1;                                            // Result = FFFFFFFF
loc_800444F8:
    a0 = lbu(s1);
    a2 = 0xFF;                                          // Result = 000000FF
    v0 = a0 << 2;
    if (a0 == a2) goto loc_80044538;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += a0;
    v1 = lw(v1 + 0x28);
    v0 <<= 4;
    a0 = v0 + v1;
    if (fp == 0) goto loc_8004452C;
    a1 = fp;
    updatetrackstat();
loc_8004452C:
    s2--;
    if (s2 == 0) goto loc_80044544;
loc_80044538:
    s0--;
    s1++;
    if (s0 != s7) goto loc_800444F8;
loc_80044544:
    s6--;
    v0 = s6 & 0xFF;
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80044570;
    }
loc_80044554:
    s3 += 0x18;
    s5 += 0x18;
    v0 = s4;
    v0 &= 0xFF;
    s4--;
    if (v0 != 0) goto loc_800444A8;
loc_8004456C:
    v0 = 1;                                             // Result = 00000001
loc_80044570:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80044578:
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

void wess_seq_stoptype() noexcept {
loc_800445AC:
    sp -= 0x40;
    sw(ra, sp + 0x3C);
    sw(fp, sp + 0x38);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    sw(a0, sp + 0x10);
    Is_Module_Loaded();
    if (v0 == 0) goto loc_8004470C;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s4 = lbu(v1 + 0xB);
    v0 = s4;
    if (s6 == 0) goto loc_80044700;
    v0 &= 0xFF;
    s4--;
    if (v0 == 0) goto loc_80044700;
    s7 = -1;                                            // Result = FFFFFFFF
    fp = 0x80070000;                                    // Result = 80070000
    fp += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    s3 = s5 + 0xC;
loc_8004462C:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_800446E8;
    v0 = lw(s3 - 0x4);
    a1 = lw(sp + 0x10);
    if (v0 != a1) goto loc_800446D8;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s0 = lbu(v0 + 0x1C);
    s1 = lw(s3);
    s0--;
    if (s0 == s7) goto loc_800446D8;
loc_80044674:
    v1 = lbu(s1);
    a1 = 0xFF;                                          // Result = 000000FF
    a0 = v1 << 2;
    if (v1 == a1) goto loc_800446CC;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 += v1;
    v0 = lw(v0 + 0x28);
    a0 <<= 4;
    a0 += v0;
    v0 = lbu(a0 + 0x3);
    v0 <<= 2;
    v0 += fp;
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    s2--;
    pcall(v0);
    if (s2 == 0) goto loc_800446D8;
loc_800446CC:
    s0--;
    s1++;
    if (s0 != s7) goto loc_80044674;
loc_800446D8:
    s6--;
    v0 = s6 & 0xFF;
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80044704;
    }
loc_800446E8:
    s3 += 0x18;
    s5 += 0x18;
    v0 = s4;
    v0 &= 0xFF;
    s4--;
    if (v0 != 0) goto loc_8004462C;
loc_80044700:
    v0 = 1;                                             // Result = 00000001
loc_80044704:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_8004470C:
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

void wess_seq_load_err() noexcept {
loc_80044740:
    sp -= 0x18;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5978);                               // Load from: 80075978
    a1 = a0;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_80044768;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x597C);                               // Load from: 8007597C
    pcall(v0);
loc_80044768:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_seq_loader_install_error_handler() noexcept {
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5978);                                // Store to: 80075978
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x597C);                                // Store to: 8007597C
    return;
}

void Is_Seq_Seq_Num_Valid() noexcept {
loc_80044790:
    v0 = 0;                                             // Result = 00000000
    if (i32(a0) < 0) goto loc_800447B4;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x596C);                               // Load from: gWess_max_sequences (8007596C)
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800447B4;
    }
    v0 = 0;                                             // Result = 00000000
loc_800447B4:
    return;
}

void open_sequence_data() noexcept {
loc_800447BC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5974);                               // Load from: 80075974
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 != 0) goto loc_80044800;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5964);                               // Load from: gWess_seq_loader_fileName (80075964)
    module_open();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5980);                                // Store to: 80075980
    if (v0 != 0) goto loc_80044800;
    a0 = 1;                                             // Result = 00000001
    wess_seq_load_err();
    v0 = 0;                                             // Result = 00000000
    goto loc_80044818;
loc_80044800:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5974);                               // Load from: 80075974
    v0 = 1;                                             // Result = 00000001
    v1++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5974);                                // Store to: 80075974
loc_80044818:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void close_sequence_data() noexcept {
loc_80044828:
    sp -= 0x18;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5974);                               // Load from: 80075974
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x10);
    if (v1 != v0) goto loc_80044850;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5980);                               // Load from: 80075980
    module_close();
loc_80044850:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5974);                               // Load from: 80075974
    {
        const bool bJump = (i32(v0) <= 0)
        v0--;
        if (bJump) goto loc_8004486C;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5974);                                // Store to: 80075974
loc_8004486C:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void load_sequence_data() noexcept {
loc_8004487C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x50;
    sw(s5, sp + 0x3C);
    s5 = a0;
    sw(s1, sp + 0x2C);
    s1 = a1;
    sw(fp, sp + 0x48);
    fp = s1;
    sw(ra, sp + 0x4C);
    sw(s7, sp + 0x44);
    sw(s6, sp + 0x40);
    sw(s4, sp + 0x38);
    sw(s3, sp + 0x34);
    sw(s2, sp + 0x30);
    sw(s0, sp + 0x28);
    if (v0 == 0) goto loc_80044FF0;
    Is_Seq_Seq_Num_Valid();
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80044FF4;
    }
    open_sequence_data();
    {
        const bool bJump = (v0 != 0)
        v0 = s5 << 2;
        if (bJump) goto loc_800448E8;
    }
    a0 = 1;                                             // Result = 00000001
    goto loc_80044E9C;
loc_800448E8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s5;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    s7 = lw(v0 + 0x10);
    a1 = lw(v0 + 0x8);
    sw(s1, v0 + 0x4);
    if (s7 == 0) goto loc_80044924;
    v0 = s7 << 5;
    s1 += v0;
    goto loc_80044928;
loc_80044924:
    s1 += 0x20;
loc_80044928:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5980);                               // Load from: 80075980
    a2 = 0;                                             // Result = 00000000
    module_seek();
    s0 = 4;                                             // Result = 00000004
    if (v0 != 0) goto loc_80044E98;
    a1 = 4;                                             // Result = 00000004
    v0 = s5 << 2;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 += s5;
    v1 = lw(v1 + 0xC);
    s2 = v0 << 2;
    a0 = lw(v1 + 0x10);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    a0 += s2;
    module_read();
    a0 = 2;                                             // Result = 00000002
    if (v0 != s0) goto loc_80044E9C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s2;
    s4 = lh(v0);
    v0 = -1;                                            // Result = FFFFFFFF
    s4--;
    s0 = 0x18;                                          // Result = 00000018
    if (s4 == v0) goto loc_80044E60;
    s6 = 0x80080000;                                    // Result = 80080000
    s6 -= 0xFB0;                                        // Result = 8007F050
    s3 = s2;
    s2 = 0;                                             // Result = 00000000
loc_800449BC:
    a0 = s6;                                            // Result = 8007F050
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    a1 = 0x18;                                          // Result = 00000018
    module_read();
    a0 = 2;                                             // Result = 00000002
    if (v0 != s0) goto loc_80044E9C;
    v1 = lbu(s6);                                       // Load from: 8007F050
    t0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80044ACC;
    v0 = 0x32;                                          // Result = 00000032
    {
        const bool bJump = (v1 == v0)
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_80044ACC;
    }
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    a0 = lbu(a1 + 0x8);
    a0--;
    {
        const bool bJump = (a0 == v0)
        v0 = a0 << 2;
        if (bJump) goto loc_80044AE8;
    }
    t1 = v1;
    a3 = a1;
    v1 = lw(a1 + 0x18);
    v0 += a0;
    v0 <<= 2;
    v0 += a0;
    a1 = v0 << 2;
    a2 = a1 + v1;
loc_80044A34:
    v0 = lw(a2 + 0x4C);
    if (t1 != v0) goto loc_80044AD4;
    v1 = lbu(s6 + 0x4);                                 // Load from: 8007F054
    v0 = 3;                                             // Result = 00000003
    if (v1 == 0) goto loc_80044A5C;
    if (v1 != v0) goto loc_80044A70;
loc_80044A5C:
    v0 = lw(a2 + 0x50);
    v0 &= 1;
    if (v0 != 0) goto loc_80044ACC;
loc_80044A70:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xFAC);                               // Load from: 8007F054
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0)
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80044AA4;
    }
    v0 = lw(a3 + 0x18);
    v0 += a1;
    v0 = lw(v0 + 0x50);
    v0 &= 2;
    {
        const bool bJump = (v0 != 0)
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80044ACC;
    }
loc_80044AA4:
    if (v1 != v0) goto loc_80044AD4;
    v0 = lw(a3 + 0x18);
    v0 += a1;
    v0 = lw(v0 + 0x50);
    v0 &= 4;
    if (v0 == 0) goto loc_80044AD4;
loc_80044ACC:
    t0 = 1;                                             // Result = 00000001
    goto loc_80044AE8;
loc_80044AD4:
    a1 -= 0x54;
    a0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a2 -= 0x54;
    if (a0 != v0) goto loc_80044A34;
loc_80044AE8:
    a2 = 1;                                             // Result = 00000001
    if (t0 == 0) goto loc_80044E24;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    v1 = lw(s6);                                        // Load from: 8007F050
    a0 = lw(s6 + 0x4);                                  // Load from: 8007F054
    a1 = lw(s6 + 0x8);                                  // Load from: 8007F058
    a2 = lw(s6 + 0xC);                                  // Load from: 8007F05C
    sw(v1, v0);
    sw(a0, v0 + 0x4);
    sw(a1, v0 + 0x8);
    sw(a2, v0 + 0xC);
    v1 = lw(s6 + 0x10);                                 // Load from: 8007F060
    a0 = lw(s6 + 0x14);                                 // Load from: 8007F064
    sw(v1, v0 + 0x10);
    sw(a0, v0 + 0x14);
    v1 = lbu(s6);                                       // Load from: 8007F050
    v0 = 0x32;                                          // Result = 00000032
    if (v1 != v0) goto loc_80044D20;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    sb(0, v0);
    v1 = lbu(s6 + 0x4);                                 // Load from: 8007F054
    a0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80044BA8;
    v0 = 3;                                             // Result = 00000003
    {
        const bool bJump = (v1 != v0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80044BFC;
    }
loc_80044BA8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lbu(v1 + 0x8);
    a3 = v0;
    if (i32(v0) <= 0) goto loc_80044D28;
    a2 = v1;
    a1 = lw(a2 + 0x18);
loc_80044BCC:
    v1 = a1;
    v0 = lw(v1 + 0x50);
    v0 &= 1;
    a0++;
    if (v0 != 0) goto loc_80044C5C;
    v0 = (i32(a0) < i32(a3));
    a1 = v1 + 0x54;
    if (v0 != 0) goto loc_80044BCC;
    goto loc_80044D20;
loc_80044BFC:
    {
        const bool bJump = (v1 != v0)
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80044CCC;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lbu(v1 + 0x8);
    a0 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80044D28;
    a3 = v0;
    a2 = v1;
    a1 = lw(a2 + 0x18);
loc_80044C2C:
    v1 = a1;
    v0 = lw(v1 + 0x50);
    v0 &= 2;
    a0++;
    if (v0 != 0) goto loc_80044C80;
    v0 = (i32(a0) < i32(a3));
    a1 = v1 + 0x54;
    if (v0 != 0) goto loc_80044C2C;
    goto loc_80044D20;
loc_80044C5C:
    v0 = lw(a2 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v1 = lbu(a1 + 0x4C);
    v0 += s2;
    goto loc_80044CC4;
loc_80044C80:
    v0 = lw(a2 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v1 = lbu(a1 + 0x4C);
    v0 += s2;
    goto loc_80044CC4;
loc_80044CA4:
    v0 = lw(a1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v1 = lbu(v1 + 0x4C);
    v0 += s2;
loc_80044CC4:
    sb(v1, v0);
    goto loc_80044D20;
loc_80044CCC:
    if (v1 != v0) goto loc_80044D20;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lbu(v1 + 0x8);
    a0 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80044D28;
    a2 = v0;
    a1 = v1;
    v1 = lw(a1 + 0x18);
loc_80044CFC:
    v0 = lw(v1 + 0x50);
    v0 &= 4;
    a0++;
    if (v0 != 0) goto loc_80044CA4;
    v0 = (i32(a0) < i32(a2));
    v1 += 0x54;
    if (v0 != 0) goto loc_80044CFC;
loc_80044D20:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
loc_80044D28:
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    sw(s1, v0 + 0x18);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    v0 += s2;
    v1 = lh(v0 + 0x12);
    a0 = lw(v0 + 0x18);
    v1 <<= 2;
    s1 += v1;
    s0 = v1;
    a1 = s0;
    module_read();
    a0 = 2;                                             // Result = 00000002
    if (s0 != v0) goto loc_80044E9C;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    sw(s1, v0 + 0x1C);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    v0 += s2;
    v1 = lw(v0 + 0x14);
    a0 = lw(v0 + 0x1C);
    s1 += v1;
    v0 = s1 & 1;
    s1 += v0;
    v0 = s1 & 2;
    s1 += v0;
    s0 = v1;
    a1 = s0;
    module_read();
    s2 += 0x20;                                         // Result = 00000020
    if (s0 != v0) goto loc_80044E90;
    s4--;
    goto loc_80044E54;
loc_80044E24:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5980);                               // Load from: 80075980
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lh(a1 - 0xF9E);                                // Load from: 8007F062
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF9C);                                // Load from: 8007F064
    a1 <<= 2;
    a1 += v0;
    module_seek();
    a0 = 3;                                             // Result = 00000003
    if (v0 != 0) goto loc_80044E9C;
    s4--;
loc_80044E54:
    v0 = -1;                                            // Result = FFFFFFFF
    s0 = 0x18;                                          // Result = 00000018
    if (s4 != v0) goto loc_800449BC;
loc_80044E60:
    v0 = s5 << 2;
    if (s7 == 0) goto loc_80044EAC;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s5;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    sh(s7, v0);
    goto loc_80044FE8;
loc_80044E90:
    a0 = 2;                                             // Result = 00000002
    goto loc_80044E9C;
loc_80044E98:
    a0 = 3;                                             // Result = 00000003
loc_80044E9C:
    wess_seq_load_err();
    v0 = 0;                                             // Result = 00000000
    goto loc_80044FF4;
loc_80044EAC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    a0 = s5 << 2;
    v0 = lw(v0 + 0xC);
    a0 += s5;
    v0 = lw(v0 + 0x10);
    a0 <<= 2;
    v0 += a0;
    v0 = lw(v0 + 0x4);
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0xF98;                                        // Result = 8007F068
    v1 = lw(a3);                                        // Load from: 8007F068
    a1 = lw(a3 + 0x4);                                  // Load from: 8007F06C
    a2 = lw(a3 + 0x8);                                  // Load from: 8007F070
    sw(v1, v0);
    sw(a1, v0 + 0x4);
    sw(a2, v0 + 0x8);
    v1 = lw(a3 + 0xC);                                  // Load from: 8007F074
    a1 = lw(a3 + 0x10);                                 // Load from: 8007F078
    a2 = lw(a3 + 0x14);                                 // Load from: 8007F07C
    sw(v1, v0 + 0xC);
    sw(a1, v0 + 0x10);
    sw(a2, v0 + 0x14);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    sw(s1, v0 + 0x18);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    sw(s1, v0 + 0x1C);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x5970);                              // Load from: 80075970
    a1 = lw(v0 + 0x1C);
    a2 = lw(v0 + 0x14);
    sb(v1, a1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    v1 = lw(v0 + 0x1C);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5971);                              // Load from: 80075971
    s1 += a2;
    sb(v0, v1 + 0x1);
    v0 = s1 & 1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    s1 += v0;
    v0 = lw(v1 + 0xC);
    v1 = s1 & 2;
    v0 = lw(v0 + 0x10);
    s1 += v1;
    a0 += v0;
    v0 = 1;                                             // Result = 00000001
    sh(v0, a0);
loc_80044FE8:
    close_sequence_data();
loc_80044FF0:
    v0 = s1 - fp;
loc_80044FF4:
    ra = lw(sp + 0x4C);
    fp = lw(sp + 0x48);
    s7 = lw(sp + 0x44);
    s6 = lw(sp + 0x40);
    s5 = lw(sp + 0x3C);
    s4 = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x50;
    return;
}

void wess_seq_loader_init() noexcept {
loc_80045028:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5960);                                 // Store to: gbWess_seq_loader_enable (80075960)
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x5964);                                // Store to: gWess_seq_loader_fileName (80075964)
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5968);                                // Store to: gpWess_seq_loader_pm_stat (80075968)
    s0 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_80045124;
    v0 = lw(a0 + 0xC);
    s0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5960);                                // Store to: gbWess_seq_loader_enable (80075960)
    v1 = lh(v0 + 0x8);
    v0 = 0x80;                                          // Result = 00000080
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xF96);                                 // Store to: 8007F06A
    v0 = 0x7F;                                          // Result = 0000007F
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xF8E);                                 // Store to: 8007F072
    v0 = 0x40;                                          // Result = 00000040
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xF8D);                                 // Store to: 8007F073
    v0 = 0x78;                                          // Result = 00000078
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xF8A);                                 // Store to: 8007F076
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xF88);                                 // Store to: 8007F078
    v0 = 2;                                             // Result = 00000002
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF98);                                  // Store to: 8007F068
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF97);                                  // Store to: 8007F069
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF93);                                  // Store to: 8007F06D
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF94);                                  // Store to: 8007F06C
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xF92);                                  // Store to: 8007F06E
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xF90);                                  // Store to: 8007F070
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF8C);                                  // Store to: 8007F074
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF8B);                                  // Store to: 8007F075
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xF86);                                  // Store to: 8007F07A
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF84);                                 // Store to: 8007F07C
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x596C);                                // Store to: gWess_max_sequences (8007596C)
    v0 = s0;                                            // Result = 00000001
    if (a2 != s0) goto loc_80045128;
    open_sequence_data();
    {
        const bool bJump = (v0 != 0)
        v0 = s0;                                        // Result = 00000001
        if (bJump) goto loc_80045128;
    }
    a0 = 1;                                             // Result = 00000001
    wess_seq_load_err();
    v0 = 0;                                             // Result = 00000000
    goto loc_80045128;
loc_80045124:
    v0 = s0;                                            // Result = 00000000
loc_80045128:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_seq_loader_exit() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    close_sequence_data();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5960);                                 // Store to: gbWess_seq_loader_enable (80075960)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_seq_sizeof() noexcept {
loc_80045164:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    if (v0 == 0) goto loc_800451D4;
    Is_Seq_Seq_Num_Valid();
    {
        const bool bJump = (v0 != 0)
        v0 = s0 << 2;
        if (bJump) goto loc_800451A0;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_800451DC;
loc_800451A0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v1 += v0;
    v0 = lw(v1 + 0x4);
    {
        const bool bJump = (v0 != 0)
        v0 = s1;                                        // Result = 00000000
        if (bJump) goto loc_800451DC;
    }
    s1 = lw(v1 + 0xC);
loc_800451D4:
    v0 = s1;
loc_800451DC:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_seq_load() noexcept {
loc_800451F4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x1C);
    if (v0 == 0) goto loc_80045278;
    Is_Seq_Seq_Num_Valid();
    {
        const bool bJump = (v0 != 0)
        v0 = s0 << 2;
        if (bJump) goto loc_80045238;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_8004527C;
loc_80045238:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    v0 = lw(v0 + 0x4);
    {
        const bool bJump = (v0 != 0)
        v0 = s1;                                        // Result = 00000000
        if (bJump) goto loc_8004527C;
    }
    a0 = s0;
    a1 = s2;
    load_sequence_data();
    s1 = v0;
loc_80045278:
    v0 = s1;
loc_8004527C:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_seq_free() noexcept {
loc_80045298:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    if (v0 == 0) goto loc_8004530C;
    Is_Seq_Seq_Num_Valid();
    {
        const bool bJump = (v0 != 0)
        v0 = s0 << 2;
        if (bJump) goto loc_800452D4;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_80045310;
loc_800452D4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v1 += v0;
    v0 = lw(v1 + 0x4);
    {
        const bool bJump = (v0 == 0)
        v0 = s1;                                        // Result = 00000000
        if (bJump) goto loc_80045310;
    }
    sw(0, v1 + 0x4);
    s1 = 1;                                             // Result = 00000001
loc_8004530C:
    v0 = s1;
loc_80045310:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void psxspu_init_reverb() noexcept {
loc_80045328:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s0, sp + 0x10);
    s0 = 0x80080000;                                    // Result = 80080000
    s0 -= 0xF80;                                        // Result = 8007F080
    v1 = lw(sp + 0x30);
    v0 = 0x1F;                                          // Result = 0000001F
    sw(ra, sp + 0x18);
    sw(v0, s0);                                         // Store to: 8007F080
    v0 = s1 | 0x100;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF7C);                                 // Store to: 8007F084
    at = 0x80080000;                                    // Result = 80080000
    sh(a1, at - 0xF78);                                 // Store to: 8007F088
    at = 0x80080000;                                    // Result = 80080000
    sh(a2, at - 0xF76);                                 // Store to: 8007F08A
    at = 0x80080000;                                    // Result = 80080000
    sw(a3, at - 0xF74);                                 // Store to: 8007F08C
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF70);                                 // Store to: 8007F090
    a0 = s0;                                            // Result = 8007F080
    LIBSPU_SpuSetReverbModeParam();
    a0 = s0;                                            // Result = 8007F080
    LIBSPU_SpuSetReverbDepth();
    if (s1 != 0) goto loc_800453BC;
    a0 = 0;                                             // Result = 00000000
    LIBSPU_SpuSetReverb();
    v0 = 0x70000;                                       // Result = 00070000
    v0 |= 0xF000;                                       // Result = 0007F000
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x598C);                                // Store to: 8007598C
    a0 = 0;                                             // Result = 00000000
    goto loc_800453D8;
loc_800453BC:
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuSetReverb();
    LIBSPU_SpuGetReverbOffsetAddr();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x598C);                                // Store to: 8007598C
    a0 = 1;                                             // Result = 00000001
loc_800453D8:
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    LIBSPU_SpuSetReverbVoice();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void psxspu_set_reverb_depth() noexcept {
    sp -= 0x18;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0xF78;                                        // Result = 8007F088
    sw(ra, sp + 0x10);
    sh(a0, v0);                                         // Store to: 8007F088
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80080000;                                    // Result = 80080000
    sh(a1, at - 0xF76);                                 // Store to: 8007F08A
    a0 = v0 - 8;                                        // Result = 8007F080
    LIBSPU_SpuSetReverbDepth();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxspu_init() noexcept {
loc_80045450:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5984);                               // Load from: 80075984
    sp -= 0x48;
    sw(ra, sp + 0x44);
    sw(s0, sp + 0x40);
    if (v0 != 0) goto loc_800454E8;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    s0 = 1;                                             // Result = 00000001
    LIBSPU_SpuInit();
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 -= 0x6AF8;                                       // Result = 800A9508
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5984);                                // Store to: 80075984
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuInitMalloc();
    a0 = 0;                                             // Result = 00000000
    LIBSPU_SpuSetTransferMode();
    a0 = 0;                                             // Result = 00000000
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    sw(0, sp + 0x10);
    psxspu_init_reverb();
    a0 = sp + 0x18;
    v0 = 0x3C3;                                         // Result = 000003C3
    sw(v0, sp + 0x18);
    v0 = 0x3FFF;                                        // Result = 00003FFF
    sh(v0, sp + 0x1C);
    sh(v0, sp + 0x1E);
    v0 = 0x3CFF;                                        // Result = 00003CFF
    sh(v0, sp + 0x28);
    sh(v0, sp + 0x2A);
    sw(0, sp + 0x2C);
    sw(s0, sp + 0x30);
    LIBSPU_SpuSetCommonAttr();
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5988);                                // Store to: 80075988
loc_800454E8:
    ra = lw(sp + 0x44);
    s0 = lw(sp + 0x40);
    sp += 0x48;
    return;
}

void psxspu_update_master_vol() noexcept {
loc_800454FC:
    sp -= 0x40;
    v0 = 3;                                             // Result = 00000003
    sh(a0, sp + 0x14);
    sh(a0, sp + 0x16);
    a0 = sp + 0x10;
    sw(ra, sp + 0x38);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    sw(v0, sp + 0x10);
    LIBSPU_SpuSetCommonAttr();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x38);
    sp += 0x40;
    return;
}

void psxspu_update_master_vol_mode() noexcept {
loc_80045540:
    sp -= 0x40;
    v0 = 0xC0;                                          // Result = 000000C0
    sh(a0, sp + 0x20);
    sh(a0, sp + 0x22);
    a0 = sp + 0x10;
    sw(ra, sp + 0x38);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    sw(v0, sp + 0x10);
    LIBSPU_SpuSetCommonAttr();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x38);
    sp += 0x40;
    return;
}

void psxspu_setcdmixon() noexcept {
loc_80045584:
    sp -= 0x40;
    v0 = 0x200;                                         // Result = 00000200
    sw(s0, sp + 0x38);
    s0 = 1;                                             // Result = 00000001
    a0 = sp + 0x10;
    sw(ra, sp + 0x3C);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    sw(v0, sp + 0x10);
    sw(s0, sp + 0x28);
    LIBSPU_SpuSetCommonAttr();
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x3C);
    s0 = lw(sp + 0x38);
    sp += 0x40;
    return;
}

void psxspu_setcdmixoff() noexcept {
loc_800455CC:
    sp -= 0x40;
    v0 = 0x200;                                         // Result = 00000200
    a0 = sp + 0x10;
    sw(ra, sp + 0x38);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    sw(v0, sp + 0x10);
    sw(0, sp + 0x28);
    LIBSPU_SpuSetCommonAttr();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x38);
    sp += 0x40;
    return;
}

void psxspu_fadeengine() noexcept {
loc_8004560C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5988);                               // Load from: 80075988
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_80045710;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x59A8);                               // Load from: 800759A8
    {
        const bool bJump = (i32(v1) <= 0)
        v1--;
        if (bJump) goto loc_80045698;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59AC);                               // Load from: 800759AC
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x59B4);                               // Load from: 800759B4
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x59A8);                                // Store to: 800759A8
    v0 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59AC);                                // Store to: 800759AC
    if (v1 != 0) goto loc_80045674;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59B0);                               // Load from: 800759B0
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59AC);                                // Store to: 800759AC
loc_80045674:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lh(v0 + 0x59AE);                               // Load from: 800759AE
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59A4);                                // Store to: 800759A4
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lh(a0 + 0x59A4);                               // Load from: 800759A4
    psxspu_update_master_vol_mode();
loc_80045698:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5994);                               // Load from: 80075994
    {
        const bool bJump = (i32(v1) <= 0)
        v1--;
        if (bJump) goto loc_80045710;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5998);                               // Load from: 80075998
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x59A0);                               // Load from: 800759A0
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5994);                                // Store to: 80075994
    v0 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5998);                                // Store to: 80075998
    if (v1 != 0) goto loc_800456EC;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x599C);                               // Load from: 8007599C
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5998);                                // Store to: 80075998
loc_800456EC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lh(v0 + 0x599A);                               // Load from: 8007599A
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5990);                                // Store to: 80075990
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lh(a0 + 0x5990);                               // Load from: 80075990
    psxspu_update_master_vol();
loc_80045710:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxspu_set_cd_vol() noexcept {
loc_80045720:
    sp -= 0x18;
    v0 = a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59A4);                                // Store to: 800759A4
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lh(a0 + 0x59A4);                               // Load from: 800759A4
    v0 <<= 16;
    sw(ra, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59AC);                                // Store to: 800759AC
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x59A8);                                 // Store to: 800759A8
    psxspu_update_master_vol_mode();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxspu_get_cd_vol() noexcept {
loc_8004577C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59A4);                               // Load from: 800759A4
    return;
}

void psxspu_start_cd_fade() noexcept {
loc_8004578C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x594C);                               // Load from: gbWess_WessTimerActive (8007594C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    v1 = 0x10620000;                                    // Result = 10620000
    if (v0 == 0) goto loc_80045828;
    v1 |= 0x4DD3;                                       // Result = 10624DD3
    v0 = a0 << 4;
    v0 -= a0;
    v0 <<= 3;
    mult(v0, v1);
    v0 = u32(i32(v0) >> 31);
    a0 = a1 << 16;
    v1 = hi;
    v1 = u32(i32(v1) >> 6);
    v1 -= v0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59AC);                               // Load from: 800759AC
    v1++;
    v0 = a0 - v0;
    div(v0, v1);
    if (v1 != 0) goto loc_800457EC;
    _break(0x1C00);
loc_800457EC:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at)
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80045804;
    }
    if (v0 != at) goto loc_80045804;
    _break(0x1800);
loc_80045804:
    v0 = lo;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x59B0);                                // Store to: 800759B0
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x59A8);                                // Store to: 800759A8
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59B4);                                // Store to: 800759B4
    v0 = 1;                                             // Result = 00000001
    goto loc_80045834;
loc_80045828:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x59A8);                                 // Store to: 800759A8
    v0 = 1;                                             // Result = 00000001
loc_80045834:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    return;
}

void psxspu_stop_cd_fade() noexcept {
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x59A8);                                 // Store to: 800759A8
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    return;
}

void psxspu_get_cd_fade_status() noexcept {
loc_80045868:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59A8);                               // Load from: 800759A8
    v0 = (i32(v0) < 2);
    v0 ^= 1;
    return;
}

void psxspu_set_master_vol() noexcept {
loc_80045880:
    sp -= 0x18;
    v0 = a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5990);                                // Store to: 80075990
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lh(a0 + 0x5990);                               // Load from: 80075990
    v0 <<= 16;
    sw(ra, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5998);                                // Store to: 80075998
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5994);                                 // Store to: 80075994
    psxspu_update_master_vol();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxspu_get_master_vol() noexcept {
loc_800458DC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5990);                               // Load from: 80075990
    return;
}

void psxspu_start_master_fade() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x594C);                               // Load from: gbWess_WessTimerActive (8007594C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    v1 = 0x10620000;                                    // Result = 10620000
    if (v0 == 0) goto loc_80045988;
    v1 |= 0x4DD3;                                       // Result = 10624DD3
    v0 = a0 << 4;
    v0 -= a0;
    v0 <<= 3;
    mult(v0, v1);
    v0 = u32(i32(v0) >> 31);
    a0 = a1 << 16;
    v1 = hi;
    v1 = u32(i32(v1) >> 6);
    v1 -= v0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5998);                               // Load from: 80075998
    v1++;
    v0 = a0 - v0;
    div(v0, v1);
    if (v1 != 0) goto loc_8004594C;
    _break(0x1C00);
loc_8004594C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at)
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80045964;
    }
    if (v0 != at) goto loc_80045964;
    _break(0x1800);
loc_80045964:
    v0 = lo;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x599C);                                // Store to: 8007599C
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5994);                                // Store to: 80075994
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59A0);                                // Store to: 800759A0
    v0 = 1;                                             // Result = 00000001
    goto loc_80045994;
loc_80045988:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5994);                                 // Store to: 80075994
    v0 = 1;                                             // Result = 00000001
loc_80045994:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    return;
}

void psxspu_stop_master_fade() noexcept {
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5994);                                 // Store to: 80075994
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    return;
}

void psxspu_get_master_fade_status() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5994);                               // Load from: 80075994
    v0 = (i32(v0) < 2);
    v0 ^= 1;
    return;
}

void start_record_music_mute() noexcept {
loc_800459E0:
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5A10);                                // Store to: gpWess_pnotestate (80075A10)
    if (a0 == 0) goto loc_800459F4;
    sw(0, a0);
loc_800459F4:
    return;
}

void end_record_music_mute() noexcept {
loc_800459FC:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5A10);                                 // Store to: gpWess_pnotestate (80075A10)
    return;
}

void add_music_mute_note() noexcept {
loc_80045A0C:
    t0 = lw(sp + 0x10);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A10);                               // Load from: gpWess_pnotestate (80075A10)
    t1 = lw(sp + 0x14);
    if (v1 == 0) goto loc_80045AC4;
    v0 = lw(v1);
    v0 <<= 4;
    v0 += v1;
    sh(a0, v0 + 0x4);
    v0 = lw(v1);
    v0 <<= 4;
    v0 += v1;
    sh(a1, v0 + 0x6);
    v0 = lw(v1);
    v0 <<= 4;
    v0 += v1;
    sb(a2, v0 + 0x8);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A10);                               // Load from: gpWess_pnotestate (80075A10)
    v0 = lw(v1);
    v0 <<= 4;
    v1 += v0;
    sb(a3, v1 + 0x9);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A10);                               // Load from: gpWess_pnotestate (80075A10)
    v0 = lw(v1);
    v0 <<= 4;
    v0 += v1;
    sw(t0, v0 + 0xC);
    v0 = lw(v1);
    v0 <<= 4;
    v0 += v1;
    sw(t1, v0 + 0x10);
    v0 = lw(v1);
    v0++;
    sw(v0, v1);
loc_80045AC4:
    return;
}

void PSX_UNKNOWN_DrvFunc() noexcept {
    v1 = 0x10000000;                                    // Result = 10000000
    v0 = 0x1F;                                          // Result = 0000001F
    goto loc_80045AF0;
loc_80045AD8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A07);                              // Load from: gWess_UNKNOWN_status_byte (80075A07)
    {
        const bool bJump = (v0 == 0)
        v0 += 0xFF;
        if (bJump) goto loc_80045B04;
    }
    v1 = u32(i32(v1) >> 1);
loc_80045AF0:
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x5A07);                                // Store to: gWess_UNKNOWN_status_byte (80075A07)
    v0 = (i32(a0) < i32(v1));
    if (v0 != 0) goto loc_80045AD8;
loc_80045B04:
    return;
}

void TriggerPSXVoice() noexcept {
loc_80045B0C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = 0x60000;                                       // Result = 00060000
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    v1 = lbu(s0 + 0x2);
    v0 |= 0xE3;                                         // Result = 000600E3
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE6C);                                 // Store to: 8007F194
    v0 = 1;                                             // Result = 00000001
    a1 = v0 << v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(a1, at - 0xE70);                                 // Store to: 8007F190
    v1 = lbu(s0 + 0x3);
    v0 = v1 << 2;
    v0 += v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE94);                                // Load from: 8007F16C
    v0 <<= 4;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF5C);                                 // Store to: 8007F0A4
    v0 = lbu(v0 + 0x9);
    s1 = a2;
    if (v0 == 0) goto loc_80045BC8;
    v0 = lbu(s0 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0xE18;                                        // Result = 8007F1E8
    at += v0;
    v0 = lbu(at);
    if (v0 != 0) goto loc_80045C0C;
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuSetReverbVoice();
    v1 = lbu(s0 + 0x2);
    v0 = 0x7F;                                          // Result = 0000007F
    at = 0x80080000;                                    // Result = 80080000
    at -= 0xE18;                                        // Result = 8007F1E8
    at += v1;
    sb(v0, at);
    goto loc_80045C0C;
loc_80045BC8:
    v0 = lbu(s0 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0xE18;                                        // Result = 8007F1E8
    at += v0;
    v0 = lbu(at);
    if (v0 == 0) goto loc_80045C0C;
    a0 = 0;                                             // Result = 00000000
    LIBSPU_SpuSetReverbVoice();
    v0 = lbu(s0 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0xE18;                                        // Result = 8007F1E8
    at += v0;
    sb(0, at);
loc_80045C0C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    {
        const bool bJump = (v0 == 0)
        v0 = 0x40;                                      // Result = 00000040
        if (bJump) goto loc_80045C88;
    }
    v0 = lw(s0 + 0x8);
    v1 = lbu(v0 + 0x3);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF5C);                                // Load from: 8007F0A4
    v1 <<= 24;
    v0 = lbu(v0 + 0xD);
    v1 = u32(i32(v1) >> 24);
    v0 += v1;
    v0 -= 0x40;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xF58);                                 // Store to: 8007F0A8
    v0 = (i32(v0) < 0x80);
    {
        const bool bJump = (v0 != 0)
        v0 = 0x7F;                                      // Result = 0000007F
        if (bJump) goto loc_80045C64;
    }
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xF58);                                 // Store to: 8007F0A8
loc_80045C64:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lh(v0 - 0xF58);                                // Load from: 8007F0A8
    if (i32(v0) >= 0) goto loc_80045C90;
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xF58);                                  // Store to: 8007F0A8
    goto loc_80045C90;
loc_80045C88:
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xF58);                                 // Store to: 8007F0A8
loc_80045C90:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF5C);                                // Load from: 8007F0A4
    v0 = lbu(a0 + 0x13);
    if (v0 != 0) goto loc_80045CE4;
    v0 = lw(s0 + 0x8);
    v1 = lbu(v0 + 0x2);
    v0 = s1 & 0xFF;
    mult(v0, v1);
    v1 = lo;
    v0 = lbu(a0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A04);                              // Load from: gWess_master_sfx_volume (80075A04)
    mult(v1, v0);
    goto loc_80045D1C;
loc_80045CE4:
    v0 = lw(s0 + 0x8);
    v1 = lbu(v0 + 0x2);
    v0 = s1 & 0xFF;
    mult(v0, v1);
    v1 = lo;
    v0 = lbu(a0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A05);                              // Load from: gWess_master_mus_volume (80075A05)
    mult(v1, v0);
loc_80045D1C:
    v0 = lo;
    v0 = u32(i32(v0) >> 21);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF60);                                 // Store to: 8007F0A0
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    v0 = 1;                                             // Result = 00000001
    if (v1 != 0) goto loc_80045D68;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xF60);                               // Load from: 8007F0A0
    v0 <<= 6;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE68);                                 // Store to: 8007F198
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE66);                                 // Store to: 8007F19A
    goto loc_80045E00;
loc_80045D68:
    {
        const bool bJump = (v1 != v0)
        v0 = 0x80;                                      // Result = 00000080
        if (bJump) goto loc_80045DBC;
    }
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF60);                                // Load from: 8007F0A0
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xF58);                                // Load from: 8007F0A8
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE68);                                 // Store to: 8007F198
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE66);                                 // Store to: 8007F19A
    goto loc_80045E00;
loc_80045DBC:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF60);                                // Load from: 8007F0A0
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xF58);                                // Load from: 8007F0A8
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE66);                                 // Store to: 8007F19A
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE68);                                 // Store to: 8007F198
loc_80045E00:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF5C);                                // Load from: 8007F0A4
    v1 = lh(v0 + 0xE);
    if (v1 != 0) goto loc_80045E28;
    v0 = lbu(s0 + 0x5);
    v0 <<= 8;
    goto loc_80045EFC;
loc_80045E28:
    if (i32(v1) <= 0) goto loc_80045E84;
    v0 = lw(s0 + 0x8);
    v0 = lb(v0 + 0x9);
    mult(v1, v0);
    v0 = lo;
    v0 += 0x20;
    v1 = u32(i32(v0) >> 13);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF6C);                                 // Store to: 8007F094
    v0 &= 0x1FFF;
    v0 = u32(i32(v0) >> 6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF68);                                 // Store to: 8007F098
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF64);                                 // Store to: 8007F09C
    v0 = lbu(s0 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF68);                               // Load from: 8007F098
    v0 += v1;
    goto loc_80045EE8;
loc_80045E84:
    v0 = lw(s0 + 0x8);
    v0 = lb(v0 + 0x8);
    mult(v1, v0);
    v1 = 0x20;                                          // Result = 00000020
    v0 = lo;
    v1 -= v0;
    v0 = u32(i32(v1) >> 13);
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF6C);                                 // Store to: 8007F094
    v1 &= 0x1FFF;
    v1 = u32(i32(v1) >> 6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF68);                                 // Store to: 8007F098
    v0 = 0x80;                                          // Result = 00000080
    v0 -= v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF64);                                 // Store to: 8007F09C
    v0 = lbu(s0 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF68);                               // Load from: 8007F098
    v0 -= v1;
loc_80045EE8:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF64);                               // Load from: 8007F09C
    v0 <<= 8;
    v1 &= 0x7F;
    v0 |= v1;
loc_80045EFC:
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE5A);                                 // Store to: 8007F1A6
    v1 = lw(s0 + 0x8);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE58;                                        // Result = 8007F1A8
    v0 = lbu(v1 + 0x4);
    v1 = lbu(v1 + 0x5);
    v0 <<= 8;
    v0 |= v1;
    sh(v0, a0);                                         // Store to: 8007F1A8
    v0 = lw(s0 + 0xC);
    v0 = lw(v0 + 0x8);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE54);                                 // Store to: 8007F1AC
    v0 = lw(s0 + 0x8);
    v0 = lhu(v0 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE36);                                 // Store to: 8007F1CA
    v0 = lw(s0 + 0x8);
    v0 = lhu(v0 + 0xE);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE34);                                 // Store to: 8007F1CC
    a0 -= 0x18;                                         // Result = 8007F190
    LIBSPU_SpuSetKeyOnWithAttr();
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PSX_DriverInit() noexcept {
loc_80045F8C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(a0);
    v1 = lw(a0 + 0x20);
    a1 = lw(a0 + 0x28);
    a2 = lw(a0 + 0x30);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xE9C);                                 // Store to: 8007F164
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE84);                                 // Store to: 8007F17C
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xE98);                                 // Store to: gWess_Dvr_pss (8007F168)
    at = 0x80080000;                                    // Result = 80080000
    sw(a1, at - 0xE94);                                 // Store to: 8007F16C
    at = 0x80080000;                                    // Result = 80080000
    sw(a2, at - 0xF48);                                 // Store to: 8007F0B8
    v0 = lbu(a0 + 0x7);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xF54);                                 // Store to: 8007F0AC
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xF54);                               // Load from: 8007F0AC
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF4C);                                  // Store to: 8007F0B4
    if (v0 == 0) goto loc_8004604C;
    a3 = 1;                                             // Result = 00000001
    a1 = v0;
loc_80045FFC:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF4C);                                // Load from: 8007F0B4
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    a0 = v0 + a2;
    v0 = lbu(a0 + 0x1);
    {
        const bool bJump = (v0 != a3)
        v0 = v1 + 1;
        if (bJump) goto loc_80046038;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xE90);                                 // Store to: 8007F170
    goto loc_8004604C;
loc_80046038:
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF4C);                                 // Store to: 8007F0B4
    v0 = (i32(v0) < i32(a1));
    if (v0 != 0) goto loc_80045FFC;
loc_8004604C:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE9C);                                // Load from: 8007F164
    v0 = lw(v1 + 0xC);
    v0 = lbu(v0 + 0xA);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xF50);                                 // Store to: 8007F0B0
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xF50);                               // Load from: 8007F0B0
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF4C);                                  // Store to: 8007F0B4
    a3 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_800460E8;
    a2 = lw(v1 + 0x18);
    a1 = v0;
loc_80046090:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF4C);                                // Load from: 8007F0B4
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 += v1;
    v0 <<= 2;
    a0 = v0 + a2;
    v0 = lbu(a0 + 0x4);
    {
        const bool bJump = (v0 != a3)
        v0 = v1 + 1;
        if (bJump) goto loc_800460D4;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xE88);                                 // Store to: 8007F178
    goto loc_800460E8;
loc_800460D4:
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF4C);                                 // Store to: 8007F0B4
    v0 = (i32(v0) < i32(a1));
    if (v0 != 0) goto loc_80046090;
loc_800460E8:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xE88);                                // Load from: 8007F178
    v1 = lbu(a0 + 0x5);
    v0 = lh(a0 + 0x8);
    a1 = lw(a0 + 0x1C);
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xE8C);                                 // Store to: 8007F174
    v1 = lh(a0 + 0xC);
    a0 = lh(a0 + 0x10);
    v0 += a1;
    at = 0x80080000;                                    // Result = 80080000
    sw(a1, at - 0xE80);                                 // Store to: 8007F180
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE7C);                                 // Store to: 8007F184
    v1 <<= 4;
    v1 += v0;
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 2;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xE78);                                 // Store to: 8007F188
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE74);                                 // Store to: 8007F18C
    psxspu_init();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE18;                                        // Result = 8007F1E8
    v1 = 0x7F;                                          // Result = 0000007F
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF4C);                                  // Store to: 8007F0B4
loc_8004616C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF4C);                                // Load from: 8007F0B4
    v0 += a0;
    sb(v1, v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF4C);                                // Load from: 8007F0B4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF4C);                                 // Store to: 8007F0B4
    v0 = (i32(v0) < 0x18);
    if (v0 != 0) goto loc_8004616C;
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSX_DriverExit() noexcept {
loc_800461B4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBSPU_SpuQuit();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSX_DriverEntry1() noexcept {
loc_800461D4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE9C);                                // Load from: 8007F164
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lbu(v0 + 0x6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF44);                                 // Store to: 8007F0BC
    if (v0 == 0) goto loc_800462D8;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF34);                                 // Store to: 8007F0CC
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF40);                                 // Store to: 8007F0C0
    if (v1 == v0) goto loc_800462D8;
    s0 = 3;                                             // Result = 00000003
loc_80046238:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF34);                                // Load from: 8007F0CC
    v0 = lw(a0);
    v0 &= 3;
    if (v0 != s0) goto loc_800462A4;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE84);                                // Load from: 8007F17C
    v1 = lw(v0);
    v0 = lw(a0 + 0x10);
    v0 = (v0 < v1);
    if (v0 == 0) goto loc_800462A4;
    PSX_voiceparmoff();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF44);                                // Load from: 8007F0BC
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF44);                                 // Store to: 8007F0BC
    if (v0 == 0) goto loc_800462D8;
loc_800462A4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF34);                                // Load from: 8007F0CC
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF40);                                // Load from: 8007F0C0
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF34);                                 // Store to: 8007F0CC
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF40);                                 // Store to: 8007F0C0
    if (v1 != v0) goto loc_80046238;
loc_800462D8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A08);                               // Load from: 80075A08
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF30);                                  // Store to: 8007F0D0
    if (v0 == 0) goto loc_80046300;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF30);                                 // Store to: 8007F0D0
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5A08);                                 // Store to: 80075A08
loc_80046300:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A0C);                               // Load from: 80075A0C
    if (v0 == 0) goto loc_80046374;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x5A07);                              // Load from: gWess_UNKNOWN_status_byte (80075A07)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE70;                                        // Result = 8007F190
    sw(v0, a0);                                         // Store to: 8007F190
    v0 = 0x4400;                                        // Result = 00004400
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE6C);                                 // Store to: 8007F194
    v0 = 7;                                             // Result = 00000007
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE44);                                 // Store to: 8007F1BC
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at - 0xE3A);                                 // Store to: 8007F1C6
    LIBSPU_SpuSetVoiceAttr();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF30);                                // Load from: 8007F0D0
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A0C);                               // Load from: 80075A0C
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5A0C);                                 // Store to: 80075A0C
    v0 |= v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF30);                                 // Store to: 8007F0D0
loc_80046374:
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0xF30);                                // Load from: 8007F0D0
    if (a1 == 0) goto loc_80046398;
    a0 = 0;                                             // Result = 00000000
    LIBSPU_SpuSetKey();
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF30);                                  // Store to: 8007F0D0
loc_80046398:
    s0 = 0x80080000;                                    // Result = 80080000
    s0 -= 0xE30;                                        // Result = 8007F1D0
    a0 = s0;                                            // Result = 8007F1D0
    LIBSPU_SpuGetAllKeysStatus();
    s1 = s0;                                            // Result = 8007F1D0
    s0 = -1;                                            // Result = FFFFFFFF
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE90);                                // Load from: 8007F170
    v0 = 0x18;                                          // Result = 00000018
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF3C);                                 // Store to: 8007F0C4
    v0 = 0x17;                                          // Result = 00000017
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF38);                                  // Store to: 8007F0C8
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF3C);                                 // Store to: 8007F0C4
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF34);                                 // Store to: 8007F0CC
loc_800463E0:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF34);                                // Load from: 8007F0CC
    v0 = lw(a0);
    v0 &= 1;
    if (v0 == 0) goto loc_80046428;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF38);                                // Load from: 8007F0C8
    v0 += s1;
    v0 = lbu(v0);
    if (v0 != 0) goto loc_80046428;
    PSX_voiceparmoff();
loc_80046428:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF38);                                // Load from: 8007F0C8
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF3C);                                // Load from: 8007F0C4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF38);                                 // Store to: 8007F0C8
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF34);                                // Load from: 8007F0CC
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF3C);                                 // Store to: 8007F0C4
    v0 += 0x18;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF34);                                 // Store to: 8007F0CC
    if (v1 != s0) goto loc_800463E0;
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PSX_DriverEntry2() noexcept {
loc_80046484:
    return;
}

void PSX_DriverEntry3() noexcept {
loc_8004648C:
    return;
}

void PSX_TrkOff() noexcept {
loc_80046494:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lbu(s0 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE98);                                // Load from: gWess_Dvr_pss (8007F168)
    v0 <<= 3;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF2C);                                 // Store to: 8007F0D4
    PSX_TrkMute();
    v0 = lbu(s0 + 0x10);
    if (v0 == 0) goto loc_80046524;
    v0 = lw(s0);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF2C);                                // Load from: 8007F0D4
    v0 |= 0x88;
    sw(v0, s0);
    v0 = lbu(v1 + 0x5);
    v0--;
    sb(v0, v1 + 0x5);
    v0 &= 0xFF;
    if (v0 != 0) goto loc_8004652C;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF2C);                                // Load from: 8007F0D4
    sb(0, v0 + 0x1);
    goto loc_8004652C;
loc_80046524:
    a0 = s0;
    Eng_TrkOff();
loc_8004652C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSX_TrkMute() noexcept {
loc_80046540:
    sp -= 0x28;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    v0 = lbu(s0 + 0x10);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF28);                                 // Store to: 8007F0D8
    if (v0 == 0) goto loc_800466E4;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF20);                                 // Store to: 8007F0E0
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF24);                                 // Store to: 8007F0DC
    s1 = 1;                                             // Result = 00000001
    if (v1 == v0) goto loc_800466E4;
loc_8004659C:
    t0 = 0x80080000;                                    // Result = 80080000
    t0 = lw(t0 - 0xF20);                                // Load from: 8007F0E0
    a0 = lw(t0);
    v0 = a0 & 1;
    if (v0 == 0) goto loc_800466B0;
    v1 = lbu(t0 + 0x3);
    v0 = lbu(s0 + 0x1);
    if (v1 != v0) goto loc_800466B0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A10);                               // Load from: gpWess_pnotestate (80075A10)
    {
        const bool bJump = (v0 == 0)
        v0 = a0 & 2;
        if (bJump) goto loc_80046648;
    }
    if (v0 != 0) goto loc_80046648;
    v0 = lbu(s0 + 0x13);
    v1 = 0x1F;                                          // Result = 0000001F
    if (v0 != s1) goto loc_8004664C;
    v1 = lbu(s0 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE98);                                // Load from: gWess_Dvr_pss (8007F168)
    v0 <<= 3;
    v0 += v1;
    v1 = lw(t0 + 0x8);
    a0 = lh(v0 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF1C);                                 // Store to: 8007F0E4
    a1 = lbu(t0 + 0x3);
    a2 = lbu(t0 + 0x5);
    a3 = lbu(t0 + 0x6);
    sw(v1, sp + 0x10);
    v0 = lw(t0 + 0xC);
    sw(v0, sp + 0x14);
    add_music_mute_note();
loc_80046648:
    v1 = 0x1F;                                          // Result = 0000001F
loc_8004664C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A07);                              // Load from: gWess_UNKNOWN_status_byte (80075A07)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF20);                                // Load from: 8007F0E0
    v1 -= v0;
    v0 = 0x10000000;                                    // Result = 10000000
    v0 = i32(v0) >> v1;
    sw(v0, a0 + 0x14);
    PSX_voicerelease();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF20);                                // Load from: 8007F0E0
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF28);                                // Load from: 8007F0D8
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A0C);                               // Load from: 80075A0C
    v0 = lbu(v0 + 0x2);
    a0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xF28);                                 // Store to: 8007F0D8
    v0 = s1 << v0;
    v0 |= v1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5A0C);                                // Store to: 80075A0C
    if (a0 == 0) goto loc_800466E4;
loc_800466B0:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF20);                                // Load from: 8007F0E0
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF24);                                // Load from: 8007F0DC
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF20);                                 // Store to: 8007F0E0
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF24);                                 // Store to: 8007F0DC
    if (v1 != v0) goto loc_8004659C;
loc_800466E4:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void PSX_PatchChg() noexcept {
loc_800466FC:
    v1 = lw(a0 + 0x34);
    v0 = lbu(v1 + 0x2);
    v1 = lbu(v1 + 0x1);
    v0 <<= 8;
    v1 |= v0;
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at - 0xF18);                                 // Store to: gWess_Dvr_thepatch (8007F0E8)
    sh(v1, a0 + 0xA);
    return;
}

void PSX_PatchMod() noexcept {
loc_80046724:
    return;
}

void PSX_PitchMod() noexcept {
loc_8004672C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x34);
    v1 = lbu(v0 + 0x2);
    v0 = lbu(v0 + 0x1);
    v1 <<= 8;
    v0 |= v1;
    a0 = v0;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xEFC);                                 // Store to: 8007F104
    v0 <<= 16;
    v1 = lh(s0 + 0xE);
    v0 = u32(i32(v0) >> 16);
    if (v1 == v0) goto loc_80046960;
    v0 = lbu(s0 + 0x10);
    sh(a0, s0 + 0xE);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF14);                                 // Store to: 8007F0EC
    if (v0 == 0) goto loc_80046960;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF00);                                 // Store to: 8007F100
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF10);                                 // Store to: 8007F0F0
    s2 = 0x20;                                          // Result = 00000020
    if (v1 == v0) goto loc_80046960;
    s1 = 0x80080000;                                    // Result = 80080000
    s1 -= 0xE70;                                        // Result = 8007F190
loc_800467CC:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF00);                                // Load from: 8007F100
    v0 = lw(a0);
    v0 &= 1;
    if (v0 == 0) goto loc_8004692C;
    v1 = lbu(a0 + 0x3);
    v0 = lbu(s0 + 0x1);
    {
        const bool bJump = (v1 != v0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8004692C;
    }
    v1 = lbu(a0 + 0x2);
    sw(s2, s1 + 0x4);                                   // Store to: 8007F194
    v0 = v0 << v1;
    sw(v0, s1);                                         // Store to: 8007F190
    v1 = lh(s0 + 0xE);
    if (v1 != 0) goto loc_8004682C;
    v0 = lbu(a0 + 0x5);
    v0 <<= 8;
    goto loc_800468FC;
loc_8004682C:
    if (i32(v1) <= 0) goto loc_80046888;
    v0 = lw(a0 + 0x8);
    v0 = lb(v0 + 0x9);
    mult(v1, v0);
    v0 = lo;
    v0 += 0x20;
    v1 = u32(i32(v0) >> 13);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF0C);                                 // Store to: 8007F0F4
    v0 &= 0x1FFF;
    v0 = u32(i32(v0) >> 6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF08);                                 // Store to: 8007F0F8
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF04);                                 // Store to: 8007F0FC
    v0 = lbu(a0 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF08);                               // Load from: 8007F0F8
    v0 += v1;
    goto loc_800468E8;
loc_80046888:
    v0 = lw(a0 + 0x8);
    v0 = lb(v0 + 0x8);
    mult(v1, v0);
    v1 = lo;
    v1 = s2 - v1;
    v0 = u32(i32(v1) >> 13);
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF0C);                                 // Store to: 8007F0F4
    v1 &= 0x1FFF;
    v1 = u32(i32(v1) >> 6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF08);                                 // Store to: 8007F0F8
    v0 = 0x80;                                          // Result = 00000080
    v0 -= v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF04);                                 // Store to: 8007F0FC
    v0 = lbu(a0 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF08);                               // Load from: 8007F0F8
    v0 -= v1;
loc_800468E8:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF04);                               // Load from: 8007F0FC
    v0 <<= 8;
    v1 &= 0x7F;
    v0 |= v1;
loc_800468FC:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE70;                                        // Result = 8007F190
    sh(v0, s1 + 0x16);                                  // Store to: 8007F1A6
    LIBSPU_SpuSetVoiceAttr();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF14);                                // Load from: 8007F0EC
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF14);                                 // Store to: 8007F0EC
    if (v0 == 0) goto loc_80046960;
loc_8004692C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF00);                                // Load from: 8007F100
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF10);                                // Load from: 8007F0F0
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF00);                                 // Store to: 8007F100
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF10);                                 // Store to: 8007F0F0
    if (v1 != v0) goto loc_800467CC;
loc_80046960:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PSX_ZeroMod() noexcept {
loc_8004697C:
    return;
}

void PSX_ModuMod() noexcept {
loc_80046984:
    return;
}

void PSX_VolumeMod() noexcept {
loc_8004698C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x34);
    v1 = lbu(s0 + 0x10);
    v0 = lbu(v0 + 0x1);
    sb(v0, s0 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEF8);                                 // Store to: 8007F108
    if (v1 == 0) goto loc_80046C88;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEEC);                                 // Store to: 8007F114
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEF4);                                 // Store to: 8007F10C
    s2 = 1;                                             // Result = 00000001
    if (v1 == v0) goto loc_80046C88;
    s1 = 0x80080000;                                    // Result = 80080000
    s1 -= 0xE70;                                        // Result = 8007F190
loc_80046A00:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEEC);                                // Load from: 8007F114
    v0 = lw(a0);
    v0 &= 1;
    if (v0 == 0) goto loc_80046C54;
    v1 = lbu(a0 + 0x3);
    v0 = lbu(s0 + 0x1);
    if (v1 != v0) goto loc_80046C54;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    {
        const bool bJump = (v0 == 0)
        v0 = 0x40;                                      // Result = 00000040
        if (bJump) goto loc_80046AA8;
    }
    v0 = lw(a0 + 0x8);
    v1 = lbu(v0 + 0x3);
    v0 = lbu(s0 + 0xD);
    v1 <<= 24;
    v1 = u32(i32(v1) >> 24);
    v0 += v1;
    v0 -= 0x40;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xEE8);                                 // Store to: 8007F118
    v0 = (i32(v0) < 0x80);
    {
        const bool bJump = (v0 != 0)
        v0 = 0x7F;                                      // Result = 0000007F
        if (bJump) goto loc_80046A84;
    }
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xEE8);                                 // Store to: 8007F118
loc_80046A84:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lh(v0 - 0xEE8);                                // Load from: 8007F118
    if (i32(v0) >= 0) goto loc_80046AB0;
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xEE8);                                  // Store to: 8007F118
    goto loc_80046AB0;
loc_80046AA8:
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xEE8);                                 // Store to: 8007F118
loc_80046AB0:
    v0 = lbu(s0 + 0x13);
    if (v0 != 0) goto loc_80046B04;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEEC);                                // Load from: 8007F114
    v1 = lw(v0 + 0x8);
    a0 = lbu(v0 + 0x6);
    v0 = lbu(v1 + 0x2);
    mult(a0, v0);
    v1 = lo;
    v0 = lbu(s0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A04);                              // Load from: gWess_master_sfx_volume (80075A04)
    mult(v1, v0);
    goto loc_80046B48;
loc_80046B04:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEEC);                                // Load from: 8007F114
    v1 = lw(v0 + 0x8);
    a0 = lbu(v0 + 0x6);
    v0 = lbu(v1 + 0x2);
    mult(a0, v0);
    v1 = lo;
    v0 = lbu(s0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A05);                              // Load from: gWess_master_mus_volume (80075A05)
    mult(v1, v0);
loc_80046B48:
    v0 = lo;
    v0 = u32(i32(v0) >> 21);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEF0);                                 // Store to: 8007F110
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEEC);                                // Load from: 8007F114
    v1 = lbu(v0 + 0x2);
    v0 = 3;                                             // Result = 00000003
    sw(v0, s1 + 0x4);                                   // Store to: 8007F194
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    v1 = s2 << v1;
    sw(v1, s1);                                         // Store to: 8007F190
    if (v0 != 0) goto loc_80046BA0;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xEF0);                               // Load from: 8007F110
    v0 <<= 6;
    sh(v0, s1 + 0x8);                                   // Store to: 8007F198
    sh(v0, s1 + 0xA);                                   // Store to: 8007F19A
    goto loc_80046C24;
loc_80046BA0:
    {
        const bool bJump = (v0 != s2)
        v0 = 0x80;                                      // Result = 00000080
        if (bJump) goto loc_80046BE8;
    }
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEF0);                                // Load from: 8007F110
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xEE8);                                // Load from: 8007F118
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0x8);                                   // Store to: 8007F198
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0xA);                                   // Store to: 8007F19A
    goto loc_80046C24;
loc_80046BE8:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEF0);                                // Load from: 8007F110
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xEE8);                                // Load from: 8007F118
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0xA);                                   // Store to: 8007F19A
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0x8);                                   // Store to: 8007F198
loc_80046C24:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE70;                                        // Result = 8007F190
    LIBSPU_SpuSetVoiceAttr();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEF8);                                // Load from: 8007F108
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEF8);                                 // Store to: 8007F108
    if (v0 == 0) goto loc_80046C88;
loc_80046C54:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEEC);                                // Load from: 8007F114
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xEF4);                                // Load from: 8007F10C
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEEC);                                 // Store to: 8007F114
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEF4);                                 // Store to: 8007F10C
    if (v1 != v0) goto loc_80046A00;
loc_80046C88:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PSX_PanMod() noexcept {
loc_80046CA4:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x34);
    v0 = lbu(v0 + 0x1);
    sb(v0, s0 + 0xD);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    if (v0 == 0) goto loc_80046F64;
    v0 = lbu(s0 + 0x10);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEE4);                                 // Store to: 8007F11C
    if (v0 == 0) goto loc_80046F64;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xED8);                                 // Store to: 8007F128
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEE0);                                 // Store to: 8007F120
    s2 = 1;                                             // Result = 00000001
    if (v1 == v0) goto loc_80046F64;
    s1 = 0x80080000;                                    // Result = 80080000
    s1 -= 0xE70;                                        // Result = 8007F190
loc_80046D34:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xED8);                                // Load from: 8007F128
    v0 = lw(a0);
    v0 &= 1;
    if (v0 == 0) goto loc_80046F30;
    v1 = lbu(a0 + 0x3);
    v0 = lbu(s0 + 0x1);
    if (v1 != v0) goto loc_80046F30;
    v0 = lw(a0 + 0x8);
    v1 = lbu(v0 + 0x3);
    v0 = lbu(s0 + 0xD);
    v1 <<= 24;
    v1 = u32(i32(v1) >> 24);
    v0 += v1;
    v0 -= 0x40;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xED4);                                 // Store to: 8007F12C
    v0 = (i32(v0) < 0x80);
    {
        const bool bJump = (v0 != 0)
        v0 = 0x7F;                                      // Result = 0000007F
        if (bJump) goto loc_80046DA4;
    }
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xED4);                                 // Store to: 8007F12C
loc_80046DA4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lh(v0 - 0xED4);                                // Load from: 8007F12C
    if (i32(v0) >= 0) goto loc_80046DC0;
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xED4);                                  // Store to: 8007F12C
loc_80046DC0:
    v0 = lbu(s0 + 0x13);
    if (v0 != 0) goto loc_80046E08;
    v0 = lw(a0 + 0x8);
    v1 = lbu(a0 + 0x6);
    v0 = lbu(v0 + 0x2);
    mult(v1, v0);
    v1 = lo;
    v0 = lbu(s0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A04);                              // Load from: gWess_master_sfx_volume (80075A04)
    mult(v1, v0);
    goto loc_80046E40;
loc_80046E08:
    v0 = lw(a0 + 0x8);
    v1 = lbu(a0 + 0x6);
    v0 = lbu(v0 + 0x2);
    mult(v1, v0);
    v1 = lo;
    v0 = lbu(s0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A05);                              // Load from: gWess_master_mus_volume (80075A05)
    mult(v1, v0);
loc_80046E40:
    v0 = lo;
    v0 = u32(i32(v0) >> 21);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEDC);                                 // Store to: 8007F124
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xED8);                                // Load from: 8007F128
    v1 = lbu(v0 + 0x2);
    v0 = 3;                                             // Result = 00000003
    sw(v0, s1 + 0x4);                                   // Store to: 8007F194
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    v1 = s2 << v1;
    sw(v1, s1);                                         // Store to: 8007F190
    if (v0 != s2) goto loc_80046EC0;
    v0 = 0x80;                                          // Result = 00000080
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEDC);                                // Load from: 8007F124
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xED4);                                // Load from: 8007F12C
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0x8);                                   // Store to: 8007F198
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0xA);                                   // Store to: 8007F19A
    goto loc_80046F00;
loc_80046EC0:
    v0 = 0x80;                                          // Result = 00000080
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEDC);                                // Load from: 8007F124
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xED4);                                // Load from: 8007F12C
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0xA);                                   // Store to: 8007F19A
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0x8);                                   // Store to: 8007F198
loc_80046F00:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE70;                                        // Result = 8007F190
    LIBSPU_SpuSetVoiceAttr();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEE4);                                // Load from: 8007F11C
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEE4);                                 // Store to: 8007F11C
    if (v0 == 0) goto loc_80046F64;
loc_80046F30:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xED8);                                // Load from: 8007F128
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xEE0);                                // Load from: 8007F120
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xED8);                                 // Store to: 8007F128
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEE0);                                 // Store to: 8007F120
    if (v1 != v0) goto loc_80046D34;
loc_80046F64:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PSX_PedalMod() noexcept {
loc_80046F80:
    return;
}

void PSX_ReverbMod() noexcept {
loc_80046F88:
    return;
}

void PSX_ChorusMod() noexcept {
loc_80046F90:
    return;
}

void PSX_voiceon() noexcept {
loc_80046F98:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(a0);
    v1 = -3;                                            // Result = FFFFFFFD
    v0 |= 1;
    v0 &= v1;
    sw(v0, a0);
    v0 = lbu(a1 + 0x1);
    t0 = lbu(sp + 0x28);
    t1 = lbu(sp + 0x2C);
    sb(v0, a0 + 0x3);
    v0 = lbu(a1 + 0x8);
    sb(t0, a0 + 0x5);
    sb(t1, a0 + 0x6);
    sb(0, a0 + 0x7);
    sb(v0, a0 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE84);                                // Load from: 8007F17C
    sw(a2, a0 + 0x8);
    sw(a3, a0 + 0xC);
    v0 = lw(v0);
    sw(v0, a0 + 0x10);
    v1 = lhu(a2 + 0xE);
    v0 = v1 & 0x20;
    {
        const bool bJump = (v0 == 0)
        v0 = v1 & 0x1F;
        if (bJump) goto loc_80047018;
    }
    v1 = 0x1F;                                          // Result = 0000001F
    v1 -= v0;
    v0 = 0x10000000;                                    // Result = 10000000
    goto loc_80047024;
loc_80047018:
    v1 = 0x1F;                                          // Result = 0000001F
    v1 -= v0;
    v0 = 0x5DC0000;                                     // Result = 05DC0000
loc_80047024:
    v0 = i32(v0) >> v1;
    sw(v0, a0 + 0x14);
    v0 = lbu(a1 + 0x10);
    v0++;
    sb(v0, a1 + 0x10);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE9C);                                // Load from: 8007F164
    a2 = t1;
    v0 = lbu(v1 + 0x6);
    a1 = t0;
    v0++;
    sb(v0, v1 + 0x6);
    TriggerPSXVoice();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSX_voiceparmoff() noexcept {
loc_8004706C:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lbu(s0 + 0x3);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xE9C);                                // Load from: 8007F164
    v0 = v1 << 2;
    v0 += v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE94);                                // Load from: 8007F16C
    v0 <<= 4;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xED0);                                 // Store to: 8007F130
    v0 = lbu(a0 + 0x6);
    v0--;
    sb(v0, a0 + 0x6);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xED0);                                // Load from: 8007F130
    v0 = lbu(v1 + 0x10);
    v0--;
    sb(v0, v1 + 0x10);
    v0 &= 0xFF;
    if (v0 != 0) goto loc_80047108;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xED0);                                // Load from: 8007F130
    v0 = lw(a0);
    v0 &= 0x80;
    if (v0 == 0) goto loc_80047108;
    Eng_TrkOff();
loc_80047108:
    v0 = lw(s0);
    v1 = -2;                                            // Result = FFFFFFFE
    v0 &= v1;
    v1 = -3;                                            // Result = FFFFFFFD
    v0 &= v1;
    sw(v0, s0);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSX_voicerelease() noexcept {
loc_80047134:
    v0 = 1;                                             // Result = 00000001
    v1 = lbu(a0 + 0x2);
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5A08);                               // Load from: 80075A08
    v0 = v0 << v1;
    v0 |= a1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5A08);                                // Store to: 80075A08
    v0 = lw(a0);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE84);                                // Load from: 8007F17C
    v0 |= 2;
    sw(v0, a0);
    v0 = lw(v1);
    v1 = lw(a0 + 0x14);
    v0 += v1;
    sw(v0, a0 + 0x10);
    return;
}

void PSX_voicenote() noexcept {
loc_80047180:
    sp -= 0x30;
    sw(s4, sp + 0x28);
    s4 = lbu(sp + 0x40);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(s2, sp + 0x20);
    s2 = a1;
    sw(s3, sp + 0x24);
    s3 = a2;
    sw(s1, sp + 0x1C);
    sw(ra, sp + 0x2C);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xEC8);                                  // Store to: 8007F138
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEC4);                                 // Store to: 8007F13C
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xECC);                                 // Store to: 8007F134
    s1 = a3;
    if (v1 == v0) goto loc_80047328;
    a2 = 1;                                             // Result = 00000001
loc_800471E8:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEC4);                                // Load from: 8007F13C
    a1 = lw(a0);
    v0 = a1 & 1;
    a3 = s3;
    if (v0 != 0) goto loc_80047230;
    a1 = s0;
    a2 = s2;
    v0 = s1 & 0xFF;
    sw(v0, sp + 0x10);
    sw(s4, sp + 0x14);
    PSX_voiceon();
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xEC8);                                  // Store to: 8007F138
    goto loc_80047328;
loc_80047230:
    v1 = lbu(a0 + 0x4);
    v0 = lbu(s0 + 0x8);
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_800472F4;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A18);                               // Load from: 80075A18
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 != 0)
        v0 = a1 & 2;
        if (bJump) goto loc_800472CC;
    }
    if (v0 == 0) goto loc_80047290;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A14);                               // Load from: 80075A14
    v0 = lw(v0);
    v0 &= 2;
    if (v0 == 0) goto loc_800472CC;
    goto loc_800472B0;
loc_80047290:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A14);                               // Load from: 80075A14
    v0 = lw(v0);
    v0 &= 2;
    if (v0 != 0) goto loc_800472F4;
loc_800472B0:
    v0 = lw(a0 + 0x10);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A1C);                               // Load from: 80075A1C
    v0 = (v0 < v1);
    if (v0 == 0) goto loc_800472F4;
loc_800472CC:
    at = 0x80080000;                                    // Result = 80080000
    sw(a2, at - 0xEC8);                                 // Store to: 8007F138
    v0 = lbu(a0 + 0x4);
    v1 = lw(a0 + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5A14);                                // Store to: 80075A14
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5A18);                                // Store to: 80075A18
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5A1C);                                // Store to: 80075A1C
loc_800472F4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEC4);                                // Load from: 8007F13C
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xECC);                                // Load from: 8007F134
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEC4);                                 // Store to: 8007F13C
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xECC);                                 // Store to: 8007F134
    if (v1 != v0) goto loc_800471E8;
loc_80047328:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEC8);                                // Load from: 8007F138
    if (v0 == 0) goto loc_80047370;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5A14);                               // Load from: 80075A14
    PSX_voiceparmoff();
    a1 = s0;
    a2 = s2;
    a3 = s3;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5A14);                               // Load from: 80075A14
    v0 = s1 & 0xFF;
    sw(v0, sp + 0x10);
    sw(s4, sp + 0x14);
    PSX_voiceon();
loc_80047370:
    ra = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void PSX_NoteOn() noexcept {
loc_80047394:
    sp -= 0x20;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x1C);
    v0 = lw(s0 + 0x34);
    v1 = lw(s0 + 0x34);
    v0 = lbu(v0 + 0x1);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xEBC);                                 // Store to: 8007F144
    v0 = lbu(v1 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xEB8);                                 // Store to: 8007F148
    v1 = lbu(s0 + 0x13);
    v0 = 2;                                             // Result = 00000002
    if (v1 != v0) goto loc_80047428;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xEBC);                               // Load from: 8007F144
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE74);                                // Load from: 8007F18C
    v1 <<= 2;
    v1 += v0;
    v0 = lh(v1);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xE80);                                // Load from: 8007F180
    v0 <<= 2;
    v0 += a0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEB0);                                 // Store to: 8007F150
    v0 = lbu(v1 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xEBC);                                 // Store to: 8007F144
    goto loc_80047444;
loc_80047428:
    v0 = lh(s0 + 0xA);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE80);                                // Load from: 8007F180
    v0 <<= 2;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEB0);                                 // Store to: 8007F150
loc_80047444:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEB0);                                // Load from: 8007F150
    v1 = lbu(v0);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xEC0);                                  // Store to: 8007F140
    v0 = v1 - 1;
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xEB4);                                 // Store to: 8007F14C
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xEB4);                                 // Store to: 8007F14C
    v0 &= 0xFF;
    v1 = 0xFF;                                          // Result = 000000FF
    if (v0 == v1) goto loc_80047564;
loc_80047480:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEB0);                                // Load from: 8007F150
    v1 = lh(v0 + 0x2);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEC0);                                // Load from: 8007F140
    v1 += v0;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE7C);                                // Load from: 8007F184
    v1 <<= 4;
    a1 = v1 + v0;
    v1 = lh(a1 + 0xA);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE78);                                // Load from: 8007F188
    v0 <<= 2;
    a2 = v0 + v1;
    v0 = lw(a2 + 0x8);
    at = 0x80080000;                                    // Result = 80080000
    sw(a1, at - 0xEAC);                                 // Store to: 8007F154
    at = 0x80080000;                                    // Result = 80080000
    sw(a2, at - 0xEA8);                                 // Store to: 8007F158
    if (v0 == 0) goto loc_8004752C;
    v0 = lbu(a1 + 0x6);
    a3 = 0x80080000;                                    // Result = 80080000
    a3 = lbu(a3 - 0xEBC);                               // Load from: 8007F144
    v0 = (a3 < v0);
    if (v0 != 0) goto loc_8004752C;
    v0 = lbu(a1 + 0x7);
    v0 = (v0 < a3);
    a0 = s0;
    if (v0 != 0) goto loc_8004752C;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xEB8);                               // Load from: 8007F148
    sw(v0, sp + 0x10);
    PSX_voicenote();
loc_8004752C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xEB4);                               // Load from: 8007F14C
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xEC0);                                // Load from: 8007F140
    v0--;
    v1++;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xEB4);                                 // Store to: 8007F14C
    v0 &= 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEC0);                                 // Store to: 8007F140
    v1 = 0xFF;                                          // Result = 000000FF
    if (v0 != v1) goto loc_80047480;
loc_80047564:
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void PSX_NoteOff() noexcept {
loc_80047578:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    sp -= 0x20;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEA0);                                 // Store to: 8007F160
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEA4);                                 // Store to: 8007F15C
    s0 = a0;
    if (v1 == v0) goto loc_80047648;
    s2 = 1;                                             // Result = 00000001
    s1 = -1;                                            // Result = FFFFFFFF
loc_800475C4:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEA0);                                // Load from: 8007F160
    v0 = lw(a0);
    v0 &= 3;
    if (v0 != s2) goto loc_80047618;
    v0 = lw(s0 + 0x34);
    v1 = lbu(a0 + 0x5);
    v0 = lbu(v0 + 0x1);
    if (v1 != v0) goto loc_80047618;
    v1 = lbu(a0 + 0x3);
    v0 = lbu(s0 + 0x1);
    if (v1 != v0) goto loc_80047618;
    PSX_voicerelease();
loc_80047618:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEA0);                                // Load from: 8007F160
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xEA4);                                // Load from: 8007F15C
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEA0);                                 // Store to: 8007F160
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEA4);                                 // Store to: 8007F15C
    if (v1 != s1) goto loc_800475C4;
loc_80047648:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
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

void psx_main() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x7E30;                                       // Result = gPSXCD_cbsyncsave (80077E30)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x613C;                                       // Result = 800A9EC4
loc_80050724:
    sw(0, v0);
    v0 += 4;
    at = (v0 < v1);
    if (at != 0) goto loc_80050724;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BF8);                               // Load from: StackEndAddr (80077BF8)
    v0 = addi(v0, -0x8);
    t0 = 0x80000000;                                    // Result = 80000000
    sp = v0 | t0;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x613C;                                       // Result = 800A9EC4
    a0 <<= 3;                                           // Result = 0054F620
    a0 >>= 3;                                           // Result = 000A9EC4
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BFC);                               // Load from: StackSize (80077BFC)
    a1 = v0 - v1;
    a1 -= a0;
    a0 |= t0;                                           // Result = 800A9EC4
    at = 0x80070000;                                    // Result = 80070000
    sw(ra, at + 0x7E58);                                // Store to: gProgramReturnAddr (80077E58)
    gp = 0x80070000;                                    // Result = 80070000
    gp += 0x75E0;                                       // Result = GPU_REG_GP0 (800775E0)
    fp = sp;
    a0 = addi(a0, 0x4);                                 // Result = 800A9EC8
    LIBAPI_InitHeap();
    ra = 0x80070000;                                    // Result = 80070000
    ra = lw(ra + 0x7E58);                               // Load from: gProgramReturnAddr (80077E58)
    I_Main();
    _break(0x1);
}
