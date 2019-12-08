#include "p_maputl.h"

#include "PsxVm/PsxVm.h"

void P_AproxDistance() noexcept {
loc_8001C030:
    if (i32(a0) >= 0) goto loc_8001C03C;
    a0 = -a0;
loc_8001C03C:
    if (i32(a1) >= 0) goto loc_8001C048;
    a1 = -a1;
loc_8001C048:
    v0 = (i32(a0) < i32(a1));
    v1 = a0 + a1;
    if (v0 != 0) goto loc_8001C05C;
    v0 = u32(i32(a1) >> 1);
    goto loc_8001C060;
loc_8001C05C:
    v0 = u32(i32(a0) >> 1);
loc_8001C060:
    v0 = v1 - v0;
    return;
}

void P_PointOnLineSide() noexcept {
loc_8001C068:
    a3 = lw(a2 + 0x8);
    t0 = a0;
    if (a3 != 0) goto loc_8001C0AC;
    v0 = lw(a2);
    v0 = lw(v0);
    v0 = (i32(v0) < i32(t0));
    if (v0 != 0) goto loc_8001C0A0;
    v0 = lw(a2 + 0xC);
    v0 = (i32(v0) > 0);
    goto loc_8001C120;
loc_8001C0A0:
    v0 = lw(a2 + 0xC);
    v0 >>= 31;
    goto loc_8001C120;
loc_8001C0AC:
    v1 = lw(a2 + 0xC);
    if (v1 != 0) goto loc_8001C0E0;
    v0 = lw(a2);
    v0 = lw(v0 + 0x4);
    v0 = (i32(v0) < i32(a1));
    {
        const bool bJump = (v0 != 0)
        v0 = (i32(a3) > 0);
        if (bJump) goto loc_8001C120;
    }
    v0 = a3 >> 31;
    goto loc_8001C120;
loc_8001C0E0:
    a0 = lw(a2);
    v0 = lw(a0);
    v1 = u32(i32(v1) >> 16);
    v0 = t0 - v0;
    v0 = u32(i32(v0) >> 16);
    mult(v1, v0);
    v1 = u32(i32(a3) >> 16);
    v0 = lw(a0 + 0x4);
    a0 = lo;
    v0 = a1 - v0;
    v0 = u32(i32(v0) >> 16);
    mult(v0, v1);
    v0 = lo;
    v0 = (i32(v0) < i32(a0));
    v0 ^= 1;
loc_8001C120:
    return;
}

void P_PointOnDivlineSide() noexcept {
    sp -= 0x28;
    a3 = a0;
    sw(s0, sp + 0x18);
    s0 = a2;
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    a2 = lw(s0 + 0x8);
    t0 = a1;
    if (a2 != 0) goto loc_8001C17C;
    v0 = lw(s0);
    v0 = (i32(v0) < i32(a3));
    if (v0 != 0) goto loc_8001C170;
    v0 = lw(s0 + 0xC);
    v0 = (i32(v0) > 0);
    goto loc_8001C204;
loc_8001C170:
    v0 = lw(s0 + 0xC);
    v0 >>= 31;
    goto loc_8001C204;
loc_8001C17C:
    a0 = lw(s0 + 0xC);
    if (a0 != 0) goto loc_8001C1A8;
    v0 = lw(s0 + 0x4);
    v0 = (i32(v0) < i32(t0));
    {
        const bool bJump = (v0 != 0)
        v0 = (i32(a2) > 0);
        if (bJump) goto loc_8001C204;
    }
    v0 = a2 >> 31;
    goto loc_8001C204;
loc_8001C1A8:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a1 = a3 - v0;
    s1 = t0 - v1;
    v0 = a0 ^ a2;
    v0 ^= a1;
    v0 ^= s1;
    v1 = 0x80000000;                                    // Result = 80000000
    if (i32(v0) >= 0) goto loc_8001C1DC;
    v0 = a1 ^ a0;
    v0 &= v1;
    v0 = (v0 > 0);
    goto loc_8001C204;
loc_8001C1DC:
    a0 = u32(i32(a0) >> 8);
    a1 = u32(i32(a1) >> 8);
    FixedMul();
    a1 = lw(s0 + 0x8);
    a0 = u32(i32(s1) >> 8);
    s0 = v0;
    a1 = u32(i32(a1) >> 8);
    FixedMul();
    v0 = (i32(v0) < i32(s0));
    v0 ^= 1;
loc_8001C204:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void P_MakeDivline() noexcept {
loc_8001C21C:
    v0 = lw(a0);
    v0 = lw(v0);
    sw(v0, a1);
    v0 = lw(a0);
    v0 = lw(v0 + 0x4);
    sw(v0, a1 + 0x4);
    v0 = lw(a0 + 0x8);
    sw(v0, a1 + 0x8);
    v0 = lw(a0 + 0xC);
    sw(v0, a1 + 0xC);
    return;
}

void P_LineOpening() noexcept {
loc_8001C25C:
    v1 = lw(a0 + 0x20);
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 != v0) goto loc_8001C278;
    sw(0, gp + 0xC9C);                                  // Store to: gOpenRange (8007827C)
    goto loc_8001C2F0;
loc_8001C278:
    a2 = lw(a0 + 0x38);
    a0 = lw(a0 + 0x3C);
    a1 = lw(a2 + 0x4);
    v1 = lw(a0 + 0x4);
    v0 = (i32(a1) < i32(v1));
    if (v0 == 0) goto loc_8001C2A4;
    sw(a1, gp + 0xADC);                                 // Store to: gOpenTop (800780BC)
    goto loc_8001C2A8;
loc_8001C2A4:
    sw(v1, gp + 0xADC);                                 // Store to: gOpenTop (800780BC)
loc_8001C2A8:
    a1 = lw(a2);
    v1 = lw(a0);
    v0 = (i32(v1) < i32(a1));
    if (v0 == 0) goto loc_8001C2D0;
    v0 = lw(a0);
    sw(a1, gp + 0x950);                                 // Store to: gOpenBottom (80077F30)
    goto loc_8001C2D8;
loc_8001C2D0:
    v0 = lw(a2);
    sw(v1, gp + 0x950);                                 // Store to: gOpenBottom (80077F30)
loc_8001C2D8:
    sw(v0, gp + 0xBFC);                                 // Store to: gLowFloor (800781DC)
    v0 = lw(gp + 0xADC);                                // Load from: gOpenTop (800780BC)
    v1 = lw(gp + 0x950);                                // Load from: gOpenBottom (80077F30)
    v0 -= v1;
    sw(v0, gp + 0xC9C);                                 // Store to: gOpenRange (8007827C)
loc_8001C2F0:
    return;
}

