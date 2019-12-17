#include "p_password.h"

#include "Doom/d_main.h"
#include "PsxVm/PsxVm.h"

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
        const bool bJump = (v1 != at);
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
        const bool bJump = (a0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80037F38;
    }
    if (v0 != at) goto loc_80037F38;
    tge(zero, zero, 0x5D);
loc_80037F38:
    a1 = lo;
    v0 = hi;
    {
        const bool bJump = (v0 == 0);
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
        const bool bJump = (a2 != at);
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
        const bool bJump = (a3 != at);
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
    _thunk_D_memcpy();
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
        const bool bJump = (a0 != v0);
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
        const bool bJump = (v0 == 0);
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
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x12);
    v0 >>= 4;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x13);
    v0 &= 0xF;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x13);
    v0 >>= 4;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80038538;
    }
    v0 = lbu(sp + 0x14);
    v0 &= 0xF;
    v0 = (v0 < 9);
    {
        const bool bJump = (v0 == 0);
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
        const bool bJump = (v0 != 0);
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
