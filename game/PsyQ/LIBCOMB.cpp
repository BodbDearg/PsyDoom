#include "LIBCOMB.h"

#include "LIBAPI.h"
#include "PsxVm/PsxVm.h"

void LIBCOMB_UNKNOWN_func_1() noexcept {
loc_800574A8:
    sp -= 0x28;
    t8 = 0x80070000;                                    // Result = 80070000
    t8 = lw(t8 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t6 = 8;                                             // Result = 00000008
    at = 0x80070000;                                    // Result = 80070000
    sw(ra, sp + 0x1C);
    sw(s0, sp + 0x18);
    sw(t6, at + 0x7E80);                                // Store to: 80077E80
    t7 = 0x50;                                          // Result = 00000050
    t9 = 0x5DC;                                         // Result = 000005DC
    sh(t7, t8 + 0xA);
    sw(t9, sp + 0x24);
    t0 = lw(sp + 0x24);
    t1 = t0 - 1;
    sw(t1, sp + 0x24);
    t2 = lw(sp + 0x24);
    if (i32(t2) <= 0) goto loc_80057518;
    t3 = lw(sp + 0x24);
loc_800574FC:
    t4 = t3 - 1;
    sw(t4, sp + 0x24);
    t5 = lw(sp + 0x24);
    t3 = lw(sp + 0x24);
    if (i32(t5) > 0) goto loc_800574FC;
loc_80057518:
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t6 = 0xD8;                                          // Result = 000000D8
    t0 = 0x80070000;                                    // Result = 80070000
    sh(t6, t7 + 0xE);
    t0 = lw(t0 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t8 = 0xD8;                                          // Result = 000000D8
    at = 0x80070000;                                    // Result = 80070000
    sw(t8, at + 0x7E7C);                                // Store to: 80077E7C
    t9 = 0xCE;                                          // Result = 000000CE
    t2 = 0x80070000;                                    // Result = 80070000
    sh(t9, t0 + 0x8);
    t2 = lw(t2 + 0x7E80);                               // Load from: 80077E80
    s0 = 0x80070000;                                    // Result = 80070000
    t3 = t2 << 2;
    at = 0x80070000;                                    // Result = 80070000
    t1 = 0xCE;                                          // Result = 000000CE
    s0 += t3;
    t4 = 0x80070000;                                    // Result = 80070000
    t4 = lw(t4 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    s0 = lhu(s0 + 0x753C);
    sw(t1, at + 0x7E5C);                                // Store to: 80077E5C
    at = 0x80070000;                                    // Result = 80070000
    sh(s0, t4 + 0xA);
    sw(s0, at + 0x7E60);                                // Store to: 80077E60
    at = 0x80070000;                                    // Result = 80070000
    t5 = 3;                                             // Result = 00000003
    a1 = 0x80070000;                                    // Result = 80070000
    sw(t5, at + 0x7E64);                                // Store to: 80077E64
    a1 += 0x7524;                                       // Result = gLIBCOMB_COMB_UNKNOWN_vars_1[1] (80077524)
    a0 = 3;                                             // Result = 00000003
    LIBAPI_SysEnqIntRP();
    t6 = 0x80070000;                                    // Result = 80070000
    t6 = lw(t6 + 0x7534);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_2[1] (80077534)
    at = 0x80070000;                                    // Result = 80070000
    t7 = lw(t6 + 0x4);
    t8 = t7 | 0x100;
    sw(t8, t6 + 0x4);
    sw(0, at + 0x7E6C);                                 // Store to: 80077E6C
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7E68);                                 // Store to: 80077E68
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7E74);                                 // Store to: 80077E74
    at = 0x80070000;                                    // Result = 80070000
    ra = lw(sp + 0x1C);
    sw(0, at + 0x7E70);                                 // Store to: 80077E70
    at = 0x80070000;                                    // Result = 80070000
    s0 = lw(sp + 0x18);
    sw(0, at + 0x7E78);                                 // Store to: 80077E78
    sp += 0x28;
    return;
}

void LIBCOMB_UNKNOWN_func_2() noexcept {
    sp -= 0x10;
    t6 = 0x5DC;                                         // Result = 000005DC
    sw(t6, sp + 0xC);
    t7 = lw(sp + 0xC);
    t8 = t7 - 1;
    sw(t8, sp + 0xC);
    t9 = lw(sp + 0xC);
    t3 = 0x1F0000;                                      // Result = 001F0000
    if (i32(t9) <= 0) goto loc_80057638;
    t0 = lw(sp + 0xC);
loc_80057618:
    t1 = t0 - 1;
    sw(t1, sp + 0xC);
    t2 = lw(sp + 0xC);
    t0 = lw(sp + 0xC);
    if (i32(t2) > 0) goto loc_80057618;
    t3 = 0x1F0000;                                      // Result = 001F0000
loc_80057638:
    t3 |= 0xA400;                                       // Result = 001FA400
    div(t3, a0);
    t4 = 0x80070000;                                    // Result = 80070000
    t4 = lw(t4 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t5 = 0x5DC;                                         // Result = 000005DC
    a1 = lo;
    a1 &= 0xFFFF;
    sh(a1, t4 + 0xE);
    sw(t5, sp + 0x4);
    t6 = lw(sp + 0x4);
    t7 = t6 - 1;
    sw(t7, sp + 0x4);
    t8 = lw(sp + 0x4);
    if (a0 != 0) goto loc_8005767C;
    _break(0x1C00);
loc_8005767C:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80057694;
    }
    if (t3 != at) goto loc_80057694;
    _break(0x1800);
loc_80057694:
    a0 = a1;
    if (i32(t8) <= 0) goto loc_800576C0;
    t9 = lw(sp + 0x4);
loc_800576A4:
    t0 = t9 - 1;
    sw(t0, sp + 0x4);
    t1 = lw(sp + 0x4);
    t9 = lw(sp + 0x4);
    if (i32(t1) > 0) goto loc_800576A4;
loc_800576C0:
    t2 = 0x80070000;                                    // Result = 80070000
    t2 = lw(t2 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t5 = 0x5DC;                                         // Result = 000005DC
    t3 = lhu(t2 + 0xA);
    t4 = t3 | 0x10;
    sh(t4, t2 + 0xA);
    sw(t5, sp);
    t6 = lw(sp);
    t7 = t6 - 1;
    sw(t7, sp);
    t8 = lw(sp);
    if (i32(t8) <= 0) goto loc_80057720;
    t9 = lw(sp);
loc_80057704:
    t0 = t9 - 1;
    sw(t0, sp);
    t1 = lw(sp);
    t9 = lw(sp);
    if (i32(t1) > 0) goto loc_80057704;
loc_80057720:
    sp += 0x10;
    return;
}

void LIBCOMB_UNKNOWN_func_3() noexcept {
loc_80057728:
    t6 = 0x80070000;                                    // Result = 80070000
    t6 = lw(t6 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    sp -= 0x10;
    sw(a0, sp + 0x10);
    t7 = lhu(t6 + 0xA);
    t9 = 0x5DC;                                         // Result = 000005DC
    t8 = t7 | 0x10;
    sh(t8, t6 + 0xA);
    sw(t9, sp + 0xC);
    t0 = lw(sp + 0xC);
    t1 = t0 - 1;
    sw(t1, sp + 0xC);
    t2 = lw(sp + 0xC);
    if (i32(t2) <= 0) goto loc_8005778C;
    t3 = lw(sp + 0xC);
loc_80057770:
    t4 = t3 - 1;
    sw(t4, sp + 0xC);
    t5 = lw(sp + 0xC);
    t3 = lw(sp + 0xC);
    if (i32(t5) > 0) goto loc_80057770;
loc_8005778C:
    t7 = 0x80070000;                                    // Result = 80070000
    t8 = 0x80070000;                                    // Result = 80070000
    t8 = lw(t8 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t7 = lw(t7 + 0x7E5C);                               // Load from: 80077E5C
    t6 = 0x5DC;                                         // Result = 000005DC
    sh(t7, t8 + 0x8);
    sw(t6, sp + 0x8);
    t9 = lw(sp + 0x8);
    t0 = t9 - 1;
    sw(t0, sp + 0x8);
    t1 = lw(sp + 0x8);
    if (i32(t1) <= 0) goto loc_800577E8;
    t2 = lw(sp + 0x8);
loc_800577CC:
    t3 = t2 - 1;
    sw(t3, sp + 0x8);
    t4 = lw(sp + 0x8);
    t2 = lw(sp + 0x8);
    if (i32(t4) > 0) goto loc_800577CC;
loc_800577E8:
    t5 = 0x80070000;                                    // Result = 80070000
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t5 = lw(t5 + 0x7E60);                               // Load from: 80077E60
    t8 = 0x5DC;                                         // Result = 000005DC
    sh(t5, t7 + 0xA);
    sw(t8, sp + 0x4);
    t6 = lw(sp + 0x4);
    t9 = t6 - 1;
    sw(t9, sp + 0x4);
    t0 = lw(sp + 0x4);
    v0 = 0;                                             // Result = 00000000
    if (i32(t0) <= 0) goto loc_80057848;
    t1 = lw(sp + 0x4);
loc_80057828:
    t2 = t1 - 1;
    sw(t2, sp + 0x4);
    t3 = lw(sp + 0x4);
    t1 = lw(sp + 0x4);
    if (i32(t3) > 0) goto loc_80057828;
    v0 = 0;                                             // Result = 00000000
loc_80057848:
    sp += 0x10;
    return;
}

void LIBCOMB_UNKNOWN_func_4() noexcept {
loc_80057850:
    sp -= 0x28;
    sw(a1, sp + 0x2C);
    t6 = lw(sp + 0x2C);
    sw(s0, sp + 0x18);
    at = 1;                                             // Result = 00000001
    sw(ra, sp + 0x1C);
    s0 = a0;
    sw(a0, sp + 0x28);
    if (t6 != at) goto loc_80057B08;
    t7 = lw(s0);
    t8 = t7 & 0x8000;
    if (t8 == 0) goto loc_80057988;
    t9 = 0x80070000;                                    // Result = 80070000
    t9 = lw(t9 + 0x7E6C);                               // Load from: 80077E6C
    t1 = 0x10;                                          // Result = 00000010
    if (t9 != 0) goto loc_8005797C;
    t0 = lw(s0 + 0xC);
    if (t0 == 0) goto loc_80057970;
    t1 = lw(s0 + 0x8);
    at = 0x80070000;                                    // Result = 80070000
    sw(t1, at + 0x7E6C);                                // Store to: 80077E6C
    t2 = lw(s0 + 0xC);
    t3 = 0x80070000;                                    // Result = 80070000
    t3 = lw(t3 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    at = 0x80070000;                                    // Result = 80070000
    sw(t2, at + 0x7E74);                                // Store to: 80077E74
    t4 = lhu(t3 + 0x4);
    t5 = t4 & 0x38;
    if (t5 == 0) goto loc_800578FC;
    t6 = lhu(t3 + 0xA);
    t8 = 5;                                             // Result = 00000005
    t7 = t6 | 0x10;
    sh(t7, t3 + 0xA);
    sw(t8, s0 + 0x18);
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_80057D94;
loc_800578FC:
    t9 = 0x80070000;                                    // Result = 80070000
    t9 = lw(t9 + 0x7E60);                               // Load from: 80077E60
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7E78);                               // Load from: 80077E78
    at = 0x80070000;                                    // Result = 80070000
    t0 = t9 | 0x826;
    sw(t0, at + 0x7E60);                                // Store to: 80077E60
    if (t1 != 0) goto loc_80057970;
    t2 = 0x80070000;                                    // Result = 80070000
    t2 = lw(t2 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t4 = 0x5DC;                                         // Result = 000005DC
    sh(t0, t2 + 0xA);
    sw(t4, sp + 0x20);
    t5 = lw(sp + 0x20);
    t6 = t5 - 1;
    sw(t6, sp + 0x20);
    t7 = lw(sp + 0x20);
    if (i32(t7) <= 0) goto loc_80057970;
    t3 = lw(sp + 0x20);
loc_80057954:
    t8 = t3 - 1;
    sw(t8, sp + 0x20);
    t9 = lw(sp + 0x20);
    t3 = lw(sp + 0x20);
    if (i32(t9) > 0) goto loc_80057954;
loc_80057970:
    v0 = 0;                                             // Result = 00000000
    goto loc_80057D94;
    t1 = 0x10;                                          // Result = 00000010
loc_8005797C:
    sw(t1, s0 + 0x18);
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_80057D94;
loc_80057988:
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7E6C);                               // Load from: 80077E6C
    if (t0 == 0) goto loc_800579AC;
    t2 = 0x10;                                          // Result = 00000010
    sw(t2, s0 + 0x18);
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_80057D94;
loc_800579AC:
    t4 = lw(s0 + 0xC);
    if (t4 == 0) goto loc_80057AEC;
    t5 = lw(s0 + 0x8);
    at = 0x80070000;                                    // Result = 80070000
    sw(t5, at + 0x7E6C);                                // Store to: 80077E6C
    t6 = lw(s0 + 0xC);
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    at = 0x80070000;                                    // Result = 80070000
    sw(t6, at + 0x7E74);                                // Store to: 80077E74
    t3 = lhu(t7 + 0xA);
    t9 = 0x80070000;                                    // Result = 80070000
    t8 = t3 | 4;
    sh(t8, t7 + 0xA);
    t9 = lw(t9 + 0x7E74);                               // Load from: 80077E74
    if (t9 == 0) goto loc_80057AC8;
loc_800579FC:
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t0 = lhu(t1 + 0x4);
    t2 = t0 & 2;
    if (t2 != 0) goto loc_80057A84;
loc_80057A1C:
    t4 = 0x80070000;                                    // Result = 80070000
    t4 = lw(t4 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t5 = lhu(t4 + 0x4);
    t6 = t5 & 0x38;
    if (t6 == 0) goto loc_80057A64;
    t3 = 5;                                             // Result = 00000005
    sw(t3, s0 + 0x18);
    t8 = 0x80070000;                                    // Result = 80070000
    t8 = lw(t8 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t7 = lhu(t8 + 0xA);
    t9 = t7 | 0x10;
    sh(t9, t8 + 0xA);
    goto loc_80057AC8;
loc_80057A64:
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t0 = lhu(t1 + 0x4);
    t2 = t0 & 2;
    if (t2 == 0) goto loc_80057A1C;
loc_80057A84:
    t4 = 0x80070000;                                    // Result = 80070000
    t4 = lw(t4 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t6 = 0x80070000;                                    // Result = 80070000
    t6 = lw(t6 + 0x7E6C);                               // Load from: 80077E6C
    t5 = lbu(t4);
    t3 = 0x80070000;                                    // Result = 80070000
    sb(t5, t6);
    t3 = lw(t3 + 0x7E6C);                               // Load from: 80077E6C
    t9 = 0x80070000;                                    // Result = 80070000
    t9 = lw(t9 + 0x7E74);                               // Load from: 80077E74
    at = 0x80070000;                                    // Result = 80070000
    t7 = t3 + 1;
    sw(t7, at + 0x7E6C);                                // Store to: 80077E6C
    at = 0x80070000;                                    // Result = 80070000
    t8 = t9 - 1;
    sw(t8, at + 0x7E74);                                // Store to: 80077E74
    if (t8 != 0) goto loc_800579FC;
loc_80057AC8:
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    a0 = 0xF0000000;                                    // Result = F0000000
    t0 = lhu(t1 + 0xA);
    a0 |= 0xB;                                          // Result = F000000B
    t2 = t0 & 0xFFFB;
    a1 = 0x400;                                         // Result = 00000400
    sh(t2, t1 + 0xA);
    LIBAPI_DeliverEvent();
loc_80057AEC:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7E6C);                                 // Store to: 80077E6C
    t5 = 0x80070000;                                    // Result = 80070000
    t5 = lw(t5 + 0x7E74);                               // Load from: 80077E74
    t4 = lw(s0 + 0xC);
    v0 = t4 - t5;
    goto loc_80057D94;
loc_80057B08:
    t6 = lw(sp + 0x2C);
    at = 2;                                             // Result = 00000002
    if (t6 != at) goto loc_80057D88;
    t3 = lw(s0);
    t7 = t3 & 2;
    if (t7 == 0) goto loc_80057D88;
    t9 = lw(s0);
    t8 = t9 & 0x8000;
    if (t8 == 0) goto loc_80057BDC;
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7E68);                               // Load from: 80077E68
    t2 = 0x10;                                          // Result = 00000010
    if (t0 != 0) goto loc_80057BD0;
    t2 = lw(s0 + 0xC);
    if (t2 == 0) goto loc_80057BC4;
    t1 = lw(s0 + 0x8);
    at = 0x80070000;                                    // Result = 80070000
    sw(t1, at + 0x7E68);                                // Store to: 80077E68
    t4 = lw(s0 + 0xC);
    t5 = 0x80070000;                                    // Result = 80070000
    t5 = lw(t5 + 0x7E60);                               // Load from: 80077E60
    at = 0x80070000;                                    // Result = 80070000
    t3 = 0x80070000;                                    // Result = 80070000
    t3 = lw(t3 + 0x7E78);                               // Load from: 80077E78
    sw(t4, at + 0x7E70);                                // Store to: 80077E70
    at = 0x80070000;                                    // Result = 80070000
    t6 = t5 | 0x1001;
    sw(t6, at + 0x7E60);                                // Store to: 80077E60
    if (t3 != 0) goto loc_80057BC4;
    t9 = 0x80070000;                                    // Result = 80070000
    t9 = lw(t9 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t7 = 0x10;                                          // Result = 00000010
    sh(t7, t9 + 0xA);
    t8 = 0x80070000;                                    // Result = 80070000
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t8 = lw(t8 + 0x7E60);                               // Load from: 80077E60
    sh(t8, t0 + 0xA);
loc_80057BC4:
    v0 = 0;                                             // Result = 00000000
    goto loc_80057D94;
    t2 = 0x10;                                          // Result = 00000010
loc_80057BD0:
    sw(t2, s0 + 0x18);
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_80057D94;
loc_80057BDC:
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7E68);                               // Load from: 80077E68
    if (t1 == 0) goto loc_80057C00;
    t4 = 0x10;                                          // Result = 00000010
    sw(t4, s0 + 0x18);
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_80057D94;
loc_80057C00:
    t5 = lw(s0 + 0xC);
    if (t5 == 0) goto loc_80057D6C;
    t6 = lw(s0 + 0x8);
    at = 0x80070000;                                    // Result = 80070000
    sw(t6, at + 0x7E68);                                // Store to: 80077E68
    t3 = lw(s0 + 0xC);
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    at = 0x80070000;                                    // Result = 80070000
    sw(t3, at + 0x7E70);                                // Store to: 80077E70
    t9 = lhu(t7 + 0xA);
    t0 = 0x80070000;                                    // Result = 80070000
    t8 = t9 | 1;
    sh(t8, t7 + 0xA);
    t0 = lw(t0 + 0x7E70);                               // Load from: 80077E70
    if (t0 == 0) goto loc_80057D48;
loc_80057C50:
    t2 = 0x80070000;                                    // Result = 80070000
    t2 = lw(t2 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t1 = lhu(t2 + 0x4);
    t4 = t1 & 1;
    if (t4 != 0) goto loc_80057D04;
loc_80057C70:
    t5 = 0x80070000;                                    // Result = 80070000
    t5 = lw(t5 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t6 = lhu(t5 + 0x4);
    t3 = t6 & 0x180;
    if (t3 != 0) goto loc_80057C9C;
    t9 = 0x20;                                          // Result = 00000020
    sw(t9, s0 + 0x18);
    goto loc_80057D48;
loc_80057C9C:
    t8 = 0x80070000;                                    // Result = 80070000
    t8 = lw(t8 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t7 = lhu(t8 + 0x4);
    t0 = t7 & 0x38;
    if (t0 == 0) goto loc_80057CE4;
    t2 = 5;                                             // Result = 00000005
    sw(t2, s0 + 0x18);
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t4 = lhu(t1 + 0xA);
    t5 = t4 | 0x10;
    sh(t5, t1 + 0xA);
    goto loc_80057D48;
loc_80057CE4:
    t6 = 0x80070000;                                    // Result = 80070000
    t6 = lw(t6 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t3 = lhu(t6 + 0x4);
    t9 = t3 & 1;
    if (t9 == 0) goto loc_80057C70;
loc_80057D04:
    t8 = 0x80070000;                                    // Result = 80070000
    t8 = lw(t8 + 0x7E68);                               // Load from: 80077E68
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t7 = lbu(t8);
    t2 = 0x80070000;                                    // Result = 80070000
    sb(t7, t0);
    t2 = lw(t2 + 0x7E68);                               // Load from: 80077E68
    t5 = 0x80070000;                                    // Result = 80070000
    t5 = lw(t5 + 0x7E70);                               // Load from: 80077E70
    at = 0x80070000;                                    // Result = 80070000
    t4 = t2 + 1;
    sw(t4, at + 0x7E68);                                // Store to: 80077E68
    at = 0x80070000;                                    // Result = 80070000
    t1 = t5 - 1;
    sw(t1, at + 0x7E70);                                // Store to: 80077E70
    if (t1 != 0) goto loc_80057C50;
loc_80057D48:
    t6 = 0x80070000;                                    // Result = 80070000
    t6 = lw(t6 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    a0 = 0xF0000000;                                    // Result = F0000000
    t3 = lhu(t6 + 0xA);
    a0 |= 0xB;                                          // Result = F000000B
    t9 = t3 & 0xFFFE;
    a1 = 0x800;                                         // Result = 00000800
    sh(t9, t6 + 0xA);
    LIBAPI_DeliverEvent();
loc_80057D6C:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7E68);                                 // Store to: 80077E68
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7E70);                               // Load from: 80077E70
    t8 = lw(s0 + 0xC);
    v0 = t8 - t7;
    goto loc_80057D94;
loc_80057D88:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7560;                                       // Result = STR_siocons_bad_func_err[0] (80077560)
    LIBCOMB__ioabort();
loc_80057D94:
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void LIBCOMB_UNKNOWN_func_5() noexcept {
loc_80057DA4:
    sw(a0, sp);
    a1 = a0;
    t6 = lw(a1);
    t7 = t6 & 2;
    at = 0x80070000;                                    // Result = 80070000
    if (t7 == 0) goto loc_80057DCC;
    sw(0, at + 0x7E68);                                 // Store to: 80077E68
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7E70);                                 // Store to: 80077E70
loc_80057DCC:
    t8 = lw(a1);
    t9 = t8 & 1;
    at = 0x80070000;                                    // Result = 80070000
    if (t9 == 0) goto loc_80057DEC;
    sw(0, at + 0x7E6C);                                 // Store to: 80077E6C
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7E74);                                 // Store to: 80077E74
loc_80057DEC:
    return;
}

void LIBCOMB_UNKNOWN_func_6() noexcept {
loc_80057DF4:
    return;
}

void LIBCOMB_UNKNOWN_func_7() noexcept {
loc_80057DFC:
    t6 = 0x80070000;                                    // Result = 80070000
    t6 = lw(t6 + 0x7534);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_2[1] (80077534)
    at = 0x100;                                         // Result = 00000100
    t7 = lw(t6);
    t8 = lw(t6 + 0x4);
    t9 = t7 & t8;
    t0 = t9 & 0x100;
    if (t0 == at) goto loc_80057E2C;
    v0 = 0;                                             // Result = 00000000
    return;
loc_80057E2C:
    t2 = 0x80070000;                                    // Result = 80070000
    t2 = lw(t2 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t1 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(t1, at + 0x7E78);                                // Store to: 80077E78
    t3 = lhu(t2 + 0xA);
    v0 = 1;                                             // Result = 00000001
    t4 = t3 & 0xFFDD;
    sh(t4, t2 + 0xA);
    return;
}

void LIBCOMB_UNKNOWN_func_8() noexcept {
loc_80057E58:
    t6 = 0x80070000;                                    // Result = 80070000
    t6 = lw(t6 + 0x7E64);                               // Load from: 80077E64
    sp -= 0x20;
    t7 = t6 & 1;
    sw(ra, sp + 0x14);
    if (t7 == 0) goto loc_80057E80;
    t9 = 0x80070000;                                    // Result = 80070000
    t9 = lw(t9 + 0x7534);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_2[1] (80077534)
    t8 = -0x101;                                        // Result = FFFFFEFF
    sw(t8, t9);
loc_80057E80:
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t1 = lhu(t0 + 0x4);
    t2 = t1 & 0x38;
    if (t2 != 0) goto loc_80057F10;
    LIBCOMB_UNKNOWN_func_9();
    t3 = 0x80070000;                                    // Result = 80070000
    t3 = lw(t3 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t6 = 0x5DC;                                         // Result = 000005DC
    t4 = lhu(t3 + 0xA);
    t5 = t4 | 0x10;
    sh(t5, t3 + 0xA);
    sw(t6, sp + 0x1C);
    t7 = lw(sp + 0x1C);
    t8 = t7 - 1;
    sw(t8, sp + 0x1C);
    t9 = lw(sp + 0x1C);
    if (i32(t9) <= 0) goto loc_80057F20;
    t0 = lw(sp + 0x1C);
loc_80057EEC:
    t1 = t0 - 1;
    sw(t1, sp + 0x1C);
    t2 = lw(sp + 0x1C);
    t0 = lw(sp + 0x1C);
    if (i32(t2) > 0) goto loc_80057EEC;
    goto loc_80057F20;
loc_80057F10:
    a0 = 0xF0000000;                                    // Result = F0000000
    a0 |= 0xB;                                          // Result = F000000B
    a1 = 0x8000;                                        // Result = 00008000
    LIBAPI_DeliverEvent();
loc_80057F20:
    t4 = 0x80070000;                                    // Result = 80070000
    t5 = 0x80070000;                                    // Result = 80070000
    t5 = lw(t5 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t4 = lw(t4 + 0x7E60);                               // Load from: 80077E60
    t3 = 0x80070000;                                    // Result = 80070000
    sh(t4, t5 + 0xA);
    t3 = lw(t3 + 0x7E64);                               // Load from: 80077E64
    at = 0x80070000;                                    // Result = 80070000
    t6 = t3 & 2;
    sw(0, at + 0x7E78);                                 // Store to: 80077E78
    if (t6 == 0) goto loc_80057F54;
    LIBAPI_ReturnFromException();
loc_80057F54:
    ra = lw(sp + 0x14);
    sp += 0x20;
    return;
}

void LIBCOMB_UNKNOWN_func_9() noexcept {
loc_80057F64:
    t6 = 0x80070000;                                    // Result = 80070000
    t6 = lw(t6 + 0x7E6C);                               // Load from: 80077E6C
    sp -= 0x20;
    sw(ra, sp + 0x14);
    if (t6 == 0) goto loc_80058098;
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t8 = lhu(t7 + 0x4);
    t9 = t8 & 2;
    ra = lw(sp + 0x14);
    if (t9 == 0) goto loc_8005809C;
loc_80057F98:
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t2 = 0x80070000;                                    // Result = 80070000
    t2 = lw(t2 + 0x7E6C);                               // Load from: 80077E6C
    t1 = lbu(t0);
    t3 = 0x80070000;                                    // Result = 80070000
    sb(t1, t2);
    t3 = lw(t3 + 0x7E6C);                               // Load from: 80077E6C
    t5 = 0x80070000;                                    // Result = 80070000
    t5 = lw(t5 + 0x7E74);                               // Load from: 80077E74
    at = 0x80070000;                                    // Result = 80070000
    t4 = t3 + 1;
    sw(t4, at + 0x7E6C);                                // Store to: 80077E6C
    at = 0x80070000;                                    // Result = 80070000
    t6 = t5 - 1;
    sw(t6, at + 0x7E74);                                // Store to: 80077E74
    if (t6 != 0) goto loc_80058064;
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7E60);                               // Load from: 80077E60
    at = -0x827;                                        // Result = FFFFF7D9
    t9 = 0x80070000;                                    // Result = 80070000
    t8 = t7 & at;
    t9 = lw(t9 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    at = 0x80070000;                                    // Result = 80070000
    sw(t8, at + 0x7E60);                                // Store to: 80077E60
    t0 = 0x5DC;                                         // Result = 000005DC
    sh(t8, t9 + 0xA);
    sw(t0, sp + 0x1C);
    t1 = lw(sp + 0x1C);
    t2 = t1 - 1;
    sw(t2, sp + 0x1C);
    t3 = lw(sp + 0x1C);
    if (i32(t3) <= 0) goto loc_80058044;
loc_80058028:
    t4 = lw(sp + 0x1C);
    t5 = t4 - 1;
    sw(t5, sp + 0x1C);
    t6 = lw(sp + 0x1C);
loc_80058044:
    at = 0x80070000;                                    // Result = 80070000
    if (i32(t6) > 0) goto loc_80058028;
    a0 = 0xF0000000;                                    // Result = F0000000
    sw(0, at + 0x7E6C);                                 // Store to: 80077E6C
    a0 |= 0xB;                                          // Result = F000000B
    a1 = 0x400;                                         // Result = 00000400
    LIBAPI_DeliverEvent();
    ra = lw(sp + 0x14);
    goto loc_8005809C;
loc_80058064:
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7E6C);                               // Load from: 80077E6C
    ra = lw(sp + 0x14);
    if (t7 == 0) goto loc_8005809C;
    t8 = 0x80070000;                                    // Result = 80070000
    t8 = lw(t8 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t9 = lhu(t8 + 0x4);
    t0 = t9 & 2;
    if (t0 != 0) goto loc_80057F98;
loc_80058098:
    ra = lw(sp + 0x14);
loc_8005809C:
    sp += 0x20;
    return;
}

void LIBCOMB_ChangeClearSIO() noexcept {
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x7E64);                                // Store to: 80077E64
    return;
}

void LIBCOMB_AddCOMB() noexcept {
loc_800580B4:
    sp -= 0x18;
    sw(ra, sp + 0x14);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7578;                                       // Result = gpSTR_sio_1 (80077578)
    LIBAPI_AddDrv();
    LIBAPI_FlushCache();
    ra = lw(sp + 0x14);
    sp += 0x18;
    return;
}

void LIBCOMB_DelCOMB() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x14);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x75D8;                                       // Result = STR_sio_2[0] (800775D8)
    LIBAPI_DelDrv();
    LIBAPI_FlushCache();
    ra = lw(sp + 0x14);
    sp += 0x18;
    return;
}

void LIBCOMB_UNKNOWN_ctrl_help() noexcept {
loc_8005810C:
    a3 = a0;
    sp -= 0x20;
    at = (a3 < 4);
    sw(0, sp + 0x1C);
    if (at == 0) goto loc_80058520;
    t6 = a3 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += t6;
    t6 = lw(at + 0x21F4);
    switch (t6) {
        case 0x8005813C: goto loc_8005813C;
        case 0x800581F0: goto loc_800581F0;
        case 0x8005828C: goto loc_8005828C;
        case 0x800584F8: goto loc_800584F8;
        default: jump_table_err(); break;
    }
loc_8005813C:
    a3 = a1;
    goto loc_800581C8;
loc_80058144:
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t8 = lhu(t7 + 0x4);
    sw(t8, sp + 0x1C);
    goto loc_80058528;
loc_8005815C:
    t9 = 0x80070000;                                    // Result = 80070000
    t9 = lw(t9 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t0 = lhu(t9 + 0xA);
    sw(t0, sp + 0x1C);
    goto loc_80058528;
loc_80058174:
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t2 = lhu(t1 + 0x8);
    sw(t2, sp + 0x1C);
    goto loc_80058528;
loc_8005818C:
    t3 = 0x80070000;                                    // Result = 80070000
    t3 = lw(t3 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t4 = lhu(t3 + 0xE);
    sw(t4, sp + 0x1C);
    goto loc_80058528;
loc_800581A4:
    t5 = 0x80070000;                                    // Result = 80070000
    t5 = lw(t5 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t6 = lbu(t5);
    sw(t6, sp + 0x1C);
    goto loc_80058528;
loc_800581BC:
    t7 = -1;                                            // Result = FFFFFFFF
    sw(t7, sp + 0x1C);
    goto loc_80058528;
loc_800581C8:
    at = (a3 < 5);
    if (at == 0) goto loc_800581BC;
    t8 = a3 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += t8;
    t8 = lw(at + 0x2204);
    switch (t8) {
        case 0x80058144: goto loc_80058144;
        case 0x8005815C: goto loc_8005815C;
        case 0x80058174: goto loc_80058174;
        case 0x8005818C: goto loc_8005818C;
        case 0x800581A4: goto loc_800581A4;
        default: jump_table_err(); break;
    }
loc_800581F0:
    a3 = a1;
    goto loc_80058264;
loc_800581F8:
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t9 = a2 & 0xFF;
    sh(t9, t0 + 0x4);
    goto loc_80058528;
loc_8005820C:
    t2 = 0x80070000;                                    // Result = 80070000
    t2 = lw(t2 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t1 = a2 & 0xFF;
    sh(t1, t2 + 0xA);
    goto loc_80058528;
loc_80058220:
    t4 = 0x80070000;                                    // Result = 80070000
    t4 = lw(t4 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t3 = a2 & 0xFF;
    sh(t3, t4 + 0x8);
    goto loc_80058528;
loc_80058234:
    t6 = 0x80070000;                                    // Result = 80070000
    t6 = lw(t6 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t5 = a2 & 0xFF;
    sh(t5, t6 + 0xE);
    goto loc_80058528;
loc_80058248:
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    sb(a2, t7);
    goto loc_80058528;
loc_80058258:
    t8 = -1;                                            // Result = FFFFFFFF
    sw(t8, sp + 0x1C);
    goto loc_80058528;
loc_80058264:
    at = (a3 < 5);
    if (at == 0) goto loc_80058258;
    t9 = a3 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += t9;
    t9 = lw(at + 0x2218);
    switch (t9) {
        case 0x800581F8: goto loc_800581F8;
        case 0x8005820C: goto loc_8005820C;
        case 0x80058220: goto loc_80058220;
        case 0x80058234: goto loc_80058234;
        case 0x80058248: goto loc_80058248;
        default: jump_table_err(); break;
    }
loc_8005828C:
    a3 = a1;
    goto loc_800584D8;
loc_80058294:
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t3 = 0x5DC;                                         // Result = 000005DC
    t1 = lhu(t0 + 0xA);
    t2 = t1 | 0x40;
    sh(t2, t0 + 0xA);
    sw(t3, sp + 0x14);
    t4 = lw(sp + 0x14);
    t5 = t4 - 1;
    sw(t5, sp + 0x14);
    t6 = lw(sp + 0x14);
    if (i32(t6) <= 0) goto loc_800582F4;
    t7 = lw(sp + 0x14);
loc_800582D8:
    t8 = t7 - 1;
    sw(t8, sp + 0x14);
    t9 = lw(sp + 0x14);
    t7 = lw(sp + 0x14);
    if (i32(t9) > 0) goto loc_800582D8;
loc_800582F4:
    t1 = 0x80070000;                                    // Result = 80070000
    t2 = 0x80070000;                                    // Result = 80070000
    t2 = lw(t2 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t1 = lw(t1 + 0x7E5C);                               // Load from: 80077E5C
    t0 = 0x5DC;                                         // Result = 000005DC
    sh(t1, t2 + 0x8);
    sw(t0, sp + 0x10);
    t3 = lw(sp + 0x10);
    t4 = t3 - 1;
    sw(t4, sp + 0x10);
    t5 = lw(sp + 0x10);
    if (i32(t5) <= 0) goto loc_80058350;
    t6 = lw(sp + 0x10);
loc_80058334:
    t7 = t6 - 1;
    sw(t7, sp + 0x10);
    t8 = lw(sp + 0x10);
    t6 = lw(sp + 0x10);
    if (i32(t8) > 0) goto loc_80058334;
loc_80058350:
    t9 = 0x80070000;                                    // Result = 80070000
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t9 = lw(t9 + 0x7E60);                               // Load from: 80077E60
    t2 = 0x5DC;                                         // Result = 000005DC
    sh(t9, t1 + 0xA);
    sw(t2, sp + 0xC);
    t0 = lw(sp + 0xC);
    t3 = t0 - 1;
    sw(t3, sp + 0xC);
    t4 = lw(sp + 0xC);
    if (i32(t4) <= 0) goto loc_800583AC;
    t5 = lw(sp + 0xC);
loc_80058390:
    t6 = t5 - 1;
    sw(t6, sp + 0xC);
    t7 = lw(sp + 0xC);
    t5 = lw(sp + 0xC);
    if (i32(t7) > 0) goto loc_80058390;
loc_800583AC:
    t8 = 0x80070000;                                    // Result = 80070000
    t9 = 0x80070000;                                    // Result = 80070000
    t9 = lw(t9 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t8 = lw(t8 + 0x7E7C);                               // Load from: 80077E7C
    t1 = 0x5DC;                                         // Result = 000005DC
    sh(t8, t9 + 0xE);
    sw(t1, sp + 0x8);
    t2 = lw(sp + 0x8);
    t0 = t2 - 1;
    sw(t0, sp + 0x8);
    t3 = lw(sp + 0x8);
    v0 = lw(sp + 0x1C);
    if (i32(t3) <= 0) goto loc_8005852C;
    t4 = lw(sp + 0x8);
loc_800583EC:
    t5 = t4 - 1;
    sw(t5, sp + 0x8);
    t6 = lw(sp + 0x8);
    t4 = lw(sp + 0x8);
    if (i32(t6) > 0) goto loc_800583EC;
    v0 = lw(sp + 0x1C);
    goto loc_8005852C;
loc_80058410:
    t7 = 0x80070000;                                    // Result = 80070000
    t7 = lw(t7 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t1 = 0x5DC;                                         // Result = 000005DC
    t8 = lhu(t7 + 0xA);
    t9 = t8 | 0x10;
    sh(t9, t7 + 0xA);
    sw(t1, sp + 0x4);
    t2 = lw(sp + 0x4);
    t0 = t2 - 1;
    sw(t0, sp + 0x4);
    t3 = lw(sp + 0x4);
    v0 = lw(sp + 0x1C);
    if (i32(t3) <= 0) goto loc_8005852C;
    t4 = lw(sp + 0x4);
loc_80058454:
    t5 = t4 - 1;
    sw(t5, sp + 0x4);
    t6 = lw(sp + 0x4);
    t4 = lw(sp + 0x4);
    if (i32(t6) > 0) goto loc_80058454;
    v0 = lw(sp + 0x1C);
    goto loc_8005852C;
loc_80058478:
    t8 = 0x80070000;                                    // Result = 80070000
    t8 = lw(t8 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t9 = lhu(t8 + 0x4);
    t7 = t9 & 2;
    v0 = lw(sp + 0x1C);
    if (t7 == 0) goto loc_8005852C;
loc_80058498:
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    t2 = lbu(t1);
    sb(t2, sp);
    t0 = lhu(t1 + 0x4);
    t3 = t0 & 2;
    if (t3 != 0) goto loc_80058498;
    v0 = lw(sp + 0x1C);
    goto loc_8005852C;
    t4 = -1;                                            // Result = FFFFFFFF
loc_800584D0:
    sw(t4, sp + 0x1C);
    goto loc_80058528;
loc_800584D8:
    at = 1;                                             // Result = 00000001
    if (a3 == 0) goto loc_80058294;
    {
        const bool bJump = (a3 == at);
        at = 2;                                         // Result = 00000002
        if (bJump) goto loc_80058410;
    }
    if (a3 == at) goto loc_80058478;
    t4 = -1;                                            // Result = FFFFFFFF
    goto loc_800584D0;
loc_800584F8:
    t5 = 0x1F0000;                                      // Result = 001F0000
    t5 |= 0xA400;                                       // Result = 001FA400
    divu(t5, a1);
    t6 = lo;
    sw(t6, sp + 0x1C);
    if (a1 != 0) goto loc_80058518;
    _break(0x1C00);
loc_80058518:
    v0 = lw(sp + 0x1C);
    goto loc_8005852C;
loc_80058520:
    t8 = -1;                                            // Result = FFFFFFFF
    sw(t8, sp + 0x1C);
loc_80058528:
    v0 = lw(sp + 0x1C);
loc_8005852C:
    sp += 0x20;
    return;
}

void LIBCOMB__comb_control() noexcept {
loc_80058534:
    sp -= 0x38;
    sw(a0, sp + 0x38);
    sw(s0, sp + 0x14);
    s0 = lw(sp + 0x38);
    sw(ra, sp + 0x1C);
    sw(a1, sp + 0x3C);
    at = (s0 < 4);
    sw(a2, sp + 0x40);
    sw(s1, sp + 0x18);
    sw(0, sp + 0x34);
    if (at == 0) goto loc_800589F0;
    t6 = s0 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += t6;
    t6 = lw(at + 0x222C);
    switch (t6) {
        case 0x8005857C: goto loc_8005857C;
        case 0x800586E8: goto loc_800586E8;
        case 0x80058864: goto loc_80058864;
        case 0x800589C8: goto loc_800589C8;
        default: jump_table_err(); break;
    }
loc_8005857C:
    s0 = lw(sp + 0x3C);
    at = (s0 < 6);
    goto loc_800586C4;
loc_80058588:
    a0 = lw(sp + 0x38);
    a1 = lw(sp + 0x3C);
    a2 = lw(sp + 0x40);
    LIBCOMB_UNKNOWN_ctrl_help();
    sw(v0, sp + 0x34);
    goto loc_800589F8;
loc_800585A4:
    a0 = lw(sp + 0x38);
    a1 = lw(sp + 0x3C);
    a2 = lw(sp + 0x40);
    LIBCOMB_UNKNOWN_ctrl_help();
    s0 = v0;
    sw(s0, sp + 0x40);
    t7 = lw(sp + 0x40);
    t8 = t7 & 2;
    s0 = 0;                                             // Result = 00000000
    if (t8 == 0) goto loc_800585E0;
    s0 = 1;                                             // Result = 00000001
    goto loc_800585E0;
    s0 = 0;                                             // Result = 00000000
loc_800585E0:
    t9 = lw(sp + 0x40);
    t0 = t9 & 0x20;
    s1 = 0;                                             // Result = 00000000
    if (t0 == 0) goto loc_80058600;
    s1 = 2;                                             // Result = 00000002
    goto loc_80058600;
    s1 = 0;                                             // Result = 00000000
loc_80058600:
    t1 = s0 | s1;
    sw(t1, sp + 0x34);
    goto loc_800589F8;
loc_8005860C:
    a0 = lw(sp + 0x38);
    a1 = lw(sp + 0x3C);
    a2 = lw(sp + 0x40);
    LIBCOMB_UNKNOWN_ctrl_help();
    t2 = 0x1F0000;                                      // Result = 001F0000
    t2 |= 0xA400;                                       // Result = 001FA400
    s0 = v0;
    div(t2, s0);
    t3 = lo;
    sw(t3, sp + 0x34);
    if (s0 != 0) goto loc_80058644;
    _break(0x1C00);
loc_80058644:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8005865C;
    }
    if (t2 != at) goto loc_8005865C;
    _break(0x1800);
loc_8005865C:
    ra = lw(sp + 0x1C);
    goto loc_800589FC;
loc_80058664:
    t4 = 0x80070000;                                    // Result = 80070000
    t4 = lw(t4 + 0x7E80);                               // Load from: 80077E80
    sw(t4, sp + 0x34);
    goto loc_800589F8;
loc_80058674:
    t5 = lw(sp + 0x40);
    t7 = lw(sp + 0x40);
    if (t5 != 0) goto loc_80058698;
    t6 = 0x80070000;                                    // Result = 80070000
    t6 = lw(t6 + 0x7E70);                               // Load from: 80077E70
    sw(t6, sp + 0x34);
    goto loc_800589F8;
    t7 = lw(sp + 0x40);
loc_80058698:
    at = 1;                                             // Result = 00000001
    t9 = -1;                                            // Result = FFFFFFFF
    if (t7 != at) goto loc_800586B8;
    t8 = 0x80070000;                                    // Result = 80070000
    t8 = lw(t8 + 0x7E74);                               // Load from: 80077E74
    sw(t8, sp + 0x34);
    goto loc_800589F8;
    t9 = -1;                                            // Result = FFFFFFFF
loc_800586B8:
    sw(t9, sp + 0x34);
    goto loc_800589F8;
    at = (s0 < 6);
loc_800586C4:
    if (at == 0) goto loc_800589F8;
    t0 = s0 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += t0;
    t0 = lw(at + 0x223C);
    switch (t0) {
        case 0x80058588: goto loc_80058588;
        case 0x800585A4: goto loc_800585A4;
        case 0x800589F8: goto loc_800589F8;
        case 0x8005860C: goto loc_8005860C;
        case 0x80058664: goto loc_80058664;
        case 0x80058674: goto loc_80058674;
        default: jump_table_err(); break;
    }
loc_800586E8:
    s0 = lw(sp + 0x3C);
    t9 = s0 - 1;
    goto loc_8005883C;
loc_800586F4:
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7E60);                               // Load from: 80077E60
    t3 = lw(sp + 0x40);
    at = -0x23;                                         // Result = FFFFFFDD
    t2 = t1 & at;
    at = 0x80070000;                                    // Result = 80070000
    t4 = t3 & 1;
    sw(t2, at + 0x7E60);                                // Store to: 80077E60
    if (t4 == 0) goto loc_80058720;
    s0 = 2;                                             // Result = 00000002
    goto loc_80058724;
loc_80058720:
    s0 = 0;                                             // Result = 00000000
loc_80058724:
    t5 = 0x80070000;                                    // Result = 80070000
    t5 = lw(t5 + 0x7E60);                               // Load from: 80077E60
    t7 = lw(sp + 0x40);
    at = 0x80070000;                                    // Result = 80070000
    t6 = t5 | s0;
    t8 = t7 & 2;
    sw(t6, at + 0x7E60);                                // Store to: 80077E60
    if (t8 == 0) goto loc_8005874C;
    s0 = 0x20;                                          // Result = 00000020
    goto loc_80058750;
loc_8005874C:
    s0 = 0;                                             // Result = 00000000
loc_80058750:
    t9 = 0x80070000;                                    // Result = 80070000
    t9 = lw(t9 + 0x7E60);                               // Load from: 80077E60
    at = 0x80070000;                                    // Result = 80070000
    a0 = lw(sp + 0x38);
    a1 = lw(sp + 0x3C);
    t0 = t9 | s0;
    sw(t0, at + 0x7E60);                                // Store to: 80077E60
    a2 = t0;
    LIBCOMB_UNKNOWN_ctrl_help();
    ra = lw(sp + 0x1C);
    goto loc_800589FC;
loc_8005877C:
    t1 = lw(sp + 0x40);
    at = 0x80070000;                                    // Result = 80070000
    a0 = lw(sp + 0x38);
    a1 = lw(sp + 0x3C);
    sw(t1, at + 0x7E5C);                                // Store to: 80077E5C
    a2 = t1;
    LIBCOMB_UNKNOWN_ctrl_help();
    sw(v0, sp + 0x34);
    goto loc_800589F8;
loc_800587A0:
    a1 = lw(sp + 0x40);
    a0 = 3;                                             // Result = 00000003
    a2 = 0;                                             // Result = 00000000
    LIBCOMB_UNKNOWN_ctrl_help();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7E7C);                                // Store to: 80077E7C
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7E7C);                               // Load from: 80077E7C
    a0 = lw(sp + 0x38);
    a1 = lw(sp + 0x3C);
    LIBCOMB_UNKNOWN_ctrl_help();
    sw(v0, sp + 0x34);
    goto loc_800589F8;
loc_800587D8:
    t2 = lw(sp + 0x3C);
    at = 0x80070000;                                    // Result = 80070000
    t3 = 0x80070000;                                    // Result = 80070000
    t3 = lw(t3 + 0x7E60);                               // Load from: 80077E60
    sw(t2, at + 0x7E80);                                // Store to: 80077E80
    at = -0x301;                                        // Result = FFFFFCFF
    t6 = 0x80070000;                                    // Result = 80070000
    t5 = t2 << 2;
    t6 += t5;
    t4 = t3 & at;
    t6 = lw(t6 + 0x753C);
    at = 0x80070000;                                    // Result = 80070000
    sw(t4, at + 0x7E60);                                // Store to: 80077E60
    t7 = t4 | t6;
    sw(t7, at + 0x7E60);                                // Store to: 80077E60
    a2 = t7;
    a0 = 1;                                             // Result = 00000001
    a1 = 2;                                             // Result = 00000002
    LIBCOMB_UNKNOWN_ctrl_help();
    ra = lw(sp + 0x1C);
    goto loc_800589FC;
loc_8005882C:
    t8 = -1;                                            // Result = FFFFFFFF
    sw(t8, sp + 0x34);
    goto loc_800589F8;
    t9 = s0 - 1;
loc_8005883C:
    at = (t9 < 4);
    if (at == 0) goto loc_8005882C;
    t9 <<= 2;
    at = 0x80010000;                                    // Result = 80010000
    at += t9;
    t9 = lw(at + 0x2254);
    switch (t9) {
        case 0x800586F4: goto loc_800586F4;
        case 0x8005877C: goto loc_8005877C;
        case 0x800587A0: goto loc_800587A0;
        case 0x800587D8: goto loc_800587D8;
        default: jump_table_err(); break;
    }
loc_80058864:
    s0 = lw(sp + 0x3C);
    at = (s0 < 4);
    goto loc_800589A4;
loc_80058870:
    a0 = lw(sp + 0x38);
    a1 = lw(sp + 0x3C);
    a2 = lw(sp + 0x40);
    LIBCOMB_UNKNOWN_ctrl_help();
    sw(v0, sp + 0x34);
    goto loc_800589F8;
loc_8005888C:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7E68);                                 // Store to: 80077E68
    at = 0x80070000;                                    // Result = 80070000
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7E60);                               // Load from: 80077E60
    sw(0, at + 0x7E70);                                 // Store to: 80077E70
    at = -0x1002;                                       // Result = FFFFEFFE
    t3 = 0x80070000;                                    // Result = 80070000
    t1 = t0 & at;
    t3 = lw(t3 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    at = 0x80070000;                                    // Result = 80070000
    sw(t1, at + 0x7E60);                                // Store to: 80077E60
    t2 = 0x5DC;                                         // Result = 000005DC
    sh(t1, t3 + 0xA);
    sw(t2, sp + 0x28);
    t5 = lw(sp + 0x28);
    t4 = t5 - 1;
    sw(t4, sp + 0x28);
    t6 = lw(sp + 0x28);
    ra = lw(sp + 0x1C);
    if (i32(t6) <= 0) goto loc_800589FC;
    t7 = lw(sp + 0x28);
loc_800588EC:
    t8 = t7 - 1;
    sw(t8, sp + 0x28);
    t9 = lw(sp + 0x28);
    t7 = lw(sp + 0x28);
    if (i32(t9) > 0) goto loc_800588EC;
    ra = lw(sp + 0x1C);
    goto loc_800589FC;
loc_80058910:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7E6C);                                 // Store to: 80077E6C
    at = 0x80070000;                                    // Result = 80070000
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7E60);                               // Load from: 80077E60
    sw(0, at + 0x7E74);                                 // Store to: 80077E74
    at = -0x827;                                        // Result = FFFFF7D9
    t3 = 0x80070000;                                    // Result = 80070000
    t1 = t0 & at;
    t3 = lw(t3 + 0x7520);                               // Load from: gLIBCOMB_COMB_UNKNOWN_vars_1[0] (80077520)
    at = 0x80070000;                                    // Result = 80070000
    sw(t1, at + 0x7E60);                                // Store to: 80077E60
    t2 = 0x5DC;                                         // Result = 000005DC
    sh(t1, t3 + 0xA);
    sw(t2, sp + 0x24);
    t5 = lw(sp + 0x24);
    t4 = t5 - 1;
    sw(t4, sp + 0x24);
    t6 = lw(sp + 0x24);
    ra = lw(sp + 0x1C);
    if (i32(t6) <= 0) goto loc_800589FC;
    t7 = lw(sp + 0x24);
loc_80058970:
    t8 = t7 - 1;
    sw(t8, sp + 0x24);
    t9 = lw(sp + 0x24);
    t7 = lw(sp + 0x24);
    if (i32(t9) > 0) goto loc_80058970;
    ra = lw(sp + 0x1C);
    goto loc_800589FC;
loc_80058994:
    t0 = -1;                                            // Result = FFFFFFFF
    sw(t0, sp + 0x34);
    goto loc_800589F8;
    at = (s0 < 4);
loc_800589A4:
    if (at == 0) goto loc_80058994;
    t1 = s0 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += t1;
    t1 = lw(at + 0x2264);
    switch (t1) {
        case 0x80058870: goto loc_80058870;
        case 0x8005888C: goto loc_8005888C;
        case 0x80058910: goto loc_80058910;
        default: jump_table_err(); break;
    }
loc_800589C8:
    a0 = 0;                                             // Result = 00000000
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    LIBCOMB_UNKNOWN_ctrl_help();
    s0 = v0;
    t3 = s0 & 0x180;
    t2 = t3 ^ 0x180;
    t2 = (t2 < 1);
    sw(t2, sp + 0x34);
    goto loc_800589F8;
loc_800589F0:
    t5 = -1;                                            // Result = FFFFFFFF
    sw(t5, sp + 0x34);
loc_800589F8:
    ra = lw(sp + 0x1C);
loc_800589FC:
    v0 = lw(sp + 0x34);
    s0 = lw(sp + 0x14);
    s1 = lw(sp + 0x18);
    sp += 0x38;
    return;
}

void LIBCOMB__ioabort() noexcept {
loc_80058A48:
    t2 = 0xC0;                                          // Result = 000000C0
    t1 = 0x19;                                          // Result = 00000019
    bios_call(t2);
}
