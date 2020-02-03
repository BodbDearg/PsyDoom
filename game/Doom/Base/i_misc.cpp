#include "i_misc.h"

#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_main.h"
#include "i_main.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBC2.h"
#include "PsyQ/LIBGPU.h"

void I_DrawNumber() noexcept {
loc_8003A3C8:
    sp -= 0x58;
    v0 = 0xC3;                                          // Result = 000000C3
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20D);                                 // Store to: 1F80020D
    v0 = 0xB;                                           // Result = 0000000B
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    v0 = 0x10;                                          // Result = 00000010
    sw(s4, sp + 0x50);
    sw(s3, sp + 0x4C);
    sw(s2, sp + 0x48);
    sw(s1, sp + 0x44);
    sw(s0, sp + 0x40);
    at = 0x1F800000;                                    // Result = 1F800000
    sh(a1, at + 0x20A);                                 // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    t7 = a0;
    if (i32(a2) >= 0) goto loc_8003A42C;
    s3 = 1;                                             // Result = 00000001
    a2 = -a2;
    goto loc_8003A430;
loc_8003A420:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_8003A8A0;
loc_8003A42C:
    s3 = 0;                                             // Result = 00000000
loc_8003A430:
    t5 = sp;
    t4 = 0;                                             // Result = 00000000
    a0 = 0x66660000;                                    // Result = 66660000
    a0 |= 0x6667;                                       // Result = 66666667
    mult(a2, a0);
loc_8003A444:
    v1 = u32(i32(a2) >> 31);
    v0 = hi;
    v0 = u32(i32(v0) >> 2);
    v0 -= v1;
    v1 = v0;
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 1;
    v0 = a2 - v0;
    sw(v0, t5);
    a2 = v1;
    t5 += 4;
    if (a2 == 0) goto loc_8003A488;
    t4++;
    v0 = (i32(t4) < 0x10);
    mult(a2, a0);
    if (v0 != 0) goto loc_8003A444;
loc_8003A488:
    t5 = sp;
    if (i32(t4) < 0) goto loc_8003A71C;
    s4 = 0x1F800000;                                    // Result = 1F800000
    s4 += 0x200;                                        // Result = 1F800200
    t6 = 0xFF0000;                                      // Result = 00FF0000
    t6 |= 0xFFFF;                                       // Result = 00FFFFFF
    s2 = 0x80080000;                                    // Result = 80080000
    s2 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = s2 & t6;                                       // Result = 00086550
    s0 = 0x4000000;                                     // Result = 04000000
    t9 = 0x80000000;                                    // Result = 80000000
    t8 = -1;                                            // Result = FFFFFFFF
loc_8003A4B8:
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t7, at + 0x208);                                 // Store to: 1F800208
    v0 = lw(t5);
    t5 += 4;
    t3 = s4 + 4;                                        // Result = 1F800204
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 <<= 2;
    t1 = t0 << 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F84;                                       // Result = BigFontTexcoords_0[0] (80073F84)
    at += v0;
    v0 = lbu(at);
    t2 = t1 + 4;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
loc_8003A500:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8003A568;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_8003A62C;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(s2, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003A568:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003A61C;
    if (v1 == a0) goto loc_8003A500;
loc_8003A58C:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= s0;
    if (v0 == 0) goto loc_8003A500;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t6;
    v0 |= t9;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t8) goto loc_8003A5F8;
    a3 = -1;                                            // Result = FFFFFFFF
loc_8003A5DC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_8003A5DC;
loc_8003A5F8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003A500;
    goto loc_8003A58C;
loc_8003A61C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_8003A62C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t6;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    a2 += 4;
    if (t0 == t8) goto loc_8003A6F4;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003A668:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_8003A668;
    goto loc_8003A6F4;
loc_8003A688:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= s0;
    if (v0 == 0) goto loc_8003A710;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t6;
    v0 |= t9;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t8) goto loc_8003A6F4;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003A6D8:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003A6D8;
loc_8003A6F4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003A688;
loc_8003A710:
    t4--;
    t7 -= 0xB;
    if (i32(t4) >= 0) goto loc_8003A4B8;
