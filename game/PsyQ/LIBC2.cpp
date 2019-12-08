#include "LIBC2.h"

#include "LIBAPI.h"
#include "PsxVm/PsxVm.h"

void LIBC2_sprintf() noexcept {
loc_80049E5C:
    sw(a1, sp + 0x4);
    sw(a2, sp + 0x8);
    sw(a3, sp + 0xC);
    sp -= 0x250;
    sw(s3, sp + 0x23C);
    sw(ra, sp + 0x24C);
    sw(s6, sp + 0x248);
    sw(s5, sp + 0x244);
    sw(s4, sp + 0x240);
    sw(s2, sp + 0x238);
    v0 = sp + 0x258;
    sw(v0, sp + 0x220);
    sw(s1, sp + 0x234);
    sw(s0, sp + 0x230);
    sw(a1, sp + 0x254);
    a1 = lb(a1);
    s3 = a0;
    s2 = 0;                                             // Result = 00000000
    if (a1 == 0) goto loc_8004A67C;
    s6 = 0x2D;                                          // Result = 0000002D
    s5 = 0x2B;                                          // Result = 0000002B
    s4 = 0x20;                                          // Result = 00000020
    v0 = 0x25;                                          // Result = 00000025
loc_80049EB8:
    {
        const bool bJump = (a1 != v0);
        v0 = s3 + s2;
        if (bJump) goto loc_8004A5C0;
    }
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x5B68;                                       // Result = 80075B68
    v1 = lw(a1 + 0x4);                                  // Load from: 80075B6C
    v0 = lw(a1);                                        // Load from: 80075B68
    a0 = lw(a1 + 0x8);                                  // Load from: 80075B70
    sw(v1, sp + 0x214);
    sw(v0, sp + 0x210);
    v1 = 0x30;                                          // Result = 00000030
    a2 = 0x23;                                          // Result = 00000023
    sw(a0, sp + 0x218);
loc_80049EE8:
    a0 = lw(sp + 0x254);
    v0 = a0 + 1;
    sw(v0, sp + 0x254);
    a1 = lb(a0 + 0x1);
    if (a1 != s6) goto loc_80049F1C;
    v0 = lw(sp + 0x210);
    v0 |= 1;
    sw(v0, sp + 0x210);
    goto loc_80049EE8;
loc_80049F1C:
    if (a1 != s5) goto loc_80049F38;
    v0 = lw(sp + 0x210);
    v0 |= 2;
    sw(v0, sp + 0x210);
    goto loc_80049EE8;
loc_80049F38:
    if (a1 != s4) goto loc_80049F48;
    sb(a1, sp + 0x211);
    goto loc_80049EE8;
loc_80049F48:
    if (a1 != a2) goto loc_80049F64;
    v0 = lw(sp + 0x210);
    v0 |= 4;
    sw(v0, sp + 0x210);
    goto loc_80049EE8;
loc_80049F64:
    v0 = 0x2A;                                          // Result = 0000002A
    if (a1 != v1) goto loc_80049F80;
    v0 = lw(sp + 0x210);
    v0 |= 8;
    sw(v0, sp + 0x210);
    goto loc_80049EE8;
loc_80049F80:
    {
        const bool bJump = (a1 != v0);
        v0 = a1 - 0x30;
        if (bJump) goto loc_8004A00C;
    }
    v1 = lw(sp + 0x220);
    v0 = v1 + 4;
    sw(v0, sp + 0x220);
    v0 = lw(v1);
    sw(v0, sp + 0x214);
    if (i32(v0) >= 0) goto loc_80049FBC;
    v1 = lw(sp + 0x210);
    v0 = -v0;
    v1 |= 1;
    sw(v1, sp + 0x210);
    sw(v0, sp + 0x214);
loc_80049FBC:
    v0 = a0 + 2;
    sw(v0, sp + 0x254);
    a1 = lb(a0 + 0x2);
    v0 = 0x2E;                                          // Result = 0000002E
    goto loc_8004A018;
loc_80049FD4:
    v1 = lw(sp + 0x214);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 1;
    v0 -= 0x30;
    v1 = lw(sp + 0x254);
    v0 += a1;
    sw(v0, sp + 0x214);
    v0 = v1 + 1;
    sw(v0, sp + 0x254);
    a1 = lb(v1 + 0x1);
    v0 = a1 - 0x30;
loc_8004A00C:
    v0 = (v0 < 0xA);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x2E;                                      // Result = 0000002E
        if (bJump) goto loc_80049FD4;
    }
loc_8004A018:
    if (a1 != v0) goto loc_8004A0D4;
    a0 = lw(sp + 0x254);
    v0 = a0 + 1;
    sw(v0, sp + 0x254);
    a1 = lb(a0 + 0x1);
    v0 = 0x2A;                                          // Result = 0000002A
    {
        const bool bJump = (a1 != v0);
        v0 = a1 - 0x30;
        if (bJump) goto loc_8004A0A8;
    }
    v1 = lw(sp + 0x220);
    v0 = v1 + 4;
    sw(v0, sp + 0x220);
    v0 = lw(v1);
    sw(v0, sp + 0x218);
    v0 = a0 + 2;
    sw(v0, sp + 0x254);
    a1 = lb(a0 + 0x2);
    goto loc_8004A0B4;
loc_8004A070:
    v1 = lw(sp + 0x218);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 1;
    v0 -= 0x30;
    v1 = lw(sp + 0x254);
    v0 += a1;
    sw(v0, sp + 0x218);
    v0 = v1 + 1;
    sw(v0, sp + 0x254);
    a1 = lb(v1 + 0x1);
    v0 = a1 - 0x30;
loc_8004A0A8:
    v0 = (v0 < 0xA);
    if (v0 != 0) goto loc_8004A070;
