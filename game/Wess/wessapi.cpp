#include "wessapi.h"

#include "psxcmd.h"
#include "PsxVm/PsxVm.h"
#include "wessapi_t.h"
#include "wessarc.h"
#include "wessseq.h"

const VmPtr<bool32_t>   gbWess_module_loaded(0x800758F8);       // TODO: COMMENT

void trackstart() noexcept {
loc_80041734:
    v1 = lw(a0);
    v0 = v1 & 8;
    {
        const bool bJump = (v0 == 0);
        v0 = -9;                                        // Result = FFFFFFF7
        if (bJump) goto loc_80041770;
    }
    v0 &= v1;
    sw(v0, a0);
    v0 = lbu(a1 + 0x5);
    v0++;
    sb(v0, a1 + 0x5);
    v0 &= 0xFF;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80041770;
    }
    sb(v0, a1 + 0x1);
loc_80041770:
    return;
}

void trackstop() noexcept {
loc_80041778:
    v1 = lw(a0);
    v0 = v1 & 8;
    {
        const bool bJump = (v0 != 0);
        v0 = v1 | 8;
        if (bJump) goto loc_800417B0;
    }
    sw(v0, a0);
    v0 = lbu(a1 + 0x5);
    v0--;
    sb(v0, a1 + 0x5);
    v0 &= 0xFF;
    if (v0 != 0) goto loc_800417B0;
    sb(0, a1 + 0x1);
loc_800417B0:
    return;
}

void queue_wess_seq_pause() noexcept {
    sp -= 0x48;
    sw(a0, sp + 0x10);
    a0 = lw(sp + 0x10);
    sw(ra, sp + 0x44);
    sw(fp, sp + 0x40);
    sw(s7, sp + 0x3C);
    sw(s6, sp + 0x38);
    sw(s5, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    sw(a1, sp + 0x18);
    Is_Seq_Num_Valid();
    if (v0 == 0) goto loc_8004192C;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s7 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s6 = lbu(v1 + 0xB);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s7 == 0) goto loc_80041920;
    s6--;
    {
        const bool bJump = (s6 == v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80041924;
    }
    fp = -1;                                            // Result = FFFFFFFF
    s4 = s5 + 0xC;
loc_80041838:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_80041910;
    v0 = lh(s4 - 0xA);
    a2 = lw(sp + 0x10);
    if (v0 != a2) goto loc_80041904;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s3 = lbu(s4 - 0x8);
    s1 = lbu(v0 + 0x1C);
    s2 = lw(s4);
    s1--;
    if (s1 == fp) goto loc_80041904;
loc_80041880:
    a0 = lbu(s2);
    v0 = 0xFF;                                          // Result = 000000FF
    {
        const bool bJump = (a0 == v0);
        v0 = a0 << 2;
        if (bJump) goto loc_800418F8;
    }
    v0 += a0;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 <<= 4;
    v1 = lw(v1 + 0x28);
    a1 = s5;
    s0 = v0 + v1;
    a0 = s0;
    trackstop();
    a2 = lw(sp + 0x18);
    v0 = 1;                                             // Result = 00000001
    s3--;
    if (a2 != v0) goto loc_800418F0;
    v0 = lbu(s0 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    v0 <<= 2;
    v0 += a2;
    v0 = lw(v0);
    v0 = lw(v0 + 0x18);
    a0 = s0;
    ptr_call(v0);
loc_800418F0:
    if (s3 == 0) goto loc_80041904;
loc_800418F8:
    s1--;
    s2++;
    if (s1 != fp) goto loc_80041880;
loc_80041904:
    s7--;
    v0 = 1;                                             // Result = 00000001
    if (s7 == 0) goto loc_80041924;
loc_80041910:
    s4 += 0x18;
    s6--;
    s5 += 0x18;
    if (s6 != fp) goto loc_80041838;
loc_80041920:
    v0 = 1;                                             // Result = 00000001
loc_80041924:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_8004192C:
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

void queue_wess_seq_restart() noexcept {
    sp -= 0x40;
    sw(fp, sp + 0x38);
    fp = a0;
    sw(ra, sp + 0x3C);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    Is_Seq_Num_Valid();
    if (v0 == 0) goto loc_80041A98;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s4 = lw(v0 + 0x20);
    s5 = lbu(v1 + 0xB);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s6 == 0) goto loc_80041A8C;
    s5--;
    {
        const bool bJump = (s5 == v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80041A90;
    }
    a2 = -1;                                            // Result = FFFFFFFF
    s3 = s4 + 0xC;
loc_800419D8:
    v0 = lw(s4);
    v0 &= 1;
    if (v0 == 0) goto loc_80041A7C;
    v0 = lh(s3 - 0xA);
    if (v0 != fp) goto loc_80041A70;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s0 = lbu(v0 + 0x1C);
    s1 = lw(s3);
    s0--;
    if (s0 == a2) goto loc_80041A70;
    s7 = -1;                                            // Result = FFFFFFFF
loc_80041A20:
    v1 = lbu(s1);
    a3 = 0xFF;                                          // Result = 000000FF
    a1 = s4;
    if (v1 == a3) goto loc_80041A64;
    s2--;
    a0 = v1 << 2;
    a0 += v1;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 <<= 4;
    v0 = lw(v0 + 0x28);
    sw(a2, sp + 0x10);
    a0 += v0;
    trackstart();
    a2 = lw(sp + 0x10);
    if (s2 == 0) goto loc_80041A70;
loc_80041A64:
    s0--;
    s1++;
    if (s0 != s7) goto loc_80041A20;
loc_80041A70:
    s6--;
    v0 = 1;                                             // Result = 00000001
    if (s6 == 0) goto loc_80041A90;
loc_80041A7C:
    s3 += 0x18;
    s5--;
    s4 += 0x18;
    if (s5 != a2) goto loc_800419D8;
loc_80041A8C:
    v0 = 1;                                             // Result = 00000001
loc_80041A90:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80041A98:
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

void queue_wess_seq_pauseall() noexcept {
loc_80041ACC:
    sp -= 0x40;
    sw(s0, sp + 0x18);
    s0 = a1;
    sw(ra, sp + 0x3C);
    sw(fp, sp + 0x38);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(a0, sp + 0x10);
    v0 = Is_Module_Loaded();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80041C54;
    }
    a2 = lw(sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    if (a2 != v0) goto loc_80041B28;
    a0 = s0;
    start_record_music_mute();
loc_80041B28:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s7 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s4 = lw(v0 + 0x20);
    s6 = lbu(v1 + 0xB);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s7 == 0) goto loc_80041C34;
    s6--;
    if (s6 == v0) goto loc_80041C34;
    fp = -1;                                            // Result = FFFFFFFF
    s5 = s4 + 0xC;
loc_80041B60:
    v0 = lw(s4);
    v0 &= 1;
    if (v0 == 0) goto loc_80041C24;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s3 = lbu(s5 - 0x8);
    s1 = lbu(v0 + 0x1C);
    s2 = lw(s5);
    s1--;
    if (s1 == fp) goto loc_80041C18;
loc_80041B94:
    a0 = lbu(s2);
    v0 = 0xFF;                                          // Result = 000000FF
    {
        const bool bJump = (a0 == v0);
        v0 = a0 << 2;
        if (bJump) goto loc_80041C0C;
    }
    v0 += a0;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 <<= 4;
    v1 = lw(v1 + 0x28);
    a2 = lw(sp + 0x10);
    s0 = v0 + v1;
    v0 = 1;                                             // Result = 00000001
    s3--;
    if (a2 != v0) goto loc_80041BF8;
    v0 = lbu(s0 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    v0 <<= 2;
    v0 += a2;
    v0 = lw(v0);
    v0 = lw(v0 + 0x18);
    a0 = s0;
    ptr_call(v0);
loc_80041BF8:
    a0 = s0;
    a1 = s4;
    trackstop();
    if (s3 == 0) goto loc_80041C18;
loc_80041C0C:
    s1--;
    s2++;
    if (s1 != fp) goto loc_80041B94;
loc_80041C18:
    s7--;
    if (s7 == 0) goto loc_80041C34;
loc_80041C24:
    s5 += 0x18;
    s6--;
    s4 += 0x18;
    if (s6 != fp) goto loc_80041B60;
loc_80041C34:
    a2 = lw(sp + 0x10);
    s0 = 1;                                             // Result = 00000001
    if (a2 != s0) goto loc_80041C4C;
    end_record_music_mute();
loc_80041C4C:
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80041C54:
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

void queue_wess_seq_restartall() noexcept {
loc_80041C88:
    sp -= 0x58;
    sw(s5, sp + 0x44);
    s5 = a0;
    sw(ra, sp + 0x54);
    sw(fp, sp + 0x50);
    sw(s7, sp + 0x4C);
    sw(s6, sp + 0x48);
    sw(s4, sp + 0x40);
    sw(s3, sp + 0x3C);
    sw(s2, sp + 0x38);
    sw(s1, sp + 0x34);
    sw(s0, sp + 0x30);
    v0 = Is_Module_Loaded();
    if (v0 == 0) goto loc_80041E44;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    t1 = lbu(v0 + 0x4);
    sw(t1, sp + 0x18);
    v1 = lw(v0 + 0xC);
    fp = lw(v0 + 0x20);
    t0 = lbu(v1 + 0xB);
    v0 = -1;                                            // Result = FFFFFFFF
    if (t1 == 0) goto loc_80041E30;
    t0--;
    if (t0 == v0) goto loc_80041E30;
    s7 = fp + 2;
loc_80041D04:
    v0 = lw(fp);
    v0 &= 1;
    t1 = -1;                                            // Result = FFFFFFFF
    if (v0 == 0) goto loc_80041E1C;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s6 = lbu(s7 + 0x2);
    s2 = lbu(v0 + 0x1C);
    s4 = lw(s7 + 0xA);
    s2--;
    if (s2 == t1) goto loc_80041E08;
loc_80041D38:
    a0 = lbu(s4);
    v0 = 0xFF;                                          // Result = 000000FF
    a1 = fp;
    if (a0 == v0) goto loc_80041DF8;
    v0 = a0 << 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += a0;
    v1 = lw(v1 + 0x28);
    v0 <<= 4;
    sw(t0, sp + 0x28);
    s3 = v0 + v1;
    a0 = s3;
    trackstart();
    t0 = lw(sp + 0x28);
    if (s5 == 0) goto loc_80041DEC;
    v0 = lw(s5);
    s1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80041DEC;
    s0 = s5;
loc_80041D90:
    v1 = lbu(s4);
    v0 = lh(s0 + 0x6);
    s1++;
    if (v1 != v0) goto loc_80041DD8;
    v1 = lh(s7);
    v0 = lh(s0 + 0x4);
    a0 = s3;
    if (v1 != v0) goto loc_80041DD8;
    v0 = lbu(s0 + 0x9);
    a3 = lbu(s0 + 0x8);
    sw(v0, sp + 0x10);
    a1 = lw(s0 + 0xC);
    a2 = lw(s0 + 0x10);
    sw(t0, sp + 0x28);
    PSX_voicenote();
    t0 = lw(sp + 0x28);
loc_80041DD8:
    v0 = lw(s5);
    v0 = (i32(s1) < i32(v0));
    s0 += 0x10;
    if (v0 != 0) goto loc_80041D90;
loc_80041DEC:
    s6--;
    if (s6 == 0) goto loc_80041E08;
loc_80041DF8:
    s2--;
    t1 = -1;                                            // Result = FFFFFFFF
    s4++;
    if (s2 != t1) goto loc_80041D38;
loc_80041E08:
    t1 = lw(sp + 0x18);
    t1--;
    sw(t1, sp + 0x18);
    if (t1 == 0) goto loc_80041E30;
loc_80041E1C:
    s7 += 0x18;
    t0--;
    t1 = -1;                                            // Result = FFFFFFFF
    fp += 0x18;
    if (t0 != t1) goto loc_80041D04;
loc_80041E30:
    v0 = 1;                                             // Result = 00000001
    if (s5 == 0) goto loc_80041E3C;
    sw(0, s5);
loc_80041E3C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80041E44:
    ra = lw(sp + 0x54);
    fp = lw(sp + 0x50);
    s7 = lw(sp + 0x4C);
    s6 = lw(sp + 0x48);
    s5 = lw(sp + 0x44);
    s4 = lw(sp + 0x40);
    s3 = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x58;
    return;
}

void zeroset() noexcept {
loc_80041E78:
    sp -= 8;
    v0 = a1 - 1;
    if (a1 == 0) goto loc_80041E98;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80041E88:
    sb(0, a0);
    v0--;
    a0++;
    if (v0 != v1) goto loc_80041E88;
loc_80041E98:
    sp += 8;
    return;
}

void wess_install_error_handler() noexcept {
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x5918);                                // Store to: gpWess_Error_func (80075918)
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x591C);                                // Store to: gpWess_Error_module (8007591C)
    return;
}

void wess_get_master_status() noexcept {
loc_80041EBC:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    return;
}

void Is_System_Active() noexcept {
loc_80041ECC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F4);                               // Load from: gbWess_sysinit (800758F4)
    v0 = (v0 > 0);
    return;
}

// TODO: COMMENT
bool Is_Module_Loaded() noexcept {
    return *gbWess_module_loaded;
}

void Is_Seq_Num_Valid() noexcept {
loc_80041EEC:
    v0 = 0;                                             // Result = 00000000
    if (i32(a0) < 0) goto loc_80041F40;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5900);                               // Load from: gWess_max_seq_num (80075900)
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = a0 << 2;
        if (bJump) goto loc_80041F14;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_80041F40;
loc_80041F14:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v1 = lw(v1 + 0xC);
    v0 += a0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    v0 = lw(v0 + 0x4);
    v0 = (v0 > 0);
loc_80041F40:
    return;
}

void Register_Early_Exit() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58FC);                               // Load from: gbWess_early_exit (800758FC)
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80041F64;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58FC);                                // Store to: gbWess_early_exit (800758FC)
loc_80041F64:
    return;
}

