#include "r_bsp.h"

#include "Doom/Base/i_main.h"
#include "Doom/d_main.h"
#include "Doom/Game/p_setup.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGTE.h"

void R_BSP() noexcept {
loc_8002ACE8:
    sp -= 0x18;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x70B8;                                       // Result = 800A8F48
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x6E4C;                                       // Result = gpDrawSubsectors[0] (800A91B4)
    a1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x10);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7F9C);                                // Store to: gppEndDrawSubsector (80078064)
    a2 = 0x100;                                         // Result = 00000100
    _thunk_D_memset();
    a0 = *gNumBspNodes;
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7E0C);                                 // Store to: gbIsSkyVisible (800781F4)
    a0--;
    R_RenderBSPNode();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void R_RenderBSPNode() noexcept {
loc_8002AD3C:
    sp -= 0x18;
    v1 = a0;
    v0 = v1 & 0x8000;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_8002AD84;
    v0 = -1;                                            // Result = FFFFFFFF
    a0 = 0xFFFF0000;                                    // Result = FFFF0000
    if (v1 != v0) goto loc_8002AD70;
    a0 = 0;                                             // Result = 00000000
    R_Subsector();
    goto loc_8002AE60;
loc_8002AD70:
    a0 |= 0x7FFF;                                       // Result = FFFF7FFF
    a0 &= v1;
    R_Subsector();
    goto loc_8002AE60;
loc_8002AD84:
    v0 = v1 << 3;
    v0 -= v1;
    v1 = *gpBspNodes;
    v0 <<= 3;
    s0 = v0 + v1;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EE4);                               // Load from: gViewY (80077EE4)
    v1 = lw(s0 + 0x4);
    v0 -= v1;
    v1 = lh(s0 + 0xA);
    v0 = u32(i32(v0) >> 16);
    mult(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EE0);                               // Load from: gViewX (80077EE0)
    v1 = lw(s0);
    v0 -= v1;
    v1 = lo;
    a0 = lh(s0 + 0xE);
    v0 = u32(i32(v0) >> 16);
    mult(a0, v0);
    v0 = lo;
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_8002AE28;
    a0 = s0 + 0x10;
    R_CheckBBox();
    if (v0 == 0) goto loc_8002AE0C;
    a0 = lw(s0 + 0x30);
    R_RenderBSPNode();
loc_8002AE0C:
    a0 = s0 + 0x20;
    R_CheckBBox();
    if (v0 == 0) goto loc_8002AE60;
    a0 = lw(s0 + 0x34);
    goto loc_8002AE58;
loc_8002AE28:
    a0 = s0 + 0x20;
    R_CheckBBox();
    if (v0 == 0) goto loc_8002AE44;
    a0 = lw(s0 + 0x34);
    R_RenderBSPNode();
loc_8002AE44:
    a0 = s0 + 0x10;
    R_CheckBBox();
    if (v0 == 0) goto loc_8002AE60;
    a0 = lw(s0 + 0x30);
loc_8002AE58:
    R_RenderBSPNode();
loc_8002AE60:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void R_CheckBBox() noexcept {
loc_8002AE74:
    sp -= 0x50;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE0);                               // Load from: gViewX (80077EE0)
    a3 = a0;
    sw(ra, sp + 0x48);
    sw(s5, sp + 0x44);
    sw(s4, sp + 0x40);
    sw(s3, sp + 0x3C);
    sw(s2, sp + 0x38);
    sw(s1, sp + 0x34);
    sw(s0, sp + 0x30);
    v0 = lw(a3 + 0x8);
    v0 = (i32(v1) < i32(v0));
    if (v0 == 0) goto loc_8002AEBC;
    a1 = 0;                                             // Result = 00000000
    goto loc_8002AED4;
loc_8002AEBC:
    v0 = lw(a3 + 0xC);
    v0 = (i32(v0) < i32(v1));
    a1 = 2;                                             // Result = 00000002
    if (v0 != 0) goto loc_8002AED4;
    a1 = 1;                                             // Result = 00000001
loc_8002AED4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7EE4);                               // Load from: gViewY (80077EE4)
    v0 = lw(a3);
    v0 = (i32(v0) < i32(a0));
    if (v0 == 0) goto loc_8002AEF8;
    v1 = 0;                                             // Result = 00000000
    goto loc_8002AF10;