loc_8004A0B4:
    v0 = lw(sp + 0x218);
    if (i32(v0) < 0) goto loc_8004A0D4;
    v0 = lw(sp + 0x210);
    v0 |= 0x10;
    sw(v0, sp + 0x210);
loc_8004A0D4:
    v1 = lw(sp + 0x210);
    v0 = v1 & 1;
    s1 = sp + 0x210;
    if (v0 == 0) goto loc_8004A0F4;
    v0 = -9;                                            // Result = FFFFFFF7
    v0 &= v1;
    sw(v0, sp + 0x210);
loc_8004A0F4:
    v1 = a1 - 0x4C;
loc_8004A0F8:
    v0 = (v1 < 0x2D);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_8004A5B4;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += v0;
    v0 = lw(at + 0x1790);
    switch (v0) {
        case 0x8004A13C: goto loc_8004A13C;
        case 0x8004A5B4: goto loc_8004A5B4;
        case 0x8004A3CC: goto loc_8004A3CC;
        case 0x8004A4D4: goto loc_8004A4D4;
        case 0x8004A168: goto loc_8004A168;
        case 0x8004A11C: goto loc_8004A11C;
        case 0x8004A12C: goto loc_8004A12C;
        case 0x8004A580: goto loc_8004A580;
        case 0x8004A2C8: goto loc_8004A2C8;
        case 0x8004A3B8: goto loc_8004A3B8;
        case 0x8004A4F4: goto loc_8004A4F4;
        case 0x8004A1B8: goto loc_8004A1B8;
        case 0x8004A3D8: goto loc_8004A3D8;
        default: jump_table_err(); break;
    }
loc_8004A11C:
    v0 = lw(sp + 0x210);
    v0 |= 0x20;
    goto loc_8004A148;
loc_8004A12C:
    v0 = lw(sp + 0x210);
    v0 |= 0x40;
    goto loc_8004A148;
loc_8004A13C:
    v0 = lw(sp + 0x210);
    v0 |= 0x80;
loc_8004A148:
    v1 = lw(sp + 0x254);
    sw(v0, sp + 0x210);
    v0 = v1 + 1;
    sw(v0, sp + 0x254);
    a1 = lb(v1 + 0x1);
    v1 = a1 - 0x4C;
    goto loc_8004A0F8;
loc_8004A168:
    v1 = lw(sp + 0x220);
    v0 = v1 + 4;
    sw(v0, sp + 0x220);
    a0 = lw(v1);
    v1 = lw(sp + 0x210);
    v0 = v1 & 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = a0 << 16;
        if (bJump) goto loc_8004A194;
    }
    a0 = u32(i32(v0) >> 16);
loc_8004A194:
    v0 = v1 & 2;
    if (i32(a0) >= 0) goto loc_8004A1A8;
    a0 = -a0;
    sb(s6, sp + 0x211);
    goto loc_8004A1E0;
loc_8004A1A8:
    if (v0 == 0) goto loc_8004A1E0;
    sb(s5, sp + 0x211);
    goto loc_8004A1E0;
loc_8004A1B8:
    v1 = lw(sp + 0x220);
    v0 = v1 + 4;
    sw(v0, sp + 0x220);
    v0 = lw(sp + 0x210);
    a0 = lw(v1);
    v0 &= 0x20;
    sb(0, sp + 0x211);
    if (v0 == 0) goto loc_8004A1E0;
    a0 &= 0xFFFF;
loc_8004A1E0:
    v1 = lw(sp + 0x210);
    v0 = v1 & 0x10;
    {
        const bool bJump = (v0 != 0);
        v0 = v1 & 8;
        if (bJump) goto loc_8004A22C;
    }
    if (v0 == 0) goto loc_8004A218;
    v1 = lw(sp + 0x214);
    v0 = lb(sp + 0x211);
    sw(v1, sp + 0x218);
    if (v0 == 0) goto loc_8004A218;
    v0 = v1 - 1;
    sw(v0, sp + 0x218);
loc_8004A218:
    v0 = lw(sp + 0x218);
    {
        const bool bJump = (i32(v0) > 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8004A22C;
    }
    sw(v0, sp + 0x218);
loc_8004A22C:
    s0 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_8004A270;
    a1 = 0xCCCC0000;                                    // Result = CCCC0000
    a1 |= 0xCCCD;                                       // Result = CCCCCCCD
loc_8004A23C:
    multu(a0, a1);
    s1--;
    s0++;
    v1 = hi;
    v1 >>= 3;
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 1;
    v0 = a0 - v0;
    v0 += 0x30;
    a0 = v1;
    sb(v0, s1);
    if (a0 != 0) goto loc_8004A23C;
loc_8004A270:
    v0 = lw(sp + 0x218);
    v0 = (i32(s0) < i32(v0));
    if (v0 == 0) goto loc_8004A2A8;
    v1 = 0x30;                                          // Result = 00000030
    s1--;
loc_8004A28C:
    sb(v1, s1);
    v0 = lw(sp + 0x218);
    s0++;
    v0 = (i32(s0) < i32(v0));
    s1--;
    if (v0 != 0) goto loc_8004A28C;
    s1++;
loc_8004A2A8:
    v0 = lb(sp + 0x211);
    v1 = v0;
    if (v0 == 0) goto loc_8004A5CC;
    s1--;
    sb(v1, s1);
    s0++;
    goto loc_8004A5CC;
loc_8004A2C8:
    v1 = lw(sp + 0x220);
    v0 = v1 + 4;
    sw(v0, sp + 0x220);
    a0 = lw(v1);
    v1 = lw(sp + 0x210);
    v0 = v1 & 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 0x10;
        if (bJump) goto loc_8004A2F4;
    }
    a0 &= 0xFFFF;