void wess_install_handler() noexcept {
loc_80041F6C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    init_WessTimer();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_restore_handler() noexcept {
loc_80041F8C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    exit_WessTimer();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_init() noexcept {
loc_80041FAC:
    sp -= 0x18;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F4);                               // Load from: gbWess_sysinit (800758F4)
    v1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x10);
    if (v0 != 0) goto loc_80041FFC;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x594C);                               // Load from: gbWess_WessTimerActive (8007594C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    if (v0 != 0) goto loc_80041FE4;
    wess_install_handler();
loc_80041FE4:
    wess_low_level_init();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58F4);                                // Store to: gbWess_sysinit (800758F4)
    v1 = 1;                                             // Result = 00000001
loc_80041FFC:
    v0 = v1;
    ra = lw(sp + 0x10);
    sp += 0x18;
}

void wess_exit() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    Is_System_Active();
    if (v0 == 0) goto loc_80042088;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F4);                               // Load from: gbWess_sysinit (800758F4)
    if (v0 == 0) goto loc_80042088;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    if (v0 == 0) goto loc_8004205C;
    wess_unload_module();
loc_8004205C:
    wess_low_level_exit();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x594C);                               // Load from: gbWess_WessTimerActive (8007594C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x58F4);                                 // Store to: gbWess_sysinit (800758F4)
    v0 |= s0;
    if (v0 == 0) goto loc_80042088;
    wess_restore_handler();
loc_80042088:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_get_wmd_start() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x590C);                               // Load from: gpWess_wmd_mem (8007590C)
    return;
}

void wess_get_wmd_end() noexcept {
loc_800420AC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5910);                               // Load from: gpWess_wmd_end (80075910)
    return;
}

void free_mem_if_mine() noexcept {
loc_800420BC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5908);                               // Load from: gbWess_wmd_mem_is_mine (80075908)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_800420FC;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x590C);                               // Load from: gpWess_wmd_mem (8007590C)
    if (a0 == 0) goto loc_800420F4;
    wess_free();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x590C);                                 // Store to: gpWess_wmd_mem (8007590C)
