#include "r_sky.h"

#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGPU.h"

void R_DrawSky() noexcept {
loc_8002C07C:
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s0, sp + 0x20);
    v1 = lw(a1 + 0x1C);
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 != v0) goto loc_8002C120;
    v1 = lbu(a1 + 0x8);
    v0 = lhu(a1 + 0xA);
    v1 >>= 1;
    v0 &= 0xF;
    v0 <<= 6;
    v1 += v0;
    sh(v1, sp + 0x18);
    v1 = lhu(a1 + 0xA);
    a0 = lbu(a1 + 0x9);
    v0 = 0x20;                                          // Result = 00000020
    sh(v0, sp + 0x1C);
    v0 = 0x80;                                          // Result = 00000080
    sh(v0, sp + 0x1E);
    v1 &= 0x10;
    v1 <<= 4;
    a0 += v1;
    sh(a0, sp + 0x1A);
    v0 = lh(a1 + 0x10);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7DC4);                               // Load from: gpLumpCache (8007823C)
    v0 <<= 2;
    v0 += v1;
    a1 = lw(v0);
    a0 = sp + 0x18;
    a1 += 8;
    LIBGPU_LoadImage();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C10);                               // Load from: gNumFramesDrawn (80077C10)
    sw(v0, v1 + 0x1C);
loc_8002C120:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    v0 = lbu(v1 + 0x8);
    a0 = s0;                                            // Result = 1F800200
    sh(v0, sp + 0x18);
    v0 = lbu(v1 + 0x9);
    a1 = 0;                                             // Result = 00000000
    sh(v0, sp + 0x1A);
    a2 = lbu(v1 + 0x4);
    v0 = 0x80;                                          // Result = 00000080
    sh(v0, sp + 0x1E);
    v0 = sp + 0x18;
    sh(a2, sp + 0x1C);
    a3 = lhu(v1 + 0xA);
    a2 = 0;                                             // Result = 00000000
    sw(v0, sp + 0x10);
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
loc_8002C1A8:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8002C210;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_8002C2D4;
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
loc_8002C210:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8002C2C4;
    if (v1 == a0) goto loc_8002C1A8;
loc_8002C234:
    v0 = lw(gp + 0x5D8);                                // Load from: GPU_REG_GP1 (80077BB8)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_8002C1A8;
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
    if (a1 == t4) goto loc_8002C2A0;
    a3 = -1;                                            // Result = FFFFFFFF
loc_8002C284:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x5D4);                                // Load from: GPU_REG_GP0 (80077BB4)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_8002C284;
loc_8002C2A0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8002C1A8;
    goto loc_8002C234;
loc_8002C2C4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_8002C2D4:
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
    if (t0 == v0) goto loc_8002C334;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8002C31C:
    v0 = lw(s0);
    s0 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_8002C31C;
loc_8002C334:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_8002C3E8;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8002C360:
    v0 = lw(gp + 0x5D8);                                // Load from: GPU_REG_GP1 (80077BB8)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8002C3E8;
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
    if (a1 == t0) goto loc_8002C3CC;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8002C3B0:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x5D4);                                // Load from: GPU_REG_GP0 (80077BB4)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8002C3B0;
loc_8002C3CC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8002C360;
loc_8002C3E8:
    t0 = 4;                                             // Result = 00000004
    t1 = 0x1F800000;                                    // Result = 1F800000
    t1 += 0x204;                                        // Result = 1F800204
    t5 = 0x10;                                          // Result = 00000010
    t6 = 0x14;                                          // Result = 00000014
    t2 = 0xFF0000;                                      // Result = 00FF0000
    t2 |= 0xFFFF;                                       // Result = 00FFFFFF
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    t8 = s0 & t2;                                       // Result = 00086550
    t7 = 0x4000000;                                     // Result = 04000000
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FB0);                               // Load from: gpSkyTexture (80078050)
    v0 = 4;                                             // Result = 00000004
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v0 = 0x65;                                          // Result = 00000065
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    v0 = 0x100;                                         // Result = 00000100
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x208);                                  // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x20A);                                  // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    v0 = lhu(a0 + 0x6);
    t4 = 0x80000000;                                    // Result = 80000000
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D6C);                               // Load from: gViewAngle (80078294)
    v1 = lbu(a0 + 0x8);
    v0 >>= 22;
    v1 -= v0;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v1, at + 0x20C);                                 // Store to: 1F80020C
    v1 = lbu(a0 + 0x9);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0x7D34);                              // Load from: gPaletteClutId_CurMapSky (800782CC)
    t3 = -1;                                            // Result = FFFFFFFF
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20E);                                 // Store to: 1F80020E
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v1, at + 0x20D);                                 // Store to: 1F80020D
loc_8002C4A4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t5 + a0;
        if (bJump) goto loc_8002C50C;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t6 + a0;
        if (bJump) goto loc_8002C5D0;
    }
    v0 = lw(a3);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= t8;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8002C50C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t5 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8002C5C0;
    if (v1 == a0) goto loc_8002C4A4;
loc_8002C530:
    v0 = lw(gp + 0x5D8);                                // Load from: GPU_REG_GP1 (80077BB8)
    v0 = lw(v0);
    v0 &= t7;
    if (v0 == 0) goto loc_8002C4A4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t2;
    v0 |= t4;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t3) goto loc_8002C59C;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8002C580:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x5D4);                                // Load from: GPU_REG_GP0 (80077BB4)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8002C580;
loc_8002C59C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8002C4A4;
    goto loc_8002C530;
loc_8002C5C0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t6;
loc_8002C5D0:
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
    t0--;                                               // Result = 00000003
    v0 = -1;                                            // Result = FFFFFFFF
    a3 += 4;
    if (t0 == v0) goto loc_8002C630;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8002C618:
    v0 = lw(t1);
    t1 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_8002C618;
loc_8002C630:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_8002C6E4;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8002C65C:
    v0 = lw(gp + 0x5D8);                                // Load from: GPU_REG_GP1 (80077BB8)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8002C6E4;
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
    if (a1 == t0) goto loc_8002C6C8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8002C6AC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x5D4);                                // Load from: GPU_REG_GP0 (80077BB4)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8002C6AC;
loc_8002C6C8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8002C65C;
loc_8002C6E4:
    ra = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x28;
    return;
}