loc_8004A2F4:
    {
        const bool bJump = (v0 != 0);
        v0 = v1 & 8;
        if (bJump) goto loc_8004A324;
    }
    if (v0 == 0) goto loc_8004A310;
    v0 = lw(sp + 0x214);
    sw(v0, sp + 0x218);
loc_8004A310:
    v0 = lw(sp + 0x218);
    {
        const bool bJump = (i32(v0) > 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8004A324;
    }
    sw(v0, sp + 0x218);
loc_8004A324:
    s0 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_8004A348;
loc_8004A32C:
    v0 = a0 & 7;
    v0 += 0x30;
    sb(v0, s1 - 0x1);
    a0 >>= 3;
    s1--;
    s0++;
    if (a0 != 0) goto loc_8004A32C;
loc_8004A348:
    v0 = lw(sp + 0x210);
    v0 &= 4;
    if (v0 == 0) goto loc_8004A380;
    v0 = 0x30;                                          // Result = 00000030
    if (s0 == 0) goto loc_8004A380;
    v1 = lb(s1);
    {
        const bool bJump = (v1 == v0);
        v0 = 0x30;                                      // Result = 00000030
        if (bJump) goto loc_8004A380;
    }
    s1--;
    sb(v0, s1);
    s0++;
loc_8004A380:
    v0 = lw(sp + 0x218);
    v0 = (i32(s0) < i32(v0));
    v1 = 0x30;                                          // Result = 00000030
    if (v0 == 0) goto loc_8004A5CC;
    s1--;
loc_8004A398:
    sb(v1, s1);
    v0 = lw(sp + 0x218);
    s0++;
    v0 = (i32(s0) < i32(v0));
    s1--;
    if (v0 != 0) goto loc_8004A398;
    s1++;
    goto loc_8004A5CC;
loc_8004A3B8:
    v1 = lw(sp + 0x210);
    v0 = 8;                                             // Result = 00000008
    v1 |= 0x50;
    sw(v1, sp + 0x210);
    sw(v0, sp + 0x218);
loc_8004A3CC:
    a3 = 0x80010000;                                    // Result = 80010000
    a3 += 0x1768;                                       // Result = STR_HexChars_Upper_Copy1[0] (80011768)
    goto loc_8004A3E0;
loc_8004A3D8:
    a3 = 0x80010000;                                    // Result = 80010000
    a3 += 0x177C;                                       // Result = STR_HexChars_Lower_Copy1[0] (8001177C)
loc_8004A3E0:
    v1 = lw(sp + 0x220);
    v0 = v1 + 4;
    sw(v0, sp + 0x220);
    a0 = lw(v1);
    v1 = lw(sp + 0x210);
    v0 = v1 & 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 0x10;
        if (bJump) goto loc_8004A40C;
    }
    a0 &= 0xFFFF;
loc_8004A40C:
    {
        const bool bJump = (v0 != 0);
        v0 = v1 & 8;
        if (bJump) goto loc_8004A448;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 4;
        if (bJump) goto loc_8004A434;
    }
    a2 = lw(sp + 0x214);
    sw(a2, sp + 0x218);
    if (v0 == 0) goto loc_8004A434;
    v0 = a2 - 2;
    sw(v0, sp + 0x218);
loc_8004A434:
    v0 = lw(sp + 0x218);
    {
        const bool bJump = (i32(v0) > 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8004A448;
    }
    sw(v0, sp + 0x218);
loc_8004A448:
    s0 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_8004A470;
loc_8004A450:
    v0 = a0 & 0xF;
    v0 += a3;
    v0 = lbu(v0);
    a0 >>= 4;
    s1--;
    s0++;
    sb(v0, s1);
    if (a0 != 0) goto loc_8004A450;
loc_8004A470:
    v0 = lw(sp + 0x218);
    v0 = (i32(s0) < i32(v0));
    if (v0 == 0) goto loc_8004A4A8;
    v1 = 0x30;                                          // Result = 00000030
    s1--;
loc_8004A48C:
    sb(v1, s1);
    v0 = lw(sp + 0x218);
    s0++;
    v0 = (i32(s0) < i32(v0));
    s1--;
    if (v0 != 0) goto loc_8004A48C;
    s1++;
loc_8004A4A8:
    v0 = lw(sp + 0x210);
    v0 &= 4;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x30;                                      // Result = 00000030
        if (bJump) goto loc_8004A5CC;
    }
    s1--;
    s1--;
    sb(a1, s1 + 0x1);
    s0 += 2;
    sb(v0, s1);
    goto loc_8004A5CC;
loc_8004A4D4:
    v0 = lw(sp + 0x220);
    s1--;
    v1 = v0 + 4;
    sw(v1, sp + 0x220);
    v0 = lbu(v0);
    s0 = 1;                                             // Result = 00000001
    sb(v0, s1);
    goto loc_8004A5CC;
loc_8004A4F4:
    v0 = lw(sp + 0x220);
    v1 = v0 + 4;
    sw(v1, sp + 0x220);
    v1 = lw(sp + 0x210);
    s1 = lw(v0);
    v0 = v1 & 4;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 0x10;
        if (bJump) goto loc_8004A544;
    }
    s0 = lbu(s1);
    s1++;
    if (v0 == 0) goto loc_8004A5CC;
    v1 = lw(sp + 0x218);
    v0 = (i32(v1) < i32(s0));
    if (v0 == 0) goto loc_8004A5CC;
    s0 = v1;
    goto loc_8004A5CC;
loc_8004A544:
    a0 = s1;
    if (v0 != 0) goto loc_8004A55C;
    a0 = s1;
    LIBC2_strlen();
    s0 = v0;
    goto loc_8004A5CC;
loc_8004A55C:
    a2 = lw(sp + 0x218);
    a1 = 0;                                             // Result = 00000000
    LIBC2_memchr();
    s0 = v0 - s1;
    if (v0 != 0) goto loc_8004A5CC;
    s0 = lw(sp + 0x218);
    goto loc_8004A5CC;