loc_800420F4:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5908);                                 // Store to: gbWess_wmd_mem_is_mine (80075908)
loc_800420FC:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_unload_module() noexcept {
loc_8004210C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    sp -= 0x30;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    if (v0 == 0) goto loc_800421F8;
    s0 = 0;                                             // Result = 00000000
    wess_seq_stopall();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5920);                               // Load from: gWess_CmdFuncArr[0] (80075920)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0 + 0x4);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    ptr_call(v0);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(v0 + 0x8);
    if (s2 == 0) goto loc_800421E8;
    s3 = 0x80070000;                                    // Result = 80070000
    s3 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    s1 = 0;                                             // Result = 00000000
loc_80042184:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a0 + 0x18);
    v1 = s1 + v0;
    v0 = lw(v1 + 0x50);
    v0 &= 7;
    s1 += 0x54;
    if (v0 == 0) goto loc_800421D8;
    v0 = lw(v1 + 0x4C);
    v0 <<= 2;
    v0 += s3;
    v0 = lw(v0);
    v0 = lw(v0 + 0x4);
    ptr_call(v0);
loc_800421D8:
    s0++;
    v0 = (i32(s0) < i32(s2));
    if (v0 != 0) goto loc_80042184;
loc_800421E8:
    free_mem_if_mine();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x58F8);                                 // Store to: gbWess_module_loaded (800758F8)
loc_800421F8:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void wess_memcpy() noexcept {
loc_80042218:
    sp -= 8;
    v1 = a2 - 1;
    if (a2 == 0) goto loc_80042240;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80042228:
    v0 = lbu(a1);
    a1++;
    v1--;
    sb(v0, a0);
    a0++;
    if (v1 != a2) goto loc_80042228;
loc_80042240:
    sp += 8;
    return;
}

void conditional_read() noexcept {
loc_8004224C:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s0, sp + 0x10);
    s0 = a2;
    sw(ra, sp + 0x18);
    if (a0 == 0) goto loc_800422B8;
    a0 = lw(s1);
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    wess_memcpy();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 += s0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 = lw(s1);
    v0 += s0;
    v1 = v0 & 1;
    v1 += v0;
    v0 = v1 & 2;
    v0 += v1;
    sw(v0, s1);
    goto loc_800422D0;
loc_800422B8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 += s0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
loc_800422D0:
    v0 = 1;                                             // Result = 00000001
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_load_module() noexcept {
loc_800422EC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    sp -= 0x80;
    sw(s2, sp + 0x60);
    s2 = a0;
    sw(s1, sp + 0x5C);
    s1 = a1;
    sw(s0, sp + 0x58);
    s0 = a2;
    sw(s3, sp + 0x64);
    sw(ra, sp + 0x7C);
    sw(fp, sp + 0x78);
    sw(s7, sp + 0x74);
    sw(s6, sp + 0x70);
    sw(s5, sp + 0x6C);
    sw(s4, sp + 0x68);
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5904);                                // Store to: gWess_mem_limit (80075904)
    s3 = a3;
    if (v0 == 0) goto loc_80042344;
    wess_unload_module();
loc_80042344:
    a0 = s3;
    get_num_Wess_Sound_Drivers();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E4);                                // Store to: gWess_num_sd (800758E4)
    v0 = 1;                                             // Result = 00000001
    if (s1 != 0) goto loc_8004238C;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5908);                                // Store to: gbWess_wmd_mem_is_mine (80075908)
    a0 = s0;
    wess_malloc();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x590C);                                // Store to: gpWess_wmd_mem (8007590C)
    if (v0 != 0) goto loc_8004239C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    goto loc_80043090;
loc_8004238C:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5908);                                 // Store to: gbWess_wmd_mem_is_mine (80075908)
    at = 0x80070000;                                    // Result = 80070000
    sw(s1, at + 0x590C);                                // Store to: gpWess_wmd_mem (8007590C)
loc_8004239C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x590C);                               // Load from: gpWess_wmd_mem (8007590C)
    a1 = s0;
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x5914);                                // Store to: gWess_wmd_size (80075914)
    zeroset();
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5900);                                 // Store to: gWess_max_seq_num (80075900)
    Is_System_Active();
    if (v0 == 0) goto loc_800423D8;
    t3 = 4;                                             // Result = 00000004
    if (s2 != 0) goto loc_800423F0;
loc_800423D8:
    free_mem_if_mine();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58F8);                               // Load from: gbWess_module_loaded (800758F8)
    goto loc_80043090;
loc_800423F0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x590C);                               // Load from: gpWess_wmd_mem (8007590C)
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x58F0);                               // Load from: gpWess_fp_wmd_file (800758F0)
    a1 = s2;
    sb(t3, sp + 0x18);
    sb(0, sp + 0x20);
    v1 = v0 + 0x38;
    sw(v1, v0 + 0xC);
    a0 = lw(v0 + 0xC);
    a2 = 0x10;                                          // Result = 00000010
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x58EC);                                // Store to: gpWess_tmp_fp_wmd_file_2 (800758EC)
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    sw(v0, sp + 0x10);
    sw(v1, sp + 0x10);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x5954;                                       // Result = gWess_Millicount (80075954)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x78A8);                                // Store to: gpWess_pm_stat (800A8758)
    sw(a3, v0 + 0x34);
    sw(v1, v0);
    v0 += 0x4C;
    sw(v0, sp + 0x10);
    wess_memcpy();
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    sb(0, sp + 0x28);
    a2 = lw(a1 + 0xC);
    v1 = 0x80010000;                                    // Result = 80010000
    v1 = lw(v1 + 0x1760);                               // Load from: STR_SPSX[0] (80011760)
    a0 = lw(a2);
    v0 += 0x10;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 = 1;                                             // Result = 00000001
    if (a0 != v1) goto loc_800424A0;
    v1 = lw(a2 + 0x4);
    if (v1 == v0) goto loc_800424B0;
loc_800424A0:
    free_mem_if_mine();
loc_800424A8:
    v0 = 0;                                             // Result = 00000000
    goto loc_80043090;
loc_800424B0:
    v1 = lw(sp + 0x10);
    v0 = lw(a1 + 0xC);
    sw(v1, a1 + 0x20);
    a0 = lbu(v0 + 0xB);
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    a0 = lw(a1 + 0xC);
    v1 += v0;
    sw(v1, sp + 0x10);
    sw(v1, a1 + 0x28);
    a0 = lbu(a0 + 0xC);
    v0 = a0 << 2;
    v0 += a0;
    v0 <<= 4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lbu(a0 + 0x58E4);                              // Load from: gWess_num_sd (800758E4)
    v1 += v0;
    sw(v1, sp + 0x10);
    sb(a0, a1 + 0x8);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(sp + 0x10);
    v1 = lbu(a0 + 0x8);
    sw(v0, a0 + 0x14);
    v0 += v1;
    v1 = v0 & 1;
    v0 += v1;
    v1 = v0 & 2;
    v0 += v1;
    sw(v0, sp + 0x10);
    v0 = -1;                                            // Result = FFFFFFFF
    v1 = lbu(a0 + 0x8);
    a1 = lw(a0 + 0x14);
    v1--;
    a0 = 0x80;                                          // Result = 00000080
    if (v1 == v0) goto loc_8004255C;
loc_8004254C:
    sb(a0, a1);
    v1--;
    a1++;
    if (v1 != v0) goto loc_8004254C;
