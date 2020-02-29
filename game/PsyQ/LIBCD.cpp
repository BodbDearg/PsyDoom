#include "LIBCD.h"

#include "LIBAPI.h"
#include "LIBC2.h"
#include "LIBETC.h"

BEGIN_THIRD_PARTY_INCLUDES

#include <device/cdrom/cdrom.h>
#include <disc/disc.h>

END_THIRD_PARTY_INCLUDES

// CD-ROM constants
static constexpr int32_t CD_SECTORS_PER_SEC      = 75;       // The number of CD sectors per second of audio
static constexpr int32_t CD_LEAD_SECTORS   = 150;      // How many sectors are assigned to the lead in track, which has the TOC for the disc

// N.B: must be done LAST due to MIPS register macros
#include "PsxVm/PsxVm.h"

void LIBCD_CdInit() noexcept {
loc_80054B00:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = 4;                                             // Result = 00000004
    sw(ra, sp + 0x14);
loc_80054B10:
    a0 = 1;                                             // Result = 00000001
    LIBCD_CdReset();
    v1 = 1;                                             // Result = 00000001
    s0--;
    if (v0 != v1) goto loc_80054B5C;
    a0 = 0x80050000;                                    // Result = 80050000
    a0 += 0x4B90;                                       // Result = LIBCD_EVENT_def_cbsync (80054B90)
    LIBCD_CdSyncCallback();
    a0 = 0x80050000;                                    // Result = 80050000
    a0 += 0x4BB8;                                       // Result = LIBCD_EVENT_def_cbready (80054BB8)
    LIBCD_CdReadyCallback();
    a0 = 0x80050000;                                    // Result = 80050000
    a0 += 0x4BE0;                                       // Result = LIBCD_EVENT_def_cbread (80054BE0)
    LIBCD_CdReadCallback();
    v0 = 1;                                             // Result = 00000001
    goto loc_80054B7C;
loc_80054B5C:
    v0 = -1;                                            // Result = FFFFFFFF
    if (s0 != v0) goto loc_80054B10;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1F54;                                       // Result = STR_LIBCD_CdInit_Failed_Err[0] (80011F54)
    LIBC2_printf();
    v0 = 0;                                             // Result = 00000000
loc_80054B7C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_EVENT_def_cbsync() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0xF0000000;                                    // Result = F0000000
    a0 |= 3;                                            // Result = F0000003
    a1 = 0x20;                                          // Result = 00000020
    LIBAPI_DeliverEvent();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_EVENT_def_cbready() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0xF0000000;                                    // Result = F0000000
    a0 |= 3;                                            // Result = F0000003
    a1 = 0x40;                                          // Result = 00000040
    LIBAPI_DeliverEvent();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_EVENT_def_cbread() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = 0xF0000000;                                    // Result = F0000000
    a0 |= 3;                                            // Result = F0000003
    a1 = 0x40;                                          // Result = 00000040
    LIBAPI_DeliverEvent();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_CdStatus() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7204);                              // Load from: gLIBCD_CD_status (80077204)
    return;
}

void LIBCD_CdLastCom() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7210);                              // Load from: gLIBCD_CD_com (80077210)
    return;
}

void LIBCD_CdReset() noexcept {
loc_80054C28:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x14);
    s0 = a0;
    LIBCD_CD_init();
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80054C64;
    }
    v0 = 1;                                             // Result = 00000001
    if (s0 != v0) goto loc_80054C64;
    LIBCD_CD_initvol();
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80054C64;
    }
    v0 = 1;                                             // Result = 00000001
loc_80054C64:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_CdFlush() noexcept {
loc_80054C78:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBCD_CD_flush();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_CdSetDebug() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7200);                               // Load from: gLIBCD_CD_debug (80077200)
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x7200);                                // Store to: gLIBCD_CD_debug (80077200)
    return;
}

void LIBCD_CdComstr() noexcept {
    a0 &= 0xFF;
    v0 = (a0 < 0x1C);
    {
        const bool bJump = (v0 == 0);
        v0 = a0 << 2;
        if (bJump) goto loc_80054CD8;
    }
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7214;                                       // Result = gLIBCD_CD_comstr[0] (80077214)
    at += v0;
    v0 = lw(at);
    goto loc_80054CE0;
loc_80054CD8:
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x1F6C;                                       // Result = STR_Sys_none_Msg[0] (80011F6C)
loc_80054CE0:
    return;
}

void LIBCD_CdIntstr() noexcept {
    a0 &= 0xFF;
    v0 = (a0 < 7);
    {
        const bool bJump = (v0 == 0);
        v0 = a0 << 2;
        if (bJump) goto loc_80054D10;
    }
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7294;                                       // Result = gLIBCD_CD_intstr[0] (80077294)
    at += v0;
    v0 = lw(at);
    goto loc_80054D18;
loc_80054D10:
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x1F6C;                                       // Result = STR_Sys_none_Msg[0] (80011F6C)
loc_80054D18:
    return;
}

void LIBCD_CdSync() noexcept {
loc_80054D20:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBCD_CD_sync();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_CdReady() noexcept {
    // Speed up the emulation of the CD
    #if PC_PSX_DOOM_MODS
        emulate_cdrom();
    #endif

    LIBCD_CD_ready();
}

void LIBCD_CdSyncCallback() noexcept {
loc_80054D60:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x71F4);                                // Store to: gpLIBCD_CD_cbsync (800771F4)
    return;
}

void LIBCD_CdReadyCallback() noexcept {
loc_80054D78:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x71F8);                               // Load from: gpLIBCD_CD_cbready (800771F8)
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x71F8);                                // Store to: gpLIBCD_CD_cbready (800771F8)
    return;
}

void LIBCD_CdReadCallback() noexcept {
loc_80054D90:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x71FC);                               // Load from: gpLIBCD_CD_cbread (800771FC)
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x71FC);                                // Store to: gpLIBCD_CD_cbread (800771FC)
    return;
}

void LIBCD_CdControl() noexcept {
loc_80054DA8:
    sp -= 0x38;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s2, sp + 0x18);
    s2 = a2;
    sw(s4, sp + 0x20);
    s4 = a0;
    sw(s0, sp + 0x10);
    s0 = 3;                                             // Result = 00000003
    sw(s3, sp + 0x1C);
    s3 = s4 & 0xFF;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x7174;                                       // Result = 80077174
    sw(s5, sp + 0x24);
    s5 = 0x80070000;                                    // Result = 80070000
    s5 = lw(s5 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    v0 = s3 << 2;
    sw(s6, sp + 0x28);
    s6 = v0 + v1;
    sw(s7, sp + 0x2C);
    s7 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x30);
    v0 = 1;                                             // Result = 00000001
loc_80054E04:
    a0 = s4 & 0xFF;
    if (s3 == v0) goto loc_80054E58;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x71F4);                                 // Store to: gpLIBCD_CD_cbsync (800771F4)
    LIBCD_CD_shell();
    if (s1 == 0) goto loc_80054E4C;
    v0 = lw(s6);
    a0 = 2;                                             // Result = 00000002
    if (v0 == 0) goto loc_80054E4C;
    a1 = s1;
    a2 = s2;
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    if (v0 != 0) goto loc_80054E70;
loc_80054E4C:
    at = 0x80070000;                                    // Result = 80070000
    sw(s5, at + 0x71F4);                                // Store to: gpLIBCD_CD_cbsync (800771F4)
    a0 = s4 & 0xFF;
loc_80054E58:
    a1 = s1;
    a2 = s2;
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    {
        const bool bJump = (v0 == 0);
        v0 = s7 + 1;                                    // Result = 00000001
        if (bJump) goto loc_80054E90;
    }
loc_80054E70:
    s0--;
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s0 != v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80054E04;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(s5, at + 0x71F4);                                // Store to: gpLIBCD_CD_cbsync (800771F4)
    s7 = -1;                                            // Result = FFFFFFFF
    v0 = s7 + 1;                                        // Result = 00000000
loc_80054E90:
    ra = lw(sp + 0x30);
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

void LIBCD_CdControlF() noexcept {
loc_80054EC0:
    sp -= 0x30;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s0, sp + 0x10);
    s0 = 3;                                             // Result = 00000003
    sw(s2, sp + 0x18);
    s2 = s3 & 0xFF;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x7174;                                       // Result = 80077174
    sw(s4, sp + 0x20);
    s4 = 0x80070000;                                    // Result = 80070000
    s4 = lw(s4 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    v0 = s2 << 2;
    sw(s5, sp + 0x24);
    s5 = v0 + v1;
    sw(s6, sp + 0x28);
    s6 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x2C);
    v0 = 1;                                             // Result = 00000001
loc_80054F14:
    a0 = s3 & 0xFF;
    if (s2 == v0) goto loc_80054F68;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x71F4);                                 // Store to: gpLIBCD_CD_cbsync (800771F4)
    LIBCD_CD_shell();
    if (s1 == 0) goto loc_80054F5C;
    v0 = lw(s5);
    a0 = 2;                                             // Result = 00000002
    if (v0 == 0) goto loc_80054F5C;
    a1 = s1;
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    if (v0 != 0) goto loc_80054F80;
loc_80054F5C:
    at = 0x80070000;                                    // Result = 80070000
    sw(s4, at + 0x71F4);                                // Store to: gpLIBCD_CD_cbsync (800771F4)
    a0 = s3 & 0xFF;
loc_80054F68:
    a1 = s1;
    a2 = 0;                                             // Result = 00000000
    a3 = 1;                                             // Result = 00000001
    LIBCD_CD_cw();
    {
        const bool bJump = (v0 == 0);
        v0 = s6 + 1;                                    // Result = 00000001
        if (bJump) goto loc_80054FA0;
    }
