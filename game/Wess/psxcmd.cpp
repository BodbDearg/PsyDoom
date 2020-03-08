#include "psxcmd.h"

#include "psxspu.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBSPU.h"
#include "wessseq.h"

void start_record_music_mute() noexcept {
loc_800459E0:
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5A10);                                // Store to: gpWess_pnotestate (80075A10)
    if (a0 == 0) goto loc_800459F4;
    sw(0, a0);
loc_800459F4:
    return;
}

void end_record_music_mute() noexcept {
loc_800459FC:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5A10);                                 // Store to: gpWess_pnotestate (80075A10)
    return;
}

void add_music_mute_note() noexcept {
loc_80045A0C:
    t0 = lw(sp + 0x10);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A10);                               // Load from: gpWess_pnotestate (80075A10)
    t1 = lw(sp + 0x14);
    if (v1 == 0) goto loc_80045AC4;
    v0 = lw(v1);
    v0 <<= 4;
    v0 += v1;
    sh(a0, v0 + 0x4);
    v0 = lw(v1);
    v0 <<= 4;
    v0 += v1;
    sh(a1, v0 + 0x6);
    v0 = lw(v1);
    v0 <<= 4;
    v0 += v1;
    sb(a2, v0 + 0x8);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A10);                               // Load from: gpWess_pnotestate (80075A10)
    v0 = lw(v1);
    v0 <<= 4;
    v1 += v0;
    sb(a3, v1 + 0x9);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A10);                               // Load from: gpWess_pnotestate (80075A10)
    v0 = lw(v1);
    v0 <<= 4;
    v0 += v1;
    sw(t0, v0 + 0xC);
    v0 = lw(v1);
    v0 <<= 4;
    v0 += v1;
    sw(t1, v0 + 0x10);
    v0 = lw(v1);
    v0++;
    sw(v0, v1);
loc_80045AC4:
    return;
}

void PSX_UNKNOWN_DrvFunc() noexcept {
    v1 = 0x10000000;                                    // Result = 10000000
    v0 = 0x1F;                                          // Result = 0000001F
    goto loc_80045AF0;
loc_80045AD8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A07);                              // Load from: gWess_UNKNOWN_status_byte (80075A07)
    {
        const bool bJump = (v0 == 0);
        v0 += 0xFF;
        if (bJump) goto loc_80045B04;
    }
    v1 = u32(i32(v1) >> 1);
loc_80045AF0:
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x5A07);                                // Store to: gWess_UNKNOWN_status_byte (80075A07)
    v0 = (i32(a0) < i32(v1));
    if (v0 != 0) goto loc_80045AD8;
loc_80045B04:
    return;
}

void TriggerPSXVoice() noexcept {
loc_80045B0C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = 0x60000;                                       // Result = 00060000
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    v1 = lbu(s0 + 0x2);
    v0 |= 0xE3;                                         // Result = 000600E3
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE6C);                                 // Store to: 8007F194
    v0 = 1;                                             // Result = 00000001
    a1 = v0 << v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(a1, at - 0xE70);                                 // Store to: 8007F190
    v1 = lbu(s0 + 0x3);
    v0 = v1 << 2;
    v0 += v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE94);                                // Load from: 8007F16C
    v0 <<= 4;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF5C);                                 // Store to: 8007F0A4
    v0 = lbu(v0 + 0x9);
    s1 = a2;
    if (v0 == 0) goto loc_80045BC8;
    v0 = lbu(s0 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0xE18;                                        // Result = 8007F1E8
    at += v0;
    v0 = lbu(at);
    if (v0 != 0) goto loc_80045C0C;
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuSetReverbVoice(a0, a1);
    v1 = lbu(s0 + 0x2);
    v0 = 0x7F;                                          // Result = 0000007F
    at = 0x80080000;                                    // Result = 80080000
    at -= 0xE18;                                        // Result = 8007F1E8
    at += v1;
    sb(v0, at);
    goto loc_80045C0C;
loc_80045BC8:
    v0 = lbu(s0 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0xE18;                                        // Result = 8007F1E8
    at += v0;
    v0 = lbu(at);
    if (v0 == 0) goto loc_80045C0C;
    a0 = 0;                                             // Result = 00000000
    LIBSPU_SpuSetReverbVoice(a0, a1);
    v0 = lbu(s0 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0xE18;                                        // Result = 8007F1E8
    at += v0;
    sb(0, at);
loc_80045C0C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    {
        const bool bJump = (v0 == 0);
        v0 = 0x40;                                      // Result = 00000040
        if (bJump) goto loc_80045C88;
    }
    v0 = lw(s0 + 0x8);
    v1 = lbu(v0 + 0x3);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF5C);                                // Load from: 8007F0A4
    v1 <<= 24;
    v0 = lbu(v0 + 0xD);
    v1 = u32(i32(v1) >> 24);
    v0 += v1;
    v0 -= 0x40;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xF58);                                 // Store to: 8007F0A8
    v0 = (i32(v0) < 0x80);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x7F;                                      // Result = 0000007F
        if (bJump) goto loc_80045C64;
    }
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xF58);                                 // Store to: 8007F0A8
loc_80045C64:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lh(v0 - 0xF58);                                // Load from: 8007F0A8
    if (i32(v0) >= 0) goto loc_80045C90;
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xF58);                                  // Store to: 8007F0A8
    goto loc_80045C90;
loc_80045C88:
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xF58);                                 // Store to: 8007F0A8
loc_80045C90:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF5C);                                // Load from: 8007F0A4
    v0 = lbu(a0 + 0x13);
    if (v0 != 0) goto loc_80045CE4;
    v0 = lw(s0 + 0x8);
    v1 = lbu(v0 + 0x2);
    v0 = s1 & 0xFF;
    mult(v0, v1);
    v1 = lo;
    v0 = lbu(a0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A04);                              // Load from: gWess_master_sfx_volume (80075A04)
    mult(v1, v0);
    goto loc_80045D1C;