loc_8004255C:
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 = lw(a2 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 = lw(sp + 0x10);
    v1 = lbu(a2 + 0x8);
    sw(a0, a2 + 0x18);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 += v1;
    v0 <<= 2;
    a0 += v0;
    sw(a0, sp + 0x10);
    if (s3 == 0) goto loc_80042708;
    a1 = lbu(a2 + 0x8);
    v0 = -1;                                            // Result = FFFFFFFF
    a1--;
    {
        const bool bJump = (a1 == v0);
        v0 = a1 << 2;
        if (bJump) goto loc_80042708;
    }
    t2 = a2;
    t1 = v0 + s3;
    v0 += a1;
    v0 <<= 2;
    v0 += a1;
    t0 = v0 << 2;
loc_800425BC:
    v0 = lw(t1);
    v0 = lw(v0);
    if (v0 == 0) goto loc_800426F4;
    a3 = 0;                                             // Result = 00000000
loc_800425D8:
    v0 = lw(t2 + 0x18);
    v1 = lw(t1);
    v0 += t0;
    v1 += a3;
    v1 = lw(v1);
    v0 += a3;
    sw(v1, v0 + 0x24);
    v0 = lw(t2 + 0x18);
    v1 = lw(t1);
    v0 += t0;
    v1 += a3;
    v1 = lw(v1 + 0x4);
    v0 += a3;
    sw(v1, v0 + 0x28);
    v0 = lw(t2 + 0x18);
    a2 = t0 + v0;
    a0 = a3 + a2;
    v1 = lw(a0 + 0x24);
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8004263C;
    }
    v0 = lw(a0 + 0x28);
    sw(v0, a2 + 0x4C);
    goto loc_800426D0;
loc_8004263C:
    if (v1 != v0) goto loc_80042664;
    v1 = -2;                                            // Result = FFFFFFFE
    v0 = lw(a2 + 0x50);
    a0 = lw(a0 + 0x28);
    v1 &= v0;
    v0 &= 1;
    v0 |= a0;
    v0 &= 1;
    goto loc_800426C8;
loc_80042664:
    v0 = 3;                                             // Result = 00000003
    if (v1 != v0) goto loc_80042698;
    v1 = -3;                                            // Result = FFFFFFFD
    v0 = lw(a2 + 0x50);
    a0 = lw(a0 + 0x28);
    v1 &= v0;
    v0 >>= 1;
    v0 &= 1;
    v0 |= a0;
    v0 &= 1;
    v0 <<= 1;
    goto loc_800426C8;
loc_80042698:
    v0 = 4;                                             // Result = 00000004
    {
        const bool bJump = (v1 != v0);
        v0 = a1 << 2;
        if (bJump) goto loc_800426D4;
    }
    v1 = -5;                                            // Result = FFFFFFFB
    v0 = lw(a2 + 0x50);
    a0 = lw(a0 + 0x28);
    v1 &= v0;
    v0 >>= 2;
    v0 &= 1;
    v0 |= a0;
    v0 &= 1;
    v0 <<= 2;
loc_800426C8:
    v1 |= v0;
    sw(v1, a2 + 0x50);
loc_800426D0:
    v0 = a1 << 2;
loc_800426D4:
    v0 += s3;
    v0 = lw(v0);
    a3 += 8;
    v0 += a3;
    v0 = lw(v0);
    if (v0 != 0) goto loc_800425D8;
loc_800426F4:
    t1 -= 4;
    a1--;
    v0 = -1;                                            // Result = FFFFFFFF
    t0 -= 0x54;
    if (a1 != v0) goto loc_800425BC;
loc_80042708:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    sb(0, v0 + 0x7);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0 + 0xC);
    s1 = lbu(v0 + 0xA);
    v0 = -1;                                            // Result = FFFFFFFF
    s1--;
    if (s1 == v0) goto loc_80042934;
    s2 = -1;                                            // Result = FFFFFFFF
    s0 = 0x80080000;                                    // Result = 80080000
    s0 -= 0x1038;                                       // Result = 8007EFC8
    s3 = s0 - 4;                                        // Result = 8007EFC4
loc_80042750:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x103C;                                       // Result = 8007EFC4
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    a2 = 0x1C;                                          // Result = 0000001C
    wess_memcpy();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += 0x1C;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    a1 = lbu(v1 + 0x8);
    a1--;
    v0 = a1 << 2;
    if (a1 == s2) goto loc_80042928;
    v0 += a1;
    v0 <<= 2;
    v0 += a1;
    a3 = v0 << 2;
loc_800427A8:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0 + 0x18);
    v1 = lbu(s0);                                       // Load from: 8007EFC8
    a2 = a3 + v0;
    v0 = lw(a2 + 0x4C);
    a1--;
    if (v1 != v0) goto loc_80042920;
    v0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    v1 = lw(s0);                                        // Load from: 8007EFC8
    a0 = lw(s0 + 0x4);                                  // Load from: 8007EFCC
    a1 = lw(s0 + 0x8);                                  // Load from: 8007EFD0
    sw(v0, a2);
    sw(v1, a2 + 0x4);
    sw(a0, a2 + 0x8);
    sw(a1, a2 + 0xC);
    v0 = lw(s0 + 0xC);                                  // Load from: 8007EFD4
    v1 = lw(s0 + 0x10);                                 // Load from: 8007EFD8
    a0 = lw(s0 + 0x14);                                 // Load from: 8007EFDC
    sw(v0, a2 + 0x10);
    sw(v1, a2 + 0x14);
    sw(a0, a2 + 0x18);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v1 = lbu(s0 + 0x1);                                 // Load from: 8007EFC9
    v0 = lbu(a0 + 0x7);
    v0 += v1;
    sb(v0, a0 + 0x7);
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v1 = lw(sp + 0x10);
    v0 = lw(a1 + 0x18);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 += a3;
    sw(v1, v0 + 0x1C);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x58EC);                               // Load from: gpWess_tmp_fp_wmd_file_2 (800758EC)
    v0 = lw(a1 + 0x18);
    s6 = a0 - v1;
    v0 += a3;
    sw(s6, v0 + 0x20);
    v1 = lh(s0 + 0x4);                                  // Load from: 8007EFCC
    v0 = lh(s0 + 0x6);                                  // Load from: 8007EFCE
    mult(v1, v0);
    a0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    a1 = sp + 0x10;
    a2 = lo;
    a0 &= 1;
    conditional_read();
    a1 = sp + 0x10;
    if (v0 == 0) goto loc_800424A8;
    v0 = lh(s0 + 0x8);                                  // Load from: 8007EFD0
    v1 = lh(s0 + 0xA);                                  // Load from: 8007EFD2
    mult(v0, v1);
    a0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    a2 = lo;
    a0 &= 2;
    conditional_read();
    a1 = sp + 0x10;
    if (v0 == 0) goto loc_800424A8;
    v0 = lh(s0 + 0xC);                                  // Load from: 8007EFD4
    v1 = lh(s0 + 0xE);                                  // Load from: 8007EFD6
    mult(v0, v1);
    a0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    a2 = lo;
    a0 &= 4;
    conditional_read();
    a1 = sp + 0x10;
    if (v0 == 0) goto loc_800424A8;
    v0 = lh(s0 + 0x10);                                 // Load from: 8007EFD8
    v1 = lh(s0 + 0x12);                                 // Load from: 8007EFDA
    mult(v0, v1);
    a0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    a2 = lo;
    a0 &= 8;
    conditional_read();
    a1 = sp + 0x10;
    if (v0 == 0) goto loc_800424A8;
    a0 = lw(s0 - 0x4);                                  // Load from: 8007EFC4
    a2 = lw(s3 + 0x18);                                 // Load from: 8007EFDC
    a0 &= 0x10;
    conditional_read();
    s1--;
    if (v0 != 0) goto loc_8004292C;
    v0 = 0;                                             // Result = 00000000
    goto loc_80043090;
loc_80042920:
    a3 -= 0x54;
    if (a1 != s2) goto loc_800427A8;
loc_80042928:
    s1--;
loc_8004292C:
    if (s1 != s2) goto loc_80042750;
loc_80042934:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lbu(a0 + 0x7);
    v1 = v0 << 1;
    v1 += v0;
    v0 = lw(sp + 0x10);
    v1 <<= 3;
    v1 += v0;
    sw(v1, sp + 0x10);
    v1 = lbu(a0 + 0x8);
    sw(v0, a0 + 0x30);
    if (v1 == 0) goto loc_80042A78;
    t1 = v1;
    a3 = 0;                                             // Result = 00000000
    v0 = lw(a0 + 0x18);
    v1 = lbu(a0 + 0x7);
    a1 = lbu(v0 + 0x5);
    s2 = 0;                                             // Result = 00000000
    if (i32(v1) <= 0) goto loc_80042A78;
    t2 = -1;                                            // Result = FFFFFFFF
    a2 = 0;                                             // Result = 00000000
    t0 = 0;                                             // Result = 00000000
