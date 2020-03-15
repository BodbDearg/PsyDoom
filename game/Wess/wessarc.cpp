#include "WESSARC.h"

#include "psxcd.h"
#include "psxspu.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBAPI.h"
#include "PsyQ/LIBSPU.h"
#include "wessseq.h"

// Keeps track of global time (MS) for the sequencer and other operations
const VmPtr<uint32_t> gWess_Millicount(0x80075954);

// True if the 'WessInterruptHandler' function is active and receiving periodic callbacks
const VmPtr<bool32_t> gbWess_WessTimerActive(0x8007594C);

// Temporary buffers used for holding a sector worth of data
const VmPtr<uint8_t[CD_SECTOR_SIZE]> gWess_sectorBuffer1(0x8009656C);
const VmPtr<uint8_t[CD_SECTOR_SIZE]> gWess_sectorBuffer2(0x80096D7C);

// TODO: COMMENT
const VmPtr<bool32_t> gbWess_SeqOn(0x80075948);

//------------------------------------------------------------------------------------------------------------------------------------------
// Gives the number of ticks or interrupts per second the music system uses.
// In PSX DOOM the sequencer runs at roughly 120 Hz intervals.
//------------------------------------------------------------------------------------------------------------------------------------------
int16_t GetIntsPerSec() noexcept {
    return 120;
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
    
    const int32_t rcnt2Event = LIBAPI_OpenEvent(RCntCNT2, EvSpINT, EvMdINTR, WessInterruptHandler);

    at = 0x80070000;                                    // Result = 80070000
    sw(rcnt2Event, at + 0x595C);                        // Store to: gWess_EV2 (8007595C)

    LIBAPI_EnableEvent(rcnt2Event);
    LIBAPI_SetRCnt(RCntCNT2, 34722, RCntMdINTR);        // This generates interrupts at about 121.9284 Hz
    LIBAPI_StartRCnt(RCntCNT2);

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
    v0 = psxspu_get_master_vol();
    a0 = v0;
    psxspu_set_master_vol(a0);
    v0 = psxspu_get_cd_vol();
    a0 = v0;
    psxspu_set_cd_vol(a0);
    LIBAPI_EnterCriticalSection();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x595C);                               // Load from: gWess_EV2 (8007595C)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x594C);                                 // Store to: gbWess_WessTimerActive (8007594C)
    LIBAPI_DisableEvent(a0);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x595C);                               // Load from: gWess_EV2 (8007595C)
    LIBAPI_CloseEvent(a0);
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
    v0 = ptrToVmAddr(psxcd_open((CdMapTbl_File) a0));
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
    v0 = psxcd_read(vmAddrToPtr<void>(a0), a1, *vmAddrToPtr<PsxCd_File>(a2));
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_seek() noexcept {
loc_80043DB4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = psxcd_seek(*vmAddrToPtr<PsxCd_File>(a0), a1, (PsxCd_SeekMode) a2);
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_tell() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 = psxcd_tell(*vmAddrToPtr<PsxCd_File>(a0));
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void module_close() noexcept {
loc_80043DF4:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_close(*vmAddrToPtr<PsxCd_File>(a0));
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
    v0 = ptrToVmAddr(psxcd_open((CdMapTbl_File) a0));
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
    v0 = psxcd_read(vmAddrToPtr<void>(a0), a1, *vmAddrToPtr<PsxCd_File>(a2));
    a0 = s2;
    v0 = LIBSPU_SpuSetTransferStartAddr(a0);
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = s1;
    v0 = LIBSPU_SpuWrite(vmAddrToPtr<void>(a0), a1);
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
    v0 = psxcd_read(vmAddrToPtr<void>(a0), a1, *vmAddrToPtr<PsxCd_File>(a2));
    v0 = LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
    a0 = s2;
    v0 = LIBSPU_SpuSetTransferStartAddr(a0);
    a0 = s0;                                            // Result = gWess_data_read_chunk2[0] (80096D7C)
    a1 = s1;
    v0 = LIBSPU_SpuWrite(vmAddrToPtr<void>(a0), a1);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x1008);                                 // Store to: 8007EFF8
    goto loc_80043F90;
loc_80043F54:
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a2 = v1;
    v0 = psxcd_read(vmAddrToPtr<void>(a0), a1, *vmAddrToPtr<PsxCd_File>(a2));
    v0 = LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
    a0 = s2;
    v0 = LIBSPU_SpuSetTransferStartAddr(a0);
    a0 = s0;                                            // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = s1;
    v0 = LIBSPU_SpuWrite(vmAddrToPtr<void>(a0), a1);
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
    v0 = psxcd_seek(*vmAddrToPtr<PsxCd_File>(a0), a1, (PsxCd_SeekMode) a2);
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
    v0 = LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
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
    psxcd_close(*vmAddrToPtr<PsxCd_File>(a0));
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialization function to setup platform specific WESS stuff
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_low_level_init() noexcept {
    psxspu_init();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup function to shut down platform specific WESS stuff
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_low_level_exit() noexcept {
    // Didn't do anything...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocate memory for the sound system
//------------------------------------------------------------------------------------------------------------------------------------------
void* wess_malloc() noexcept {
    // Not implemented in PSX DOOM - always returned 'null'
    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Free memory for the sound system
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_free([[maybe_unused]] void* const pMem) noexcept {
    // Not implemented in PSX DOOM because 'wess_malloc' wasn't implemented...
}