void P_UnsetThingPosition() noexcept {
loc_8001C2F8:
    v0 = lw(a0 + 0x64);
    v0 &= 8;
    if (v0 != 0) goto loc_8001C35C;
    v1 = lw(a0 + 0x1C);
    if (v1 == 0) goto loc_8001C328;
    v0 = lw(a0 + 0x20);
    sw(v0, v1 + 0x20);
loc_8001C328:
    v1 = lw(a0 + 0x20);
    if (v1 == 0) goto loc_8001C344;
    v0 = lw(a0 + 0x1C);
    sw(v0, v1 + 0x1C);
    goto loc_8001C35C;
loc_8001C344:
    v0 = lw(a0 + 0xC);
    v1 = lw(v0);
    v0 = lw(a0 + 0x1C);
    sw(v0, v1 + 0x4C);
loc_8001C35C:
    v0 = lw(a0 + 0x64);
    v0 &= 0x10;
    if (v0 != 0) goto loc_8001C400;
    v1 = lw(a0 + 0x30);
    if (v1 == 0) goto loc_8001C38C;
    v0 = lw(a0 + 0x34);
    sw(v0, v1 + 0x34);
loc_8001C38C:
    v1 = lw(a0 + 0x34);
    if (v1 == 0) goto loc_8001C3A8;
    v0 = lw(a0 + 0x30);
    sw(v0, v1 + 0x30);
    goto loc_8001C400;
loc_8001C3A8:
    v0 = lw(a0 + 0x4);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7E6C);                               // Load from: gBlockmapOriginY (80078194)
    v0 -= v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D7C);                               // Load from: gBlockmapWidth (80078284)
    v0 = u32(i32(v0) >> 23);
    mult(v0, v1);
    v1 = lw(a0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7E74);                               // Load from: gBlockmapOriginX (8007818C)
    a0 = lw(a0 + 0x30);
    v1 -= v0;
    v1 = u32(i32(v1) >> 23);
    v0 = lo;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EDC);                               // Load from: gppBlockLinks (80077EDC)
    v0 <<= 2;
    v0 += v1;
    sw(a0, v0);
loc_8001C400:
    return;
}