loc_8004A580:
    v0 = lw(sp + 0x220);
    v1 = v0 + 4;
    sw(v1, sp + 0x220);
    v1 = lw(sp + 0x210);
    s1 = lw(v0);
    v0 = v1 & 0x20;
    if (v0 == 0) goto loc_8004A5AC;
    sh(s2, s1);
    goto loc_8004A65C;
loc_8004A5AC:
    sw(s2, s1);
    goto loc_8004A65C;
loc_8004A5B4:
    v0 = 0x25;                                          // Result = 00000025
    {
        const bool bJump = (a1 != v0);
        v0 = s3 + s2;
        if (bJump) goto loc_8004A67C;
    }
loc_8004A5C0:
    sb(a1, v0);
    s2++;
    goto loc_8004A65C;
loc_8004A5CC:
    v0 = lw(sp + 0x214);
    v0 = (i32(s0) < i32(v0));
    a0 = s3 + s2;
    if (v0 == 0) goto loc_8004A61C;
    v0 = lw(sp + 0x210);
    v0 &= 1;
    a1 = s1;
    if (v0 != 0) goto loc_8004A620;
    v1 = s2 + s3;
loc_8004A5F8:
    sb(s4, v1);
    v0 = lw(sp + 0x214);
    v1++;
    v0--;
    sw(v0, sp + 0x214);
    v0 = (i32(s0) < i32(v0));
    s2++;
    if (v0 != 0) goto loc_8004A5F8;
    a0 = s3 + s2;
loc_8004A61C:
    a1 = s1;
loc_8004A620:
    a2 = s0;
    LIBC2_memmove();
    v0 = lw(sp + 0x214);
    v0 = (i32(s0) < i32(v0));
    s2 += s0;
    if (v0 == 0) goto loc_8004A65C;
    v1 = s2 + s3;
loc_8004A640:
    sb(s4, v1);
    v0 = lw(sp + 0x214);
    s0++;
    v1++;
    v0 = (i32(s0) < i32(v0));
    s2++;
    if (v0 != 0) goto loc_8004A640;
loc_8004A65C:
    v1 = lw(sp + 0x254);
    v0 = v1 + 1;
    sw(v0, sp + 0x254);
    a1 = lb(v1 + 0x1);
    v0 = 0x25;                                          // Result = 00000025
    if (a1 != 0) goto loc_80049EB8;
loc_8004A67C:
    v0 = s3 + s2;
    sb(0, v0);
    ra = lw(sp + 0x24C);
    v0 = s2;
    s2 = lw(sp + 0x238);
    s0 = lw(sp + 0x230);
    s1 = lw(sp + 0x234);
    s3 = lw(sp + 0x23C);
    s4 = lw(sp + 0x240);
    s5 = lw(sp + 0x244);
    s6 = lw(sp + 0x248);
    sp += 0x250;
    return;
}

void LIBC2_memchr() noexcept {
loc_8004A6BC:
    v0 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_8004A704;
    if (i32(a2) <= 0) goto loc_8004A704;
    a2--;
    goto loc_8004A6DC;
loc_8004A6D4:
    v0 = a0 - 1;
    goto loc_8004A704;
loc_8004A6DC:
    v0 = 0;                                             // Result = 00000000
    if (i32(a2) < 0) goto loc_8004A704;
    a1 &= 0xFF;
loc_8004A6E8:
    v0 = lbu(a0);
    a0++;
    if (v0 == a1) goto loc_8004A6D4;
    a2--;
    v0 = 0;                                             // Result = 00000000
    if (i32(a2) >= 0) goto loc_8004A6E8;
loc_8004A704:
    return;
}

void LIBC2_strlen() noexcept {
loc_8004A70C:
    v1 = 0;                                             // Result = 00000000
    if (a0 != 0) goto loc_8004A720;
    v0 = 0;                                             // Result = 00000000
    goto loc_8004A734;
loc_8004A71C:
    v1++;
loc_8004A720:
    v0 = lb(a0);
    a0++;
    if (v0 != 0) goto loc_8004A71C;
    v0 = v1;
loc_8004A734:
    return;
}

void LIBC2_memmove() noexcept {
loc_8004A73C:
    v0 = (a0 < a1);
    {
        const bool bJump = (v0 != 0);
        v0 = a2;
        if (bJump) goto loc_8004A77C;
    }
    a2--;
    if (i32(v0) <= 0) goto loc_8004A7A0;
    a3 = a2 + a0;
    a1 += a2;
loc_8004A758:
    v0 = lbu(a1);
    v1 = a2;
    a2--;
    a1--;
    sb(v0, a3);
    a3--;
    if (i32(v1) > 0) goto loc_8004A758;
    goto loc_8004A7A0;
loc_8004A77C:
    a2--;
    if (i32(v0) <= 0) goto loc_8004A7A0;
loc_8004A784:
    v0 = lbu(a1);
    v1 = a2;
    a2--;
    a1++;
    sb(v0, a0);
    a0++;
    if (i32(v1) > 0) goto loc_8004A784;
loc_8004A7A0:
    v0 = a0;
    return;
}

void LIBC2_printf() noexcept {
loc_8004AD90:
    sp -= 0x18;
    v0 = a0;
    sw(a1, sp + 0x1C);
    sw(a2, sp + 0x20);
    sw(v0, sp + 0x18);
    a2 = sp + 0x1C;
    a1 = v0;
    a0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x10);
    sw(a3, sp + 0x24);
    LIBC2_prnt();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBC2_setjmp() noexcept {
loc_8004ADD0:
    sw(ra, a0);
    sw(gp, a0 + 0x2C);
    sw(sp, a0 + 0x4);
    sw(fp, a0 + 0x8);
    sw(s0, a0 + 0xC);
    sw(s1, a0 + 0x10);
    sw(s2, a0 + 0x14);
    sw(s3, a0 + 0x18);
    sw(s4, a0 + 0x1C);
    sw(s5, a0 + 0x20);
    sw(s6, a0 + 0x24);
    sw(s7, a0 + 0x28);
    v0 = 0;                                             // Result = 00000000
    return;
}

