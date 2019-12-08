#include "r_segs.h"

#include "PsxVm/PsxVm.h"

void R_DrawSubsectorSeg() noexcept {
loc_8002D3AC:
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0x7FF4);                               // Load from: gpCurSector (8007800C)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7EEC);                               // Load from: gViewZ (80077EEC)
    sp -= 0x58;
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
    sw(a0, sp + 0x28);
    s0 = lw(a0 + 0x4);
    v0 = lw(a2 + 0x4);
    a0 = lw(s0 + 0x14);
    v0 = a1 - v0;
    s4 = u32(i32(v0) >> 16);
    s5 = s4;
    s3 = lw(a0 + 0x10);
    v0 = lw(a2);
    v1 = s3 | 0x100;
    v0 = a1 - v0;
    s7 = u32(i32(v0) >> 16);
    sw(v1, a0 + 0x10);
    s1 = lw(s0 + 0x1C);
    fp = s7;
    if (s1 == 0) goto loc_8002D5B8;
    v1 = lw(s1 + 0x4);
    v0 = lw(s1);
    a0 = a1 - v1;
    s6 = u32(i32(a0) >> 16);
    v0 = a1 - v0;
    a0 = lw(a2 + 0x4);
    v1 = (i32(v1) < i32(a0));
    s2 = u32(i32(v0) >> 16);
    if (v1 == 0) goto loc_8002D4F4;
    v1 = lw(s1 + 0xC);
    v0 = -1;                                            // Result = FFFFFFFF
    a0 = s6 - s5;
    if (v1 == v0) goto loc_8002D4F4;
    v0 = (i32(a0) < 0x100);
    {
        const bool bJump = (v0 != 0);
        v0 = s3 & 8;
        if (bJump) goto loc_8002D474;
    }
    a0 = 0xFF;                                          // Result = 000000FF
loc_8002D474:
    if (v0 == 0) goto loc_8002D490;
    v0 = lw(s0 + 0x10);
    t0 = lh(v0 + 0x6);
    t1 = t0 + a0;
    goto loc_8002D4A8;
loc_8002D490:
    v0 = lw(s0 + 0x10);
    v0 = lh(v0 + 0x6);
    t1 = v0 + 0xFF;
    t0 = t1 - a0;
loc_8002D4A8:
    a2 = s4;
    a3 = s6;
    v0 = lw(s0 + 0x10);
    a0 = lw(sp + 0x28);
    v0 = lw(v0 + 0x8);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F6C);                               // Load from: gpTextureTranslation (80077F6C)
    v0 <<= 2;
    v0 += v1;
    a1 = lw(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7ED8);                               // Load from: gpTextures (80078128)
    s5 = a3;
    sw(t0, sp + 0x10);
    sw(t1, sp + 0x14);
    sw(0, sp + 0x18);
    a1 <<= 5;
    a1 += v0;
    R_DrawWallColumns();
loc_8002D4F4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FF4);                               // Load from: gpCurSector (8007800C)
    v1 = lw(s1);
    v0 = lw(v0);
    v0 = (i32(v0) < i32(v1));
    a0 = s7 - s2;
    if (v0 == 0) goto loc_8002D5AC;
    v0 = (i32(a0) < 0x100);
    {
        const bool bJump = (v0 != 0);
        v0 = s3 & 0x10;
        if (bJump) goto loc_8002D524;
    }
    a0 = 0xFF;                                          // Result = 000000FF
loc_8002D524:
    if (v0 == 0) goto loc_8002D54C;
    v0 = lw(s0 + 0x10);
    v1 = lh(v0 + 0x6);
    v0 = s2 - s4;
    v0 += v1;
    v1 = -0x81;                                         // Result = FFFFFF7F
    t0 = v0 & v1;
    goto loc_8002D558;
loc_8002D54C:
    v0 = lw(s0 + 0x10);
    t0 = lh(v0 + 0x6);
loc_8002D558:
    t1 = t0 + a0;
    a2 = s2;
    a3 = s7;
    v0 = lw(s0 + 0x10);
    a0 = lw(sp + 0x28);
    v0 = lw(v0 + 0xC);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F6C);                               // Load from: gpTextureTranslation (80077F6C)
    v0 <<= 2;
    v0 += v1;
    a1 = lw(v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7ED8);                               // Load from: gpTextures (80078128)
    fp = a2;
    sw(t0, sp + 0x10);
    sw(t1, sp + 0x14);
    sw(0, sp + 0x18);
    a1 <<= 5;
    a1 += v0;
    R_DrawWallColumns();