loc_80045CE4:
    v0 = lw(s0 + 0x8);
    v1 = lbu(v0 + 0x2);
    v0 = s1 & 0xFF;
    mult(v0, v1);
    v1 = lo;
    v0 = lbu(a0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A05);                              // Load from: gWess_master_mus_volume (80075A05)
    mult(v1, v0);
loc_80045D1C:
    v0 = lo;
    v0 = u32(i32(v0) >> 21);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF60);                                 // Store to: 8007F0A0
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    v0 = 1;                                             // Result = 00000001
    if (v1 != 0) goto loc_80045D68;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xF60);                               // Load from: 8007F0A0
    v0 <<= 6;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE68);                                 // Store to: 8007F198
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE66);                                 // Store to: 8007F19A
    goto loc_80045E00;
loc_80045D68:
    {
        const bool bJump = (v1 != v0);
        v0 = 0x80;                                      // Result = 00000080
        if (bJump) goto loc_80045DBC;
    }
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF60);                                // Load from: 8007F0A0
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xF58);                                // Load from: 8007F0A8
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE68);                                 // Store to: 8007F198
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE66);                                 // Store to: 8007F19A
    goto loc_80045E00;
loc_80045DBC:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF60);                                // Load from: 8007F0A0
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xF58);                                // Load from: 8007F0A8
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE66);                                 // Store to: 8007F19A
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE68);                                 // Store to: 8007F198
loc_80045E00:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF5C);                                // Load from: 8007F0A4
    v1 = lh(v0 + 0xE);
    if (v1 != 0) goto loc_80045E28;
    v0 = lbu(s0 + 0x5);
    v0 <<= 8;
    goto loc_80045EFC;
loc_80045E28:
    if (i32(v1) <= 0) goto loc_80045E84;
    v0 = lw(s0 + 0x8);
    v0 = lb(v0 + 0x9);
    mult(v1, v0);
    v0 = lo;
    v0 += 0x20;
    v1 = u32(i32(v0) >> 13);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF6C);                                 // Store to: 8007F094
    v0 &= 0x1FFF;
    v0 = u32(i32(v0) >> 6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF68);                                 // Store to: 8007F098
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF64);                                 // Store to: 8007F09C
    v0 = lbu(s0 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF68);                               // Load from: 8007F098
    v0 += v1;
    goto loc_80045EE8;
loc_80045E84:
    v0 = lw(s0 + 0x8);
    v0 = lb(v0 + 0x8);
    mult(v1, v0);
    v1 = 0x20;                                          // Result = 00000020
    v0 = lo;
    v1 -= v0;
    v0 = u32(i32(v1) >> 13);
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF6C);                                 // Store to: 8007F094
    v1 &= 0x1FFF;
    v1 = u32(i32(v1) >> 6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF68);                                 // Store to: 8007F098
    v0 = 0x80;                                          // Result = 00000080
    v0 -= v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF64);                                 // Store to: 8007F09C
    v0 = lbu(s0 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF68);                               // Load from: 8007F098
    v0 -= v1;
loc_80045EE8:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF64);                               // Load from: 8007F09C
    v0 <<= 8;
    v1 &= 0x7F;
    v0 |= v1;
loc_80045EFC:
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE5A);                                 // Store to: 8007F1A6
    v1 = lw(s0 + 0x8);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE58;                                        // Result = 8007F1A8
    v0 = lbu(v1 + 0x4);
    v1 = lbu(v1 + 0x5);
    v0 <<= 8;
    v0 |= v1;
    sh(v0, a0);                                         // Store to: 8007F1A8
    v0 = lw(s0 + 0xC);
    v0 = lw(v0 + 0x8);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE54);                                 // Store to: 8007F1AC
    v0 = lw(s0 + 0x8);
    v0 = lhu(v0 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE36);                                 // Store to: 8007F1CA
    v0 = lw(s0 + 0x8);
    v0 = lhu(v0 + 0xE);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xE34);                                 // Store to: 8007F1CC
    a0 -= 0x18;                                         // Result = 8007F190
    LIBSPU_SpuSetKeyOnWithAttr();
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PSX_DriverInit() noexcept {
loc_80045F8C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(a0);
    v1 = lw(a0 + 0x20);
    a1 = lw(a0 + 0x28);
    a2 = lw(a0 + 0x30);
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xE9C);                                 // Store to: 8007F164
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE84);                                 // Store to: 8007F17C
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xE98);                                 // Store to: gWess_Dvr_pss (8007F168)
    at = 0x80080000;                                    // Result = 80080000
    sw(a1, at - 0xE94);                                 // Store to: 8007F16C
    at = 0x80080000;                                    // Result = 80080000
    sw(a2, at - 0xF48);                                 // Store to: 8007F0B8
    v0 = lbu(a0 + 0x7);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xF54);                                 // Store to: 8007F0AC
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xF54);                               // Load from: 8007F0AC
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF4C);                                  // Store to: 8007F0B4
    if (v0 == 0) goto loc_8004604C;
    a3 = 1;                                             // Result = 00000001
    a1 = v0;
loc_80045FFC:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF4C);                                // Load from: 8007F0B4
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    a0 = v0 + a2;
    v0 = lbu(a0 + 0x1);
    {
        const bool bJump = (v0 != a3);
        v0 = v1 + 1;
        if (bJump) goto loc_80046038;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xE90);                                 // Store to: 8007F170
    goto loc_8004604C;
loc_80046038:
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF4C);                                 // Store to: 8007F0B4
    v0 = (i32(v0) < i32(a1));
    if (v0 != 0) goto loc_80045FFC;
loc_8004604C:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE9C);                                // Load from: 8007F164
    v0 = lw(v1 + 0xC);
    v0 = lbu(v0 + 0xA);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xF50);                                 // Store to: 8007F0B0
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xF50);                               // Load from: 8007F0B0
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF4C);                                  // Store to: 8007F0B4
    a3 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_800460E8;
    a2 = lw(v1 + 0x18);
    a1 = v0;
