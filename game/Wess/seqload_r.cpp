#include "seqload_r.h"

#include "PsxVm/PsxVm.h"
#include "seqload.h"

void wess_seq_range_sizeof() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(s2, sp + 0x18);
    s2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s1, sp + 0x14);
    if (v0 == 0) goto loc_80049AB8;
    s1 = a0;
    if (s0 != 0) goto loc_80049A8C;
    v0 = 0;                                             // Result = 00000000
    goto loc_80049ABC;
loc_80049A8C:
    s0--;
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s0 == v0);
        v0 = s2;                                        // Result = 00000000
        if (bJump) goto loc_80049ABC;
    }
    s3 = -1;                                            // Result = FFFFFFFF
loc_80049AA0:
    a0 = s1;
    wess_seq_sizeof();
    s2 += v0;
    s0--;
    s1++;
    if (s0 != s3) goto loc_80049AA0;
loc_80049AB8:
    v0 = s2;
loc_80049ABC:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void wess_seq_range_load() noexcept {
loc_80049ADC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x28;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(s4, sp + 0x20);
    s4 = a2;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x24);
    sw(s3, sp + 0x1C);
    if (v0 == 0) goto loc_80049B68;
    open_sequence_data();
    if (v0 == 0) goto loc_80049B2C;
    {
        const bool bJump = (s0 != 0);
        s0--;
        if (bJump) goto loc_80049B34;
    }
loc_80049B2C:
    v0 = 0;                                             // Result = 00000000
    goto loc_80049B6C;
loc_80049B34:
    v0 = -1;                                            // Result = FFFFFFFF
    if (s0 == v0) goto loc_80049B60;
    s3 = -1;                                            // Result = FFFFFFFF
loc_80049B44:
    a0 = s2;
    a1 = s4 + s1;
    wess_seq_load();
    s1 += v0;
    s0--;
    s2++;
    if (s0 != s3) goto loc_80049B44;
loc_80049B60:
    close_sequence_data();
loc_80049B68:
    v0 = s1;
loc_80049B6C:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void wess_seq_range_free() noexcept {
loc_80049B90:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(s3, sp + 0x1C);
    s3 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x20);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    if (v0 == 0) goto loc_80049BF8;
    s1 = a0;
    if (s0 != 0) goto loc_80049BCC;
    v0 = 0;                                             // Result = 00000000
    goto loc_80049BFC;
loc_80049BCC:
    s0--;
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s0 == v0);
        v0 = s3;                                        // Result = 00000000
        if (bJump) goto loc_80049BFC;
    }
    s3 = 1;                                             // Result = 00000001
    s2 = -1;                                            // Result = FFFFFFFF
loc_80049BE4:
    a0 = s1;
    wess_seq_free();
    s0--;
    s1++;
    if (s0 != s2) goto loc_80049BE4;
loc_80049BF8:
    v0 = s3;
loc_80049BFC:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}
