#include "PSXSPU.h"

#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBSPU.h"

void psxspu_init_reverb() noexcept {
loc_80045328:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s0, sp + 0x10);
    s0 = 0x80080000;                                    // Result = 80080000
    s0 -= 0xF80;                                        // Result = 8007F080
    v1 = lw(sp + 0x30);
    v0 = 0x1F;                                          // Result = 0000001F
    sw(ra, sp + 0x18);
    sw(v0, s0);                                         // Store to: 8007F080
    v0 = s1 | 0x100;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF7C);                                 // Store to: 8007F084
    at = 0x80080000;                                    // Result = 80080000
    sh(a1, at - 0xF78);                                 // Store to: 8007F088
    at = 0x80080000;                                    // Result = 80080000
    sh(a2, at - 0xF76);                                 // Store to: 8007F08A
    at = 0x80080000;                                    // Result = 80080000
    sw(a3, at - 0xF74);                                 // Store to: 8007F08C
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0xF70);                                 // Store to: 8007F090
    a0 = s0;                                            // Result = 8007F080
    LIBSPU_SpuSetReverbModeParam();
    a0 = s0;                                            // Result = 8007F080
    LIBSPU_SpuSetReverbDepth();
    if (s1 != 0) goto loc_800453BC;
    a0 = 0;                                             // Result = 00000000
    LIBSPU_SpuSetReverb();
    v0 = 0x70000;                                       // Result = 00070000
    v0 |= 0xF000;                                       // Result = 0007F000
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x598C);                                // Store to: 8007598C
    a0 = 0;                                             // Result = 00000000
    goto loc_800453D8;
loc_800453BC:
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuSetReverb();
    LIBSPU_SpuGetReverbOffsetAddr();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x598C);                                // Store to: 8007598C
    a0 = 1;                                             // Result = 00000001
loc_800453D8:
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    LIBSPU_SpuSetReverbVoice();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void psxspu_set_reverb_depth() noexcept {
    sp -= 0x18;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0xF78;                                        // Result = 8007F088
    sw(ra, sp + 0x10);
    sh(a0, v0);                                         // Store to: 8007F088
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80080000;                                    // Result = 80080000
    sh(a1, at - 0xF76);                                 // Store to: 8007F08A
    a0 = v0 - 8;                                        // Result = 8007F080
    LIBSPU_SpuSetReverbDepth();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxspu_init() noexcept {
loc_80045450:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5984);                               // Load from: 80075984
    sp -= 0x48;
    sw(ra, sp + 0x44);
    sw(s0, sp + 0x40);
    if (v0 != 0) goto loc_800454E8;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    s0 = 1;                                             // Result = 00000001
    LIBSPU_SpuInit();
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 -= 0x6AF8;                                       // Result = 800A9508
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5984);                                // Store to: 80075984
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuInitMalloc();
    a0 = 0;                                             // Result = 00000000
    LIBSPU_SpuSetTransferMode();
    a0 = 0;                                             // Result = 00000000
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    sw(0, sp + 0x10);
    psxspu_init_reverb();
    a0 = sp + 0x18;
    v0 = 0x3C3;                                         // Result = 000003C3
    sw(v0, sp + 0x18);
    v0 = 0x3FFF;                                        // Result = 00003FFF
    sh(v0, sp + 0x1C);
    sh(v0, sp + 0x1E);
    v0 = 0x3CFF;                                        // Result = 00003CFF
    sh(v0, sp + 0x28);
    sh(v0, sp + 0x2A);
    sw(0, sp + 0x2C);
    sw(s0, sp + 0x30);
    LIBSPU_SpuSetCommonAttr();
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5988);                                // Store to: 80075988
loc_800454E8:
    ra = lw(sp + 0x44);
    s0 = lw(sp + 0x40);
    sp += 0x48;
    return;
}

