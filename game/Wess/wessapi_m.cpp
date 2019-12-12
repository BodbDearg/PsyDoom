#include "wessapi_m.h"

#include "PsxVm/PsxVm.h"
#include "wessapi.h"

void wess_master_sfx_volume_get() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    Is_Module_Loaded();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800497F8;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A04);                              // Load from: gWess_master_sfx_volume (80075A04)
loc_800497F8:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_master_mus_volume_get() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    Is_Module_Loaded();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80049828;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A05);                              // Load from: gWess_master_mus_volume (80075A05)
loc_80049828:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_master_sfx_vol_set() noexcept {
loc_80049838:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    Is_Module_Loaded();
    if (v0 == 0) goto loc_8004985C;
    at = 0x80070000;                                    // Result = 80070000
    sb(s0, at + 0x5A04);                                // Store to: gWess_master_sfx_volume (80075A04)
loc_8004985C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_master_mus_vol_set() noexcept {
loc_80049870:
    sp -= 0x48;
    sw(s0, sp + 0x20);
    s0 = a0;
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s6, sp + 0x38);
    sw(s5, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    Is_Module_Loaded();
    if (v0 == 0) goto loc_800499FC;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sb(s0, at + 0x5A05);                                // Store to: gWess_master_mus_volume (80075A05)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    fp = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s7 = lw(v0 + 0x20);
    s6 = lbu(v1 + 0xB);
    v0 = s6;
    if (fp == 0) goto loc_800499F0;
    v0 &= 0xFF;
    s6--;
    if (v0 == 0) goto loc_800499F0;
    a1 = -1;                                            // Result = FFFFFFFF
    s5 = s7 + 0xC;
loc_800498F0:
    v0 = lw(s7);
    v0 &= 1;
    if (v0 == 0) goto loc_800499D8;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s4 = lbu(s5 - 0x8);
    s2 = lbu(v0 + 0x1C);
    s3 = lw(s5);
    s2--;
    if (s2 == a1) goto loc_800499C8;
loc_80049924:
    a0 = lbu(s3);
    v0 = 0xFF;                                          // Result = 000000FF
    {
        const bool bJump = (a0 == v0);
        v0 = a0 << 2;
        if (bJump) goto loc_800499BC;
    }
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += a0;
    v1 = lw(v1 + 0x28);
    v0 <<= 4;
    s1 = v0 + v1;
    v1 = lbu(s1 + 0x13);
    v0 = 1;                                             // Result = 00000001
    s4--;
    if (v1 != v0) goto loc_800499B4;
    s0 = lw(s1 + 0x34);
    v0 = sp + 0x10;
    sw(v0, s1 + 0x34);
    v0 = 0xC;                                           // Result = 0000000C
    sb(v0, sp + 0x10);
    v1 = lw(s1 + 0x34);
    v0 = lbu(s1 + 0xC);
    sb(v0, v1 + 0x1);
    v0 = lbu(s1 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    v0 <<= 2;
    v0 += a2;
    v0 = lw(v0);
    v0 = lw(v0 + 0x30);
    a0 = s1;
    sw(a1, sp + 0x18);
    ptr_call(v0);
    sw(s0, s1 + 0x34);
    a1 = lw(sp + 0x18);
loc_800499B4:
    if (s4 == 0) goto loc_800499C8;
loc_800499BC:
    s2--;
    s3++;
    if (s2 != a1) goto loc_80049924;
loc_800499C8:
    fp--;
    v0 = fp & 0xFF;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800499F4;
    }
loc_800499D8:
    s5 += 0x18;
    s7 += 0x18;
    v0 = s6;
    v0 &= 0xFF;
    s6--;
    if (v0 != 0) goto loc_800498F0;
loc_800499F0:
    v0 = 1;                                             // Result = 00000001
loc_800499F4:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_800499FC:
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

void wess_pan_mode_get() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x5A06);                              // Load from: gWess_pan_status (80075A06)
    return;
}

void wess_pan_mode_set() noexcept {
    at = 0x80070000;                                    // Result = 80070000
    sb(a0, at + 0x5A06);                                // Store to: gWess_pan_status (80075A06)
    return;
}