loc_80046090:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF4C);                                // Load from: 8007F0B4
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 += v1;
    v0 <<= 2;
    a0 = v0 + a2;
    v0 = lbu(a0 + 0x4);
    {
        const bool bJump = (v0 != a3);
        v0 = v1 + 1;
        if (bJump) goto loc_800460D4;
    }
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xE88);                                 // Store to: 8007F178
    goto loc_800460E8;
loc_800460D4:
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF4C);                                 // Store to: 8007F0B4
    v0 = (i32(v0) < i32(a1));
    if (v0 != 0) goto loc_80046090;
loc_800460E8:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xE88);                                // Load from: 8007F178
    v1 = lbu(a0 + 0x5);
    v0 = lh(a0 + 0x8);
    a1 = lw(a0 + 0x1C);
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xE8C);                                 // Store to: 8007F174
    v1 = lh(a0 + 0xC);
    a0 = lh(a0 + 0x10);
    v0 += a1;
    at = 0x80080000;                                    // Result = 80080000
    sw(a1, at - 0xE80);                                 // Store to: 8007F180
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE7C);                                 // Store to: 8007F184
    v1 <<= 4;
    v1 += v0;
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 2;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xE78);                                 // Store to: 8007F188
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE74);                                 // Store to: 8007F18C
    psxspu_init();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE18;                                        // Result = 8007F1E8
    v1 = 0x7F;                                          // Result = 0000007F
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF4C);                                  // Store to: 8007F0B4
loc_8004616C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF4C);                                // Load from: 8007F0B4
    v0 += a0;
    sb(v1, v0);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF4C);                                // Load from: 8007F0B4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF4C);                                 // Store to: 8007F0B4
    v0 = (i32(v0) < 0x18);
    if (v0 != 0) goto loc_8004616C;
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSX_DriverExit() noexcept {
loc_800461B4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBSPU_SpuQuit();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSX_DriverEntry1() noexcept {
loc_800461D4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE9C);                                // Load from: 8007F164
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lbu(v0 + 0x6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF44);                                 // Store to: 8007F0BC
    if (v0 == 0) goto loc_800462D8;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF34);                                 // Store to: 8007F0CC
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF40);                                 // Store to: 8007F0C0
    if (v1 == v0) goto loc_800462D8;
    s0 = 3;                                             // Result = 00000003
loc_80046238:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF34);                                // Load from: 8007F0CC
    v0 = lw(a0);
    v0 &= 3;
    if (v0 != s0) goto loc_800462A4;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE84);                                // Load from: 8007F17C
    v1 = lw(v0);
    v0 = lw(a0 + 0x10);
    v0 = (v0 < v1);
    if (v0 == 0) goto loc_800462A4;
    PSX_voiceparmoff();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF44);                                // Load from: 8007F0BC
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF44);                                 // Store to: 8007F0BC
    if (v0 == 0) goto loc_800462D8;
loc_800462A4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF34);                                // Load from: 8007F0CC
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF40);                                // Load from: 8007F0C0
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF34);                                 // Store to: 8007F0CC
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF40);                                 // Store to: 8007F0C0
    if (v1 != v0) goto loc_80046238;
loc_800462D8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A08);                               // Load from: 80075A08
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF30);                                  // Store to: 8007F0D0
    if (v0 == 0) goto loc_80046300;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF30);                                 // Store to: 8007F0D0
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5A08);                                 // Store to: 80075A08
loc_80046300:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A0C);                               // Load from: 80075A0C
    if (v0 == 0) goto loc_80046374;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x5A07);                              // Load from: gWess_UNKNOWN_status_byte (80075A07)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE70;                                        // Result = 8007F190
    sw(v0, a0);                                         // Store to: 8007F190
    v0 = 0x4400;                                        // Result = 00004400
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE6C);                                 // Store to: 8007F194
    v0 = 7;                                             // Result = 00000007
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xE44);                                 // Store to: 8007F1BC
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at - 0xE3A);                                 // Store to: 8007F1C6
    LIBSPU_SpuSetVoiceAttr(*vmAddrToPtr<SpuVoiceAttr>(a0));
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF30);                                // Load from: 8007F0D0
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A0C);                               // Load from: 80075A0C
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5A0C);                                 // Store to: 80075A0C
    v0 |= v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF30);                                 // Store to: 8007F0D0
loc_80046374:
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0xF30);                                // Load from: 8007F0D0
    if (a1 == 0) goto loc_80046398;
    a0 = 0;                                             // Result = 00000000
    LIBSPU_SpuSetKey(a0, a1);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF30);                                  // Store to: 8007F0D0
loc_80046398:
    s0 = 0x80080000;                                    // Result = 80080000
    s0 -= 0xE30;                                        // Result = 8007F1D0
    a0 = s0;                                            // Result = 8007F1D0
    LIBSPU_SpuGetAllKeysStatus(vmAddrToPtr<uint8_t>(a0));
    s1 = s0;                                            // Result = 8007F1D0
    s0 = -1;                                            // Result = FFFFFFFF
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE90);                                // Load from: 8007F170
    v0 = 0x18;                                          // Result = 00000018
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF3C);                                 // Store to: 8007F0C4
    v0 = 0x17;                                          // Result = 00000017
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xF38);                                  // Store to: 8007F0C8
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF3C);                                 // Store to: 8007F0C4
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF34);                                 // Store to: 8007F0CC
loc_800463E0:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF34);                                // Load from: 8007F0CC
    v0 = lw(a0);
    v0 &= 1;
    if (v0 == 0) goto loc_80046428;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF38);                                // Load from: 8007F0C8
    v0 += s1;
    v0 = lbu(v0);
    if (v0 != 0) goto loc_80046428;
    PSX_voiceparmoff();
