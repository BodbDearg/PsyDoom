#include "SEQLOAD.h"

#include "PsxVm/PsxVm.h"

void wess_seq_load_err() noexcept {
loc_80044740:
    sp -= 0x18;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5978);                               // Load from: 80075978
    a1 = a0;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_80044768;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x597C);                               // Load from: 8007597C
    pcall(v0);
loc_80044768:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_seq_loader_install_error_handler() noexcept {
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5978);                                // Store to: 80075978
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x597C);                                // Store to: 8007597C
    return;
}

void Is_Seq_Seq_Num_Valid() noexcept {
loc_80044790:
    v0 = 0;                                             // Result = 00000000
    if (i32(a0) < 0) goto loc_800447B4;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x596C);                               // Load from: gWess_max_sequences (8007596C)
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800447B4;
    }
    v0 = 0;                                             // Result = 00000000
loc_800447B4:
    return;
}

void open_sequence_data() noexcept {
loc_800447BC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5974);                               // Load from: 80075974
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 != 0) goto loc_80044800;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5964);                               // Load from: gWess_seq_loader_fileName (80075964)
    module_open();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5980);                                // Store to: 80075980
    if (v0 != 0) goto loc_80044800;
    a0 = 1;                                             // Result = 00000001
    wess_seq_load_err();
    v0 = 0;                                             // Result = 00000000
    goto loc_80044818;
loc_80044800:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5974);                               // Load from: 80075974
    v0 = 1;                                             // Result = 00000001
    v1++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5974);                                // Store to: 80075974
loc_80044818:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void close_sequence_data() noexcept {
loc_80044828:
    sp -= 0x18;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5974);                               // Load from: 80075974
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x10);
    if (v1 != v0) goto loc_80044850;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5980);                               // Load from: 80075980
    module_close();
loc_80044850:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5974);                               // Load from: 80075974
    {
        const bool bJump = (i32(v0) <= 0);
        v0--;
        if (bJump) goto loc_8004486C;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5974);                                // Store to: 80075974
loc_8004486C:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void load_sequence_data() noexcept {
loc_8004487C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x50;
    sw(s5, sp + 0x3C);
    s5 = a0;
    sw(s1, sp + 0x2C);
    s1 = a1;
    sw(fp, sp + 0x48);
    fp = s1;
    sw(ra, sp + 0x4C);
    sw(s7, sp + 0x44);
    sw(s6, sp + 0x40);
    sw(s4, sp + 0x38);
    sw(s3, sp + 0x34);
    sw(s2, sp + 0x30);
    sw(s0, sp + 0x28);
    if (v0 == 0) goto loc_80044FF0;
    Is_Seq_Seq_Num_Valid();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80044FF4;
    }
    open_sequence_data();
    {
        const bool bJump = (v0 != 0);
        v0 = s5 << 2;
        if (bJump) goto loc_800448E8;
    }
    a0 = 1;                                             // Result = 00000001
    goto loc_80044E9C;
loc_800448E8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s5;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    s7 = lw(v0 + 0x10);
    a1 = lw(v0 + 0x8);
    sw(s1, v0 + 0x4);
    if (s7 == 0) goto loc_80044924;
    v0 = s7 << 5;
    s1 += v0;
    goto loc_80044928;
loc_80044924:
    s1 += 0x20;
loc_80044928:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5980);                               // Load from: 80075980
    a2 = 0;                                             // Result = 00000000
    module_seek();
    s0 = 4;                                             // Result = 00000004
    if (v0 != 0) goto loc_80044E98;
    a1 = 4;                                             // Result = 00000004
    v0 = s5 << 2;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 += s5;
    v1 = lw(v1 + 0xC);
    s2 = v0 << 2;
    a0 = lw(v1 + 0x10);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    a0 += s2;
    module_read();
    a0 = 2;                                             // Result = 00000002
    if (v0 != s0) goto loc_80044E9C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s2;
    s4 = lh(v0);
    v0 = -1;                                            // Result = FFFFFFFF
    s4--;
    s0 = 0x18;                                          // Result = 00000018
    if (s4 == v0) goto loc_80044E60;
    s6 = 0x80080000;                                    // Result = 80080000
    s6 -= 0xFB0;                                        // Result = 8007F050
    s3 = s2;
    s2 = 0;                                             // Result = 00000000
