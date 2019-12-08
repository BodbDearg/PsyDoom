#include "LIBSPU.h"

#include "PsxVm/PsxVm.h"

void LIBSPU_SpuSetVoiceAttr() noexcept {
loc_80050894:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = 0;                                             // Result = 00000000
    a2 = 0x17;                                          // Result = 00000017
    a3 = 0;                                             // Result = 00000000
    LIBSPU__SpuSetVoiceAttr();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU__SpuSetVoiceAttr() noexcept {
loc_800508BC:
    sp -= 0x50;
    sw(s0, sp + 0x28);
    s0 = a0;
    sw(s7, sp + 0x44);
    s7 = a2;
    sw(ra, sp + 0x4C);
    sw(fp, sp + 0x48);
    sw(s6, sp + 0x40);
    sw(s5, sp + 0x3C);
    sw(s4, sp + 0x38);
    sw(s3, sp + 0x34);
    sw(s2, sp + 0x30);
    sw(s1, sp + 0x2C);
    s1 = lw(s0 + 0x4);
    fp = a3;
    s2 = (s1 < 1);
    if (i32(a1) >= 0) goto loc_80050904;
    a1 = 0;                                             // Result = 00000000
loc_80050904:
    v0 = (i32(a1) < 0x18);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s7) < 0x18);
        if (bJump) goto loc_8005092C;
    }
    if (v0 != 0) goto loc_8005091C;
    s7 = 0x17;                                          // Result = 00000017
loc_8005091C:
    v0 = (i32(s7) < i32(a1));
    if (i32(s7) < 0) goto loc_8005092C;
    s7++;
    if (v0 == 0) goto loc_80050934;
loc_8005092C:
    v0 = -3;                                            // Result = FFFFFFFD
    goto loc_80050F88;
loc_80050934:
    s4 = a1;
    v0 = (i32(s4) < i32(s7));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80050F24;
    }
loc_80050944:
    v1 = lw(s0);
    v0 = v0 << s4;
    v0 &= v1;
    if (v0 == 0) goto loc_80050F14;
    s3 = s4 << 3;
    if (s2 != 0) goto loc_8005096C;
    v0 = s1 & 0x10;
    if (v0 == 0) goto loc_80050984;
loc_8005096C:
    v0 = s4 << 4;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    a0 = lhu(s0 + 0x14);
    v0 += v1;
    sh(a0, v0 + 0x4);
loc_80050984:
    v0 = s1 & 0x40;
    if (s2 != 0) goto loc_80050994;
    if (v0 == 0) goto loc_800509AC;
loc_80050994:
    v1 = lhu(s0 + 0x18);
    v0 = s4 << 1;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7CBC;                                       // Result = LIBSPU__spu_voice_centerNote[0] (80097CBC)
    at += v0;
    sh(v1, at);
loc_800509AC:
    v0 = s4 << 1;
    if (s2 != 0) goto loc_800509C0;
    v0 = s1 & 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = s4 << 1;
        if (bJump) goto loc_800509FC;
    }
loc_800509C0:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7CBC;                                       // Result = LIBSPU__spu_voice_centerNote[0] (80097CBC)
    at += v0;
    a1 = lhu(at);
    a3 = lhu(s0 + 0x16);
    a0 = a1 >> 8;
    a1 &= 0xFF;
    a2 = a3 >> 8;
    a3 &= 0xFF;
    LIBSPU__spu_note2pitch();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    v1 = s3 << 1;
    v1 += a0;
    sh(v0, v1 + 0x4);
loc_800509FC:
    v0 = s1 & 0x80;
    if (s2 != 0) goto loc_80050A0C;
    if (v0 == 0) goto loc_80050A64;
loc_80050A0C:
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x70DC);                               // Load from: gLIBSPU__spu_mem_mode (800A8F24)
    a0 = lw(s0 + 0x1C);
    if (a1 == 0) goto loc_80050A44;
    divu(a0, a1);
    if (a1 != 0) goto loc_80050A30;
    _break(0x1C00);
loc_80050A30:
    v0 = hi;
    if (v0 == 0) goto loc_80050A44;
    a0 += a1;
loc_80050A44:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    a0 = a0 >> v0;
    v0 = s3 << 1;
    v0 += v1;
    sh(a0, v0 + 0x6);
loc_80050A64:
    v0 = s1 & 0x800;
    if (s2 != 0) goto loc_80050A7C;
    {
        const bool bJump = (v0 == 0);
        v0 = s1 & 0x100;
        if (bJump) goto loc_80050AE4;
    }
    a1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80050AA0;
loc_80050A7C:
    v1 = lw(s0 + 0x24);
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 == v0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_80050A9C;
    }
    a1 = 0;                                             // Result = 00000000
    if (v1 != v0) goto loc_80050AA0;
    a1 = 0x80;                                          // Result = 00000080
    goto loc_80050AA0;
loc_80050A9C:
    a1 = 0;                                             // Result = 00000000
loc_80050AA0:
    v0 = lhu(s0 + 0x30);
    v0 = (v0 < 0x80);
    a2 = 0x7F;                                          // Result = 0000007F
    if (v0 == 0) goto loc_80050AB8;
    a2 = lhu(s0 + 0x30);
loc_80050AB8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    v1 = s3 << 1;
    v1 += v0;
    v0 = lhu(v1 + 0x8);
    a0 = v0 & 0xFF;
    v0 = a2 | a1;
    v0 <<= 8;
    v0 |= a0;
    sh(v0, v1 + 0x8);
loc_80050AE4:
    v0 = s1 & 0x1000;
    if (s2 != 0) goto loc_80050AF4;
    if (v0 == 0) goto loc_80050B34;
loc_80050AF4:
    v0 = lhu(s0 + 0x32);
    v0 = (v0 < 0x10);
    a2 = 0xF;                                           // Result = 0000000F
    if (v0 == 0) goto loc_80050B0C;
    a2 = lhu(s0 + 0x32);
loc_80050B0C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    v1 = s3 << 1;
    v1 += v0;
    v0 = lhu(v1 + 0x8);
    a0 = v0 & 0xFF0F;
    v0 = a2 << 4;
    v0 |= a0;
    sh(v0, v1 + 0x8);
loc_80050B34:
    v0 = s1 & 0x8000;
    if (s2 != 0) goto loc_80050B44;
    if (v0 == 0) goto loc_80050B80;
loc_80050B44:
    v0 = lhu(s0 + 0x38);
    v0 = (v0 < 0x10);
    a2 = 0xF;                                           // Result = 0000000F
    if (v0 == 0) goto loc_80050B5C;
    a2 = lhu(s0 + 0x38);
loc_80050B5C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    v1 = s3 << 1;
    v1 += v0;
    v0 = lhu(v1 + 0x8);
    v0 &= 0xFFF0;
    v0 |= a2;
    sh(v0, v1 + 0x8);
loc_80050B80:
    v0 = s1 & 0x2000;
    if (s2 != 0) goto loc_80050B98;
    {
        const bool bJump = (v0 == 0);
        v0 = s1 & 0x200;
        if (bJump) goto loc_80050C2C;
    }
    a1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80050BE8;
loc_80050B98:
    v1 = lw(s0 + 0x28);
    v0 = 3;                                             // Result = 00000003
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 4);
        if (bJump) goto loc_80050BE4;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80050BC0;
    }
    a1 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_80050BE8;
    a1 = 0x100;                                         // Result = 00000100
    goto loc_80050BE8;
loc_80050BC0:
    v0 = 5;                                             // Result = 00000005
    {
        const bool bJump = (v1 == v0);
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_80050BDC;
    }
    a1 = 0x300;                                         // Result = 00000300
    if (v1 == v0) goto loc_80050BE8;
    a1 = 0x100;                                         // Result = 00000100
    goto loc_80050BE8;
loc_80050BDC:
    a1 = 0x200;                                         // Result = 00000200
    goto loc_80050BE8;
loc_80050BE4:
    a1 = 0x100;                                         // Result = 00000100
loc_80050BE8:
    v0 = lhu(s0 + 0x34);
    v0 = (v0 < 0x80);
    a2 = 0x7F;                                          // Result = 0000007F
    if (v0 == 0) goto loc_80050C00;
    a2 = lhu(s0 + 0x34);
loc_80050C00:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    v1 = s3 << 1;
    v1 += v0;
    v0 = lhu(v1 + 0xA);
    a0 = v0 & 0x3F;
    v0 = a2 | a1;
    v0 <<= 6;
    v0 |= a0;
    sh(v0, v1 + 0xA);
loc_80050C2C:
    v0 = s1 & 0x4000;
    if (s2 != 0) goto loc_80050C44;
    {
        const bool bJump = (v0 == 0);
        v0 = s1 & 0x400;
        if (bJump) goto loc_80050CA8;
    }
    a1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80050C68;
loc_80050C44:
    v1 = lw(s0 + 0x2C);
    v0 = 3;                                             // Result = 00000003
    {
        const bool bJump = (v1 == v0);
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_80050C64;
    }
    a1 = 0;                                             // Result = 00000000
    if (v1 != v0) goto loc_80050C68;
    a1 = 0x20;                                          // Result = 00000020
    goto loc_80050C68;
loc_80050C64:
    a1 = 0;                                             // Result = 00000000
loc_80050C68:
    v0 = lhu(s0 + 0x36);
    v0 = (v0 < 0x20);
    a2 = 0x1F;                                          // Result = 0000001F
    if (v0 == 0) goto loc_80050C80;
    a2 = lhu(s0 + 0x36);
loc_80050C80:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    v1 = s3 << 1;
    v1 += v0;
    v0 = lhu(v1 + 0xA);
    a0 = v0 & 0xFFC0;
    v0 = a2 | a1;
    v0 |= a0;
    sh(v0, v1 + 0xA);
loc_80050CA8:
    v0 = s3 << 1;
    if (s2 != 0) goto loc_80050CC0;
    v0 = 0x20000;                                       // Result = 00020000
    v0 &= s1;
    {
        const bool bJump = (v0 == 0);
        v0 = s3 << 1;
        if (bJump) goto loc_80050CD4;
    }
loc_80050CC0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    a0 = lhu(s0 + 0x3A);
    v0 += v1;
    sh(a0, v0 + 0x8);
loc_80050CD4:
    v0 = s3 << 1;
    if (s2 != 0) goto loc_80050CEC;
    v0 = 0x40000;                                       // Result = 00040000
    v0 &= s1;
    {
        const bool bJump = (v0 == 0);
        v0 = s3 << 1;
        if (bJump) goto loc_80050D00;
    }
loc_80050CEC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    a0 = lhu(s0 + 0x3C);
    v0 += v1;
    sh(a0, v0 + 0xA);
loc_80050D00:
    v0 = 0x10000;                                       // Result = 00010000
    if (s2 != 0) goto loc_80050D14;
    v0 &= s1;
    if (v0 == 0) goto loc_80050D6C;
loc_80050D14:
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x70DC);                               // Load from: gLIBSPU__spu_mem_mode (800A8F24)
    a0 = lw(s0 + 0x20);
    if (a1 == 0) goto loc_80050D4C;
    divu(a0, a1);
    if (a1 != 0) goto loc_80050D38;
    _break(0x1C00);
loc_80050D38:
    v0 = hi;
    if (v0 == 0) goto loc_80050D4C;
    a0 += a1;
loc_80050D4C:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    a0 = a0 >> v0;
    v0 = s3 << 1;
    v0 += v1;
    sh(a0, v0 + 0xE);
loc_80050D6C:
    v0 = s1 & 1;
    if (s2 != 0) goto loc_80050D84;
    {
        const bool bJump = (v0 == 0);
        v0 = s1 & 4;
        if (bJump) goto loc_80050E40;
    }
    if (v0 == 0) goto loc_80050DEC;
loc_80050D84:
    v1 = lh(s0 + 0xC);
    v0 = (v1 < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80050DEC;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x1E78;                                       // Result = JumpTable_LIBSPU_SpuSetCommonAttr_1[0] (80011E78)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80050DEC: goto loc_80050DEC;
        case 0x80050DB4: goto loc_80050DB4;
        case 0x80050DBC: goto loc_80050DBC;
        case 0x80050DC4: goto loc_80050DC4;
        case 0x80050DCC: goto loc_80050DCC;
        case 0x80050DD4: goto loc_80050DD4;
        case 0x80050DDC: goto loc_80050DDC;
        case 0x80050DE4: goto loc_80050DE4;
        default: jump_table_err(); break;
    }
loc_80050DB4:
    a1 = 0x8000;                                        // Result = 00008000
    goto loc_80050DF4;
loc_80050DBC:
    a1 = 0x9000;                                        // Result = 00009000
    goto loc_80050DF4;
loc_80050DC4:
    a1 = 0xA000;                                        // Result = 0000A000
    goto loc_80050DF4;
loc_80050DCC:
    a1 = 0xB000;                                        // Result = 0000B000
    goto loc_80050DF4;
loc_80050DD4:
    a1 = 0xC000;                                        // Result = 0000C000
    goto loc_80050DF4;
loc_80050DDC:
    a1 = 0xD000;                                        // Result = 0000D000
    goto loc_80050DF4;
loc_80050DE4:
    a1 = 0xE000;                                        // Result = 0000E000
    goto loc_80050DF4;
loc_80050DEC:
    s5 = lhu(s0 + 0x8);
    a1 = 0;                                             // Result = 00000000
loc_80050DF4:
    v0 = s5 & 0x7FFF;
    if (a1 == 0) goto loc_80050E28;
    a0 = lh(s0 + 0x8);
    v0 = (i32(a0) < 0x80);
    v1 = a0;
    if (v0 != 0) goto loc_80050E18;
    s5 = 0x7F;                                          // Result = 0000007F
    goto loc_80050E24;
loc_80050E18:
    s5 = v1;
    if (i32(a0) >= 0) goto loc_80050E24;
    s5 = 0;                                             // Result = 00000000
loc_80050E24:
    v0 = s5 & 0x7FFF;
loc_80050E28:
    v1 = s3 << 1;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    v0 |= a1;
    v1 += a0;
    sh(v0, v1);
loc_80050E40:
    v0 = s1 & 2;
    if (s2 != 0) goto loc_80050E58;
    {
        const bool bJump = (v0 == 0);
        v0 = s1 & 8;
        if (bJump) goto loc_80050F14;
    }
    if (v0 == 0) goto loc_80050EC0;
loc_80050E58:
    v1 = lh(s0 + 0xE);
    v0 = (v1 < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80050EC0;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x1E98;                                       // Result = JumpTable_LIBSPU_SpuSetCommonAttr_2[0] (80011E98)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80050EC0: goto loc_80050EC0;
        case 0x80050E88: goto loc_80050E88;
        case 0x80050E90: goto loc_80050E90;
        case 0x80050E98: goto loc_80050E98;
        case 0x80050EA0: goto loc_80050EA0;
        case 0x80050EA8: goto loc_80050EA8;
        case 0x80050EB0: goto loc_80050EB0;
        case 0x80050EB8: goto loc_80050EB8;
        default: jump_table_err(); break;
    }
loc_80050E88:
    a1 = 0x8000;                                        // Result = 00008000
    goto loc_80050EC8;
loc_80050E90:
    a1 = 0x9000;                                        // Result = 00009000
    goto loc_80050EC8;
loc_80050E98:
    a1 = 0xA000;                                        // Result = 0000A000
    goto loc_80050EC8;
loc_80050EA0:
    a1 = 0xB000;                                        // Result = 0000B000
    goto loc_80050EC8;
loc_80050EA8:
    a1 = 0xC000;                                        // Result = 0000C000
    goto loc_80050EC8;
