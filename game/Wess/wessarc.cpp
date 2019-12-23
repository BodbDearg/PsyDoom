#include "WESSARC.h"

#include "psxcd.h"
#include "psxspu.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBAPI.h"
#include "PsyQ/LIBSPU.h"
#include "wessseq.h"

void GetIntsPerSec() noexcept {
loc_80043B30:
    v0 = 0x78;                                          // Result = 00000078
    return;
}

void CalcPartsPerInt() noexcept {
loc_80043B38:
    a2 <<= 16;
    a0 <<= 16;
    a0 = u32(i32(a0) >> 16);
    v0 = a0 << 4;
    v0 -= a0;
    v1 = v0 << 1;
    a2 += v1;
    a2 += 0x1E;
    v0 <<= 2;
    divu(a2, v0);
    if (v0 != 0) goto loc_80043B6C;
    _break(0x1C00);
loc_80043B6C:
    a2 = lo;
    a1 <<= 16;
    a1 = u32(i32(a1) >> 16);
    mult(a2, a1);
    v0 = lo;
    return;
}

void WessInterruptHandler() noexcept {
    sp -= 0x18;
    a0 = 0x80000;                                       // Result = 00080000
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5950);                               // Load from: gWess_T2counter (80075950)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5958);                               // Load from: 80075958
    a0 |= 0x5555;                                       // Result = 00085555
    sw(ra, sp + 0x10);
    v1 += a0;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5950);                                // Store to: gWess_T2counter (80075950)
    v0 = v1 >> 16;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5958);                                // Store to: 80075958
    v1 &= 0xFFFF;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x5958);                                // Store to: 80075958
    v0 += a0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5954);                                // Store to: gWess_Millicount (80075954)
    psxspu_fadeengine();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5948);                               // Load from: gbWess_SeqOn (80075948)
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80043C0C;
    }
    SeqEngine();
    v0 = 0;                                             // Result = 00000000
loc_80043C0C:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void init_WessTimer() noexcept {
loc_80043C1C:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5948);                                 // Store to: gbWess_SeqOn (80075948)
    LIBAPI_EnterCriticalSection();
    a0 = 0xF2000000;                                    // Result = F2000000
    a0 |= 2;                                            // Result = F2000002
    a1 = 2;                                             // Result = 00000002
    a3 = 0x80040000;                                    // Result = 80040000
    a3 += 0x3B88;                                       // Result = WessInterruptHandler (80043B88)
    a2 = 0x1000;                                        // Result = 00001000
    LIBAPI_OpenEvent();
    a0 = v0;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x595C);                                // Store to: gWess_EV2 (8007595C)
    LIBAPI_EnableEvent();
    a0 = 0xF2000000;                                    // Result = F2000000
    a0 |= 2;                                            // Result = F2000002
    a1 = 0x87A2;                                        // Result = 000087A2
    a2 = 0x1000;                                        // Result = 00001000
    LIBAPI_SetRCnt();
    a0 = 0xF2000000;                                    // Result = F2000000
    a0 |= 2;                                            // Result = F2000002
    LIBAPI_StartRCnt();
    v0 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x594C);                                // Store to: gbWess_WessTimerActive (8007594C)
    LIBAPI_ExitCriticalSection();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void exit_WessTimer() noexcept {
loc_80043CA8:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxspu_get_master_vol();
    a0 = v0;
    psxspu_set_master_vol();
    psxspu_get_cd_vol();
    a0 = v0;
    psxspu_set_cd_vol();
    LIBAPI_EnterCriticalSection();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x595C);                               // Load from: gWess_EV2 (8007595C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x594C);                                 // Store to: gbWess_WessTimerActive (8007594C)
    LIBAPI_DisableEvent();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x595C);                               // Load from: gWess_EV2 (8007595C)
    LIBAPI_CloseEvent();
    LIBAPI_ExitCriticalSection();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Wess_init_for_LoadFileData() noexcept {
    v0 = 1;                                             // Result = 00000001
    return;
}