loc_800449BC:
    a0 = s6;                                            // Result = 8007F050
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    a1 = 0x18;                                          // Result = 00000018
    module_read();
    a0 = 2;                                             // Result = 00000002
    if (v0 != s0) goto loc_80044E9C;
    v1 = lbu(s6);                                       // Load from: 8007F050
    t0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80044ACC;
    v0 = 0x32;                                          // Result = 00000032
    {
        const bool bJump = (v1 == v0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_80044ACC;
    }
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    a0 = lbu(a1 + 0x8);
    a0--;
    {
        const bool bJump = (a0 == v0);
        v0 = a0 << 2;
        if (bJump) goto loc_80044AE8;
    }
    t1 = v1;
    a3 = a1;
    v1 = lw(a1 + 0x18);
    v0 += a0;
    v0 <<= 2;
    v0 += a0;
    a1 = v0 << 2;
    a2 = a1 + v1;
loc_80044A34:
    v0 = lw(a2 + 0x4C);
    if (t1 != v0) goto loc_80044AD4;
    v1 = lbu(s6 + 0x4);                                 // Load from: 8007F054
    v0 = 3;                                             // Result = 00000003
    if (v1 == 0) goto loc_80044A5C;
    if (v1 != v0) goto loc_80044A70;
loc_80044A5C:
    v0 = lw(a2 + 0x50);
    v0 &= 1;
    if (v0 != 0) goto loc_80044ACC;
loc_80044A70:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0xFAC);                               // Load from: 8007F054
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80044AA4;
    }
    v0 = lw(a3 + 0x18);
    v0 += a1;
    v0 = lw(v0 + 0x50);
    v0 &= 2;
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80044ACC;
    }
loc_80044AA4:
    if (v1 != v0) goto loc_80044AD4;
    v0 = lw(a3 + 0x18);
    v0 += a1;
    v0 = lw(v0 + 0x50);
    v0 &= 4;
    if (v0 == 0) goto loc_80044AD4;
loc_80044ACC:
    t0 = 1;                                             // Result = 00000001
    goto loc_80044AE8;
loc_80044AD4:
    a1 -= 0x54;
    a0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a2 -= 0x54;
    if (a0 != v0) goto loc_80044A34;