void P_SetThingPosition() noexcept {
loc_8001C408:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a0 = lw(s0);
    a1 = lw(s0 + 0x4);
    R_PointInSubsector();
    v1 = lw(s0 + 0x64);
    v1 &= 8;
    sw(v0, s0 + 0xC);
    if (v1 != 0) goto loc_8001C468;
    v1 = lw(v0);
    sw(0, s0 + 0x20);
    v0 = lw(v1 + 0x4C);
    sw(v0, s0 + 0x1C);
    v0 = lw(v1 + 0x4C);
    if (v0 == 0) goto loc_8001C464;
    sw(s0, v0 + 0x20);
loc_8001C464:
    sw(s0, v1 + 0x4C);
loc_8001C468:
    v0 = lw(s0 + 0x64);
    v0 &= 0x10;
    if (v0 != 0) goto loc_8001C52C;
    v1 = lw(s0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7E74);                               // Load from: gBlockmapOriginX (8007818C)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E6C);                               // Load from: gBlockmapOriginY (80078194)
    v1 -= v0;
    v0 = lw(s0 + 0x4);
    v1 = u32(i32(v1) >> 23);
    v0 -= a0;
    a0 = u32(i32(v0) >> 23);
    if (i32(v1) < 0) goto loc_8001C524;
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7D7C);                               // Load from: gBlockmapWidth (80078284)
    v0 = (i32(v1) < i32(a1));
    if (v0 == 0) goto loc_8001C524;
    if (i32(a0) < 0) goto loc_8001C524;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EB8);                               // Load from: gBlockmapHeight (80077EB8)
    v0 = (i32(a0) < i32(v0));
    mult(a0, a1);
    if (v0 == 0) goto loc_8001C524;
    sw(0, s0 + 0x34);
    v0 = lo;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EDC);                               // Load from: gppBlockLinks (80077EDC)
    v0 <<= 2;
    v1 += v0;
    v0 = lw(v1);
    sw(v0, s0 + 0x30);
    v0 = lw(v1);
    if (v0 == 0) goto loc_8001C51C;
    sw(s0, v0 + 0x34);
loc_8001C51C:
    sw(s0, v1);
    goto loc_8001C52C;
loc_8001C524:
    sw(0, s0 + 0x34);
    sw(0, s0 + 0x30);
loc_8001C52C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_BlockLinesIterator() noexcept {
loc_8001C540:
    sp -= 0x30;
    sw(s2, sp + 0x28);
    s2 = a2;
    sw(ra, sp + 0x2C);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    if (i32(a0) < 0) goto loc_8001C640;
    v0 = 1;                                             // Result = 00000001
    if (i32(a1) < 0) goto loc_8001C644;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D7C);                               // Load from: gBlockmapWidth (80078284)
    v0 = (i32(a0) < i32(v1));
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001C644;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EB8);                               // Load from: gBlockmapHeight (80077EB8)
    v0 = (i32(a1) < i32(v0));
    mult(a1, v1);
    if (v0 != 0) goto loc_8001C59C;
    v0 = 1;                                             // Result = 00000001
    goto loc_8001C644;
loc_8001C59C:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7EC0);                               // Load from: gpBlockmap (80078140)
    v0 = lo;
    v0 += a0;
    v0 <<= 1;
    v0 += v1;
    v0 = lh(v0);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7F3C);                               // Load from: gpBlockmapLump (800780C4)
    v0 <<= 1;
    s0 = v0 + v1;
    v0 = -1;                                            // Result = FFFFFFFF
    v1 = lh(s0);
    a0 = lhu(s0);
    {
        const bool bJump = (v1 == v0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001C644;
    }
    s1 = -1;                                            // Result = FFFFFFFF
    v1 = a0 << 16;
loc_8001C5E4:
    v1 = u32(i32(v1) >> 16);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EB0);                               // Load from: gpLines (80077EB0)
    v0 <<= 2;
    a0 = v0 + v1;
    v0 = lw(a0 + 0x40);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    s0 += 2;
    if (v0 == v1) goto loc_8001C630;
    sw(v1, a0 + 0x40);
    pcall(s2);
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001C644;
    }
loc_8001C630:
    v0 = lh(s0);
    a0 = lhu(s0);
    v1 = a0 << 16;
    if (v0 != s1) goto loc_8001C5E4;
loc_8001C640:
    v0 = 1;                                             // Result = 00000001
loc_8001C644:
    ra = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void P_BlockThingsIterator() noexcept {
loc_8001C660:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a2;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    if (i32(a0) < 0) goto loc_8001C708;
    v0 = 1;                                             // Result = 00000001
    if (i32(a1) < 0) goto loc_8001C70C;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D7C);                               // Load from: gBlockmapWidth (80078284)
    v0 = (i32(a0) < i32(v1));
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001C70C;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EB8);                               // Load from: gBlockmapHeight (80077EB8)
    v0 = (i32(a1) < i32(v0));
    mult(a1, v1);
    if (v0 != 0) goto loc_8001C6C0;
    v0 = 1;                                             // Result = 00000001
    goto loc_8001C70C;
loc_8001C6B8:
    v0 = 0;                                             // Result = 00000000
    goto loc_8001C70C;
loc_8001C6C0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EDC);                               // Load from: gppBlockLinks (80077EDC)
    v0 = lo;
    v0 += a0;
    v0 <<= 2;
    v0 += v1;
    s0 = lw(v0);
    v0 = 1;                                             // Result = 00000001
    if (s0 == 0) goto loc_8001C70C;
loc_8001C6E8:
    a0 = s0;
    pcall(s1);
    if (v0 == 0) goto loc_8001C6B8;
    s0 = lw(s0 + 0x30);
    if (s0 != 0) goto loc_8001C6E8;
loc_8001C708:
    v0 = 1;                                             // Result = 00000001
loc_8001C70C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