loc_8002AEF8:
    v0 = lw(a3 + 0x4);
    v0 = (i32(a0) < i32(v0));
    v1 = 2;                                             // Result = 00000002
    if (v0 != 0) goto loc_8002AF10;
    v1 = 1;                                             // Result = 00000001
loc_8002AF10:
    v0 = v1 << 2;
    a2 = v0 + a1;
    v0 = 5;                                             // Result = 00000005
    a1 = a2 << 4;
    if (a2 == v0) goto loc_8002B1D8;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7898;                                       // Result = 80067898
    at += a1;
    v0 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x789C;                                       // Result = 8006789C
    at += a1;
    v1 = lw(at);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x78A0;                                       // Result = 800678A0
    at += a1;
    a0 = lw(at);
    v0 <<= 2;
    v0 += a3;
    v1 <<= 2;
    v1 += a3;
    a0 <<= 2;
    a0 += a3;
    s4 = lw(v0);
    s2 = lw(v1);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x78A4;                                       // Result = 800678A4
    at += a1;
    v0 = lw(at);
    s5 = lw(a0);
    v0 <<= 2;
    v0 += a3;
    s3 = lw(v0);
    v0 = 4;                                             // Result = 00000004
    if (a2 == v0) goto loc_8002AFE8;
    v0 = (i32(a2) < 5);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8002AFBC;
    }
    {
        const bool bJump = (a2 == v0);
        v0 = 0x20000;                                   // Result = 00020000
        if (bJump) goto loc_8002AFD8;
    }
    a0 = sp + 0x10;
    goto loc_8002B02C;
loc_8002AFBC:
    v0 = 6;                                             // Result = 00000006
    {
        const bool bJump = (a2 == v0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_8002AFFC;
    }
    v1 = 0xFFFE0000;                                    // Result = FFFE0000
    if (a2 == v0) goto loc_8002B010;
    a0 = sp + 0x10;
    goto loc_8002B02C;
loc_8002AFD8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE4);                               // Load from: gViewY (80077EE4)
    v1 -= s2;
    goto loc_8002B020;
loc_8002AFE8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EE0);                               // Load from: gViewX (80077EE0)
    v1 = 0xFFFE0000;                                    // Result = FFFE0000
    v0 -= s4;
    goto loc_8002B020;
loc_8002AFFC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE0);                               // Load from: gViewX (80077EE0)
    v0 = 0x20000;                                       // Result = 00020000
    v1 -= s4;
    goto loc_8002B020;
loc_8002B010:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EE4);                               // Load from: gViewY (80077EE4)
    v0 -= s2;
loc_8002B020:
    v0 = (i32(v0) < i32(v1));
    a0 = sp + 0x10;
    if (v0 == 0) goto loc_8002B1D8;
loc_8002B02C:
    s0 = sp + 0x18;
    a1 = s0;
    s1 = sp + 0x28;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE0);                               // Load from: gViewX (80077EE0)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EE4);                               // Load from: gViewY (80077EE4)
    a2 = s1;
    sh(0, sp + 0x12);
    v1 = s4 - v1;
    v1 = u32(i32(v1) >> 16);
    v0 = s2 - v0;
    v0 = u32(i32(v0) >> 16);
    sh(v1, sp + 0x10);
    sh(v0, sp + 0x14);
    LIBGTE_RotTrans();
    a0 = sp + 0x10;
    a1 = s0;
    s4 = lw(sp + 0x18);
    s2 = lw(sp + 0x20);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE0);                               // Load from: gViewX (80077EE0)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EE4);                               // Load from: gViewY (80077EE4)
    a2 = s1;
    sh(0, sp + 0x12);
    v1 = s5 - v1;
    v1 = u32(i32(v1) >> 16);
    v0 = s3 - v0;
    v0 = u32(i32(v0) >> 16);
    sh(v1, sp + 0x10);
    sh(v0, sp + 0x14);
    LIBGTE_RotTrans();
    v0 = -s2;
    v0 = (i32(s4) < i32(v0));
    s5 = lw(sp + 0x18);
    s3 = lw(sp + 0x20);
    {
        const bool bJump = (v0 == 0);
        v0 = -s3;
        if (bJump) goto loc_8002B0D4;
    }
    v0 = (i32(s5) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8002B2B0;
    }
loc_8002B0D4:
    v0 = (i32(s2) < i32(s4));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s3) < i32(s5));
        if (bJump) goto loc_8002B0E8;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8002B2B0;
    }