loc_80050EB0:
    a1 = 0xD000;                                        // Result = 0000D000
    goto loc_80050EC8;
loc_80050EB8:
    a1 = 0xE000;                                        // Result = 0000E000
    goto loc_80050EC8;
loc_80050EC0:
    s6 = lhu(s0 + 0xA);
    a1 = 0;                                             // Result = 00000000
loc_80050EC8:
    v0 = s6 & 0x7FFF;
    if (a1 == 0) goto loc_80050EFC;
    a0 = lh(s0 + 0xA);
    v0 = (i32(a0) < 0x80);
    v1 = a0;
    if (v0 != 0) goto loc_80050EEC;
    s6 = 0x7F;                                          // Result = 0000007F
    goto loc_80050EF8;
loc_80050EEC:
    s6 = v1;
    if (i32(a0) >= 0) goto loc_80050EF8;
    s6 = 0;                                             // Result = 00000000
loc_80050EF8:
    v0 = s6 & 0x7FFF;
loc_80050EFC:
    v1 = s3 << 1;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x687C);                               // Load from: gLIBSPU__spu_RXX (8007687C)
    v0 |= a1;
    v1 += a0;
    sh(v0, v1 + 0x2);
loc_80050F14:
    s4++;
    v0 = (i32(s4) < i32(s7));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80050944;
    }
loc_80050F24:
    v0 = 0;                                             // Result = 00000000
    if (fp != 0) goto loc_80050F88;
    v0 = 1;                                             // Result = 00000001
    sw(v0, sp + 0x14);
    sw(0, sp + 0x10);
    goto loc_80050F70;
loc_80050F40:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
loc_80050F70:
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 2);
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80050F40;
    }
loc_80050F88:
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

void LIBSPU__spu_note2pitch() noexcept {
loc_80050FBC:
    a3 &= 0xFFFF;
    a1 &= 0xFFFF;
    v0 = a3 + a1;
    t0 = a0;
    if (i32(v0) >= 0) goto loc_80050FD4;
    v0 += 7;
loc_80050FD4:
    v0 = u32(i32(v0) >> 3);
    a1 = v0;
    v0 = (i32(v0) < 0x10);
    v1 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80051004;
loc_80050FE8:
    v0 = a1 - 0x10;
    a1 = v0;
    v0 <<= 16;
    v0 = u32(i32(v0) >> 16);
    v0 = (i32(v0) < 0x10);
    v1++;
    if (v0 == 0) goto loc_80050FE8;
loc_80051004:
    v0 = a1 << 16;
    a0 = 0x2AAA0000;                                    // Result = 2AAA0000
    if (i32(v0) >= 0) goto loc_80051014;
    a1 = 0;                                             // Result = 00000000
loc_80051014:
    a0 |= 0xAAAB;                                       // Result = 2AAAAAAB
    v0 = a2 + 0x3C;
    v0 -= t0;
    v0 += v1;
    v0 <<= 16;
    v1 = u32(i32(v0) >> 16);
    mult(v1, a0);
    v0 = u32(i32(v0) >> 31);
    a0 = hi;
    a0 = u32(i32(a0) >> 1);
    a0 -= v0;
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 2;
    v1 -= v0;
    v1 <<= 16;
    v1 = u32(i32(v1) >> 12);
    v0 = a1 << 16;
    v0 = u32(i32(v0) >> 16);
    v1 += v0;
    v1 <<= 1;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x6880;                                       // Result = 80076880
    at += v1;
    v1 = lhu(at);
    a0 -= 5;
    a0 <<= 16;
    v0 = u32(i32(a0) >> 16);
    if (i32(v0) <= 0) goto loc_80051094;
    v1 = v1 << v0;
    goto loc_800510A8;
loc_80051094:
    if (i32(v0) >= 0) goto loc_800510A8;
    v1 &= 0xFFFF;
    v0 = -v0;
    v1 = i32(v1) >> v0;
loc_800510A8:
    v0 = v1 & 0xFFFF;
    return;
}

void LIBSPU__spu_pitch2note() noexcept {
    t0 = a0;
    t1 = a1;
    a3 = 0;                                             // Result = 00000000
    v1 = 0;                                             // Result = 00000000
    a1 = a2 & 0xFFFF;
loc_800510C4:
    v0 = i32(a1) >> v1;
    v0 &= 1;
    if (v0 == 0) goto loc_800510D8;
    a3 = v1;
loc_800510D8:
    v1++;
    v0 = (i32(v1) < 0x10);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800510C4;
    }
    a1 = a3 + 1;                                        // Result = 00000001
    a1 = v0 << a1;                                      // Result = 00000002
    v0 = v0 << a3;                                      // Result = 00000001
    a1 -= v0;                                           // Result = 00000001
    a0 = a2 & 0xFFFF;
    a0 -= v0;
    a2 = a0 << 1;
    a2 += a0;
    a2 <<= 2;
    div(a2, a1);
    if (a1 != 0) goto loc_8005111C;
    _break(0x1C00);
loc_8005111C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80051134;
    }
    if (a2 != at) goto loc_80051134;
    tge(zero, zero, 0x5D);
loc_80051134:
    a2 = lo;
    mult(a1, a2);
    v1 = lo;
    v0 = 0x2AAA0000;                                    // Result = 2AAA0000
    v0 |= 0xAAAB;                                       // Result = 2AAAAAAB
    mult(v1, v0);
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 = u32(i32(v0) >> 1);
    v0 -= v1;
    a0 -= v0;
    v1 = a0 << 1;
    v1 += a0;
    v1 <<= 9;
    div(v1, a1);
    if (a1 != 0) goto loc_80051180;
    _break(0x1C00);
loc_80051180:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80051198;
    }
    if (v1 != at) goto loc_80051198;
    tge(zero, zero, 0x5D);
loc_80051198:
    v1 = lo;
    a1 = t0 & 0xFFFF;
    a0 = a3 - 0xC;                                      // Result = FFFFFFF4
    v0 = a0 << 1;                                       // Result = FFFFFFE8
    v0 += a0;                                           // Result = FFFFFFDC
    v0 <<= 2;                                           // Result = FFFFFF70
    a1 += v0;
    a1 += a2;
    v0 = t1 & 0xFFFF;
    v1 += v0;
    v0 = v1;
    if (i32(v1) >= 0) goto loc_800511CC;
    v0 = v1 + 0x7F;
loc_800511CC:
    a0 = u32(i32(v0) >> 7);
    v0 = a0 << 7;
    a0 = v1 - v0;
    v0 = (i32(v1) < 0x80);
    v0 ^= 1;
    a1 += v0;
    v0 = (i32(a1) < 0x80);
    if (i32(a1) < 0) goto loc_800511FC;
    {
        const bool bJump = (v0 == 0);
        v0 = a1 << 8;
        if (bJump) goto loc_800511FC;
    }
    v0 |= a0;
    goto loc_80051200;
loc_800511FC:
    v0 = -1;                                            // Result = FFFFFFFF
loc_80051200:
    return;
}

void LIBSPU_SpuSetReverbModeParam() noexcept {
loc_80051208:
    sp -= 0x90;
    sw(s2, sp + 0x70);
    s2 = a0;
    sw(fp, sp + 0x88);
    fp = 0;                                             // Result = 00000000
    sw(s4, sp + 0x78);
    s4 = 0;                                             // Result = 00000000
    sw(s6, sp + 0x80);
    s6 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x8C);
    sw(s7, sp + 0x84);
    sw(s5, sp + 0x7C);
    sw(s3, sp + 0x74);
    sw(s1, sp + 0x6C);
    sw(s0, sp + 0x68);
    s3 = lw(s2);
    s7 = 0;                                             // Result = 00000000
    s5 = (s3 < 1);
    sw(0, sp + 0x10);
    if (s5 != 0) goto loc_80051264;
    v0 = s3 & 1;
    if (v0 == 0) goto loc_80051384;
loc_80051264:
    s0 = lw(s2 + 0x4);
    v0 = s0 & 0x100;
    {
        const bool bJump = (v0 == 0);
        v0 = -0x101;                                    // Result = FFFFFEFF
        if (bJump) goto loc_80051280;
    }
    s0 &= v0;
    fp = 1;                                             // Result = 00000001
loc_80051280:
    v0 = (s0 < 0xA);
    {
        const bool bJump = (v0 == 0);
        v0 = s0 << 2;
        if (bJump) goto loc_800512B4;
    }
    at = 0x80070000;                                    // Result = 80070000
    at += 0x6E78;                                       // Result = 80076E78
    at += v0;
    a0 = lw(at);
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x6E78;                                       // Result = 80076E78
    LIBSPU__SpuIsInAllocateArea_();
    s4 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_800512BC;
loc_800512B4:
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_800516E8;
loc_800512BC:
    a2 = sp + 0x10;
    a1 = 0x43;                                          // Result = 00000043
    at = 0x800B0000;                                    // Result = 800B0000
    sw(s0, at - 0x6E70);                                // Store to: gLIBSPU__spu_rev_attr[2] (800A9190)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6E70);                               // Load from: gLIBSPU__spu_rev_attr[2] (800A9190)
    a3 = -1;                                            // Result = FFFFFFFF
    a0 = v1 << 2;
    a0 += s1;
    v0 = v1 << 4;
    v0 += v1;
    v0 <<= 2;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x6EC8;                                       // Result = 80076EC8
    a0 = lw(a0);
    v1 += v0;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x6A54);                                // Store to: gLIBSPU__spu_rev_offsetaddr (80076A54)
loc_80051304:
    v0 = lbu(v1);
    v1++;
    a1--;
    sb(v0, a2);
    a2++;
    if (a1 != a3) goto loc_80051304;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6E70);                               // Load from: gLIBSPU__spu_rev_attr[2] (800A9190)
    v0 = 7;                                             // Result = 00000007
    {
        const bool bJump = (v1 == v0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_80051340;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 0xFF;                                      // Result = 000000FF
        if (bJump) goto loc_8005135C;
    }
    goto loc_80051374;
loc_80051340:
    v0 = 0xFF;                                          // Result = 000000FF
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x6E64);                                // Store to: gLIBSPU__spu_rev_attr[8] (800A919C)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x6E68);                                // Store to: gLIBSPU__spu_rev_attr[6] (800A9198)
    goto loc_80051384;
loc_8005135C:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(0, at - 0x6E64);                                 // Store to: gLIBSPU__spu_rev_attr[8] (800A919C)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x6E68);                                // Store to: gLIBSPU__spu_rev_attr[6] (800A9198)
    goto loc_80051384;
loc_80051374:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(0, at - 0x6E64);                                 // Store to: gLIBSPU__spu_rev_attr[8] (800A919C)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(0, at - 0x6E68);                                 // Store to: gLIBSPU__spu_rev_attr[6] (800A9198)
loc_80051384:
    v0 = s3 & 8;
    if (s5 != 0) goto loc_80051394;
    if (v0 == 0) goto loc_800514AC;
loc_80051394:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6E70);                               // Load from: gLIBSPU__spu_rev_attr[2] (800A9190)
    v0 = (i32(v1) < 9);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 7);
        if (bJump) goto loc_800514A4;
    }
    if (v0 != 0) goto loc_800514A4;
    s6 = 1;                                             // Result = 00000001
    if (s4 != 0) goto loc_8005140C;
    a1 = sp + 0x10;
    a0 = 0x43;                                          // Result = 00000043
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E70);                               // Load from: gLIBSPU__spu_rev_attr[2] (800A9190)
    a2 = -1;                                            // Result = FFFFFFFF
    v1 = v0 << 4;
    v1 += v0;
    v1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x6EC8;                                       // Result = 80076EC8
    v1 += v0;
loc_800513E8:
    v0 = lbu(v1);
    v1++;
    a0--;
    sb(v0, a1);
    a1++;
    if (a0 != a2) goto loc_800513E8;
    v0 = 0xC010000;                                     // Result = 0C010000
    v0 |= 0x1C00;                                       // Result = 0C011C00
    sw(v0, sp + 0x10);
loc_8005140C:
    a2 = 0x81020000;                                    // Result = 81020000
    a0 = lw(s2 + 0xC);
    a2 |= 0x409;                                        // Result = 81020409
    v1 = a0 << 13;
    mult(v1, a2);
    v0 = hi;
    a1 = a0 << 12;
    mult(a1, a2);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a0, at - 0x6E68);                                // Store to: gLIBSPU__spu_rev_attr[6] (800A9198)
    a0 = lhu(sp + 0x14);
    v0 += v1;
    v0 = u32(i32(v0) >> 6);
    v1 = u32(i32(v1) >> 31);
    v0 -= v1;
    v0 -= a0;
    sh(v0, sp + 0x28);
    v0 = lhu(sp + 0x16);
    a0 = lhu(sp + 0x36);
    v1 = hi;
    v1 += a1;
    v1 = u32(i32(v1) >> 6);
    a1 = u32(i32(a1) >> 31);
    v1 -= a1;
    v0 = v1 - v0;
    sh(v0, sp + 0x2A);
    v0 = lhu(sp + 0x2E);
    a0 += v1;
    sh(a0, sp + 0x34);
    v0 += v1;
    sh(v0, sp + 0x2C);
    v0 = lhu(sp + 0x4C);
    a0 = lhu(sp + 0x4E);
    v0 += v1;
    v1 += a0;
    sh(v0, sp + 0x48);
    sh(v1, sp + 0x4A);
    goto loc_800514AC;
loc_800514A4:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(0, at - 0x6E68);                                 // Store to: gLIBSPU__spu_rev_attr[6] (800A9198)
loc_800514AC:
    v0 = s3 & 0x10;
    if (s5 != 0) goto loc_800514BC;
    if (v0 == 0) goto loc_8005158C;
loc_800514BC:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6E70);                               // Load from: gLIBSPU__spu_rev_attr[2] (800A9190)
    v0 = (i32(v1) < 9);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 7);
        if (bJump) goto loc_80051584;
    }
    if (v0 != 0) goto loc_80051584;
    s7 = 1;                                             // Result = 00000001
    if (s4 != 0) goto loc_80051544;
    a1 = sp + 0x10;
    if (s6 != 0) goto loc_80051534;
    a0 = 0x43;                                          // Result = 00000043
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E70);                               // Load from: gLIBSPU__spu_rev_attr[2] (800A9190)
    a2 = -1;                                            // Result = FFFFFFFF
    v1 = v0 << 4;
    v1 += v0;
    v1 <<= 2;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x6EC8;                                       // Result = 80076EC8
    v1 += v0;
loc_80051514:
    v0 = lbu(v1);
    v1++;
    a0--;
    sb(v0, a1);
    a1++;
    if (a0 != a2) goto loc_80051514;
    v0 = 0x80;                                          // Result = 00000080
    goto loc_80051540;
loc_80051534:
    v0 = lw(sp + 0x10);
    v0 |= 0x80;
loc_80051540:
    sw(v0, sp + 0x10);
loc_80051544:
    a0 = 0x81020000;                                    // Result = 81020000
    v0 = lw(s2 + 0x10);
    a0 |= 0x409;                                        // Result = 81020409
    v1 = v0 << 7;
    v1 += v0;
    v1 <<= 8;
    mult(v1, a0);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x6E64);                                // Store to: gLIBSPU__spu_rev_attr[8] (800A919C)
    v0 = hi;
    v0 += v1;
    v0 = u32(i32(v0) >> 6);
    v1 = u32(i32(v1) >> 31);
    v0 -= v1;
    sh(v0, sp + 0x22);
    goto loc_8005158C;