loc_8002D5AC:
    v0 = s3 & 0x600;
    if (v0 == 0) goto loc_8002D650;
loc_8002D5B8:
    a0 = fp - s5;
    v0 = (i32(a0) < 0x100);
    {
        const bool bJump = (v0 != 0);
        v0 = s3 & 0x10;
        if (bJump) goto loc_8002D5CC;
    }
    a0 = 0xFF;                                          // Result = 000000FF
loc_8002D5CC:
    if (v0 == 0) goto loc_8002D5F0;
    v0 = lw(s0 + 0x10);
    v0 = lh(v0 + 0x6);
    t1 = v0 + 0xFF;
    t0 = t1 - a0;
    goto loc_8002D604;
loc_8002D5F0:
    v0 = lw(s0 + 0x10);
    t0 = lh(v0 + 0x6);
    t1 = t0 + a0;
loc_8002D604:
    a2 = s5;
    v0 = lw(s0 + 0x10);
    a0 = lw(sp + 0x28);
    v0 = lw(v0 + 0x10);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F6C);                               // Load from: gpTextureTranslation (80077F6C)
    v0 <<= 2;
    v0 += v1;
    a1 = lw(v0);
    v0 = s3 & 0x400;
    sw(v0, sp + 0x18);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7ED8);                               // Load from: gpTextures (80078128)
    a3 = fp;
    sw(t0, sp + 0x10);
    sw(t1, sp + 0x14);
    a1 <<= 5;
    a1 += v0;
    R_DrawWallColumns();
loc_8002D650:
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

void R_DrawWallColumns() noexcept {
loc_8002D684:
    sp -= 0x88;
    sw(s4, sp + 0x70);
    s4 = a0;
    sw(s1, sp + 0x64);
    s1 = a1;
    sw(fp, sp + 0x80);
    fp = a3;
    sw(ra, sp + 0x84);
    sw(s7, sp + 0x7C);
    sw(s6, sp + 0x78);
    sw(s5, sp + 0x74);
    sw(s3, sp + 0x6C);
    sw(s2, sp + 0x68);
    sw(s0, sp + 0x60);
    sw(a2, sp + 0x58);
    v0 = lw(s4);
    v1 = lw(s4 + 0x8);
    s5 = lw(v0 + 0x14);
    v1 = lw(v1 + 0x14);
    s6 = lw(sp + 0xA0);
    s2 = v1 - s5;
    sw(v1, sp + 0x18);
    if (i32(s2) <= 0) goto loc_8002E274;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F34);                               // Load from: gpViewPlayer (80077F34)
    v0 = lw(v0 + 0xC0);
    v0 &= 0x80;
    if (v0 == 0) goto loc_8002D704;
    s6 = 1;                                             // Result = 00000001
loc_8002D704:
    v1 = lw(s1 + 0x1C);
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 != v0) goto loc_8002D79C;
    s0 = 0x800A0000;                                    // Result = 800A0000
    s0 -= 0x78B8;                                       // Result = gTmpWadLumpBuffer[0] (80098748)
    v0 = lh(s1 + 0x10);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7DC4);                               // Load from: gpLumpCache (8007823C)
    v0 <<= 2;
    v0 += v1;
    a0 = lw(v0);
    a1 = s0;                                            // Result = gTmpWadLumpBuffer[0] (80098748)
    decode();
    a0 = sp + 0x10;
    a1 = s0 + 8;                                        // Result = gTmpWadLumpBuffer[2] (80098750)
    v1 = lbu(s1 + 0x8);
    v0 = lhu(s1 + 0xA);
    v1 >>= 1;
    v0 &= 0xF;
    v0 <<= 6;
    v1 += v0;
    sh(v1, sp + 0x10);
    v1 = lhu(s1 + 0xA);
    a2 = lbu(s1 + 0x9);
    v0 = 0x20;                                          // Result = 00000020
    sh(v0, sp + 0x14);
    v0 = 0x80;                                          // Result = 00000080
    sh(v0, sp + 0x16);
    v1 &= 0x10;
    v1 <<= 4;
    a2 += v1;
    sh(a2, sp + 0x12);
    LIBGPU_LoadImage();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C10);                               // Load from: gNumFramesDrawn (80077C10)
    sw(v0, s1 + 0x1C);