loc_80054F80:
    s0--;
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s0 != v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80054F14;
    }
    at = 0x80070000;                                    // Result = 80070000
    sw(s4, at + 0x71F4);                                // Store to: gpLIBCD_CD_cbsync (800771F4)
    s6 = -1;                                            // Result = FFFFFFFF
    v0 = s6 + 1;                                        // Result = 00000000
loc_80054FA0:
    ra = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x30;
}

void LIBCD_CdControlB() noexcept {
loc_80054FCC:
    sp -= 0x30;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s2, sp + 0x18);
    s2 = a2;
    sw(s4, sp + 0x20);
    s4 = a0;
    sw(s0, sp + 0x10);
    s0 = 3;                                             // Result = 00000003
    sw(s3, sp + 0x1C);
    s3 = s4 & 0xFF;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x7174;                                       // Result = 80077174
    sw(s5, sp + 0x24);
    s5 = 0x80070000;                                    // Result = 80070000
    s5 = lw(s5 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    v0 = s3 << 2;
    sw(s6, sp + 0x28);
    s6 = v0 + v1;
    sw(ra, sp + 0x2C);
loc_8005501C:
    v0 = 1;                                             // Result = 00000001
    a0 = s4 & 0xFF;
    if (s3 == v0) goto loc_80055074;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x71F4);                                 // Store to: gpLIBCD_CD_cbsync (800771F4)
    LIBCD_CD_shell();
    if (s1 == 0) goto loc_80055068;
    v0 = lw(s6);
    a0 = 2;                                             // Result = 00000002
    if (v0 == 0) goto loc_80055068;
    a1 = s1;
    a2 = s2;
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    if (v0 != 0) goto loc_8005508C;
loc_80055068:
    at = 0x80070000;                                    // Result = 80070000
    sw(s5, at + 0x71F4);                                // Store to: gpLIBCD_CD_cbsync (800771F4)
    a0 = s4 & 0xFF;
loc_80055074:
    a1 = s1;
    a2 = s2;
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800550A4;
    }
loc_8005508C:
    s0--;
    v0 = -1;                                            // Result = FFFFFFFF
    if (s0 != v0) goto loc_8005501C;
    at = 0x80070000;                                    // Result = 80070000
    sw(s5, at + 0x71F4);                                // Store to: gpLIBCD_CD_cbsync (800771F4)
loc_800550A4:
    a0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_800550C0;
    a1 = s2;
    LIBCD_CD_sync();
    v0 ^= 2;
    v0 = (v0 < 1);
    goto loc_800550C4;
loc_800550C0:
    v0 = 0;                                             // Result = 00000000
loc_800550C4:
    ra = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x30;
    return;
}

void LIBCD_CdMix() noexcept {
loc_800550F0:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBCD_CD_vol();
    v0 = 1;                                             // Result = 00000001
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_CdGetSector() noexcept {
loc_80055114:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBCD_CD_getsector();
    v0 = (v0 < 1);
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_CdDataCallback() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = a0;
    a0 = 3;                                             // Result = 00000003
    LIBETC_DMACallback();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_CdDataSync() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBCD_CD_datasync();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_CdReadSync() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBCD_CD_readsync();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_CdRead() noexcept {
    sp -= 0x28;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s3, sp + 0x1C);
    s3 = a2;
    sw(s0, sp + 0x10);
    s0 = 3;                                             // Result = 00000003
    sw(s4, sp + 0x20);
    s4 = -1;                                            // Result = FFFFFFFF
    sw(ra, sp + 0x24);
    a0 = s2;
loc_800551D0:
    a1 = s1;
    a2 = s3;
    LIBCD_CD_readm();
    s0--;
    if (v0 != 0) goto loc_800551EC;
    v0 = 1;                                             // Result = 00000001
    goto loc_800551F8;
loc_800551EC:
    a0 = s2;
    if (s0 != s4) goto loc_800551D0;
    v0 = 0;                                             // Result = 00000000
loc_800551F8:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert from a CD position in terms of an absolute sector to a binary coded decimal CD position in terms of minutes,
// seconds and sector number. Note that the given sector number is assumed to NOT include the lead in track, so that will be added to the
// returned BCD minute, second and sector number.
//------------------------------------------------------------------------------------------------------------------------------------------
CdlLOC& LIBCD_CdIntToPos(const int32_t sectorNum, CdlLOC& pos) noexcept {
    const int32_t totalSeconds = (sectorNum + CD_LEAD_SECTORS) / CD_SECTORS_PER_SEC;
    const int32_t sector = (sectorNum + CD_LEAD_SECTORS) % CD_SECTORS_PER_SEC;
    const int32_t minute = totalSeconds / 60;
    const int32_t second = totalSeconds % 60;

    // For more about this particular way of doing decimal to BCD conversion, see:
    // https://stackoverflow.com/questions/45655484/bcd-to-decimal-and-decimal-to-bcd
    pos.second = (uint8_t) second + (uint8_t)(second / 10) * 6;
    pos.sector = (uint8_t) sector + (uint8_t)(sector / 10) * 6;
    pos.minute = (uint8_t) minute + (uint8_t)(minute / 10) * 6;
    return pos;
}

void _thunk_LIBCD_CdIntToPos() noexcept {
    v0 = LIBCD_CdIntToPos(a0, *vmAddrToPtr<CdlLOC>(a1));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert from a CD position in terms of seconds, minutes to an absolute sector number.
// Note: the hidden 'lead in' track is discounted from the returned absolute sector number.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBCD_CdPosToInt(const CdlLOC& pos) noexcept {
    // Convert minute, second and sector counts from BCD to decimal
    const uint32_t minute = (uint32_t)(pos.minute >> 4) * 10 + (pos.minute & 0xF);
    const uint32_t second = (uint32_t)(pos.second >> 4) * 10 + (pos.second & 0xF);
    const uint32_t sector = (uint32_t)(pos.sector >> 4) * 10 + (pos.sector & 0xF);

    // Figure out the absolute sector number and exclude the hidden lead in track which contains the TOC
    return (minute * 60 + second) * CD_SECTORS_PER_SEC + sector - CD_LEAD_SECTORS;
}

void _thunk_LIBCD_CdPosToInt() noexcept {
    v0 = LIBCD_CdPosToInt(*vmAddrToPtr<CdlLOC>(a0));
}

void LIBCD_BIOS_getintr() noexcept {
loc_800553A0:
    sp -= 0x20;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B4);                               // Load from: 800774B4
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    sb(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74C0);                               // Load from: 800774C0
    v0 = lbu(v0);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x74CC;                                       // Result = 800774CC
    v0 &= 7;
    sb(v0, v1);                                         // Store to: 800774CC
    v0 = lbu(v1);                                       // Load from: 800774CC
    s1 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_800553F8;
    v0 = 0;                                             // Result = 00000000
    goto loc_800558C4;
loc_800553F8:
    v1 = 0;                                             // Result = 00000000
loc_800553FC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    v0 = lbu(v0);
    v0 &= 0x20;
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 8);
        if (bJump) goto loc_80055470;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B8);                               // Load from: 800774B8
    v0 = lbu(v0);
    at = 0x80080000;                                    // Result = 80080000
    at += 0x6100;                                       // Result = 80086100
    at += v1;
    sb(v0, at);
    v1++;
    v0 = (i32(v1) < 8);
    if (v0 == 0) goto loc_80055470;
    goto loc_800553FC;
loc_80055458:
    at = 0x80080000;                                    // Result = 80080000
    at += 0x6100;                                       // Result = 80086100
    at += v1;
    sb(0, at);
    v1++;
    v0 = (i32(v1) < 8);
loc_80055470:
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80055458;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B4);                               // Load from: 800774B4
    sb(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74C0);                               // Load from: 800774C0
    v1 = 7;                                             // Result = 00000007
    sb(v1, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74BC);                               // Load from: 800774BC
    sb(v1, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7210);                              // Load from: gLIBCD_CD_com (80077210)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x73B4;                                       // Result = 800773B4
    at += v0;
    v0 = lw(at);
    if (v0 == 0) goto loc_80055540;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7204);                               // Load from: gLIBCD_CD_status (80077204)
    v0 &= 0x10;
    if (v0 != 0) goto loc_80055520;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6100;                                       // Result = 80086100
    v0 = lbu(v0);                                       // Load from: 80086100
    v0 &= 0x10;
    if (v0 == 0) goto loc_80055520;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7208);                               // Load from: gLIBCD_CD_status1 (80077208)
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7208);                                // Store to: gLIBCD_CD_status1 (80077208)
loc_80055520:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6100;                                       // Result = 80086100
    v0 = lbu(v0);                                       // Load from: 80086100
    v0 &= 0xFF;
    s1 = v0 & 0x1D;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7204);                                // Store to: gLIBCD_CD_status (80077204)
loc_80055540:
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x74CC;                                       // Result = 800774CC
    v0 = lbu(s0);                                       // Load from: 800774CC
    v1 = 5;                                             // Result = 00000005
    v0 &= 0xFF;
    if (v0 != v1) goto loc_80055590;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7210);                              // Load from: gLIBCD_CD_com (80077210)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7204);                               // Load from: gLIBCD_CD_status (80077204)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7214;                                       // Result = gLIBCD_CD_comstr[0] (80077214)
    at += v0;
    a1 = lw(at);
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x20D4;                                       // Result = STR_Sys_DiskError_Msg2[0] (800120D4)
    LIBC2_printf();