loc_80051584:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(0, at - 0x6E64);                                 // Store to: gLIBSPU__spu_rev_attr[8] (800A919C)
loc_8005158C:
    a1 = 0x6A;                                          // Result = 0000006A
    if (s4 == 0) goto loc_800515D0;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x58;
    LIBSPU__spu_ioctl();
    v0 = lw(sp + 0x58);
    a1 = 0x60;                                          // Result = 00000060
    if (v0 == 0) goto loc_80051640;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x5C;
    sw(0, sp + 0x5C);
    LIBSPU__spu_ioctl();
    goto loc_80051640;
loc_800515D0:
    v0 = s3 & 2;
    if (s5 != 0) goto loc_800515E0;
    if (v0 == 0) goto loc_80051604;
loc_800515E0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A04);                               // Load from: 80076A04
    v1 = lhu(s2 + 0x8);
    sh(v1, v0 + 0x184);
    v0 = lhu(s2 + 0x8);
    at = 0x800B0000;                                    // Result = 800B0000
    sh(v0, at - 0x6E6C);                                // Store to: gLIBSPU__spu_rev_attr[4] (800A9194)
loc_80051604:
    v0 = s3 & 4;
    if (s5 != 0) goto loc_80051614;
    if (v0 == 0) goto loc_80051664;
loc_80051614:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A04);                               // Load from: 80076A04
    v1 = lhu(s2 + 0xA);
    sh(v1, v0 + 0x186);
    v0 = lhu(s2 + 0xA);
    at = 0x800B0000;                                    // Result = 800B0000
    sh(v0, at - 0x6E6A);                                // Store to: gLIBSPU__spu_rev_attr[5] (800A9196)
    goto loc_80051664;
loc_80051640:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A04);                               // Load from: 80076A04
    sh(0, v0 + 0x184);
    sh(0, v0 + 0x186);
    at = 0x800B0000;                                    // Result = 800B0000
    sh(0, at - 0x6E6C);                                 // Store to: gLIBSPU__spu_rev_attr[4] (800A9194)
    at = 0x800B0000;                                    // Result = 800B0000
    sh(0, at - 0x6E6A);                                 // Store to: gLIBSPU__spu_rev_attr[5] (800A9196)
loc_80051664:
    a1 = 0x63;                                          // Result = 00000063
    if (s4 != 0) goto loc_8005167C;
    if (s6 != 0) goto loc_8005167C;
    if (s7 == 0) goto loc_8005168C;
loc_8005167C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x10;
    LIBSPU__spu_ioctl();
loc_8005168C:
    if (fp == 0) goto loc_800516A4;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x6E70);                               // Load from: gLIBSPU__spu_rev_attr[2] (800A9190)
    LIBSPU_SpuClearReverbWorkArea();
loc_800516A4:
    v0 = 0;                                             // Result = 00000000
    if (s4 == 0) goto loc_800516E8;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x6A54;                                       // Result = gLIBSPU__spu_rev_offsetaddr (80076A54)
    a1 = 0x61;                                          // Result = 00000061
    LIBSPU__spu_ioctl();
    v0 = lw(sp + 0x58);
    a1 = 0x60;                                          // Result = 00000060
    if (v0 == 0) goto loc_800516E4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x58;
    LIBSPU__spu_ioctl();
loc_800516E4:
    v0 = 0;                                             // Result = 00000000
loc_800516E8:
    ra = lw(sp + 0x8C);
    fp = lw(sp + 0x88);
    s7 = lw(sp + 0x84);
    s6 = lw(sp + 0x80);
    s5 = lw(sp + 0x7C);
    s4 = lw(sp + 0x78);
    s3 = lw(sp + 0x74);
    s2 = lw(sp + 0x70);
    s1 = lw(sp + 0x6C);
    s0 = lw(sp + 0x68);
    sp += 0x90;
    return;
}

void LIBSPU__SpuIsInAllocateArea() noexcept {
    t0 = 0x80000000;                                    // Result = 80000000
    a3 = 0x40000000;                                    // Result = 40000000
    a2 = 0xFFF0000;                                     // Result = 0FFF0000
    a2 |= 0xFFFF;                                       // Result = 0FFFFFFF
    a1 = 0x80090000;                                    // Result = 80090000
    a1 = lw(a1 + 0x7784);                               // Load from: gLIBSPU__spu_memList (80097784)
loc_80051734:
    v1 = lw(a1);
    v0 = v1 & t0;
    {
        const bool bJump = (v0 != 0);
        v0 = v1 & a3;
        if (bJump) goto loc_80051778;
    }
    v1 &= a2;
    if (v0 != 0) goto loc_80051780;
    v0 = (v1 < a0);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80051784;
    }
    v0 = lw(a1 + 0x4);
    v0 += v1;
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80051784;
    }
loc_80051778:
    a1 += 8;
    goto loc_80051734;
loc_80051780:
    v0 = 0;                                             // Result = 00000000
loc_80051784:
    return;
}

void LIBSPU__SpuIsInAllocateArea_() noexcept {
loc_8005178C:
    t0 = 0x80000000;                                    // Result = 80000000
    a3 = 0x40000000;                                    // Result = 40000000
    a2 = 0xFFF0000;                                     // Result = 0FFF0000
    a2 |= 0xFFFF;                                       // Result = 0FFFFFFF
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    a1 = 0x80090000;                                    // Result = 80090000
    a1 = lw(a1 + 0x7784);                               // Load from: gLIBSPU__spu_memList (80097784)
    a0 = a0 << v0;
loc_800517B0:
    v1 = lw(a1);
    v0 = v1 & t0;
    {
        const bool bJump = (v0 != 0);
        v0 = v1 & a3;
        if (bJump) goto loc_800517F0;
    }
    v1 &= a2;
    if (v0 != 0) goto loc_800517F8;
    v0 = (v1 < a0);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800517FC;
    }
    v0 = lw(a1 + 0x4);
    v0 += v1;
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800517FC;
    }
loc_800517F0:
    a1 += 8;
    goto loc_800517B0;
loc_800517F8:
    v0 = 0;                                             // Result = 00000000
loc_800517FC:
    return;
}

void LIBSPU__spu_reset() noexcept {
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v1 + 0x1AA);
    sp -= 8;
    v0 = a0 & 0x7FCF;
    sh(v0, v1 + 0x1AA);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x4);
    sw(0, sp);
    goto loc_8005185C;
loc_80051834:
    v1 = lw(sp + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x4);
    v0 = lw(sp);
    v0++;
    sw(v0, sp);
    v0 = lw(sp);
loc_8005185C:
    v0 = lw(sp);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051834;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 &= 0xFFCF;
    sh(a0, v0 + 0x1AA);
    v0 = 0;                                             // Result = 00000000
    sp += 8;
    return;
}

void LIBSPU__spu_init() noexcept {
loc_80051894:
    sp -= 0x48;
    sw(s1, sp + 0x3C);
    s1 = a0;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A18);                               // Load from: 80076A18
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x40);
    sw(s0, sp + 0x38);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60EC);                                // Store to: gLIBSPU__spu_transMode (800860EC)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60F0);                                 // Store to: gLIBSPU__spu_addrMode (800860F0)
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at + 0x60E8);                                 // Store to: gLIBSPU__spu_tsa[0] (800860E8)
    v0 = lw(a0);
    v1 = 0xB0000;                                       // Result = 000B0000
    v0 |= v1;
    sw(v0, a0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    sh(0, v0 + 0x1AA);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_8005194C;
loc_8005190C:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_8005190C;
loc_8005194C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60E4);                                 // Store to: 800860E4
    v0 &= 0x7FF;
    {
        const bool bJump = (v0 == 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_800519D4;
    }
loc_80051970:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60E4);                               // Load from: 800860E4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60E4);                                // Store to: 800860E4
    v0 = (i32(v0) < 0x1389);
    if (v0 != 0) goto loc_800519B4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1EB8;                                       // Result = STR_Sys_SPU_TO_Msg[0] (80011EB8)
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1EC8;                                       // Result = STR_Sys_wait_reset_Msg[0] (80011EC8)
    LIBC2_printf();
    v0 = 2;                                             // Result = 00000002
    goto loc_800519D4;
loc_800519B4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    v0 &= 0x7FF;
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80051970;
    }
loc_800519D4:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x70DC);                                // Store to: gLIBSPU__spu_mem_mode (800A8F24)
    v0 = 3;                                             // Result = 00000003
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x6E60);                                // Store to: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v0 = 4;                                             // Result = 00000004
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    s0 = 0xFFFF;                                        // Result = 0000FFFF
    sh(v0, v1 + 0x1AC);
    sh(0, v1 + 0x180);
    sh(0, v1 + 0x182);
    sh(0, v1 + 0x184);
    sh(0, v1 + 0x186);
    sh(s0, v1 + 0x18C);
    sh(s0, v1 + 0x18E);
    sh(0, v1 + 0x198);
    sh(0, v1 + 0x19A);
    v0 = 0;                                             // Result = 00000000
    if (s1 != 0) goto loc_80051DF4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x6A24;                                       // Result = 80076A24
    a1 = 0x10;                                          // Result = 00000010
    v0 = 0x200;                                         // Result = 00000200
    sh(0, v1 + 0x190);
    sh(0, v1 + 0x192);
    sh(0, v1 + 0x194);
    sh(0, v1 + 0x196);
    sh(0, v1 + 0x1B0);
    sh(0, v1 + 0x1B2);
    sh(0, v1 + 0x1B4);
    sh(0, v1 + 0x1B6);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x60E8);                                // Store to: gLIBSPU__spu_tsa[0] (800860E8)
    LIBSPU__spu_writeByIO();
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, sp + 0x10);
    v0 = 0x1F;                                          // Result = 0000001F
    sw(v0, sp + 0x14);
    v0 = 0x3FFF;                                        // Result = 00003FFF
    sh(v0, sp + 0x20);
    v0 = 0x200;                                         // Result = 00000200
    sh(0, sp + 0x18);
    sh(0, sp + 0x1A);
    sw(v0, sp + 0x24);
    sw(0, sp + 0x2C);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60F0);                                 // Store to: gLIBSPU__spu_addrMode (800860F0)
    a0 = sp + 0x10;
    LIBSPU__spu_setVoiceAttr();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x188);
    sh(s0, v1 + 0x188);
    v0 = lhu(v1 + 0x18A);
    v0 |= 0xFF;
    sh(v0, v1 + 0x18A);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051B24;
loc_80051AE4:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051AE4;
loc_80051B24:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051B84;
loc_80051B44:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051B44;
loc_80051B84:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051BE4;
loc_80051BA4:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051BA4;
loc_80051BE4:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051C44;
loc_80051C04:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051C04;
loc_80051C44:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x18C);
    v0 = 0xFFFF;                                        // Result = 0000FFFF
    sh(v0, v1 + 0x18C);
    v0 = lhu(v1 + 0x18E);
    v0 |= 0xFF;
    sh(v0, v1 + 0x18E);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051CD0;
loc_80051C90:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051C90;
loc_80051CD0:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051D30;
loc_80051CF0:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051CF0;
loc_80051D30:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051D90;
loc_80051D50:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051D50;
loc_80051D90:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x34);
    sw(0, sp + 0x30);
    v0 = lw(sp + 0x30);
    v0 = (i32(v0) < 0xF0);
    a0 = sp + 0x30;
    if (v0 == 0) goto loc_80051DF0;
loc_80051DB0:
    v1 = lw(a0 + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, a0 + 0x4);
    v0 = lw(a0);
    v0++;
    sw(v0, a0);
    v0 = lw(a0);
    v0 = lw(a0);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051DB0;
loc_80051DF0:
    v0 = 0;                                             // Result = 00000000
loc_80051DF4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = 1;                                             // Result = 00000001
    at = 0x80090000;                                    // Result = 80090000
    sw(v1, at + 0x655C);                                // Store to: gLIBSPU__spu_inTransfer (8009655C)
    v1 = 0xC000;                                        // Result = 0000C000
    sh(v1, a0 + 0x1AA);
    at = 0x80090000;                                    // Result = 80090000
    sw(0, at + 0x7C20);                                 // Store to: gLIBSPU__spu_transferCallback (80097C20)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x652C);                                 // Store to: 8008652C
    ra = lw(sp + 0x40);
    s1 = lw(sp + 0x3C);
    s0 = lw(sp + 0x38);
    sp += 0x48;
    return;
}

void LIBSPU__spu_close() noexcept {
loc_80051E38:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    at = 0x80090000;                                    // Result = 80090000
    sw(0, at + 0x7C20);                                 // Store to: gLIBSPU__spu_transferCallback (80097C20)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x652C);                                 // Store to: 8008652C
    a0 = 0;                                             // Result = 00000000
    LIBSPU__SpuDataCallback();
    v0 = 1;                                             // Result = 00000001
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU__spu_open() noexcept {
loc_80051E6C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0x80050000;                                    // Result = 80050000
    a0 += 0x21DC;                                       // Result = LIBSPU__spu_FiDMA (800521DC)
    LIBSPU__SpuDataCallback();
    v0 = 3;                                             // Result = 00000003
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU__spu_writeByIO() noexcept {
loc_80051E98:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 + 0x60E8);                              // Load from: gLIBSPU__spu_tsa[0] (800860E8)
    sp -= 0x38;
    sw(s1, sp + 0x24);
    s1 = a1;
    sw(ra, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s0, sp + 0x20);
    a1 = lhu(v1 + 0x1AE);
    s2 = a0;
    sh(v0, v1 + 0x1A6);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x14);
    sw(0, sp + 0x10);
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    s4 = a1 & 0x7FF;
    if (v0 == 0) goto loc_80051F34;
loc_80051EF4:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051EF4;
loc_80051F34:
    v0 = (s1 < 0x41);
    if (s1 == 0) goto loc_8005211C;
    s3 = 0xD;                                           // Result = 0000000D
loc_80051F40:
    s0 = s1;
    if (v0 != 0) goto loc_80051F4C;
    s0 = 0x40;                                          // Result = 00000040
loc_80051F4C:
    v1 = 0;                                             // Result = 00000000
    if (i32(s0) <= 0) goto loc_80051F78;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
loc_80051F5C:
    v0 = lhu(s2);
    s2 += 2;
    v1 += 2;
    sh(v0, a0 + 0x1A8);
    v0 = (i32(v1) < i32(s0));
    if (v0 != 0) goto loc_80051F5C;
loc_80051F78:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v1 + 0x1AA);
    v0 = a0 & 0xFFCF;
    a0 = v0 | 0x10;
    sh(a0, v1 + 0x1AA);
    sw(s3, sp + 0x14);
    sw(0, sp + 0x10);
    goto loc_80051FD0;
loc_80051FA8:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
loc_80051FD0:
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80051FA8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60E4);                                 // Store to: 800860E4
    v0 &= 0x400;
    if (v0 == 0) goto loc_80052070;
loc_8005200C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60E4);                               // Load from: 800860E4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60E4);                                // Store to: 800860E4
    v0 = (i32(v0) < 0x1389);
    if (v0 != 0) goto loc_80052050;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1EB8;                                       // Result = STR_Sys_SPU_TO_Msg[0] (80011EB8)
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1ED8;                                       // Result = STR_Sys_wait_wrdy_H_Msg[0] (80011ED8)
    LIBC2_printf();
    goto loc_80052070;
loc_80052050:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    v0 &= 0x400;
    if (v0 != 0) goto loc_8005200C;
loc_80052070:
    sw(s3, sp + 0x14);
    sw(0, sp + 0x10);
    goto loc_800520A8;
