#include "PSXCD.h"

#include "Doom/cdmaptbl.h"
#include "PcPsx/ModMgr.h"
#include "PcPsx/Types.h"
#include "psxspu.h"
#include "PsxVm/PsxVm.h"
#include "PsxVm/VmPtr.h"
#include "PsxVm/VmSVal.h"
#include "PsyQ/LIBCD.h"
#include "wessarc.h"

// PSXCD module commands within PsxCd_Command
enum PsxCd_CmdOp : int32_t {
    PSXCD_COMMAND_END       = 0,    // Finish up the async read
    PSXCD_COMMAND_COPY      = 1,
    PSXCD_COMMAND_SEEK      = 2,
    PSXCD_COMMAND_READ      = 3,
    PSXCD_COMMAND_READCOPY  = 4
};

// Holds a command to read sector data
struct PsxCd_Command {
    PsxCd_CmdOp     command;        // What command was executed
    int32_t         amount;         // Number of bytes involved in the read/copy operation
    VmPtr<uint8_t>  pdest;          // Destination to save bytes to (if required by command type)
    VmPtr<uint8_t>  psrc;           // Source to get bytes from (if required by command type)
    CdlLOC          io_loc;         // I/O location for the command
};

// Time it takes to fade out CD audio (milliseconds)
static constexpr int32_t FADE_TIME_MS = 250;

// Locations on the disc for all CD tracks.
// This is used to determine where to seek to for cd audio playback.
static const VmPtr<CdlLOC[CdlMAXTOC]>   gTrackCdlLOC(0x800783F8);

// Various flags
static const VmPtr<bool32_t>    gbPSXCD_IsCdInit(0x80077D70);               // If true then the 'psxcd' module has been initialized
static const VmPtr<bool32_t>    gbPSXCD_init_pos(0x80077D74);               // If this flag is false then we need to initialize the cd position for data reading (we don't know it), true if we initialized the cd position
static const VmPtr<bool32_t>    gbPSXCD_async_on(0x80077D64);               // True when there is an asynchronous read happening
static const VmPtr<int32_t>     gbPSXCD_cb_enable_flag(0x80077D7C);         // Non zero (true) if callbacks are currently enabled
static const VmPtr<bool32_t>    gbPSXCD_playflag(0x80077DC8);               // If true then we are playing cd audio
static const VmPtr<bool32_t>    gbPSXCD_loopflag(0x80077DD8);               // If true then the currently played cd audio track will be looped
static const VmPtr<bool32_t>    gbPSXCD_seeking_for_play(0x80077D5C);       // If true then we are currently seeking to the location where the cd audio track being played will start
static const VmPtr<bool32_t>    gbPSXCD_waiting_for_pause(0x80077D60);      // If true then we are waiting for a cd 'pause' operation to complete
static const VmPtr<bool32_t>    gbPSXCD_critical_error(0x80077D80);         // True if a critical error occurred

// Whether the cdrom is currently in data or audio mode.
// 0 = audio mode, 1 = data mode, -1 = undefined.
static const VmPtr<int32_t>     gPSXCD_psxcd_mode(0x80077D6C);

// Data reading mode stuff
static const VmPtr<PsxCd_File>      gPSXCD_cdfile(0x8007831C);              // Used to hold a file temporarily after opening
static const VmPtr<CdlLOC>          gPSXCD_cur_io_loc(0x80077D78);          // Current IO location on disc
static const VmPtr<CdlLOC>          gPSXCD_sectorbuf_contents(0x80077D68);  // Where the current sector buffer contents came from on disc
static const VmPtr<VmPtr<void>>     gpPSXCD_lastdestptr(0x80077DC0);        // Async read: destination memory chunk being written to
static const VmPtr<int32_t>         gPSXCD_lastreadbytes(0x80077DC4);       // Async read: number of bytes being read
static const VmPtr<PsxCd_File>      gPSXCD_lastfilestruct(0x800783D0);      // Async read: details for the file being read
static const VmPtr<int32_t>         gPSXCD_cur_cmd(0x80077DBC);             // Async read: index of the current command being issued in the loop iteration
static const VmPtr<PsxCd_Command>   gPSXCD_psxcd_cmds(0x80078344);          // Async read: commands issued to the cd

// Sector buffer for when we are reading data
static const VmPtr<uint8_t[CD_SECTOR_SIZE]> gPSXCD_sectorbuf(0x800A9518);

// Audio mode stuff
static const VmPtr<CdlLOC>      gPSXCD_lastloc(0x80077DF4);             // The last valid intended cd-audio disc seek location
static const VmPtr<CdlLOC>      gPSXCD_newloc(0x80077DF0);              // Last known location for cd audio playback, this gets continously saved to so we can restore if we want to pause
static const VmPtr<CdlLOC>      gPSXCD_beginloc(0x80077DF8);            // Start sector of the current audio track
static const VmPtr<CdlLOC>      gPSXCD_cdloc(0x80077DE8);               // Temporary CdlLOC variable used in various places
static const VmPtr<int32_t>     gPSXCD_playvol(0x80077DCC);             // Specified playback volume for cd audio
static const VmPtr<int32_t>     gPSXCD_loopvol(0x80077DDC);             // Volume to playback cd audio when looping around again
static const VmPtr<CdlATV>      gPSXCD_cdatv(0x80077DEC);               // The volume mixing levels of cd audio sent to the SPU for output
static const VmPtr<int32_t>     gPSXCD_playfadeuptime(0x80077DD0);      // If > 0 then fade up cd audio volume in this amount of time (MS) when beginning playback
static const VmPtr<int32_t>     gPSXCD_loopfadeuptime(0x80077DE4);      // Same as the regular 'fade up time', but used when we loop
static const VmPtr<int32_t>     gPSXCD_looptrack(0x80077DD4);           // What track to loop after the current one ends
static const VmPtr<int32_t>     gPSXCD_loopsectoroffset(0x80077DE0);    // The sector offset to begin the loop track at

