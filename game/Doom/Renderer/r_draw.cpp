#include "r_draw.h"

#include "Doom/Base/i_main.h"
#include "Doom/Game/p_setup.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGTE.h"
#include "r_main.h"
#include "r_plane.h"
#include "r_segs.h"
#include "r_things.h"

void R_DrawSubsector() noexcept {
loc_8002C6F8:
    sp -= 0x78;
    sw(s5, sp + 0x6C);
    s5 = a0;
    v0 = 0x1F800000;                                    // Result = 1F800000
    v0 += 0xA8;                                         // Result = 1F8000A8
    sw(s1, sp + 0x5C);
    s1 = v0 + 4;                                        // Result = 1F8000AC
    sw(v0, sp + 0x28);
    v0 += 0xAC;                                         // Result = 1F800154
    sw(s2, sp + 0x60);
    s2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x70);
    sw(s4, sp + 0x68);
    sw(s3, sp + 0x64);
    sw(s0, sp + 0x58);
    sw(v0, sp + 0x2C);
    v0 = lh(s5 + 0xA);
    v1 = *gpLeafEdges;
    s4 = lh(s5 + 0x8);
    v0 <<= 3;
    s3 = v0 + v1;
    if (i32(s4) <= 0) goto loc_8002C850;
loc_8002C754:
    v0 = lw(s3 + 0x4);
    s0 = lw(s3);
    sw(v0, s1 + 0x4);
    sw(s0, s1);
    v1 = lw(s0 + 0x18);
    v0 = *gNumFramesDrawn;
    a0 = sp + 0x10;
    if (v1 == v0) goto loc_8002C83C;
    v0 = lw(s0);
    v1 = *gViewX;
    a1 = sp + 0x18;
    sh(0, sp + 0x12);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    sh(v0, sp + 0x10);
    v0 = lw(s0 + 0x4);
    v1 = *gViewY;
    a2 = sp + 0x38;
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    sh(v0, sp + 0x14);
    LIBGTE_RotTrans();
    v0 = lw(sp + 0x18);
    sw(v0, s0 + 0xC);
    v1 = lw(sp + 0x20);
    v0 = (i32(v1) < 4);
    sw(v1, s0 + 0x10);
    if (v0 != 0) goto loc_8002C82C;
    v0 = 0x800000;                                      // Result = 00800000
    div(v0, v1);
    if (v1 != 0) goto loc_8002C7F0;
    _break(0x1C00);
loc_8002C7F0:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002C808;
    }
    if (v0 != at) goto loc_8002C808;
    tge(zero, zero, 0x5D);
loc_8002C808:
    v0 = lo;
    v1 = lw(s0 + 0xC);
    mult(v0, v1);
    sw(v0, s0 + 0x8);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
loc_8002C82C:
    v0 = *gNumFramesDrawn;
    sw(v0, s0 + 0x18);
loc_8002C83C:
    s2++;
    s3 += 8;
    v0 = (i32(s2) < i32(s4));
    s1 += 8;
    if (v0 != 0) goto loc_8002C754;
loc_8002C850:
    v0 = lw(sp + 0x28);
    s2 = 0;                                             // Result = 00000000
    sw(s4, v0);
    v0 = lw(sp + 0x28);
    s3 = 0;                                             // Result = 00000000
    sw(0, gp + 0xC40);                                  // Store to: 80078220
    s0 = lw(v0);
    s1 = v0 + 4;
    if (i32(s0) <= 0) goto loc_8002C8D0;
    v1 = sp + 0x10;
loc_8002C87C:
    v0 = lw(s1);
    v0 = lw(v0 + 0x10);
    v0 = (i32(v0) < 4);
    a1 = sp + 0x28;
    if (v0 == 0) goto loc_8002C8C0;
    v0 = s3 << 2;                                       // Result = 00000000
    v0 += v1;
    if (s3 != 0) goto loc_8002C8A8;
    a1 = sp + 0x2C;
loc_8002C8A8:
    a0 = lw(v0 + 0x18);
    a1 = lw(a1);
    s3 ^= 1;                                            // Result = 00000001
    R_FrontZClip();
    v1 = s3 << 2;                                       // Result = 00000004
    goto loc_8002C8D4;