loc_80052080:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
loc_800520A8:
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80052080;
    sw(s3, sp + 0x14);
    sw(0, sp + 0x10);
    goto loc_800520F8;
loc_800520D0:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
loc_800520F8:
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_800520D0;
    s1 -= s0;
    v0 = (s1 < 0x41);
    if (s1 != 0) goto loc_80051F40;
loc_8005211C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v0 + 0x1AA);
    a0 &= 0xFFCF;
    sh(a0, v0 + 0x1AA);
    v0 = lhu(v0 + 0x1AE);
    a1 = s4 & 0xFFFF;
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60E4);                                 // Store to: 800860E4
    v0 &= 0x7FF;
    if (v0 == a1) goto loc_800521B8;
loc_80052154:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60E4);                               // Load from: 800860E4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60E4);                                // Store to: 800860E4
    v0 = (i32(v0) < 0x1389);
    if (v0 != 0) goto loc_80052198;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1EB8;                                       // Result = STR_Sys_SPU_TO_Msg[0] (80011EB8)
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1EEC;                                       // Result = STR_Sys_wait_dmaf_clear_Msg[0] (80011EEC)
    LIBC2_printf();
    goto loc_800521B8;
loc_80052198:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    v0 &= 0x7FF;
    if (v0 != a1) goto loc_80052154;
loc_800521B8:
    ra = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x38;
    return;
}

void LIBSPU__spu_FiDMA() noexcept {
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60F4);                               // Load from: 800860F4
    sp -= 0x20;
    sw(ra, sp + 0x18);
    if (v0 != 0) goto loc_80052294;
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x14);
    sw(0, sp + 0x10);
    goto loc_8005222C;
loc_80052204:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
loc_8005222C:
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    {
        const bool bJump = (v0 != 0);
        v0 = 0xD;                                       // Result = 0000000D
        if (bJump) goto loc_80052204;
    }
    sw(v0, sp + 0x14);
    sw(0, sp + 0x10);
    goto loc_8005227C;
loc_80052254:
    v1 = lw(sp + 0x14);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x14);
    v0 = lw(sp + 0x10);
    v0++;
    sw(v0, sp + 0x10);
    v0 = lw(sp + 0x10);
loc_8005227C:
    v0 = lw(sp + 0x10);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80052254;
loc_80052294:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(v0 + 0x1AA);
    v1 &= 0xFFCF;
    sh(v1, v0 + 0x1AA);
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7C20);                               // Load from: gLIBSPU__spu_transferCallback (80097C20)
    a0 = 0xF0000000;                                    // Result = F0000000
    if (v0 == 0) goto loc_800522E0;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7C20);                               // Load from: gLIBSPU__spu_transferCallback (80097C20)
    pcall(v0);
    goto loc_800522EC;
loc_800522E0:
    a0 |= 9;                                            // Result = F0000009
    a1 = 0x20;                                          // Result = 00000020
    LIBAPI_DeliverEvent();
loc_800522EC:
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void LIBSPU__spu_r() noexcept {
loc_800522FC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    sp -= 8;
    sh(a1, v0 + 0x1A6);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x4);
    sw(0, sp);
    v0 = lw(sp);
    v0 = (i32(v0) < 0xF0);
    a3 = a0;
    if (v0 == 0) goto loc_8005236C;
loc_8005232C:
    v1 = lw(sp + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x4);
    v0 = lw(sp);
    v0++;
    sw(v0, sp);
    v0 = lw(sp);
    v0 = lw(sp);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_8005232C;
loc_8005236C:
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x4);
    sw(0, sp);
    goto loc_800523A8;
loc_80052380:
    v1 = lw(sp + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x4);
    v0 = lw(sp);
    v0++;
    sw(v0, sp);
    v0 = lw(sp);
loc_800523A8:
    v0 = lw(sp);
    v0 = (i32(v0) < 0xF0);
    if (v0 != 0) goto loc_80052380;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 |= 0x30;
    sh(v0, v1 + 0x1AA);
    v0 = 0xD;                                           // Result = 0000000D
    sw(v0, sp + 0x4);
    sw(0, sp);
    goto loc_80052418;
loc_800523F0:
    v1 = lw(sp + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x4);
    v0 = lw(sp);
    v0++;
    sw(v0, sp);
    v0 = lw(sp);
loc_80052418:
    v0 = lw(sp);
    v0 = (i32(v0) < 0xF0);
    {
        const bool bJump = (v0 != 0);
        v0 = 0xD;                                       // Result = 0000000D
        if (bJump) goto loc_800523F0;
    }
    sw(v0, sp + 0x4);
    sw(0, sp);
    goto loc_80052468;
loc_80052440:
    v1 = lw(sp + 0x4);
    v0 = v1 << 1;
    v0 += v1;
    sw(v0, sp + 0x4);
    v0 = lw(sp);
    v0++;
    sw(v0, sp);
    v0 = lw(sp);
loc_80052468:
    v0 = lw(sp);
    v0 = (i32(v0) < 0xF0);
    v1 = 0xF0FF0000;                                    // Result = F0FF0000
    if (v0 != 0) goto loc_80052440;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A20);                               // Load from: 80076A20
    v0 = lw(a0);
    v1 |= 0xFFFF;                                       // Result = F0FFFFFF
    v0 &= v1;
    v1 = 0x22000000;                                    // Result = 22000000
    v0 |= v1;
    sw(v0, a0);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A1C);                               // Load from: 80076A1C
    v0 = lw(a0);
    v1 = 0x900000;                                      // Result = 00900000
    v0 |= v1;
    sw(v0, a0);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A18);                               // Load from: 80076A18
    a1 = 0x1000000;                                     // Result = 01000000
    v0 = lw(a0);
    v1 = 0x80000;                                       // Result = 00080000
    v0 |= v1;
    sw(v0, a0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A0C);                               // Load from: 80076A0C
    a1 |= 0x200;                                        // Result = 01000200
    sw(a3, v0);
    v0 = a2 << 16;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A10);                               // Load from: 80076A10
    v0 |= 0x10;
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A14);                               // Load from: 80076A14
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60F4);                                // Store to: 800860F4
    sw(a1, v1);
    sp += 8;
    return;
}

void LIBSPU__spu_t() noexcept {
loc_80052524:
    sw(a2, sp + 0x8);
    a2 = sp + 4;
    sw(a3, sp + 0xC);
    a3 = 1;                                             // Result = 00000001
    sw(a0, sp);
    sw(a1, sp + 0x4);
    sw(a0, sp);
    if (a0 == a3) goto loc_800525A8;
    v0 = (i32(a0) < 2);
    if (v0 == 0) goto loc_80052560;
    v0 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_80052618;
    goto loc_80052804;
loc_80052560:
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (a0 == v0);
        v0 = 3;                                         // Result = 00000003
        if (bJump) goto loc_8005257C;
    }
    {
        const bool bJump = (a0 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80052684;
    }
    goto loc_80052804;
loc_8005257C:
    a0 = lw(sp + 0x4);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = a0 >> v0;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x60E8);                                // Store to: gLIBSPU__spu_tsa[0] (800860E8)
    sh(v0, v1 + 0x1A6);
    v0 = 0;                                             // Result = 00000000
    goto loc_80052804;
loc_800525A8:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lhu(a0 + 0x60E8);                              // Load from: gLIBSPU__spu_tsa[0] (800860E8)
    v0 = lhu(a1 + 0x1A6);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60F4);                                 // Store to: 800860F4
    v0 &= 0xFFFF;
    v1 = 0;                                             // Result = 00000000
    if (v0 == a0) goto loc_800525F0;
    v1++;                                               // Result = 00000001
loc_800525D4:
    v0 = (i32(v1) < 0xF01);
    {
        const bool bJump = (v0 == 0);
        v0 = -2;                                        // Result = FFFFFFFE
        if (bJump) goto loc_80052804;
    }
    v0 = lhu(a1 + 0x1A6);
    v1++;
    if (v0 != a0) goto loc_800525D4;
loc_800525F0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v1 + 0x1AA);
    v0 = a0 & 0xFFCF;
    a0 = v0 | 0x20;
    sh(a0, v1 + 0x1AA);
    v0 = 0;                                             // Result = 00000000
    goto loc_80052804;
loc_80052618:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lhu(a0 + 0x60E8);                              // Load from: gLIBSPU__spu_tsa[0] (800860E8)
    v0 = lhu(a1 + 0x1A6);
    at = 0x80080000;                                    // Result = 80080000
    sw(a3, at + 0x60F4);                                // Store to: 800860F4
    v0 &= 0xFFFF;
    v1 = 0;                                             // Result = 00000000
    if (v0 == a0) goto loc_80052660;
    v1++;                                               // Result = 00000001
loc_80052644:
    v0 = (i32(v1) < 0xF01);
    {
        const bool bJump = (v0 == 0);
        v0 = -2;                                        // Result = FFFFFFFE
        if (bJump) goto loc_80052804;
    }
    v0 = lhu(a1 + 0x1A6);
    v1++;
    if (v0 != a0) goto loc_80052644;
loc_80052660:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v0 + 0x1AA);
    a0 |= 0x30;
    sh(a0, v0 + 0x1AA);
    v0 = 0;                                             // Result = 00000000
    goto loc_80052804;
loc_80052684:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60F4);                               // Load from: 800860F4
    a0 = 0x20;                                          // Result = 00000020
    if (v0 != a3) goto loc_8005269C;
    a0 = 0x30;                                          // Result = 00000030
loc_8005269C:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = 0;                                             // Result = 00000000
    v0 = lhu(a1 + 0x1AA);
    a0 &= 0xFFFF;
    v0 &= 0x30;
    v1++;                                               // Result = 00000001
    if (v0 == a0) goto loc_800526DC;
loc_800526BC:
    v0 = (i32(v1) < 0xF01);
    {
        const bool bJump = (v0 == 0);
        v0 = -2;                                        // Result = FFFFFFFE
        if (bJump) goto loc_80052804;
    }
    v0 = lhu(a1 + 0x1AA);
    v0 &= 0x30;
    v1++;
    if (v0 != a0) goto loc_800526BC;
loc_800526DC:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x60F4);                               // Load from: 800860F4
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v1 = 0xF0FF0000;                                // Result = F0FF0000
        if (bJump) goto loc_80052710;
    }
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A20);                               // Load from: 80076A20
    v0 = lw(a0);
    v1 |= 0xFFFF;                                       // Result = F0FFFFFF
    v0 &= v1;
    v1 = 0x22000000;                                    // Result = 22000000
    goto loc_8005272C;
loc_80052710:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A20);                               // Load from: 80076A20
    v0 = lw(a0);
    v1 |= 0xFFFF;                                       // Result = F0FFFFFF
    v0 &= v1;
    v1 = 0x20000000;                                    // Result = 20000000
loc_8005272C:
    v0 |= v1;
    sw(v0, a0);
    a2 += 4;
    a0 = lw(a2 - 0x4);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at + 0x60F8);                                // Store to: 800860F8
    a0 = lw(a2);
    v1 = a0 >> 6;
    v0 = a0 & 0x3F;
    v0 = (v0 > 0);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A1C);                               // Load from: 80076A1C
    v1 += v0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at + 0x60FC);                                // Store to: 800860FC
    v0 = lw(a0);
    v1 = 0x900000;                                      // Result = 00900000
    v0 |= v1;
    sw(v0, a0);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A18);                               // Load from: 80076A18
    v0 = lw(a0);
    v1 = 0x80000;                                       // Result = 00080000
    v0 |= v1;
    sw(v0, a0);
    a0 = 0x1000000;                                     // Result = 01000000
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A0C);                               // Load from: 80076A0C
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60F8);                               // Load from: 800860F8
    a0 |= 0x201;                                        // Result = 01000201
    sw(v0, v1);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60FC);                               // Load from: 800860FC
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A10);                               // Load from: 80076A10
    v0 <<= 16;
    v0 |= 0x10;
    sw(v0, v1);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x60F4);                               // Load from: 800860F4
    v0 = 1;                                             // Result = 00000001
    if (v1 != v0) goto loc_800527F0;
    a0 = 0x1000000;                                     // Result = 01000000
    a0 |= 0x200;                                        // Result = 01000200
loc_800527F0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A14);                               // Load from: 80076A14
    sw(a0, v0);
    v0 = 0;                                             // Result = 00000000
loc_80052804:
    return;
}

void LIBSPU__spu_write() noexcept {
loc_8005280C:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s0, sp + 0x10);
    s0 = a1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x60EC);                               // Load from: gLIBSPU__spu_transMode (800860EC)
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x18);
    if (v1 != v0) goto loc_80052870;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 + 0x60E8);                              // Load from: gLIBSPU__spu_tsa[0] (800860E8)
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    a0 = 2;                                             // Result = 00000002
    a1 = v0 << a1;
    LIBSPU__spu_t();
    a0 = 1;                                             // Result = 00000001
    LIBSPU__spu_t();
    a0 = 3;                                             // Result = 00000003
    a1 = s1;
    a2 = s0;
    LIBSPU__spu_t();
    v0 = s0;
    goto loc_80052880;
loc_80052870:
    a0 = s1;
    a1 = s0;
    LIBSPU__spu_writeByIO();
    v0 = s0;
loc_80052880:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void LIBSPU__spu_read() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s0, sp + 0x10);
    s0 = a1;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 + 0x60E8);                              // Load from: gLIBSPU__spu_tsa[0] (800860E8)
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    a0 = 2;                                             // Result = 00000002
    sw(ra, sp + 0x18);
    a1 = v0 << a1;
    LIBSPU__spu_t();
    a0 = 0;                                             // Result = 00000000
    LIBSPU__spu_t();
    a0 = 3;                                             // Result = 00000003
    a1 = s1;
    a2 = s0;
    LIBSPU__spu_t();
    v0 = s0;
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void LIBSPU__spu_ioctl() noexcept {
loc_80052900:
    sp -= 0x18;
    v0 = 0x52;                                          // Result = 00000052
    sw(ra, sp + 0x10);
    if (a1 == v0) goto loc_80052ECC;
    v0 = (a1 < 0x53);
    {
        const bool bJump = (v0 == 0);
        v0 = 0x2B;                                      // Result = 0000002B
        if (bJump) goto loc_80052A10;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x2C);
        if (bJump) goto loc_80052D40;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x13;                                      // Result = 00000013
        if (bJump) goto loc_8005299C;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x14);
        if (bJump) goto loc_80052CA8;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80052970;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 3);
        if (bJump) goto loc_80052B68;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8005295C;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80052B18;
    }
    goto loc_800531DC;
loc_8005295C:
    v0 = 0x10;                                          // Result = 00000010
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80052B90;
    }
    goto loc_800531DC;
loc_80052970:
    v0 = 0x15;                                          // Result = 00000015
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x15);
        if (bJump) goto loc_80052D30;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x16;                                      // Result = 00000016
        if (bJump) goto loc_80052D18;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0x21;                                      // Result = 00000021
        if (bJump) goto loc_80052C80;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80052DD0;
    }
    goto loc_800531DC;
loc_8005299C:
    v0 = 0x40;                                          // Result = 00000040
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x41);
        if (bJump) goto loc_80052DF8;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x2E;                                      // Result = 0000002E
        if (bJump) goto loc_800529E4;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x2F);
        if (bJump) goto loc_80052DD0;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x2D;                                      // Result = 0000002D
        if (bJump) goto loc_800529D0;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80052DB0;
    }
    goto loc_800531DC;
loc_800529D0:
    v0 = 0x2F;                                          // Result = 0000002F
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80052DE8;
    }
    goto loc_800531DC;