// Number of sectors read in data and audio mode respectively
static const VmPtr<int32_t>     gPSXCD_readcount(0x80077D94);           // Number of data sectors read
static const VmPtr<int32_t>     gPSXCD_playcount(0x80077D98);           // Number of audio sectors read

// CD commands issued and results
static const VmPtr<uint8_t>     gPSXCD_cdl_com(0x80077D92);             // The last command issued to the cdrom via 'LIBCD_CdControl'
static const VmPtr<uint8_t>     gPSXCD_cdl_err_com(0x80077D93);         // The last command issued to the cdrom via 'LIBCD_CdControl' which was an error

static const VmPtr<uint8_t[8]>  gPSXCD_cd_param(0x80077D9C);            // Parameters for the last command issued to the cdrom via 'LIBCD_CdControl' (if there were parameters)

static const VmPtr<int32_t>     gPSXCD_sync_intr(0x80077DA4);           // Int result of the last 'LIBCD_CdSync' call when waiting until command completion
static const VmPtr<int32_t>     gPSXCD_check_intr(0x80077DB0);          // Int result of the last 'LIBCD_CdSync' call when querying the status of something
static const VmPtr<int32_t>     gPSXCD_cdl_err_intr(0x80077D88);        // Int result of the last 'LIBCD_CdSync' call with an error

static const VmPtr<uint8_t[8]>  gPSXCD_sync_result(0x80077DA8);         // Result bytes for the last 'LIBCD_CdSync' call when waiting until command completion
static const VmPtr<uint8_t[8]>  gPSXCD_check_result(0x80077DB4);        // Result bytes for the last 'LIBCD_CdSync' call when querying the status of something

static const VmPtr<int32_t>     gPSXCD_cdl_err_count(0x80077D8C);       // A count of how many cd errors that occurred
static const VmPtr<uint8_t>     gPSXCD_cdl_err_stat(0x80077D91);        // The first result byte (status byte) for when the last error which occurred

// Previous 'CdReadyCallback' and 'CDSyncCallback' functions used by LIBCD prior to initializing this module.
// Used for restoring once we shutdown this module.
static CdlCB gPSXCD_cbsyncsave;
static CdlCB gPSXCD_cbreadysave;