loc_80042998:
    if (t1 == 0) goto loc_80042A58;
    a1--;
    if (a1 == t2) goto loc_800429F4;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v1 + 0x18);
    v1 = lw(v1 + 0x30);
    v0 += t0;
    v0 = lbu(v0 + 0x4);
    v1 += a2;
    sb(v0, v1 + 0x1);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0 + 0x30);
    v0 += a2;
    sb(a3, v0 + 0x2);
    a3++;                                               // Result = 00000001
    goto loc_80042A58;
loc_800429F4:
    t1--;
    t0 += 0x54;                                         // Result = 00000054
    if (t1 == 0) goto loc_80042A58;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a0 + 0x18);
    v1 = t0 + v0;
    a1 = lbu(v1 + 0x5);
    a1--;
    a3 = 0;                                             // Result = 00000000
    if (a1 == t2) goto loc_80042A58;
    v0 = lw(a0 + 0x30);
    v1 = lbu(v1 + 0x4);
    v0 += a2;
    sb(v1, v0 + 0x1);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0 + 0x30);
    a3 = 1;                                             // Result = 00000001
    v0 += a2;
    sb(0, v0 + 0x2);
loc_80042A58:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lbu(v0 + 0x7);
    s2++;
    v0 = (i32(s2) < i32(v0));
    a2 += 0x18;
    if (v0 != 0) goto loc_80042998;
loc_80042A78:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a1 = lw(sp + 0x10);
    v0 = lw(a0 + 0xC);
    sw(a1, v0 + 0x10);
    v0 = lw(a0 + 0xC);
    v1 = lh(v0 + 0x8);
    s2 = 0;                                             // Result = 00000000
    v0 = v1 << 2;
    v0 += v1;
    v1 = lw(a0 + 0xC);
    v0 <<= 2;
    v1 = lh(v1 + 0x8);
    v0 += a1;
    sw(v0, sp + 0x10);
    if (i32(v1) <= 0) goto loc_80042DB0;
    s7 = -1;                                            // Result = FFFFFFFF
    fp = 0x80080000;                                    // Result = 80080000
    fp -= 0x1020;                                       // Result = 8007EFE0
    s4 = fp + 0x12;                                     // Result = 8007EFF2
    s5 = 0;                                             // Result = 00000000
loc_80042AD4:
    a2 = 4;                                             // Result = 00000004
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 = lw(v0 + 0xC);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x58EC);                               // Load from: gpWess_tmp_fp_wmd_file_2 (800758EC)
    a0 = lw(v0 + 0x10);
    s6 = a1 - v1;
    a0 += s5;
    wess_memcpy();
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s3 = 0;                                             // Result = 00000000
    v0 = lw(v0 + 0xC);
    v1 = lw(v0 + 0x10);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v1 += s5;
    s1 = lh(v1);
    v0 += 4;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    s1--;
    s0 = 0;                                             // Result = 00000000
    if (s1 == s7) goto loc_80042D1C;
    a0 = fp;                                            // Result = 8007EFE0
loc_80042B48:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    a2 = 0x18;                                          // Result = 00000018
    wess_memcpy();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v1 = lbu(fp);                                       // Load from: 8007EFE0
    v0 += 0x18;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    t0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_80042C60;
    v0 = 0x32;                                          // Result = 00000032
    if (v1 == v0) goto loc_80042C60;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a1 = lbu(v0 + 0x8);
    a1--;
    if (a1 == s7) goto loc_80042C78;
    t1 = v1;
    a3 = v0;
    v1 = lw(v0 + 0x18);
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    v0 += a1;
    a0 = v0 << 2;
    a2 = a0 + v1;
loc_80042BC8:
    v0 = lw(a2 + 0x4C);
    if (t1 != v0) goto loc_80042C68;
    v1 = lbu(fp + 0x4);                                 // Load from: 8007EFE4
    v0 = 3;                                             // Result = 00000003
    if (v1 == 0) goto loc_80042BF0;
    if (v1 != v0) goto loc_80042C04;
loc_80042BF0:
    v0 = lw(a2 + 0x50);
    v0 &= 1;
    if (v0 != 0) goto loc_80042C60;
loc_80042C04:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lbu(v1 - 0x101C);                              // Load from: 8007EFE4
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80042C38;
    }
    v0 = lw(a3 + 0x18);
    v0 += a0;
    v0 = lw(v0 + 0x50);
    v0 &= 2;
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80042C60;
    }
loc_80042C38:
    if (v1 != v0) goto loc_80042C68;
    v0 = lw(a3 + 0x18);
    v0 += a0;
    v0 = lw(v0 + 0x50);
    v0 &= 4;
    if (v0 == 0) goto loc_80042C68;
loc_80042C60:
    t0 = 1;                                             // Result = 00000001
    goto loc_80042C78;
loc_80042C68:
    a0 -= 0x54;
    a1--;
    a2 -= 0x54;
    if (a1 != s7) goto loc_80042BC8;
loc_80042C78:
    if (t0 == 0) goto loc_80042CEC;
    v0 = lh(s4);                                        // Load from: 8007EFF2
    v1 = lw(s4 + 0x2);                                  // Load from: 8007EFF4
    t3 = lbu(sp + 0x20);
    v0 <<= 2;
    v1 += 0x20;
    v0 += v1;
    s0 += v0;
    v0 = s0 & 1;
    s0 += v0;
    v1 = s0 & 2;
    v0 = lbu(s4 - 0x11);                                // Load from: 8007EFE1
    v0 = (t3 < v0);
    s0 += v1;
    if (v0 == 0) goto loc_80042CC8;
    t3 = lbu(s4 - 0x11);                                // Load from: 8007EFE1
    sb(t3, sp + 0x20);
loc_80042CC8:
    v0 = lbu(s4 - 0x6);                                 // Load from: 8007EFEC
    t3 = lbu(sp + 0x28);
    v0 = (t3 < v0);
    s3++;                                               // Result = 00000001
    if (v0 == 0) goto loc_80042CEC;
    t3 = lbu(s4 - 0x6);                                 // Load from: 8007EFEC
    sb(t3, sp + 0x28);
loc_80042CEC:
    s1--;
    v0 = lh(s4);                                        // Load from: 8007EFF2
    v1 = lw(s4 + 0x2);                                  // Load from: 8007EFF4
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x58E8);                               // Load from: gpWess_tmp_fp_wmd_file_1 (800758E8)
    v0 <<= 2;
    v0 += v1;
    v0 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x58E8);                                // Store to: gpWess_tmp_fp_wmd_file_1 (800758E8)
    a0 = fp;                                            // Result = 8007EFE0
    if (s1 != s7) goto loc_80042B48;
loc_80042D1C:
    v0 = lbu(sp + 0x18);
    v0 = (i32(v0) < i32(s3));
    if (v0 == 0) goto loc_80042D34;
    sb(s3, sp + 0x18);
loc_80042D34:
    s2++;
    if (s3 != 0) goto loc_80042D40;
    s0 = 0x24;                                          // Result = 00000024
loc_80042D40:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s5;
    sw(s3, v0 + 0x10);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s5;
    sw(s0, v0 + 0xC);
    v0 = lw(v1 + 0xC);
    v0 = lw(v0 + 0x10);
    v0 += s5;
    sw(s6, v0 + 0x8);
    v0 = lw(v1 + 0xC);
    v0 = lh(v0 + 0x8);
    v0 = (i32(s2) < i32(v0));
    s5 += 0x14;
    if (v0 != 0) goto loc_80042AD4;