loc_80055590:
    v0 = lbu(s0);                                       // Load from: 800774CC
    v1 = v0 - 1;
    v0 = (v1 < 5);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_800558AC;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x210C;                                       // Result = JumpTable_LIBCD_BIOS_getintr[0] (8001210C)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80055734: goto loc_80055734;
        case 0x800556E4: goto loc_800556E4;
        case 0x800555C4: goto loc_800555C4;
        case 0x80055784: goto loc_80055784;
        case 0x8005581C: goto loc_8005581C;
        default: jump_table_err(); break;
    }
loc_800555C4:
    v1 = 5;                                             // Result = 00000005
    if (s1 == 0) goto loc_80055614;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74CD;                                       // Result = 800774CD
    sb(v1, v0);                                         // Store to: 800774CD
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    v1 = v0;                                            // Result = LIBCD_BIOS_result[0] (80086108)
    a1 = v1 - 8;                                        // Result = 80086100
    if (v1 == 0) goto loc_8005572C;
    a0 = 7;                                             // Result = 00000007
    a2 = -1;                                            // Result = FFFFFFFF
loc_800555F4:
    v0 = lbu(a1);
    a1++;
    a0--;
    sb(v0, v1);
    v1++;
    if (a0 != a2) goto loc_800555F4;
    v0 = 2;                                             // Result = 00000002
    goto loc_800558C4;
loc_80055614:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x74CD;                                       // Result = 800774CD
    v0 = lbu(v1);                                       // Load from: 800774CD
    if (v0 != 0) goto loc_80055698;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7210);                              // Load from: gLIBCD_CD_com (80077210)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x72B4;                                       // Result = gLIBCD_CD_pos (800772B4)
    at += v0;
    v0 = lw(at);
    {
        const bool bJump = (v0 == 0);
        v0 = 3;                                         // Result = 00000003
        if (bJump) goto loc_80055698;
    }
    sb(v0, v1);                                         // Store to: 800774CD
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    v1 = v0;                                            // Result = LIBCD_BIOS_result[0] (80086108)
    a1 = v1 - 8;                                        // Result = 80086100
    if (v1 == 0) goto loc_80055690;
    a0 = 7;                                             // Result = 00000007
    a2 = -1;                                            // Result = FFFFFFFF
loc_80055678:
    v0 = lbu(a1);
    a1++;
    a0--;
    sb(v0, v1);
    v1++;
    if (a0 != a2) goto loc_80055678;
loc_80055690:
    v0 = 1;                                             // Result = 00000001
    goto loc_800558C4;
loc_80055698:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74CD;                                       // Result = 800774CD
    v1 = 2;                                             // Result = 00000002
    sb(v1, v0);                                         // Store to: 800774CD
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    v1 = v0;                                            // Result = LIBCD_BIOS_result[0] (80086108)
    a1 = v1 - 8;                                        // Result = 80086100
    if (v1 == 0) goto loc_8005572C;
    a0 = 7;                                             // Result = 00000007
    a2 = -1;                                            // Result = FFFFFFFF
loc_800556C4:
    v0 = lbu(a1);
    a1++;
    a0--;
    sb(v0, v1);
    v1++;
    if (a0 != a2) goto loc_800556C4;
    v0 = 2;                                             // Result = 00000002
    goto loc_800558C4;
loc_800556E4:
    v0 = 2;                                             // Result = 00000002
    if (s1 == 0) goto loc_800556F0;
    v0 = 5;                                             // Result = 00000005
loc_800556F0:
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x74CD);                                // Store to: 800774CD
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    v1 = v0;                                            // Result = LIBCD_BIOS_result[0] (80086108)
    a1 = v1 - 8;                                        // Result = 80086100
    if (v1 == 0) goto loc_8005572C;
    a0 = 7;                                             // Result = 00000007
    a2 = -1;                                            // Result = FFFFFFFF
loc_80055714:
    v0 = lbu(a1);
    a1++;
    a0--;
    sb(v0, v1);
    v1++;
    if (a0 != a2) goto loc_80055714;
loc_8005572C:
    v0 = 2;                                             // Result = 00000002
    goto loc_800558C4;
loc_80055734:
    v0 = 1;                                             // Result = 00000001
    if (s1 == 0) goto loc_80055740;
    v0 = 5;                                             // Result = 00000005
loc_80055740:
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x74CE);                                // Store to: 800774CE
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    v1 = v0;                                            // Result = LIBCD_BIOS_result[2] (80086110)
    a1 = v1 - 0x10;                                     // Result = 80086100
    if (v1 == 0) goto loc_80055814;
    a0 = 7;                                             // Result = 00000007
    a2 = -1;                                            // Result = FFFFFFFF
loc_80055764:
    v0 = lbu(a1);
    a1++;
    a0--;
    sb(v0, v1);
    v1++;
    if (a0 != a2) goto loc_80055764;
    v0 = 4;                                             // Result = 00000004
    goto loc_800558C4;
loc_80055784:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6118;                                       // Result = LIBCD_BIOS_result[4] (80086118)
    a1 = v0;                                            // Result = LIBCD_BIOS_result[4] (80086118)
    a2 = a1 - 0x18;                                     // Result = 80086100
    a0 = 4;                                             // Result = 00000004
    at = 0x80070000;                                    // Result = 80070000
    sb(a0, at + 0x74CF);                                // Store to: 800774CF
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x74CF);                              // Load from: 800774CF
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74CE;                                       // Result = 800774CE
    sb(v1, v0);                                         // Store to: 800774CE
    at = 0x80070000;                                    // Result = 80070000
    sb(a0, at + 0x74CF);                                // Store to: 800774CF
    v1 = 7;                                             // Result = 00000007
    if (a1 == 0) goto loc_800557E0;
    a0 = -1;                                            // Result = FFFFFFFF
loc_800557C8:
    v0 = lbu(a2);
    a2++;
    v1--;
    sb(v0, a1);
    a1++;
    if (v1 != a0) goto loc_800557C8;
loc_800557E0:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    v1 = v0;                                            // Result = LIBCD_BIOS_result[2] (80086110)
    a1 = v1 - 0x10;                                     // Result = 80086100
    if (v1 == 0) goto loc_80055814;
    a0 = 7;                                             // Result = 00000007
    a2 = -1;                                            // Result = FFFFFFFF
loc_800557FC:
    v0 = lbu(a1);
    a1++;
    a0--;
    sb(v0, v1);
    v1++;
    if (a0 != a2) goto loc_800557FC;
loc_80055814:
    v0 = 4;                                             // Result = 00000004
    goto loc_800558C4;
loc_8005581C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    a0 = v0;                                            // Result = LIBCD_BIOS_result[0] (80086108)
    a1 = a0 - 8;                                        // Result = 80086100
    v0 = 5;                                             // Result = 00000005
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x74CE);                                // Store to: 800774CE
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x74CE);                              // Load from: 800774CE
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74CD;                                       // Result = 800774CD
    sb(v1, v0);                                         // Store to: 800774CD
    v1 = 7;                                             // Result = 00000007
    if (a0 == 0) goto loc_80055870;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80055858:
    v0 = lbu(a1);
    a1++;
    v1--;
    sb(v0, a0);
    a0++;
    if (v1 != a2) goto loc_80055858;
loc_80055870:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    v1 = v0;                                            // Result = LIBCD_BIOS_result[2] (80086110)
    a1 = v1 - 0x10;                                     // Result = 80086100
    if (v1 == 0) goto loc_800558A4;
    a0 = 7;                                             // Result = 00000007
    a2 = -1;                                            // Result = FFFFFFFF
loc_8005588C:
    v0 = lbu(a1);
    a1++;
    a0--;
    sb(v0, v1);
    v1++;
    if (a0 != a2) goto loc_8005588C;
loc_800558A4:
    v0 = 6;                                             // Result = 00000006
    goto loc_800558C4;
loc_800558AC:
    a1 = lbu(s0);                                       // Load from: 800774CC
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x20EC;                                       // Result = STR_Sys_CDROM_UnknownIntr_Err[0] (800120EC)
    LIBC2_printf();
    v0 = -1;                                            // Result = FFFFFFFF