loc_80044AE8:
    a2 = 1;                                             // Result = 00000001
    if (t0 == 0) goto loc_80044E24;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    v1 = lw(s6);                                        // Load from: 8007F050
    a0 = lw(s6 + 0x4);                                  // Load from: 8007F054
    a1 = lw(s6 + 0x8);                                  // Load from: 8007F058
    a2 = lw(s6 + 0xC);                                  // Load from: 8007F05C
    sw(v1, v0);
    sw(a0, v0 + 0x4);
    sw(a1, v0 + 0x8);
    sw(a2, v0 + 0xC);
    v1 = lw(s6 + 0x10);                                 // Load from: 8007F060
    a0 = lw(s6 + 0x14);                                 // Load from: 8007F064
    sw(v1, v0 + 0x10);
    sw(a0, v0 + 0x14);
    v1 = lbu(s6);                                       // Load from: 8007F050
    v0 = 0x32;                                          // Result = 00000032
    if (v1 != v0) goto loc_80044D20;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    sb(0, v0);
    v1 = lbu(s6 + 0x4);                                 // Load from: 8007F054
    a0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80044BA8;
    v0 = 3;                                             // Result = 00000003
    {
        const bool bJump = (v1 != v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80044BFC;
    }
loc_80044BA8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lbu(v1 + 0x8);
    a3 = v0;
    if (i32(v0) <= 0) goto loc_80044D28;
    a2 = v1;
    a1 = lw(a2 + 0x18);
loc_80044BCC:
    v1 = a1;
    v0 = lw(v1 + 0x50);
    v0 &= 1;
    a0++;
    if (v0 != 0) goto loc_80044C5C;
    v0 = (i32(a0) < i32(a3));
    a1 = v1 + 0x54;
    if (v0 != 0) goto loc_80044BCC;
    goto loc_80044D20;
loc_80044BFC:
    {
        const bool bJump = (v1 != v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80044CCC;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lbu(v1 + 0x8);
    a0 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80044D28;
    a3 = v0;
    a2 = v1;
    a1 = lw(a2 + 0x18);
loc_80044C2C:
    v1 = a1;
    v0 = lw(v1 + 0x50);
    v0 &= 2;
    a0++;
    if (v0 != 0) goto loc_80044C80;
    v0 = (i32(a0) < i32(a3));
    a1 = v1 + 0x54;
    if (v0 != 0) goto loc_80044C2C;
    goto loc_80044D20;
loc_80044C5C:
    v0 = lw(a2 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v1 = lbu(a1 + 0x4C);
    v0 += s2;
    goto loc_80044CC4;
loc_80044C80:
    v0 = lw(a2 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v1 = lbu(a1 + 0x4C);
    v0 += s2;
    goto loc_80044CC4;
loc_80044CA4:
    v0 = lw(a1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v1 = lbu(v1 + 0x4C);
    v0 += s2;
loc_80044CC4:
    sb(v1, v0);
    goto loc_80044D20;
loc_80044CCC:
    if (v1 != v0) goto loc_80044D20;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lbu(v1 + 0x8);
    a0 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80044D28;
    a2 = v0;
    a1 = v1;
    v1 = lw(a1 + 0x18);
loc_80044CFC:
    v0 = lw(v1 + 0x50);
    v0 &= 4;
    a0++;
    if (v0 != 0) goto loc_80044CA4;
    v0 = (i32(a0) < i32(a2));
    v1 += 0x54;
    if (v0 != 0) goto loc_80044CFC;
loc_80044D20:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
loc_80044D28:
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    sw(s1, v0 + 0x18);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    v0 += s2;
    v1 = lh(v0 + 0x12);
    a0 = lw(v0 + 0x18);
    v1 <<= 2;
    s1 += v1;
    s0 = v1;
    a1 = s0;
    module_read();
    a0 = 2;                                             // Result = 00000002
    if (s0 != v0) goto loc_80044E9C;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    v0 += s2;
    sw(s1, v0 + 0x1C);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s3;
    v0 = lw(v0 + 0x4);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x5980);                               // Load from: 80075980
    v0 += s2;
    v1 = lw(v0 + 0x14);
    a0 = lw(v0 + 0x1C);
    s1 += v1;
    v0 = s1 & 1;
    s1 += v0;
    v0 = s1 & 2;
    s1 += v0;
    s0 = v1;
    a1 = s0;
    module_read();
    s2 += 0x20;                                         // Result = 00000020
    if (s0 != v0) goto loc_80044E90;
    s4--;
    goto loc_80044E54;
loc_80044E24:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5980);                               // Load from: 80075980
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lh(a1 - 0xF9E);                                // Load from: 8007F062
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xF9C);                                // Load from: 8007F064
    a1 <<= 2;
    a1 += v0;
    module_seek();
    a0 = 3;                                             // Result = 00000003
    if (v0 != 0) goto loc_80044E9C;
    s4--;
loc_80044E54:
    v0 = -1;                                            // Result = FFFFFFFF
    s0 = 0x18;                                          // Result = 00000018
    if (s4 != v0) goto loc_800449BC;
loc_80044E60:
    v0 = s5 << 2;
    if (s7 == 0) goto loc_80044EAC;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s5;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    sh(s7, v0);
    goto loc_80044FE8;
loc_80044E90:
    a0 = 2;                                             // Result = 00000002
    goto loc_80044E9C;
loc_80044E98:
    a0 = 3;                                             // Result = 00000003
loc_80044E9C:
    wess_seq_load_err();
    v0 = 0;                                             // Result = 00000000
    goto loc_80044FF4;
loc_80044EAC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    a0 = s5 << 2;
    v0 = lw(v0 + 0xC);
    a0 += s5;
    v0 = lw(v0 + 0x10);
    a0 <<= 2;
    v0 += a0;
    v0 = lw(v0 + 0x4);
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0xF98;                                        // Result = 8007F068
    v1 = lw(a3);                                        // Load from: 8007F068
    a1 = lw(a3 + 0x4);                                  // Load from: 8007F06C
    a2 = lw(a3 + 0x8);                                  // Load from: 8007F070
    sw(v1, v0);
    sw(a1, v0 + 0x4);
    sw(a2, v0 + 0x8);
    v1 = lw(a3 + 0xC);                                  // Load from: 8007F074
    a1 = lw(a3 + 0x10);                                 // Load from: 8007F078
    a2 = lw(a3 + 0x14);                                 // Load from: 8007F07C
    sw(v1, v0 + 0xC);
    sw(a1, v0 + 0x10);
    sw(a2, v0 + 0x14);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    sw(s1, v0 + 0x18);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    sw(s1, v0 + 0x1C);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x5970);                              // Load from: 80075970
    a1 = lw(v0 + 0x1C);
    a2 = lw(v0 + 0x14);
    sb(v1, a1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v0 = lw(v0 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += a0;
    v0 = lw(v0 + 0x4);
    v1 = lw(v0 + 0x1C);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5971);                              // Load from: 80075971
    s1 += a2;
    sb(v0, v1 + 0x1);
    v0 = s1 & 1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    s1 += v0;
    v0 = lw(v1 + 0xC);
    v1 = s1 & 2;
    v0 = lw(v0 + 0x10);
    s1 += v1;
    a0 += v0;
    v0 = 1;                                             // Result = 00000001
    sh(v0, a0);
loc_80044FE8:
    close_sequence_data();
loc_80044FF0:
    v0 = s1 - fp;
loc_80044FF4:
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

void wess_seq_loader_init() noexcept {
loc_80045028:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5960);                                 // Store to: gbWess_seq_loader_enable (80075960)
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x5964);                                // Store to: gWess_seq_loader_fileName (80075964)
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5968);                                // Store to: gpWess_seq_loader_pm_stat (80075968)
    s0 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_80045124;
    v0 = lw(a0 + 0xC);
    s0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5960);                                // Store to: gbWess_seq_loader_enable (80075960)
    v1 = lh(v0 + 0x8);
    v0 = 0x80;                                          // Result = 00000080
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xF96);                                 // Store to: 8007F06A
    v0 = 0x7F;                                          // Result = 0000007F
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xF8E);                                 // Store to: 8007F072
    v0 = 0x40;                                          // Result = 00000040
    at = 0x80080000;                                    // Result = 80080000
    sb(v0, at - 0xF8D);                                 // Store to: 8007F073
    v0 = 0x78;                                          // Result = 00000078
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xF8A);                                 // Store to: 8007F076
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at - 0xF88);                                 // Store to: 8007F078
    v0 = 2;                                             // Result = 00000002
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF98);                                  // Store to: 8007F068
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF97);                                  // Store to: 8007F069
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF93);                                  // Store to: 8007F06D
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF94);                                  // Store to: 8007F06C
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xF92);                                  // Store to: 8007F06E
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xF90);                                  // Store to: 8007F070
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF8C);                                  // Store to: 8007F074
    at = 0x80080000;                                    // Result = 80080000
    sb(0, at - 0xF8B);                                  // Store to: 8007F075
    at = 0x80080000;                                    // Result = 80080000
    sh(0, at - 0xF86);                                  // Store to: 8007F07A
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xF84);                                 // Store to: 8007F07C
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x596C);                                // Store to: gWess_max_sequences (8007596C)
    v0 = s0;                                            // Result = 00000001
    if (a2 != s0) goto loc_80045128;
    open_sequence_data();
    {
        const bool bJump = (v0 != 0);
        v0 = s0;                                        // Result = 00000001
        if (bJump) goto loc_80045128;
    }
    a0 = 1;                                             // Result = 00000001
    wess_seq_load_err();
    v0 = 0;                                             // Result = 00000000
    goto loc_80045128;