loc_80042DB0:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 = lw(sp + 0x10);
    v0 = lw(v1 + 0xC);
    sw(a0, v1 + 0x10);
    v0 = lbu(v0 + 0xF);
    t3 = lbu(sp + 0x18);
    v0 <<= 3;
    v0 += a0;
    sw(v0, sp + 0x10);
    sb(t3, v1 + 0x1C);
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 = lw(a2 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a2 + 0xC);
    v0 = lbu(v0 + 0xB);
    s2 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80042EE0;
    t0 = 0xFF;                                          // Result = 000000FF
    a3 = 0;                                             // Result = 00000000
loc_80042E08:
    v0 = lw(a2 + 0x20);
    v1 = lw(sp + 0x10);
    v0 += a3;
    sw(v1, v0 + 0x10);
    v0 = lw(a2 + 0xC);
    v0 = lbu(v0 + 0xD);
    v0 += v1;
    v1 = v0 & 1;
    v1 += v0;
    a0 = v1 & 2;
    v0 = lw(a2 + 0x20);
    a0 += v1;
    v0 += a3;
    sw(a0, v0 + 0x14);
    v0 = lw(a2 + 0xC);
    sw(a0, sp + 0x10);
    v0 = lbu(v0 + 0xE);
    s1 = lbu(sp + 0x18);
    v0 += a0;
    v1 = v0 & 1;
    v1 += v0;
    v0 = v1 & 2;
    v0 += v1;
    a1 = v0;
    a0 = s1 + a1;
    s1--;
    v1 = a0 & 1;
    v0 = lw(a2 + 0x20);
    v1 += a0;
    sw(a1, sp + 0x10);
    v0 += a3;
    sw(a1, v0 + 0xC);
    v0 = v1 & 2;
    v0 += v1;
    sw(v0, sp + 0x10);
    v0 = -1;                                            // Result = FFFFFFFF
    if (s1 == v0) goto loc_80042EB8;
loc_80042EA8:
    sb(t0, a1);
    s1--;
    a1++;
    if (s1 != v0) goto loc_80042EA8;
loc_80042EB8:
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 = lw(a2 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a2 + 0xC);
    v0 = lbu(v0 + 0xB);
    s2++;
    v0 = (i32(s2) < i32(v0));
    a3 += 0x18;
    if (v0 != 0) goto loc_80042E08;
loc_80042EE0:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    t3 = lbu(sp + 0x20);
    sb(t3, v0 + 0x2C);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    t3 = lbu(sp + 0x28);
    sb(t3, v0 + 0x24);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a0 + 0xC);
    v0 = lbu(v0 + 0xC);
    s2 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80042F94;
    a2 = 0;                                             // Result = 00000000
loc_80042F30:
    v0 = lw(a0 + 0x28);
    v0 += a2;
    sb(s2, v0 + 0x1);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(a0 + 0x28);
    a1 = lw(sp + 0x10);
    v0 += a2;
    sw(a1, v0 + 0x3C);
    v1 = lbu(a0 + 0x24);
    v0 = lw(a0 + 0x28);
    v1 <<= 2;
    v1 += a1;
    v0 += a2;
    sw(v1, v0 + 0x44);
    v0 = lw(a0 + 0xC);
    s2++;
    sw(v1, sp + 0x10);
    v0 = lbu(v0 + 0xC);
    v0 = (i32(s2) < i32(v0));
    a2 += 0x50;
    if (v0 != 0) goto loc_80042F30;
loc_80042F94:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5920);                               // Load from: gWess_CmdFuncArr[0] (80075920)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v0);
    s2 = 0;                                             // Result = 00000000
    ptr_call(v0);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lbu(a0 + 0x8);
    {
        const bool bJump = (i32(v0) <= 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80043044;
    }
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    s0 = 0;                                             // Result = 00000000
loc_80042FDC:
    v0 = lw(a0 + 0x18);
    v1 = s0 + v0;
    v0 = lw(v1 + 0x50);
    v0 &= 7;
    s0 += 0x54;
    if (v0 == 0) goto loc_80043024;
    v0 = lw(v1 + 0x4C);
    v0 <<= 2;
    v0 += s1;
    v0 = lw(v0);
    v0 = lw(v0);
    ptr_call(v0);
loc_80043024:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lbu(a0 + 0x8);
    s2++;
    v0 = (i32(s2) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80042FDC;
    }
loc_80043044:
    a1 = lw(sp + 0x10);
    v1 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x58F8);                                // Store to: gbWess_module_loaded (800758F8)
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 = a1 & 1;
    a0 += a1;
    a1 = lw(v1 + 0xC);
    v1 = a0 & 2;
    a1 = lh(a1 + 0x8);
    v1 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5910);                                // Store to: gpWess_wmd_end (80075910)
    sw(v1, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x5900);                                // Store to: gWess_max_seq_num (80075900)
loc_80043090:
    ra = lw(sp + 0x7C);
    fp = lw(sp + 0x78);
    s7 = lw(sp + 0x74);
    s6 = lw(sp + 0x70);
    s5 = lw(sp + 0x6C);
    s4 = lw(sp + 0x68);
    s3 = lw(sp + 0x64);
    s2 = lw(sp + 0x60);
    s1 = lw(sp + 0x5C);
    s0 = lw(sp + 0x58);
    sp += 0x80;
    return;
}

void filltrackstat() noexcept {
loc_800430C4:
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s1, sp + 0x14);
    v0 = lw(s0);
    v1 = -0x11;                                         // Result = FFFFFFEF
    v0 |= 1;
    v0 &= v1;
    v1 = -0x21;                                         // Result = FFFFFFDF
    v0 &= v1;
    v1 = -0x41;                                         // Result = FFFFFFBF
    v0 &= v1;
    v1 = -0x81;                                         // Result = FFFFFF7F
    v0 &= v1;
    sw(v0, s0);
    v0 = lbu(s2);
    sb(v0, s0 + 0x3);
    v0 = lbu(s2 + 0x2);
    sb(v0, s0 + 0x8);
    v0 = lbu(s2 + 0x4);
    sb(0, s0 + 0x10);
    sb(v0, s0 + 0x13);
    v0 = lbu(s2 + 0x1);
    sb(v0, s0 + 0x11);
    v1 = lhu(s2 + 0xE);
    v0 = lw(s0 + 0x3C);
    sw(0, s0 + 0x20);
    sw(0, s0 + 0x24);
    sw(0, s0 + 0x28);
    sw(v0, s0 + 0x40);
    sh(v1, s0 + 0x14);
    v0 = lhu(s2 + 0x12);
    sh(v0, s0 + 0x18);
    v0 = lw(s2 + 0x14);
    sw(v0, s0 + 0x48);
    v0 = lbu(s2 + 0xD);
    s3 = a2;
    sb(v0, s0 + 0x12);
    if (s3 == 0) goto loc_80043194;
    v0 = lw(s3);
    s1 = v0;
    if (v0 != 0) goto loc_80043198;
loc_80043194:
    s1 = 0;                                             // Result = 00000000
loc_80043198:
    v0 = s1 & 1;
    if (v0 == 0) goto loc_800431B0;
    v0 = lbu(s3 + 0x4);
    sb(v0, s0 + 0xC);
    goto loc_800431BC;
loc_800431B0:
    v0 = lbu(s2 + 0xA);
    sb(v0, s0 + 0xC);
loc_800431BC:
    v0 = s1 & 2;
    if (v0 == 0) goto loc_800431D4;
    v0 = lbu(s3 + 0x5);
    sb(v0, s0 + 0xD);
    goto loc_800431E0;
loc_800431D4:
    v0 = lbu(s2 + 0xB);
    sb(v0, s0 + 0xD);
loc_800431E0:
    v0 = s1 & 4;
    if (v0 == 0) goto loc_800431F8;
    v0 = lhu(s3 + 0x6);
    sh(v0, s0 + 0xA);
    goto loc_80043204;
loc_800431F8:
    v0 = lhu(s2 + 0x6);
    sh(v0, s0 + 0xA);
loc_80043204:
    v0 = s1 & 8;                                        // Result = 00000000
    if (v0 == 0) goto loc_8004321C;
    v0 = lhu(s3 + 0x8);
    sh(v0, s0 + 0xE);
    goto loc_80043228;