loc_800558C4:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void LIBCD_CD_sync() noexcept {
loc_800558DC:
    sp -= 0x38;
    sw(s5, sp + 0x2C);
    s5 = a0;
    sw(s6, sp + 0x30);
    s6 = a1;
    a0 = -1;                                            // Result = FFFFFFFF
    sw(ra, sp + 0x34);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    _thunk_LIBETC_VSync();
    s3 = 0x80070000;                                    // Result = 80070000
    s3 += 0x7294;                                       // Result = gLIBCD_CD_intstr[0] (80077294)
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x74CD;                                       // Result = 800774CD
    s4 = s2 + 1;                                        // Result = 800774CE
    v0 += 0x1E0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6120);                                // Store to: LIBCD_BIOS_alarm[0] (80086120)
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x2120;                                       // Result = STR_Sys_CD_Sync_Msg[0] (80012120)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x6124);                                 // Store to: LIBCD_BIOS_alarm[1] (80086124)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6128);                                // Store to: LIBCD_BIOS_alarm[2] (80086128)
loc_80055948:
    // Emulate a little in case code is polling for CD data, waiting for events etc.
    #if PC_PSX_DOOM_MODS
        emulate_a_little();
    #endif

    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x6120);                               // Load from: LIBCD_BIOS_alarm[0] (80086120)
    v1 = (i32(v1) < i32(v0));
    if (v1 != 0) goto loc_80055990;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x6124);                               // Load from: LIBCD_BIOS_alarm[1] (80086124)
    v0 = v1 + 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6124);                                // Store to: LIBCD_BIOS_alarm[1] (80086124)
    v0 = 0x1E0000;                                      // Result = 001E0000
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_800559F8;
loc_80055990:
    a0 = lbu(s2);                                       // Load from: 800774CD
    v0 = lbu(s2 + 0x1);                                 // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 + 0x6128);                               // Load from: LIBCD_BIOS_alarm[2] (80086128)
    v0 <<= 2;
    v0 += s3;
    a0 <<= 2;
    v1 = lw(v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7210);                              // Load from: gLIBCD_CD_com (80077210)
    a0 += s3;
    v0 <<= 2;
    sw(v1, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7214;                                       // Result = gLIBCD_CD_comstr[0] (80077214)
    at += v0;
    a2 = lw(at);
    a3 = lw(a0);
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x20B0;                                       // Result = STR_Sys_TimeOutSync_Msg[0] (800120B0)
    LIBC2_printf();
    LIBCD_CD_init();
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_800559FC;
loc_800559F8:
    v0 = 0;                                             // Result = 00000000
loc_800559FC:
    {
        const bool bJump = (v0 != 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_80055B24;
    }
    LIBETC_CheckCallback();
    if (v0 == 0) goto loc_80055ABC;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    v0 = lbu(v0);
    s1 = v0 & 3;
loc_80055A2C:
    LIBCD_BIOS_getintr();
    s0 = v0;
    v0 = s0 & 4;
    if (s0 == 0) goto loc_80055AAC;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 2;
        if (bJump) goto loc_80055A74;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x71F8);                               // Load from: gpLIBCD_CD_cbready (800771F8)
    if (v0 == 0) goto loc_80055A70;
    a0 = lbu(s4);                                       // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    ptr_call(v0);
loc_80055A70:
    v0 = s0 & 2;
loc_80055A74:
    if (v0 == 0) goto loc_80055A2C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    if (v0 == 0) goto loc_80055A2C;
    a0 = lbu(s2);                                       // Load from: 800774CD
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    ptr_call(v0);
    goto loc_80055A2C;
loc_80055AAC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    sb(s1, v0);
loc_80055ABC:
    v0 = lbu(s2);                                       // Load from: 800774CD
    a2 = v0 & 0xFF;
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (a2 == v0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_80055ADC;
    }
    if (a2 != v0) goto loc_80055B1C;
loc_80055ADC:
    v0 = 2;                                             // Result = 00000002
    sb(v0, s2);                                         // Store to: 800774CD
    a1 = s6;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    v1 = 7;                                             // Result = 00000007
    if (a1 == 0) goto loc_80055B14;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80055AFC:
    v0 = lbu(a0);
    a0++;
    v1--;
    sb(v0, a1);
    a1++;
    if (v1 != a3) goto loc_80055AFC;
loc_80055B14:
    v0 = a2;
    goto loc_80055B24;
loc_80055B1C:
    v0 = 0;                                             // Result = 00000000
    if (s5 == 0) goto loc_80055948;
loc_80055B24:
    ra = lw(sp + 0x34);
    s6 = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x38;
}

void LIBCD_CD_ready() noexcept {
loc_80055B50:
    sp -= 0x40;
    sw(s7, sp + 0x34);
    s7 = a0;
    sw(s4, sp + 0x28);
    s4 = a1;
    a0 = -1;                                            // Result = FFFFFFFF
    sw(ra, sp + 0x38);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    _thunk_LIBETC_VSync();
    s5 = 0x80070000;                                    // Result = 80070000
    s5 += 0x7294;                                       // Result = gLIBCD_CD_intstr[0] (80077294)
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x74CD;                                       // Result = 800774CD
    s6 = s2 + 1;                                        // Result = 800774CE
    s3 = s2 + 2;                                        // Result = 800774CF
    v0 += 0x1E0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6120);                                // Store to: LIBCD_BIOS_alarm[0] (80086120)
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x2128;                                       // Result = STR_Sys_CD_Ready_Msg[0] (80012128)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x6124);                                 // Store to: LIBCD_BIOS_alarm[1] (80086124)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6128);                                // Store to: LIBCD_BIOS_alarm[2] (80086128)
loc_80055BC4:
    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x6120);                               // Load from: LIBCD_BIOS_alarm[0] (80086120)
    v1 = (i32(v1) < i32(v0));
    if (v1 != 0) goto loc_80055C0C;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x6124);                               // Load from: LIBCD_BIOS_alarm[1] (80086124)
    v0 = v1 + 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6124);                                // Store to: LIBCD_BIOS_alarm[1] (80086124)
    v0 = 0x1E0000;                                      // Result = 001E0000
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_80055C74;
loc_80055C0C:
    a0 = lbu(s2);                                       // Load from: 800774CD
    v0 = lbu(s2 + 0x1);                                 // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 + 0x6128);                               // Load from: LIBCD_BIOS_alarm[2] (80086128)
    v0 <<= 2;
    v0 += s5;
    a0 <<= 2;
    v1 = lw(v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7210);                              // Load from: gLIBCD_CD_com (80077210)
    a0 += s5;
    v0 <<= 2;
    sw(v1, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7214;                                       // Result = gLIBCD_CD_comstr[0] (80077214)
    at += v0;
    a2 = lw(at);
    a3 = lw(a0);
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x20B0;                                       // Result = STR_Sys_TimeOutSync_Msg[0] (800120B0)
    LIBC2_printf();
    LIBCD_CD_init();
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_80055C78;
loc_80055C74:
    v0 = 0;                                             // Result = 00000000
loc_80055C78:
    {
        const bool bJump = (v0 != 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_80055DE0;
    }
    LIBETC_CheckCallback();
    if (v0 == 0) goto loc_80055D38;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    v0 = lbu(v0);
    s1 = v0 & 3;
loc_80055CA8:
    LIBCD_BIOS_getintr();
    s0 = v0;
    v0 = s0 & 4;
    if (s0 == 0) goto loc_80055D28;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 2;
        if (bJump) goto loc_80055CF0;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x71F8);                               // Load from: gpLIBCD_CD_cbready (800771F8)
    if (v0 == 0) goto loc_80055CEC;
    a0 = lbu(s6);                                       // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    ptr_call(v0);
loc_80055CEC:
    v0 = s0 & 2;
loc_80055CF0:
    if (v0 == 0) goto loc_80055CA8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    if (v0 == 0) goto loc_80055CA8;
    a0 = lbu(s2);                                       // Load from: 800774CD
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    ptr_call(v0);
    goto loc_80055CA8;
loc_80055D28:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    sb(s1, v0);
loc_80055D38:
    v0 = lbu(s3);                                       // Load from: 800774CF
    a2 = v0 & 0xFF;
    if (a2 == 0) goto loc_80055D88;
    sb(0, s3);                                          // Store to: 800774CF
    a0 = 0x80080000;                                    // Result = 80080000
    a0 += 0x6118;                                       // Result = LIBCD_BIOS_result[4] (80086118)
    a1 = s4;
    if (s4 == 0) goto loc_80055DD0;
    v1 = 7;                                             // Result = 00000007
    a3 = -1;                                            // Result = FFFFFFFF
loc_80055D68:
    v0 = lbu(a0);
    a0++;
    v1--;
    sb(v0, a1);
    a1++;
    if (v1 != a3) goto loc_80055D68;
    v0 = a2;
    goto loc_80055DE0;
loc_80055D88:
    v0 = lbu(s3 - 0x1);                                 // Load from: 800774CE
    a2 = v0 & 0xFF;
    if (a2 == 0) goto loc_80055DD8;
    sb(0, s3 - 0x1);                                    // Store to: 800774CE
    a1 = s4;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    v1 = 7;                                             // Result = 00000007
    if (a1 == 0) goto loc_80055DD0;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80055DB8:
    v0 = lbu(a0);
    a0++;
    v1--;
    sb(v0, a1);
    a1++;
    if (v1 != a3) goto loc_80055DB8;
loc_80055DD0:
    v0 = a2;
    goto loc_80055DE0;
loc_80055DD8:
    v0 = 0;                                             // Result = 00000000
    if (s7 == 0) goto loc_80055BC4;
loc_80055DE0:
    ra = lw(sp + 0x38);
    s7 = lw(sp + 0x34);
    s6 = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x40;
}