loc_8002D79C:
    v0 = lbu(s1 + 0x8);
    sh(v0, sp + 0x10);
    v0 = lbu(s1 + 0x9);
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    sh(v0, sp + 0x12);
    v0 = lhu(s1 + 0x4);
    a0 = s0;                                            // Result = 1F800200
    sh(v0, sp + 0x14);
    v0 = lhu(s1 + 0x6);
    a1 = sp + 0x10;
    sh(v0, sp + 0x16);
    LIBGPU_SetTexWindow();
    s0 += 4;                                            // Result = 1F800204
    t2 = 0xFF0000;                                      // Result = 00FF0000
    t2 |= 0xFFFF;                                       // Result = 00FFFFFF
    t6 = 0x80080000;                                    // Result = 80080000
    t6 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s3 = t6 & t2;                                       // Result = 00086550
    t5 = 0x4000000;                                     // Result = 04000000
    t4 = 0x80000000;                                    // Result = 80000000
    t3 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t7 = t1 + 4;
loc_8002D810:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8002D874;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_8002DA74;
    v0 = lw(a2);
    at = 0x80070000;                                    // Result = 80070000
    sw(t6, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s3;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8002D874:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8002D928;
    if (v1 == a0) goto loc_8002D810;
loc_8002D898:
    v0 = lw(gp + 0x5D8);                                // Load from: GPU_REG_GP1 (80077BB8)
    v0 = lw(v0);
    v0 &= t5;
    if (v0 == 0) goto loc_8002D810;
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
    if (a1 == t3) goto loc_8002D904;
    a3 = -1;                                            // Result = FFFFFFFF
loc_8002D8E8:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x5D4);                                // Load from: GPU_REG_GP0 (80077BB4)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_8002D8E8;
loc_8002D904:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8002D810;
    goto loc_8002D898;
loc_8002D928:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t7;
loc_8002D938:
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
    if (t0 == v0) goto loc_8002D998;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8002D980:
    v0 = lw(s0);
    s0 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_8002D980;
loc_8002D998:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    {
        const bool bJump = (v1 == v0);
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_8002DA50;
    }
    t2 = 0x4000000;                                     // Result = 04000000
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8002D9C8:
    v0 = lw(gp + 0x5D8);                                // Load from: GPU_REG_GP1 (80077BB8)
    v0 = lw(v0);
    v0 &= t2;
    {
        const bool bJump = (v0 == 0);
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_8002DA50;
    }
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
    if (a1 == t0) goto loc_8002DA34;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8002DA18:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x5D4);                                // Load from: GPU_REG_GP0 (80077BB4)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8002DA18;
loc_8002DA34:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    {
        const bool bJump = (v1 != v0);
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_8002D9C8;
    }
loc_8002DA50:
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v0 = 0x24;                                          // Result = 00000024
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    v0 = 0x26;                                          // Result = 00000026
    if (s6 == 0) goto loc_8002DA88;
    goto loc_8002DA80;
loc_8002DA74:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_8002D938;
loc_8002DA80:
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
loc_8002DA88:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lhu(v0 + 0x7F7C);                              // Load from: g3dViewPaletteClutId (80077F7C)
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20E);                                 // Store to: 1F80020E
    v0 = lhu(s1 + 0xA);
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x216);                                 // Store to: 1F800216
    v0 = lw(s4);
    v1 = lw(s4 + 0x8);
    s3 = lw(v0 + 0x8);
    v1 = lw(v1 + 0x8);
    v0 = v1 - s3;
    div(v0, s2);
    if (s2 != 0) goto loc_8002DAD4;
    _break(0x1C00);
loc_8002DAD4:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s2 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002DAEC;
    }
    if (v0 != at) goto loc_8002DAEC;
    tge(zero, zero, 0x5D);
loc_8002DAEC:
    s7 = lo;
    sw(s7, sp + 0x40);
    s7 = lw(sp + 0x58);
    mult(s7, s3);
    t3 = lo;
    mult(s7, v1);
    v0 = lo;
    v0 -= t3;
    div(v0, s2);
    if (s2 != 0) goto loc_8002DB24;
    _break(0x1C00);
loc_8002DB24:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s2 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002DB3C;
    }
    if (v0 != at) goto loc_8002DB3C;
    tge(zero, zero, 0x5D);