void module_open() noexcept {
loc_80043D20:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    _thunk_psxcd_open();
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0x1004;                                       // Result = gWess_module_fileref[0] (8007EFFC)
    a2 = v0;
    t0 = a2 + 0x20;
loc_80043D40:
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
    if (a2 != t0) goto loc_80043D40;
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0x1004;                                       // Result = gWess_module_fileref[0] (8007EFFC)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_read() noexcept {
loc_80043D94:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    _thunk_psxcd_read();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_seek() noexcept {
loc_80043DB4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    _thunk_psxcd_seek();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_tell() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_tell();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_close() noexcept {
loc_80043DF4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_close();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void get_num_Wess_Sound_Drivers() noexcept {
loc_80043E14:
    v0 = 1;                                             // Result = 00000001
    return;
}

void data_open() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    _thunk_psxcd_open();
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0xFDC;                                        // Result = gWess_data_fileref[0] (8007F024)
    a2 = v0;
    t0 = a2 + 0x20;
loc_80043E3C:
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
    if (a2 != t0) goto loc_80043E3C;
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0xFDC;                                        // Result = gWess_data_fileref[0] (8007F024)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void data_read_chunk() noexcept {
loc_80043E90:
    sp -= 0x20;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0xFB4);                                // Load from: 8007F04C
    v1 = a0;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s2, sp + 0x18);
    s2 = a2;
    sw(ra, sp + 0x1C);
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_80043F00;
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a2 = v1;
    _thunk_psxcd_read();
    a0 = s2;
    LIBSPU_SpuSetTransferStartAddr();
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = s1;
    LIBSPU_SpuWrite();
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x1008);                                // Store to: 8007EFF8
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0xFB4);                                  // Store to: 8007F04C
    goto loc_80043F90;
loc_80043F00:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x1008);                               // Load from: 8007EFF8
    a1 = s1;
    if (v0 == 0) goto loc_80043F54;
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x6D7C;                                       // Result = gWess_data_read_chunk2[0] (80096D7C)
    a0 = s0;                                            // Result = gWess_data_read_chunk2[0] (80096D7C)
    a2 = v1;
    _thunk_psxcd_read();
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    a0 = s2;
    LIBSPU_SpuSetTransferStartAddr();
    a0 = s0;                                            // Result = gWess_data_read_chunk2[0] (80096D7C)
    a1 = s1;
    LIBSPU_SpuWrite();
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x1008);                                 // Store to: 8007EFF8
    goto loc_80043F90;
loc_80043F54:
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a2 = v1;
    _thunk_psxcd_read();
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    a0 = s2;
    LIBSPU_SpuSetTransferStartAddr();
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = s1;
    LIBSPU_SpuWrite();
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x1008);                                // Store to: 8007EFF8
loc_80043F90:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void data_read() noexcept {
    sp -= 0x28;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(s3, sp + 0x1C);
    s3 = a2;
    sw(s0, sp + 0x10);
    s0 = s3;
    sw(s1, sp + 0x14);
    s1 = a1;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x598C);                               // Load from: 8007598C
    a1 = a3;
    v0 -= s1;
    v0 = (v0 < s3);
    sw(ra, sp + 0x20);
    if (v0 == 0) goto loc_80043FF4;
    v0 = 0;                                             // Result = 00000000
    goto loc_80044058;
loc_80043FF4:
    a0 = s2;
    a2 = 0;                                             // Result = 00000000
    _thunk_psxcd_seek();
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0xFB4);                                 // Store to: 8007F04C
    v0 = (i32(s3) < 0x800);
    if (v0 != 0) goto loc_80044038;
loc_80044018:
    a0 = s2;
    a1 = 0x800;                                         // Result = 00000800
    a2 = s1;
    data_read_chunk();
    s0 -= 0x800;
    v0 = (i32(s0) < 0x800);
    s1 += 0x800;
    if (v0 == 0) goto loc_80044018;
loc_80044038:
    a0 = s2;
    if (s0 == 0) goto loc_8004404C;
    a1 = s0;
    a2 = s1;
    data_read_chunk();
loc_8004404C:
    a0 = 1;                                             // Result = 00000001
    LIBSPU_SpuIsTransferCompleted();
    v0 = s3;
loc_80044058:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void data_close() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_close();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_low_level_init() noexcept {
loc_80043AF8:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxspu_init();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void wess_low_level_exit() noexcept {
loc_80043B18:
    return;
}

void wess_malloc() noexcept {
loc_80043B20:
    v0 = 0;                                             // Result = 00000000
    return;
}

void wess_free() noexcept {
loc_80043B28:
    return;
}