loc_8002C8C0:
    s2++;
    v0 = (i32(s2) < i32(s0));
    s1 += 8;
    if (v0 != 0) goto loc_8002C87C;
loc_8002C8D0:
    v1 = s3 << 2;                                       // Result = 00000000
loc_8002C8D4:
    v0 = sp + 0x10;
    s0 = v1 + v0;
    a1 = lw(s0 + 0x18);
    a0 = 0;                                             // Result = 00000000
    R_CheckEdgeVisible();
    if (i32(v0) < 0) goto loc_8002CA64;
    v1 = s3 << 2;
    if (i32(v0) <= 0) goto loc_8002C924;
    a1 = sp + 0x28;
    if (s3 != 0) goto loc_8002C904;
    a1 = sp + 0x2C;
loc_8002C904:
    a0 = lw(s0 + 0x18);
    a1 = lw(a1);
    R_LeftEdgeClip();
    v0 = (i32(v0) < 3);
    s3 ^= 1;                                            // Result = 00000001
    if (v0 != 0) goto loc_8002CA64;
    v1 = s3 << 2;                                       // Result = 00000004
loc_8002C924:
    v0 = sp + 0x10;
    s0 = v1 + v0;
    a1 = lw(s0 + 0x18);
    a0 = 1;                                             // Result = 00000001
    R_CheckEdgeVisible();
    if (i32(v0) < 0) goto loc_8002CA64;
    if (i32(v0) <= 0) goto loc_8002C970;
    a1 = sp + 0x28;
    if (s3 != 0) goto loc_8002C954;
    a1 = sp + 0x2C;
loc_8002C954:
    a0 = lw(s0 + 0x18);
    a1 = lw(a1);
    R_RightEdgeClip();
    v0 = (i32(v0) < 3);
    s3 ^= 1;
    if (v0 != 0) goto loc_8002CA64;
loc_8002C970:
    v0 = s3 << 2;
    v0 += sp;
    v1 = lw(v0 + 0x28);
    s0 = lw(v1);
    s1 = v1 + 4;
    v0 = s0 << 3;
    v0 += s1;
    a0 = lw(v1 + 0x4);
    a1 = lw(v1 + 0x8);
    sw(a0, v0);
    sw(a1, v0 + 0x4);
    s2 = 0;                                             // Result = 00000000
    if (i32(s0) <= 0) goto loc_8002C9E0;
loc_8002C9A8:
    v0 = lw(s1 + 0x4);
    s2++;
    if (v0 == 0) goto loc_8002C9D4;
    v0 = lhu(v0 + 0x20);
    v0 &= 1;
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s2) < i32(s0));
        if (bJump) goto loc_8002C9D8;
    }
    a0 = s1;
    R_DrawSubsectorSeg();
loc_8002C9D4:
    v0 = (i32(s2) < i32(s0));
loc_8002C9D8:
    s1 += 8;
    if (v0 != 0) goto loc_8002C9A8;
loc_8002C9E0:
    v0 = *gpCurDrawSector;
    v1 = *gViewZ;
    v0 = lw(v0);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = s3 << 2;                                   // Result = 00000000
        if (bJump) goto loc_8002CA14;
    }
    v0 += sp;
    a0 = lw(v0 + 0x28);
    a1 = 0;                                             // Result = 00000000
    R_DrawSubsectorFlat();
loc_8002CA14:
    a0 = *gpCurDrawSector;
    v1 = lw(a0 + 0xC);
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 == v0) goto loc_8002CA5C;
    v1 = lw(a0 + 0x4);
    v0 = *gViewZ;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = s3 << 2;                                   // Result = 00000000
        if (bJump) goto loc_8002CA5C;
    }
    v0 += sp;
    a0 = lw(v0 + 0x28);
    a1 = 1;                                             // Result = 00000001
    R_DrawSubsectorFlat();
loc_8002CA5C:
    a0 = s5;
    R_DrawSubsectorSprites();
loc_8002CA64:
    ra = lw(sp + 0x70);
    s5 = lw(sp + 0x6C);
    s4 = lw(sp + 0x68);
    s3 = lw(sp + 0x64);
    s2 = lw(sp + 0x60);
    s1 = lw(sp + 0x5C);
    s0 = lw(sp + 0x58);
    sp += 0x78;
    return;
}