loc_8004321C:
    v0 = lhu(s2 + 0x8);
    sh(v0, s0 + 0xE);
loc_80043228:
    v0 = s1 & 0x10;                                     // Result = 00000000
    if (v0 == 0) goto loc_8004325C;
    v0 = lbu(s0 + 0x12);
    v1 = lbu(s3 + 0xA);
    v0 = i32(v0) >> v1;
    v0 &= 1;
    if (v0 == 0) goto loc_8004325C;
    v0 = lw(s0);
    v0 |= 2;
    goto loc_80043268;
loc_8004325C:
    v0 = lw(s0);
    v1 = -3;                                            // Result = FFFFFFFD
    v0 &= v1;
loc_80043268:
    sw(v0, s0);
    v0 = s1 & 0x20;                                     // Result = 00000000
    if (v0 == 0) goto loc_80043284;
    v0 = lhu(s3 + 0xC);
    goto loc_80043288;
loc_80043284:
    v0 = lhu(s2 + 0x10);
loc_80043288:
    sh(v0, s0 + 0x16);
    v0 = GetIntsPerSec();
    v0 <<= 16;
    a1 = lh(s0 + 0x14);
    a2 = lh(s0 + 0x16);
    a0 = u32(i32(v0) >> 16);
    CalcPartsPerInt();
    sw(v0, s0 + 0x1C);
    v0 = s1 & 0x40;                                     // Result = 00000000
    v1 = -0x11;                                         // Result = FFFFFFEF
    if (v0 == 0) goto loc_800432D4;
    v0 = lw(s0 + 0x28);
    a0 = lw(s3 + 0x10);
    v1 = lw(s0);
    v0 += a0;
    v1 |= 0x10;
    sw(v0, s0 + 0x2C);
    sw(v1, s0);
    goto loc_800432E4;
loc_800432D4:
    v0 = lw(s0);
    v0 &= v1;
    sw(v0, s0);
loc_800432E4:
    v0 = s1 & 0x80;                                     // Result = 00000000
    v1 = -0x21;                                         // Result = FFFFFFDF
    if (v0 == 0) goto loc_800432FC;
    v0 = lw(s0);
    v0 |= 0x20;
    goto loc_80043308;
loc_800432FC:
    v0 = lw(s0);
    v0 &= v1;
loc_80043308:
    sw(v0, s0);
    v0 = s1 & 0x100;                                    // Result = 00000000
    if (v0 == 0) goto loc_80043324;
    v0 = lbu(s3 + 0xB);
    sb(v0, s0 + 0x9);
    goto loc_80043330;
loc_80043324:
    v0 = lbu(s2 + 0x5);
    sb(v0, s0 + 0x9);
loc_80043330:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void assigntrackstat() noexcept {
loc_80043350:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x14);
    s0 = a0;
    sw(v0, s0 + 0x4C);
    v0 = lhu(s1 + 0x12);
    sh(v0, s0 + 0x1A);
    a0 = lw(s1 + 0x1C);
    a1 = s0 + 4;
    sw(a0, s0 + 0x30);
    Read_Vlq();
    sw(v0, s0 + 0x34);
    v0 = lw(s1 + 0x18);
    sw(v0, s0 + 0x38);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_seq_structrig() noexcept {
loc_800433B4:
    sp -= 0x58;
    sw(a0, sp + 0x10);
    sw(a1, sp + 0x18);
    a0 = lw(sp + 0x18);
    sw(ra, sp + 0x54);
    sw(fp, sp + 0x50);
    sw(s7, sp + 0x4C);
    sw(s6, sp + 0x48);
    sw(s5, sp + 0x44);
    sw(s4, sp + 0x40);
    sw(s3, sp + 0x3C);
    sw(s2, sp + 0x38);
    sw(s1, sp + 0x34);
    sw(s0, sp + 0x30);
    sw(a2, sp + 0x20);
    sw(a3, sp + 0x28);
    Is_Seq_Num_Valid();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80043678;
    }
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v1 + 0xC);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s7 = lbu(v0 + 0xB);
    a0 = s7 & 0xFF;
    s4 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_8004346C;
    a1 = lw(v1 + 0x20);
    v0 = s4 & 0xFF;                                     // Result = 00000000
loc_80043434:
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v1 += a1;
    v0 = lw(v1);
    v0 &= 1;
    if (v0 == 0) goto loc_8004346C;
    s4++;
    v0 = s4 & 0xFF;
    v0 = (v0 < a0);
    {
        const bool bJump = (v0 != 0);
        v0 = s4 & 0xFF;
        if (bJump) goto loc_80043434;
    }
loc_8004346C:
    a0 = s4 & 0xFF;
    s5 = 0;                                             // Result = 00000000
    if (a0 != s7) goto loc_8004348C;
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80043484:
    v0 = 0;                                             // Result = 00000000
    goto loc_80043678;
loc_8004348C:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    t0 = lw(sp + 0x10);
    v0 = lw(v1 + 0xC);
    fp = lhu(t0);
    v1 = lw(v1 + 0x20);
    s7 = lbu(v0 + 0xC);
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    s2 = v0 + v1;
    s6 = lw(s2 + 0xC);
    s3 = 0;                                             // Result = 00000000
    if (s7 == 0) goto loc_800435C4;
    v1 = s3 & 0xFF;                                     // Result = 00000000
loc_800434C8:
    v0 = v1 << 2;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 += v1;
    v1 = lw(a0 + 0x28);
    v0 <<= 4;
    s1 = v0 + v1;
    v0 = lw(s1);
    v0 &= 1;
    a0 = s1;
    if (v0 != 0) goto loc_800435B0;
    t0 = lw(sp + 0x10);
    s0 = s5 & 0xFF;                                     // Result = 00000000
    v0 = lw(t0 + 0x4);
    s0 <<= 5;                                           // Result = 00000000
    sb(s4, s1 + 0x2);
    a2 = lw(sp + 0x68);
    s0 += v0;
    a1 = s0;
    filltrackstat();
    a0 = s1;
    a1 = s0;
    assigntrackstat();
    t0 = lw(sp + 0x28);
    v0 = -5;                                            // Result = FFFFFFFB
    if (t0 == 0) goto loc_8004354C;
    v0 = lw(s1);
    v0 |= 0xC;
    sw(v0, s1);
    goto loc_80043574;
loc_8004354C:
    v1 = lw(s1);
    v1 &= v0;
    v0 = -9;                                            // Result = FFFFFFF7
    v1 &= v0;
    sw(v1, s1);
    v0 = lbu(s2 + 0x5);
    v0++;
    sb(v0, s2 + 0x5);
loc_80043574:
    s5++;                                               // Result = 00000001
    v0 = lbu(s2 + 0x4);
    v1 = fp - 1;
    v0++;
    sb(v0, s2 + 0x4);
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    fp = v1;
    v0 = lbu(a0 + 0x5);
    v1 <<= 16;
    v0++;
    sb(v0, a0 + 0x5);
    sb(s3, s6);
    s6++;
    if (v1 == 0) goto loc_800435C4;
loc_800435B0:
    s3++;
    v0 = s3 & 0xFF;
    v0 = (v0 < s7);
    v1 = s3 & 0xFF;
    if (v0 != 0) goto loc_800434C8;
loc_800435C4:
    v0 = s5 & 0xFF;
    if (v0 == 0) goto loc_8004365C;
    t0 = lhu(sp + 0x18);
    sh(t0, s2 + 0x2);
    t0 = lw(sp + 0x20);
    sw(t0, s2 + 0x8);
    t0 = lw(sp + 0x28);
    v1 = -3;                                            // Result = FFFFFFFD
    if (t0 == 0) goto loc_80043610;
    v0 = lw(s2);
    v0 |= 2;
    sw(v0, s2);
    sb(0, s2 + 0x1);
    goto loc_80043628;
loc_80043610:
    v0 = lw(s2);
    v0 &= v1;
    sw(v0, s2);
    v0 = 1;                                             // Result = 00000001
    sb(v0, s2 + 0x1);