loc_800529E4:
    v0 = 0x42;                                          // Result = 00000042
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x42);
        if (bJump) goto loc_80052E50;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x50;                                      // Result = 00000050
        if (bJump) goto loc_80052E34;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0x51;                                      // Result = 00000051
        if (bJump) goto loc_80052E88;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80052EA8;
    }
    goto loc_800531DC;
loc_80052A10:
    v0 = 0x68;                                          // Result = 00000068
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x69);
        if (bJump) goto loc_80052FC8;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x61;                                      // Result = 00000061
        if (bJump) goto loc_80052A94;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x62);
        if (bJump) goto loc_80052FAC;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x54;                                      // Result = 00000054
        if (bJump) goto loc_80052A54;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x54);
        if (bJump) goto loc_80052F20;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x60;                                      // Result = 00000060
        if (bJump) goto loc_80052F04;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80052F58;
    }
    goto loc_800531DC;
loc_80052A54:
    v0 = 0x65;                                          // Result = 00000065
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x66);
        if (bJump) goto loc_80053070;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x63;                                      // Result = 00000063
        if (bJump) goto loc_80052A78;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80053060;
    }
    goto loc_800531DC;
loc_80052A78:
    v0 = 0x66;                                          // Result = 00000066
    {
        const bool bJump = (a1 == v0);
        v0 = 0x67;                                      // Result = 00000067
        if (bJump) goto loc_800530AC;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800530D4;
    }
    goto loc_800531DC;
loc_80052A94:
    v0 = 0x203;                                         // Result = 00000203
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x204);
        if (bJump) goto loc_80053154;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x6A;                                      // Result = 0000006A
        if (bJump) goto loc_80052AD0;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = (a1 < 0x6A);
        if (bJump) goto loc_80052F84;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x100;                                     // Result = 00000100
        if (bJump) goto loc_80053038;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0x101;                                     // Result = 00000101
        if (bJump) goto loc_8005310C;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8005311C;
    }
    goto loc_800531DC;
loc_80052AD0:
    v0 = 0x210;                                         // Result = 00000210
    a0 = a2;
    if (a1 == v0) goto loc_8005317C;
    v0 = (a1 < 0x211);
    {
        const bool bJump = (v0 == 0);
        v0 = 0x204;                                     // Result = 00000204
        if (bJump) goto loc_80052AF8;
    }
    {
        const bool bJump = (a1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8005312C;
    }
    goto loc_800531DC;
loc_80052AF8:
    v0 = 0x212;                                         // Result = 00000212
    a0 = a2;
    if (a1 == v0) goto loc_80053188;
    v0 = 0x213;                                         // Result = 00000213
    a0 = a2 + 0x800;
    if (a1 == v0) goto loc_80053190;
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052B18:
    if (a2 != 0) goto loc_80052B44;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 |= 0x4000;
    sh(v0, v1 + 0x1AA);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052B44:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 &= 0xBFFF;
    sh(v0, v1 + 0x1AA);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052B68:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AA);
    v0 &= 0x4000;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80052FA4;
    }
loc_80052B88:
    sw(0, a2);
    goto loc_800531D8;
loc_80052B90:
    if (a2 != 0) goto loc_80052C0C;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v1 + 0x1AA);
    a0 &= 0xFFBF;
    sh(a0, v1 + 0x1AA);
    v0 = lhu(v1 + 0x1AA);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60E4);                                 // Store to: 800860E4
    v0 &= 0x40;
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800531DC;
    }
loc_80052BCC:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60E4);                               // Load from: 800860E4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60E4);                                // Store to: 800860E4
    v0 = (i32(v0) < 0x1389);
    if (v0 == 0) goto loc_800531C0;
    v0 = lhu(v1 + 0x1AA);
    v0 &= 0x40;
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80052BCC;
    }
    goto loc_800531DC;
loc_80052C0C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v1 + 0x1AA);
    a0 |= 0x40;
    sh(a0, v1 + 0x1AA);
    v0 = lhu(v1 + 0x1AA);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x60E4);                                 // Store to: 800860E4
    v0 &= 0x40;
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800531DC;
    }
loc_80052C40:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60E4);                               // Load from: 800860E4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60E4);                                // Store to: 800860E4
    v0 = (i32(v0) < 0x1389);
    if (v0 == 0) goto loc_800531C0;
    v0 = lhu(v1 + 0x1AA);
    v0 &= 0x40;
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80052C40;
    }
    goto loc_800531DC;
loc_80052C80:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AA);
    v0 &= 0x40;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80052FA4;
    }
    sw(0, a2);
    goto loc_800531D8;
loc_80052CA8:
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x70DC);                               // Load from: gLIBSPU__spu_mem_mode (800A8F24)
    a0 = lw(a2);
    if (a1 == 0) goto loc_80052CF8;
    div(a0, a1);
    if (a1 != 0) goto loc_80052CCC;
    _break(0x1C00);
loc_80052CCC:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80052CE4;
    }
    if (a0 != at) goto loc_80052CE4;
    tge(zero, zero, 0x5D);
loc_80052CE4:
    v0 = hi;
    if (v0 == 0) goto loc_80052CF8;
    a0 += a1;
loc_80052CF8:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = i32(a0) >> v0;
    sh(a0, v1 + 0x1A4);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052D18:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v0 + 0x1A4);
    goto loc_80053048;
loc_80052D30:
    a0 = a2;
    LIBSPU__SpuCallback();
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052D40:
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x70DC);                               // Load from: gLIBSPU__spu_mem_mode (800A8F24)
    a0 = lw(a2);
    if (a1 == 0) goto loc_80052D90;
    div(a0, a1);
    if (a1 != 0) goto loc_80052D64;
    _break(0x1C00);
loc_80052D64:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80052D7C;
    }
    if (a0 != at) goto loc_80052D7C;
    tge(zero, zero, 0x5D);
loc_80052D7C:
    v0 = hi;
    if (v0 == 0) goto loc_80052D90;
    a0 += a1;
loc_80052D90:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    a0 = i32(a0) >> v0;
    at = 0x80080000;                                    // Result = 80080000
    sh(a0, at + 0x60E8);                                // Store to: gLIBSPU__spu_tsa[0] (800860E8)
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052DB0:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 + 0x60E8);                              // Load from: gLIBSPU__spu_tsa[0] (800860E8)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v0 = v0 << v1;
    sw(v0, a2);
    goto loc_800531D8;
loc_80052DD0:
    v0 = lw(a2);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x60EC);                                // Store to: gLIBSPU__spu_transMode (800860EC)
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052DE8:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60EC);                               // Load from: gLIBSPU__spu_transMode (800860EC)
    sw(v0, a2);
    goto loc_800531D8;
loc_80052DF8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(a2);
    a0 = lhu(v0 + 0x190);
    v1 |= a0;
    a0 = v1;
    sh(a0, v0 + 0x190);
    a0 = lhu(v0 + 0x192);
    v1 = lbu(a2 + 0x2);
    a0 |= v1;
    sh(a0, v0 + 0x192);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052E34:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(v0 + 0x192);
    v0 = lhu(v0 + 0x190);
    v1 &= 0xFF;
    goto loc_800530C4;
loc_80052E50:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(a2);
    a0 = lhu(v0 + 0x190);
    v1 = ~v1;
    a0 &= v1;
    sh(a0, v0 + 0x190);
    v1 = lbu(a2 + 0x2);
    a0 = lhu(v0 + 0x192);
    v1 = ~v1;
    a0 &= v1;
    sh(a0, v0 + 0x192);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052E88:
    v1 = lhu(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 &= 0x3F;
    a0 = lhu(v0 + 0x1AA);
    v1 <<= 8;
    a0 &= 0xC0FF;
    goto loc_80052F74;
loc_80052EA8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AA);
    v0 &= 0x3F00;
    v0 >>= 8;
    sh(v0, a2);
    goto loc_800531D8;
loc_80052ECC:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(a2);
    v0 = lhu(a0 + 0x194);
    v0 |= v1;
    sh(v0, a0 + 0x194);
    v1 = lbu(a2 + 0x2);
    v0 = lhu(a0 + 0x196);
    v0 |= v1;
    sh(v0, a0 + 0x196);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052F04:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(v0 + 0x196);
    v0 = lhu(v0 + 0x194);
    v1 &= 0xFF;
    goto loc_800530C4;
loc_80052F20:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a2);
    v1 = lhu(a0 + 0x194);
    v0 = ~v0;
    v1 &= v0;
    sh(v1, a0 + 0x194);
    v0 = lbu(a2 + 0x2);
    v1 = lhu(a0 + 0x196);
    v0 = ~v0;
    v1 &= v0;
    sh(v1, a0 + 0x196);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052F58:
    v1 = lhu(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 &= 1;
    a0 = lhu(v0 + 0x1AA);
    v1 <<= 7;
    a0 &= 0xFF7F;
loc_80052F74:
    a0 |= v1;
    sh(a0, v0 + 0x1AA);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052F84:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v0 + 0x1AA);
    v0 = a0 & 0x80;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80052B88;
    }
loc_80052FA4:
    sw(v0, a2);
    goto loc_800531D8;
loc_80052FAC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a2);
    sh(v0, v1 + 0x1A2);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80052FC8:
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x70DC);                               // Load from: gLIBSPU__spu_mem_mode (800A8F24)
    a0 = lw(a2);
    if (a1 == 0) goto loc_80053018;
    div(a0, a1);
    if (a1 != 0) goto loc_80052FEC;
    _break(0x1C00);
loc_80052FEC:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80053004;
    }
    if (a0 != at) goto loc_80053004;
    tge(zero, zero, 0x5D);
loc_80053004:
    v0 = hi;
    if (v0 == 0) goto loc_80053018;
    a0 += a1;
loc_80053018:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = i32(a0) >> v0;
    sh(a0, v1 + 0x1A2);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80053038:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    a0 = lhu(v0 + 0x1A2);
loc_80053048:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    a0 = a0 << v0;
    sw(a0, a2);
    goto loc_800531D8;
loc_80053060:
    a0 = a2;
    LIBSPU__spu_setReverbAttr();
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80053070:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(a2);
    a0 = lhu(v0 + 0x198);
    v1 |= a0;
    a0 = v1;
    sh(a0, v0 + 0x198);
    a0 = lhu(v0 + 0x19A);
    v1 = lbu(a2 + 0x2);
    a0 |= v1;
    sh(a0, v0 + 0x19A);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_800530AC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(v0 + 0x19A);
    v0 = lhu(v0 + 0x198);
    v1 &= 0xFF;
loc_800530C4:
    v1 <<= 16;
    v0 |= v1;
    sw(v0, a2);
    goto loc_800531D8;
loc_800530D4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(a2);
    a0 = lhu(v0 + 0x198);
    v1 = ~v1;
    a0 &= v1;
    sh(a0, v0 + 0x198);
    v1 = lbu(a2 + 0x2);
    a0 = lhu(v0 + 0x19A);
    v1 = ~v1;
    a0 &= v1;
    sh(a0, v0 + 0x19A);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_8005310C:
    a0 = a2;
    LIBSPU__spu_setCommonAttr();
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_8005311C:
    a0 = a2;
    LIBSPU__spu_getCommonAttr();
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_8005312C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(a2);
    sh(v1, v0 + 0x188);
    v1 = lbu(a2 + 0x2);
    sh(v1, v0 + 0x18A);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_80053154:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = lhu(a2);
    sh(v1, v0 + 0x18C);
    v1 = lbu(a2 + 0x2);
    sh(v1, v0 + 0x18E);
    v0 = 0;                                             // Result = 00000000
    goto loc_800531DC;
loc_8005317C:
    a1 = 0;                                             // Result = 00000000
    a2 = 0x40;                                          // Result = 00000040
    goto loc_80053198;
loc_80053188:
    a1 = 0;                                             // Result = 00000000
    goto loc_80053194;
loc_80053190:
    a1 = 0x100;                                         // Result = 00000100
loc_80053194:
    a2 = 0x20;                                          // Result = 00000020
loc_80053198:
    LIBSPU__spu_r();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v0 + 0x1AE);
    v0 &= 0x800;
    v0 = (v0 < 1);
    goto loc_800531DC;
loc_800531C0:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1EB8;                                       // Result = STR_Sys_SPU_TO_Msg[0] (80011EB8)
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1F00;                                       // Result = STR_Sys_wait_io_ctl_irq_Msg[0] (80011F00)
    LIBC2_printf();
loc_800531D8:
    v0 = 0;                                             // Result = 00000000
loc_800531DC:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU__spu_setVoiceAttr() noexcept {
loc_800531EC:
    t2 = 0;                                             // Result = 00000000
    t1 = lw(a0 + 0x4);
    t3 = 0x800B0000;                                    // Result = 800B0000
    t3 = lw(t3 - 0x70DC);                               // Load from: gLIBSPU__spu_mem_mode (800A8F24)
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    t0 = (t1 < 1);
    t5 = t1 & 0x10;
    t4 = a3;
loc_80053210:
    v0 = 1;                                             // Result = 00000001
    v1 = lw(a0);
    v0 = v0 << t2;
    v0 &= v1;
    if (v0 == 0) goto loc_80053354;
    a2 = t2 << 3;
    if (t0 != 0) goto loc_8005323C;
    v0 = t1 & 1;
    if (v0 == 0) goto loc_80053248;
loc_8005323C:
    v0 = lhu(a0 + 0x8);
    sh(v0, t4);
loc_80053248:
    v0 = a2 << 1;
    if (t0 != 0) goto loc_8005325C;
    v0 = t1 & 2;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 << 1;
        if (bJump) goto loc_80053268;
    }
loc_8005325C:
    v1 = lhu(a0 + 0xA);
    v0 += a3;
    sh(v1, v0 + 0x2);
loc_80053268:
    v0 = a2 << 1;
    if (t0 != 0) goto loc_8005327C;
    v0 = t1 & 4;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 << 1;
        if (bJump) goto loc_80053288;
    }
loc_8005327C:
    v1 = lhu(a0 + 0x10);
    v0 += a3;
    sh(v1, v0 + 0x4);
loc_80053288:
    v0 = t1 & 8;
    if (t0 != 0) goto loc_80053298;
    if (v0 == 0) goto loc_800532FC;
loc_80053298:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 + 0x60F0);                               // Load from: gLIBSPU__spu_addrMode (800860F0)
    {
        const bool bJump = (v0 != 0);
        v0 = a2 << 1;
        if (bJump) goto loc_800532B8;
    }
    v1 = lhu(a0 + 0x14);
    v0 += a3;
    goto loc_800532F8;
loc_800532B8:
    a1 = lw(a0 + 0x14);
    if (t3 == 0) goto loc_800532E8;
    divu(a1, t3);
    if (t3 != 0) goto loc_800532D4;
    _break(0x1C00);
loc_800532D4:
    v0 = hi;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 << 1;
        if (bJump) goto loc_800532E8;
    }
    a1 += t3;
loc_800532E8:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v0 += a3;
    v1 = a1 >> v1;
loc_800532F8:
    sh(v1, v0 + 0x6);
loc_800532FC:
    v0 = a2 << 1;
    if (t0 != 0) goto loc_8005330C;
    if (t5 == 0) goto loc_80053318;
loc_8005330C:
    v1 = lhu(a0 + 0x1E);
    v0 += a3;
    sh(v1, v0 + 0x8);
loc_80053318:
    v0 = a2 << 1;
    if (t0 != 0) goto loc_80053328;
    if (t5 == 0) goto loc_80053334;
loc_80053328:
    v1 = lhu(a0 + 0x1C);
    v0 += a3;
    sh(v1, v0 + 0xA);