void R_FrontZClip() noexcept {
loc_8002CA8C:
    sp -= 0x50;
    sw(s3, sp + 0x34);
    s3 = a0 + 4;
    sw(s1, sp + 0x2C);
    sw(a1, sp + 0x18);
    s1 = a1 + 4;
    sw(s7, sp + 0x44);
    s7 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x4C);
    sw(fp, sp + 0x48);
    sw(s6, sp + 0x40);
    sw(s5, sp + 0x3C);
    sw(s4, sp + 0x38);
    sw(s2, sp + 0x30);
    sw(s0, sp + 0x28);
    sw(a0, sp + 0x10);
    fp = lw(a0);
    s2 = 0;                                             // Result = 00000000
    if (i32(fp) <= 0) goto loc_8002CD28;
loc_8002CADC:
    v0 = fp - 1;
    s4 = s3 + 8;
    if (s7 != v0) goto loc_8002CAF4;
    a2 = lw(sp + 0x10);
    s4 = a2 + 4;
loc_8002CAF4:
    a2 = 4;                                             // Result = 00000004
    v0 = lw(s3);
    v1 = lw(s4);
    v0 = lw(v0 + 0x10);
    v1 = lw(v1 + 0x10);
    s5 = a2 - v0;
    s6 = a2 - v1;
    if (s5 != 0) goto loc_8002CB28;
    v0 = lw(s3);
    v1 = lw(s3 + 0x4);
    sw(v0, s1);
    sw(v1, s1 + 0x4);
    goto loc_8002CCF8;
loc_8002CB28:
    if (i32(s5) >= 0) goto loc_8002CB60;
    v0 = lw(s3);
    v1 = lw(s3 + 0x4);
    sw(v0, s1);
    sw(v1, s1 + 0x4);
    s2++;                                               // Result = 00000001
    v0 = (i32(s2) < 0x15);                              // Result = 00000001
    s1 += 8;
    if (v0 != 0) goto loc_8002CB60;
    I_Error("FrontZClip: Point Overflow");
loc_8002CB60:
    v0 = ~s6;
    if (s6 == 0) goto loc_8002CD18;
    v0 >>= 31;
    v1 = s5 >> 31;
    if (v0 != v1) goto loc_8002CD18;
    v1 = lw(gp + 0xC40);                                // Load from: 80078220
    a0 = v1 + 1;
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7CEC;                                       // Result = 80097CEC
    sw(a0, gp + 0xC40);                                 // Store to: 80078220
    a0 = (i32(a0) < 0x20);
    s0 = v0 + v1;
    if (a0 != 0) goto loc_8002CBB8;
    I_Error("FrontZClip: exceeded max new vertexes\n");
loc_8002CBB8:
    a0 = s5 << 16;
    v0 = s5 - s6;
    div(a0, v0);
    if (v0 != 0) goto loc_8002CBD0;
    _break(0x1C00);
loc_8002CBD0:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002CBE8;
    }
    if (a0 != at) goto loc_8002CBE8;
    tge(zero, zero, 0x5D);
loc_8002CBE8:
    a0 = lo;
    v0 = lw(s4);
    v1 = lw(s3);
    v0 = lw(v0 + 0xC);
    v1 = lw(v1 + 0xC);
    v0 -= v1;
    mult(a0, v0);
    a2 = 4;                                             // Result = 00000004
    sw(a2, s0 + 0x10);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += v1;
    sw(v0, s0 + 0xC);
    v0 = lw(s4);
    v1 = lw(s3);
    v0 = lw(v0);
    v1 = lw(v1);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(a0, v0);
    v0 = lo;
    v0 += v1;
    sw(v0, s0);
    v0 = lw(s4);
    v1 = lw(s3);
    v0 = lw(v0 + 0x4);
    a1 = lw(v1 + 0x4);
    v0 -= a1;
    v0 = u32(i32(v0) >> 16);
    mult(a0, v0);
    a0 = lo;
    v0 = lw(s0 + 0x10);
    v1 = 0x800000;                                      // Result = 00800000
    div(v1, v0);
    if (v0 != 0) goto loc_8002CC88;
    _break(0x1C00);
loc_8002CC88:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002CCA0;
    }
    if (v1 != at) goto loc_8002CCA0;
    tge(zero, zero, 0x5D);