loc_8002DB3C:
    s7 = lo;
    mult(fp, s3);
    t2 = lo;
    mult(fp, v1);
    v0 = lo;
    v0 -= t2;
    sw(s7, sp + 0x28);
    div(v0, s2);
    if (s2 != 0) goto loc_8002DB68;
    _break(0x1C00);
loc_8002DB68:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s2 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002DB80;
    }
    if (v0 != at) goto loc_8002DB80;
    tge(zero, zero, 0x5D);
loc_8002DB80:
    s7 = lo;
    sw(s7, sp + 0x38);
    t1 = lw(s4 + 0x4);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE0);                               // Load from: gViewX (80077EE0)
    a1 = lw(t1 + 0xC);
    t0 = lw(t1);
    v0 = a1 >> 19;
    v0 <<= 2;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v0;
    a0 = lw(at);
    a2 = lw(t0);
    t4 = u32(i32(a0) >> 8);
    a2 -= v1;
    a2 = u32(i32(a2) >> 8);
    mult(a2, t4);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v0 += a3;
    v0 = lw(v0);
    t5 = u32(i32(v0) >> 8);
    v0 = lw(t0 + 0x4);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE4);                               // Load from: gViewY (80077EE4)
    a0 = lo;
    v0 -= v1;
    v0 = u32(i32(v0) >> 8);
    mult(v0, t5);
    v1 = lo;
    mult(a2, t5);
    a2 = lo;
    mult(v0, t4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D6C);                               // Load from: gViewAngle (80078294)
    a0 -= v1;
    a1 -= v0;
    v0 = 0x40000000;                                    // Result = 40000000
    a1 += v0;
    a1 >>= 19;
    a1 <<= 2;
    a3 += a1;
    v0 = lw(a3);
    a3 = lo;
    a0 = u32(i32(a0) >> 8);
    t5 = u32(i32(v0) >> 8);
    mult(a0, t5);
    v0 = lo;
    v1 = s5 - 0x80;
    fp = u32(i32(v0) >> 4);
    mult(v1, fp);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += a1;
    v0 = lw(at);
    a1 = lo;
    a0 <<= 3;
    t4 = u32(i32(v0) >> 8);
    mult(a0, t4);
    a0 = lo;
    s6 = u32(i32(v0) >> 12);
    mult(v1, s6);
    a2 += a3;
    v0 = 0x640000;                                      // Result = 00640000
    t3 += v0;
    t2 += v0;
    t9 = a1 + a0;
    v1 = t5 << 3;
    a0 = 0x10000;                                       // Result = 00010000
    v0 = lo;
    s1 = v0 - v1;
    v1 = lw(t1 + 0x10);
    v0 = lw(t1 + 0x8);
    v1 = lw(v1);
    v0 += a0;
    v0 += v1;
    v0 -= a2;
    a2 = lh(t1 + 0x22);
    v0 = u32(i32(v0) >> 8);
    sw(v0, sp + 0x48);
    a3 = lh(t1 + 0x24);
    v0 = (i32(s5) < i32(a2));
    {
        const bool bJump = (v0 == 0);
        v0 = a2 - s5;
        if (bJump) goto loc_8002DD24;
    }
    s7 = lw(sp + 0x28);
    mult(v0, s7);
    a1 = lo;
    s7 = lw(sp + 0x38);
    mult(v0, s7);
    a0 = lo;
    mult(v0, fp);
    v1 = lo;
    mult(v0, s6);
    s5 = a2;
    t3 += a1;
    t2 += a0;
    t9 += v1;
    v0 = lo;
    s1 += v0;
loc_8002DD24:
    s7 = lw(sp + 0x18);
    v0 = (i32(a3) < i32(s7));
    s0 = s5;
    if (v0 == 0) goto loc_8002DD40;
    sw(a3, sp + 0x18);
    s7 = lw(sp + 0x18);
loc_8002DD40:
    v0 = (i32(s0) < i32(s7));
    t8 = 0xFF0000;                                      // Result = 00FF0000
    if (v0 == 0) goto loc_8002E274;
    t8 |= 0xFFFF;                                       // Result = 00FFFFFF
    s7 = 0x80080000;                                    // Result = 80080000
    s7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s7 &= t8;                                           // Result = 00086550
    sw(s7, sp + 0x58);
    s5 = 0x4000000;                                     // Result = 04000000
    s4 = 0x80000000;                                    // Result = 80000000