loc_80046428:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF38);                                // Load from: 8007F0C8
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF3C);                                // Load from: 8007F0C4
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF38);                                 // Store to: 8007F0C8
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF34);                                // Load from: 8007F0CC
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF3C);                                 // Store to: 8007F0C4
    v0 += 0x18;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF34);                                 // Store to: 8007F0CC
    if (v1 != s0) goto loc_800463E0;
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PSX_DriverEntry2() noexcept {
loc_80046484:
    return;
}

void PSX_DriverEntry3() noexcept {
loc_8004648C:
    return;
}

void PSX_TrkOff() noexcept {
loc_80046494:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lbu(s0 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE98);                                // Load from: gWess_Dvr_pss (8007F168)
    v0 <<= 3;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF2C);                                 // Store to: 8007F0D4
    PSX_TrkMute();
    v0 = lbu(s0 + 0x10);
    if (v0 == 0) goto loc_80046524;
    v0 = lw(s0);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF2C);                                // Load from: 8007F0D4
    v0 |= 0x88;
    sw(v0, s0);
    v0 = lbu(v1 + 0x5);
    v0--;
    sb(v0, v1 + 0x5);
    v0 &= 0xFF;
    if (v0 != 0) goto loc_8004652C;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF2C);                                // Load from: 8007F0D4
    sb(0, v0 + 0x1);
    goto loc_8004652C;
loc_80046524:
    a0 = s0;
    Eng_TrkOff();
loc_8004652C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSX_TrkMute() noexcept {
loc_80046540:
    sp -= 0x28;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    v0 = lbu(s0 + 0x10);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF28);                                 // Store to: 8007F0D8
    if (v0 == 0) goto loc_800466E4;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF20);                                 // Store to: 8007F0E0
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF24);                                 // Store to: 8007F0DC
    s1 = 1;                                             // Result = 00000001
    if (v1 == v0) goto loc_800466E4;
loc_8004659C:
    t0 = 0x80080000;                                    // Result = 80080000
    t0 = lw(t0 - 0xF20);                                // Load from: 8007F0E0
    a0 = lw(t0);
    v0 = a0 & 1;
    if (v0 == 0) goto loc_800466B0;
    v1 = lbu(t0 + 0x3);
    v0 = lbu(s0 + 0x1);
    if (v1 != v0) goto loc_800466B0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A10);                               // Load from: gpWess_pnotestate (80075A10)
    {
        const bool bJump = (v0 == 0);
        v0 = a0 & 2;
        if (bJump) goto loc_80046648;
    }
    if (v0 != 0) goto loc_80046648;
    v0 = lbu(s0 + 0x13);
    v1 = 0x1F;                                          // Result = 0000001F
    if (v0 != s1) goto loc_8004664C;
    v1 = lbu(s0 + 0x2);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE98);                                // Load from: gWess_Dvr_pss (8007F168)
    v0 <<= 3;
    v0 += v1;
    v1 = lw(t0 + 0x8);
    a0 = lh(v0 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF1C);                                 // Store to: 8007F0E4
    a1 = lbu(t0 + 0x3);
    a2 = lbu(t0 + 0x5);
    a3 = lbu(t0 + 0x6);
    sw(v1, sp + 0x10);
    v0 = lw(t0 + 0xC);
    sw(v0, sp + 0x14);
    add_music_mute_note();
loc_80046648:
    v1 = 0x1F;                                          // Result = 0000001F
loc_8004664C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A07);                              // Load from: gWess_UNKNOWN_status_byte (80075A07)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF20);                                // Load from: 8007F0E0
    v1 -= v0;
    v0 = 0x10000000;                                    // Result = 10000000
    v0 = i32(v0) >> v1;
    sw(v0, a0 + 0x14);
    PSX_voicerelease();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF20);                                // Load from: 8007F0E0
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF28);                                // Load from: 8007F0D8
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A0C);                               // Load from: 80075A0C
    v0 = lbu(v0 + 0x2);
    a0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0xF28);                                 // Store to: 8007F0D8
    v0 = s1 << v0;
    v0 |= v1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5A0C);                                // Store to: 80075A0C
    if (a0 == 0) goto loc_800466E4;
loc_800466B0:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF20);                                // Load from: 8007F0E0
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF24);                                // Load from: 8007F0DC
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF20);                                 // Store to: 8007F0E0
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF24);                                 // Store to: 8007F0DC
    if (v1 != v0) goto loc_8004659C;
loc_800466E4:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void PSX_PatchChg() noexcept {
loc_800466FC:
    v1 = lw(a0 + 0x34);
    v0 = lbu(v1 + 0x2);
    v1 = lbu(v1 + 0x1);
    v0 <<= 8;
    v1 |= v0;
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at - 0xF18);                                 // Store to: gWess_Dvr_thepatch (8007F0E8)
    sh(v1, a0 + 0xA);
    return;
}

void PSX_PatchMod() noexcept {
loc_80046724:
    return;
}

void PSX_PitchMod() noexcept {
loc_8004672C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x34);
    v1 = lbu(v0 + 0x2);
    v0 = lbu(v0 + 0x1);
    v1 <<= 8;
    v0 |= v1;
    a0 = v0;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xEFC);                                 // Store to: 8007F104
    v0 <<= 16;
    v1 = lh(s0 + 0xE);
    v0 = u32(i32(v0) >> 16);
    if (v1 == v0) goto loc_80046960;
    v0 = lbu(s0 + 0x10);
    sh(a0, s0 + 0xE);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF14);                                 // Store to: 8007F0EC
    if (v0 == 0) goto loc_80046960;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF00);                                 // Store to: 8007F100
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF10);                                 // Store to: 8007F0F0
    s2 = 0x20;                                          // Result = 00000020
    if (v1 == v0) goto loc_80046960;
    s1 = 0x80080000;                                    // Result = 80080000
    s1 -= 0xE70;                                        // Result = 8007F190