loc_8002CCA0:
    v1 = lo;
    v0 = lw(s0 + 0xC);
    mult(v1, v0);
    v0 = *gNumFramesDrawn;
    a0 += a1;
    sw(a0, s0 + 0x4);
    sw(v1, s0 + 0x8);
    sw(v0, s0 + 0x18);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
    if (i32(s5) <= 0) goto loc_8002CCF0;
    if (i32(s6) >= 0) goto loc_8002CCF0;
    v0 = lw(s3 + 0x4);
    sw(v0, s1 + 0x4);
    goto loc_8002CCF4;
loc_8002CCF0:
    sw(0, s1 + 0x4);
loc_8002CCF4:
    sw(s0, s1);
loc_8002CCF8:
    s2++;                                               // Result = 00000001
    v0 = (i32(s2) < 0x15);                              // Result = 00000001
    s1 += 8;
    if (v0 != 0) goto loc_8002CD18;
    I_Error("FrontZClip: Point Overflow");
loc_8002CD18:
    s7++;
    v0 = (i32(s7) < i32(fp));
    s3 += 8;
    if (v0 != 0) goto loc_8002CADC;
loc_8002CD28:
    a2 = lw(sp + 0x18);
    sw(s2, a2);
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

void R_CheckEdgeVisible() noexcept {
loc_8002CD68:
    t0 = a1 + 4;
    a3 = 0x1F800000;                                    // Result = 1F800000
    a1 = lw(a1);
    a2 = 0;                                             // Result = 00000000
    if (a0 != 0) goto loc_8002CDE0;
    v0 = (i32(a2) < i32(a1));
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8002CE38;
    t1 = 1;                                             // Result = 00000001
loc_8002CD90:
    v0 = lw(t0);
    v1 = lw(v0 + 0x10);
    v0 = lw(v0 + 0xC);
    v1 = -v1;
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002CDBC;
    sw(t1, a3);
    a2--;
    goto loc_8002CDC4;
loc_8002CDBC:
    sw(0, a3);
    a2++;
loc_8002CDC4:
    a0++;
    t0 += 8;
    v0 = (i32(a0) < i32(a1));
    a3 += 4;
    if (v0 != 0) goto loc_8002CD90;
    goto loc_8002CE38;
loc_8002CDE0:
    v0 = (i32(a2) < i32(a1));
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8002CE38;
    t1 = 1;                                             // Result = 00000001
loc_8002CDF0:
    v0 = lw(t0);
    v1 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002CE1C;
    sw(t1, a3);
    a2--;
    goto loc_8002CE24;
loc_8002CE1C:
    sw(0, a3);
    a2++;
loc_8002CE24:
    a0++;
    t0 += 8;
    v0 = (i32(a0) < i32(a1));
    a3 += 4;
    if (v0 != 0) goto loc_8002CDF0;
loc_8002CE38:
    v0 = 0x1F800000;                                    // Result = 1F800000
    v0 = lw(v0);                                        // Load from: 1F800000
    sw(v0, a3);
    if (a2 != a1) goto loc_8002CE50;
    v0 = 0;                                             // Result = 00000000
    goto loc_8002CE60;
loc_8002CE50:
    v1 = -a1;
    v0 = -1;                                            // Result = FFFFFFFF
    if (a2 == v1) goto loc_8002CE60;
    v0 = 1;                                             // Result = 00000001
loc_8002CE60:
    return;
}

void R_LeftEdgeClip() noexcept {
loc_8002CE68:
    sp -= 0x48;
    sw(s3, sp + 0x2C);
    sw(a1, sp + 0x10);
    s3 = a1 + 4;
    sw(s6, sp + 0x38);
    s6 = 0;                                             // Result = 00000000
    sw(s4, sp + 0x30);
    s4 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s5, sp + 0x34);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    s7 = lw(a0);
    fp = a0 + 4;
    if (i32(s7) <= 0) goto loc_8002D0CC;
    s1 = fp;
    s5 = 0x1F800000;                                    // Result = 1F800000
loc_8002CEC0:
    v0 = lw(s5);
    if (v0 != 0) goto loc_8002CEEC;
    v0 = lw(s1);
    v1 = lw(s1 + 0x4);
    sw(v0, s3);
    sw(v1, s3 + 0x4);
    s3 += 8;
    s4++;                                               // Result = 00000001
    v0 = lw(s5);