// Function forward declarations
void psxcd_async_read_cancel() noexcept;
static int32_t psxcd_async_read(void* const pDest, const int32_t numBytes, PsxCd_File& file) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Simple memcpy() operation
//------------------------------------------------------------------------------------------------------------------------------------------
static void PSXCD_psxcd_memcpy(void* const pDst, const void* const pSrc, uint32_t count) noexcept {
    uint8_t* const pDstBytes = (uint8_t*) pDst;
    const uint8_t* const pSrcBytes = (const uint8_t*) pSrc;

    for (uint32_t i = 0; i < count; ++i) {
        pDstBytes[i] = pSrcBytes[i];
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait until the current CD command has completed successfully (with a timeout)
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_sync() noexcept {
    // PC-PSX: this logic is no longer neccessary.
    // The reimplementation of LIBCD executes everything synchronously.
    #if !PC_PSX_DOOM_MODS
        const uint32_t timeoutMs = *gWess_Millicount + 8000;

        while (*gWess_Millicount < timeoutMs) {
            *gPSXCD_sync_intr = LIBCD_CdSync(1, gPSXCD_sync_result.get());

            if (*gPSXCD_sync_intr == CdlDiskError) {
                // Cancel the current command if there was a problem and try again
                LIBCD_CdFlush();
                *gPSXCD_cdl_err_count += 1;
                *gPSXCD_cdl_err_intr = *gPSXCD_sync_intr + 80;   // '+': Just to make the codes more unique, so their source is known
                *gPSXCD_cdl_err_com = *gPSXCD_cdl_com;
                *gPSXCD_cdl_err_stat = gPSXCD_sync_result[0];
            }

            if (*gPSXCD_sync_intr == CdlComplete)
                break;
        }
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait until the current CD command has completed successfully.
// Do so with a timeout, or give up if there is an error.
// Return 'true' if the CD command succeeded.
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_critical_sync() noexcept {
    // PC-PSX: this logic is no longer neccessary.
    // The reimplementation of LIBCD executes everything synchronously.
    #if PC_PSX_DOOM_MODS
        return true;
    #else
        const uint32_t timeoutMs = *gWess_Millicount + 8000;

        while (*gWess_Millicount < timeoutMs) {
            *gPSXCD_sync_intr = LIBCD_CdSync(1, gPSXCD_sync_result.get());

            if (*gPSXCD_sync_intr == CdlDiskError) {
                // Cancel the current command if there was a problem
                LIBCD_CdFlush();
                *gPSXCD_cdl_err_count += 1;
                *gPSXCD_cdl_err_intr = *gPSXCD_sync_intr + 70;      // '+': Just to make the codes more unique, so their source is known
                *gPSXCD_cdl_err_com = *gPSXCD_cdl_com;
                *gPSXCD_cdl_err_stat = gPSXCD_sync_result[0];

                return false;   // Give up if an error happens!
            }

            if (*gPSXCD_sync_intr == CdlComplete)
                return true;
        }

        return false;   // Timeout!
    #endif
}

void PSXCD_cbcomplete(const CdlSyncStatus status, const uint8_t pResult[8]) noexcept {
    // TEMP until this is converted to C++
    struct ResultBytes {
        char bytes[8];
    };

    a0 = status;
    VmSVal<ResultBytes> resultBytesVmStack;

    for (int32_t i = 0; i < 8; ++i) {
        resultBytesVmStack->bytes[i] = pResult[i];
    }
    
    a1 = resultBytesVmStack.addr();

    //---------------------

    v0 = *gbPSXCD_cb_enable_flag;
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
    psxspu_set_cd_vol(a0);
    a0 = lw(gp + 0x7F0);                                // Load from: gPSXCD_playfadeuptime (80077DD0)
    a1 = lw(gp + 0x7EC);                                // Load from: gPSXCD_playvol (80077DCC)
    psxspu_start_cd_fade(a0, a1);
    sw(0, gp + 0x7F0);                                  // Store to: gPSXCD_playfadeuptime (80077DD0)
    v0 = 3;                                             // Result = 00000003
    goto loc_8003F434;
loc_8003F424:
    a0 = lw(gp + 0x7EC);                                // Load from: gPSXCD_playvol (80077DCC)
    psxspu_set_cd_vol(a0);
    v0 = 3;                                             // Result = 00000003
loc_8003F434:
    sb(v0, gp + 0x7B2);                                 // Store to: gPSXCD_cdl_com (80077D92)
    a0 = 3;                                             // Result = 00000003
    a1 = 0;                                             // Result = 00000000
    _thunk_LIBCD_CdControlF();
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

void PSXCD_cbready(const CdlSyncStatus status, const uint8_t pResult[8]) noexcept {
    // TEMP until this is converted to C++
    struct ResultBytes {
        char bytes[8];
    };

    a0 = status;
    VmSVal<ResultBytes> resultBytesVmStack;

    for (int32_t i = 0; i < 8; ++i) {
        resultBytesVmStack->bytes[i] = pResult[i];
    }
    
    a1 = resultBytesVmStack.addr();

    //---------------------

    v0 = *gbPSXCD_cb_enable_flag;
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
    _thunk_LIBCD_CdGetSector();
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
    PSXCD_psxcd_memcpy(vmAddrToPtr<void>(a0), vmAddrToPtr<void>(a1), a2);
    goto loc_8003F5C0;
loc_8003F5B8:
    a1 = 0x200;                                         // Result = 00000200
    _thunk_LIBCD_CdGetSector();
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
    _thunk_LIBCD_CdGetSector();
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
    PSXCD_psxcd_memcpy(vmAddrToPtr<void>(a0), vmAddrToPtr<void>(a1), a2);
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
    _thunk_LIBCD_CdControlF();
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
    psxcd_play_at_andloop(a0, a1, a2, a3, lw(sp + 0x10), lw(sp + 0x14), lw(sp + 0x18), lw(sp + 0x1C));
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Disable/ignore internal cd related callbacks
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_disable_callbacks() noexcept {
    *gbPSXCD_cb_enable_flag = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable internal cd related callbacks
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_enable_callbacks() noexcept {
    *gbPSXCD_cb_enable_flag = 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the WESS (Williams Entertainment Sound System) CD handling module
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_init() noexcept {
    // If we've already done this then just no-op
    if (*gbPSXCD_IsCdInit)
        return;

    // Initialize LIBCD
    LIBCD_CdInit();
    *gbPSXCD_IsCdInit = true;

    // Clear the locations for all CD tracks
    for (int32_t trackIdx = 0; trackIdx < CdlMAXTOC; ++trackIdx) {
        CdlLOC& trackpos = gTrackCdlLOC[trackIdx];
        trackpos.minute = 0;
        trackpos.second = 0;
        trackpos.sector = 0;
        trackpos.track = 0;
    }

    // Initialize the SPU
    psxspu_init();

    // Get the locations of all CD tracks
    const int32_t trackCount = LIBCD_CdGetToc(gTrackCdlLOC.get());

    if (trackCount != 0) {
        *gbPSXCD_init_pos = false;
        *gbPSXCD_async_on = false;
        
        // Set command complete and data callbacks and save the old ones for later restoring
        gPSXCD_cbsyncsave = LIBCD_CdSyncCallback(PSXCD_cbcomplete);
        gPSXCD_cbreadysave = LIBCD_CdReadyCallback(PSXCD_cbready);

        psxcd_enable_callbacks();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shut down the WESS (Williams Entertainment Sound System) CD handling module
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_exit() noexcept {
    // Restore these previous callbacks
    LIBCD_CdSyncCallback(gPSXCD_cbsyncsave);
    LIBCD_CdReadyCallback(gPSXCD_cbreadysave);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Switches the cdrom drive into data reading mode
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_set_data_mode() noexcept {
    if (*gPSXCD_psxcd_mode != 1) {
        // Currently in audio mode: begin fading out audio and clear playback flags
        *gbPSXCD_playflag = 0;
        *gbPSXCD_loopflag = 0;
        *gbPSXCD_seeking_for_play = 0;

        const int32_t cdVol = psxspu_get_cd_vol();

        if (cdVol != 0) {
            psxspu_start_cd_fade(FADE_TIME_MS, 0);

            while (psxspu_get_cd_fade_status()) {
                // Wait for the fade to complete...
                // TODO: PC-PSX: yield CPU cycles here and handle window events if pausing.
            }
        }

        // Disable cd audio mixing
        psxspu_setcdmixoff();
        
        // Ensure no pending commands and set the cdrom mode to double speed data
        psxcd_sync();

        *gPSXCD_psxcd_mode = 1;
        *gPSXCD_cdl_com = CdlSetmode;
        gPSXCD_cd_param[0] = CdlModeSpeed;  // Operate the CDROM at 2x speed
        LIBCD_CdControl(CdlSetmode, gPSXCD_cd_param.get(), nullptr);
        psxcd_sync();
    } else {
        // Already in data mode: just cancel any reads or commands in flight
        if (*gbPSXCD_async_on) {
            psxcd_async_read_cancel();
        }

        psxcd_sync();
        
        if (*gPSXCD_cdl_com == CdlPause) {
            LIBCD_CdFlush();
        }
    }
    
    // Stop the cdrom at the current location when we are done to finish up
    *gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControl(CdlPause, nullptr, nullptr);
    psxcd_sync();
    LIBCD_CdFlush();
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

    #if PC_PSX_DOOM_MODS
        // Modding mechanism: allow files to be overriden with user files in a specified directory.
        if (ModMgr::areOverridesAvailableForFile(discFile)) {
            return (ModMgr::openOverridenFile(discFile, *gPSXCD_cdfile)) ? gPSXCD_cdfile.get() : nullptr;
        }

        // Clear all fields in the PSXCD file prior to using it - some fields like the track number don't appear to be set otherwise.
        // This is required for there to be no issues with modding.
        *gPSXCD_cdfile = {};
    #endif

    // Figure out where the file is on disc and save it's size
    const PsxCd_MapTblEntry& fileTableEntry = CD_MAP_TBL[(uint32_t) discFile];
    LIBCD_CdIntToPos(fileTableEntry.startSector, gPSXCD_cdfile->file.pos);
    gPSXCD_cdfile->file.size = fileTableEntry.size;

    // Initialize file IO position and status
    gPSXCD_cdfile->new_io_loc = gPSXCD_cdfile->file.pos;
    gPSXCD_cdfile->io_block_offset = 0;

    for (uint8_t& statusByte : gPSXCD_cdfile->io_result) {
        statusByte = 0;
    }

    return gPSXCD_cdfile.get();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes some positional related variables
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_init_pos() noexcept {
    *gbPSXCD_seeking_for_play = false;
    *gbPSXCD_waiting_for_pause = false;
    *gbPSXCD_init_pos = false;
    *gbPSXCD_critical_error = false;
    *gbPSXCD_playflag = false;
    *gbPSXCD_loopflag = false;
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
    _thunk_LIBCD_CdSync();
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
    v0 = psxcd_async_read(vmAddrToPtr<void>(a0), a1, *vmAddrToPtr<PsxCd_File>(a2));
    v0 = 1;                                             // Result = 00000001
    goto loc_8003FCB4;
loc_8003FCB0:
    v0 = 0;                                             // Result = 00000000
loc_8003FCB4:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the cdrom is currently seeking to a location for audio playback
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_seeking_for_play() noexcept {
    // If we are not seeking then the answer is simple
    if (!*gbPSXCD_seeking_for_play)
        return false;
    
    // PC-PSX: this fancy error handling is not necessary in this emulated environment
    #if !PC_PSX_DOOM_MODS
        // The cdrom is still busy seeking: check to make sure there was not an error
        *gPSXCD_check_intr = LIBCD_CdSync(1, gPSXCD_check_result.get());
    
        if ((*gPSXCD_check_intr == CdlDiskError) || ((gPSXCD_check_result[0] & CdlStatStandby) == 0)) {
            // Some sort of error happened and not on standby: cancel the current command and record the command details
            LIBCD_CdFlush();

            *gPSXCD_cdl_err_count += 1;
            *gPSXCD_cdl_err_intr = *gPSXCD_check_intr + 110;    // '+': Just to make the codes more unique, so their source is known
            *gPSXCD_cdl_err_com = *gPSXCD_cdl_com;
            *gPSXCD_cdl_err_stat = gPSXCD_check_result[0];

            // Try to seek to the last valid intended cd audio location
            psxcd_sync();
            *gPSXCD_cdl_com = CdlSeekP;
            LIBCD_CdControlF(CdlSeekP, (uint8_t*) gPSXCD_lastloc.get());
        }
    #endif
    
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the cdrom is currently in the process of pausing
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_waiting_for_pause() noexcept {
    // If we are not pausing then the answer is simple
    if (!*gbPSXCD_waiting_for_pause)
        return false;

    // PC-PSX: this fancy error handling is not necessary in this emulated environment
    #if !PC_PSX_DOOM_MODS
        // The cdrom is still busy pausing: check to make sure there was not an error
        *gPSXCD_check_intr = LIBCD_CdSync(1, gPSXCD_check_result.get());

        if ((*gPSXCD_check_intr == CdlDiskError) || ((gPSXCD_check_result[0] & CdlStatStandby) == 0)) {
            // Some sort of error happened and not on standby: cancel the current command and record the command details
            LIBCD_CdFlush();

            *gPSXCD_cdl_err_count += 1;
            *gPSXCD_cdl_err_intr = *gPSXCD_check_intr + 120;    // '+': Just to make the codes more unique, so their source is known
            *gPSXCD_cdl_err_com = *gPSXCD_cdl_com;
            *gPSXCD_cdl_err_stat = gPSXCD_check_result[0];

            // Retry the pause command
            psxcd_sync();
            *gPSXCD_cdl_com = CdlPause;
            LIBCD_CdControlF(CdlPause, nullptr);
        }
    #endif

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the specified number of bytes synchronously from the given CD file.
// Returns the number of bytes read.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_read(void* const pDest, int32_t numBytes, PsxCd_File& file) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    #if PC_PSX_DOOM_MODS
        if (ModMgr::isFileOverriden(file)) {
            return ModMgr::readFromOverridenFile(pDest, numBytes, file);
        }
    #endif

    // Kick off the async read.
    // Note: number of bytes read will not match request if there was an error!
    const int32_t retBytesRead = psxcd_async_read(pDest, numBytes, file);

    // Continue reading until done
    bool bIsDoingAsyncRead = true;

    do {
        psxcd_async_on();
        bIsDoingAsyncRead = (v0 != 0);
    } while (bIsDoingAsyncRead);
    
    return retBytesRead;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cancels the currently active async read, if any
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_async_read_cancel() noexcept {
    // If we are not doing an async read then there is nothing to do
    if (!*gbPSXCD_async_on)
        return;

    // Finish up any current commands and mark the data position as uninitialized
    *gbPSXCD_async_on = false;
    *gbPSXCD_init_pos = false;
    psxcd_sync();

    // Pause the cdrom
    *gbPSXCD_waiting_for_pause = true;
    *gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControlF(CdlPause, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Submit commands to the cdrom to do an asynchronous read of the specified number of bytes
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t psxcd_async_read(void* const pDest, const int32_t numBytes, PsxCd_File& file) noexcept {
    // If no bytes are being read or the file is invalid then there is nothing to do
    if ((numBytes == 0) || (file.file.pos == 0))
        return 0;

    bool bReadError;

    do {
        // Clear the error flag and ensure we are in data mode
        bReadError = false;
        psxcd_set_data_mode();
        
        // Save read details
        *gpPSXCD_lastdestptr = pDest;
        *gPSXCD_lastreadbytes = numBytes;
        *gPSXCD_lastfilestruct = file;

        // We start putting commands at the beginning of the command list
        *gPSXCD_cur_cmd = 0;

        // First handle the read location being misaligned with sector boundaries and queue read/copy commands for up until the end of the start sector.
        // This is done to get I/O to sector align so we can read a series of whole sectors in one continous operation (below).
        int32_t numBytesLeft = numBytes;
        uint8_t* pDestBytes = (uint8_t*) pDest;

        if ((file.io_block_offset != 0) && (numBytesLeft != 0)) {
            // Do we need to seek to the read location in the file? If so then queue up a seek command
            if ((!*gbPSXCD_init_pos) || (*gPSXCD_cur_io_loc != file.new_io_loc)) {
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command = PSXCD_COMMAND_SEEK;
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].io_loc = file.new_io_loc;
                *gPSXCD_cur_cmd += 1;

                // We know where we are going to be now
                *gPSXCD_cur_io_loc = file.new_io_loc;
                *gbPSXCD_init_pos = true;
            }
            
            // Decide how many bytes do we want to read from the sector
            const int32_t sectorBytesLeft = CD_SECTOR_SIZE - file.io_block_offset;
            const int32_t bytesToCopy = (numBytesLeft <= sectorBytesLeft) ? numBytesLeft : sectorBytesLeft;

            // Do we already have the required sector in the sector buffer?
            // If that is the case we can just copy the buffer contents, otherwise we need to read from the CD and then copy.
            if ((*gPSXCD_cur_cmd == PSXCD_COMMAND_END) && (*gPSXCD_sectorbuf_contents == *gPSXCD_cur_io_loc)) {
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command = PSXCD_COMMAND_COPY;                    
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].psrc = gPSXCD_sectorbuf.get() + file.io_block_offset;
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].pdest = pDestBytes;
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].amount = bytesToCopy;

                *gPSXCD_cur_cmd += 1;
            } else {
                // Haven't got the sector buffered, need to read first before we can copy
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command = PSXCD_COMMAND_READCOPY;
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].psrc = gPSXCD_sectorbuf.get() + file.io_block_offset;
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].pdest = pDestBytes;
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].amount = bytesToCopy;
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].io_loc = *gPSXCD_cur_io_loc;

                *gPSXCD_sectorbuf_contents = *gPSXCD_cur_io_loc;
                *gPSXCD_cur_cmd += 1;
            }

            // Move along past the bytes read
            numBytesLeft -= bytesToCopy;
            pDestBytes += bytesToCopy;
            file.io_block_offset = file.io_block_offset + bytesToCopy;

            // Move onto a new sector if required
            if (file.io_block_offset == CD_SECTOR_SIZE) {
                const int32_t curSector = LIBCD_CdPosToInt(*gPSXCD_cur_io_loc);
                LIBCD_CdIntToPos(curSector + 1, *gPSXCD_cur_io_loc);
                file.new_io_loc = *gPSXCD_cur_io_loc;
                file.io_block_offset = 0;
            }
        }
        
        // Next queue commands to read as many whole sectors as we can in one read operation.
        const int32_t wholeSectorsToRead = numBytesLeft / CD_SECTOR_SIZE;

        if (wholeSectorsToRead > 0) {
            // Do we need to seek to the read location in the file? If so then queue up a seek command
            if ((!*gbPSXCD_init_pos) || (*gPSXCD_cur_io_loc != file.new_io_loc)) {
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command = PSXCD_COMMAND_SEEK;
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].io_loc = file.new_io_loc;
                *gPSXCD_cur_cmd += 1;

                // We know where we are going to be now
                *gPSXCD_cur_io_loc = file.new_io_loc;
                *gbPSXCD_init_pos = true;
            }

            // How many bytes would be read by this operation?
            const int32_t wholeSectorBytes = wholeSectorsToRead * CD_SECTOR_SIZE;
            
            // Queue the read command
            gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command = PSXCD_COMMAND_READ;
            gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].amount = wholeSectorsToRead;
            gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].pdest = pDestBytes;

            // TODO: what is this check trying to do?
            if ((uintptr_t) pDestBytes & 3) {
                const int32_t curSector = LIBCD_CdPosToInt(*gPSXCD_cur_io_loc);
                LIBCD_CdIntToPos(curSector + wholeSectorsToRead - 1, *gPSXCD_sectorbuf_contents);
            }

            gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].io_loc = *gPSXCD_cur_io_loc;
            *gPSXCD_cur_cmd += 1;

            // Move along in the ouput
            pDestBytes += wholeSectorBytes;
            numBytesLeft -= wholeSectorBytes;

            // Figure out which sector to go to next
            {
                const int32_t curSector = LIBCD_CdPosToInt(*gPSXCD_cur_io_loc);
                LIBCD_CdIntToPos(curSector + wholeSectorsToRead, *gPSXCD_cur_io_loc);
            }

            file.io_block_offset = 0;
            file.new_io_loc = *gPSXCD_cur_io_loc;
        }

        // Queue commands to read the remaining few bits of data
        if (numBytesLeft != 0) {
            // Do we need to seek to the read location in the file? If so then queue up a seek command
            if ((!*gbPSXCD_init_pos) || (*gPSXCD_cur_io_loc != file.new_io_loc)) {
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command = PSXCD_COMMAND_SEEK;
                gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].io_loc = file.new_io_loc;
                *gPSXCD_cur_cmd += 1;

                // We know where we are going to be now
                *gPSXCD_cur_io_loc = file.new_io_loc;
                *gbPSXCD_init_pos = true;
            }

            // Queue the read and copy command
            gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command = PSXCD_COMMAND_READCOPY;
            gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].psrc = gPSXCD_sectorbuf.get();
            gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].pdest = pDestBytes;
            gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].amount = numBytesLeft;
            gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].io_loc = *gPSXCD_cur_io_loc;
            *gPSXCD_cur_cmd += 1;

            // Move along the I/O location
            *gPSXCD_sectorbuf_contents = *gPSXCD_cur_io_loc;
            file.io_block_offset = numBytesLeft;
        }

        // Terminate the command list.
        // After this all 'commands' are queued and we can begin issuing them to the CDROM via LIBCD.
        gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command = PSXCD_COMMAND_END;
        *gPSXCD_cur_cmd = 0;

        // Issue command: are we to copy bytes to the destination buffer?
        if (gPSXCD_psxcd_cmds[0].command == PSXCD_COMMAND_COPY) {            
            PSXCD_psxcd_memcpy(gPSXCD_psxcd_cmds[0].pdest.get(), gPSXCD_psxcd_cmds[0].psrc.get(), gPSXCD_psxcd_cmds[0].amount);
            *gPSXCD_cur_cmd += 1;

            // No more commands?
            if (gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command == PSXCD_COMMAND_END)
                return numBytes;
        }

        // Issue command: are we to seek to a location?
        if (gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command == PSXCD_COMMAND_SEEK) {
            psxcd_sync();
            *gPSXCD_cdl_com = CdlSetloc;
            LIBCD_CdControl(CdlSetloc, (const uint8_t*) &gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].io_loc, nullptr);
            *gPSXCD_cur_cmd += 1;

            // No more commands?
            if (gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command == PSXCD_COMMAND_END)
                return numBytes;
        }

        // Issue whatever remains in the command list at this point
        switch (gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].command) {
            // No more commands?
            case PSXCD_COMMAND_END:
                return numBytes;

            // Are we to read? If so then kick that off
            case PSXCD_COMMAND_READ:
            case PSXCD_COMMAND_READCOPY: {
                if (psxcd_critical_sync()) {
                    *gPSXCD_cdl_com = CdlReadN;
                    LIBCD_CdControl(CdlReadN, (const uint8_t*) &gPSXCD_psxcd_cmds[*gPSXCD_cur_cmd].io_loc, nullptr);
                    
                    if (psxcd_critical_sync()) {
                        // We are now doing an asynchronous read.
                        // The flag won't be cleared until it is done or cancelled.
                        *gbPSXCD_async_on = true; 
                    } else {
                        bReadError = true;
                    }
                } else {
                    bReadError = true;
                }
            }   break;

            // Unknown/unexpected command here: read failed!
            default:
                return 0;
        }

        // If a read error happened then try redo the entire read
        if (bReadError) {
            file = *gPSXCD_lastfilestruct;
        }
    } while (bReadError);
    
    return numBytes;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek to a specified position in a file, relatively or absolutely.