loc_8003A71C:
    t2 = 0xFF0000;                                      // Result = 00FF0000
    if (s3 == 0) goto loc_8003A9B4;
    t3 = 0x1F800000;                                    // Result = 1F800000
    t3 += 0x204;                                        // Result = 1F800204
    t2 |= 0xFFFF;                                       // Result = 00FFFFFF
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = s0 & t2;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x3FAC);                              // Load from: BigFontTexcoords_Minus[0] (80073FAC)
    t4 = -1;                                            // Result = FFFFFFFF
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t7, at + 0x208);                                 // Store to: 1F800208
    t1 = t0 << 2;
    t7 = t1 + 4;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
loc_8003A778:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8003A7DC;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_8003A420;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003A7DC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003A890;
    if (v1 == a0) goto loc_8003A778;
loc_8003A800:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_8003A778;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t2;
    v0 |= t5;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_8003A86C;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003A850:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003A850;
loc_8003A86C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003A778;
    goto loc_8003A800;
loc_8003A890:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t7;
loc_8003A8A0:
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
    t0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a3 += 4;
    if (t0 == v0) goto loc_8003A900;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003A8E8:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_8003A8E8;
loc_8003A900:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_8003A9B4;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003A92C:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8003A9B4;
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
    if (a1 == t0) goto loc_8003A998;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003A97C:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003A97C;
loc_8003A998:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003A92C;
loc_8003A9B4:
    s4 = lw(sp + 0x50);
    s3 = lw(sp + 0x4C);
    s2 = lw(sp + 0x48);
    s1 = lw(sp + 0x44);
    s0 = lw(sp + 0x40);
    sp += 0x58;
    return;
}

void I_DrawStringSmall() noexcept {
loc_8003A9D4:
    sp -= 0x10;
    t5 = a0;
    v0 = 8;                                             // Result = 00000008
    sw(s2, sp + 0x8);
    sw(s1, sp + 0x4);
    sw(s0, sp);
    at = 0x1F800000;                                    // Result = 1F800000
    sh(a1, at + 0x20A);                                 // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    a0 = lbu(a2);
    a2++;
    if (a0 == 0) goto loc_8003ACEC;
    s2 = 0x1F800000;                                    // Result = 1F800000
    s2 += 0x200;                                        // Result = 1F800200
    t4 = 0xFF0000;                                      // Result = 00FF0000
    t4 |= 0xFFFF;                                       // Result = 00FFFFFF
    s1 = 0x80080000;                                    // Result = 80080000
    s1 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s0 = s1 & t4;                                       // Result = 00086550
    t9 = 0x4000000;                                     // Result = 04000000
    t8 = 0x80000000;                                    // Result = 80000000
    t7 = -1;                                            // Result = FFFFFFFF
loc_8003AA3C:
    v0 = a0 - 0x61;
    v0 = (v0 < 0x1A);
    if (v0 == 0) goto loc_8003AA50;
    a0 -= 0x20;
loc_8003AA50:
    a0 -= 0x21;
    v0 = (a0 < 0x40);
    if (v0 != 0) goto loc_8003AA74;
    t5 += 8;
    goto loc_8003ACDC;
loc_8003AA68:
    v0 = t2 + 4;
    v0 += a0;
    goto loc_8003ABF4;
loc_8003AA74:
    at = 0x1F800000;                                    // Result = 1F800000
    sh(t5, at + 0x208);                                 // Store to: 1F800208
    v1 = a0;
    if (i32(a0) >= 0) goto loc_8003AA88;
    v1 = a0 + 0x1F;
loc_8003AA88:
    t3 = s2 + 4;                                        // Result = 1F800204
    v1 = u32(i32(v1) >> 5);
    v0 = v1 << 5;
    v0 = a0 - v0;
    v0 <<= 3;
    v1 <<= 3;
    t1 = 0x1F800000;                                    // Result = 1F800000
    t1 = lbu(t1 + 0x203);                               // Load from: 1F800203
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 -= 0x58;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v1, at + 0x20D);                                 // Store to: 1F80020D
    t2 = t1 << 2;
    t6 = t2 + 4;
