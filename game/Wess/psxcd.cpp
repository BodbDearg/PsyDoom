#include "PSXCD.h"

#include "Doom/cdmaptbl.h"
#include "psxspu.h"
#include "PsxVm/PsxVm.h"
#include "PsxVm/VmPtr.h"
#include "PsyQ/LIBCD.h"

// Used to hold a file temporarily after opening
static const VmPtr<PsxCd_File> gPSXCD_cdfile(0x8007831C);

// Last IO location on disc
static const VmPtr<CdlLOC> gPSXCD_cur_io_loc(0x80077D78);

// Function forward declarations
void psxcd_async_read_cancel() noexcept;
void psxcd_async_read() noexcept;

void PSXCD_psxcd_memcpy() noexcept {
loc_8003F200:
    sp -= 8;
    v1 = a2 - 1;
    if (a2 == 0) goto loc_8003F228;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003F210:
    v0 = lbu(a1);
    a1++;
    v1--;
    sb(v0, a0);
    a0++;
    if (v1 != a2) goto loc_8003F210;
loc_8003F228:
    sp += 8;
    return;
}

void psxcd_sync() noexcept {
loc_8003F234:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    sw(ra, sp + 0x18);
    s0 += 0x1F40;
    v0 = (v0 < s0);
    sw(s1, sp + 0x14);
    if (v0 == 0) goto loc_8003F2D8;
    s1 = 5;                                             // Result = 00000005
loc_8003F264:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DA8;                                       // Result = gPSXCD_sync_result[0] (80077DA8)
    a0 = 1;                                             // Result = 00000001
    LIBCD_CdSync();
    sw(v0, gp + 0x7C4);                                 // Store to: gPSXCD_sync_intr (80077DA4)
    if (v0 != s1) goto loc_8003F2B0;
    LIBCD_CdFlush();
    v0 = lw(gp + 0x7C4);                                // Load from: gPSXCD_sync_intr (80077DA4)
    a0 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    a1 = lbu(gp + 0x7C8);                               // Load from: gPSXCD_sync_result[0] (80077DA8)
    v1 = lw(gp + 0x7AC);                                // Load from: gPSXCD_cdl_errcount (80077D8C)
    v0 += 0x50;
    v1++;
    sw(v0, gp + 0x7A8);                                 // Store to: gPSXCD_cdl_errintr (80077D88)
    sb(a0, gp + 0x7B3);                                 // Store to: gPSXCD_cdl_errcom (80077D93)
    sb(a1, gp + 0x7B1);                                 // Store to: gPSXCD_cdl_errstat (80077D91)
    sw(v1, gp + 0x7AC);                                 // Store to: gPSXCD_cdl_errcount (80077D8C)
loc_8003F2B0:
    v1 = lw(gp + 0x7C4);                                // Load from: gPSXCD_sync_intr (80077DA4)
    v0 = 2;                                             // Result = 00000002
    if (v1 == v0) goto loc_8003F2D8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    v0 = (v0 < s0);
    if (v0 != 0) goto loc_8003F264;
loc_8003F2D8:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void psxcd_critical_sync() noexcept {
loc_8003F2F0:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    s0 += 0x1F40;
    v0 = (v0 < s0);
    sw(ra, sp + 0x14);
    if (v0 == 0) goto loc_8003F394;
loc_8003F318:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DA8;                                       // Result = gPSXCD_sync_result[0] (80077DA8)
    a0 = 1;                                             // Result = 00000001
    LIBCD_CdSync();
    v1 = v0;
    sw(v1, gp + 0x7C4);                                 // Store to: gPSXCD_sync_intr (80077DA4)
    v0 = 5;                                             // Result = 00000005
    {
        const bool bJump = (v1 != v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8003F374;
    }
    LIBCD_CdFlush();
    v1 = lw(gp + 0x7C4);                                // Load from: gPSXCD_sync_intr (80077DA4)
    a1 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    a2 = lbu(gp + 0x7C8);                               // Load from: gPSXCD_sync_result[0] (80077DA8)
    a0 = lw(gp + 0x7AC);                                // Load from: gPSXCD_cdl_errcount (80077D8C)
    v1 += 0x46;
    a0++;
    sw(v1, gp + 0x7A8);                                 // Store to: gPSXCD_cdl_errintr (80077D88)
    sb(a1, gp + 0x7B3);                                 // Store to: gPSXCD_cdl_errcom (80077D93)
    sb(a2, gp + 0x7B1);                                 // Store to: gPSXCD_cdl_errstat (80077D91)
    sw(a0, gp + 0x7AC);                                 // Store to: gPSXCD_cdl_errcount (80077D8C)
    v0 = 0;                                             // Result = 00000000
    goto loc_8003F398;
loc_8003F374:
    {
        const bool bJump = (v1 == v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8003F398;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    v0 = (v0 < s0);
    if (v0 != 0) goto loc_8003F318;
loc_8003F394:
    v0 = 0;                                             // Result = 00000000
loc_8003F398:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSXCD_cbcomplete() noexcept {
    v0 = lw(gp + 0x79C);                                // Load from: gbPSXCD_cb_enable_flag (80077D7C)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_8003F480;
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (a0 != v0);
        v0 = a0 + 0xA;
        if (bJump) goto loc_8003F460;
    }
    v1 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    v0 = 0x16;                                          // Result = 00000016
    {
        const bool bJump = (v1 != v0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_8003F44C;
    }
    v0 = lw(gp + 0x7E8);                                // Load from: gbPSXCD_playflag (80077DC8)
    sw(0, gp + 0x77C);                                  // Store to: gbPSXCD_seeking_for_play (80077D5C)
    if (v0 == 0) goto loc_8003F480;
    psxspu_setcdmixon();
    v0 = lw(gp + 0x7F0);                                // Load from: gPSXCD_playfadeuptime (80077DD0)
    if (v0 == 0) goto loc_8003F424;
    a0 = 0;                                             // Result = 00000000
    psxspu_set_cd_vol();
    a0 = lw(gp + 0x7F0);                                // Load from: gPSXCD_playfadeuptime (80077DD0)
    a1 = lw(gp + 0x7EC);                                // Load from: gPSXCD_playvol (80077DCC)
    psxspu_start_cd_fade();
    sw(0, gp + 0x7F0);                                  // Store to: gPSXCD_playfadeuptime (80077DD0)
    v0 = 3;                                             // Result = 00000003
    goto loc_8003F434;
loc_8003F424:
    a0 = lw(gp + 0x7EC);                                // Load from: gPSXCD_playvol (80077DCC)
    psxspu_set_cd_vol();
    v0 = 3;                                             // Result = 00000003
loc_8003F434:
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a0 = 3;                                             // Result = 00000003
    a1 = 0;                                             // Result = 00000000
    LIBCD_CdControlF();
    goto loc_8003F480;
loc_8003F44C:
    if (v1 != v0) goto loc_8003F480;
    sw(0, gp + 0x780);                                  // Store to: gbPSXCD_waiting_for_pause (80077D60)
    goto loc_8003F480;
loc_8003F460:
    v1 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    sw(v0, gp + 0x7A8);                                 // Store to: gPSXCD_cdl_errintr (80077D88)
    v0 = lw(gp + 0x7AC);                                // Load from: gPSXCD_cdl_errcount (80077D8C)
    sb(v1, gp + 0x7B3);                                 // Store to: gPSXCD_cdl_errcom (80077D93)
    v1 = lbu(a1);
    v0++;
    sw(v0, gp + 0x7AC);                                 // Store to: gPSXCD_cdl_errcount (80077D8C)
    sb(v1, gp + 0x7B1);                                 // Store to: gPSXCD_cdl_errstat (80077D91)
loc_8003F480:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PSXCD_cbready() noexcept {
    v0 = lw(gp + 0x79C);                                // Load from: gbPSXCD_cb_enable_flag (80077D7C)
    sp -= 0x30;
    sw(s0, sp + 0x20);
    s0 = a0;
    sw(s1, sp + 0x24);
    s1 = a1;
    sw(ra, sp + 0x2C);
    sw(s2, sp + 0x28);
    if (v0 == 0) goto loc_8003F878;
    sw(s0, gp + 0x7A4);                                 // Store to: gPSXCD_cdl_intr (80077D84)
    v0 = lbu(s1);
    sb(v0, gp + 0x7B0);                                 // Store to: gPSXCD_cdl_stat (80077D90)
    v0 = lbu(s1);
    v0 &= 0x20;
    if (v0 == 0) goto loc_8003F74C;
    v0 = lw(gp + 0x784);                                // Load from: gbPSXCD_async_on (80077D64)
    {
        const bool bJump = (v0 == 0);
        v0 = 6;                                         // Result = 00000006
        if (bJump) goto loc_8003F74C;
    }
    v1 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    s2 = 1;                                             // Result = 00000001
    if (v1 != v0) goto loc_8003F74C;
    v0 = lw(gp + 0x7B4);                                // Load from: gPSXCD_readcount (80077D94)
    v0++;
    sw(v0, gp + 0x7B4);                                 // Store to: gPSXCD_readcount (80077D94)
    v0 = 5;                                             // Result = 00000005
    if (s0 != s2) goto loc_8003F720;
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v1 = v0 << 2;
    v1 += v0;
    v1 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v1;
    a0 = lw(at);
    v0 = 3;                                             // Result = 00000003
    {
        const bool bJump = (a0 == v0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_8003F550;
    }
    if (a0 == v0) goto loc_8003F630;
    goto loc_8003F6A4;
loc_8003F550:
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB4;                                       // Result = gPSXCD_psxcd_cmd_1[2] (8007834C)
    at += v1;
    a0 = lw(at);
    v0 = a0 & 3;
    if (v0 == 0) goto loc_8003F5B8;
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6AE8;                                       // Result = gPSXCD_sectorbuf[0] (800A9518)
    a0 = s0;                                            // Result = gPSXCD_sectorbuf[0] (800A9518)
    a1 = 0x200;                                         // Result = 00000200
    LIBCD_CdGetSector();
    v1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    a1 = s0;                                            // Result = gPSXCD_sectorbuf[0] (800A9518)
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB4;                                       // Result = gPSXCD_psxcd_cmd_1[2] (8007834C)
    at += v0;
    a0 = lw(at);
    a2 = 0x800;                                         // Result = 00000800
    PSXCD_psxcd_memcpy();
    goto loc_8003F5C0;
loc_8003F5B8:
    a1 = 0x200;                                         // Result = 00000200
    LIBCD_CdGetSector();
loc_8003F5C0:
    a1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0 = a1 << 2;
    v0 += a1;
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB4;                                       // Result = gPSXCD_psxcd_cmd_1[2] (8007834C)
    at += v0;
    v1 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB8;                                       // Result = gPSXCD_psxcd_cmd_1[1] (80078348)
    at += v0;
    a0 = lw(at);
    v1 += 0x800;
    a0--;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB4;                                       // Result = gPSXCD_psxcd_cmd_1[2] (8007834C)
    at += v0;
    sw(v1, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB8;                                       // Result = gPSXCD_psxcd_cmd_1[1] (80078348)
    at += v0;
    sw(a0, at);
    v0 = a1 + 1;
    if (a0 != 0) goto loc_8003F6C8;
    sw(v0, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
    goto loc_8003F6C8;
loc_8003F630:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x6AE8;                                       // Result = gPSXCD_sectorbuf[0] (800A9518)
    a1 = 0x200;                                         // Result = 00000200
    LIBCD_CdGetSector();
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v1 = v0 << 2;
    v1 += v0;
    v1 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB4;                                       // Result = gPSXCD_psxcd_cmd_1[2] (8007834C)
    at += v1;
    a0 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB0;                                       // Result = gPSXCD_psxcd_cmd_1[3] (80078350)
    at += v1;
    a1 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB8;                                       // Result = gPSXCD_psxcd_cmd_1[1] (80078348)
    at += v1;
    a2 = lw(at);
    PSXCD_psxcd_memcpy();
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0++;
    sw(v0, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
    goto loc_8003F6C8;
loc_8003F6A4:
    v1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v0;
    sw(0, at);
loc_8003F6C8:
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v1 = v0 << 2;
    v1 += v0;
    v1 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v1;
    v0 = lw(at);
    a0 = 9;                                             // Result = 00000009
    if (v0 != 0) goto loc_8003F878;
    a1 = 0;                                             // Result = 00000000
    v0 = 1;                                             // Result = 00000001
    sw(0, gp + 0x784);                                  // Store to: gbPSXCD_async_on (80077D64)
    sw(v0, gp + 0x780);                                 // Store to: gbPSXCD_waiting_for_pause (80077D60)
    v0 = 9;                                             // Result = 00000009
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    LIBCD_CdControlF();
    goto loc_8003F878;
loc_8003F720:
    if (s0 != v0) goto loc_8003F734;
    LIBCD_CdFlush();
    sw(s2, gp + 0x7A0);                                 // Store to: gbPSXCD_critical_error (80077D80)
loc_8003F734:
    v0 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    sw(s0, gp + 0x7A8);                                 // Store to: gPSXCD_cdl_errintr (80077D88)
    sb(v0, gp + 0x7B3);                                 // Store to: gPSXCD_cdl_errcom (80077D93)
    v0 = lw(gp + 0x7AC);                                // Load from: gPSXCD_cdl_errcount (80077D8C)
    goto loc_8003F868;
loc_8003F74C:
    v0 = lbu(s1);
    v0 &= 0x80;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8003F854;
    }
    {
        const bool bJump = (s0 != v0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_8003F7DC;
    }
    v0 = lbu(s1 + 0x4);
    v0 &= 0x80;
    if (v0 != 0) goto loc_8003F878;
    v0 = lw(gp + 0x7B8);                                // Load from: gPSXCD_playcount (80077D98)
    v0++;
    sw(v0, gp + 0x7B8);                                 // Store to: gPSXCD_playcount (80077D98)
    v1 = lbu(s1 + 0x1);
    a0 = v1 >> 4;
    v0 = a0 << 2;
    v0 += a0;
    v0 <<= 1;
    v1 &= 0xF;
    v0 += v1;
    sb(v0, gp + 0x813);                                 // Store to: gPSXCD_newloc + 3 (80077DF3) (80077DF3)
    v0 = lbu(s1 + 0x3);
    sb(v0, gp + 0x810);                                 // Store to: gPSXCD_newloc (80077DF0)
    v0 = lbu(s1 + 0x4);
    sb(v0, gp + 0x811);                                 // Store to: gPSXCD_newloc + 1 (80077DF1) (80077DF1)
    v0 = lbu(s1 + 0x5);
    sb(v0, gp + 0x812);                                 // Store to: gPSXCD_newloc + 2 (80077DF2) (80077DF2)
    goto loc_8003F878;
loc_8003F7DC:
    {
        const bool bJump = (s0 != v0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_8003F838;
    }
    v1 = lw(gp + 0x7E8);                                // Load from: gbPSXCD_playflag (80077DC8)
    v0 = 1;                                             // Result = 00000001
    sw(0, gp + 0x814);                                  // Store to: gPSXCD_lastloc (80077DF4)
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    if (v1 == 0) goto loc_8003F878;
    v0 = lw(gp + 0x7F8);                                // Load from: gbPSXCD_loopflag (80077DD8)
    if (v0 == 0) goto loc_8003F878;
    a0 = lw(gp + 0x7F4);                                // Load from: gPSXCD_looptrack (80077DD4)
    a1 = lw(gp + 0x7FC);                                // Load from: gPSXCD_loopvol (80077DDC)
    a2 = lw(gp + 0x800);                                // Load from: gPSXCD_loopsectoroffset (80077DE0)
    a3 = lw(gp + 0x804);                                // Load from: gPSXCD_loopfadeuptime (80077DE4)
    sw(a0, sp + 0x10);
    sw(a1, sp + 0x14);
    sw(a2, sp + 0x18);
    sw(a3, sp + 0x1C);
    psxcd_play_at_andloop();
    goto loc_8003F878;
loc_8003F838:
    if (s0 != v0) goto loc_8003F848;
    LIBCD_CdFlush();
loc_8003F848:
    v1 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    v0 = s0 + 0x14;
    goto loc_8003F85C;
loc_8003F854:
    v1 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    v0 = s0 + 0x1E;
loc_8003F85C:
    sw(v0, gp + 0x7A8);                                 // Store to: gPSXCD_cdl_errintr (80077D88)
    v0 = lw(gp + 0x7AC);                                // Load from: gPSXCD_cdl_errcount (80077D8C)
    sb(v1, gp + 0x7B3);                                 // Store to: gPSXCD_cdl_errcom (80077D93)
loc_8003F868:
    v1 = lbu(s1);
    v0++;
    sw(v0, gp + 0x7AC);                                 // Store to: gPSXCD_cdl_errcount (80077D8C)
    sb(v1, gp + 0x7B1);                                 // Store to: gPSXCD_cdl_errstat (80077D91)
loc_8003F878:
    ra = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void psxcd_disable_callbacks() noexcept {
loc_8003F894:
    sw(0, gp + 0x79C);                                  // Store to: gbPSXCD_cb_enable_flag (80077D7C)
    return;
}

void psxcd_enable_callbacks() noexcept {
loc_8003F8A0:
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x79C);                                 // Store to: gbPSXCD_cb_enable_flag (80077D7C)
    return;
}

void psxcd_init() noexcept {
loc_8003F8B0:
    v0 = lw(gp + 0x790);                                // Load from: gbPSXCD_IsCdInit (80077D70)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 != 0) goto loc_8003F97C;
    LIBCD_CdInit();
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x790);                                 // Store to: gbPSXCD_IsCdInit (80077D70)
    v1 = 0;                                             // Result = 00000000
loc_8003F8D4:
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7C08;                                       // Result = gCdlLOCArray[0] (800783F8)
    at += v1;
    sb(0, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7C07;                                       // Result = gCdlLOCArray[0] (800783F9)
    at += v1;
    sb(0, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7C06;                                       // Result = gCdlLOCArray[0] (800783FA)
    at += v1;
    sb(0, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7C05;                                       // Result = gCdlLOCArray[0] (800783FB)
    at += v1;
    sb(0, at);
    v1 += 4;
    v0 = (i32(v1) < 0x190);
    if (v0 != 0) goto loc_8003F8D4;
    psxspu_init();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 -= 0x7C08;                                       // Result = gCdlLOCArray[0] (800783F8)
    LIBCD_CdGetToc();
    if (v0 == 0) goto loc_8003F97C;
    a0 = 0x80040000;                                    // Result = 80040000
    a0 -= 0xC54;                                        // Result = PSXCD_cbcomplete (8003F3AC)
    sw(0, gp + 0x794);                                  // Store to: gPSXCD_init_pos (80077D74)
    sw(0, gp + 0x784);                                  // Store to: gbPSXCD_async_on (80077D64)
    LIBCD_CdSyncCallback();
    a0 = 0x80040000;                                    // Result = 80040000
    a0 -= 0xB70;                                        // Result = PSXCD_cbready (8003F490)
    sw(v0, gp + 0x850);                                 // Store to: gPSXCD_cbsyncsave (80077E30)
    LIBCD_CdReadyCallback();
    sw(v0, gp + 0x854);                                 // Store to: gPSXCD_cbreadysave (80077E34)
    psxcd_enable_callbacks();
loc_8003F97C:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_exit() noexcept {
    a0 = lw(gp + 0x850);                                // Load from: gPSXCD_cbsyncsave (80077E30)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBCD_CdSyncCallback();
    a0 = lw(gp + 0x854);                                // Load from: gPSXCD_cbreadysave (80077E34)
    LIBCD_CdReadyCallback();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_set_data_mode() noexcept {
loc_8003F9BC:
    sp -= 0x18;
    v1 = lw(gp + 0x78C);                                // Load from: gPSXCD_psxcd_mode (80077D6C)
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x10);
    if (v1 == v0) goto loc_8003FA60;
    sw(0, gp + 0x7E8);                                  // Store to: gbPSXCD_playflag (80077DC8)
    sw(0, gp + 0x7F8);                                  // Store to: gbPSXCD_loopflag (80077DD8)
    sw(0, gp + 0x77C);                                  // Store to: gbPSXCD_seeking_for_play (80077D5C)
    psxspu_get_cd_vol();
    a0 = 0xFA;                                          // Result = 000000FA
    if (v0 == 0) goto loc_8003FA04;
    a1 = 0;                                             // Result = 00000000
    psxspu_start_cd_fade();
loc_8003F9F4:
    psxspu_get_cd_fade_status();
    if (v0 != 0) goto loc_8003F9F4;
loc_8003FA04:
    psxspu_setcdmixoff();
    psxcd_sync();
    a0 = 0xE;                                           // Result = 0000000E
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7D9C;                                       // Result = gPSXCD_cd_param[0] (80077D9C)
    v0 = 0x80;                                          // Result = 00000080
    sb(v0, gp + 0x7BC);                                 // Store to: gPSXCD_cd_param[0] (80077D9C)
    v0 = 0xE;                                           // Result = 0000000E
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a2 = 0;                                             // Result = 00000000
    LIBCD_CdControl();
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x78C);                                 // Store to: gPSXCD_psxcd_mode (80077D6C)
    psxcd_sync();
    a0 = 9;                                             // Result = 00000009
    a1 = 0;                                             // Result = 00000000
    v0 = 9;                                             // Result = 00000009
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a2 = 0;                                             // Result = 00000000
    goto loc_8003FAA4;
loc_8003FA60:
    v0 = lw(gp + 0x784);                                // Load from: gbPSXCD_async_on (80077D64)
    if (v0 == 0) goto loc_8003FA78;
    psxcd_async_read_cancel();
loc_8003FA78:
    v1 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    v0 = 9;                                             // Result = 00000009
    if (v1 == v0) goto loc_8003FAAC;
    psxcd_sync();
    v0 = 9;                                             // Result = 00000009
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a0 = 9;                                             // Result = 00000009
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
loc_8003FAA4:
    LIBCD_CdControl();
loc_8003FAAC:
    psxcd_sync();
    LIBCD_CdFlush();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open a specified CD file for reading
//------------------------------------------------------------------------------------------------------------------------------------------
PsxCd_File* psxcd_open(const CdMapTbl_File discFile) noexcept {
    // Sanity check the file is valid!
    #if PC_PSX_DOOM_MODS
        if (discFile >= CdMapTbl_File::END) {
            FATAL_ERROR("psxcd_open: invalid file specified!");
        }
    #endif

    // Figure out where the file is on disc and save it's size
    const PsxCd_MapTblEntry& fileTableEntry = CD_MAP_TBL[(uint32_t) discFile];

    a0 = fileTableEntry.startSector;
    a1 = ptrToVmAddr(&gPSXCD_cdfile->file.pos);
    LIBCD_CdIntToPos();

    gPSXCD_cdfile->file.size = fileTableEntry.size;;

    // PC-PSX: 'name' was not initialized in the original PSX code as filenames were not used in the retail version of the game.
    // Initialize here for good measure:
    #if PC_PSX_DOOM_MODS
        for (char& c : gPSXCD_cdfile->file.name) {
            c = 0;
        }
    #endif

    // Initialize file IO position and status
    gPSXCD_cdfile->new_io_loc = gPSXCD_cdfile->file.pos;
    gPSXCD_cdfile->io_block_offset = 0;

    for (uint8_t& statusByte : gPSXCD_cdfile->io_result) {
        statusByte = 0;
    }

    return gPSXCD_cdfile.get();
}

void _thunk_psxcd_open() noexcept {
    v0 = ptrToVmAddr(psxcd_open((CdMapTbl_File) a0));
}

void psxcd_init_pos() noexcept {
loc_8003FB9C:
    sw(0, gp + 0x794);                                  // Store to: gPSXCD_init_pos (80077D74)
    sw(0, gp + 0x7E8);                                  // Store to: gbPSXCD_playflag (80077DC8)
    sw(0, gp + 0x7F8);                                  // Store to: gbPSXCD_loopflag (80077DD8)
    sw(0, gp + 0x77C);                                  // Store to: gbPSXCD_seeking_for_play (80077D5C)
    sw(0, gp + 0x780);                                  // Store to: gbPSXCD_waiting_for_pause (80077D60)
    sw(0, gp + 0x7A0);                                  // Store to: gbPSXCD_critical_error (80077D80)
    return;
}

void psxcd_async_on() noexcept {
loc_8003FBBC:
    v0 = lw(gp + 0x784);                                // Load from: gbPSXCD_async_on (80077D64)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_8003FCB0;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DB4;                                       // Result = gPSXCD_check_result[0] (80077DB4)
    a0 = 1;                                             // Result = 00000001
    LIBCD_CdSync();
    v1 = lw(gp + 0x7A0);                                // Load from: gbPSXCD_critical_error (80077D80)
    a0 = v0;
    sw(a0, gp + 0x7D0);                                 // Store to: gPSXCD_check_intr (80077DB0)
    v0 = 5;                                             // Result = 00000005
    if (v1 != 0) goto loc_8003FC0C;
    if (a0 == v0) goto loc_8003FC0C;
    v0 = lbu(gp + 0x7D4);                               // Load from: gPSXCD_check_result[0] (80077DB4)
    v0 &= 2;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8003FCB4;
    }
loc_8003FC0C:
    LIBCD_CdFlush();
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0x7C58;                                       // Result = gPSXCD_newfilestruct[0] (800783A8)
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7C30;                                       // Result = gPSXCD_lastfilestruct[0] (800783D0)
    v0 = lw(gp + 0x7D0);                                // Load from: gPSXCD_check_intr (80077DB0)
    a0 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    a1 = lbu(gp + 0x7D4);                               // Load from: gPSXCD_check_result[0] (80077DB4)
    v1 = lw(gp + 0x7AC);                                // Load from: gPSXCD_cdl_errcount (80077D8C)
    t0 = a2 + 0x20;                                     // Result = gPSXCD_lastfilestruct[8] (800783F0)
    sw(0, gp + 0x7A0);                                  // Store to: gbPSXCD_critical_error (80077D80)
    v0 += 0x64;
    v1++;
    sw(v0, gp + 0x7A8);                                 // Store to: gPSXCD_cdl_errintr (80077D88)
    sb(a0, gp + 0x7B3);                                 // Store to: gPSXCD_cdl_errcom (80077D93)
    sb(a1, gp + 0x7B1);                                 // Store to: gPSXCD_cdl_errstat (80077D91)
    sw(v1, gp + 0x7AC);                                 // Store to: gPSXCD_cdl_errcount (80077D8C)
loc_8003FC54:
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
    if (a2 != t0) goto loc_8003FC54;
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    a0 = lw(gp + 0x7E0);                                // Load from: gpPSXCD_lastdestptr (80077DC0)
    a1 = lw(gp + 0x7E4);                                // Load from: gPSXCD_lastreadbytes (80077DC4)
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7C58;                                       // Result = gPSXCD_newfilestruct[0] (800783A8)
    psxcd_async_read();
    v0 = 1;                                             // Result = 00000001
    goto loc_8003FCB4;
loc_8003FCB0:
    v0 = 0;                                             // Result = 00000000
loc_8003FCB4:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_seeking_for_play() noexcept {
loc_8003FCC4:
    v0 = lw(gp + 0x77C);                                // Load from: gbPSXCD_seeking_for_play (80077D5C)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_8003FD60;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DB4;                                       // Result = gPSXCD_check_result[0] (80077DB4)
    a0 = 1;                                             // Result = 00000001
    LIBCD_CdSync();
    sw(v0, gp + 0x7D0);                                 // Store to: gPSXCD_check_intr (80077DB0)
    v1 = 5;                                             // Result = 00000005
    if (v0 == v1) goto loc_8003FD08;
    v0 = lbu(gp + 0x7D4);                               // Load from: gPSXCD_check_result[0] (80077DB4)
    v0 &= 2;
    if (v0 != 0) goto loc_8003FD58;
loc_8003FD08:
    LIBCD_CdFlush();
    v0 = lw(gp + 0x7D0);                                // Load from: gPSXCD_check_intr (80077DB0)
    a0 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    a1 = lbu(gp + 0x7D4);                               // Load from: gPSXCD_check_result[0] (80077DB4)
    v1 = lw(gp + 0x7AC);                                // Load from: gPSXCD_cdl_errcount (80077D8C)
    v0 += 0x6E;
    v1++;
    sw(v0, gp + 0x7A8);                                 // Store to: gPSXCD_cdl_errintr (80077D88)
    sb(a0, gp + 0x7B3);                                 // Store to: gPSXCD_cdl_errcom (80077D93)
    sb(a1, gp + 0x7B1);                                 // Store to: gPSXCD_cdl_errstat (80077D91)
    sw(v1, gp + 0x7AC);                                 // Store to: gPSXCD_cdl_errcount (80077D8C)
    psxcd_sync();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    v0 = 0x16;                                          // Result = 00000016
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a0 = 0x16;                                          // Result = 00000016
    LIBCD_CdControlF();
loc_8003FD58:
    v0 = 1;                                             // Result = 00000001
    goto loc_8003FD64;
loc_8003FD60:
    v0 = 0;                                             // Result = 00000000
loc_8003FD64:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_waiting_for_pause() noexcept {
    v0 = lw(gp + 0x780);                                // Load from: gbPSXCD_waiting_for_pause (80077D60)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_8003FE0C;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DB4;                                       // Result = gPSXCD_check_result[0] (80077DB4)
    a0 = 1;                                             // Result = 00000001
    LIBCD_CdSync();
    sw(v0, gp + 0x7D0);                                 // Store to: gPSXCD_check_intr (80077DB0)
    v1 = 5;                                             // Result = 00000005
    if (v0 == v1) goto loc_8003FDB8;
    v0 = lbu(gp + 0x7D4);                               // Load from: gPSXCD_check_result[0] (80077DB4)
    v0 &= 2;
    if (v0 != 0) goto loc_8003FE04;
loc_8003FDB8:
    LIBCD_CdFlush();
    v0 = lw(gp + 0x7D0);                                // Load from: gPSXCD_check_intr (80077DB0)
    a0 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    a1 = lbu(gp + 0x7D4);                               // Load from: gPSXCD_check_result[0] (80077DB4)
    v1 = lw(gp + 0x7AC);                                // Load from: gPSXCD_cdl_errcount (80077D8C)
    v0 += 0x78;
    v1++;
    sw(v0, gp + 0x7A8);                                 // Store to: gPSXCD_cdl_errintr (80077D88)
    sb(a0, gp + 0x7B3);                                 // Store to: gPSXCD_cdl_errcom (80077D93)
    sb(a1, gp + 0x7B1);                                 // Store to: gPSXCD_cdl_errstat (80077D91)
    sw(v1, gp + 0x7AC);                                 // Store to: gPSXCD_cdl_errcount (80077D8C)
    psxcd_sync();
    a0 = 9;                                             // Result = 00000009
    v0 = 9;                                             // Result = 00000009
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a1 = 0;                                             // Result = 00000000
    LIBCD_CdControlF();
loc_8003FE04:
    v0 = 1;                                             // Result = 00000001
    goto loc_8003FE10;
loc_8003FE0C:
    v0 = 0;                                             // Result = 00000000
loc_8003FE10:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the specified number of bytes synchronously from the given CD file.
// Returns the number of bytes read.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_read(void* const pDest, int32_t numBytes, PsxCd_File& file) noexcept {
    // Kick off the async read.
    // Note: number of bytes read will not match request if there was an error!    
    a0 = ptrToVmAddr(pDest);
    a1 = numBytes;
    a2 = ptrToVmAddr(&file);
    psxcd_async_read();
    const int32_t retBytesRead = v0;

    // Continue reading until done
    bool bIsDoingAsyncRead = true;

    do {
        // Need to emulate the cdrom so the read actually happens!
        // Also move along sound emulation if required.
        #if PC_PSX_DOOM_MODS
            emulate_cdrom();
            emulate_sound_if_required();
        #endif

        psxcd_async_on();
        bIsDoingAsyncRead = (v0 != 0);
    } while (bIsDoingAsyncRead);
    
    return retBytesRead;
}

void _thunk_psxcd_read() noexcept {
    v0 = psxcd_read(vmAddrToPtr<void>(a0), a1, *vmAddrToPtr<PsxCd_File>(a2));
}

void psxcd_async_read_cancel() noexcept {
loc_8003FE58:
    v0 = lw(gp + 0x784);                                // Load from: gbPSXCD_async_on (80077D64)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_8003FE94;
    sw(0, gp + 0x784);                                  // Store to: gbPSXCD_async_on (80077D64)
    sw(0, gp + 0x794);                                  // Store to: gPSXCD_init_pos (80077D74)
    psxcd_sync();
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x780);                                 // Store to: gbPSXCD_waiting_for_pause (80077D60)
    v0 = 9;                                             // Result = 00000009
    a0 = 9;                                             // Result = 00000009
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a1 = 0;                                             // Result = 00000000
    LIBCD_CdControlF();
loc_8003FE94:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_async_read() noexcept {
loc_8003FEA4:
    sp -= 0x40;
    sw(s5, sp + 0x2C);
    s5 = a1;
    sw(s1, sp + 0x1C);
    s1 = a2;
    sw(ra, sp + 0x3C);
    sw(fp, sp + 0x38);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s0, sp + 0x18);
    sw(a0, sp + 0x10);
    if (s5 == 0) goto loc_8004063C;
    v0 = lw(s1);
    fp = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8004063C;
    s7 = 0x80080000;                                    // Result = 80080000
    s7 -= 0x7CAC;                                       // Result = gPSXCD_psxcd_cmd_1[4] (80078354)
loc_8003FEF8:
    s6 = 0;                                             // Result = 00000000
    psxcd_set_data_mode();
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0x7C30;                                       // Result = gPSXCD_lastfilestruct[0] (800783D0)
    a2 = s1;
    t1 = lw(sp + 0x10);
    t0 = s1 + 0x20;
    sw(s5, gp + 0x7E4);                                 // Store to: gPSXCD_lastreadbytes (80077DC4)
    sw(t1, gp + 0x7E0);                                 // Store to: gpPSXCD_lastdestptr (80077DC0)
loc_8003FF1C:
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
    if (a2 != t0) goto loc_8003FF1C;
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    s4 = lw(sp + 0x10);
    t0 = lw(s1 + 0x1C);
    sw(0, gp + 0x7DC);                                  // Store to: gPSXCD_cur_cmd (80077DBC)
    s2 = s5;
    if (t0 == 0) goto loc_80040174;
    v0 = (s2 < 0x800);
    if (s5 == 0) goto loc_80040178;
    v0 = lw(gp + 0x794);                                // Load from: gPSXCD_init_pos (80077D74)
    if (v0 == 0) goto loc_8003FF98;
    v1 = *gPSXCD_cur_io_loc;
    v0 = lw(s1 + 0x18);
    {
        const bool bJump = (v1 == v0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_8003FFF8;
    }
loc_8003FF98:
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    t1 = 2;                                             // Result = 00000002
    at = 0x80080000;                                    // Result = 80080000
    sw(t1, at - 0x7CBC);                                // Store to: gPSXCD_psxcd_cmd_1[0] (80078344)
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7CAC;                                       // Result = gPSXCD_psxcd_cmd_1[4] (80078354)
    v1 = lwl(v1, s1 + 0x1B);
    v1 = lwr(v1, s1 + 0x18);
    swl(v1, a2 + 0x3);                                  // Store to: gPSXCD_psxcd_cmd_1[4] (80078357)
    swr(v1, a2);                                        // Store to: gPSXCD_psxcd_cmd_1[4] (80078354)
    v0++;
    sw(v0, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
    a1 = gPSXCD_cur_io_loc;
    v0 = lwl(v0, s1 + 0x1B);
    v0 = lwr(v0, s1 + 0x18);    
    *gPSXCD_cur_io_loc = v0;
    sw(fp, gp + 0x794);                                 // Store to: gPSXCD_init_pos (80077D74)
    v0 = 0x800;                                         // Result = 00000800
loc_8003FFF8:
    a3 = v0 - t0;
    v0 = (s2 < a3);
    if (v0 == 0) goto loc_8004000C;
    a3 = s2;
loc_8004000C:
    a0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v1 = a0 << 2;
    if (a0 != 0) goto loc_80040030;
    v1 = lw(gp + 0x788);                                // Load from: gPSXCD_sectorbuf_contents (80077D68)
    v0 = *gPSXCD_cur_io_loc;
    {
        const bool bJump = (v1 == v0);
        v1 = a0 << 2;
        if (bJump) goto loc_800400E8;
    }
loc_80040030:
    v1 += a0;
    v1 <<= 2;
    v0 = 4;                                             // Result = 00000004
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v1;
    sw(v0, at);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x6AE8;                                       // Result = gPSXCD_sectorbuf[0] (800A9518)
    v0 += t0;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB8;                                       // Result = gPSXCD_psxcd_cmd_1[1] (80078348)
    at += v1;
    sw(a3, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB4;                                       // Result = gPSXCD_psxcd_cmd_1[2] (8007834C)
    at += v1;
    sw(s4, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB0;                                       // Result = gPSXCD_psxcd_cmd_1[3] (80078350)
    at += v1;
    sw(v0, at);
    v1 += s7;
    a2 = gPSXCD_cur_io_loc;
    v0 = *gPSXCD_cur_io_loc;
    swl(v0, v1 + 0x3);
    swr(v0, v1);
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    a2 = gPSXCD_cur_io_loc;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7D68;                                       // Result = gPSXCD_sectorbuf_contents (80077D68)
    v1 = *gPSXCD_cur_io_loc;
    swl(v1, a1 + 0x3);                                  // Store to: gPSXCD_sectorbuf_contents + 3 (80077D6B) (80077D6B)
    swr(v1, a1);                                        // Store to: gPSXCD_sectorbuf_contents (80077D68)
    v0++;
    sw(v0, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
    s2 -= a3;
    goto loc_8004011C;
loc_800400E8:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x6AE8;                                       // Result = gPSXCD_sectorbuf[0] (800A9518)
    v0 += t0;
    at = 0x80080000;                                    // Result = 80080000
    sw(fp, at - 0x7CBC);                                // Store to: gPSXCD_psxcd_cmd_1[0] (80078344)
    at = 0x80080000;                                    // Result = 80080000
    sw(a3, at - 0x7CB8);                                // Store to: gPSXCD_psxcd_cmd_1[1] (80078348)
    at = 0x80080000;                                    // Result = 80080000
    sw(s4, at - 0x7CB4);                                // Store to: gPSXCD_psxcd_cmd_1[2] (8007834C)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7CB0);                                // Store to: gPSXCD_psxcd_cmd_1[3] (80078350)
    sw(fp, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
    s2 -= a3;
loc_8004011C:
    s4 += a3;
    v1 = t0 + a3;
    v0 = 0x800;                                         // Result = 00000800
    sw(v1, s1 + 0x1C);
    if (v1 != v0) goto loc_80040174;
    a0 = gPSXCD_cur_io_loc;
    LIBCD_CdPosToInt();
    a1 = gPSXCD_cur_io_loc;
    a0 = v0 + 1;
    LIBCD_CdIntToPos();
    a1 = gPSXCD_cur_io_loc;
    v0 = *gPSXCD_cur_io_loc;
    swl(v0, s1 + 0x1B);
    swr(v0, s1 + 0x18);
    sw(0, s1 + 0x1C);
loc_80040174:
    v0 = (s2 < 0x800);
loc_80040178:
    s3 = s2 >> 11;
    if (v0 != 0) goto loc_8004031C;
    v0 = lw(gp + 0x794);                                // Load from: gPSXCD_init_pos (80077D74)
    s0 = s3 << 11;
    if (v0 == 0) goto loc_800401A4;
    v1 = *gPSXCD_cur_io_loc;
    v0 = lw(s1 + 0x18);
    if (v1 == v0) goto loc_8004021C;
loc_800401A4:
    v1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    t1 = 2;                                             // Result = 00000002
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v0;
    sw(t1, at);
    v0 += s7;
    v1 = lwl(v1, s1 + 0x1B);
    v1 = lwr(v1, s1 + 0x18);
    swl(v1, v0 + 0x3);
    swr(v1, v0);
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0++;
    sw(v0, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
    a1 = gPSXCD_cur_io_loc;
    v0 = lwl(v0, s1 + 0x1B);
    v0 = lwr(v0, s1 + 0x18);
    *gPSXCD_cur_io_loc = v0;
    sw(fp, gp + 0x794);                                 // Store to: gPSXCD_init_pos (80077D74)
loc_8004021C:
    v1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v1 = 3;                                             // Result = 00000003
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v0;
    sw(v1, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB8;                                       // Result = gPSXCD_psxcd_cmd_1[1] (80078348)
    at += v0;
    sw(s3, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB4;                                       // Result = gPSXCD_psxcd_cmd_1[2] (8007834C)
    at += v0;
    sw(s4, at);
    v0 = s4 & 3;
    s4 += s0;
    if (v0 == 0) goto loc_80040294;
    a0 = gPSXCD_cur_io_loc;
    LIBCD_CdPosToInt();
    v0 += s3;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7D68;                                       // Result = gPSXCD_sectorbuf_contents (80077D68)
    a0 = v0 - 1;
    LIBCD_CdIntToPos();
loc_80040294:
    s2 -= s0;
    v1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    s0 = gPSXCD_cur_io_loc;
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 += s7;
    a2 = gPSXCD_cur_io_loc;
    v1 = *gPSXCD_cur_io_loc;
    swl(v1, v0 + 0x3);
    swr(v1, v0);
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0++;
    sw(v0, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
    a0 = s0;                                            // Result = gPSXCD_cur_io_loc (80077D78)
    LIBCD_CdPosToInt();
    a0 = v0 + s3;
    a1 = s0;                                            // Result = gPSXCD_cur_io_loc (80077D78)
    LIBCD_CdIntToPos();
    sw(0, s1 + 0x1C);
    a1 = gPSXCD_cur_io_loc;
    v0 = *gPSXCD_cur_io_loc;
    swl(v0, s1 + 0x1B);
    swr(v0, s1 + 0x18);
loc_8004031C:
    if (s2 == 0) goto loc_8004047C;
    v0 = lw(gp + 0x794);                                // Load from: gPSXCD_init_pos (80077D74)
    if (v0 == 0) goto loc_80040348;
    v1 = *gPSXCD_cur_io_loc;
    v0 = lw(s1 + 0x18);
    if (v1 == v0) goto loc_800403C0;
loc_80040348:
    v1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    t1 = 2;                                             // Result = 00000002
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v0;
    sw(t1, at);
    v0 += s7;
    v1 = lwl(v1, s1 + 0x1B);
    v1 = lwr(v1, s1 + 0x18);
    swl(v1, v0 + 0x3);
    swr(v1, v0);
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0++;
    sw(v0, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
    a1 = gPSXCD_cur_io_loc;
    v0 = lwl(v0, s1 + 0x1B);
    v0 = lwr(v0, s1 + 0x18);
    *gPSXCD_cur_io_loc = v0;
    sw(fp, gp + 0x794);                                 // Store to: gPSXCD_init_pos (80077D74)
loc_800403C0:
    v1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v1 = 4;                                             // Result = 00000004
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v0;
    sw(v1, at);
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x6AE8;                                       // Result = gPSXCD_sectorbuf[0] (800A9518)
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB8;                                       // Result = gPSXCD_psxcd_cmd_1[1] (80078348)
    at += v0;
    sw(s2, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB4;                                       // Result = gPSXCD_psxcd_cmd_1[2] (8007834C)
    at += v0;
    sw(s4, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CB0;                                       // Result = gPSXCD_psxcd_cmd_1[3] (80078350)
    at += v0;
    sw(v1, at);
    v0 += s7;
    a2 = gPSXCD_cur_io_loc;
    v1 = *gPSXCD_cur_io_loc;
    swl(v1, v0 + 0x3);
    swr(v1, v0);
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    a2 = gPSXCD_cur_io_loc;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7D68;                                       // Result = gPSXCD_sectorbuf_contents (80077D68)
    v1 = *gPSXCD_cur_io_loc;
    swl(v1, a1 + 0x3);                                  // Store to: gPSXCD_sectorbuf_contents + 3 (80077D6B) (80077D6B)
    swr(v1, a1);                                        // Store to: gPSXCD_sectorbuf_contents (80077D68)
    sw(s2, s1 + 0x1C);
    v0++;
    sw(v0, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
loc_8004047C:
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v1 = v0 << 2;
    v1 += v0;
    v1 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v1;
    sw(0, at);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7CBC);                               // Load from: gPSXCD_psxcd_cmd_1[0] (80078344)
    sw(0, gp + 0x7DC);                                  // Store to: gPSXCD_cur_cmd (80077DBC)
    if (v0 != fp) goto loc_80040508;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7CB4);                               // Load from: gPSXCD_psxcd_cmd_1[2] (8007834C)
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7CB0);                               // Load from: gPSXCD_psxcd_cmd_1[3] (80078350)
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0x7CB8);                               // Load from: gPSXCD_psxcd_cmd_1[1] (80078348)
    PSXCD_psxcd_memcpy();
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0++;
    v1 = v0 << 2;
    v1 += v0;
    v1 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v1;
    v1 = lw(at);
    sw(v0, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
    v0 = s5;
    if (v1 == 0) goto loc_800406A0;
loc_80040508:
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v1 = v0 << 2;
    v1 += v0;
    v1 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v1;
    v0 = lw(at);
    t1 = 2;                                             // Result = 00000002
    if (v0 != t1) goto loc_8004059C;
    psxcd_sync();
    a0 = 2;                                             // Result = 00000002
    a2 = 0;                                             // Result = 00000000
    v0 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    t1 = 2;                                             // Result = 00000002
    sb(t1, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a1 = v0 << 2;
    a1 += v0;
    a1 <<= 2;
    a1 += s7;
    LIBCD_CdControl();
    v1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v1++;
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v0;
    v0 = lw(at);
    sw(v1, gp + 0x7DC);                                 // Store to: gPSXCD_cur_cmd (80077DBC)
    {
        const bool bJump = (v0 == 0);
        v0 = s5;
        if (bJump) goto loc_800406A0;
    }
loc_8004059C:
    v1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7CBC;                                       // Result = gPSXCD_psxcd_cmd_1[0] (80078344)
    at += v0;
    v1 = lw(at);
    if (v1 == 0) goto loc_8004069C;
    v0 = (i32(v1) < 5);
    if (i32(v1) < 0) goto loc_8004063C;
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 3);
        if (bJump) goto loc_8004063C;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800406A0;
    }
    psxcd_critical_sync();
    a0 = 6;                                             // Result = 00000006
    if (v0 == 0) goto loc_80040628;
    a2 = 0;                                             // Result = 00000000
    v1 = lw(gp + 0x7DC);                                // Load from: gPSXCD_cur_cmd (80077DBC)
    v0 = 6;                                             // Result = 00000006
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a1 = v1 << 2;
    a1 += v1;
    a1 <<= 2;
    a1 += s7;
    LIBCD_CdControl();
    psxcd_critical_sync();
    if (v0 != 0) goto loc_80040630;
loc_80040628:
    s6 = 1;                                             // Result = 00000001
    goto loc_80040644;
loc_80040630:
    sw(fp, gp + 0x784);                                 // Store to: gbPSXCD_async_on (80077D64)
    goto loc_80040644;
loc_8004063C:
    v0 = 0;                                             // Result = 00000000
    goto loc_800406A0;
loc_80040644:
    v0 = s5;
    if (s6 == 0) goto loc_800406A0;
    a3 = s1;
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7C30;                                       // Result = gPSXCD_lastfilestruct[0] (800783D0)
    t0 = a2 + 0x20;                                     // Result = gPSXCD_lastfilestruct[8] (800783F0)
loc_8004065C:
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
    if (a2 != t0) goto loc_8004065C;
    v0 = lw(a2);
    v1 = lw(a2 + 0x4);
    sw(v0, a3);
    sw(v1, a3 + 0x4);
    if (s6 != 0) goto loc_8003FEF8;
loc_8004069C:
    v0 = s5;
loc_800406A0:
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek to a specified position in a file, relatively or absolutely.
// Returns '0' on success, any other value on failure.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_seek(PsxCd_File& file, int32_t offset, const PsxCd_SeekMode mode) noexcept {
    // Is this an actual valid file? If not then just NOP the call and return '0' for 'success':
    if (file.file.pos == CdlLOC{ 0, 0, 0, 0 })
        return 0;

    if (mode == PsxCd_SeekMode::SET) {
        // Seek to an absolute position in the file: first get the start sector for the file
        a0 = ptrToVmAddr(&file.file.pos);
        LIBCD_CdPosToInt();
        const int32_t fileStartSec = v0;

        // Figure out the disc sector to go to for the requested location within the file
        const int32_t sectorInFile = ((offset < 0) ? offset + (CD_SECTOR_SIZE - 1) : offset) / CD_SECTOR_SIZE;
        const int32_t newDiscSector = fileStartSec + sectorInFile;

        a0 = newDiscSector;
        a1 = ptrToVmAddr(&file.new_io_loc);
        LIBCD_CdIntToPos();

        // Figure out the offset within the destination sector we want to go to
        file.io_block_offset = (uint32_t)(offset - sectorInFile * CD_SECTOR_SIZE);
    } 
    else if (mode == PsxCd_SeekMode::CUR) {
        // Seek relative to the current IO position: first get the current IO sector
        a0 = gPSXCD_cur_io_loc;
        LIBCD_CdPosToInt();
        const int32_t curIoSec = v0;

        // Figure out the disc sector to go to for the requested relative offset
        const int32_t secOffset = (file.io_block_offset + offset) / CD_SECTOR_SIZE;
        const int32_t newDiscSector = secOffset + curIoSec;

        a0 = newDiscSector;
        a1 = ptrToVmAddr(&file.new_io_loc);
        LIBCD_CdIntToPos();
        
        // Figure out the offset within the destination sector we want to go to
        file.io_block_offset = ((uint32_t)(file.io_block_offset + offset)) % CD_SECTOR_SIZE;
    }
    else {
        // Seek relative to the end: first get the start sector for the file
        a0 = ptrToVmAddr(&file.file.pos);
        LIBCD_CdPosToInt();
        const int32_t fileStartSec = v0;

        // Figure out the disc sector to go to for the requested location within the file
        const int32_t sectorInFile = (file.file.size - offset) / CD_SECTOR_SIZE;
        const int32_t newDiscSector = fileStartSec + sectorInFile;

        a0 = newDiscSector;
        a1 = ptrToVmAddr(&file.new_io_loc);
        LIBCD_CdIntToPos();

        // Figure out the offset within the destination sector we want to go to
        file.io_block_offset = ((uint32_t)(file.file.size - offset) % CD_SECTOR_SIZE);
    }

    // Return '0' for success - this never actually fails for the retail game.
    // All we are doing here really is modifying the file datastructure.
    return 0;
}

void _thunk_psxcd_seek() noexcept {
    v0 = psxcd_seek(*vmAddrToPtr<PsxCd_File>(a0), a1, (PsxCd_SeekMode) a2);
}

void psxcd_tell() noexcept {
loc_800407C8:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1);
    if (v0 == 0) goto loc_80040814;
    a0 = s1 + 0x18;
    LIBCD_CdPosToInt();
    a0 = s1;
    s0 = v0;
    LIBCD_CdPosToInt();
    s0 -= v0;
    v0 = lw(s1 + 0x1C);
    s0 <<= 11;
    v0 += s0;
    goto loc_80040818;
loc_80040814:
    v0 = 0;                                             // Result = 00000000
loc_80040818:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Placeholder for 'closing' a cd file - didn't need to do anything for retail PSX DOOM.
// In the retail .exe an open cd file is simply a struct describing the current IO location, filename etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_close([[maybe_unused]] PsxCd_File& file) noexcept {
    // Nothing to do...
}

void _thunk_psxcd_close() noexcept {
    psxcd_close(*vmAddrToPtr<PsxCd_File>(a0));
}

void psxcd_set_audio_mode() noexcept {
loc_80040838:
    v0 = lw(gp + 0x78C);                                // Load from: gPSXCD_psxcd_mode (80077D6C)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_800408C4;
    v0 = lw(gp + 0x784);                                // Load from: gbPSXCD_async_on (80077D64)
    a0 = 0xE;                                           // Result = 0000000E
    if (v0 == 0) goto loc_80040864;
    psxcd_async_read_cancel();
    a0 = 0xE;                                           // Result = 0000000E
loc_80040864:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7D9C;                                       // Result = gPSXCD_cd_param[0] (80077D9C)
    v0 = 7;                                             // Result = 00000007
    sb(v0, gp + 0x7BC);                                 // Store to: gPSXCD_cd_param[0] (80077D9C)
    v0 = 0xE;                                           // Result = 0000000E
    sw(0, gp + 0x794);                                  // Store to: gPSXCD_init_pos (80077D74)
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a2 = 0;                                             // Result = 00000000
    LIBCD_CdControl();
    sw(0, gp + 0x78C);                                  // Store to: gPSXCD_psxcd_mode (80077D6C)
    psxcd_sync();
    a0 = 9;                                             // Result = 00000009
    a1 = 0;                                             // Result = 00000000
    v0 = 9;                                             // Result = 00000009
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a2 = 0;                                             // Result = 00000000
    LIBCD_CdControl();
    psxcd_sync();
    LIBCD_CdFlush();
    goto loc_800408CC;
loc_800408C4:
    psxcd_sync();
loc_800408CC:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_set_loop_volume() noexcept {
    sw(a0, gp + 0x7FC);                                 // Store to: gPSXCD_loopvol (80077DDC)
    return;
}

void psxcd_play_at_andloop() noexcept {
loc_800408E8:
    sp -= 0x38;
    sw(s5, sp + 0x24);
    s5 = lw(sp + 0x48);
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s2, sp + 0x18);
    s2 = a2;
    sw(s3, sp + 0x1C);
    sw(s6, sp + 0x28);
    s6 = lw(sp + 0x4C);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0x7C08;                                       // Result = gCdlLOCArray[0] (800783F8)
    sw(s7, sp + 0x2C);
    s7 = lw(sp + 0x50);
    a0 <<= 2;
    sw(s0, sp + 0x10);
    s0 = a0 + v0;
    sw(ra, sp + 0x30);
    sw(s4, sp + 0x20);
    v0 = lw(s0);
    s4 = lw(sp + 0x54);
    s3 = a3;
    if (v0 == 0) goto loc_80040A18;
    psxcd_set_audio_mode();
    sw(s1, gp + 0x7EC);                                 // Store to: gPSXCD_playvol (80077DCC)
    a0 = s0;
    LIBCD_CdPosToInt();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DE8;                                       // Result = gPSXCD_cdloc (80077DE8)
    a0 = v0 + s2;
    LIBCD_CdIntToPos();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DE8;                                       // Result = gPSXCD_cdloc (80077DE8)
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x7E8);                                 // Store to: gbPSXCD_playflag (80077DC8)
    sw(v0, gp + 0x7F8);                                 // Store to: gbPSXCD_loopflag (80077DD8)
    sw(v0, gp + 0x77C);                                 // Store to: gbPSXCD_seeking_for_play (80077D5C)
    v0 = 0x16;                                          // Result = 00000016
    sw(0, gp + 0x7B8);                                  // Store to: gPSXCD_playcount (80077D98)
    sw(s5, gp + 0x7F4);                                 // Store to: gPSXCD_looptrack (80077DD4)
    sw(s6, gp + 0x7FC);                                 // Store to: gPSXCD_loopvol (80077DDC)
    sw(s7, gp + 0x800);                                 // Store to: gPSXCD_loopsectoroffset (80077DE0)
    sw(s4, gp + 0x804);                                 // Store to: gPSXCD_loopfadeuptime (80077DE4)
    sw(s3, gp + 0x7F0);                                 // Store to: gPSXCD_playfadeuptime (80077DD0)
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a0 = 0x16;                                          // Result = 00000016
    LIBCD_CdControlF();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    v0 = lwl(v0, s0 + 0x3);
    v0 = lwr(v0, s0);
    swl(v0, a1 + 0x3);                                  // Store to: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    swr(v0, a1);                                        // Store to: gPSXCD_lastloc (80077DF4)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF8;                                       // Result = gPSXCD_beginloc (80077DF8)
    v0 = lwl(v0, a1 + 0x3);                             // Load from: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    v0 = lwr(v0, a1);                                   // Load from: gPSXCD_lastloc (80077DF4)
    swl(v0, a0 + 0x3);                                  // Store to: gPSXCD_beginloc + 3 (80077DFB) (80077DFB)
    swr(v0, a0);                                        // Store to: gPSXCD_beginloc (80077DF8)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF0;                                       // Result = gPSXCD_newloc (80077DF0)
    v0 = lwl(v0, a1 + 0x3);                             // Load from: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    v0 = lwr(v0, a1);                                   // Load from: gPSXCD_lastloc (80077DF4)
    swl(v0, a0 + 0x3);                                  // Store to: gPSXCD_newloc + 3 (80077DF3) (80077DF3)
    swr(v0, a0);                                        // Store to: gPSXCD_newloc (80077DF0)
loc_80040A18:
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

void psxcd_play_at() noexcept {
loc_80040A48:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a1;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0x7C08;                                       // Result = gCdlLOCArray[0] (800783F8)
    a0 <<= 2;
    sw(s0, sp + 0x10);
    s0 = a0 + v0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    v0 = lw(s0);
    s2 = a2;
    if (v0 == 0) goto loc_80040B50;
    sw(0, gp + 0x7E8);                                  // Store to: gbPSXCD_playflag (80077DC8)
    sw(0, gp + 0x7F8);                                  // Store to: gbPSXCD_loopflag (80077DD8)
    sw(0, gp + 0x77C);                                  // Store to: gbPSXCD_seeking_for_play (80077D5C)
    psxcd_set_audio_mode();
    sw(s1, gp + 0x7EC);                                 // Store to: gPSXCD_playvol (80077DCC)
    a0 = s0;
    LIBCD_CdPosToInt();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DE8;                                       // Result = gPSXCD_cdloc (80077DE8)
    a0 = v0 + s2;
    LIBCD_CdIntToPos();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DE8;                                       // Result = gPSXCD_cdloc (80077DE8)
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x7E8);                                 // Store to: gbPSXCD_playflag (80077DC8)
    sw(v0, gp + 0x77C);                                 // Store to: gbPSXCD_seeking_for_play (80077D5C)
    v0 = 0x16;                                          // Result = 00000016
    sw(0, gp + 0x7B8);                                  // Store to: gPSXCD_playcount (80077D98)
    sw(0, gp + 0x7F8);                                  // Store to: gbPSXCD_loopflag (80077DD8)
    sw(0, gp + 0x7F0);                                  // Store to: gPSXCD_playfadeuptime (80077DD0)
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a0 = 0x16;                                          // Result = 00000016
    LIBCD_CdControlF();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    v0 = lwl(v0, s0 + 0x3);
    v0 = lwr(v0, s0);
    swl(v0, a1 + 0x3);                                  // Store to: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    swr(v0, a1);                                        // Store to: gPSXCD_lastloc (80077DF4)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF8;                                       // Result = gPSXCD_beginloc (80077DF8)
    v0 = lwl(v0, a1 + 0x3);                             // Load from: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    v0 = lwr(v0, a1);                                   // Load from: gPSXCD_lastloc (80077DF4)
    swl(v0, a0 + 0x3);                                  // Store to: gPSXCD_beginloc + 3 (80077DFB) (80077DFB)
    swr(v0, a0);                                        // Store to: gPSXCD_beginloc (80077DF8)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF0;                                       // Result = gPSXCD_newloc (80077DF0)
    v0 = lwl(v0, a1 + 0x3);                             // Load from: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    v0 = lwr(v0, a1);                                   // Load from: gPSXCD_lastloc (80077DF4)
    swl(v0, a0 + 0x3);                                  // Store to: gPSXCD_newloc + 3 (80077DF3) (80077DF3)
    swr(v0, a0);                                        // Store to: gPSXCD_newloc (80077DF0)
loc_80040B50:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void psxcd_play() noexcept {
    a2 = 0;
    psxcd_play_at();
}

void psxcd_seek_for_play_at() noexcept {
loc_80040B8C:
    sp -= 0x20;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0x7C08;                                       // Result = gCdlLOCArray[0] (800783F8)
    a0 <<= 2;
    sw(s0, sp + 0x10);
    s0 = a0 + v0;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0);
    s1 = a1;
    if (v0 == 0) goto loc_80040C88;
    sw(0, gp + 0x7E8);                                  // Store to: gbPSXCD_playflag (80077DC8)
    sw(0, gp + 0x7F8);                                  // Store to: gbPSXCD_loopflag (80077DD8)
    sw(0, gp + 0x77C);                                  // Store to: gbPSXCD_seeking_for_play (80077D5C)
    psxcd_set_audio_mode();
    a0 = s0;
    LIBCD_CdPosToInt();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DE8;                                       // Result = gPSXCD_cdloc (80077DE8)
    a0 = v0 + s1;
    LIBCD_CdIntToPos();
    a0 = 0x16;                                          // Result = 00000016
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DE8;                                       // Result = gPSXCD_cdloc (80077DE8)
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x77C);                                 // Store to: gbPSXCD_seeking_for_play (80077D5C)
    v0 = 0x16;                                          // Result = 00000016
    sw(0, gp + 0x7B8);                                  // Store to: gPSXCD_playcount (80077D98)
    sw(0, gp + 0x7E8);                                  // Store to: gbPSXCD_playflag (80077DC8)
    sw(0, gp + 0x7F8);                                  // Store to: gbPSXCD_loopflag (80077DD8)
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    LIBCD_CdControlF();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    v0 = lwl(v0, s0 + 0x3);
    v0 = lwr(v0, s0);
    swl(v0, a1 + 0x3);                                  // Store to: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    swr(v0, a1);                                        // Store to: gPSXCD_lastloc (80077DF4)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF8;                                       // Result = gPSXCD_beginloc (80077DF8)
    v0 = lwl(v0, a1 + 0x3);                             // Load from: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    v0 = lwr(v0, a1);                                   // Load from: gPSXCD_lastloc (80077DF4)
    swl(v0, a0 + 0x3);                                  // Store to: gPSXCD_beginloc + 3 (80077DFB) (80077DFB)
    swr(v0, a0);                                        // Store to: gPSXCD_beginloc (80077DF8)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF0;                                       // Result = gPSXCD_newloc (80077DF0)
    v0 = lwl(v0, a1 + 0x3);                             // Load from: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    v0 = lwr(v0, a1);                                   // Load from: gPSXCD_lastloc (80077DF4)
    swl(v0, a0 + 0x3);                                  // Store to: gPSXCD_newloc + 3 (80077DF3) (80077DF3)
    swr(v0, a0);                                        // Store to: gPSXCD_newloc (80077DF0)
loc_80040C88:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void psxcd_seek_for_play() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = 0;                                             // Result = 00000000
    psxcd_seek_for_play_at();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_play_status() noexcept {
    sp -= 0x18;
    v1 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    v0 = 3;                                             // Result = 00000003
    sw(ra, sp + 0x10);
    if (v1 == v0) goto loc_80040CE0;
    v0 = 0x16;                                          // Result = 00000016
    {
        const bool bJump = (v1 != v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80040D48;
    }
loc_80040CE0:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DB4;                                       // Result = gPSXCD_check_result[0] (80077DB4)
    a0 = 1;                                             // Result = 00000001
    LIBCD_CdSync();
    sw(v0, gp + 0x7D0);                                 // Store to: gPSXCD_check_intr (80077DB0)
    v1 = 5;                                             // Result = 00000005
    if (v0 == v1) goto loc_80040D14;
    v0 = lbu(gp + 0x7D4);                               // Load from: gPSXCD_check_result[0] (80077DB4)
    v0 &= 2;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80040D48;
    }
loc_80040D14:
    LIBCD_CdFlush();
    v1 = lw(gp + 0x7D0);                                // Load from: gPSXCD_check_intr (80077DB0)
    a1 = lbu(gp + 0x7B2);                               // Load from: gPSXCD_cdl_com (80077D92)
    a2 = lbu(gp + 0x7D4);                               // Load from: gPSXCD_check_result[0] (80077DB4)
    a0 = lw(gp + 0x7AC);                                // Load from: gPSXCD_cdl_errcount (80077D8C)
    v1 += 0x5A;
    a0++;
    sw(v1, gp + 0x7A8);                                 // Store to: gPSXCD_cdl_errintr (80077D88)
    sb(a1, gp + 0x7B3);                                 // Store to: gPSXCD_cdl_errcom (80077D93)
    sb(a2, gp + 0x7B1);                                 // Store to: gPSXCD_cdl_errstat (80077D91)
    sw(a0, gp + 0x7AC);                                 // Store to: gPSXCD_cdl_errcount (80077D8C)
    v0 = 0;                                             // Result = 00000000
loc_80040D48:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_stop() noexcept {
loc_80040D58:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    sw(0, gp + 0x7E8);                                  // Store to: gbPSXCD_playflag (80077DC8)
    sw(0, gp + 0x7F8);                                  // Store to: gbPSXCD_loopflag (80077DD8)
    sw(0, gp + 0x77C);                                  // Store to: gbPSXCD_seeking_for_play (80077D5C)
    sw(0, gp + 0x814);                                  // Store to: gPSXCD_lastloc (80077DF4)
    psxspu_get_cd_vol();
    a0 = 0xFA;                                          // Result = 000000FA
    if (v0 == 0) goto loc_80040D98;
    a1 = 0;                                             // Result = 00000000
    psxspu_start_cd_fade();
loc_80040D88:
    psxspu_get_cd_fade_status();
    if (v0 != 0) goto loc_80040D88;
loc_80040D98:
    psxcd_sync();
    a0 = 9;                                             // Result = 00000009
    a1 = 0;                                             // Result = 00000000
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x780);                                 // Store to: gbPSXCD_waiting_for_pause (80077D60)
    v0 = 9;                                             // Result = 00000009
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    LIBCD_CdControlF();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_pause() noexcept {
loc_80040DD0:
    v0 = lw(gp + 0x814);                                // Load from: gPSXCD_lastloc (80077DF4)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    sw(0, gp + 0x7E8);                                  // Store to: gbPSXCD_playflag (80077DC8)
    sw(0, gp + 0x77C);                                  // Store to: gbPSXCD_seeking_for_play (80077D5C)
    if (v0 == 0) goto loc_80040E64;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF0;                                       // Result = gPSXCD_newloc (80077DF0)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    v0 = lwl(v0, a1 + 0x3);                             // Load from: gPSXCD_newloc + 3 (80077DF3) (80077DF3)
    v0 = lwr(v0, a1);                                   // Load from: gPSXCD_newloc (80077DF0)
    swl(v0, a0 + 0x3);                                  // Store to: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    swr(v0, a0);                                        // Store to: gPSXCD_lastloc (80077DF4)
    psxspu_get_cd_vol();
    a0 = 0xFA;                                          // Result = 000000FA
    if (v0 == 0) goto loc_80040E3C;
    a1 = 0;                                             // Result = 00000000
    psxspu_start_cd_fade();
loc_80040E2C:
    psxspu_get_cd_fade_status();
    if (v0 != 0) goto loc_80040E2C;
loc_80040E3C:
    psxcd_sync();
    a0 = 9;                                             // Result = 00000009
    a1 = 0;                                             // Result = 00000000
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x780);                                 // Store to: gbPSXCD_waiting_for_pause (80077D60)
    v0 = 9;                                             // Result = 00000009
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    LIBCD_CdControlF();
loc_80040E64:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_restart() noexcept {
loc_80040E74:
    v0 = lw(gp + 0x814);                                // Load from: gPSXCD_lastloc (80077DF4)
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    if (v0 == 0) goto loc_80040EE8;
    psxcd_set_audio_mode();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DF4;                                       // Result = gPSXCD_lastloc (80077DF4)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DE8;                                       // Result = gPSXCD_cdloc (80077DE8)
    v0 = lwl(v0, a1 + 0x3);                             // Load from: gPSXCD_lastloc + 3 (80077DF7) (80077DF7)
    v0 = lwr(v0, a1);                                   // Load from: gPSXCD_lastloc (80077DF4)
    swl(v0, a0 + 0x3);                                  // Store to: gPSXCD_cdloc + 3 (80077DEB) (80077DEB)
    swr(v0, a0);                                        // Store to: gPSXCD_cdloc (80077DE8)
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0x7E8);                                 // Store to: gbPSXCD_playflag (80077DC8)
    sw(v0, gp + 0x77C);                                 // Store to: gbPSXCD_seeking_for_play (80077D5C)
    v0 = 0x16;                                          // Result = 00000016
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7DE8;                                       // Result = gPSXCD_cdloc (80077DE8)
    sw(s0, gp + 0x7EC);                                 // Store to: gPSXCD_playvol (80077DCC)
    sw(0, gp + 0x7B8);                                  // Store to: gPSXCD_playcount (80077D98)
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a0 = 0x16;                                          // Result = 00000016
    LIBCD_CdControlF();
loc_80040EE8:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_elapsed_sectors() noexcept {
    // Emulate the cdrom a bit in case this function is polled in a loop
    #if PC_PSX_DOOM_MODS
        emulate_cdrom();
    #endif

loc_80040EFC:
    v0 = lw(gp + 0x818);                                // Load from: gPSXCD_beginloc (80077DF8)
    sp -= 0x18;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_80040F38;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF0;                                       // Result = gPSXCD_newloc (80077DF0)
    LIBCD_CdPosToInt();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF8;                                       // Result = gPSXCD_beginloc (80077DF8)
    s0 = v0;
    LIBCD_CdPosToInt();
    v0 = s0 - v0;
    goto loc_80040F3C;
loc_80040F38:
    v0 = 0;                                             // Result = 00000000
loc_80040F3C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void psxcd_set_stereo() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (a0 == 0) goto loc_80040F78;
    v0 = 0x7F;                                          // Result = 0000007F
    sb(v0, gp + 0x80C);                                 // Store to: gPSXCD_cdatv[0] (80077DEC)
    sb(0, gp + 0x80D);                                  // Store to: gPSXCD_cdatv[1] (80077DED)
    sb(v0, gp + 0x80E);                                 // Store to: gPSXCD_cdatv[2] (80077DEE)
    sb(0, gp + 0x80F);                                  // Store to: gPSXCD_cdatv[3] (80077DEF)
    goto loc_80040F8C;
loc_80040F78:
    v0 = 0x3F;                                          // Result = 0000003F
    sb(v0, gp + 0x80C);                                 // Store to: gPSXCD_cdatv[0] (80077DEC)
    sb(v0, gp + 0x80D);                                 // Store to: gPSXCD_cdatv[1] (80077DED)
    sb(v0, gp + 0x80E);                                 // Store to: gPSXCD_cdatv[2] (80077DEE)
    sb(v0, gp + 0x80F);                                 // Store to: gPSXCD_cdatv[3] (80077DEF)
loc_80040F8C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DEC;                                       // Result = gPSXCD_cdatv[0] (80077DEC)
    LIBCD_CdMix();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}