void LIBC2_longjmp() noexcept {
    ra = lw(a0);
    gp = lw(a0 + 0x2C);
    sp = lw(a0 + 0x4);
    fp = lw(a0 + 0x8);
    s0 = lw(a0 + 0xC);
    s1 = lw(a0 + 0x10);
    s2 = lw(a0 + 0x14);
    s3 = lw(a0 + 0x18);
    s4 = lw(a0 + 0x1C);
    s5 = lw(a0 + 0x20);
    s6 = lw(a0 + 0x24);
    s7 = lw(a0 + 0x28);
    v0 = a1;
    return;
}

void LIBC2_prnt() noexcept {
loc_8004AE50:
    sp -= 0x1C0;
    sw(s1, sp + 0x19C);
    sw(ra, sp + 0x1BC);
    sw(fp, sp + 0x1B8);
    sw(s7, sp + 0x1B4);
    sw(s6, sp + 0x1B0);
    sw(s5, sp + 0x1AC);
    sw(s4, sp + 0x1A8);
    sw(s3, sp + 0x1A4);
    s1 = a2;
    sw(s2, sp + 0x1A0);
    sw(s0, sp + 0x198);
    if (a1 != 0) goto loc_8004AE8C;
    v0 = 0;                                             // Result = 00000000
    goto loc_8004B484;
loc_8004AE8C:
    a3 = 0x80010000;                                    // Result = 80010000
    a3 += 0x1898;                                       // Result = STR_HexChars_Lower_Copy2[0] (80011898)
    sw(a3, sp + 0x190);
    s7 = a1;
    sw(0, sp + 0x170);
loc_8004AEA0:
    a0 = lbu(s7);
    v0 = 0x25;                                          // Result = 00000025
    if (a0 != 0) goto loc_8004AEBC;
loc_8004AEB0:
    v0 = lw(sp + 0x170);
    goto loc_8004B484;
loc_8004AEBC:
    s3 = 0;                                             // Result = 00000000
    if (a0 != v0) goto loc_8004B474;
    sw(0, sp + 0x178);
    sw(0, sp + 0x180);
    s4 = -1;                                            // Result = FFFFFFFF
    fp = 0;                                             // Result = 00000000
    sw(0, sp + 0x188);
loc_8004AED8:
    v1 = lbu(s7 + 0x1);
    s7++;
    v0 = (v1 < 0x79);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_8004B464;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += v0;
    v0 = lw(at + 0x18C8);
    switch (v0) {
        case 0x8004AEB0: goto loc_8004AEB0;
        case 0x8004B464: goto loc_8004B464;
        case 0x8004AF04: goto loc_8004AF04;
        case 0x8004AF14: goto loc_8004AF14;
        case 0x8004AF1C: goto loc_8004AF1C;
        case 0x8004AF3C: goto loc_8004AF3C;
        case 0x8004AF34: goto loc_8004AF34;
        case 0x8004AF44: goto loc_8004AF44;
        case 0x8004AFCC: goto loc_8004AFCC;
        case 0x8004AFD4: goto loc_8004AFD4;
        case 0x8004B05C: goto loc_8004B05C;
        case 0x8004B028: goto loc_8004B028;
        case 0x8004B0D8: goto loc_8004B0D8;
        case 0x8004B17C: goto loc_8004B17C;
        case 0x8004B1B0: goto loc_8004B1B0;
        case 0x8004B040: goto loc_8004B040;
        case 0x8004B060: goto loc_8004B060;
        case 0x8004B030: goto loc_8004B030;
        case 0x8004B038: goto loc_8004B038;
        case 0x8004B09C: goto loc_8004B09C;
        case 0x8004B0DC: goto loc_8004B0DC;
        case 0x8004B10C: goto loc_8004B10C;
        case 0x8004B11C: goto loc_8004B11C;
        case 0x8004B180: goto loc_8004B180;
        case 0x8004B1BC: goto loc_8004B1BC;
        default: jump_table_err(); break;
    }
loc_8004AF04:
    if (fp != 0) goto loc_8004AED8;
    fp = 0x20;                                          // Result = 00000020
    goto loc_8004AED8;
loc_8004AF14:
    s3 |= 8;
    goto loc_8004AED8;
loc_8004AF1C:
    a3 = lw(s1);
    s1 += 4;
    sw(a3, sp + 0x188);
    if (i32(a3) >= 0) goto loc_8004AED8;
    a3 = -a3;
    sw(a3, sp + 0x188);
loc_8004AF34:
    s3 |= 0x10;
    goto loc_8004AED8;
loc_8004AF3C:
    fp = 0x2B;                                          // Result = 0000002B
    goto loc_8004AED8;
loc_8004AF44:
    v1 = lbu(s7 + 0x1);
    s7++;
    v0 = 0x2A;                                          // Result = 0000002A
    {
        const bool bJump = (v1 != v0);
        v0 = (v1 < 0x80);
        if (bJump) goto loc_8004AF68;
    }
    s0 = lw(s1);
    s1 += 4;
    s4 = s0;
    goto loc_8004AFB8;
loc_8004AF68:
    s0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8004AFB0;
loc_8004AF70:
    a0 = lbu(s7);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += a0;
    v0 = lbu(v0 + 0x5C75);
    v0 &= 4;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 << 2;
        if (bJump) goto loc_8004AFB0;
    }
    v1 = lbu(s7 + 0x1);
    v0 += s0;
    v0 <<= 1;
    v0 -= 0x30;
    s7++;
    v1 = (v1 < 0x80);
    s0 = v0 + a0;
    if (v1 != 0) goto loc_8004AF70;