loc_8003AACC:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_8003AB30;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_8003AA68;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(s1, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s0;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003AB30:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t2 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003ABE4;
    if (v1 == a0) goto loc_8003AACC;
loc_8003AB54:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_8003AACC;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t7) goto loc_8003ABC0;
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003ABA4:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != t0) goto loc_8003ABA4;
loc_8003ABC0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003AACC;
    goto loc_8003AB54;
loc_8003ABE4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t6;
loc_8003ABF4:
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
    sb(t1, a3 + 0x3);
    t1--;
    a3 += 4;
    if (t1 == t7) goto loc_8003ACBC;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003AC30:
    v0 = lw(t3);
    t3 += 4;
    t1--;
    sw(v0, a3);
    a3 += 4;
    if (t1 != v1) goto loc_8003AC30;
    goto loc_8003ACBC;
loc_8003AC50:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_8003ACD8;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t8;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t7) goto loc_8003ACBC;
    a3 = -1;                                            // Result = FFFFFFFF
loc_8003ACA0:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_8003ACA0;
loc_8003ACBC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003AC50;
loc_8003ACD8:
    t5 += 8;
loc_8003ACDC:
    a0 = lbu(a2);
    a2++;
    if (a0 != 0) goto loc_8003AA3C;
loc_8003ACEC:
    s2 = lw(sp + 0x8);
    s1 = lw(sp + 0x4);
    s0 = lw(sp);
    sp += 0x10;
    return;
}

void I_DrawPausedOverlay() noexcept {
loc_8003AD04:
    v1 = *gCurPlayerIndex;
    sp -= 0x58;
    sw(ra, sp + 0x54);
    sw(s0, sp + 0x50);
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    s0 = v1 + v0;
    v0 = lw(s0 + 0xC0);
    v0 &= 0x100;
    a1 = 0x6B;                                          // Result = 0000006B
    if (v0 != 0) goto loc_8003AD64;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7A70;                                       // Result = gTexInfo_PAUSE[0] (80097A70)
    a3 = *gPaletteClutId_Main;
    a2 = 0x6C;                                          // Result = 0000006C
    _thunk_I_CacheAndDrawSprite();
loc_8003AD64:
    v1 = lw(s0 + 0xC0);
    v0 = v1 & 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 0x10;
        if (bJump) goto loc_8003ADD8;
    }
    a2 = *gMapNumToCheatWarpTo;
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1634;                                       // Result = STR_WarpToLevel[0] (80011634)
    a0 = sp + 0x10;
    LIBC2_sprintf();
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = 0x28;                                          // Result = 00000028
    a2 = sp + 0x10;
    I_DrawString();
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = 0x3C;                                          // Result = 0000003C
    a2 = *gMapNumToCheatWarpTo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x40BC;                                       // Result = StatusBarWeaponBoxesXPos[6] (800740BC)
    a2 <<= 5;
    a2 += v0;
    I_DrawString();
    goto loc_8003B0DC;
loc_8003ADCC:
    v0 = t3 + 4;                                        // Result = 00000018
    v0 += a0;
    goto loc_8003AFB8;