void psxspu_update_master_vol() noexcept {
loc_800454FC:
    sp -= 0x40;
    v0 = 3;                                             // Result = 00000003
    sh(a0, sp + 0x14);
    sh(a0, sp + 0x16);
    a0 = sp + 0x10;
    sw(ra, sp + 0x38);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    sw(v0, sp + 0x10);
    LIBSPU_SpuSetCommonAttr();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x38);
    sp += 0x40;
    return;
}

void psxspu_update_master_vol_mode() noexcept {
loc_80045540:
    sp -= 0x40;
    v0 = 0xC0;                                          // Result = 000000C0
    sh(a0, sp + 0x20);
    sh(a0, sp + 0x22);
    a0 = sp + 0x10;
    sw(ra, sp + 0x38);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    sw(v0, sp + 0x10);
    LIBSPU_SpuSetCommonAttr();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x38);
    sp += 0x40;
    return;
}

void psxspu_setcdmixon() noexcept {
loc_80045584:
    sp -= 0x40;
    v0 = 0x200;                                         // Result = 00000200
    sw(s0, sp + 0x38);
    s0 = 1;                                             // Result = 00000001
    a0 = sp + 0x10;
    sw(ra, sp + 0x3C);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    sw(v0, sp + 0x10);
    sw(s0, sp + 0x28);
    LIBSPU_SpuSetCommonAttr();
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x3C);
    s0 = lw(sp + 0x38);
    sp += 0x40;
    return;
}

void psxspu_setcdmixoff() noexcept {
loc_800455CC:
    sp -= 0x40;
    v0 = 0x200;                                         // Result = 00000200
    a0 = sp + 0x10;
    sw(ra, sp + 0x38);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    sw(v0, sp + 0x10);
    sw(0, sp + 0x28);
    LIBSPU_SpuSetCommonAttr();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x38);
    sp += 0x40;
    return;
}

void psxspu_fadeengine() noexcept {
loc_8004560C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5988);                               // Load from: 80075988
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_80045710;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x59A8);                               // Load from: 800759A8
    {
        const bool bJump = (i32(v1) <= 0);
        v1--;
        if (bJump) goto loc_80045698;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59AC);                               // Load from: 800759AC
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x59B4);                               // Load from: 800759B4
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x59A8);                                // Store to: 800759A8
    v0 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59AC);                                // Store to: 800759AC
    if (v1 != 0) goto loc_80045674;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59B0);                               // Load from: 800759B0
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59AC);                                // Store to: 800759AC
loc_80045674:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lh(v0 + 0x59AE);                               // Load from: 800759AE
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59A4);                                // Store to: 800759A4
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lh(a0 + 0x59A4);                               // Load from: 800759A4
    psxspu_update_master_vol_mode();
loc_80045698:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5994);                               // Load from: 80075994
    {
        const bool bJump = (i32(v1) <= 0);
        v1--;
        if (bJump) goto loc_80045710;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5998);                               // Load from: 80075998
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x59A0);                               // Load from: 800759A0
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5994);                                // Store to: 80075994
    v0 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5998);                                // Store to: 80075998
    if (v1 != 0) goto loc_800456EC;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x599C);                               // Load from: 8007599C
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5998);                                // Store to: 80075998
loc_800456EC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lh(v0 + 0x599A);                               // Load from: 8007599A
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5990);                                // Store to: 80075990
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lh(a0 + 0x5990);                               // Load from: 80075990
    psxspu_update_master_vol();
loc_80045710:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxspu_set_cd_vol() noexcept {
loc_80045720:
    sp -= 0x18;
    v0 = a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59A4);                                // Store to: 800759A4
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lh(a0 + 0x59A4);                               // Load from: 800759A4
    v0 <<= 16;
    sw(ra, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59AC);                                // Store to: 800759AC
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x59A8);                                 // Store to: 800759A8
    psxspu_update_master_vol_mode();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxspu_get_cd_vol() noexcept {
loc_8004577C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59A4);                               // Load from: 800759A4
    return;
}