loc_80053334:
    v0 = a2 << 1;
    if (t0 != 0) goto loc_80053348;
    v0 = t1 & 0x100;
    {
        const bool bJump = (v0 == 0);
        v0 = a2 << 1;
        if (bJump) goto loc_80053354;
    }
loc_80053348:
    v1 = lhu(a0 + 0x18);
    v0 += a3;
    sh(v1, v0 + 0xE);
loc_80053354:
    t2++;
    v0 = (i32(t2) < 0x18);
    t4 += 0x10;
    if (v0 != 0) goto loc_80053210;
    return;
}

void LIBSPU__spu_setReverbAttr() noexcept {
loc_8005336C:
    a1 = lw(a0);
    a2 = (a1 < 1);
    v0 = a1 & 1;
    if (a2 != 0) goto loc_80053388;
    if (v0 == 0) goto loc_8005339C;
loc_80053388:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x4);
    sh(v0, v1 + 0x1C0);
loc_8005339C:
    v0 = a1 & 2;
    if (a2 != 0) goto loc_800533AC;
    if (v0 == 0) goto loc_800533C0;
loc_800533AC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x6);
    sh(v0, v1 + 0x1C2);
loc_800533C0:
    v0 = a1 & 4;
    if (a2 != 0) goto loc_800533D0;
    if (v0 == 0) goto loc_800533E4;
loc_800533D0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x8);
    sh(v0, v1 + 0x1C4);
loc_800533E4:
    v0 = a1 & 8;
    if (a2 != 0) goto loc_800533F4;
    if (v0 == 0) goto loc_80053408;
loc_800533F4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0xA);
    sh(v0, v1 + 0x1C6);
loc_80053408:
    v0 = a1 & 0x10;
    if (a2 != 0) goto loc_80053418;
    if (v0 == 0) goto loc_8005342C;
loc_80053418:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0xC);
    sh(v0, v1 + 0x1C8);
loc_8005342C:
    v0 = a1 & 0x20;
    if (a2 != 0) goto loc_8005343C;
    if (v0 == 0) goto loc_80053450;
loc_8005343C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0xE);
    sh(v0, v1 + 0x1CA);
loc_80053450:
    v0 = a1 & 0x40;
    if (a2 != 0) goto loc_80053460;
    if (v0 == 0) goto loc_80053474;
loc_80053460:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x10);
    sh(v0, v1 + 0x1CC);
loc_80053474:
    v0 = a1 & 0x80;
    if (a2 != 0) goto loc_80053484;
    if (v0 == 0) goto loc_80053498;
loc_80053484:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x12);
    sh(v0, v1 + 0x1CE);
loc_80053498:
    v0 = a1 & 0x100;
    if (a2 != 0) goto loc_800534A8;
    if (v0 == 0) goto loc_800534BC;
loc_800534A8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x14);
    sh(v0, v1 + 0x1D0);
loc_800534BC:
    v0 = a1 & 0x200;
    if (a2 != 0) goto loc_800534CC;
    if (v0 == 0) goto loc_800534E0;
loc_800534CC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x16);
    sh(v0, v1 + 0x1D2);
loc_800534E0:
    v0 = a1 & 0x400;
    if (a2 != 0) goto loc_800534F0;
    if (v0 == 0) goto loc_80053504;
loc_800534F0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x18);
    sh(v0, v1 + 0x1D4);
loc_80053504:
    v0 = a1 & 0x800;
    if (a2 != 0) goto loc_80053514;
    if (v0 == 0) goto loc_80053528;
loc_80053514:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x1A);
    sh(v0, v1 + 0x1D6);
loc_80053528:
    v0 = a1 & 0x1000;
    if (a2 != 0) goto loc_80053538;
    if (v0 == 0) goto loc_8005354C;
loc_80053538:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x1C);
    sh(v0, v1 + 0x1D8);
loc_8005354C:
    v0 = a1 & 0x2000;
    if (a2 != 0) goto loc_8005355C;
    if (v0 == 0) goto loc_80053570;
loc_8005355C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x1E);
    sh(v0, v1 + 0x1DA);
loc_80053570:
    v0 = a1 & 0x4000;
    if (a2 != 0) goto loc_80053580;
    if (v0 == 0) goto loc_80053594;
loc_80053580:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x20);
    sh(v0, v1 + 0x1DC);
loc_80053594:
    v0 = a1 & 0x8000;
    if (a2 != 0) goto loc_800535A4;
    if (v0 == 0) goto loc_800535B8;
loc_800535A4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x22);
    sh(v0, v1 + 0x1DE);
loc_800535B8:
    v0 = 0x10000;                                       // Result = 00010000
    if (a2 != 0) goto loc_800535CC;
    v0 &= a1;
    if (v0 == 0) goto loc_800535E0;
loc_800535CC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x24);
    sh(v0, v1 + 0x1E0);
loc_800535E0:
    v0 = 0x20000;                                       // Result = 00020000
    if (a2 != 0) goto loc_800535F4;
    v0 &= a1;
    if (v0 == 0) goto loc_80053608;
loc_800535F4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x26);
    sh(v0, v1 + 0x1E2);
loc_80053608:
    v0 = 0x40000;                                       // Result = 00040000
    if (a2 != 0) goto loc_8005361C;
    v0 &= a1;
    if (v0 == 0) goto loc_80053630;
loc_8005361C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x28);
    sh(v0, v1 + 0x1E4);
loc_80053630:
    v0 = 0x80000;                                       // Result = 00080000
    if (a2 != 0) goto loc_80053644;
    v0 &= a1;
    if (v0 == 0) goto loc_80053658;
loc_80053644:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x2A);
    sh(v0, v1 + 0x1E6);
loc_80053658:
    v0 = 0x100000;                                      // Result = 00100000
    if (a2 != 0) goto loc_8005366C;
    v0 &= a1;
    if (v0 == 0) goto loc_80053680;
loc_8005366C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x2C);
    sh(v0, v1 + 0x1E8);
loc_80053680:
    v0 = 0x200000;                                      // Result = 00200000
    if (a2 != 0) goto loc_80053694;
    v0 &= a1;
    if (v0 == 0) goto loc_800536A8;
loc_80053694:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x2E);
    sh(v0, v1 + 0x1EA);
loc_800536A8:
    v0 = 0x400000;                                      // Result = 00400000
    if (a2 != 0) goto loc_800536BC;
    v0 &= a1;
    if (v0 == 0) goto loc_800536D0;
loc_800536BC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x30);
    sh(v0, v1 + 0x1EC);
loc_800536D0:
    v0 = 0x800000;                                      // Result = 00800000
    if (a2 != 0) goto loc_800536E4;
    v0 &= a1;
    if (v0 == 0) goto loc_800536F8;
loc_800536E4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x32);
    sh(v0, v1 + 0x1EE);
loc_800536F8:
    v0 = 0x1000000;                                     // Result = 01000000
    if (a2 != 0) goto loc_8005370C;
    v0 &= a1;
    if (v0 == 0) goto loc_80053720;
loc_8005370C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x34);
    sh(v0, v1 + 0x1F0);
loc_80053720:
    v0 = 0x2000000;                                     // Result = 02000000
    if (a2 != 0) goto loc_80053734;
    v0 &= a1;
    if (v0 == 0) goto loc_80053748;
loc_80053734:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x36);
    sh(v0, v1 + 0x1F2);
loc_80053748:
    v0 = 0x4000000;                                     // Result = 04000000
    if (a2 != 0) goto loc_8005375C;
    v0 &= a1;
    if (v0 == 0) goto loc_80053770;
loc_8005375C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x38);
    sh(v0, v1 + 0x1F4);
loc_80053770:
    v0 = 0x8000000;                                     // Result = 08000000
    if (a2 != 0) goto loc_80053784;
    v0 &= a1;
    if (v0 == 0) goto loc_80053798;
loc_80053784:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x3A);
    sh(v0, v1 + 0x1F6);
loc_80053798:
    v0 = 0x10000000;                                    // Result = 10000000
    if (a2 != 0) goto loc_800537AC;
    v0 &= a1;
    if (v0 == 0) goto loc_800537C0;
loc_800537AC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x3C);
    sh(v0, v1 + 0x1F8);
loc_800537C0:
    v0 = 0x20000000;                                    // Result = 20000000
    if (a2 != 0) goto loc_800537D4;
    v0 &= a1;
    if (v0 == 0) goto loc_800537E8;
loc_800537D4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x3E);
    sh(v0, v1 + 0x1FA);
loc_800537E8:
    v0 = 0x40000000;                                    // Result = 40000000
    if (a2 != 0) goto loc_800537FC;
    v0 &= a1;
    if (v0 == 0) goto loc_80053810;
loc_800537FC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x40);
    sh(v0, v1 + 0x1FC);
loc_80053810:
    if (a2 != 0) goto loc_80053820;
    if (i32(a1) >= 0) goto loc_80053834;
loc_80053820:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x42);
    sh(v0, v1 + 0x1FE);
loc_80053834:
    v0 = 0;                                             // Result = 00000000
    return;
}

void LIBSPU__spu_setCommonAttr() noexcept {
loc_8005383C:
    a2 = a0;
    a1 = lw(a2);
    a3 = (a1 < 1);
    v0 = a1 & 1;
    if (a3 != 0) goto loc_8005385C;
    if (v0 == 0) goto loc_80053870;
loc_8005385C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a2 + 0x4);
    sh(v0, v1 + 0x180);
loc_80053870:
    v0 = a1 & 2;
    if (a3 != 0) goto loc_80053880;
    if (v0 == 0) goto loc_80053894;
loc_80053880:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a2 + 0x6);
    sh(v0, v1 + 0x182);
loc_80053894:
    v0 = a1 & 4;
    if (a3 != 0) goto loc_800538A4;
    if (v0 == 0) goto loc_800538B8;
loc_800538A4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a2 + 0x8);
    sh(v0, v1 + 0x184);
loc_800538B8:
    v0 = a1 & 8;
    if (a3 != 0) goto loc_800538C8;
    if (v0 == 0) goto loc_800538DC;
loc_800538C8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a2 + 0xA);
    sh(v0, v1 + 0x186);
loc_800538DC:
    v0 = a1 & 0x10;
    if (a3 != 0) goto loc_800538EC;
    if (v0 == 0) goto loc_80053900;
loc_800538EC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a2 + 0xC);
    sh(v0, v1 + 0x1B0);
loc_80053900:
    v0 = a1 & 0x20;
    if (a3 != 0) goto loc_80053910;
    if (v0 == 0) goto loc_80053924;
loc_80053910:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a2 + 0xE);
    sh(v0, v1 + 0x1B2);
loc_80053924:
    v0 = a1 & 0x100;
    if (a3 != 0) goto loc_80053934;
    if (v0 == 0) goto loc_80053978;
loc_80053934:
    v0 = lw(a2 + 0x10);
    if (v0 != 0) goto loc_8005395C;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 &= 0xFFFB;
    goto loc_80053974;
loc_8005395C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 |= 4;
loc_80053974:
    sh(v0, v1 + 0x1AA);
loc_80053978:
    v0 = a1 & 0x400;
    if (a3 != 0) goto loc_80053988;
    if (v0 == 0) goto loc_800539CC;
loc_80053988:
    v0 = lw(a2 + 0x14);
    if (v0 != 0) goto loc_800539B0;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 &= 0xFFFE;
    goto loc_800539C8;
loc_800539B0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 |= 1;
loc_800539C8:
    sh(v0, v1 + 0x1AA);
loc_800539CC:
    v0 = a1 & 0x40;
    if (a3 != 0) goto loc_800539DC;
    if (v0 == 0) goto loc_800539F0;
loc_800539DC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a2 + 0x18);
    sh(v0, v1 + 0x1B4);
loc_800539F0:
    v0 = a1 & 0x80;
    if (a3 != 0) goto loc_80053A00;
    if (v0 == 0) goto loc_80053A14;
loc_80053A00:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a2 + 0x1A);
    sh(v0, v1 + 0x1B6);
loc_80053A14:
    v0 = a1 & 0x200;
    if (a3 != 0) goto loc_80053A24;
    if (v0 == 0) goto loc_80053A68;
loc_80053A24:
    v0 = lw(a2 + 0x1C);
    if (v0 != 0) goto loc_80053A4C;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 &= 0xFFF7;
    goto loc_80053A64;
loc_80053A4C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 |= 8;
loc_80053A64:
    sh(v0, v1 + 0x1AA);
loc_80053A68:
    v0 = a1 & 0x800;
    if (a3 != 0) goto loc_80053A78;
    if (v0 == 0) goto loc_80053ABC;
loc_80053A78:
    v0 = lw(a2 + 0x20);
    if (v0 != 0) goto loc_80053AA0;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 &= 0xFFFD;
    goto loc_80053AB8;
loc_80053AA0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(v1 + 0x1AA);
    v0 |= 2;
loc_80053AB8:
    sh(v0, v1 + 0x1AA);
loc_80053ABC:
    v0 = a1 & 0x1000;
    if (a3 != 0) goto loc_80053ACC;
    if (v0 == 0) goto loc_80053B00;
loc_80053ACC:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x1AA);
    v0 &= 0xC0FF;
    sh(v0, a0 + 0x1AA);
    v0 = lhu(a2 + 0x24);
    v1 = lhu(a0 + 0x1AA);
    v0 &= 0x3F;
    v0 <<= 8;
    v1 |= v0;
    sh(v1, a0 + 0x1AA);
loc_80053B00:
    v0 = a1 & 0x2000;
    if (a3 != 0) goto loc_80053B10;
    if (v0 == 0) goto loc_80053B44;
loc_80053B10:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x1AA);
    v0 &= 0xFF7F;
    sh(v0, a0 + 0x1AA);
    v0 = lhu(a2 + 0x26);
    v1 = lhu(a0 + 0x1AA);
    v0 &= 1;
    v0 <<= 7;
    v1 |= v0;
    sh(v1, a0 + 0x1AA);
loc_80053B44:
    v0 = a1 & 0x4000;
    if (a3 != 0) goto loc_80053B54;
    if (v0 == 0) goto loc_80053B88;
loc_80053B54:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x1AC);
    v0 &= 0xFFF1;
    sh(v0, a0 + 0x1AC);
    v0 = lhu(a2 + 0x28);
    v1 = lhu(a0 + 0x1AC);
    v0 &= 7;
    v0 <<= 1;
    v1 |= v0;
    sh(v1, a0 + 0x1AC);
loc_80053B88:
    if (a3 != 0) goto loc_80053B9C;
    v0 = a1 & 0x8000;
    if (v0 == 0) goto loc_80053BCC;
loc_80053B9C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a0 + 0x1AC);
    v0 &= 0xFFFE;
    sh(v0, a0 + 0x1AC);
    v0 = lhu(a2 + 0x2A);
    v1 = lhu(a0 + 0x1AC);
    v0 &= 1;
    v1 |= v0;
    sh(v1, a0 + 0x1AC);
loc_80053BCC:
    v0 = 0;                                             // Result = 00000000
    return;
}