loc_8003ADD8:
    t0 = 5;                                             // Result = 00000005
    if (v0 == 0) goto loc_8003B0DC;
    t2 = 0x1F800000;                                    // Result = 1F800000
    t2 += 0x204;                                        // Result = 1F800204
    t3 = 0x14;                                          // Result = 00000014
    t1 = 0xFF0000;                                      // Result = 00FF0000
    t1 |= 0xFFFF;                                       // Result = 00FFFFFF
    t7 = 0x80080000;                                    // Result = 80080000
    t7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s0 = t7 & t1;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t4 = -1;                                            // Result = FFFFFFFF
    t8 = 0x18;                                          // Result = 00000018
    v0 = 5;                                             // Result = 00000005
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v0 = 0x28;                                          // Result = 00000028
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 = 0x100;                                         // Result = 00000100
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    v0 = 0xF0;                                          // Result = 000000F0
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x204);                                  // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x205);                                  // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x206);                                  // Store to: 1F800206
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x208);                                  // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x20A);                                  // Store to: 1F80020A
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x20E);                                  // Store to: 1F80020E
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x210);                                  // Store to: 1F800210
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x214);                                 // Store to: 1F800214
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x216);                                 // Store to: 1F800216
loc_8003AE90:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t3 + a0;
        if (bJump) goto loc_8003AEF4;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_8003ADCC;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(t7, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s0;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003AEF4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t3 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003AFA8;
    if (v1 == a0) goto loc_8003AE90;
loc_8003AF18:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_8003AE90;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t1;
    v0 |= t5;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_8003AF84;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003AF68:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003AF68;
loc_8003AF84:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003AE90;
    goto loc_8003AF18;
loc_8003AFA8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t8;
loc_8003AFB8:
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
    t0--;                                               // Result = 00000004
    v0 = -1;                                            // Result = FFFFFFFF
    a3 += 4;
    if (t0 == v0) goto loc_8003B018;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003B000:
    v0 = lw(t2);
    t2 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_8003B000;
loc_8003B018:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_8003B0CC;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003B044:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8003B0CC;
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
    if (a1 == t0) goto loc_8003B0B0;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003B094:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003B094;
loc_8003B0B0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003B044;
loc_8003B0CC:
    a0 = *gVramViewerTexPage;
    I_VramViewerDraw();
loc_8003B0DC:
    ra = lw(sp + 0x54);
    s0 = lw(sp + 0x50);
    sp += 0x58;
    return;
}

void I_UpdatePalette() noexcept {
loc_8003B0F0:
    v1 = *gCurPlayerIndex;
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    a1 = v1 + v0;
    v0 = lw(a1 + 0x34);
    a2 = lw(a1 + 0xD8);
    {
        const bool bJump = (v0 == 0);
        v0 = u32(i32(v0) >> 6);
        if (bJump) goto loc_8003B144;
    }
    v1 = 0xC;                                           // Result = 0000000C
    v1 -= v0;
    v0 = (i32(a2) < i32(v1));
    if (v0 == 0) goto loc_8003B144;
    a2 = v1;
loc_8003B144:
    a0 = lw(a1 + 0x44);
    *gbDoViewLighting = true;
    v0 = (i32(a0) < 0x3D);
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8003B16C;
    v0 = a0 & 8;
    if (v0 == 0) goto loc_8003B174;
loc_8003B16C:
    *gbDoViewLighting = false;
loc_8003B174:
    a0 = lw(a1 + 0x30);
    v0 = (i32(a0) < 0x3D);
    {
        const bool bJump = (v0 == 0);
        v0 = a0 & 8;
        if (bJump) goto loc_8003B190;
    }
    if (v0 == 0) goto loc_8003B1A0;
loc_8003B190:
    *gbDoViewLighting = false;
    v1 = 0xE;                                           // Result = 0000000E
    goto loc_8003B210;
loc_8003B1A0:
    v0 = a2 + 7;
    if (a2 == 0) goto loc_8003B1C4;
    v1 = u32(i32(v0) >> 3);
    v0 = (i32(v1) < 8);
    if (v0 != 0) goto loc_8003B1BC;
    v1 = 7;                                             // Result = 00000007
loc_8003B1BC:
    v1++;
    goto loc_8003B210;
loc_8003B1C4:
    a0 = lw(a1 + 0x3C);
    v0 = (i32(a0) < 0x3D);
    {
        const bool bJump = (v0 == 0);
        v0 = a0 & 8;
        if (bJump) goto loc_8003B1E0;
    }
    if (v0 == 0) goto loc_8003B1E8;
loc_8003B1E0:
    v1 = 0xD;                                           // Result = 0000000D
    goto loc_8003B210;
loc_8003B1E8:
    v0 = lw(a1 + 0xDC);
    {
        const bool bJump = (v0 == 0);
        v0 += 7;
        if (bJump) goto loc_8003B210;
    }
    v1 = u32(i32(v0) >> 3);
    v0 = (i32(v1) < 4);
    v1 += 9;
    if (v0 != 0) goto loc_8003B210;
    v1 = 3;                                             // Result = 00000003
    v1 += 9;                                            // Result = 0000000C
loc_8003B210:
    v0 = v1 << 1;
    at = gPaletteClutId_Main;
    at += v0;
    v0 = lhu(at);
    at = 0x80070000;                                    // Result = 80070000
    *g3dViewPaletteClutId = (uint16_t) v0;
    return;
}