loc_800467CC:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xF00);                                // Load from: 8007F100
    v0 = lw(a0);
    v0 &= 1;
    if (v0 == 0) goto loc_8004692C;
    v1 = lbu(a0 + 0x3);
    v0 = lbu(s0 + 0x1);
    {
        const bool bJump = (v1 != v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8004692C;
    }
    v1 = lbu(a0 + 0x2);
    sw(s2, s1 + 0x4);                                   // Store to: 8007F194
    v0 = v0 << v1;
    sw(v0, s1);                                         // Store to: 8007F190
    v1 = lh(s0 + 0xE);
    if (v1 != 0) goto loc_8004682C;
    v0 = lbu(a0 + 0x5);
    v0 <<= 8;
    goto loc_800468FC;
loc_8004682C:
    if (i32(v1) <= 0) goto loc_80046888;
    v0 = lw(a0 + 0x8);
    v0 = lb(v0 + 0x9);
    mult(v1, v0);
    v0 = lo;
    v0 += 0x20;
    v1 = u32(i32(v0) >> 13);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF0C);                                 // Store to: 8007F0F4
    v0 &= 0x1FFF;
    v0 = u32(i32(v0) >> 6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF08);                                 // Store to: 8007F0F8
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF04);                                 // Store to: 8007F0FC
    v0 = lbu(a0 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF08);                               // Load from: 8007F0F8
    v0 += v1;
    goto loc_800468E8;
loc_80046888:
    v0 = lw(a0 + 0x8);
    v0 = lb(v0 + 0x8);
    mult(v1, v0);
    v1 = lo;
    v1 = s2 - v1;
    v0 = u32(i32(v1) >> 13);
    v0++;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF0C);                                 // Store to: 8007F0F4
    v1 &= 0x1FFF;
    v1 = u32(i32(v1) >> 6);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF08);                                 // Store to: 8007F0F8
    v0 = 0x80;                                          // Result = 00000080
    v0 -= v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF04);                                 // Store to: 8007F0FC
    v0 = lbu(a0 + 0x5);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF08);                               // Load from: 8007F0F8
    v0 -= v1;
loc_800468E8:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lhu(v1 - 0xF04);                               // Load from: 8007F0FC
    v0 <<= 8;
    v1 &= 0x7F;
    v0 |= v1;
loc_800468FC:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE70;                                        // Result = 8007F190
    sh(v0, s1 + 0x16);                                  // Store to: 8007F1A6
    LIBSPU_SpuSetVoiceAttr(*vmAddrToPtr<SpuVoiceAttr>(a0));
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF14);                                // Load from: 8007F0EC
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF14);                                 // Store to: 8007F0EC
    if (v0 == 0) goto loc_80046960;
loc_8004692C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF00);                                // Load from: 8007F100
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xF10);                                // Load from: 8007F0F0
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF00);                                 // Store to: 8007F100
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF10);                                 // Store to: 8007F0F0
    if (v1 != v0) goto loc_800467CC;
loc_80046960:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PSX_ZeroMod() noexcept {
loc_8004697C:
    return;
}

void PSX_ModuMod() noexcept {
loc_80046984:
    return;
}

void PSX_VolumeMod() noexcept {
loc_8004698C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x34);
    v1 = lbu(s0 + 0x10);
    v0 = lbu(v0 + 0x1);
    sb(v0, s0 + 0xC);
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEF8);                                 // Store to: 8007F108
    if (v1 == 0) goto loc_80046C88;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEEC);                                 // Store to: 8007F114
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEF4);                                 // Store to: 8007F10C
    s2 = 1;                                             // Result = 00000001
    if (v1 == v0) goto loc_80046C88;
    s1 = 0x80080000;                                    // Result = 80080000
    s1 -= 0xE70;                                        // Result = 8007F190
loc_80046A00:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEEC);                                // Load from: 8007F114
    v0 = lw(a0);
    v0 &= 1;
    if (v0 == 0) goto loc_80046C54;
    v1 = lbu(a0 + 0x3);
    v0 = lbu(s0 + 0x1);
    if (v1 != v0) goto loc_80046C54;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    {
        const bool bJump = (v0 == 0);
        v0 = 0x40;                                      // Result = 00000040
        if (bJump) goto loc_80046AA8;
    }
    v0 = lw(a0 + 0x8);
    v1 = lbu(v0 + 0x3);
    v0 = lbu(s0 + 0xD);
    v1 <<= 24;
    v1 = u32(i32(v1) >> 24);
    v0 += v1;
    v0 -= 0x40;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xEE8);                                 // Store to: 8007F118
    v0 = (i32(v0) < 0x80);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x7F;                                      // Result = 0000007F
        if (bJump) goto loc_80046A84;
    }
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xEE8);                                 // Store to: 8007F118
loc_80046A84:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lh(v0 - 0xEE8);                                // Load from: 8007F118
    if (i32(v0) >= 0) goto loc_80046AB0;
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xEE8);                                  // Store to: 8007F118
    goto loc_80046AB0;
loc_80046AA8:
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xEE8);                                 // Store to: 8007F118
loc_80046AB0:
    v0 = lbu(s0 + 0x13);
    if (v0 != 0) goto loc_80046B04;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEEC);                                // Load from: 8007F114
    v1 = lw(v0 + 0x8);
    a0 = lbu(v0 + 0x6);
    v0 = lbu(v1 + 0x2);
    mult(a0, v0);
    v1 = lo;
    v0 = lbu(s0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A04);                              // Load from: gWess_master_sfx_volume (80075A04)
    mult(v1, v0);
    goto loc_80046B48;
loc_80046B04:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEEC);                                // Load from: 8007F114
    v1 = lw(v0 + 0x8);
    a0 = lbu(v0 + 0x6);
    v0 = lbu(v1 + 0x2);
    mult(a0, v0);
    v1 = lo;
    v0 = lbu(s0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A05);                              // Load from: gWess_master_mus_volume (80075A05)
    mult(v1, v0);