loc_8002B0E8:
    v0 = -s2;
    v0 = (i32(s4) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = -s3;
        if (bJump) goto loc_8002B154;
    }
    v0 = (i32(v0) < i32(s5));
    {
        const bool bJump = (v0 == 0);
        v0 = s4 + s2;
        if (bJump) goto loc_8002B154;
    }
    v1 = v0 << 16;
    v0 -= s5;
    v0 -= s3;
    div(v1, v0);
    if (v0 != 0) goto loc_8002B120;
    _break(0x1C00);
loc_8002B120:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B138;
    }
    if (v1 != at) goto loc_8002B138;
    tge(zero, zero, 0x5D);
loc_8002B138:
    v1 = lo;
    v0 = s3 - s2;
    mult(v1, v0);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    s2 += v0;
    s4 = -s2;
loc_8002B154:
    v0 = (i32(s4) < i32(s2));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s3) < i32(s5));
        if (bJump) goto loc_8002B1B8;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = s4 - s2;
        if (bJump) goto loc_8002B1B8;
    }
    v1 = v0 << 16;
    v0 -= s5;
    v0 += s3;
    div(v1, v0);
    if (v0 != 0) goto loc_8002B184;
    _break(0x1C00);
loc_8002B184:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B19C;
    }
    if (v1 != at) goto loc_8002B19C;
    tge(zero, zero, 0x5D);
loc_8002B19C:
    v1 = lo;
    v0 = s3 - s2;
    mult(v1, v0);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    s3 = s2 + v0;
    s5 = s3;
loc_8002B1B8:
    v0 = (i32(s2) < 2);
    if (i32(s2) >= 0) goto loc_8002B1C8;
    if (i32(s3) < 0) goto loc_8002B2AC;
loc_8002B1C8:
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s3) < 2);
        if (bJump) goto loc_8002B1E0;
    }
    if (v0 == 0) goto loc_8002B1E0;
loc_8002B1D8:
    v0 = 1;                                             // Result = 00000001
    goto loc_8002B2B0;
loc_8002B1E0:
    if (i32(s2) > 0) goto loc_8002B1EC;
    s2 = 1;                                             // Result = 00000001
loc_8002B1EC:
    v1 = s4 << 7;
    if (i32(s3) > 0) goto loc_8002B1F8;
    s3 = 1;                                             // Result = 00000001
loc_8002B1F8:
    div(v1, s2);
    if (s2 != 0) goto loc_8002B208;
    _break(0x1C00);
loc_8002B208:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s2 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B220;
    }
    if (v1 != at) goto loc_8002B220;
    tge(zero, zero, 0x5D);
loc_8002B220:
    v1 = lo;
    v0 = s5 << 7;
    div(v0, s3);
    if (s3 != 0) goto loc_8002B238;
    _break(0x1C00);
loc_8002B238:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s3 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B250;
    }
    if (v0 != at) goto loc_8002B250;
    tge(zero, zero, 0x5D);
loc_8002B250:
    v0 = lo;
    v1 += 0x80;
    a1 = v0 + 0x80;
    if (i32(v1) >= 0) goto loc_8002B264;
    v1 = 0;                                             // Result = 00000000
loc_8002B264:
    v0 = (i32(a1) < 0x101);
    if (v0 != 0) goto loc_8002B274;
    a1 = 0x100;                                         // Result = 00000100
loc_8002B274:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x70B8;                                       // Result = 800A8F48
    a0 = v1 + v0;
    v0 = (i32(v1) < i32(a1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8002B2B0;
    }
loc_8002B28C:
    v0 = lbu(a0);
    a0++;
    if (v0 == 0) goto loc_8002B1D8;
    v1++;
    v0 = (i32(v1) < i32(a1));
    if (v0 != 0) goto loc_8002B28C;
loc_8002B2AC:
    v0 = 0;                                             // Result = 00000000
loc_8002B2B0:
    ra = lw(sp + 0x48);
    s5 = lw(sp + 0x44);
    s4 = lw(sp + 0x40);
    s3 = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x50;
    return;
}

void R_Subsector() noexcept {
loc_8002B2D8:
    a2 = *gNumSubsectors;
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x18);
    v0 = (i32(s0) < i32(a2));
    sw(s1, sp + 0x14);
    if (v0 != 0) goto loc_8002B30C;
    I_Error("R_Subsector: ss %i with numss = %i", (int32_t) s0, (int32_t) a2);