void LIBSPU__spu_getCommonAttr() noexcept {
loc_80053BD4:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 = lhu(a1 + 0x180);
    sh(v0, a0 + 0x4);
    v0 = lhu(a1 + 0x182);
    sh(v0, a0 + 0x6);
    v0 = lhu(a1 + 0x184);
    sh(v0, a0 + 0x8);
    v0 = lhu(a1 + 0x186);
    sh(v0, a0 + 0xA);
    v0 = lhu(a1 + 0x1B0);
    sh(v0, a0 + 0xC);
    v0 = lhu(a1 + 0x1B2);
    sh(v0, a0 + 0xE);
    v0 = lhu(a1 + 0x1AA);
    v0 >>= 2;
    v0 &= 1;
    sw(v0, a0 + 0x10);
    v0 = lhu(a1 + 0x1AA);
    v0 &= 1;
    sw(v0, a0 + 0x14);
    v0 = lhu(a1 + 0x1B4);
    sh(v0, a0 + 0x18);
    v0 = lhu(a1 + 0x1B6);
    sh(v0, a0 + 0x1A);
    v0 = lhu(a1 + 0x1AA);
    v0 >>= 3;
    v0 &= 1;
    sw(v0, a0 + 0x1C);
    v0 = lhu(a1 + 0x1AA);
    v0 >>= 1;
    v0 &= 1;
    sw(v0, a0 + 0x20);
    v0 = lhu(a1 + 0x1AA);
    v0 &= 0x3F00;
    v0 >>= 8;
    sh(v0, a0 + 0x24);
    v0 = lhu(a1 + 0x1AA);
    v0 >>= 7;
    v0 &= 1;
    sh(v0, a0 + 0x26);
    v0 = lhu(a1 + 0x1AC);
    v0 &= 0xE;
    v0 >>= 1;
    sh(v0, a0 + 0x28);
    v0 = lhu(a1 + 0x1AC);
    v0 &= 1;
    sh(v0, a0 + 0x2A);
    v0 = lhu(a1 + 0x19E);
    v1 = lhu(a1 + 0x19C);
    v0 <<= 16;
    v1 |= v0;
    sw(v1, a0 + 0x2C);
    v0 = lhu(a1 + 0x1AE);
    sh(v0, a0 + 0x30);
    v0 = 0;                                             // Result = 00000000
    return;
}

void LIBSPU_SysSpu_setRegister() noexcept {
    v0 = a0 << 1;
    if (a2 != 0) goto loc_80053D24;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v0 += v1;
    sh(a1, v0);
    goto loc_80053D40;
loc_80053D24:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A08);                               // Load from: gLIBSPU__spu_RXX (80076A08)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v0 += a0;
    v1 = i32(a1) >> v1;
    sh(v1, v0);
loc_80053D40:
    return;
}

void LIBSPU__SpuDataCallback() noexcept {
loc_80053D58:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = a0;
    a0 = 4;                                             // Result = 00000004
    LIBETC_DMACallback();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU__SpuCallback() noexcept {
loc_80053D7C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = a0;
    a0 = 9;                                             // Result = 00000009
    LIBETC_InterruptCallback();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU_SpuSetCommonAttr() noexcept {
loc_80053DA0:
    t0 = lw(a0);
    t1 = (t0 < 1);
    sp -= 0x10;
    if (t1 != 0) goto loc_80053DC8;
    v0 = t0 & 1;
    {
        const bool bJump = (v0 == 0);
        v0 = t0 & 4;
        if (bJump) goto loc_80053E7C;
    }
    if (v0 == 0) goto loc_80053E30;
loc_80053DC8:
    v1 = lh(a0 + 0x8);
    v0 = (v1 < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80053E30;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x1F14;                                       // Result = JumpTable_LIBSPU_SpuSetCommonAttr_1[0] (80011F14)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80053E30: goto loc_80053E30;
        case 0x80053DF8: goto loc_80053DF8;
        case 0x80053E00: goto loc_80053E00;
        case 0x80053E08: goto loc_80053E08;
        case 0x80053E10: goto loc_80053E10;
        case 0x80053E18: goto loc_80053E18;
        case 0x80053E20: goto loc_80053E20;
        case 0x80053E28: goto loc_80053E28;
        default: jump_table_err(); break;
    }
loc_80053DF8:
    a1 = 0x8000;                                        // Result = 00008000
    goto loc_80053E38;
loc_80053E00:
    a1 = 0x9000;                                        // Result = 00009000
    goto loc_80053E38;
loc_80053E08:
    a1 = 0xA000;                                        // Result = 0000A000
    goto loc_80053E38;
loc_80053E10:
    a1 = 0xB000;                                        // Result = 0000B000
    goto loc_80053E38;
loc_80053E18:
    a1 = 0xC000;                                        // Result = 0000C000
    goto loc_80053E38;
loc_80053E20:
    a1 = 0xD000;                                        // Result = 0000D000
    goto loc_80053E38;
loc_80053E28:
    a1 = 0xE000;                                        // Result = 0000E000
    goto loc_80053E38;
loc_80053E30:
    a3 = lhu(a0 + 0x4);
    a1 = 0;                                             // Result = 00000000
loc_80053E38:
    v0 = a3 & 0x7FFF;
    if (a1 == 0) goto loc_80053E6C;
    a2 = lh(a0 + 0x4);
    v0 = (i32(a2) < 0x80);
    v1 = a2;
    if (v0 != 0) goto loc_80053E5C;
    a3 = 0x7F;                                          // Result = 0000007F
    goto loc_80053E68;
loc_80053E5C:
    a3 = v1;
    if (i32(a2) >= 0) goto loc_80053E68;
    a3 = 0;                                             // Result = 00000000
loc_80053E68:
    v0 = a3 & 0x7FFF;
loc_80053E6C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 |= a1;
    sh(v0, v1 + 0x180);
loc_80053E7C:
    v0 = t0 & 2;
    if (t1 != 0) goto loc_80053E94;
    {
        const bool bJump = (v0 == 0);
        v0 = t0 & 8;
        if (bJump) goto loc_80053F48;
    }
    if (v0 == 0) goto loc_80053EFC;
loc_80053E94:
    v1 = lh(a0 + 0xA);
    v0 = (v1 < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80053EFC;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x1F34;                                       // Result = JumpTable_LIBSPU_SpuSetCommonAttr_2[0] (80011F34)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80053EFC: goto loc_80053EFC;
        case 0x80053EC4: goto loc_80053EC4;
        case 0x80053ECC: goto loc_80053ECC;
        case 0x80053ED4: goto loc_80053ED4;
        case 0x80053EDC: goto loc_80053EDC;
        case 0x80053EE4: goto loc_80053EE4;
        case 0x80053EEC: goto loc_80053EEC;
        case 0x80053EF4: goto loc_80053EF4;
        default: jump_table_err(); break;
    }
loc_80053EC4:
    a1 = 0x8000;                                        // Result = 00008000
    goto loc_80053F04;
loc_80053ECC:
    a1 = 0x9000;                                        // Result = 00009000
    goto loc_80053F04;
loc_80053ED4:
    a1 = 0xA000;                                        // Result = 0000A000
    goto loc_80053F04;
loc_80053EDC:
    a1 = 0xB000;                                        // Result = 0000B000
    goto loc_80053F04;
loc_80053EE4:
    a1 = 0xC000;                                        // Result = 0000C000
    goto loc_80053F04;
loc_80053EEC:
    a1 = 0xD000;                                        // Result = 0000D000
    goto loc_80053F04;
loc_80053EF4:
    a1 = 0xE000;                                        // Result = 0000E000
    goto loc_80053F04;
loc_80053EFC:
    t2 = lhu(a0 + 0x6);
    a1 = 0;                                             // Result = 00000000
loc_80053F04:
    v0 = t2 & 0x7FFF;
    if (a1 == 0) goto loc_80053F38;
    a2 = lh(a0 + 0x6);
    v0 = (i32(a2) < 0x80);
    v1 = a2;
    if (v0 != 0) goto loc_80053F28;
    t2 = 0x7F;                                          // Result = 0000007F
    goto loc_80053F34;
loc_80053F28:
    t2 = v1;
    if (i32(a2) >= 0) goto loc_80053F34;
    t2 = 0;                                             // Result = 00000000
loc_80053F34:
    v0 = t2 & 0x7FFF;
loc_80053F38:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 |= a1;
    sh(v0, v1 + 0x182);
loc_80053F48:
    v0 = t0 & 0x40;
    if (t1 != 0) goto loc_80053F58;
    if (v0 == 0) goto loc_80053F6C;
loc_80053F58:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 = lhu(a0 + 0x10);
    sh(v0, v1 + 0x1B0);
loc_80053F6C:
    v0 = t0 & 0x80;
    if (t1 != 0) goto loc_80053F7C;
    if (v0 == 0) goto loc_80053F90;
loc_80053F7C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 = lhu(a0 + 0x12);
    sh(v0, v1 + 0x1B2);
loc_80053F90:
    v0 = t0 & 0x100;
    if (t1 != 0) goto loc_80053FA0;
    if (v0 == 0) goto loc_80053FE4;
loc_80053FA0:
    v0 = lw(a0 + 0x14);
    if (v0 != 0) goto loc_80053FC8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 &= 0xFFFB;
    goto loc_80053FE0;
loc_80053FC8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 |= 4;
loc_80053FE0:
    sh(v1, v0 + 0x1AA);
loc_80053FE4:
    v0 = t0 & 0x200;
    if (t1 != 0) goto loc_80053FF4;
    if (v0 == 0) goto loc_80054038;
loc_80053FF4:
    v0 = lw(a0 + 0x18);
    if (v0 != 0) goto loc_8005401C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 &= 0xFFFE;
    goto loc_80054034;
loc_8005401C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 |= 1;
loc_80054034:
    sh(v1, v0 + 0x1AA);
loc_80054038:
    v0 = t0 & 0x400;
    if (t1 != 0) goto loc_80054048;
    if (v0 == 0) goto loc_8005405C;
loc_80054048:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 = lhu(a0 + 0x1C);
    sh(v0, v1 + 0x1B4);
loc_8005405C:
    v0 = t0 & 0x800;
    if (t1 != 0) goto loc_8005406C;
    if (v0 == 0) goto loc_80054080;
loc_8005406C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v0 = lhu(a0 + 0x1E);
    sh(v0, v1 + 0x1B6);
loc_80054080:
    v0 = t0 & 0x1000;
    if (t1 != 0) goto loc_80054090;
    if (v0 == 0) goto loc_800540D4;
loc_80054090:
    v0 = lw(a0 + 0x20);
    if (v0 != 0) goto loc_800540B8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 &= 0xFFF7;
    goto loc_800540D0;
loc_800540B8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 |= 8;
loc_800540D0:
    sh(v1, v0 + 0x1AA);
loc_800540D4:
    v0 = t0 & 0x2000;
    if (t1 != 0) goto loc_800540E4;
    if (v0 == 0) goto loc_80054128;
loc_800540E4:
    v0 = lw(a0 + 0x24);
    if (v0 != 0) goto loc_8005410C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 &= 0xFFFD;
    goto loc_80054124;
loc_8005410C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A3C);                               // Load from: gLIBSPU__spu_RXX (80076A3C)
    v1 = lhu(v0 + 0x1AA);
    v1 |= 2;
loc_80054124:
    sh(v1, v0 + 0x1AA);
loc_80054128:
    sp += 0x10;
    return;
}

void LIBSPU_SpuGetReverbOffsetAddr() noexcept {
loc_80054134:
    sp -= 0x20;
    a1 = 0x69;                                          // Result = 00000069
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    sw(ra, sp + 0x18);
    a2 = sp + 0x10;
    LIBSPU__spu_ioctl();
    v0 = lw(sp + 0x10);
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void LIBSPU_SpuClearReverbWorkArea() noexcept {
loc_80054164:
    sp -= 0x38;
    sw(s0, sp + 0x20);
    s0 = a0;
    sw(s4, sp + 0x30);
    v0 = (s0 < 0xA);
    sw(ra, sp + 0x34);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(0, sp + 0x10);
    s4 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_800541B8;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x6E78;                                       // Result = 80076E78
    v0 = s0 << 2;
    s1 = v0 + v1;
    a0 = lw(s1);
    LIBSPU__SpuIsInAllocateArea_();
    if (v0 == 0) goto loc_800541C0;
loc_800541B8:
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_80054300;
loc_800541C0:
    v0 = 0x10000;                                       // Result = 00010000
    if (s0 != 0) goto loc_800541E4;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v1 = 0x10;                                          // Result = 00000010
    s1 = v1 << v0;
    v1 = 0xFFF0;                                        // Result = 0000FFF0
    s2 = v1 << v0;
    goto loc_800541FC;
loc_800541E4:
    a0 = lw(s1);
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v0 -= a0;
    s1 = v0 << v1;
    s2 = a0 << v1;
loc_800541FC:
    a1 = 0x2F;                                          // Result = 0000002F
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x14;
    LIBSPU__spu_ioctl();
    v0 = lw(sp + 0x14);
    a1 = 0x2E;                                          // Result = 0000002E
    if (v0 != 0) goto loc_8005423C;
    a2 = sp + 0x18;
    s4 = 1;                                             // Result = 00000001
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    v0 = 1;                                             // Result = 00000001
    sw(v0, sp + 0x18);
    LIBSPU__spu_ioctl();
loc_8005423C:
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7C20);                               // Load from: gLIBSPU__spu_transferCallback (80097C20)
    s3 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_80054268;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7C20);                               // Load from: gLIBSPU__spu_transferCallback (80097C20)
    sw(v0, sp + 0x10);
    at = 0x80090000;                                    // Result = 80090000
    sw(0, at + 0x7C20);                                 // Store to: gLIBSPU__spu_transferCallback (80097C20)
loc_80054268:
    v0 = (s1 < 0x401);
loc_8005426C:
    s0 = s1;
    if (v0 != 0) goto loc_8005427C;
    s0 = 0x400;                                         // Result = 00000400
    goto loc_80054280;
loc_8005427C:
    s3 = 0;                                             // Result = 00000000
loc_80054280:
    a0 = 2;                                             // Result = 00000002
    a1 = s2;
    LIBSPU__spu_t();
    a0 = 1;                                             // Result = 00000001
    LIBSPU__spu_t();
    a0 = 3;                                             // Result = 00000003
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x6A6C;                                       // Result = 80076A6C
    a2 = s0;
    LIBSPU__spu_t();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A5C);                               // Load from: gLIBSPU__spu_EVdma (80076A5C)
    s1 -= 0x400;
    s2 += 0x400;
    LIBAPI_WaitEvent();
    v0 = (s1 < 0x401);
    if (s3 != 0) goto loc_8005426C;
    a1 = 0x2E;                                          // Result = 0000002E
    if (s4 == 0) goto loc_800542DC;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x14;
    LIBSPU__spu_ioctl();
loc_800542DC:
    v0 = lw(sp + 0x10);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80054300;
    }
    v0 = lw(sp + 0x10);
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7C20);                                // Store to: gLIBSPU__spu_transferCallback (80097C20)
    v0 = 0;                                             // Result = 00000000
loc_80054300:
    ra = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x38;
    return;
}