loc_80046B48:
    v0 = lo;
    v0 = u32(i32(v0) >> 21);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEF0);                                 // Store to: 8007F110
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEEC);                                // Load from: 8007F114
    v1 = lbu(v0 + 0x2);
    v0 = 3;                                             // Result = 00000003
    sw(v0, s1 + 0x4);                                   // Store to: 8007F194
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    v1 = s2 << v1;
    sw(v1, s1);                                         // Store to: 8007F190
    if (v0 != 0) goto loc_80046BA0;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lhu(v0 - 0xEF0);                               // Load from: 8007F110
    v0 <<= 6;
    sh(v0, s1 + 0x8);                                   // Store to: 8007F198
    sh(v0, s1 + 0xA);                                   // Store to: 8007F19A
    goto loc_80046C24;
loc_80046BA0:
    {
        const bool bJump = (v0 != s2);
        v0 = 0x80;                                      // Result = 00000080
        if (bJump) goto loc_80046BE8;
    }
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEF0);                                // Load from: 8007F110
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xEE8);                                // Load from: 8007F118
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0x8);                                   // Store to: 8007F198
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0xA);                                   // Store to: 8007F19A
    goto loc_80046C24;
loc_80046BE8:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEF0);                                // Load from: 8007F110
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xEE8);                                // Load from: 8007F118
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0xA);                                   // Store to: 8007F19A
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0x8);                                   // Store to: 8007F198
loc_80046C24:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE70;                                        // Result = 8007F190
    LIBSPU_SpuSetVoiceAttr(*vmAddrToPtr<SpuVoiceAttr>(a0));
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEF8);                                // Load from: 8007F108
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEF8);                                 // Store to: 8007F108
    if (v0 == 0) goto loc_80046C88;
loc_80046C54:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEEC);                                // Load from: 8007F114
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xEF4);                                // Load from: 8007F10C
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEEC);                                 // Store to: 8007F114
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEF4);                                 // Store to: 8007F10C
    if (v1 != v0) goto loc_80046A00;
loc_80046C88:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PSX_PanMod() noexcept {
loc_80046CA4:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x34);
    v0 = lbu(v0 + 0x1);
    sb(v0, s0 + 0xD);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    if (v0 == 0) goto loc_80046F64;
    v0 = lbu(s0 + 0x10);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEE4);                                 // Store to: 8007F11C
    if (v0 == 0) goto loc_80046F64;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xED8);                                 // Store to: 8007F128
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEE0);                                 // Store to: 8007F120
    s2 = 1;                                             // Result = 00000001
    if (v1 == v0) goto loc_80046F64;
    s1 = 0x80080000;                                    // Result = 80080000
    s1 -= 0xE70;                                        // Result = 8007F190
loc_80046D34:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xED8);                                // Load from: 8007F128
    v0 = lw(a0);
    v0 &= 1;
    if (v0 == 0) goto loc_80046F30;
    v1 = lbu(a0 + 0x3);
    v0 = lbu(s0 + 0x1);
    if (v1 != v0) goto loc_80046F30;
    v0 = lw(a0 + 0x8);
    v1 = lbu(v0 + 0x3);
    v0 = lbu(s0 + 0xD);
    v1 <<= 24;
    v1 = u32(i32(v1) >> 24);
    v0 += v1;
    v0 -= 0x40;
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xED4);                                 // Store to: 8007F12C
    v0 = (i32(v0) < 0x80);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x7F;                                      // Result = 0000007F
        if (bJump) goto loc_80046DA4;
    }
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xED4);                                 // Store to: 8007F12C
loc_80046DA4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lh(v0 - 0xED4);                                // Load from: 8007F12C
    if (i32(v0) >= 0) goto loc_80046DC0;
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xED4);                                  // Store to: 8007F12C
loc_80046DC0:
    v0 = lbu(s0 + 0x13);
    if (v0 != 0) goto loc_80046E08;
    v0 = lw(a0 + 0x8);
    v1 = lbu(a0 + 0x6);
    v0 = lbu(v0 + 0x2);
    mult(v1, v0);
    v1 = lo;
    v0 = lbu(s0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A04);                              // Load from: gWess_master_sfx_volume (80075A04)
    mult(v1, v0);
    goto loc_80046E40;
loc_80046E08:
    v0 = lw(a0 + 0x8);
    v1 = lbu(a0 + 0x6);
    v0 = lbu(v0 + 0x2);
    mult(v1, v0);
    v1 = lo;
    v0 = lbu(s0 + 0xC);
    mult(v1, v0);
    v1 = lo;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A05);                              // Load from: gWess_master_mus_volume (80075A05)
    mult(v1, v0);
loc_80046E40:
    v0 = lo;
    v0 = u32(i32(v0) >> 21);
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEDC);                                 // Store to: 8007F124
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xED8);                                // Load from: 8007F128
    v1 = lbu(v0 + 0x2);
    v0 = 3;                                             // Result = 00000003
    sw(v0, s1 + 0x4);                                   // Store to: 8007F194
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    v1 = s2 << v1;
    sw(v1, s1);                                         // Store to: 8007F190
    if (v0 != s2) goto loc_80046EC0;
    v0 = 0x80;                                          // Result = 00000080
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEDC);                                // Load from: 8007F124
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xED4);                                // Load from: 8007F12C
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0x8);                                   // Store to: 8007F198
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0xA);                                   // Store to: 8007F19A
    goto loc_80046F00;
loc_80046EC0:
    v0 = 0x80;                                          // Result = 00000080
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEDC);                                // Load from: 8007F124
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lh(v1 - 0xED4);                                // Load from: 8007F12C
    a0 <<= 7;
    v0 -= v1;
    mult(a0, v0);
    v0 = lo;
    v1++;
    mult(a0, v1);
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0xA);                                   // Store to: 8007F19A
    v0 = lo;
    v0 = u32(i32(v0) >> 7);
    sh(v0, s1 + 0x8);                                   // Store to: 8007F198
loc_80046F00:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0xE70;                                        // Result = 8007F190
    LIBSPU_SpuSetVoiceAttr(*vmAddrToPtr<SpuVoiceAttr>(a0));
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEE4);                                // Load from: 8007F11C
    v0--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEE4);                                 // Store to: 8007F11C
    if (v0 == 0) goto loc_80046F64;