loc_8002CEEC:
    v1 = lw(s5 + 0x4);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8002CF08;
    }
    {
        const bool bJump = (v1 != v0);
        v0 = s7 - 1;
        if (bJump) goto loc_8002D0B8;
    }
    goto loc_8002CF10;
loc_8002CF08:
    v0 = s7 - 1;
    if (v1 != 0) goto loc_8002D0B8;
loc_8002CF10:
    v0 = (i32(s6) < i32(v0));
    s2 = fp;
    if (v0 == 0) goto loc_8002CF20;
    s2 = s1 + 8;
loc_8002CF20:
    v1 = lw(gp + 0xC40);                                // Load from: 80078220
    a0 = v1 + 1;
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7CEC;                                       // Result = 80097CEC
    sw(a0, gp + 0xC40);                                 // Store to: 80078220
    a0 = (i32(a0) < 0x20);
    s0 = v0 + v1;
    if (a0 != 0) goto loc_8002CF60;
    I_Error("LeftEdgeClip: exceeded max new vertexes\n");
loc_8002CF60:
    v1 = lw(s1);
    a0 = lw(s2);
    v0 = lw(v1 + 0xC);
    a1 = lw(v1 + 0x10);
    v1 = lw(a0 + 0xC);
    a0 = lw(a0 + 0x10);
    v0 += a1;
    v1 += a0;
    a2 = v0 << 16;
    v0 -= v1;
    div(a2, v0);
    if (v0 != 0) goto loc_8002CF98;
    _break(0x1C00);
loc_8002CF98:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002CFB0;
    }
    if (a2 != at) goto loc_8002CFB0;
    tge(zero, zero, 0x5D);
loc_8002CFB0:
    a2 = lo;
    a0 -= a1;
    mult(a2, a0);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += a1;
    sw(v0, s0 + 0x10);
    v0 = -v0;
    sw(v0, s0 + 0xC);
    v0 = lw(s2);
    v1 = lw(s1);
    v0 = lw(v0);
    v1 = lw(v1);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(a2, v0);
    v0 = lo;
    v0 += v1;
    sw(v0, s0);
    v0 = lw(s2);
    v1 = lw(s1);
    v0 = lw(v0 + 0x4);
    a1 = lw(v1 + 0x4);
    v0 -= a1;
    v0 = u32(i32(v0) >> 16);
    mult(a2, v0);
    a0 = lo;
    v0 = lw(s0 + 0x10);
    v1 = 0x800000;                                      // Result = 00800000
    div(v1, v0);
    if (v0 != 0) goto loc_8002D03C;
    _break(0x1C00);
loc_8002D03C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002D054;
    }
    if (v1 != at) goto loc_8002D054;
    tge(zero, zero, 0x5D);
loc_8002D054:
    v1 = lo;
    v0 = lw(s0 + 0xC);
    mult(v1, v0);
    v0 = *gNumFramesDrawn;
    a0 += a1;
    sw(a0, s0 + 0x4);
    sw(v1, s0 + 0x8);
    sw(v0, s0 + 0x18);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
    sw(s0, s3);
    v0 = lw(s1 + 0x4);
    s4++;                                               // Result = 00000001
    sw(v0, s3 + 0x4);
    v0 = (i32(s4) < 0x15);                              // Result = 00000001
    s3 += 8;
    if (v0 != 0) goto loc_8002D0B8;
    I_Error("LeftEdgeClip: Point Overflow");
loc_8002D0B8:
    s1 += 8;
    s6++;
    v0 = (i32(s6) < i32(s7));
    s5 += 4;
    if (v0 != 0) goto loc_8002CEC0;
loc_8002D0CC:
    a3 = lw(sp + 0x10);
    v0 = s4;                                            // Result = 00000000
    sw(s4, a3);
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

void R_RightEdgeClip() noexcept {
loc_8002D10C:
    sp -= 0x48;
    sw(s3, sp + 0x2C);
    sw(a1, sp + 0x10);
    s3 = a1 + 4;
    sw(s6, sp + 0x38);
    s6 = 0;                                             // Result = 00000000
    sw(s4, sp + 0x30);
    s4 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s5, sp + 0x34);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    s7 = lw(a0);
    fp = a0 + 4;
    if (i32(s7) <= 0) goto loc_8002D36C;
    s1 = fp;
    s5 = 0x1F800000;                                    // Result = 1F800000