void LIBCD_CD_cw() noexcept {
    sp -= 0x48;
    sw(s0, sp + 0x28);
    sw(s1, sp + 0x2C);
    sw(s2, sp + 0x30);
    sw(s3, sp + 0x34);
    sw(s4, sp + 0x38);
    sw(s5, sp + 0x3C);
    
    s1 = a0;
    s0 = a1;
    s5 = a2;
    s2 = a3;
    
    v0 = lw(0x80077200);        // Load from: gLIBCD_CD_debug (80077200)
    
    if (i32(v0) >= 2) {
        v0 = s1 & 0xFF;
        v0 <<= 2;
        at = 0x80077214;        // Result = gLIBCD_CD_comstr[0] (80077214)
        at += v0;    
        a0 = 0x80012134;        // Result = STR_Sys_MessageLine_Msg[0] (80012134)
        a1 = lw(at);
        LIBC2_printf();
    }

    v0 = s1 & 0xFF;
    v1 = v0 << 2;
    at = 0x80077434;            // Result = 80077434
    at += v1;
    v0 = lw(at);

    if (v0 != 0 && s0 == 0) {
        v0 = lw(0x80077200);        // Load from: gLIBCD_CD_debug (80077200)

        if (i32(v0) <= 0) {
            v0 = -2;
            goto loc_800561F4;
        }

        at = 0x80077214;            // Result = gLIBCD_CD_comstr[0] (80077214)
        at += v1;    
        a0 = 0x8001213C;            // Result = STR_Sys_NoParam_Err[0] (8001213C)
        a1 = lw(at);
        LIBC2_printf();

        v0 = -2;
        goto loc_800561F4;
    }

    v1 = s1 & 0xFF;
    v0 = 2;
    a0 = 0;

    if (v1 == v0) {
        v1 = s0;
        do {
            v0 = lbu(v1);
            at = 0x8007720C;            // Result = gLIBCD_CD_nopen (8007720C)
            at += a0;
            sb(v0, at);
            a0++;
            v1++;
        } while (i32(a0) < 4);
    }
    
    a0 = 0;
    a1 = 0;
    LIBCD_CD_sync();
    sb(0, 0x800774CD);              // Store to: 800774CD
    v0 = s1 & 0xFF;
    a0 = v0 << 2;
    at = 0x80077334;                // Result = 80077334
    at += a0;
    v0 = lw(at);
    v1 = 0x80077334;                // Result = 80077334

    if (v0 != 0) {
        sb(0, 0x800774CE);          // Store to: 800774CE
    }

    v0 = lw(0x800774B4);            // Load from: 800774B4
    sb(0, v0);
    v0 = v1 + 0x100;                // Result = 80077434
    v1 = a0 + v0;
    v0 = lw(v1);
    a0 = 0;

    if (i32(v0) > 0) {
        a1 = v1;
        do {
            v1 = lw(0x800774BC);        // Load from: 800774BC
            v0 = lbu(s0);
            s0++;
            sb(v0, v1);
            v0 = lw(a1);
            a0++;
        } while (i32(a0) < i32(v0));
    }

    v0 = lw(0x800774B8);            // Load from: 800774B8
    sb(s1, 0x80077210);             // Store to: gLIBCD_CD_com (80077210)
    sb(s1, v0);
    v0 = 0;

    if (s2 == 0) {
        a0 = -1;
        _thunk_LIBETC_VSync();
        v0 += 0x1E0;
        a0 = 0x800774CD;            // Result = 800774CD
        sw(v0, 0x80086120);         // Store to: LIBCD_BIOS_alarm[0] (80086120)
        sw(0, 0x80086124);          // Store to: LIBCD_BIOS_alarm[1] (80086124)
        v1 = lbu(a0);               // Load from: 800774CD
        v0 = 0x8001214C;            // Result = STR_Sys_CD_cw_Msg[0] (8001214C)
        sw(v0, 0x80086128);         // Store to: LIBCD_BIOS_alarm[2] (80086128)
        a2 = s5;
        
        if (v1 == 0) {
            s3 = 0x80077294;        // Result = gLIBCD_CD_intstr[0] (80077294)
            s2 = a0;                // Result = 800774CD
            s4 = s2 + 1;            // Result = 800774CE
            do {
                a0 = -1;
                _thunk_LIBETC_VSync();
                v1 = lw(0x80086120);                    // Load from: LIBCD_BIOS_alarm[0] (80086120)

                if (i32(v1) < i32(v0)) 
                    goto loc_8005606C;

                v1 = lw(0x80086124);                    // Load from: LIBCD_BIOS_alarm[1] (80086124)
                sw(v1 + 1, 0x80086124);                 // Store to: LIBCD_BIOS_alarm[1] (80086124)

                if (i32(v1) <= i32(0x1E0000))
                    goto loc_800560D4;

            loc_8005606C:
                {
                    a0 = lbu(s2);                       // Load from: 800774CD
                    v0 = lbu(s2 + 0x1);                 // Load from: 800774CE
                    a1 = lw(0x80086128);                // Load from: LIBCD_BIOS_alarm[2] (80086128)
                    v0 <<= 2;
                    v0 += s3;
                    a0 <<= 2;
                    v1 = lw(v0);
                    v0 = lbu(0x80077210);               // Load from: gLIBCD_CD_com (80077210)
                    a0 += s3;
                    v0 <<= 2;
                    sw(v1, sp + 0x10);
                    at = 0x80077214;                    // Result = gLIBCD_CD_comstr[0] (80077214)
                    at += v0;
                    a2 = lw(at);
                    a3 = lw(a0);
                    a0 = 0x800120B0;                    // Result = STR_Sys_TimeOutSync_Msg[0] (800120B0)
                    LIBC2_printf();
                    LIBCD_CD_init();
                    v0 = -1;
                    goto loc_800560D8;
                }
            loc_800560D4:
                {
                    v0 = 0;
                }

            loc_800560D8:
                if (v0 != 0) {
                    v0 = -1;
                    goto loc_800561F4;
                }

                LIBETC_CheckCallback();

                if (v0 != 0) {
                    v0 = lw(0x800774B4);                    // Load from: 800774B4
                    v0 = lbu(v0);
                    s1 = v0 & 3;

                    while (true) {
                        LIBCD_BIOS_getintr();
                        s0 = v0;
                        v0 = s0 & 4;

                        if (s0 == 0)
                            break;

                        if (v0 != 0) {
                            v0 = lw(0x800771F8);            // Load from: gpLIBCD_CD_cbready (800771F8)

                            if (v0 != 0) {
                                a0 = lbu(s4);               // Load from: 800774CE
                                a1 = 0x80086110;            // Result = LIBCD_BIOS_result[2] (80086110)
                                ptr_call(v0);
                            }
                        }

                        v0 = s0 & 2;

                        if (v0 == 0)
                            continue;
                    
                        v0 = lw(0x800771F4);        // Load from: gpLIBCD_CD_cbsync (800771F4)

                        if (v0 == 0)
                            continue;

                        a0 = lbu(s2);               // Load from: 800774CD
                        a1 = 0x80086108;            // Result = LIBCD_BIOS_result[0] (80086108)
                        ptr_call(v0);
                    }

                    v0 = lw(0x800774B4);            // Load from: 800774B4
                    sb(s1, v0);
                }

                v0 = lbu(s2);                       // Load from: 800774CD
                a2 = s5;
            } while (v0 == 0);
        }
        
        a0 = 0x80086108;    // Result = LIBCD_BIOS_result[0] (80086108)

        if (a2 != 0) {
            v1 = 7;
            a1 = -1;

            do {
                v0 = lbu(a0);
                a0++;
                v1--;
                sb(v0, a2);
                a2++;
            } while (v1 != a1);
        }

        v0 = lbu(0x800774CD);                   // Load from: 800774CD
        v0 ^= 5;
        v0 = (v0 < 1);
        v0 = -v0;
    }

loc_800561F4:
    s0 = lw(sp + 0x28);
    s1 = lw(sp + 0x2C);
    s2 = lw(sp + 0x30);
    s3 = lw(sp + 0x34);
    s4 = lw(sp + 0x38);
    s5 = lw(sp + 0x3C);
    
    sp += 0x48;
}

void LIBCD_CD_vol() noexcept {
loc_8005621C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B4);                               // Load from: 800774B4
    v0 = 2;                                             // Result = 00000002
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74BC);                               // Load from: 800774BC
    v0 = lbu(a0);
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74C0);                               // Load from: 800774C0
    v0 = lbu(a0 + 0x1);
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B4);                               // Load from: 800774B4
    v0 = 3;                                             // Result = 00000003
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B8);                               // Load from: 800774B8
    v0 = lbu(a0 + 0x2);
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74BC);                               // Load from: 800774BC
    v0 = lbu(a0 + 0x3);
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74C0);                               // Load from: 800774C0
    v0 = 0x20;                                          // Result = 00000020
    sb(v0, v1);
    v0 = 0;                                             // Result = 00000000
    return;
}

void LIBCD_CD_shell() noexcept {
loc_800562A4:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7204);                               // Load from: gLIBCD_CD_status (80077204)
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    v0 = a1 & 0x10;
    sw(ra, sp + 0x14);
    if (v0 == 0) goto loc_80056318;
loc_800562C8:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x2154;                                       // Result = STR_Sys_CD_open_Msg[0] (80012154)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x71F4);                                 // Store to: gpLIBCD_CD_cbsync (800771F4)
    LIBC2_printf();
    a0 = 1;                                             // Result = 00000001
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    a0 = 0x3C;                                          // Result = 0000003C
    _thunk_LIBETC_VSync();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7204);                               // Load from: gLIBCD_CD_status (80077204)
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x71F4);                                // Store to: gpLIBCD_CD_cbsync (800771F4)
    v0 = a1 & 0x10;
    if (v0 != 0) goto loc_800562C8;
loc_80056318:
    v0 = 0;                                             // Result = 00000000
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
}

void LIBCD_CD_flush() noexcept {
loc_80056330:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B4);                               // Load from: 800774B4
    v0 = 1;                                             // Result = 00000001
    sb(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74C0);                               // Load from: 800774C0
    v0 = lbu(v0);
    v0 &= 7;
    v1 = 7;                                             // Result = 00000007
    if (v0 == 0) goto loc_800563B4;
    a0 = 1;                                             // Result = 00000001
loc_80056364:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    sb(a0, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74C0);                               // Load from: 800774C0
    sb(v1, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74BC);                               // Load from: 800774BC
    sb(v1, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74C0);                               // Load from: 800774C0
    v0 = lbu(v0);
    v0 &= 7;
    if (v0 != 0) goto loc_80056364;
