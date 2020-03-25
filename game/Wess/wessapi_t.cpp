#include "wessapi_t.h"

#include "PsxVm/PsxVm.h"
#include "wessapi.h"
#include "wessarc.h"

void updatetrackstat() noexcept {
loc_80044098:
    sp -= 0x30;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(s1, sp + 0x1C);
    s1 = a1;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    if (s1 == 0) goto loc_8004436C;
    v0 = lw(s1);
    s3 = v0;
    if (v0 == 0) goto loc_8004436C;
    a0 = s3 & 1;
    v1 = s3 & 2;
    if (a0 == 0) goto loc_800440E4;
    v0 = lbu(s1 + 0x4);
    sb(v0, s0 + 0xC);
loc_800440E4:
    v0 = s3 & 3;
    if (v1 == 0) goto loc_800440FC;
    v0 = lbu(s1 + 0x5);
    sb(v0, s0 + 0xD);
    v0 = s3 & 3;
loc_800440FC:
    if (v0 == 0) goto loc_80044164;
    v0 = lbu(s1 + 0x4);
    s2 = lw(s0 + 0x34);
    sb(v0, s0 + 0xC);
    v1 = lbu(s1 + 0x5);
    v0 = sp + 0x10;
    sw(v0, s0 + 0x34);
    v0 = 0xC;                                           // Result = 0000000C
    sb(v1, s0 + 0xD);
    sb(v0, sp + 0x10);
    v1 = lw(s0 + 0x34);
    v0 = lbu(s0 + 0xC);
    sb(v0, v1 + 0x1);
    v0 = lbu(s0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x30);
    goto loc_80044214;
loc_80044164:
    v0 = sp + 0x10;
    if (a0 == 0) goto loc_800441C0;
    s2 = lw(s0 + 0x34);
    v1 = lbu(s1 + 0x4);
    sw(v0, s0 + 0x34);
    v0 = 0xC;                                           // Result = 0000000C
    sb(v1, s0 + 0xC);
    sb(v0, sp + 0x10);
    v1 = lw(s0 + 0x34);
    v0 = lbu(s0 + 0xC);
    sb(v0, v1 + 0x1);
    v0 = lbu(s0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x30);
    goto loc_80044214;
loc_800441C0:
    if (v1 == 0) goto loc_80044224;
    s2 = lw(s0 + 0x34);
    v1 = lbu(s1 + 0x5);
    sw(v0, s0 + 0x34);
    v0 = 0xD;                                           // Result = 0000000D
    sb(v1, s0 + 0xD);
    sb(v0, sp + 0x10);
    v1 = lw(s0 + 0x34);
    v0 = lbu(s0 + 0xD);
    sb(v0, v1 + 0x1);
    v0 = lbu(s0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x34);
loc_80044214:
    a0 = s0;
    ptr_call(v0);
    sw(s2, s0 + 0x34);
loc_80044224:
    v0 = s3 & 4;
    {
        const bool bJump = (v0 == 0);
        v0 = s3 & 8;
        if (bJump) goto loc_80044240;
    }
    v0 = lhu(s1 + 0x6);
    sh(v0, s0 + 0xA);
    v0 = s3 & 8;
loc_80044240:
    {
        const bool bJump = (v0 == 0);
        v0 = sp + 0x10;
        if (bJump) goto loc_800442B4;
    }
    s2 = lw(s0 + 0x34);
    v1 = lhu(s1 + 0x8);
    sw(v0, s0 + 0x34);
    v0 = 9;                                             // Result = 00000009
    sh(v1, s0 + 0xE);
    sb(v0, sp + 0x10);
    v1 = lw(s0 + 0x34);
    v0 = lbu(s0 + 0xE);
    sb(v0, v1 + 0x1);
    v0 = lhu(s0 + 0xE);
    v1 = lw(s0 + 0x34);
    v0 >>= 8;
    sb(v0, v1 + 0x2);
    v0 = lbu(s0 + 0x3);
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    at += v0;
    v0 = lw(at);
    v0 = lw(v0 + 0x24);
    a0 = s0;
    ptr_call(v0);
    sw(s2, s0 + 0x34);
loc_800442B4:
    v0 = s3 & 0x10;
    {
        const bool bJump = (v0 == 0);
        v0 = s3 & 0x20;
        if (bJump) goto loc_800442FC;
    }
    v0 = lbu(s0 + 0x12);
    v1 = lbu(s1 + 0xA);
    v0 = i32(v0) >> v1;
    v0 &= 1;
    v1 = -3;                                            // Result = FFFFFFFD
    if (v0 == 0) goto loc_800442E8;
    v0 = lw(s0);
    v0 |= 2;
    goto loc_800442F4;
loc_800442E8:
    v0 = lw(s0);
    v0 &= v1;
loc_800442F4:
    sw(v0, s0);
    v0 = s3 & 0x20;
loc_800442FC:
    {
        const bool bJump = (v0 == 0);
        v0 = s3 & 0x40;
        if (bJump) goto loc_8004432C;
    }
    v0 = lhu(s1 + 0xC);
    sh(v0, s0 + 0x16);
    v0 = GetIntsPerSec();
    v0 <<= 16;
    a1 = lh(s0 + 0x14);
    a2 = lh(s0 + 0x16);
    a0 = u32(i32(v0) >> 16);
    v0 = CalcPartsPerInt((int16_t) a0, (int16_t) a1, (int16_t) a2);
    sw(v0, s0 + 0x1C);
    v0 = s3 & 0x40;
loc_8004432C:
    {
        const bool bJump = (v0 == 0);
        v0 = s3 & 0x80;
        if (bJump) goto loc_80044354;
    }
    v0 = lw(s0 + 0x28);
    a0 = lw(s1 + 0x10);
    v1 = lw(s0);
    v0 += a0;
    v1 |= 0x10;
    sw(v0, s0 + 0x2C);
    sw(v1, s0);
    v0 = s3 & 0x80;
loc_80044354:
    if (v0 == 0) goto loc_8004436C;
    v0 = lw(s0);
    v0 |= 0x20;
    sw(v0, s0);
loc_8004436C:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void wess_seq_trigger_type() noexcept {
loc_8004438C:
    sp -= 0x20;
    v1 = a0;
    a2 = a1;
    a0 = v1 << 2;
    a0 += v1;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 <<= 2;
    sw(ra, sp + 0x18);
    v0 = lw(v0 + 0xC);
    a1 = v1;
    v0 = lw(v0 + 0x10);
    a3 = 0;                                             // Result = 00000000
    sw(0, sp + 0x10);
    a0 += v0;
    wess_seq_structrig();
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void wess_seq_trigger_type_special() noexcept {
loc_800443DC:
    sp -= 0x20;
    v1 = a0;
    a3 = a1;
    a0 = v1 << 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 += v1;
    sw(ra, sp + 0x18);
    v0 = lw(v0 + 0xC);
    a0 <<= 2;
    v0 = lw(v0 + 0x10);
    a1 = v1;
    sw(a2, sp + 0x10);
    a2 = a3;
    a3 = 0;                                             // Result = 00000000
    a0 += v0;
    wess_seq_structrig();
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void queue_wess_seq_update_type_special() noexcept {
    sp -= 0x40;
    sw(fp, sp + 0x38);
    fp = a1;
    sw(ra, sp + 0x3C);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    sw(a0, sp + 0x10);
    v0 = Is_Module_Loaded();
    if (v0 == 0) goto loc_80044578;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s4 = lbu(v1 + 0xB);
    v0 = s4;
    if (s6 == 0) goto loc_8004456C;
    v0 &= 0xFF;
    s4--;
    if (v0 == 0) goto loc_8004456C;
    s3 = s5 + 0xC;
loc_800444A8:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_80044554;
    v0 = lw(s3 - 0x4);
    a2 = lw(sp + 0x10);
    if (v0 != a2) goto loc_80044544;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s1 = lw(s3);
    s0 = lbu(v0 + 0x1C);
    v0 = -1;                                            // Result = FFFFFFFF
    s0--;
    if (s0 == v0) goto loc_80044544;
    s7 = -1;                                            // Result = FFFFFFFF
loc_800444F8:
    a0 = lbu(s1);
    a2 = 0xFF;                                          // Result = 000000FF
    v0 = a0 << 2;
    if (a0 == a2) goto loc_80044538;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += a0;
    v1 = lw(v1 + 0x28);
    v0 <<= 4;
    a0 = v0 + v1;
    if (fp == 0) goto loc_8004452C;
    a1 = fp;
    updatetrackstat();
loc_8004452C:
    s2--;
    if (s2 == 0) goto loc_80044544;
loc_80044538:
    s0--;
    s1++;
    if (s0 != s7) goto loc_800444F8;
loc_80044544:
    s6--;
    v0 = s6 & 0xFF;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80044570;
    }
loc_80044554:
    s3 += 0x18;
    s5 += 0x18;
    v0 = s4;
    v0 &= 0xFF;
    s4--;
    if (v0 != 0) goto loc_800444A8;
loc_8004456C:
    v0 = 1;                                             // Result = 00000001
loc_80044570:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80044578:
    ra = lw(sp + 0x3C);
    fp = lw(sp + 0x38);
    s7 = lw(sp + 0x34);
    s6 = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x40;
    return;
}

void wess_seq_stoptype() noexcept {
loc_800445AC:
    sp -= 0x40;
    sw(ra, sp + 0x3C);
    sw(fp, sp + 0x38);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    sw(a0, sp + 0x10);
    v0 = Is_Module_Loaded();
    if (v0 == 0) goto loc_8004470C;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s4 = lbu(v1 + 0xB);
    v0 = s4;
    if (s6 == 0) goto loc_80044700;
    v0 &= 0xFF;
    s4--;
    if (v0 == 0) goto loc_80044700;
    s7 = -1;                                            // Result = FFFFFFFF
    fp = 0x80070000;                                    // Result = 80070000
    fp += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    s3 = s5 + 0xC;
loc_8004462C:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_800446E8;
    v0 = lw(s3 - 0x4);
    a1 = lw(sp + 0x10);
    if (v0 != a1) goto loc_800446D8;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s0 = lbu(v0 + 0x1C);
    s1 = lw(s3);
    s0--;
    if (s0 == s7) goto loc_800446D8;
loc_80044674:
    v1 = lbu(s1);
    a1 = 0xFF;                                          // Result = 000000FF
    a0 = v1 << 2;
    if (v1 == a1) goto loc_800446CC;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 += v1;
    v0 = lw(v0 + 0x28);
    a0 <<= 4;
    a0 += v0;
    v0 = lbu(a0 + 0x3);
    v0 <<= 2;
    v0 += fp;
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    s2--;
    ptr_call(v0);
    if (s2 == 0) goto loc_800446D8;
loc_800446CC:
    s0--;
    s1++;
    if (s0 != s7) goto loc_80044674;
loc_800446D8:
    s6--;
    v0 = s6 & 0xFF;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80044704;
    }
loc_800446E8:
    s3 += 0x18;
    s5 += 0x18;
    v0 = s4;
    v0 &= 0xFF;
    s4--;
    if (v0 != 0) goto loc_8004462C;
loc_80044700:
    v0 = 1;                                             // Result = 00000001
loc_80044704:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_8004470C:
    ra = lw(sp + 0x3C);
    fp = lw(sp + 0x38);
    s7 = lw(sp + 0x34);
    s6 = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x40;
    return;
}