loc_8002B30C:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7F9C);                               // Load from: gppEndDrawSubsector (80078064)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x6E4C;                                       // Result = gpDrawSubsectors[0] (800A91B4)
    v0 = a0 - v0;
    v0 = u32(i32(v0) >> 2);
    v0 = (i32(v0) < 0xC0);
    {
        const bool bJump = (v0 == 0);
        v0 = s0 << 4;
        if (bJump) goto loc_8002B3A0;
    }
    v1 = *gpSubsectors;
    v0 += v1;
    v1 = lw(v0);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7FF4);                                // Store to: gpCurSector (8007800C)
    sw(v0, a0);
    a0 = lh(v0 + 0x6);
    s0 = lh(v0 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F9C);                               // Load from: gppEndDrawSubsector (80078064)
    v1 = a0 << 2;
    v1 += a0;
    v1 <<= 3;
    a0 = *gpSegs;
    v0 += 4;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7F9C);                                // Store to: gppEndDrawSubsector (80078064)
    s1 = v1 + a0;
    if (s0 == 0) goto loc_8002B3A0;
loc_8002B38C:
    a0 = s1;
    R_AddLine();
    s0--;
    s1 += 0x28;
    if (s0 != 0) goto loc_8002B38C;
loc_8002B3A0:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void R_AddLine() noexcept {
loc_8002B3B8:
    sp -= 0x48;
    sw(s3, sp + 0x3C);
    s3 = a0;
    sw(ra, sp + 0x40);
    sw(s2, sp + 0x38);
    sw(s1, sp + 0x34);
    sw(s0, sp + 0x30);
    v0 = lhu(s3 + 0x20);
    s0 = lw(s3);
    v0 &= 0xFFFE;
    sh(v0, s3 + 0x20);
    v1 = lw(s0 + 0x18);
    v0 = *gNumFramesDrawn;
    a0 = sp + 0x10;
    if (v1 == v0) goto loc_8002B4BC;
    v0 = lw(s0);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE0);                               // Load from: gViewX (80077EE0)
    a1 = sp + 0x18;
    sh(0, sp + 0x12);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    sh(v0, sp + 0x10);
    v0 = lw(s0 + 0x4);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE4);                               // Load from: gViewY (80077EE4)
    a2 = sp + 0x28;
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    sh(v0, sp + 0x14);
    LIBGTE_RotTrans();
    v0 = lw(sp + 0x18);
    sw(v0, s0 + 0xC);
    v1 = lw(sp + 0x20);
    s2 = v0;
    s1 = v1;
    v0 = (i32(s1) < 4);
    sw(s1, s0 + 0x10);
    if (v0 != 0) goto loc_8002B4AC;
    v0 = 0x800000;                                      // Result = 00800000
    div(v0, s1);
    if (s1 != 0) goto loc_8002B474;
    _break(0x1C00);
loc_8002B474:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B48C;
    }
    if (v0 != at) goto loc_8002B48C;
    tge(zero, zero, 0x5D);
loc_8002B48C:
    v0 = lo;
    mult(s2, v0);
    sw(v0, s0 + 0x8);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
loc_8002B4AC:
    v0 = *gNumFramesDrawn;
    sw(v0, s0 + 0x18);
    goto loc_8002B4C4;
loc_8002B4BC:
    s2 = lw(s0 + 0xC);
    s1 = lw(s0 + 0x10);
loc_8002B4C4:
    s0 = lw(s3 + 0x4);
    v0 = *gNumFramesDrawn;
    v1 = lw(s0 + 0x18);
    a0 = sp + 0x10;
    if (v1 == v0) goto loc_8002B5A0;
    v0 = lw(s0);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE0);                               // Load from: gViewX (80077EE0)
    a1 = sp + 0x18;
    sh(0, sp + 0x12);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    sh(v0, sp + 0x10);
    v0 = lw(s0 + 0x4);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE4);                               // Load from: gViewY (80077EE4)
    a2 = sp + 0x28;
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    sh(v0, sp + 0x14);
    LIBGTE_RotTrans();
    v0 = lw(sp + 0x18);
    sw(v0, s0 + 0xC);
    v1 = lw(sp + 0x20);
    a1 = v0;
    a0 = v1;
    v0 = (i32(a0) < 4);
    sw(a0, s0 + 0x10);
    if (v0 != 0) goto loc_8002B590;
    v0 = 0x800000;                                      // Result = 00800000
    div(v0, a0);
    if (a0 != 0) goto loc_8002B558;
    _break(0x1C00);