loc_8004AFB0:
    s7--;
    s4 = s0;
loc_8004AFB8:
    v0 = (i32(s4) < -1);
    if (v0 == 0) goto loc_8004AED8;
    s4 = -1;                                            // Result = FFFFFFFF
    goto loc_8004AED8;
loc_8004AFCC:
    s3 |= 0x20;
    goto loc_8004AED8;
loc_8004AFD4:
    s0 = 0;                                             // Result = 00000000
loc_8004AFD8:
    v1 = s0 << 2;
    v1 += s0;
    v0 = lbu(s7);
    v1 <<= 1;
    a0 = lbu(s7 + 0x1);
    v1 -= 0x30;
    s0 = v1 + v0;
    v0 = (a0 < 0x80);
    s7++;
    if (v0 == 0) goto loc_8004B01C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += a0;
    v0 = lbu(v0 + 0x5C75);
    v0 &= 4;
    if (v0 != 0) goto loc_8004AFD8;
loc_8004B01C:
    sw(s0, sp + 0x188);
    s7--;
    goto loc_8004AED8;
loc_8004B028:
    s3 |= 2;
    goto loc_8004AED8;
loc_8004B030:
    s3 |= 4;
    goto loc_8004AED8;
loc_8004B038:
    s3 |= 1;
    goto loc_8004AED8;
loc_8004B040:
    v0 = lbu(s1);
    s2 = sp + 0x10;
    s1 += 4;
    s6 = 1;                                             // Result = 00000001
    fp = 0;                                             // Result = 00000000
    sb(v0, sp + 0x10);
    goto loc_8004B29C;
loc_8004B05C:
    s3 |= 1;
loc_8004B060:
    v0 = s3 & 1;
    {
        const bool bJump = (v0 != 0);
        v0 = s3 & 4;
        if (bJump) goto loc_8004B080;
    }
    if (v0 == 0) goto loc_8004B080;
    v1 = lh(s1);
    s1 += 4;
    goto loc_8004B088;
loc_8004B080:
    v1 = lw(s1);
    s1 += 4;
loc_8004B088:
    a1 = 0xA;                                           // Result = 0000000A
    if (i32(v1) >= 0) goto loc_8004B200;
    v1 = -v1;
    fp = 0x2D;                                          // Result = 0000002D
    goto loc_8004B200;
loc_8004B09C:
    v0 = s3 & 1;
    {
        const bool bJump = (v0 != 0);
        v0 = s3 & 4;
        if (bJump) goto loc_8004B0C4;
    }
    if (v0 == 0) goto loc_8004B0C4;
    v0 = lw(s1);
    a3 = lhu(sp + 0x170);
    s1 += 4;
    sh(a3, v0);
    goto loc_8004B47C;
loc_8004B0C4:
    v0 = lw(s1);
    a3 = lw(sp + 0x170);
    s1 += 4;
    sw(a3, v0);
    goto loc_8004B47C;
loc_8004B0D8:
    s3 |= 1;
loc_8004B0DC:
    v0 = s3 & 1;
    {
        const bool bJump = (v0 != 0);
        v0 = s3 & 4;
        if (bJump) goto loc_8004B0FC;
    }
    a1 = 8;                                             // Result = 00000008
    if (v0 == 0) goto loc_8004B0FC;
    v1 = lh(s1);
    s1 += 4;
    goto loc_8004B1FC;
loc_8004B0FC:
    v1 = lw(s1);
    s1 += 4;
    a1 = 8;                                             // Result = 00000008
    goto loc_8004B1FC;
loc_8004B10C:
    v1 = lw(s1);
    s1 += 4;
    a1 = 0x10;                                          // Result = 00000010
    goto loc_8004B1FC;
loc_8004B11C:
    s2 = lw(s1);
    s1 += 4;
    if (s2 != 0) goto loc_8004B134;
    s2 = 0x80010000;                                    // Result = 80010000
    s2 += 0x18AC;                                       // Result = STR_Null_In_Parens[0] (800118AC)
loc_8004B134:
    a0 = s2;
    if (i32(s4) < 0) goto loc_8004B168;
    a1 = 0;                                             // Result = 00000000
    a2 = s4;
    LIBC2_memchr();
    fp = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8004B160;
    s6 = v0 - s2;
    v0 = (i32(s4) < i32(s6));
    if (v0 == 0) goto loc_8004B29C;
loc_8004B160:
    s6 = s4;
    goto loc_8004B29C;
loc_8004B168:
    a0 = s2;
    LIBC2_strlen();
    s6 = v0;
    fp = 0;                                             // Result = 00000000
    goto loc_8004B29C;
loc_8004B17C:
    s3 |= 1;
loc_8004B180:
    v0 = s3 & 1;
    {
        const bool bJump = (v0 != 0);
        v0 = s3 & 4;
        if (bJump) goto loc_8004B1A0;
    }
    a1 = 0xA;                                           // Result = 0000000A
    if (v0 == 0) goto loc_8004B1A0;
    v1 = lh(s1);
    s1 += 4;
    goto loc_8004B1FC;
loc_8004B1A0:
    v1 = lw(s1);
    s1 += 4;
    a1 = 0xA;                                           // Result = 0000000A
    goto loc_8004B1FC;
loc_8004B1B0:
    a3 = 0x80010000;                                    // Result = 80010000
    a3 += 0x18B4;                                       // Result = STR_HexChars_Upper_Copy2[0] (800118B4)
    sw(a3, sp + 0x190);
loc_8004B1BC:
    v0 = s3 & 1;
    {
        const bool bJump = (v0 != 0);
        v0 = s3 & 4;
        if (bJump) goto loc_8004B1DC;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = s3 & 8;
        if (bJump) goto loc_8004B1DC;
    }
    v1 = lh(s1);
    s1 += 4;
    goto loc_8004B1E8;