loc_800563B4:
    at = 0x80070000;                                    // Result = 80070000
    sb(0, at + 0x74CF);                                 // Store to: 800774CF
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x74CF);                              // Load from: 800774CF
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74CE;                                       // Result = 800774CE
    sb(v1, v0);                                         // Store to: 800774CE
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B4);                               // Load from: 800774B4
    v0 = 2;                                             // Result = 00000002
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x74CD);                                // Store to: 800774CD
    sb(0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74C0);                               // Load from: 800774C0
    sb(0, v0);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74C4);                               // Load from: 800774C4
    v0 = 0x132C;                                        // Result = 0000132C
    sw(v0, v1);
    return;
}

void LIBCD_CD_init() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);

    a0 = 0x80012168;            // Result = STR_Sys_CD_init_Msg[0] (80012168)
    a1 = 0x800774F8;            // Result = 800774F8
    LIBC2_printf();

    v1 = 0x800774D0;            // Result = 800774D0
    v0 = 9;
    a0 = -1;
    sw(0, 0x800771F8);          // Store to: gpLIBCD_CD_cbready (800771F8)
    sw(0, 0x800771F4);          // Store to: gpLIBCD_CD_cbsync (800771F4)
    sw(0, 0x80077204);          // Store to: gLIBCD_CD_status (80077204)

    do {
        sw(0, v1);
        v0--;
        v1 += 4;
    } while (v0 != a0);

    LIBETC_ResetCallback();

    a0 = 2;
    a1 = 0x8005701C;                    // Result = LIBCD_BIOS_callback (8005701C)    
    LIBETC_InterruptCallback();

    v1 = lw(0x800774B4);                // Load from: 800774B4
    v0 = 1;
    sb(v0, v1);
    v0 = lw(0x800774C0);                // Load from: 800774C0
    v0 = lbu(v0);
    v0 &= 7;
    a0 = 1;
    v1 = 7;

    while (v0 != 0) {
        v0 = lw(0x800774B4);            // Load from: 800774B4
        sb(a0, v0);
        v0 = lw(0x800774C0);            // Load from: 800774C0
        sb(v1, v0);
        v0 = lw(0x800774BC);            // Load from: 800774BC
        sb(v1, v0);
        v0 = lw(0x800774C0);            // Load from: 800774C0
        v0 = lbu(v0);
        v0 &= 7;
    }

    sb(0, 0x800774CF);              // Store to: 800774CF
    v1 = lbu(0x800774CF);           // Load from: 800774CF
    v0 = 0x800774CE;                // Result = 800774CE
    sb(v1, v0);                     // Store to: 800774CE
    v1 = lw(0x800774B4);            // Load from: 800774B4
    v0 = 2;
    sb(v0, 0x800774CD);             // Store to: 800774CD
    sb(0, v1);
    v0 = lw(0x800774C0);            // Load from: 800774C0
    sb(0, v0);
    v1 = lw(0x800774C4);            // Load from: 800774C4
    v0 = 0x132C;
    sw(v0, v1);

    a0 = 1;
    a1 = 0;
    a2 = 0;
    a3 = 0;
    LIBCD_CD_cw();

    v0 = lw(0x80077204);            // Load from: gLIBCD_CD_status (80077204)
    v0 &= 0x10;
    
    if (v0 != 0) {
        a0 = 1;
        a1 = 0;
        a2 = 0;
        a3 = 0;
        LIBCD_CD_cw();
    }

    a1 = lw(0x80077204);            // Load from: gLIBCD_CD_status (80077204)
    s0 = lw(0x800771F4);            // Load from: gpLIBCD_CD_cbsync (800771F4)
    v0 = a1 & 0x10;

    while (v0 != 0) {
        a0 = 0x80012154;                // Result = STR_Sys_CD_open_Msg[0] (80012154)
        sw(0, 0x800771F4);              // Store to: gpLIBCD_CD_cbsync (800771F4)
        LIBC2_printf();

        a0 = 1;
        a1 = 0;
        a2 = 0;
        a3 = 0;
        LIBCD_CD_cw();

        a0 = 0x3C;
        _thunk_LIBETC_VSync();

        a1 = lw(0x80077204);            // Load from: gLIBCD_CD_status (80077204)
        sw(s0, 0x800771F4);             // Store to: gpLIBCD_CD_cbsync (800771F4)
        v0 = a1 & 0x10;
    }

    a0 = 0xA;
    a1 = 0;
    a2 = 0;
    a3 = 0;
    LIBCD_CD_cw();

    if (v0 == 0) {
        a0 = 0xC;
        a1 = 0;
        a2 = 0;
        a3 = 0;
        LIBCD_CD_cw();

        if (v0 == 0) {
            a0 = 0;
            a1 = 0;
            LIBCD_CD_sync();
            v0 ^= 2;
            v0 = (v0 > 0);
            v0 = -v0;
        } else {
            v0 = -1;
        }
    } else {
        v0 = -1;
    }

    s0 = lw(sp + 0x10);
    sp += 0x18;
}

void LIBCD_CD_initvol() noexcept {
loc_80056664:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74C8);                               // Load from: 800774C8
    v0 = lhu(v1 + 0x1B8);
    sp -= 8;
    if (v0 != 0) goto loc_800566A0;
    v0 = lhu(v1 + 0x1BA);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x3FFF;                                    // Result = 00003FFF
        if (bJump) goto loc_800566A4;
    }
    sh(v0, v1 + 0x180);
    sh(v0, v1 + 0x182);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74C8);                               // Load from: 800774C8
loc_800566A0:
    v0 = 0x3FFF;                                        // Result = 00003FFF
loc_800566A4:
    sh(v0, v1 + 0x1B0);
    sh(v0, v1 + 0x1B2);
    v0 = 0xC001;                                        // Result = 0000C001
    sh(v0, v1 + 0x1AA);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B4);                               // Load from: 800774B4
    v0 = 0x80;                                          // Result = 00000080
    sb(v0, sp + 0x2);
    sb(v0, sp);
    v0 = 2;                                             // Result = 00000002
    sb(0, sp + 0x3);
    sb(0, sp + 0x1);
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74BC);                               // Load from: 800774BC
    v0 = lbu(sp);
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74C0);                               // Load from: 800774C0
    v0 = lbu(sp + 0x1);
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B4);                               // Load from: 800774B4
    v0 = 3;                                             // Result = 00000003
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B8);                               // Load from: 800774B8
    v0 = lbu(sp + 0x2);
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74BC);                               // Load from: 800774BC
    v0 = lbu(sp + 0x3);
    sb(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74C0);                               // Load from: 800774C0
    v0 = 0x20;                                          // Result = 00000020
    sb(v0, v1);
    v0 = 0;                                             // Result = 00000000
    sp += 8;
    return;
}

void LIBCD_BIOS_cd_read_retry() noexcept {
loc_80056758:
    sp -= 0x38;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x74E8;                                       // Result = 800774E8
    v0 = -1;                                            // Result = FFFFFFFF
    sw(ra, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    sw(v0, v1);                                         // Store to: 800774E8
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74D0);                               // Load from: 800774D0
    if (v0 == 0) goto loc_80056A20;
    s4 = 1;                                             // Result = 00000001
    s3 = 7;                                             // Result = 00000007
    s2 = v1;                                            // Result = 800774E8
    s1 = v1 - 0xC;                                      // Result = 800774DC
loc_800567A8:
    v0 = lw(s2);                                        // Load from: 800774E8
    if (i32(v0) >= 0) goto loc_80056A20;
    v0 = lw(s2 - 0x18);                                 // Load from: 800774D0
    v0--;
    sw(v0, s2 - 0x18);                                  // Store to: 800774D0
    v0 = lw(s2 - 0x18);                                 // Load from: 800774D0
    v0 = lw(s2 - 0x18);                                 // Load from: 800774D0
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x71F8);                                 // Store to: gpLIBCD_CD_cbready (800771F8)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x71F4);                                 // Store to: gpLIBCD_CD_cbsync (800771F4)
    v0 = (i32(v0) < 0xF);
    if (v0 == 0) goto loc_800568BC;
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lbu(a2 + 0x720C);                              // Load from: gLIBCD_CD_nopen (8007720C)
    a1 = lw(s2 - 0x18);                                 // Load from: 800774D0
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lbu(a3 + 0x720D);                              // Load from: gLIBCD_CD_nopen + 1 (8007720D) (8007720D)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x720E);                              // Load from: gLIBCD_CD_nopen + 2 (8007720E) (8007720E)
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x217C;                                       // Result = STR_Sys_CD_read_retry_Msg[0] (8001217C)
    sw(v0, sp + 0x10);
    LIBC2_printf();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7204);                               // Load from: gLIBCD_CD_status (80077204)
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    v0 = a1 & 0x10;
    if (v0 == 0) goto loc_80056888;
loc_80056838:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x2154;                                       // Result = STR_Sys_CD_open_Msg[0] (80012154)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x71F4);                                 // Store to: gpLIBCD_CD_cbsync (800771F4)
    LIBC2_printf();
    a0 = 1;                                             // Result = 00000001
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    a0 = 0x3C;                                          // Result = 0000003C
    _thunk_LIBETC_VSync();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7204);                               // Load from: gLIBCD_CD_status (80077204)
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x71F4);                                // Store to: gpLIBCD_CD_cbsync (800771F4)
    v0 = a1 & 0x10;
    if (v0 != 0) goto loc_80056838;