void LIBSPU__SpuInit() noexcept {
loc_80054334:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    LIBETC_ResetCallback();
    a0 = s0;
    LIBSPU__spu_init();
    LIBSPU_SpuStart();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(0, at - 0x6E70);                                 // Store to: gLIBSPU__spu_rev_attr[2] (800A9190)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6E70);                               // Load from: gLIBSPU__spu_rev_attr[2] (800A9190)
    at = 0x800B0000;                                    // Result = 800B0000
    sh(0, at - 0x6E6C);                                 // Store to: gLIBSPU__spu_rev_attr[4] (800A9194)
    at = 0x800B0000;                                    // Result = 800B0000
    sh(0, at - 0x6E6A);                                 // Store to: gLIBSPU__spu_rev_attr[5] (800A9196)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(0, at - 0x6E68);                                 // Store to: gLIBSPU__spu_rev_attr[6] (800A9198)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(0, at - 0x6E64);                                 // Store to: gLIBSPU__spu_rev_attr[8] (800A919C)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x6E78;                                       // Result = 80076E78
    at += v0;
    v0 = lw(at);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x6A54;                                       // Result = gLIBSPU__spu_rev_offsetaddr (80076A54)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6A4C);                                 // Store to: gLIBSPU__spu_rev_flag (80076A4C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6A50);                                 // Store to: gLIBSPU__spu_rev_reserve_wa (80076A50)
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x6A54);                                // Store to: gLIBSPU__spu_rev_offsetaddr (80076A54)
    a1 = 0x61;                                          // Result = 00000061
    LIBSPU__spu_ioctl();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x6A48;                                       // Result = gLIBSPU__spu_trans_mode (80076A48)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6A48);                                 // Store to: gLIBSPU__spu_trans_mode (80076A48)
    a1 = 0x21;                                          // Result = 00000021
    LIBSPU__spu_ioctl();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6A58);                                 // Store to: 80076A58
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6A64);                                 // Store to: 80076A64
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6A68);                                 // Store to: 80076A68
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU_SpuStart() noexcept {
loc_80054418:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6E6C);                               // Load from: gbLIBSPU__spu_isCalled (80076E6C)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 != 0) goto loc_80054488;
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x6E6C);                                // Store to: gbLIBSPU__spu_isCalled (80076E6C)
    LIBAPI_EnterCriticalSection();
    a0 = 0;                                             // Result = 00000000
    a1 = 3;                                             // Result = 00000003
    LIBSPU__spu_open();
    a0 = 0xF0000000;                                    // Result = F0000000
    a0 |= 9;                                            // Result = F0000009
    a1 = 0x20;                                          // Result = 00000020
    a2 = 0x2000;                                        // Result = 00002000
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x6A44);                                // Store to: gLIBSPU__spu_fd (80076A44)
    a3 = 0;                                             // Result = 00000000
    LIBAPI_OpenEvent();
    a0 = v0;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x6A5C);                                // Store to: gLIBSPU__spu_EVdma (80076A5C)
    LIBAPI_EnableEvent();
    LIBAPI_ExitCriticalSection();
loc_80054488:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU_SpuSetReverbDepth() noexcept {
loc_80054498:
    a1 = lw(a0);
    a2 = (a1 < 1);
    v0 = a1 & 2;
    if (a2 != 0) goto loc_800544B4;
    if (v0 == 0) goto loc_800544D8;
loc_800544B4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6E70);                               // Load from: gLIBSPU__spu_RXX (80076E70)
    v1 = lhu(a0 + 0x8);
    sh(v1, v0 + 0x184);
    v0 = lhu(a0 + 0x8);
    at = 0x800B0000;                                    // Result = 800B0000
    sh(v0, at - 0x6E6C);                                // Store to: gLIBSPU__spu_rev_attr[4] (800A9194)
loc_800544D8:
    if (a2 != 0) goto loc_800544EC;
    v0 = a1 & 4;
    if (v0 == 0) goto loc_80054510;
loc_800544EC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6E70);                               // Load from: gLIBSPU__spu_RXX (80076E70)
    v1 = lhu(a0 + 0xA);
    sh(v1, v0 + 0x186);
    v0 = lhu(a0 + 0xA);
    at = 0x800B0000;                                    // Result = 800B0000
    sh(v0, at - 0x6E6A);                                // Store to: gLIBSPU__spu_rev_attr[5] (800A9196)
loc_80054510:
    v0 = 0;                                             // Result = 00000000
    return;
}

void LIBSPU_SpuSetReverbVoice() noexcept {
loc_80054518:
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(a1, sp + 0x24);
    if (a0 == 0) goto loc_80054544;
    v0 = 1;                                             // Result = 00000001
    a1 = 0x66;                                          // Result = 00000066
    if (a0 != v0) goto loc_8005455C;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a1 = 0x65;                                          // Result = 00000065
    goto loc_80054550;
loc_80054544:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a1 = 0x67;                                          // Result = 00000067
loc_80054550:
    a2 = sp + 0x24;
    LIBSPU__spu_ioctl();
    a1 = 0x66;                                          // Result = 00000066
loc_8005455C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x10;
    LIBSPU__spu_ioctl();
    v0 = lw(sp + 0x10);
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void LIBSPU_SpuInit() noexcept {
loc_80054580:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0;                                             // Result = 00000000
    LIBSPU__SpuInit();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU_SpuSetReverb() noexcept {
loc_800545A0:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    if (s0 == 0) goto loc_800545C8;
    v0 = 1;                                             // Result = 00000001
    if (s0 == v0) goto loc_800545E8;
    goto loc_80054654;
loc_800545C8:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x6A4C;                                       // Result = gLIBSPU__spu_rev_flag (80076A4C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6A4C);                                 // Store to: gLIBSPU__spu_rev_flag (80076A4C)
    a1 = 0x60;                                          // Result = 00000060
    goto loc_8005464C;
loc_800545E8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A50);                               // Load from: gLIBSPU__spu_rev_reserve_wa (80076A50)
    a1 = 0x60;                                          // Result = 00000060
    if (v0 == s0) goto loc_80054634;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A54);                               // Load from: gLIBSPU__spu_rev_offsetaddr (80076A54)
    LIBSPU__SpuIsInAllocateArea_();
    a1 = 0x60;                                          // Result = 00000060
    if (v0 == 0) goto loc_80054634;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x6A4C;                                       // Result = gLIBSPU__spu_rev_flag (80076A4C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6A4C);                                 // Store to: gLIBSPU__spu_rev_flag (80076A4C)
    goto loc_8005464C;
loc_80054634:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x6A4C;                                       // Result = gLIBSPU__spu_rev_flag (80076A4C)
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x6A4C);                                // Store to: gLIBSPU__spu_rev_flag (80076A4C)
loc_8005464C:
    LIBSPU__spu_ioctl();
loc_80054654:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A4C);                               // Load from: gLIBSPU__spu_rev_flag (80076A4C)
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU_SpuQuit() noexcept {
loc_80054670:
    sp -= 0x18;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6E6C);                               // Load from: gbLIBSPU__spu_isCalled (80076E6C)
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x10);
    if (v1 != v0) goto loc_800546D0;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6E6C);                                 // Store to: gbLIBSPU__spu_isCalled (80076E6C)
    LIBAPI_EnterCriticalSection();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    LIBSPU__spu_close();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A5C);                               // Load from: gLIBSPU__spu_EVdma (80076A5C)
    LIBAPI_CloseEvent();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A5C);                               // Load from: gLIBSPU__spu_EVdma (80076A5C)
    LIBAPI_DisableEvent();
    LIBAPI_ExitCriticalSection();
loc_800546D0:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU_SpuIsTransferCompleted() noexcept {
loc_800546E0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A48);                               // Load from: gLIBSPU__spu_trans_mode (80076A48)
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s0, sp + 0x10);
    s0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x18);
    if (v0 == s0) goto loc_80054718;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x655C);                               // Load from: gLIBSPU__spu_inTransfer (8009655C)
    if (v0 != s0) goto loc_80054720;
loc_80054718:
    v0 = 1;                                             // Result = 00000001
    goto loc_80054770;
loc_80054720:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A5C);                               // Load from: gLIBSPU__spu_EVdma (80076A5C)
    LIBAPI_TestEvent();
    if (s1 != s0) goto loc_80054760;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80054768;
    }
loc_80054740:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A5C);                               // Load from: gLIBSPU__spu_EVdma (80076A5C)
    LIBAPI_TestEvent();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80054740;
    }
    goto loc_80054768;
loc_80054760:
    if (v0 != s0) goto loc_80054770;
loc_80054768:
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x655C);                                // Store to: gLIBSPU__spu_inTransfer (8009655C)
loc_80054770:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void LIBSPU_SpuInitMalloc() noexcept {
loc_80054788:
    v0 = a0;
    v1 = 0x40000000;                                    // Result = 40000000
    if (i32(v0) > 0) goto loc_8005479C;
    v0 = 0;                                             // Result = 00000000
    goto loc_800547D4;
loc_8005479C:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x6E60);                               // Load from: gLIBSPU__spu_mem_mode_plus (800A91A0)
    v1 |= 0x1010;                                       // Result = 40001010
    sw(v1, a1);
    v1 = 0x10000;                                       // Result = 00010000
    at = 0x80090000;                                    // Result = 80090000
    sw(a1, at + 0x7784);                                // Store to: gLIBSPU__spu_memList (80097784)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x6A38);                                 // Store to: gLIBSPU__spu_AllocLastNum (80076A38)
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x6A34);                                // Store to: gLIBSPU__spu_AllocBlockNum (80076A34)
    v1 = v1 << a0;
    v1 -= 0x1010;
    sw(v1, a1 + 0x4);
loc_800547D4:
    return;
}

void LIBSPU_SpuSetTransferMode() noexcept {
loc_800547DC:
    sp -= 0x20;
    sw(ra, sp + 0x18);
    if (a0 == 0) goto loc_800547FC;
    v0 = 1;                                             // Result = 00000001
    if (a0 != v0) goto loc_80054800;
    sw(0, sp + 0x10);
    goto loc_80054804;
loc_800547FC:
    v0 = 1;                                             // Result = 00000001
loc_80054800:
    sw(v0, sp + 0x10);
loc_80054804:
    a1 = 0x21;                                          // Result = 00000021
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x6A48);                                // Store to: gLIBSPU__spu_trans_mode (80076A48)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x10;
    LIBSPU__spu_ioctl();
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void LIBSPU_SpuSetTransferStartAddr() noexcept {
loc_80054830:
    sp -= 0x18;
    v0 = 0x70000;                                       // Result = 00070000
    v0 |= 0xFFF8;                                       // Result = 0007FFF8
    v0 = (v0 < a0);
    sw(ra, sp + 0x10);
    sw(a0, sp + 0x18);
    if (v0 == 0) goto loc_80054854;
    v0 = 0;                                             // Result = 00000000
    goto loc_80054884;
loc_80054854:
    v0 = a0 & 7;
    a1 = 0x2B;                                          // Result = 0000002B
    if (v0 == 0) goto loc_80054870;
    v0 = a0 + 8;
    v1 = -8;                                            // Result = FFFFFFF8
    v0 &= v1;
    sw(v0, sp + 0x18);
loc_80054870:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x18;
    LIBSPU__spu_ioctl();
    v0 = lw(sp + 0x18);
loc_80054884:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU_SpuWrite() noexcept {
loc_80054894:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a1;
    v0 = 0x70000;                                       // Result = 00070000
    v0 |= 0xF000;                                       // Result = 0007F000
    v0 = (v0 < s0);
    sw(ra, sp + 0x14);
    if (v0 == 0) goto loc_800548BC;
    s0 = 0x70000;                                       // Result = 00070000
    s0 |= 0xF000;                                       // Result = 0007F000
loc_800548BC:
    a1 = s0;
    LIBSPU__spu_write();
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7C20);                               // Load from: gLIBSPU__spu_transferCallback (80097C20)
    {
        const bool bJump = (v0 != 0);
        v0 = s0;
        if (bJump) goto loc_800548E0;
    }
    at = 0x80090000;                                    // Result = 80090000
    sw(0, at + 0x655C);                                 // Store to: gLIBSPU__spu_inTransfer (8009655C)
loc_800548E0:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU_SpuSetKeyOnWithAttr() noexcept {
loc_800548F4:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    LIBSPU_SpuSetVoiceAttr();
    a1 = lw(s0);
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuSetKey();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU_SpuSetKey() noexcept {
loc_80054928:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    sw(a1, sp + 0x1C);
    if (a0 == 0) goto loc_80054968;
    v0 = 1;                                             // Result = 00000001
    a1 = 0x204;                                         // Result = 00000204
    if (a0 != v0) goto loc_80054998;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x1C;
    LIBSPU__spu_ioctl();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x6A58);                               // Load from: 80076A58
    v1 = lw(sp + 0x1C);
    v0 |= v1;
    goto loc_80054990;
loc_80054968:
    a1 = 0x203;                                         // Result = 00000203
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A44);                               // Load from: gLIBSPU__spu_fd (80076A44)
    a2 = sp + 0x1C;
    LIBSPU__spu_ioctl();
    v0 = lw(sp + 0x1C);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x6A58);                               // Load from: 80076A58
    v0 = ~v0;
    v0 &= v1;
loc_80054990:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x6A58);                                // Store to: 80076A58
loc_80054998:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBSPU_SpuRGetAllKeysStatus() noexcept {
    v0 = (i32(a0) < 0x18);
    if (i32(a0) >= 0) goto loc_800549B8;
    a0 = 0;                                             // Result = 00000000
    v0 = (i32(a0) < 0x18);                              // Result = 00000001
loc_800549B8:
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a1) < 0x18);
        if (bJump) goto loc_800549DC;
    }
    if (v0 != 0) goto loc_800549CC;
    a1 = 0x17;                                          // Result = 00000017
loc_800549CC:
    v0 = (i32(a1) < i32(a0));
    if (i32(a1) < 0) goto loc_800549DC;
    a3 = a0;
    if (v0 == 0) goto loc_800549E4;
loc_800549DC:
    v0 = -3;                                            // Result = FFFFFFFD
    goto loc_80054A70;
loc_800549E4:
    a1++;
    v0 = (i32(a3) < i32(a1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80054A70;
    }
    t0 = 1;                                             // Result = 00000001
    t2 = 3;                                             // Result = 00000003
    t1 = 2;                                             // Result = 00000002
    a2 += a3;
loc_80054A04:
    v1 = a3 << 4;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7170);                               // Load from: 80077170
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A58);                               // Load from: 80076A58
    v1 += v0;
    v0 = t0 << a3;
    v0 &= a0;
    v1 = lhu(v1 + 0xC);
    if (v0 == 0) goto loc_80054A48;
    if (v1 == 0) goto loc_80054A40;
    sb(t0, a2);
    goto loc_80054A5C;
loc_80054A40:
    sb(t2, a2);
    goto loc_80054A5C;
loc_80054A48:
    if (v1 == 0) goto loc_80054A58;
    sb(t1, a2);
    goto loc_80054A5C;
loc_80054A58:
    sb(0, a2);
loc_80054A5C:
    a3++;
    v0 = (i32(a3) < i32(a1));
    a2++;
    if (v0 != 0) goto loc_80054A04;
    v0 = 0;                                             // Result = 00000000
loc_80054A70:
    return;
}

void LIBSPU_SpuGetAllKeysStatus() noexcept {
loc_80054A78:
    t2 = 0x18;                                          // Result = 00000018
    a2 = 0;                                             // Result = 00000000
    a3 = 1;                                             // Result = 00000001
    t1 = 3;                                             // Result = 00000003
    t0 = 2;                                             // Result = 00000002
    a1 = a0;
loc_80054A90:
    v1 = a2 << 4;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7170);                               // Load from: 80077170
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x6A58);                               // Load from: 80076A58
    v1 += v0;
    v0 = a3 << a2;
    v0 &= a0;
    v1 = lhu(v1 + 0xC);
    if (v0 == 0) goto loc_80054AD4;
    if (v1 == 0) goto loc_80054ACC;
    sb(a3, a1);
    goto loc_80054AE8;
loc_80054ACC:
    sb(t1, a1);
    goto loc_80054AE8;
loc_80054AD4:
    if (v1 == 0) goto loc_80054AE4;
    sb(t0, a1);
    goto loc_80054AE8;
loc_80054AE4:
    sb(0, a1);
loc_80054AE8:
    a2++;
    v0 = (i32(a2) < i32(t2));
    a1++;
    if (v0 != 0) goto loc_80054A90;
    return;
}