loc_8004B1DC:
    v1 = lw(s1);
    s1 += 4;
    v0 = s3 & 8;
loc_8004B1E8:
    a1 = 0x10;                                          // Result = 00000010
    if (v0 == 0) goto loc_8004B1FC;
    fp = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_8004B200;
    s3 |= 0x40;
loc_8004B1FC:
    fp = 0;                                             // Result = 00000000
loc_8004B200:
    sw(s4, sp + 0x178);
    if (i32(s4) < 0) goto loc_8004B210;
    v0 = -0x21;                                         // Result = FFFFFFDF
    s3 &= v0;
loc_8004B210:
    s2 = sp + 0x16C;
    if (v1 != 0) goto loc_8004B228;
    a3 = lw(sp + 0x178);
    v0 = sp - s2;
    if (a3 == 0) goto loc_8004B298;
loc_8004B228:
    divu(v1, a1);
    a3 = lw(sp + 0x190);
    s2--;
    v0 = hi;
    v0 += a3;
    v1 = lo;
    a0 = lbu(v0);
    if (a1 != 0) goto loc_8004B250;
    _break(0x1C00);
loc_8004B250:
    sb(a0, s2);
    if (v1 != 0) goto loc_8004B228;
    a3 = 0x80010000;                                    // Result = 80010000
    a3 += 0x1898;                                       // Result = STR_HexChars_Lower_Copy2[0] (80011898)
    v0 = s3 & 8;
    sw(a3, sp + 0x190);
    if (v0 == 0) goto loc_8004B294;
    v0 = 8;                                             // Result = 00000008
    {
        const bool bJump = (a1 != v0);
        v0 = sp - s2;
        if (bJump) goto loc_8004B298;
    }
    v0 = a0 << 24;
    v0 = u32(i32(v0) >> 24);
    v1 = 0x30;                                          // Result = 00000030
    {
        const bool bJump = (v0 == v1);
        v0 = 0x30;                                      // Result = 00000030
        if (bJump) goto loc_8004B294;
    }
    s2--;
    sb(v0, s2);
loc_8004B294:
    v0 = sp - s2;
loc_8004B298:
    s6 = v0 + 0x16C;
loc_8004B29C:
    a3 = lw(sp + 0x180);
    s4 = s6 + a3;
    if (fp == 0) goto loc_8004B2B0;
    s4++;
loc_8004B2B0:
    v0 = s3 & 0x40;
    if (v0 == 0) goto loc_8004B2C0;
    s4 += 2;
loc_8004B2C0:
    s5 = lw(sp + 0x178);
    v0 = (i32(s5) < i32(s4));
    {
        const bool bJump = (v0 == 0);
        v0 = s3 & 0x30;
        if (bJump) goto loc_8004B2D8;
    }
    s5 = s4;
loc_8004B2D8:
    a0 = fp;                                            // Result = 00000000
    if (v0 != 0) goto loc_8004B314;
    a3 = lw(sp + 0x188);
    v0 = (i32(s5) < i32(a3));
    if (a3 == 0) goto loc_8004B314;
    s0 = s5;
    if (v0 == 0) goto loc_8004B314;
loc_8004B2F8:
    a0 = 0x20;                                          // Result = 00000020
    LIBC2_putchar();
    a3 = lw(sp + 0x188);
    s0++;
    v0 = (i32(s0) < i32(a3));
    a0 = fp;                                            // Result = 00000000
    if (v0 != 0) goto loc_8004B2F8;
loc_8004B314:
    v0 = s3 & 0x40;
    if (a0 == 0) goto loc_8004B328;
    LIBC2_putchar();
    v0 = s3 & 0x40;
loc_8004B328:
    v1 = s3 & 0x30;
    if (v0 == 0) goto loc_8004B348;
    a0 = 0x30;                                          // Result = 00000030
    LIBC2_putchar();
    a0 = lb(s7);
    LIBC2_putchar();
    v1 = s3 & 0x30;
loc_8004B348:
    v0 = 0x20;                                          // Result = 00000020
    if (v1 != v0) goto loc_8004B384;
    a3 = lw(sp + 0x188);
    v0 = (i32(s5) < i32(a3));
    s0 = s5;
    if (v0 == 0) goto loc_8004B384;
loc_8004B368:
    a0 = 0x30;                                          // Result = 00000030
    LIBC2_putchar();
    a3 = lw(sp + 0x188);
    s0++;
    v0 = (i32(s0) < i32(a3));
    if (v0 != 0) goto loc_8004B368;
loc_8004B384:
    a3 = lw(sp + 0x178);
    s0 = s4;
    v0 = (i32(s0) < i32(a3));
    if (v0 == 0) goto loc_8004B3B4;
loc_8004B398:
    a0 = 0x30;                                          // Result = 00000030
    LIBC2_putchar();
    a3 = lw(sp + 0x178);
    s0++;
    v0 = (i32(s0) < i32(a3));
    if (v0 != 0) goto loc_8004B398;
loc_8004B3B4:
    s0 = s6 - 1;
    if (i32(s0) < 0) goto loc_8004B3E8;
loc_8004B3C0:
    a0 = lb(s2);
    s2++;
    s0--;
    LIBC2_putchar();
    if (i32(s0) < 0) goto loc_8004B3E8;
    goto loc_8004B3C0;
loc_8004B3E0:
    a0 = 0x30;                                          // Result = 00000030
    LIBC2_putchar();
loc_8004B3E8:
    a3 = lw(sp + 0x180);
    a3--;
    sw(a3, sp + 0x180);
    if (i32(a3) >= 0) goto loc_8004B3E0;
    v0 = s3 & 0x10;
    if (v0 == 0) goto loc_8004B438;
    a3 = lw(sp + 0x188);
    v0 = (i32(s5) < i32(a3));
    s0 = s5;
    if (v0 == 0) goto loc_8004B438;