void I_GetStringXPosToCenter() noexcept {
    a2 = 0;                                             // Result = 00000000
    v1 = lbu(a0);
    a0++;
    if (v1 == 0) goto loc_8003B30C;
    t2 = 0x25;                                          // Result = 00000025
    t1 = 0x21;                                          // Result = 00000021
    t0 = 0x2E;                                          // Result = 0000002E
    a3 = 0x2D;                                          // Result = 0000002D
loc_8003B25C:
    v0 = v1 - 0x41;
    v0 = (v0 < 0x1A);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 - 0x33;
        if (bJump) goto loc_8003B274;
    }
    v1 = v0 << 2;
    goto loc_8003B2E4;
loc_8003B274:
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 - 0x39;
        if (bJump) goto loc_8003B28C;
    }
    v1 = v0 << 2;
    goto loc_8003B2E4;
loc_8003B28C:
    a1 = v1 - 0x30;
    v0 = (a1 < 0xA);
    if (v0 == 0) goto loc_8003B2A4;
    v1 = a1 << 2;
    goto loc_8003B2E4;
loc_8003B2A4:
    if (v1 != t2) goto loc_8003B2B4;
    v1 = 0x2C;                                          // Result = 0000002C
    goto loc_8003B2E4;
loc_8003B2B4:
    if (v1 != t1) goto loc_8003B2C4;
    v1 = 0x30;                                          // Result = 00000030
    goto loc_8003B2E4;
loc_8003B2C4:
    if (v1 != t0) goto loc_8003B2D4;
    v1 = 0x34;                                          // Result = 00000034
    goto loc_8003B2E4;
loc_8003B2D4:
    {
        const bool bJump = (v1 == a3);
        v1 = 0x28;                                      // Result = 00000028
        if (bJump) goto loc_8003B2E4;
    }
    a2 += 6;
    goto loc_8003B2FC;
loc_8003B2E4:
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F86;                                       // Result = BigFontTexcoords_0[2] (80073F86)
    at += v1;
    v0 = lbu(at);
    a2 += v0;
loc_8003B2FC:
    v1 = lbu(a0);
    a0++;
    if (v1 != 0) goto loc_8003B25C;
loc_8003B30C:
    v0 = 0x100;                                         // Result = 00000100
    v0 -= a2;
    v1 = v0 >> 31;
    v0 += v1;
    v0 = u32(i32(v0) >> 1);
    return;
}

void I_DrawString() noexcept {
loc_8003B324:
    sp -= 0x30;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(s4, sp + 0x28);
    s4 = a1;
    sw(s2, sp + 0x20);
    s2 = a2;
    sw(s0, sp + 0x18);
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    a0 = s0;                                            // Result = 1F800200
    a1 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lhu(a3 - 0x6B0E);                              // Load from: gTexInfo_STATUS[2] (800A94F2)
    a2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x2C);
    sw(s3, sp + 0x24);
    sw(0, sp + 0x10);
    _thunk_LIBGPU_SetDrawMode();
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
loc_8003B3AC:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8003B414;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_8003B4D8;
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
loc_8003B414:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003B4C8;
    if (v1 == a0) goto loc_8003B3AC;
loc_8003B438:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_8003B3AC;
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
    if (a1 == t4) goto loc_8003B4A4;
    a3 = -1;                                            // Result = FFFFFFFF
loc_8003B488:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_8003B488;
loc_8003B4A4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003B3AC;
    goto loc_8003B438;
loc_8003B4C8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_8003B4D8:
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
    if (t0 == v0) goto loc_8003B538;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003B520:
    v0 = lw(s0);
    s0 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_8003B520;
loc_8003B538:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003B5F0;
    t2 = 0x4000000;                                     // Result = 04000000
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003B568:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8003B5F0;
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
    if (a1 == t0) goto loc_8003B5D4;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003B5B8:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003B5B8;
