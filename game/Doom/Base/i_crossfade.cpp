#include "i_crossfade.h"

#include "PsxVm/PsxVm.h"

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
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80036738;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
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
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80036980;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
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