loc_80056888:
    a0 = 8;                                             // Result = 00000008
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    a0 = 0xA;                                           // Result = 0000000A
    if (v0 != 0) goto loc_80056A08;
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    if (v0 != 0) goto loc_80056A08;
loc_800568BC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    sb(s4, v0);
    goto loc_80056904;
loc_800568D4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    sb(s4, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74C0);                               // Load from: 800774C0
    sb(s3, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74BC);                               // Load from: 800774BC
    sb(s3, v0);
loc_80056904:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74C0);                               // Load from: 800774C0
    v0 = lbu(v0);
    v0 &= 7;
    a0 = 0xE;                                           // Result = 0000000E
    if (v0 != 0) goto loc_800568D4;
    a1 = sp + 0x18;
    at = 0x80070000;                                    // Result = 80070000
    sb(0, at + 0x74CF);                                 // Store to: 800774CF
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x74CF);                              // Load from: 800774CF
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74CE;                                       // Result = 800774CE
    sb(v1, v0);                                         // Store to: 800774CE
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74B4);                               // Load from: 800774B4
    v0 = 2;                                             // Result = 00000002
    at = 0x80070000;                                    // Result = 80070000
    sb(v0, at + 0x74CD);                                // Store to: 800774CD
    sb(0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74C0);                               // Load from: 800774C0
    a2 = 0;                                             // Result = 00000000
    sb(0, v0);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74C4);                               // Load from: 800774C4
    v0 = 0x132C;                                        // Result = 0000132C
    sw(v0, v1);
    v0 = lw(s1 - 0x4);                                  // Load from: 800774D8
    a3 = 0;                                             // Result = 00000000
    sw(v0, s1);                                         // Store to: 800774DC
    v1 = lw(s1 + 0x4);                                  // Load from: 800774E0
    v0 = 0x80050000;                                    // Result = 80050000
    v0 += 0x70FC;                                       // Result = LIBCD_BIOS_cb_read (800570FC)
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x71F8);                                // Store to: gpLIBCD_CD_cbready (800771F8)
    sb(v1, sp + 0x18);
    LIBCD_CD_cw();
    a0 = 2;                                             // Result = 00000002
    if (v0 != 0) goto loc_80056A08;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x720C;                                       // Result = gLIBCD_CD_nopen (8007720C)
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    a0 = 6;                                             // Result = 00000006
    if (v0 != 0) goto loc_80056A08;
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    if (v0 != 0) goto loc_80056A08;
    v0 = lw(s1 - 0x8);                                  // Load from: 800774D4
    a0 = -1;                                            // Result = FFFFFFFF
    sw(v0, s1 + 0xC);                                   // Store to: 800774E8
    _thunk_LIBETC_VSync();
    v1 = lw(s1 - 0x8);                                  // Load from: 800774D4
    v1 += 0x1E0;
    v0 += v1;
    sw(v0, s1 + 0x10);                                  // Store to: 800774EC
loc_80056A08:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74D0;                                       // Result = 800774D0
    v0 = lw(v0);                                        // Load from: 800774D0
    if (v0 != 0) goto loc_800567A8;
loc_80056A20:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74E8;                                       // Result = 800774E8
    v0 = lw(v0);                                        // Load from: 800774E8
    ra = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x38;
    return;
}

void LIBCD_CD_readm() noexcept {
loc_80056A50:
    sp -= 0x18;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74E0;                                       // Result = 800774E0
    sw(ra, sp + 0x10);
    sw(a2, v0);                                         // Store to: 800774E0
    v0 = lw(v0);                                        // Load from: 800774E0
    v1 = v0 & 0x30;
    v0 = 0x20;                                          // Result = 00000020
    if (v1 == 0) goto loc_80056A88;
    {
        const bool bJump = (v1 == v0);
        v0 = 0x249;                                     // Result = 00000249
        if (bJump) goto loc_80056A9C;
    }
    v0 = 0x246;                                         // Result = 00000246
    goto loc_80056AAC;
loc_80056A88:
    v0 = 0x200;                                         // Result = 00000200
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x74E4);                                // Store to: 800774E4
    goto loc_80056AB8;
loc_80056A9C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x74E4);                                // Store to: 800774E4
    goto loc_80056AB8;
loc_80056AAC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x74E4;                                       // Result = 800774E4
    sw(v0, v1);                                         // Store to: 800774E4
loc_80056AB8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74D8;                                       // Result = 800774D8
    sw(a0, v0);                                         // Store to: 800774D8
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x71F8);                               // Load from: gpLIBCD_CD_cbready (800771F8)
    v0 = 0x10;                                          // Result = 00000010
    at = 0x80070000;                                    // Result = 80070000
    sw(a1, at + 0x74D4);                                // Store to: 800774D4
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x74D0);                                // Store to: 800774D0
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x74F0);                                // Store to: 800774F0
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x74F4);                                // Store to: 800774F4
    LIBCD_BIOS_cd_read_retry();
    v0 = (i32(v0) < 1);
    v0 = -v0;
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void LIBCD_CD_readsync() noexcept {
loc_80056B18:
    sp -= 0x40;
    sw(s6, sp + 0x30);
    s6 = a0;
    sw(s7, sp + 0x34);
    s7 = a1;
    a0 = -1;                                            // Result = FFFFFFFF
    sw(ra, sp + 0x38);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    _thunk_LIBETC_VSync();
    s4 = 0x80070000;                                    // Result = 80070000
    s4 += 0x7294;                                       // Result = gLIBCD_CD_intstr[0] (80077294)
    s3 = 0x80070000;                                    // Result = 80070000
    s3 += 0x74CD;                                       // Result = 800774CD
    s5 = s3 + 1;                                        // Result = 800774CE
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x74EC;                                       // Result = 800774EC
    v0 += 0x1E0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6120);                                // Store to: LIBCD_BIOS_alarm[0] (80086120)
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x21A0;                                       // Result = STR_Sys_CD_read_Msg[0] (800121A0)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x6124);                                 // Store to: LIBCD_BIOS_alarm[1] (80086124)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6128);                                // Store to: LIBCD_BIOS_alarm[2] (80086128)
loc_80056B90:
    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x6120);                               // Load from: LIBCD_BIOS_alarm[0] (80086120)
    v1 = (i32(v1) < i32(v0));
    if (v1 != 0) goto loc_80056BD8;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x6124);                               // Load from: LIBCD_BIOS_alarm[1] (80086124)
    v0 = v1 + 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6124);                                // Store to: LIBCD_BIOS_alarm[1] (80086124)
    v0 = 0x1E0000;                                      // Result = 001E0000
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_80056C40;
loc_80056BD8:
    a0 = lbu(s3);                                       // Load from: 800774CD
    v0 = lbu(s3 + 0x1);                                 // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 + 0x6128);                               // Load from: LIBCD_BIOS_alarm[2] (80086128)
    v0 <<= 2;
    v0 += s4;
    a0 <<= 2;
    v1 = lw(v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7210);                              // Load from: gLIBCD_CD_com (80077210)
    a0 += s4;
    v0 <<= 2;
    sw(v1, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7214;                                       // Result = gLIBCD_CD_comstr[0] (80077214)
    at += v0;
    a2 = lw(at);
    a3 = lw(a0);
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x20B0;                                       // Result = STR_Sys_TimeOutSync_Msg[0] (800120B0)
    LIBC2_printf();
    LIBCD_CD_init();
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_80056C44;
loc_80056C40:
    v0 = 0;                                             // Result = 00000000
loc_80056C44:
    if (v0 != 0) goto loc_80056D8C;
    LIBETC_CheckCallback();
    a0 = s7;
    if (v0 == 0) goto loc_80056D08;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    v0 = lbu(v0);
    s1 = v0 & 3;
loc_80056C74:
    LIBCD_BIOS_getintr();
    s0 = v0;
    v0 = s0 & 4;
    if (s0 == 0) goto loc_80056CF4;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 2;
        if (bJump) goto loc_80056CBC;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x71F8);                               // Load from: gpLIBCD_CD_cbready (800771F8)
    if (v0 == 0) goto loc_80056CB8;
    a0 = lbu(s5);                                       // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    ptr_call(v0);
loc_80056CB8:
    v0 = s0 & 2;
loc_80056CBC:
    if (v0 == 0) goto loc_80056C74;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    if (v0 == 0) goto loc_80056C74;
    a0 = lbu(s3);                                       // Load from: 800774CD
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    ptr_call(v0);
    goto loc_80056C74;
loc_80056CF4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    sb(s1, v0);
    a0 = s7;
loc_80056D08:
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    v1 = 7;                                             // Result = 00000007
    if (a0 == 0) goto loc_80056D34;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80056D1C:
    v0 = lbu(a1);
    a1++;
    v1--;
    sb(v0, a0);
    a0++;
    if (v1 != a2) goto loc_80056D1C;
loc_80056D34:
    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    v1 = lw(s2);                                        // Load from: 800774EC
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80056D58;
    LIBCD_BIOS_cd_read_retry();
loc_80056D58:
    v0 = lw(s2 - 0x4);                                  // Load from: 800774E8
    if (v0 != 0) goto loc_80056D70;
    a0 = 0;                                             // Result = 00000000
    LIBCD_CD_datasync();
loc_80056D70:
    if (s6 != 0) goto loc_80056D88;
    v0 = lw(s2 - 0x4);                                  // Load from: 800774E8
    if (i32(v0) > 0) goto loc_80056B90;
loc_80056D88:
    v0 = lw(s2 - 0x4);                                  // Load from: 800774E8
loc_80056D8C:
    ra = lw(sp + 0x38);
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