loc_8003B5D4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003B568;
loc_8003B5F0:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lhu(v1 - 0x6F5C);                              // Load from: gPaletteClutId_UI (800A90A4)
    v0 = 4;                                             // Result = 00000004
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v0 = 0x65;                                          // Result = 00000065
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x20E);                                 // Store to: 1F80020E
    a1 = 0;                                             // Result = 00000000
    if (s1 != v0) goto loc_8003B708;
    v1 = lbu(s2);
    a2 = s2 + 1;
    if (v1 == 0) goto loc_8003B6F4;
    t2 = 0x25;                                          // Result = 00000025
    t1 = 0x21;                                          // Result = 00000021
    t0 = 0x2E;                                          // Result = 0000002E
    a3 = 0x2D;                                          // Result = 0000002D
loc_8003B644:
    v0 = v1 - 0x41;
    v0 = (v0 < 0x1A);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 - 0x33;
        if (bJump) goto loc_8003B65C;
    }
    v1 = v0 << 2;
    goto loc_8003B6CC;
loc_8003B65C:
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 - 0x39;
        if (bJump) goto loc_8003B674;
    }
    v1 = v0 << 2;
    goto loc_8003B6CC;
loc_8003B674:
    a0 = v1 - 0x30;
    v0 = (a0 < 0xA);
    if (v0 == 0) goto loc_8003B68C;
    v1 = a0 << 2;
    goto loc_8003B6CC;
loc_8003B68C:
    if (v1 != t2) goto loc_8003B69C;
    v1 = 0x2C;                                          // Result = 0000002C
    goto loc_8003B6CC;
loc_8003B69C:
    if (v1 != t1) goto loc_8003B6AC;
    v1 = 0x30;                                          // Result = 00000030
    goto loc_8003B6CC;
loc_8003B6AC:
    if (v1 != t0) goto loc_8003B6BC;
    v1 = 0x34;                                          // Result = 00000034
    goto loc_8003B6CC;
loc_8003B6BC:
    {
        const bool bJump = (v1 == a3);
        v1 = 0x28;                                      // Result = 00000028
        if (bJump) goto loc_8003B6CC;
    }
    a1 += 6;
    goto loc_8003B6E4;
loc_8003B6CC:
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F86;                                       // Result = BigFontTexcoords_0[2] (80073F86)
    at += v1;
    v0 = lbu(at);
    a1 += v0;
loc_8003B6E4:
    v1 = lbu(a2);
    a2++;
    if (v1 != 0) goto loc_8003B644;
loc_8003B6F4:
    v0 = 0x100;                                         // Result = 00000100
    v0 -= a1;
    v1 = v0 >> 31;
    v0 += v1;
    s1 = u32(i32(v0) >> 1);
loc_8003B708:
    t0 = lbu(s2);
    s2++;
    if (t0 == 0) goto loc_8003BA9C;
    s3 = 0x1F800000;                                    // Result = 1F800000
    s3 += 0x200;                                        // Result = 1F800200
    t4 = 0xFF0000;                                      // Result = 00FF0000
    t4 |= 0xFFFF;                                       // Result = 00FFFFFF
    t9 = 0x80080000;                                    // Result = 80080000
    t9 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    t8 = t9 & t4;                                       // Result = 00086550
    s0 = 0x4000000;                                     // Result = 04000000
    t7 = 0x80000000;                                    // Result = 80000000
    t6 = -1;                                            // Result = FFFFFFFF
loc_8003B740:
    v0 = t0 - 0x41;
    v0 = (v0 < 0x1A);
    a0 = s4;
    if (v0 == 0) goto loc_8003B75C;
    v0 = t0 - 0x33;
    t0 = v0 << 2;
    goto loc_8003B7DC;
loc_8003B75C:
    v0 = t0 - 0x61;
    v0 = (v0 < 0x1A);
    {
        const bool bJump = (v0 == 0);
        v0 = t0 - 0x39;
        if (bJump) goto loc_8003B778;
    }
    t0 = v0 << 2;
    a0 += 3;
    goto loc_8003B7DC;
