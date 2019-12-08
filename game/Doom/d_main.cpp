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

void D_memset() noexcept {
loc_80012850:
    v0 = a0 & 3;
    v1 = a1;
    if (v0 == 0) goto loc_80012880;
    a2--;
loc_80012860:
    if (i32(a2) < 0) goto loc_80012904;
    sb(v1, a0);
    a0++;
    v0 = a0 & 3;
    a2--;
    if (v0 != 0) goto loc_80012860;
    a2++;
loc_80012880:
    v0 = a1 << 24;
    v1 = a1 << 16;
    v0 |= v1;
    v1 = a1 << 8;
    v0 |= v1;
    a1 |= v0;
    v0 = (i32(a2) < 0x20);
    a3 = a0;
    if (v0 != 0) goto loc_800128DC;
    a0 += 4;
loc_800128A8:
    sw(a1, a0 + 0x18);
    sw(a1, a0 + 0x14);
    sw(a1, a0 + 0x10);
    sw(a1, a0 + 0xC);
    sw(a1, a0 + 0x8);
    sw(a1, a0 + 0x4);
    sw(a1, a0);
    a0 += 0x20;
    sw(a1, a3);
    a2 -= 0x20;
    v0 = (i32(a2) < 0x20);
    a3 += 0x20;
    if (v0 == 0) goto loc_800128A8;
loc_800128DC:
    a2--;
    v0 = -1;                                            // Result = FFFFFFFF
    a0 = a3;
    if (a2 == v0) goto loc_80012904;
    v0 = a1;
    v1 = -1;                                            // Result = FFFFFFFF
loc_800128F4:
    sb(v0, a0);
    a2--;
    a0++;
    if (a2 != v1) goto loc_800128F4;
loc_80012904:
    return;
}

void D_memcpy() noexcept {
loc_8001290C:
    sp -= 8;
    v1 = a2 - 1;
    if (a2 == 0) goto loc_80012934;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8001291C:
    v0 = lbu(a1);
    a1++;
    v1--;
    sb(v0, a0);
    a0++;
    if (v1 != a2) goto loc_8001291C;
loc_80012934:
    sp += 8;
    return;
}

void D_strncpy() noexcept {
loc_80012940:
    sp -= 8;
    v1 = a2 - 1;
    if (a2 == 0) goto loc_80012970;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80012950:
    v0 = lbu(a1);
    a1++;
    sb(v0, a0);
    a0++;
    if (v0 == 0) goto loc_80012970;
    v1--;
    if (v1 != a2) goto loc_80012950;
loc_80012970:
    sp += 8;
    return;
}

void D_strncasecmp() noexcept {
loc_8001297C:
    v0 = lbu(a0);
    if (v0 == 0) goto loc_800129BC;
    v1 = lbu(a1);
    if (v1 == 0) goto loc_800129C0;
    {
        const bool bJump = (v0 != v1)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800129CC;
    }
    a0++;
    a2--;
    a1++;
    if (a2 != 0) goto loc_8001297C;
    v0 = 0;                                             // Result = 00000000
    goto loc_800129CC;
loc_800129BC:
    v1 = lbu(a1);
loc_800129C0:
    v0 ^= v1;
    v0 = (v0 > 0);
loc_800129CC:
    return;
}

void strupr() noexcept {
    v1 = lbu(a0);
    v0 = v1 - 0x61;
    if (v1 == 0) goto loc_80012A10;
loc_800129E4:
    v0 &= 0xFF;
    v0 = (v0 < 0x1A);
    if (v0 == 0) goto loc_800129F8;
    v1 -= 0x20;
loc_800129F8:
    sb(v1, a0);
    a0++;
    v1 = lbu(a0);
    v0 = v1 - 0x61;
    if (v1 != 0) goto loc_800129E4;
loc_80012A10:
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