loc_80046F30:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xED8);                                // Load from: 8007F128
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xEE0);                                // Load from: 8007F120
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xED8);                                 // Store to: 8007F128
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEE0);                                 // Store to: 8007F120
    if (v1 != v0) goto loc_80046D34;
loc_80046F64:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PSX_PedalMod() noexcept {
loc_80046F80:
    return;
}

void PSX_ReverbMod() noexcept {
loc_80046F88:
    return;
}

void PSX_ChorusMod() noexcept {
loc_80046F90:
    return;
}

void PSX_voiceon() noexcept {
loc_80046F98:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = lw(a0);
    v1 = -3;                                            // Result = FFFFFFFD
    v0 |= 1;
    v0 &= v1;
    sw(v0, a0);
    v0 = lbu(a1 + 0x1);
    t0 = lbu(sp + 0x28);
    t1 = lbu(sp + 0x2C);
    sb(v0, a0 + 0x3);
    v0 = lbu(a1 + 0x8);
    sb(t0, a0 + 0x5);
    sb(t1, a0 + 0x6);
    sb(0, a0 + 0x7);
    sb(v0, a0 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE84);                                // Load from: 8007F17C
    sw(a2, a0 + 0x8);
    sw(a3, a0 + 0xC);
    v0 = lw(v0);
    sw(v0, a0 + 0x10);
    v1 = lhu(a2 + 0xE);
    v0 = v1 & 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 0x1F;
        if (bJump) goto loc_80047018;
    }
    v1 = 0x1F;                                          // Result = 0000001F
    v1 -= v0;
    v0 = 0x10000000;                                    // Result = 10000000
    goto loc_80047024;
loc_80047018:
    v1 = 0x1F;                                          // Result = 0000001F
    v1 -= v0;
    v0 = 0x5DC0000;                                     // Result = 05DC0000
loc_80047024:
    v0 = i32(v0) >> v1;
    sw(v0, a0 + 0x14);
    v0 = lbu(a1 + 0x10);
    v0++;
    sb(v0, a1 + 0x10);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE9C);                                // Load from: 8007F164
    a2 = t1;
    v0 = lbu(v1 + 0x6);
    a1 = t0;
    v0++;
    sb(v0, v1 + 0x6);
    TriggerPSXVoice();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSX_voiceparmoff() noexcept {
loc_8004706C:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v1 = lbu(s0 + 0x3);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xE9C);                                // Load from: 8007F164
    v0 = v1 << 2;
    v0 += v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE94);                                // Load from: 8007F16C
    v0 <<= 4;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xED0);                                 // Store to: 8007F130
    v0 = lbu(a0 + 0x6);
    v0--;
    sb(v0, a0 + 0x6);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xED0);                                // Load from: 8007F130
    v0 = lbu(v1 + 0x10);
    v0--;
    sb(v0, v1 + 0x10);
    v0 &= 0xFF;
    if (v0 != 0) goto loc_80047108;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xED0);                                // Load from: 8007F130
    v0 = lw(a0);
    v0 &= 0x80;
    if (v0 == 0) goto loc_80047108;
    Eng_TrkOff();
loc_80047108:
    v0 = lw(s0);
    v1 = -2;                                            // Result = FFFFFFFE
    v0 &= v1;
    v1 = -3;                                            // Result = FFFFFFFD
    v0 &= v1;
    sw(v0, s0);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSX_voicerelease() noexcept {
loc_80047134:
    v0 = 1;                                             // Result = 00000001
    v1 = lbu(a0 + 0x2);
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5A08);                               // Load from: 80075A08
    v0 = v0 << v1;
    v0 |= a1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5A08);                                // Store to: 80075A08
    v0 = lw(a0);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE84);                                // Load from: 8007F17C
    v0 |= 2;
    sw(v0, a0);
    v0 = lw(v1);
    v1 = lw(a0 + 0x14);
    v0 += v1;
    sw(v0, a0 + 0x10);
    return;
}

void PSX_voicenote() noexcept {
loc_80047180:
    sp -= 0x30;
    sw(s4, sp + 0x28);
    s4 = lbu(sp + 0x40);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(s2, sp + 0x20);
    s2 = a1;
    sw(s3, sp + 0x24);
    s3 = a2;
    sw(s1, sp + 0x1C);
    sw(ra, sp + 0x2C);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xEC8);                                  // Store to: 8007F138
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEC4);                                 // Store to: 8007F13C
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xECC);                                 // Store to: 8007F134
    s1 = a3;
    if (v1 == v0) goto loc_80047328;
    a2 = 1;                                             // Result = 00000001
loc_800471E8:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEC4);                                // Load from: 8007F13C
    a1 = lw(a0);
    v0 = a1 & 1;
    a3 = s3;
    if (v0 != 0) goto loc_80047230;
    a1 = s0;
    a2 = s2;
    v0 = s1 & 0xFF;
    sw(v0, sp + 0x10);
    sw(s4, sp + 0x14);
    PSX_voiceon();
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xEC8);                                  // Store to: 8007F138
    goto loc_80047328;
loc_80047230:
    v1 = lbu(a0 + 0x4);
    v0 = lbu(s0 + 0x8);
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_800472F4;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A18);                               // Load from: 80075A18
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = a1 & 2;
        if (bJump) goto loc_800472CC;
    }
    if (v0 == 0) goto loc_80047290;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A14);                               // Load from: 80075A14
    v0 = lw(v0);
    v0 &= 2;
    if (v0 == 0) goto loc_800472CC;
    goto loc_800472B0;
loc_80047290:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5A14);                               // Load from: 80075A14
    v0 = lw(v0);
    v0 &= 2;
    if (v0 != 0) goto loc_800472F4;
loc_800472B0:
    v0 = lw(a0 + 0x10);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5A1C);                               // Load from: 80075A1C
    v0 = (v0 < v1);
    if (v0 == 0) goto loc_800472F4;