loc_80045124:
    v0 = s0;                                            // Result = 00000000
loc_80045128:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_seq_loader_exit() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    close_sequence_data();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5960);                                 // Store to: gbWess_seq_loader_enable (80075960)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_seq_sizeof() noexcept {
loc_80045164:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    if (v0 == 0) goto loc_800451D4;
    Is_Seq_Seq_Num_Valid();
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 2;
        if (bJump) goto loc_800451A0;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_800451DC;
loc_800451A0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v1 += v0;
    v0 = lw(v1 + 0x4);
    {
        const bool bJump = (v0 != 0);
        v0 = s1;                                        // Result = 00000000
        if (bJump) goto loc_800451DC;
    }
    s1 = lw(v1 + 0xC);
loc_800451D4:
    v0 = s1;
loc_800451DC:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_seq_load() noexcept {
loc_800451F4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x1C);
    if (v0 == 0) goto loc_80045278;
    Is_Seq_Seq_Num_Valid();
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 2;
        if (bJump) goto loc_80045238;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_8004527C;
loc_80045238:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    v0 = lw(v0 + 0x4);
    {
        const bool bJump = (v0 != 0);
        v0 = s1;                                        // Result = 00000000
        if (bJump) goto loc_8004527C;
    }
    a0 = s0;
    a1 = s2;
    load_sequence_data();
    s1 = v0;
loc_80045278:
    v0 = s1;
loc_8004527C:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_seq_free() noexcept {
loc_80045298:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    if (v0 == 0) goto loc_8004530C;
    Is_Seq_Seq_Num_Valid();
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 2;
        if (bJump) goto loc_800452D4;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_80045310;
loc_800452D4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v1 += v0;
    v0 = lw(v1 + 0x4);
    {
        const bool bJump = (v0 == 0);
        v0 = s1;                                        // Result = 00000000
        if (bJump) goto loc_80045310;
    }
    sw(0, v1 + 0x4);
    s1 = 1;                                             // Result = 00000001
loc_8004530C:
    v0 = s1;
loc_80045310:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