loc_8002DD6C:
    a2 = u32(i32(t3) >> 16);
    v0 = (i32(a2) < 0xC9);
    t1 = u32(i32(t2) >> 16);
    if (v0 == 0) goto loc_8002E23C;
    if (i32(t1) < 0) goto loc_8002E23C;
    t6 = 0;                                             // Result = 00000000
    if (s1 == 0) goto loc_8002DDC8;
    div(t9, s1);
    if (s1 != 0) goto loc_8002DD9C;
    _break(0x1C00);
loc_8002DD9C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002DDB4;
    }
    if (t9 != at) goto loc_8002DDB4;
    tge(zero, zero, 0x5D);
loc_8002DDB4:
    v0 = lo;
    s7 = lw(sp + 0x48);
    v0 += s7;
    t6 = u32(i32(v0) >> 8);
loc_8002DDC8:
    a0 = t1 - a2;
    v0 = (i32(a0) < 0x1FE);
    t4 = lw(sp + 0x98);
    t5 = lw(sp + 0x9C);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x64;                                      // Result = 00000064
        if (bJump) goto loc_8002DE88;
    }
    v0 -= a2;
    v0 <<= 16;
    div(v0, a0);
    if (a0 != 0) goto loc_8002DDF8;
    _break(0x1C00);
loc_8002DDF8:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002DE10;
    }
    if (v0 != at) goto loc_8002DE10;
    tge(zero, zero, 0x5D);
loc_8002DE10:
    v0 = lo;
    a1 = t5 - t4;
    mult(v0, a1);
    v1 = lo;
    v0 = 0x640000;                                      // Result = 00640000
    div(v0, a0);
    if (a0 != 0) goto loc_8002DE34;
    _break(0x1C00);
loc_8002DE34:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002DE4C;
    }
    if (v0 != at) goto loc_8002DE4C;
    tge(zero, zero, 0x5D);
loc_8002DE4C:
    v0 = lo;
    mult(v0, a1);
    v1 = u32(i32(v1) >> 16);
    a0 = t4 + v1;
    v0 = lo;
    v1 = u32(i32(v0) >> 16);
    if (i32(a2) >= 0) goto loc_8002DE74;
    a2 = 0;                                             // Result = 00000000
    t4 = a0 - v1;
loc_8002DE74:
    v0 = (i32(t1) < 0xC9);
    if (v0 != 0) goto loc_8002DE88;
    t5 = a0 + v1;
    t1 = 0xC8;                                          // Result = 000000C8
loc_8002DE88:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D9C);                               // Load from: gbRenderViewFullbright (80078264)
    v1 = u32(i32(s3) >> 8);
    if (v0 == 0) goto loc_8002DF44;
    v0 = (i32(v1) < 0x40);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 0xA1);
        if (bJump) goto loc_8002DEB0;
    }
    v1 = 0x40;                                          // Result = 00000040
    goto loc_8002DEBC;
loc_8002DEB0:
    if (v0 != 0) goto loc_8002DEBC;
    v1 = 0xA0;                                          // Result = 000000A0
loc_8002DEBC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7E8C);                               // Load from: gCurLightValR (80077E8C)
    mult(v1, v0);
    v0 = lo;
    a1 = u32(i32(v0) >> 7);
    v0 = (i32(a1) < 0x100);
    if (v0 != 0) goto loc_8002DEE4;
    a1 = 0xFF;                                          // Result = 000000FF
loc_8002DEE4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FCC);                               // Load from: gCurLightValG (80078034)
    mult(v1, v0);
    v0 = lo;
    a0 = u32(i32(v0) >> 7);
    v0 = (i32(a0) < 0x100);
    if (v0 != 0) goto loc_8002DF0C;
    a0 = 0xFF;                                          // Result = 000000FF
loc_8002DF0C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F70);                               // Load from: gCurLightValB (80077F70)
    mult(v1, v0);
    v0 = lo;
    v1 = u32(i32(v0) >> 7);
    v0 = (i32(v1) < 0x100);
    if (v0 != 0) goto loc_8002DF5C;
    v1 = 0xFF;                                          // Result = 000000FF
    goto loc_8002DF5C;