loc_8003B778:
    v1 = t0 - 0x30;
    v0 = (v1 < 0xA);
    {
        const bool bJump = (v0 == 0);
        v0 = 0x25;                                      // Result = 00000025
        if (bJump) goto loc_8003B790;
    }
    t0 = v1 << 2;
    goto loc_8003B7DC;
loc_8003B790:
    {
        const bool bJump = (t0 != v0);
        v0 = 0x21;                                      // Result = 00000021
        if (bJump) goto loc_8003B7A0;
    }
    t0 = 0x2C;                                          // Result = 0000002C
    goto loc_8003B7DC;
loc_8003B7A0:
    {
        const bool bJump = (t0 != v0);
        v0 = 0x2E;                                      // Result = 0000002E
        if (bJump) goto loc_8003B7B0;
    }
    t0 = 0x30;                                          // Result = 00000030
    goto loc_8003B7DC;
loc_8003B7B0:
    {
        const bool bJump = (t0 != v0);
        v0 = 0x2D;                                      // Result = 0000002D
        if (bJump) goto loc_8003B7C0;
    }
    t0 = 0x34;                                          // Result = 00000034
    goto loc_8003B7DC;
loc_8003B7C0:
    {
        const bool bJump = (t0 == v0);
        t0 = 0x28;                                      // Result = 00000028
        if (bJump) goto loc_8003B7DC;
    }
    s1 += 6;
    goto loc_8003BA8C;
loc_8003B7D0:
    v0 = t2 + 4;
    v0 += a0;
    goto loc_8003B990;
loc_8003B7DC:
    at = 0x1F800000;                                    // Result = 1F800000
    sh(s1, at + 0x208);                                 // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(a0, at + 0x20A);                                 // Store to: 1F80020A
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F84;                                       // Result = BigFontTexcoords_0[0] (80073F84)
    at += t0;
    v0 = lbu(at);
    t1 = 0x1F800000;                                    // Result = 1F800000
    t1 = lbu(t1 + 0x203);                               // Load from: 1F800203
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F85;                                       // Result = BigFontTexcoords_0[1] (80073F85)
    at += t0;
    v0 = lbu(at);
    t3 = s3 + 4;                                        // Result = 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20D);                                 // Store to: 1F80020D
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F86;                                       // Result = BigFontTexcoords_0[2] (80073F86)
    at += t0;
    v0 = lbu(at);
    t2 = t1 << 2;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F87;                                       // Result = BigFontTexcoords_0[3] (80073F87)
    at += t0;
    v0 = lbu(at);
    t5 = t2 + 4;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
loc_8003B868:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_8003B8CC;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_8003B7D0;
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
loc_8003B8CC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t2 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003B980;
    if (v1 == a0) goto loc_8003B868;
loc_8003B8F0:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= s0;
    if (v0 == 0) goto loc_8003B868;
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
    if (a1 == t6) goto loc_8003B95C;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003B940:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003B940;
loc_8003B95C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003B868;
    goto loc_8003B8F0;
loc_8003B980:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t5;
loc_8003B990:
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
    sb(t1, a3 + 0x3);
    t1--;
    a3 += 4;
    if (t1 == t6) goto loc_8003BA58;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003B9CC:
    v0 = lw(t3);
    t3 += 4;
    t1--;
    sw(v0, a3);
    a3 += 4;
    if (t1 != v1) goto loc_8003B9CC;
    goto loc_8003BA58;
loc_8003B9EC:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= s0;
    if (v0 == 0) goto loc_8003BA74;
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
    if (a1 == t6) goto loc_8003BA58;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003BA3C:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003BA3C;
loc_8003BA58:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003B9EC;
loc_8003BA74:
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3F86;                                       // Result = BigFontTexcoords_0[2] (80073F86)
    at += t0;                                           // Result = BigFontTexcoords_Minus[2] (80073FAE)
    v0 = lbu(at);                                       // Load from: BigFontTexcoords_Minus[2] (80073FAE)
    s1 += v0;
loc_8003BA8C:
    t0 = lbu(s2);
    s2++;
    if (t0 != 0) goto loc_8003B740;
loc_8003BA9C:
    ra = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}
