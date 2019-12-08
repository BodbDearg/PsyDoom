#include "i_file.h"

#include "PsxVm/PsxVm.h"

void InitOpenFileSlots() noexcept {
loc_80031EB4:
    v0 = 0x78;                                          // Result = 00000078
loc_80031EB8:
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x626C;                                       // Result = gOpenPsxCdFiles[1] (800A9D94)
    at += v0;
    sw(0, at);
    v0 -= 0x28;
    if (i32(v0) >= 0) goto loc_80031EB8;
    return;
}

void OpenFile() noexcept {
loc_80031EDC:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    psxcd_open();
    s1 = v0;
    if (s1 != 0) goto loc_80031F10;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1384;                                       // Result = STR_CannotOpen_Err[0] (80011384)
    a1 = s0;
    I_Error();
loc_80031F10:
    s0 = 0;                                             // Result = 00000000
    v1 = 0;                                             // Result = 00000000
loc_80031F18:
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x626C;                                       // Result = gOpenPsxCdFiles[1] (800A9D94)
    at += v1;
    v0 = lw(at);
    {
        const bool bJump = (v0 == 0)
        v0 = (i32(s0) < 4);
        if (bJump) goto loc_80031F48;
    }
    s0++;
    v0 = (i32(s0) < 4);
    v1 += 0x28;
    if (v0 != 0) goto loc_80031F18;
    v0 = (i32(s0) < 4);
loc_80031F48:
    if (v0 != 0) goto loc_80031F60;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1394;                                       // Result = STR_OpenFile_TooManyFiles_Err[0] (80011394)
    I_Error();
loc_80031F60:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x6270;                                       // Result = gOpenPsxCdFiles[0] (800A9D90)
    v0 = s0 << 2;
    v0 += s0;
    v0 <<= 3;
    a3 = v0 + v1;
    a2 = s1;
    t0 = a2 + 0x20;
loc_80031F80:
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    a0 = lw(a2 + 0x8);
    a1 = lw(a2 + 0xC);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    sw(a0, a3 + 0x8);
    sw(a1, a3 + 0xC);
    a2 += 0x10;
    a3 += 0x10;
    if (a2 != t0) goto loc_80031F80;
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    v0 = s0;                                            // Result = 00000000
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void CloseFile() noexcept {
loc_80031FD8:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0 << 2;
    s0 += a0;
    s0 <<= 3;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x6270;                                       // Result = gOpenPsxCdFiles[0] (800A9D90)
    sw(ra, sp + 0x14);
    a0 += s0;
    psxcd_close();
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x626C;                                       // Result = gOpenPsxCdFiles[1] (800A9D94)
    at += s0;
    sw(0, at);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void SeekAndTellFile() noexcept {
loc_80032024:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0 << 2;
    s0 += a0;
    s0 <<= 3;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x6270;                                       // Result = gOpenPsxCdFiles[0] (800A9D90)
    s0 += v0;
    sw(ra, sp + 0x14);
    a0 = s0;
    psxcd_seek();
    a0 = s0;
    psxcd_tell();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void ReadFile() noexcept {
loc_8003206C:
    sp -= 0x28;
    sw(s4, sp + 0x20);
    s4 = a1;
    sw(s2, sp + 0x18);
    s2 = a2;
    v0 = a0 << 2;
    v0 += a0;
    v0 <<= 3;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x6270;                                       // Result = gOpenPsxCdFiles[0] (800A9D90)
    sw(s1, sp + 0x14);
    s1 = v0 + v1;
    a0 = s1;
    sw(ra, sp + 0x24);
    sw(s3, sp + 0x1C);
    sw(s0, sp + 0x10);
    psxcd_tell();
    s3 = v0;
    v0 = (s2 < 0x2001);
    s0 = s2;
    if (v0 != 0) goto loc_800320C4;
    s0 = 0x2000;                                        // Result = 00002000
loc_800320C4:
    a0 = s1;
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    psxcd_seek();
    a0 = s4;
    a1 = s0;
    a2 = s1;
    psxcd_read();
    a0 = s1;
    a1 = s3;
    a2 = 0;                                             // Result = 00000000
    psxcd_seek();
    a0 = s4;
    a1 = s2;
    a2 = s1;
    psxcd_read();
    s0 = v0;
    a1 = s0;
    if (s0 == s2) goto loc_80032120;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x13B4;                                       // Result = STR_ReadFile_Read_Err[0] (800113B4)
    a2 = s2;
    I_Error();
loc_80032120:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}