loc_8002D164:
    v0 = lw(s5);
    if (v0 != 0) goto loc_8002D190;
    v0 = lw(s1);
    v1 = lw(s1 + 0x4);
    sw(v0, s3);
    sw(v1, s3 + 0x4);
    s3 += 8;
    s4++;                                               // Result = 00000001
    v0 = lw(s5);
loc_8002D190:
    v1 = lw(s5 + 0x4);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8002D1AC;
    }
    {
        const bool bJump = (v1 != v0);
        v0 = s7 - 1;
        if (bJump) goto loc_8002D358;
    }
    goto loc_8002D1B4;
loc_8002D1AC:
    v0 = s7 - 1;
    if (v1 != 0) goto loc_8002D358;
loc_8002D1B4:
    v0 = (i32(s6) < i32(v0));
    s2 = fp;
    if (v0 == 0) goto loc_8002D1C4;
    s2 = s1 + 8;
loc_8002D1C4:
    v1 = lw(gp + 0xC40);                                // Load from: 80078220
    a0 = v1 + 1;
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7CEC;                                       // Result = 80097CEC
    sw(a0, gp + 0xC40);                                 // Store to: 80078220
    a0 = (i32(a0) < 0x20);
    s0 = v0 + v1;
    if (a0 != 0) goto loc_8002D204;
    I_Error("RightEdgeClip: exceeded max new vertexes\n");
loc_8002D204:
    v1 = lw(s1);
    a0 = lw(s2);
    v0 = lw(v1 + 0xC);
    a1 = lw(v1 + 0x10);
    v1 = lw(a0 + 0xC);
    a0 = lw(a0 + 0x10);
    v0 -= a1;
    v1 -= a0;
    a2 = v0 << 16;
    v0 -= v1;
    div(a2, v0);
    if (v0 != 0) goto loc_8002D23C;
    _break(0x1C00);
loc_8002D23C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002D254;
    }
    if (a2 != at) goto loc_8002D254;
    tge(zero, zero, 0x5D);
loc_8002D254:
    a2 = lo;
    a0 -= a1;
    mult(a2, a0);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += a1;
    sw(v0, s0 + 0x10);
    sw(v0, s0 + 0xC);
    v0 = lw(s2);
    v1 = lw(s1);
    v0 = lw(v0);
    v1 = lw(v1);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(a2, v0);
    v0 = lo;
    v0 += v1;
    sw(v0, s0);
    v0 = lw(s2);
    v1 = lw(s1);
    v0 = lw(v0 + 0x4);
    a1 = lw(v1 + 0x4);
    v0 -= a1;
    v0 = u32(i32(v0) >> 16);
    mult(a2, v0);
    a0 = lo;
    v0 = lw(s0 + 0x10);
    v1 = 0x800000;                                      // Result = 00800000
    div(v1, v0);
    if (v0 != 0) goto loc_8002D2DC;
    _break(0x1C00);
loc_8002D2DC:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002D2F4;
    }
    if (v1 != at) goto loc_8002D2F4;
    tge(zero, zero, 0x5D);
loc_8002D2F4:
    v1 = lo;
    v0 = lw(s0 + 0xC);
    v1++;
    mult(v1, v0);
    v0 = *gNumFramesDrawn;
    a0 += a1;
    sw(a0, s0 + 0x4);
    sw(v1, s0 + 0x8);
    sw(v0, s0 + 0x18);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
    sw(s0, s3);
    v0 = lw(s1 + 0x4);
    s4++;                                               // Result = 00000001
    sw(v0, s3 + 0x4);
    v0 = (i32(s4) < 0x15);                              // Result = 00000001
    s3 += 8;
    if (v0 != 0) goto loc_8002D358;
    I_Error("RightEdgeClip: Point Overflow");
loc_8002D358:
    s1 += 8;
    s6++;
    v0 = (i32(s6) < i32(s7));
    s5 += 4;
    if (v0 != 0) goto loc_8002D164;
loc_8002D36C:
    a3 = lw(sp + 0x10);
    v0 = s4;                                            // Result = 00000000
    sw(s4, a3);
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