loc_8002B558:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B570;
    }
    if (v0 != at) goto loc_8002B570;
    tge(zero, zero, 0x5D);
loc_8002B570:
    v0 = lo;
    mult(a1, v0);
    sw(v0, s0 + 0x8);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    v0 += 0x80;
    sw(v0, s0 + 0x14);
loc_8002B590:
    v0 = *gNumFramesDrawn;
    sw(v0, s0 + 0x18);
    goto loc_8002B5A8;
loc_8002B5A0:
    a1 = lw(s0 + 0xC);
    a0 = lw(s0 + 0x10);
loc_8002B5A8:
    v0 = -s1;
    a2 = (i32(s2) < i32(v0));
    v0 = -a0;
    if (a2 == 0) goto loc_8002B5C4;
    v0 = (i32(a1) < i32(v0));
    if (v0 != 0) goto loc_8002B988;
loc_8002B5C4:
    v0 = (i32(s1) < i32(s2));
    mult(a1, s1);
    if (v0 == 0) goto loc_8002B5DC;
    v0 = (i32(a0) < i32(a1));
    if (v0 != 0) goto loc_8002B988;
loc_8002B5DC:
    v0 = lo;
    mult(s2, a0);
    v1 = lo;
    v0 -= v1;
    if (i32(v0) <= 0) goto loc_8002B988;
    v0 = -a0;
    if (a2 == 0) goto loc_8002B658;
    v0 = (i32(v0) < i32(a1));
    {
        const bool bJump = (v0 == 0);
        v0 = s2 + s1;
        if (bJump) goto loc_8002B658;
    }
    v1 = v0 << 16;
    v0 -= a1;
    v0 -= a0;
    div(v1, v0);
    if (v0 != 0) goto loc_8002B624;
    _break(0x1C00);
loc_8002B624:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B63C;
    }
    if (v1 != at) goto loc_8002B63C;
    tge(zero, zero, 0x5D);
loc_8002B63C:
    v1 = lo;
    v0 = a0 - s1;
    mult(v1, v0);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    s1 += v0;
    s2 = -s1;
loc_8002B658:
    v0 = (i32(s2) < i32(s1));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a0) < i32(a1));
        if (bJump) goto loc_8002B6BC;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = s2 - s1;
        if (bJump) goto loc_8002B6BC;
    }
    v1 = v0 << 16;
    v0 -= a1;
    v0 += a0;
    div(v1, v0);
    if (v0 != 0) goto loc_8002B688;
    _break(0x1C00);
loc_8002B688:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B6A0;
    }
    if (v1 != at) goto loc_8002B6A0;
    tge(zero, zero, 0x5D);
loc_8002B6A0:
    v1 = lo;
    v0 = a0 - s1;
    mult(v1, v0);
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    a0 = s1 + v0;
    a1 = a0;
loc_8002B6BC:
    v0 = (i32(s1) < 3);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a0) < 3);
        if (bJump) goto loc_8002B6D0;
    }
    if (v0 != 0) goto loc_8002B988;
loc_8002B6D0:
    v0 = (i32(s1) < 2);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a0) < 3);
        if (bJump) goto loc_8002B74C;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a0) < 2);
        if (bJump) goto loc_8002B750;
    }
    v0 = 2;                                             // Result = 00000002
    v0 -= s1;
    v0 <<= 16;
    v1 = a0 - s1;
    div(v0, v1);
    if (v1 != 0) goto loc_8002B704;
    _break(0x1C00);
loc_8002B704:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B71C;
    }
    if (v0 != at) goto loc_8002B71C;
    tge(zero, zero, 0x5D);
loc_8002B71C:
    v0 = lo;
    v1 = a1 - s2;
    mult(v0, v1);
    s1 = 2;                                             // Result = 00000002
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    s2 += v0;
    goto loc_8002B7B4;
loc_8002B73C:
    t0 = a0;
    goto loc_8002B898;
loc_8002B744:
    v1 = a0;
    goto loc_8002B8D4;
loc_8002B74C:
    v0 = (i32(a0) < 2);
loc_8002B750:
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s1) < 3);
        if (bJump) goto loc_8002B7B4;
    }
    v1 = s2 << 7;
    if (v0 != 0) goto loc_8002B7B8;
    v0 = 2;                                             // Result = 00000002
    v0 -= a0;
    v0 <<= 16;
    v1 = s1 - a0;
    div(v0, v1);
    if (v1 != 0) goto loc_8002B780;
    _break(0x1C00);