void LIBCD_CD_datasync() noexcept {
loc_80056DBC:
    sp -= 0x30;
    sw(s2, sp + 0x20);
    s2 = a0;
    a0 = -1;                                            // Result = FFFFFFFF
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    _thunk_LIBETC_VSync();
    s3 = 0x80070000;                                    // Result = 80070000
    s3 += 0x7214;                                       // Result = gLIBCD_CD_comstr[0] (80077214)
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x74CD;                                       // Result = 800774CD
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x7294;                                       // Result = gLIBCD_CD_intstr[0] (80077294)
    v0 += 0x1E0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6120);                                // Store to: LIBCD_BIOS_alarm[0] (80086120)
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x21A8;                                       // Result = STR_Sys_CD_datasync_Msg[0] (800121A8)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x6124);                                 // Store to: LIBCD_BIOS_alarm[1] (80086124)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6128);                                // Store to: LIBCD_BIOS_alarm[2] (80086128)
loc_80056E1C:
    a0 = -1;                                            // Result = FFFFFFFF
    _thunk_LIBETC_VSync();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x6120);                               // Load from: LIBCD_BIOS_alarm[0] (80086120)
    v1 = (i32(v1) < i32(v0));
    if (v1 != 0) goto loc_80056E64;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 + 0x6124);                               // Load from: LIBCD_BIOS_alarm[1] (80086124)
    v0 = v1 + 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at + 0x6124);                                // Store to: LIBCD_BIOS_alarm[1] (80086124)
    v0 = 0x1E0000;                                      // Result = 001E0000
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_80056EC4;
loc_80056E64:
    a0 = lbu(s1);                                       // Load from: 800774CD
    v0 = lbu(s1 + 0x1);                                 // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 + 0x6128);                               // Load from: LIBCD_BIOS_alarm[2] (80086128)
    v0 <<= 2;
    v0 += s0;
    a0 <<= 2;
    v1 = lw(v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lbu(v0 + 0x7210);                              // Load from: gLIBCD_CD_com (80077210)
    a0 += s0;
    v0 <<= 2;
    v0 += s3;
    sw(v1, sp + 0x10);
    a2 = lw(v0);
    a3 = lw(a0);
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x20B0;                                       // Result = STR_Sys_TimeOutSync_Msg[0] (800120B0)
    LIBC2_printf();
    LIBCD_CD_init();
    v0 = -1;                                            // Result = FFFFFFFF
    goto loc_80056EC8;
loc_80056EC4:
    v0 = 0;                                             // Result = 00000000
loc_80056EC8:
    {
        const bool bJump = (v0 != 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_80056EF8;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x751C);                               // Load from: 8007751C
    v0 = lw(v0);
    v1 = 0x1000000;                                     // Result = 01000000
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80056EF8;
    }
    v0 = 1;                                             // Result = 00000001
    if (s2 == 0) goto loc_80056E1C;
loc_80056EF8:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void LIBCD_CD_getsector() noexcept {
loc_80056F18:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(ra, sp + 0x18);
    a0 = 0;                                             // Result = 00000000
    LIBCD_CD_datasync();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    a0 = 0x20000;                                       // Result = 00020000
    sb(0, v0);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74C0);                               // Load from: 800774C0
    v0 = 0x80;                                          // Result = 00000080
    sb(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x750C);                               // Load from: 8007750C
    a0 |= 0x943;                                        // Result = 00020943
    sw(a0, v0);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74C4);                               // Load from: 800774C4
    v0 = 0x1323;                                        // Result = 00001323
    sw(v0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7510);                               // Load from: 80077510
    v0 = lw(v1);
    v0 |= 0x8000;
    sw(v0, v1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x751C);                               // Load from: 8007751C
    sw(0, v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7514);                               // Load from: 80077514
    a0 = 0;                                             // Result = 00000000
    sw(s1, v0);
    v0 = 0x10000;                                       // Result = 00010000
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7518);                               // Load from: 80077518
    s0 |= v0;
    sw(s0, v1);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x751C);                               // Load from: 8007751C
    v0 = 0x11000000;                                    // Result = 11000000
    sw(v0, v1);
    LIBCD_CD_datasync();
    v0 = 0;                                             // Result = 00000000
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x74C4);                               // Load from: 800774C4
    v1 = 0x1325;                                        // Result = 00001325
    sw(v1, a0);
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void LIBCD_CD_set_test_parmnum() noexcept {
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x7498);                                // Store to: 80077498
    return;
}

void LIBCD_BIOS_callback() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lbu(v0);
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x74CE;                                       // Result = 800774CE
    s1 = v0 & 3;
loc_80057048:
    LIBCD_BIOS_getintr();
    s0 = v0;
    v0 = s0 & 4;
    if (s0 == 0) goto loc_800570D0;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 2;
        if (bJump) goto loc_80057090;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x71F8);                               // Load from: gpLIBCD_CD_cbready (800771F8)
    if (v0 == 0) goto loc_8005708C;
    a0 = lbu(s2);                                       // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    ptr_call(v0);
loc_8005708C:
    v0 = s0 & 2;
loc_80057090:
    if (v0 == 0) goto loc_80057048;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x71F4);                               // Load from: gpLIBCD_CD_cbsync (800771F4)
    if (v1 == 0) goto loc_80057048;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74CD;                                       // Result = 800774CD
    a0 = lbu(v0);                                       // Load from: 800774CD
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    ptr_call(v1);
    goto loc_80057048;
loc_800570D0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74B4);                               // Load from: 800774B4
    sb(s1, v0);
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void LIBCD_BIOS_cb_read() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a1;
    v0 = 5;                                             // Result = 00000005
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    if (a0 != v0) goto loc_80057134;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74CD;                                       // Result = 800774CD
    v0 = lbu(v0);                                       // Load from: 800774CD
    v0 &= 0xFF;
    if (v0 == a0) goto loc_80057248;
loc_80057134:
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (a0 != v0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_800571AC;
    }
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x74E8;                                       // Result = 800774E8
    v0 = lw(s0);                                        // Load from: 800774E8
    if (i32(v0) <= 0) goto loc_800571C0;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x74DC);                               // Load from: 800774DC
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x74E4);                               // Load from: 800774E4
    LIBCD_CD_getsector();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74E4);                               // Load from: 800774E4
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74DC);                               // Load from: 800774DC
    v0 <<= 2;
    v0 += v1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x74DC);                                // Store to: 800774DC
    v0 = lw(s0);                                        // Load from: 800774E8
    v0--;
    sw(v0, s0);                                         // Store to: 800774E8
    v0 = lw(s0);                                        // Load from: 800774E8
    goto loc_800571B8;
loc_800571AC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x74E8;                                       // Result = 800774E8
    sw(v0, v1);                                         // Store to: 800774E8
loc_800571B8:
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x74E8;                                       // Result = 800774E8
loc_800571C0:
    v0 = lw(s0);                                        // Load from: 800774E8
    if (i32(v0) >= 0) goto loc_800571D8;
    LIBCD_BIOS_cd_read_retry();
loc_800571D8:
    v0 = lw(s0);                                        // Load from: 800774E8
    a0 = 9;                                             // Result = 00000009
    if (i32(v0) > 0) goto loc_80057248;
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x74F0);                               // Load from: 800774F0
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x74F4);                               // Load from: 800774F4
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x71F4);                                // Store to: gpLIBCD_CD_cbsync (800771F4)
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x71F8);                                // Store to: gpLIBCD_CD_cbready (800771F8)
    a3 = 0;                                             // Result = 00000000
    LIBCD_CD_cw();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x71FC);                               // Load from: gpLIBCD_CD_cbread (800771FC)
    if (v1 == 0) goto loc_80057248;
    v0 = lw(s0);                                        // Load from: 800774E8
    a0 = 5;                                             // Result = 00000005
    if (v0 != 0) goto loc_80057240;
    a0 = 2;                                             // Result = 00000002
loc_80057240:
    a1 = s1;
    ptr_call(v1);
loc_80057248:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Retrieve the table of contents for the disc (tack positions) and return the number of tracks.
// If there is an error then '0' or less will be returned.
// The 0th track points past the last track on the disc.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t LIBCD_CdGetToc(CdlLOC trackLocs[CdlMAXTOC]) noexcept {    
    // Get the disc: if there is none then we can't provide a TOC
    device::cdrom::CDROM& cdrom = *PsxVm::gpCdrom;
    disc::Disc* const pDisc = cdrom.disc.get();

    if (!pDisc)
        return 0;
    
    // Retrieve the info for each track.
    // Note that if the 'empty' disc is being used then the count will be '0', which is regarded as an error when returned.
    const int32_t numTracks = (int32_t) pDisc->getTrackCount();

    for (int32_t trackNum = 1; trackNum <= numTracks; ++trackNum) {
        const disc::Position discPos = pDisc->getTrackStart(trackNum);
        
        CdlLOC& loc = trackLocs[trackNum];
        loc.minute = bcd::toBcd((uint8_t) discPos.mm);
        loc.second = bcd::toBcd((uint8_t) discPos.ss);
        loc.sector = 0;     // Sector number is not provided by the underlying CD-ROM command to query track location (0x14)
        loc.track = 0;      // Should always be '0' in this PsyQ version - unused
    }

    // The first track entry (0th index) should point to the end of the disk
    const disc::Position discEndPos = pDisc->getDiskSize();

    trackLocs[0].minute = bcd::toBcd((uint8_t) discEndPos.mm);
    trackLocs[0].second = bcd::toBcd((uint8_t) discEndPos.ss);
    trackLocs[0].sector = 0;
    trackLocs[0].track = 0;

    return numTracks;
}