loc_8004B41C:
    a0 = 0x20;                                          // Result = 00000020
    LIBC2_putchar();
    a3 = lw(sp + 0x188);
    s0++;
    v0 = (i32(s0) < i32(a3));
    if (v0 != 0) goto loc_8004B41C;
loc_8004B438:
    v1 = lw(sp + 0x188);
    v0 = (i32(v1) < i32(s5));
    s7++;
    if (v0 == 0) goto loc_8004B450;
    v1 = s5;
loc_8004B450:
    a3 = lw(sp + 0x170);
    a3 += v1;
    sw(a3, sp + 0x170);
    goto loc_8004AEA0;
loc_8004B464:
    a3 = lw(sp + 0x170);
    a0 = lb(s7);
    a3++;
    sw(a3, sp + 0x170);
loc_8004B474:
    LIBC2_putchar();
loc_8004B47C:
    s7++;
    goto loc_8004AEA0;
loc_8004B484:
    ra = lw(sp + 0x1BC);
    fp = lw(sp + 0x1B8);
    s7 = lw(sp + 0x1B4);
    s6 = lw(sp + 0x1B0);
    s5 = lw(sp + 0x1AC);
    s4 = lw(sp + 0x1A8);
    s3 = lw(sp + 0x1A4);
    s2 = lw(sp + 0x1A0);
    s1 = lw(sp + 0x19C);
    s0 = lw(sp + 0x198);
    sp += 0x1C0;
    return;
}

void LIBC2_toupper() noexcept {
    v1 = a0;
    v0 = v1 & 0xFF;
    at = 0x80070000;                                    // Result = 80070000
    at += v0;
    v0 = lbu(at + 0x5C05);
    v0 &= 2;
    if (v0 == 0) goto loc_8004B4E8;
    v1 = a0 - 0x20;
loc_8004B4E8:
    v0 = v1 << 24;
    v0 = u32(i32(v0) >> 24);
    return;
}

void LIBC2_tolower() noexcept {
    v1 = a0;
    v0 = v1 & 0xFF;
    at = 0x80070000;                                    // Result = 80070000
    at += v0;
    v0 = lbu(at + 0x5C05);
    v0 &= 1;
    if (v0 == 0) goto loc_8004B51C;
    v1 = a0 + 0x20;
loc_8004B51C:
    v0 = v1 << 24;
    v0 = u32(i32(v0) >> 24);
    return;
}

void LIBC2_putchar() noexcept {
loc_8004B530:
    sp -= 0x20;
    sb(a0, sp + 0x10);
    a0 <<= 24;
    a0 = u32(i32(a0) >> 24);
    v0 = 9;                                             // Result = 00000009
    sw(ra, sp + 0x18);
    if (a0 == v0) goto loc_8004B570;
    v0 = 0xA;                                           // Result = 0000000A
    if (a0 != v0) goto loc_8004B598;
    a0 = 0xD;                                           // Result = 0000000D
    LIBC2_putchar();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7E38);                                 // Store to: gLIBC2_putchar_UNKNOWN_VARS[0] (80077E38)
    a0 = 1;                                             // Result = 00000001
    goto loc_8004B5CC;
loc_8004B570:
    a0 = 0x20;                                          // Result = 00000020
    LIBC2_putchar();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7E38);                               // Load from: gLIBC2_putchar_UNKNOWN_VARS[0] (80077E38)
    v0 &= 7;
    if (v0 == 0) goto loc_8004B5D8;
    goto loc_8004B570;
loc_8004B598:
    v0 = lbu(sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    at += v0;
    v0 = lb(at + 0x5C75);
    v0 &= 0x97;
    a0 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8004B5CC;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7E38);                               // Load from: gLIBC2_putchar_UNKNOWN_VARS[0] (80077E38)
    at = 0x80070000;                                    // Result = 80070000
    v0++;
    sw(v0, at + 0x7E38);                                // Store to: gLIBC2_putchar_UNKNOWN_VARS[0] (80077E38)
loc_8004B5CC:
    a1 = sp + 0x10;
    a2 = 1;                                             // Result = 00000001
    LIBAPI_write();
loc_8004B5D8:
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void LIBC2_puts() noexcept {
loc_8004BC78:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    if (s0 != 0) goto loc_8004BCA0;
    s0 = 0x80010000;                                    // Result = 80010000
    s0 += 0x1AE4;                                       // Result = STR_Sys_VSync_Timeout_Err[10] (80011AE4)
    goto loc_8004BCA0;
loc_8004BC98:
    a0 = u32(i32(a0) >> 24);
    LIBC2_putchar();
loc_8004BCA0:
    a0 = lbu(s0);
    a0 <<= 24;
    s0++;
    if (a0 != 0) goto loc_8004BC98;
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBC2_memcpy() noexcept {
loc_8004E8C0:
    v0 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_8004E8EC;
    v1 = a0;
    if (i32(a2) <= 0) goto loc_8004E8E8;
loc_8004E8D0:
    v0 = lbu(a1);
    a1++;
    a2--;
    sb(v0, a0);
    a0++;
    if (i32(a2) > 0) goto loc_8004E8D0;
loc_8004E8E8:
    v0 = v1;
loc_8004E8EC:
    return;
}

void LIBC2_memset() noexcept {
loc_8004FAA4:
    v0 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_8004FAC8;
    if (i32(a2) <= 0) goto loc_8004FAC8;
    v0 = a0;
loc_8004FAB8:
    sb(a1, a0);
    a2--;
    a0++;
    if (i32(a2) > 0) goto loc_8004FAB8;
loc_8004FAC8:
    return;
}