// Returns '0' on success, any other value on failure.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_seek(PsxCd_File& file, int32_t offset, const PsxCd_SeekMode mode) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    #if PC_PSX_DOOM_MODS
        if (ModMgr::isFileOverriden(file)) {
            return ModMgr::seekForOverridenFile(file, offset, mode);
        }
    #endif

    // Is this an actual valid file? If not then just NOP the call and return '0' for 'success':
    if (file.file.pos == CdlLOC{ 0, 0, 0, 0 })
        return 0;

    if (mode == PsxCd_SeekMode::SET) {
        // Seek to an absolute position in the file: figure out the sector for the requested location within the file
        const int32_t fileStartSec = LIBCD_CdPosToInt(file.file.pos);
        const int32_t sectorInFile = ((offset < 0) ? offset + (CD_SECTOR_SIZE - 1) : offset) / CD_SECTOR_SIZE;
        const int32_t newDiscSector = fileStartSec + sectorInFile;
        LIBCD_CdIntToPos(newDiscSector, file.new_io_loc);

        // Figure out the offset within the destination sector we want to go to
        file.io_block_offset = (uint32_t)(offset - sectorInFile * CD_SECTOR_SIZE);
    } 
    else if (mode == PsxCd_SeekMode::CUR) {
        // Seek relative to the current IO position: figure out the sector for the requested relative offset
        const int32_t curIoSec = LIBCD_CdPosToInt(*gPSXCD_cur_io_loc);
        const int32_t secOffset = (file.io_block_offset + offset) / CD_SECTOR_SIZE;
        const int32_t newDiscSector = curIoSec + secOffset;
        LIBCD_CdIntToPos(newDiscSector, file.new_io_loc);
        
        // Figure out the offset within the destination sector we want to go to
        file.io_block_offset = ((uint32_t)(file.io_block_offset + offset)) % CD_SECTOR_SIZE;
    }
    else {
        // Seek relative to the end: figure out the disc sector to go to for the requested location within the file
        const int32_t fileStartSec = LIBCD_CdPosToInt(file.file.pos);
        const int32_t sectorInFile = (file.file.size - offset) / CD_SECTOR_SIZE;
        const int32_t newDiscSector = fileStartSec + sectorInFile;
        LIBCD_CdIntToPos(newDiscSector, file.new_io_loc);

        // Figure out the offset within the destination sector we want to go to
        file.io_block_offset = ((uint32_t)(file.file.size - offset) % CD_SECTOR_SIZE);
    }

    // Return '0' for success - this never actually fails for the retail game.
    // All we are doing here really is modifying the file datastructure.
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the current IO offset within the given file
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_tell(PsxCd_File& file) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    #if PC_PSX_DOOM_MODS
        if (ModMgr::isFileOverriden(file)) {
            return ModMgr::tellForOverridenFile(file);
        }
    #endif

    // Is this a real file descriptor or just a dummy one?
    // If it's real figure out the current io offset within the file, otherwise just return '0':
    if (file.file.pos != 0) {
        const int32_t curSec = LIBCD_CdPosToInt(file.new_io_loc);
        const int32_t fileStartSec = LIBCD_CdPosToInt(file.file.pos);
        const int32_t sectorInFile = curSec - fileStartSec;
        const int32_t curOffset = sectorInFile * CD_SECTOR_SIZE + file.io_block_offset;
        return curOffset;
    } else {
        return 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Placeholder for 'closing' a cd file - didn't need to do anything for retail PSX DOOM.
// In the retail .exe an open cd file is simply a struct describing the current IO location, filename etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_close([[maybe_unused]] PsxCd_File& file) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    #if PC_PSX_DOOM_MODS
        if (ModMgr::isFileOverriden(file)) {
            ModMgr::closeOverridenFile(file);
            return;
        }
    #endif

    // Nothing to do if this is an ordinary game file...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Switches the cdrom into audio playback mode if in data mode
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_set_audio_mode() noexcept {
    if (*gPSXCD_psxcd_mode != 0) {
        // Currently in data mode: cancel any async reads
        if (*gbPSXCD_async_on) {
            psxcd_async_read_cancel();
        }

        // No longer know what position the cdrom is on for data reading
        *gbPSXCD_init_pos = false;

        // Switch to audio mode
        gPSXCD_cd_param[0] = CdlModeRept | CdlModeAP | CdlModeDA;
        *gPSXCD_cdl_com = CdlSetmode;
        LIBCD_CdControl(CdlSetmode, gPSXCD_cd_param.get(), nullptr);
        *gPSXCD_psxcd_mode = 0;

        // Finish up the last command and pause
        psxcd_sync();
        *gPSXCD_cdl_com = CdlPause;
        LIBCD_CdControl(CdlPause, nullptr, nullptr);
        psxcd_sync();

        // Cancel any other commands in flight
        LIBCD_CdFlush();
    } else {
        // Already in audio mode: just complete any pending commands
        psxcd_sync();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the volume to playback cd audio when looping around again
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_set_loop_volume(const int32_t vol) noexcept {
    *gPSXCD_loopvol = vol;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the given cd track and loop another track afterwards using the specified parameters.
//
//  track:              The track to play
//  vol:                Track volume
//  sectorOffset:       To start past the normal track start
//  fadeUpTime:         Milliseconds to fade in the track, or '0' if instant play
//  loopTrack:          What track to play in loop after this track ends
//  loopVol:            What volume to play that looped track at
//  loopSectorOffest:   What sector offset to use for the looped track
//  loopFadeUpTime:     Fade up time for the looped track
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_play_at_andloop(
    const int32_t track,
    const int32_t vol,
    const int32_t sectorOffset,
    const int32_t fadeUpTime,
    const int32_t loopTrack,
    const int32_t loopVol,
    const int32_t loopSectorOffest,
    const int32_t loopFadeUpTime
) noexcept {
    // If this is an invalid track then do nothing
    const CdlLOC& trackLoc = gTrackCdlLOC[track];

    if (trackLoc == 0)
        return;

    // Ensure we are in audio mode before beginning playback
    psxcd_set_audio_mode();

    // Figure out the position to begin playback from
    const int32_t trackBegSector = LIBCD_CdPosToInt(trackLoc);
    LIBCD_CdIntToPos(trackBegSector + sectorOffset, *gPSXCD_cdloc);

    // Init playback stats
    *gbPSXCD_playflag = true;
    *gbPSXCD_loopflag = true;    
    *gPSXCD_playvol = vol;
    *gPSXCD_playfadeuptime = fadeUpTime;
    *gPSXCD_playcount = 0;
    
    // Save the parameters for when we loop
    *gPSXCD_looptrack = loopTrack;
    *gPSXCD_loopvol = loopVol;
    *gPSXCD_loopsectoroffset = loopSectorOffest;
    *gPSXCD_loopfadeuptime = loopFadeUpTime;

    // Seek to the track start
    *gbPSXCD_seeking_for_play = true;
    *gPSXCD_cdl_com = CdlSeekP;
    LIBCD_CdControlF(CdlSeekP, (const uint8_t*) gPSXCD_cdloc.get());

    // Remember the track start location as the begin, current and last valid intended location.
    //
    // This looks like a BUG in the original code to me? Should it not be saving 'gPSXCD_cdloc'?
    // Otherwise the sector offset specified is forgotten about...
    *gPSXCD_newloc = trackLoc;
    *gPSXCD_lastloc = trackLoc;
    *gPSXCD_beginloc = trackLoc;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begin playint the specified cd track at the given volume level.
// A sector offset can also be specified to begin from a certain location in the track.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_play_at(const int32_t track, const int32_t vol, const int32_t sectorOffset) noexcept {
    // If this is an invalid track then do nothing
    const CdlLOC& trackLoc = gTrackCdlLOC[track];

    if (trackLoc == 0)
        return;
    
    // Ensure we are in audio mode before beginning playback
    *gbPSXCD_playflag = false;
    *gbPSXCD_loopflag = false;
    *gbPSXCD_seeking_for_play = 0;
    psxcd_set_audio_mode();

    // Figure out the position to begin playback from
    const int32_t trackBegSector = LIBCD_CdPosToInt(trackLoc);
    LIBCD_CdIntToPos(trackBegSector + sectorOffset, *gPSXCD_cdloc);

    // Init some playback stats and begin seeking to the specified track location
    *gbPSXCD_playflag = true;
    *gPSXCD_playvol = vol;
    *gbPSXCD_seeking_for_play = true;
    *gPSXCD_playcount = 0;
    *gbPSXCD_loopflag = false;      // Don't loop
    *gPSXCD_playfadeuptime = 0;     // Don't fade in

    *gPSXCD_cdl_com = CdlSeekP;
    LIBCD_CdControlF(CdlSeekP, (const uint8_t*) gPSXCD_cdloc.get());

    // Remember the track start location as the begin, current and last valid intended location.
    //
    // This looks like a BUG in the original code to me? Should it not be saving 'gPSXCD_cdloc'?
    // Otherwise the sector offset specified is forgotten about...
    *gPSXCD_newloc = trackLoc;
    *gPSXCD_lastloc = trackLoc;
    *gPSXCD_beginloc = trackLoc;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the given audio track at the specified volume level
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_play(const int32_t track, const int32_t vol) noexcept {
    psxcd_play_at(track, vol, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek to the specified offset within a cd audio track in preparation for playback
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_seek_for_play_at(const int32_t track, const int32_t sectorOffset) noexcept {
    // If this is an invalid track then do nothing
    const CdlLOC& trackLoc = gTrackCdlLOC[track];

    if (trackLoc == 0)
        return;

    // Ensure we are in audio mode before doing the seek
    *gbPSXCD_playflag = false;
    *gbPSXCD_loopflag = false;
    *gbPSXCD_seeking_for_play = false;
    psxcd_set_audio_mode();

    // Figure out the position to go to
    const uint32_t trackBegSector = LIBCD_CdPosToInt(trackLoc);
    LIBCD_CdIntToPos(trackBegSector + sectorOffset, *gPSXCD_cdloc);

    // Init some playback stats and begin seeking to the specified track location    
    *gbPSXCD_playflag = false;
    *gPSXCD_playcount = 0;    
    *gbPSXCD_loopflag = false;
    *gbPSXCD_seeking_for_play = true;

    *gPSXCD_cdl_com = CdlSeekP;
    LIBCD_CdControlF(CdlSeekP, (const uint8_t*) &gPSXCD_cdloc);

    // Remember the track start location as the begin, current and last valid intended location.
    //
    // This looks like a BUG in the original code to me? Should it not be saving 'gPSXCD_cdloc'?
    // Otherwise the sector offset specified is forgotten about...
    *gPSXCD_newloc = trackLoc;
    *gPSXCD_lastloc = trackLoc;
    *gPSXCD_beginloc = trackLoc;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek to the beginning of a cd audio track in preparation for playback
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_seek_for_play(const int32_t track) noexcept {
    psxcd_seek_for_play_at(track, 0);
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
    _thunk_LIBCD_CdSync();
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop playback of cd audio.
// Unlike 'psxcd_pause' playback CANNOT be resumed by calling 'psxcd_restart' afterwards.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_stop() noexcept {
    // No longer playing anything
    *gbPSXCD_playflag = false;
    *gbPSXCD_loopflag = false;
    *gbPSXCD_seeking_for_play = false;

    // Unlike 'psxcd_pause' DON'T remember where to resume playback from, this is a 'stop' command
    *gPSXCD_lastloc = 0;

    // Quickly fade out cd audio if playing
    const int32_t startCdVol = psxspu_get_cd_vol();

    if (startCdVol != 0) {
        psxspu_start_cd_fade(FADE_TIME_MS, 0);

        while (psxspu_get_cd_fade_status()) {
            // Wait for the fade to complete...
            // TODO: PC-PSX: yield CPU cycles here and handle window events if pausing.
        }
    }

    // Ensure no active commands, and issue the pause command
    psxcd_sync();

    *gbPSXCD_waiting_for_pause = true;
    *gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControlF(CdlPause, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pause cd audio playback and make a note of where we paused at
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_pause() noexcept {
    // No longer playing anything
    *gbPSXCD_playflag = false;
    *gbPSXCD_seeking_for_play = false;

    // If we did not start playback of anything (never set a sector location) then we don't need to do anything else
    if (*gPSXCD_lastloc == 0)
        return;

    // Save the last location we read from so we can restore in 'psxcd_restart'
    *gPSXCD_lastloc = *gPSXCD_newloc;

    // Quickly fade out cd audio if playing
    const int32_t startCdVol = psxspu_get_cd_vol();

    if (startCdVol != 0) {
        psxspu_start_cd_fade(FADE_TIME_MS, 0);

        while (psxspu_get_cd_fade_status()) {
            // Wait for the fade to complete...
            // TODO: PC-PSX: yield CPU cycles here and handle window events if pausing.
        }
    }
    
    // Ensure no active commands, and issue the pause command
    psxcd_sync();

    *gbPSXCD_waiting_for_pause = true;
    *gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControlF(CdlPause, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Restart cd audio playback: playback resumes from where the cd was last paused
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_restart(const int32_t vol) noexcept {
    // Only do the restart if we had paused previously (have a saved sector position)
    if (*gPSXCD_lastloc == 0)
        return;

    // Switch to audio mode
    psxcd_set_audio_mode();

    // Reset all these variables
    *gPSXCD_cdloc = *gPSXCD_lastloc;
    *gbPSXCD_playflag = true;
    *gbPSXCD_seeking_for_play = true;
    *gPSXCD_playcount = 0;
    *gPSXCD_playvol = vol;

    // Seek to the last intended cd location
    *gPSXCD_cdl_com = CdlSeekP;        
    LIBCD_CdControlF(CdlSeekP, (const uint8_t*) gPSXCD_cdloc.get());
}

void psxcd_elapsed_sectors() noexcept {
    // FIXME: returning 1 so some loops don't get stuck
    v0 = 1;
    return;

loc_80040EFC:
    v0 = lw(gp + 0x818);                                // Load from: gPSXCD_beginloc (80077DF8)
    sp -= 0x18;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_80040F38;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF0;                                       // Result = gPSXCD_newloc (80077DF0)
    _thunk_LIBCD_CdPosToInt();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7DF8;                                       // Result = gPSXCD_beginloc (80077DF8)
    s0 = v0;
    _thunk_LIBCD_CdPosToInt();
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Set whether cd audio is mixed as stereo or mono
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_set_stereo(const bool bStereo) noexcept {
    if (bStereo) {
        gPSXCD_cdatv->l_to_l = 127;
        gPSXCD_cdatv->l_to_r = 0;
        gPSXCD_cdatv->r_to_r = 127;
        gPSXCD_cdatv->r_to_l = 0;
    } else {
        gPSXCD_cdatv->l_to_l = 63;
        gPSXCD_cdatv->l_to_r = 63;
        gPSXCD_cdatv->r_to_r = 63;
        gPSXCD_cdatv->r_to_l = 63;
    }

    LIBCD_CdMix(*gPSXCD_cdatv);
}