void psxspu_start_cd_fade() noexcept {
loc_8004578C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x594C);                               // Load from: gbWess_WessTimerActive (8007594C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    v1 = 0x10620000;                                    // Result = 10620000
    if (v0 == 0) goto loc_80045828;
    v1 |= 0x4DD3;                                       // Result = 10624DD3
    v0 = a0 << 4;
    v0 -= a0;
    v0 <<= 3;
    mult(v0, v1);
    v0 = u32(i32(v0) >> 31);
    a0 = a1 << 16;
    v1 = hi;
    v1 = u32(i32(v1) >> 6);
    v1 -= v0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59AC);                               // Load from: 800759AC
    v1++;
    v0 = a0 - v0;
    div(v0, v1);
    if (v1 != 0) goto loc_800457EC;
    _break(0x1C00);
loc_800457EC:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80045804;
    }
    if (v0 != at) goto loc_80045804;
    _break(0x1800);
loc_80045804:
    v0 = lo;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x59B0);                                // Store to: 800759B0
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x59A8);                                // Store to: 800759A8
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59B4);                                // Store to: 800759B4
    v0 = 1;                                             // Result = 00000001
    goto loc_80045834;
loc_80045828:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x59A8);                                 // Store to: 800759A8
    v0 = 1;                                             // Result = 00000001
loc_80045834:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    return;
}

void psxspu_stop_cd_fade() noexcept {
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x59A8);                                 // Store to: 800759A8
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    return;
}

void psxspu_get_cd_fade_status() noexcept {
loc_80045868:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x59A8);                               // Load from: 800759A8
    v0 = (i32(v0) < 2);
    v0 ^= 1;
    return;
}

void psxspu_set_master_vol() noexcept {
loc_80045880:
    sp -= 0x18;
    v0 = a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5990);                                // Store to: 80075990
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lh(a0 + 0x5990);                               // Load from: 80075990
    v0 <<= 16;
    sw(ra, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5998);                                // Store to: 80075998
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5994);                                 // Store to: 80075994
    psxspu_update_master_vol();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxspu_get_master_vol() noexcept {
loc_800458DC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5990);                               // Load from: 80075990
    return;
}

void psxspu_start_master_fade() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x594C);                               // Load from: gbWess_WessTimerActive (8007594C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    v1 = 0x10620000;                                    // Result = 10620000
    if (v0 == 0) goto loc_80045988;
    v1 |= 0x4DD3;                                       // Result = 10624DD3
    v0 = a0 << 4;
    v0 -= a0;
    v0 <<= 3;
    mult(v0, v1);
    v0 = u32(i32(v0) >> 31);
    a0 = a1 << 16;
    v1 = hi;
    v1 = u32(i32(v1) >> 6);
    v1 -= v0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5998);                               // Load from: 80075998
    v1++;
    v0 = a0 - v0;
    div(v0, v1);
    if (v1 != 0) goto loc_8004594C;
    _break(0x1C00);
loc_8004594C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80045964;
    }
    if (v0 != at) goto loc_80045964;
    _break(0x1800);
loc_80045964:
    v0 = lo;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x599C);                                // Store to: 8007599C
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5994);                                // Store to: 80075994
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x59A0);                                // Store to: 800759A0
    v0 = 1;                                             // Result = 00000001
    goto loc_80045994;
loc_80045988:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5994);                                 // Store to: 80075994
    v0 = 1;                                             // Result = 00000001
loc_80045994:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    return;
}

void psxspu_stop_master_fade() noexcept {
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5988);                                 // Store to: 80075988
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5994);                                 // Store to: 80075994
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5988);                                // Store to: 80075988
    return;
}

void psxspu_get_master_fade_status() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5994);                               // Load from: 80075994
    v0 = (i32(v0) < 2);
    v0 ^= 1;
    return;
}