loc_8002B780:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B798;
    }
    if (v0 != at) goto loc_8002B798;
    tge(zero, zero, 0x5D);
loc_8002B798:
    v0 = lo;
    v1 = s2 - a1;
    mult(v0, v1);
    a0 = 2;                                             // Result = 00000002
    v0 = lo;
    v0 = u32(i32(v0) >> 16);
    a1 += v0;
loc_8002B7B4:
    v1 = s2 << 7;
loc_8002B7B8:
    div(v1, s1);
    if (s1 != 0) goto loc_8002B7C8;
    _break(0x1C00);
loc_8002B7C8:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B7E0;
    }
    if (v1 != at) goto loc_8002B7E0;
    tge(zero, zero, 0x5D);
loc_8002B7E0:
    v1 = lo;
    v0 = a1 << 7;
    div(v0, a0);
    if (a0 != 0) goto loc_8002B7F8;
    _break(0x1C00);
loc_8002B7F8:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8002B810;
    }
    if (v0 != at) goto loc_8002B810;
    tge(zero, zero, 0x5D);
loc_8002B810:
    v0 = lo;
    a2 = v1 + 0x80;
    a3 = v0 + 0x80;
    if (a2 == a3) goto loc_8002B988;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FF4);                               // Load from: gpCurSector (8007800C)
    v1 = lw(v0 + 0xC);
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8002B848;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7E0C);                                // Store to: gbIsSkyVisible (800781F4)
loc_8002B848:
    v0 = (i32(a3) < 0x101);
    if (i32(a2) >= 0) goto loc_8002B854;
    a2 = 0;                                             // Result = 00000000
loc_8002B854:
    t0 = 0x100;                                         // Result = 00000100
    if (v0 != 0) goto loc_8002B860;
    a3 = 0x100;                                         // Result = 00000100
loc_8002B860:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x70B8;                                       // Result = 800A8F48
    a1 = a2 + v0;
    v0 = (i32(a2) < i32(a3));
    a0 = a2;
    if (v0 == 0) goto loc_8002B898;
loc_8002B878:
    v0 = lbu(a1);
    a1++;
    if (v0 == 0) goto loc_8002B73C;
    a0++;
    v0 = (i32(a0) < i32(a3));
    if (v0 != 0) goto loc_8002B878;
loc_8002B898:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x70B9;                                       // Result = 800A8F47
    a1 = a3 + v0;
    a0 = a3 - 1;
    v0 = (i32(a0) < i32(a2));
    v1 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_8002B8D4;
loc_8002B8B4:
    v0 = lbu(a1);
    a1--;
    if (v0 == 0) goto loc_8002B744;
    a0--;
    v0 = (i32(a0) < i32(a2));
    if (v0 == 0) goto loc_8002B8B4;
loc_8002B8D4:
    v0 = (i32(v1) < i32(t0));
    {
        const bool bJump = (v0 != 0);
        v0 = t0 - 1;                                    // Result = 000000FF
        if (bJump) goto loc_8002B8F8;
    }
    sh(v0, s3 + 0x22);
    v0 = lhu(s3 + 0x20);
    v1++;
    sh(v1, s3 + 0x24);
    v0 |= 1;
    sh(v0, s3 + 0x20);
loc_8002B8F8:
    v0 = lw(s3 + 0x14);
    v0 = lw(v0 + 0x10);
    v0 &= 0x200;
    if (v0 != 0) goto loc_8002B988;
    a0 = lw(s3 + 0x1C);
    if (a0 == 0) goto loc_8002B95C;
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7FF4);                               // Load from: gpCurSector (8007800C)
    v1 = lw(a0 + 0x4);
    v0 = lw(a1);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002B95C;
    v0 = lw(a0);
    v1 = lw(a1 + 0x4);
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_8002B988;
loc_8002B95C:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x70B8;                                       // Result = 800A8F48
    a1 = a2 + v0;
    v0 = (i32(a2) < i32(a3));
    v1 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8002B988;
loc_8002B974:
    sb(v1, a1);
    a2++;
    v0 = (i32(a2) < i32(a3));
    a1++;
    if (v0 != 0) goto loc_8002B974;
loc_8002B988:
    ra = lw(sp + 0x40);
    s3 = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x48;
    return;
}