loc_800472CC:
    at = 0x80080000;                                    // Result = 80080000
    sw(a2, at - 0xEC8);                                 // Store to: 8007F138
    v0 = lbu(a0 + 0x4);
    v1 = lw(a0 + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5A14);                                // Store to: 80075A14
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5A18);                                // Store to: 80075A18
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5A1C);                                // Store to: 80075A1C
loc_800472F4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEC4);                                // Load from: 8007F13C
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xECC);                                // Load from: 8007F134
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEC4);                                 // Store to: 8007F13C
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xECC);                                 // Store to: 8007F134
    if (v1 != v0) goto loc_800471E8;
loc_80047328:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEC8);                                // Load from: 8007F138
    if (v0 == 0) goto loc_80047370;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5A14);                               // Load from: 80075A14
    PSX_voiceparmoff();
    a1 = s0;
    a2 = s2;
    a3 = s3;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5A14);                               // Load from: 80075A14
    v0 = s1 & 0xFF;
    sw(v0, sp + 0x10);
    sw(s4, sp + 0x14);
    PSX_voiceon();
loc_80047370:
    ra = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void PSX_NoteOn() noexcept {
loc_80047394:
    sp -= 0x20;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x1C);
    v0 = lw(s0 + 0x34);
    v1 = lw(s0 + 0x34);
    v0 = lbu(v0 + 0x1);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xEBC);                                 // Store to: 8007F144
    v0 = lbu(v1 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xEB8);                                 // Store to: 8007F148
    v1 = lbu(s0 + 0x13);
    v0 = 2;                                             // Result = 00000002
    if (v1 != v0) goto loc_80047428;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xEBC);                               // Load from: 8007F144
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE74);                                // Load from: 8007F18C
    v1 <<= 2;
    v1 += v0;
    v0 = lh(v1);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xE80);                                // Load from: 8007F180
    v0 <<= 2;
    v0 += a0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEB0);                                 // Store to: 8007F150
    v0 = lbu(v1 + 0x2);
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xEBC);                                 // Store to: 8007F144
    goto loc_80047444;
loc_80047428:
    v0 = lh(s0 + 0xA);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE80);                                // Load from: 8007F180
    v0 <<= 2;
    v0 += v1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEB0);                                 // Store to: 8007F150
loc_80047444:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEB0);                                // Load from: 8007F150
    v1 = lbu(v0);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xEC0);                                  // Store to: 8007F140
    v0 = v1 - 1;
    at = 0x80080000;                                    // Result = 80080000
    sb(v1, at - 0xEB4);                                 // Store to: 8007F14C
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xEB4);                                 // Store to: 8007F14C
    v0 &= 0xFF;
    v1 = 0xFF;                                          // Result = 000000FF
    if (v0 == v1) goto loc_80047564;
loc_80047480:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEB0);                                // Load from: 8007F150
    v1 = lh(v0 + 0x2);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEC0);                                // Load from: 8007F140
    v1 += v0;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE7C);                                // Load from: 8007F184
    v1 <<= 4;
    a1 = v1 + v0;
    v1 = lh(a1 + 0xA);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE78);                                // Load from: 8007F188
    v0 <<= 2;
    a2 = v0 + v1;
    v0 = lw(a2 + 0x8);
    at = 0x80080000;                                    // Result = 80080000
    sw(a1, at - 0xEAC);                                 // Store to: 8007F154
    at = 0x80080000;                                    // Result = 80080000
    sw(a2, at - 0xEA8);                                 // Store to: 8007F158
    if (v0 == 0) goto loc_8004752C;
    v0 = lbu(a1 + 0x6);
    a3 = 0x80080000;                                    // Result = 80080000
    a3 = lbu(a3 - 0xEBC);                               // Load from: 8007F144
    v0 = (a3 < v0);
    if (v0 != 0) goto loc_8004752C;
    v0 = lbu(a1 + 0x7);
    v0 = (v0 < a3);
    a0 = s0;
    if (v0 != 0) goto loc_8004752C;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xEB8);                               // Load from: 8007F148
    sw(v0, sp + 0x10);
    PSX_voicenote();
loc_8004752C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lbu(v0 - 0xEB4);                               // Load from: 8007F14C
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xEC0);                                // Load from: 8007F140
    v0--;
    v1++;
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xEB4);                                 // Store to: 8007F14C
    v0 &= 0xFF;
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEC0);                                 // Store to: 8007F140
    v1 = 0xFF;                                          // Result = 000000FF
    if (v0 != v1) goto loc_80047480;
loc_80047564:
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void PSX_NoteOff() noexcept {
loc_80047578:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xE90);                                // Load from: 8007F170
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xE8C);                                // Load from: 8007F174
    sp -= 0x20;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEA0);                                 // Store to: 8007F160
    v0 = -1;                                            // Result = FFFFFFFF
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEA4);                                 // Store to: 8007F15C
    s0 = a0;
    if (v1 == v0) goto loc_80047648;
    s2 = 1;                                             // Result = 00000001
    s1 = -1;                                            // Result = FFFFFFFF
loc_800475C4:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0xEA0);                                // Load from: 8007F160
    v0 = lw(a0);
    v0 &= 3;
    if (v0 != s2) goto loc_80047618;
    v0 = lw(s0 + 0x34);
    v1 = lbu(a0 + 0x5);
    v0 = lbu(v0 + 0x1);
    if (v1 != v0) goto loc_80047618;
    v1 = lbu(a0 + 0x3);
    v0 = lbu(s0 + 0x1);
    if (v1 != v0) goto loc_80047618;
    PSX_voicerelease();
loc_80047618:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xEA0);                                // Load from: 8007F160
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0xEA4);                                // Load from: 8007F15C
    v0 += 0x18;
    v1--;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xEA0);                                 // Store to: 8007F160
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xEA4);                                 // Store to: 8007F15C
    if (v1 != s1) goto loc_800475C4;
loc_80047648:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