loc_80043628:
    v0 = 0x80;                                          // Result = 00000080
    sb(v0, s2 + 0x6);
    v0 = 0x40;                                          // Result = 00000040
    sb(v0, s2 + 0x7);
    v0 = lw(s2);
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 |= 1;
    sw(v0, s2);
    v0 = lbu(v1 + 0x4);
    v0++;
    sb(v0, v1 + 0x4);
loc_8004365C:
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
    v0 = s5 & 0xFF;
    {
        const bool bJump = (v0 == 0);
        v0 = s4 & 0xFF;                                 // Result = 00000000
        if (bJump) goto loc_80043484;
    }
    v0++;                                               // Result = 00000001
loc_80043678:
    ra = lw(sp + 0x54);
    fp = lw(sp + 0x50);
    s7 = lw(sp + 0x4C);
    s6 = lw(sp + 0x48);
    s5 = lw(sp + 0x44);
    s4 = lw(sp + 0x40);
    s3 = lw(sp + 0x3C);
    s2 = lw(sp + 0x38);
    s1 = lw(sp + 0x34);
    s0 = lw(sp + 0x30);
    sp += 0x58;
    return;
}

void wess_seq_trigger() noexcept {
loc_800436AC:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = 0;                                             // Result = 00000000
    wess_seq_trigger_type();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_seq_trigger_special() noexcept {
    sp -= 0x20;
    v1 = a0;
    a0 = v1 << 2;
    a0 += v1;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 <<= 2;
    sw(ra, sp + 0x18);
    v0 = lw(v0 + 0xC);
    a2 = 0;                                             // Result = 00000000
    v0 = lw(v0 + 0x10);
    a3 = 0;                                             // Result = 00000000
    sw(a1, sp + 0x10);
    a1 = v1;
    a0 += v0;
    wess_seq_structrig();
    ra = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void wess_seq_status() noexcept {
loc_8004371C:
    // Emulate a little in case calling code is polling in a loop waiting for status to change
    #if PC_PSX_DOOM_MODS
        emulate_a_little();
    #endif

    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    Is_Seq_Num_Valid();
    a2 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_80043748;
    v0 = 0;                                             // Result = 00000000
    goto loc_800437DC;
loc_80043740:
    a2 = 2;                                             // Result = 00000002
    goto loc_800437D8;
loc_80043748:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    v0 = lw(v1 + 0xC);
    a0 = lbu(v0 + 0xB);
    a1 = lw(v1 + 0x20);
    v0 = a0;
    v0 &= 0xFF;
    a0--;
    if (v0 == 0) goto loc_800437D8;
    a3 = 1;                                             // Result = 00000001
    v1 = a1 + 1;
loc_8004377C:
    v0 = lw(a1);
    v0 &= 1;
    if (v0 == 0) goto loc_800437C0;
    v0 = lh(v1 + 0x1);
    if (v0 != s0) goto loc_800437C0;
    v0 = lbu(v1);
    if (v0 == 0) goto loc_80043740;
    {
        const bool bJump = (v0 != a3);
        v0 = a2;                                        // Result = 00000001
        if (bJump) goto loc_800437DC;
    }
    a2 = 3;                                             // Result = 00000003
    goto loc_800437D8;
loc_800437C0:
    v1 += 0x18;
    a1 += 0x18;
    v0 = a0;
    v0 &= 0xFF;
    a0--;
    if (v0 != 0) goto loc_8004377C;
loc_800437D8:
    v0 = a2;
loc_800437DC:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_seq_stop() noexcept {
loc_800437F0:
    sp -= 0x38;
    sw(s7, sp + 0x2C);
    s7 = a0;
    sw(ra, sp + 0x34);
    sw(fp, sp + 0x30);
    sw(s6, sp + 0x28);
    sw(s5, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    Is_Seq_Num_Valid();
    if (v0 == 0) goto loc_80043948;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s4 = lbu(v1 + 0xB);
    v0 = s4;
    if (s6 == 0) goto loc_8004393C;
    v0 &= 0xFF;
    s4--;
    if (v0 == 0) goto loc_8004393C;
    fp = -1;                                            // Result = FFFFFFFF
    s3 = s5 + 0xC;
loc_80043868:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_80043924;
    v0 = lh(s3 - 0xA);
    if (v0 != s7) goto loc_80043914;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s0 = lbu(v0 + 0x1C);
    s1 = lw(s3);
    s0--;
    if (s0 == fp) goto loc_80043914;
loc_800438AC:
    v1 = lbu(s1);
    a1 = 0xFF;                                          // Result = 000000FF
    a0 = v1 << 2;
    if (v1 == a1) goto loc_80043908;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    a0 += v1;
    v0 = lw(v0 + 0x28);
    a0 <<= 4;
    a0 += v0;
    v0 = lbu(a0 + 0x3);
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    v0 <<= 2;
    v0 += a1;
    v0 = lw(v0);
    v0 = lw(v0 + 0x14);
    s2--;
    ptr_call(v0);
    if (s2 == 0) goto loc_80043914;
loc_80043908:
    s0--;
    s1++;
    if (s0 != fp) goto loc_800438AC;
loc_80043914:
    s6--;
    v0 = s6 & 0xFF;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80043940;
    }
loc_80043924:
    s3 += 0x18;
    s5 += 0x18;
    v0 = s4;
    v0 &= 0xFF;
    s4--;
    if (v0 != 0) goto loc_80043868;
loc_8004393C:
    v0 = 1;                                             // Result = 00000001
loc_80043940:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80043948:
    ra = lw(sp + 0x34);
    fp = lw(sp + 0x30);
    s7 = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x38;
    return;
}

void wess_seq_stopall() noexcept {
loc_8004397C:
    sp -= 0x38;
    sw(ra, sp + 0x34);
    sw(fp, sp + 0x30);
    sw(s7, sp + 0x2C);
    sw(s6, sp + 0x28);
    sw(s5, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = Is_Module_Loaded();
    if (v0 == 0) goto loc_80043AC4;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    s6 = lbu(v0 + 0x4);
    v1 = lw(v0 + 0xC);
    s5 = lw(v0 + 0x20);
    s4 = lbu(v1 + 0xB);
    v0 = s4;
    if (s6 == 0) goto loc_80043AB8;
    v0 &= 0xFF;
    s4--;
    if (v0 == 0) goto loc_80043AB8;
    s7 = -1;                                            // Result = FFFFFFFF
    fp = 0x80070000;                                    // Result = 80070000
    fp += 0x5920;                                       // Result = gWess_CmdFuncArr[0] (80075920)
    s3 = s5 + 0xC;
loc_800439F8:
    v0 = lw(s5);
    v0 &= 1;
    if (v0 == 0) goto loc_80043AA0;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A8);                               // Load from: gpWess_pm_stat (800A8758)
    s2 = lbu(s3 - 0x8);
    s0 = lbu(v0 + 0x1C);
    s1 = lw(s3);
    s0--;
    if (s0 == s7) goto loc_80043A90;
loc_80043A2C:
    v1 = lbu(s1);
    a1 = 0xFF;                                          // Result = 000000FF
    a0 = v1 << 2;
    if (v1 == a1) goto loc_80043A84;
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
    if (s2 == 0) goto loc_80043A90;
loc_80043A84:
    s0--;
    s1++;
    if (s0 != s7) goto loc_80043A2C;
loc_80043A90:
    s6--;
    v0 = s6 & 0xFF;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80043ABC;
    }
loc_80043AA0:
    s3 += 0x18;
    s5 += 0x18;
    v0 = s4;
    v0 &= 0xFF;
    s4--;
    if (v0 != 0) goto loc_800439F8;
loc_80043AB8:
    v0 = 1;                                             // Result = 00000001
loc_80043ABC:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5948);                                // Store to: gbWess_SeqOn (80075948)
loc_80043AC4:
    ra = lw(sp + 0x34);
    fp = lw(sp + 0x30);
    s7 = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x38;
    return;
}