loc_8002DF38:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_8002E13C;
loc_8002DF44:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7E8C);                               // Load from: gCurLightValR (80077E8C)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FCC);                               // Load from: gCurLightValG (80078034)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F70);                               // Load from: gCurLightValB (80077F70)
loc_8002DF5C:
    s7 = 0x1F800000;                                    // Result = 1F800000
    s7 += 0x200;                                        // Result = 1F800200
    t7 = s7 + 4;                                        // Result = 1F800204
    s2 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = a2 - 1;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20A);                                 // Store to: 1F80020A
    v0 = s0 + 1;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    v0 = t1 + 1;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(a1, at + 0x204);                                 // Store to: 1F800204
    at = 0x1F800000;                                    // Result = 1F800000
    sb(a0, at + 0x205);                                 // Store to: 1F800205
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v1, at + 0x206);                                 // Store to: 1F800206
    at = 0x1F800000;                                    // Result = 1F800000
    sh(s0, at + 0x208);                                 // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    at = 0x1F800000;                                    // Result = 1F800000
    sh(s0, at + 0x218);                                 // Store to: 1F800218
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x21A);                                 // Store to: 1F80021A
    at = 0x1F800000;                                    // Result = 1F800000
    sb(t4, at + 0x20D);                                 // Store to: 1F80020D
    at = 0x1F800000;                                    // Result = 1F800000
    sb(t5, at + 0x215);                                 // Store to: 1F800215
    at = 0x1F800000;                                    // Result = 1F800000
    sb(t5, at + 0x21D);                                 // Store to: 1F80021D
    at = 0x1F800000;                                    // Result = 1F800000
    sb(t6, at + 0x20C);                                 // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sb(t6, at + 0x214);                                 // Store to: 1F800214
    at = 0x1F800000;                                    // Result = 1F800000
    sb(t6, at + 0x21C);                                 // Store to: 1F80021C
    t1 = t0 << 2;
    t4 = t1 + 4;
loc_8002E008:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8002E078;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_8002DF38;
    s7 = 0x80080000;                                    // Result = 80080000
    s7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(s7, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    s7 = lw(sp + 0x58);
    v0 &= v1;
    v0 |= s7;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8002E078:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8002E12C;
    if (v1 == a0) goto loc_8002E008;
loc_8002E09C:
    v0 = lw(gp + 0x5D8);                                // Load from: GPU_REG_GP1 (80077BB8)
    v0 = lw(v0);
    v0 &= s5;
    if (v0 == 0) goto loc_8002E008;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t8;
    v0 |= s4;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == s2) goto loc_8002E108;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8002E0EC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x5D4);                                // Load from: GPU_REG_GP0 (80077BB4)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8002E0EC;
loc_8002E108:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8002E008;
    goto loc_8002E09C;
loc_8002E12C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t4;
loc_8002E13C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a3);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t8;
    v1 |= v0;
    sw(v1, a3);
    sb(t0, a3 + 0x3);
    t0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a3 += 4;
    if (t0 == v0) goto loc_8002E194;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8002E17C:
    v0 = lw(t7);
    t7 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_8002E17C;
loc_8002E194:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8002E23C;
    a3 = -1;                                            // Result = FFFFFFFF
loc_8002E1B4:
    v0 = lw(gp + 0x5D8);                                // Load from: GPU_REG_GP1 (80077BB8)
    v0 = lw(v0);
    v0 &= s5;
    if (v0 == 0) goto loc_8002E23C;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t8;
    v0 |= s4;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == a3) goto loc_8002E220;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8002E204:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x5D4);                                // Load from: GPU_REG_GP0 (80077BB4)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8002E204;
loc_8002E220:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8002E1B4;
loc_8002E23C:
    s7 = lw(sp + 0x40);
    s3 += s7;
    s7 = lw(sp + 0x28);
    t9 += fp;
    t3 += s7;
    s7 = lw(sp + 0x38);
    t2 += s7;
    s7 = lw(sp + 0x18);
    s0++;
    v0 = (i32(s0) < i32(s7));
    s1 += s6;
    if (v0 != 0) goto loc_8002DD6C;
loc_8002E274:
    ra = lw(sp + 0x84);
    fp = lw(sp + 0x80);
    s7 = lw(sp + 0x7C);
    s6 = lw(sp + 0x78);
    s5 = lw(sp + 0x74);
    s4 = lw(sp + 0x70);
    s3 = lw(sp + 0x6C);
    s2 = lw(sp + 0x68);
    s1 = lw(sp + 0x64);
    s0 = lw(sp + 0x60);
    sp += 0x88;
    return;
}
