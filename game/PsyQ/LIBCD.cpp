//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing a partial reimplementation of the PSY-Q 'LIBCD' library.
// These functions are not neccesarily faithful to the original code, and are reworked to make the game run in it's new environment.
//------------------------------------------------------------------------------------------------------------------------------------------

#include "LIBCD.h"

#include "LIBAPI.h"
#include "LIBC2.h"
#include "LIBETC.h"
#include "PcPsx/Finally.h"

BEGIN_THIRD_PARTY_INCLUDES

#include <device/cdrom/cdrom.h>
#include <disc/disc.h>

END_THIRD_PARTY_INCLUDES

// N.B: must be done LAST due to MIPS register macros
#include "PsxVm/PsxVm.h"

// CD-ROM constants
static constexpr int32_t CD_SECTORS_PER_SEC = 75;       // The number of CD sectors per second of audio
static constexpr int32_t CD_LEAD_SECTORS    = 150;      // How many sectors are assigned to the lead in track, which has the TOC for the disc

// This is the 'readcnt' cdrom amount that Avocado will read data on
static constexpr int AVOCADO_DATA_READ_CNT = 1150;

// Callbacks for when a command completes and when a data sector is ready
static CdlCB gpLIBCD_CD_cbsync;
static CdlCB gpLIBCD_CD_cbready;

// Result bytes for the most recent CD command
static uint8_t gLastCdCmdResult[8];

// These are local to this module: forward declare here
void LIBCD_EVENT_def_cbsync(const CdlStatus status, const uint8_t pResult[8]) noexcept;
void LIBCD_EVENT_def_cbready(const CdlStatus status, const uint8_t pResult[8]) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Call a libcd callback ('sync' or 'ready' callbacks)
//------------------------------------------------------------------------------------------------------------------------------------------
static void invokeCallback(
    const CdlCB callback,
    const CdlStatus status,
    const uint8_t resultBytes[8]
) noexcept {
    if (callback) {
        callback(status, resultBytes);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a byte from the cdrom's result fifo buffer
//------------------------------------------------------------------------------------------------------------------------------------------
static uint8_t readCdCmdResultByte() noexcept {
    device::cdrom::CDROM& cdrom = *PsxVm::gpCdrom;
    fifo<uint8_t, 16>& cmdResultFifo = cdrom.CDROM_response;
    uint8_t resultByte = 0;

    if (!cmdResultFifo.empty()) {
        resultByte = cmdResultFifo.get();

        if (cmdResultFifo.empty()) {
            cdrom.status.responseFifoEmpty = 0;
        }
    } else {
        cdrom.status.responseFifoEmpty = 0;
    }

    return resultByte;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Step the CD-ROM and invoke 'data ready' callbacks if a new sector was read
//------------------------------------------------------------------------------------------------------------------------------------------
static void stepCdromWithCallbacks() noexcept {
    // Determine if a new sector will be read
    device::cdrom::CDROM& cdrom = *PsxVm::gpCdrom;

    const bool bWillReadNewSector = (
        (cdrom.stat.read || cdrom.stat.play) &&
        (cdrom.readcnt == AVOCADO_DATA_READ_CNT)
    );

    // Clear the data buffers if a new sector is about to be read
    if (bWillReadNewSector) {
        cdrom.rawSector.clear();
        cdrom.dataBuffer.clear();
        cdrom.dataBufferPointer = 0;
        cdrom.status.dataFifoEmpty = 0;
    }

    // Advance the cdrom emulation
    cdrom.step();

    // If we read a new sector then setup the data buffer fifo and let the callback know
    if (bWillReadNewSector) {
        ASSERT(!cdrom.rawSector.empty());
        cdrom.dataBuffer = cdrom.rawSector;
        cdrom.dataBufferPointer = 0;
        cdrom.status.dataFifoEmpty = 1;
        
        invokeCallback(gpLIBCD_CD_cbready, CdlDataReady, gLastCdCmdResult);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handle executing a command to the cdrom drive.
// Note: only a subset of the available commands are supported, just the ones needed for DOOM.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool handleCdCmd(const CdlCmd cmd, const uint8_t* const pArgs, uint8_t resultBytesOut[8]) noexcept {    
    device::cdrom::CDROM& cdrom = *PsxVm::gpCdrom;

    // Save the result globally and for the caller on exit
    uint8_t cmdResult[8] = {};
    static_assert(sizeof(cmdResult) == sizeof(gLastCdCmdResult));

    const auto exitActions = finally([&](){
        std::memcpy(gLastCdCmdResult, cmdResult, sizeof(cmdResult));

        if (resultBytesOut) {
            std::memcpy(resultBytesOut, cmdResult, sizeof(cmdResult));
        }
    });

    // Handle the command    
    fifo<uint8_t, 16>& cdparams = cdrom.CDROM_params;
    
    switch (cmd) {
        case CdlPause:
            cdrom.handleCommand(cmd);
            cmdResult[0] = readCdCmdResultByte();   // Status bits
            invokeCallback(gpLIBCD_CD_cbsync, CdlComplete, cmdResult);
            return true;

        case CdlSetloc:
            cdparams.add(pArgs[0]);     // CdlLOC.minute
            cdparams.add(pArgs[1]);     // CdlLOC.second
            cdparams.add(pArgs[2]);     // CdlLOC.sector
            cdrom.handleCommand(cmd);
            cmdResult[0] = readCdCmdResultByte();   // Status bits

            // Goto the desired sector immediately and invoke the callback
            cdrom.readSector = cdrom.seekSector;
            invokeCallback(gpLIBCD_CD_cbsync, CdlComplete, cmdResult);
            return true;

        case CdlSeekP:
            if (pArgs) {
                // Set the current read/seek position if specified
                disc::Position pos;
                pos.mm = bcd::toBinary(pArgs[0]);   // CdlLOC.minute
                pos.ss = bcd::toBinary(pArgs[1]);   // CdlLOC.second
                pos.ff = bcd::toBinary(pArgs[2]);   // CdlLOC.sector

                const int sector = pos.toLba();
                cdrom.seekSector = sector;
                cdrom.readSector = sector;
            }

            cdrom.handleCommand(cmd);
            cmdResult[0] = readCdCmdResultByte();   // Status bits
            cmdResult[1] = readCdCmdResultByte();   // Location: minute (BCD)
            cmdResult[2] = readCdCmdResultByte();   // Location: second (BCD)
            invokeCallback(gpLIBCD_CD_cbsync, CdlComplete, cmdResult);
            return true;

        case CdlSetmode:
            cdparams.add(pArgs[0]);                 // Mode
            cdrom.handleCommand(cmd);
            cmdResult[0] = readCdCmdResultByte();   // Status bits
            invokeCallback(gpLIBCD_CD_cbsync, CdlComplete, cmdResult);
            return true;

        case CdlReadN:
        case CdlReadS:
            // Set the location to read and issue the read command
            if (pArgs) {
                cdparams.add(pArgs[0]);     // CdlLOC.minute
                cdparams.add(pArgs[1]);     // CdlLOC.second
                cdparams.add(pArgs[2]);     // CdlLOC.sector
            }

            cdrom.handleCommand(cmd);
            cmdResult[0] = readCdCmdResultByte();   // Status bits
            invokeCallback(gpLIBCD_CD_cbsync, CdlComplete, cmdResult);

            // Set the read location immediately
            {
                const uint32_t minute = bcd::toBinary(pArgs[0]);
                const uint32_t second = bcd::toBinary(pArgs[1]);
                const uint32_t sector = bcd::toBinary(pArgs[2]);

                cdrom.seekSector = sector + (second * 75) + (minute * 60 * 75);
                cdrom.readSector = cdrom.seekSector;
            }
            
            return true;

        case CdlPlay:
            if (pArgs) {
                cdparams.add(pArgs[0]);     // CdlLOC.track
            }

            cdrom.handleCommand(cmd);
            cmdResult[0] = readCdCmdResultByte();   // Status bits
            return true;

        default:
            ASSERT_FAIL("Unhandled or unknown cd command!");
            break;
    }

    return false;
}

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

    LIBCD_CdSyncCallback(LIBCD_EVENT_def_cbsync);
    LIBCD_CdReadyCallback(LIBCD_EVENT_def_cbready);

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

void LIBCD_EVENT_def_cbsync([[maybe_unused]] const CdlStatus status, [[maybe_unused]] const uint8_t pResult[8]) noexcept {
    // TODO: remove/replace this
    a0 = 0xF0000003;
    a1 = 0x20;
    LIBAPI_DeliverEvent();
}

void LIBCD_EVENT_def_cbready([[maybe_unused]] const CdlStatus status, [[maybe_unused]] const uint8_t pResult[8]) noexcept {
    // TODO: remove/replace this
    a0 = 0xF0000003;
    a1 = 0x40;
    LIBAPI_DeliverEvent();
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Cancel the current cd command which is in-flight
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBCD_CdFlush() noexcept {
    // This doesn't need to do anything for this LIBCD reimplementation.
    // All commands are executed synchronously!
    
    // TODO: REMOVE
    #if 0
loc_80054C78:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    LIBCD_CD_flush();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check for a cd command's completion or wait for it to complete.
// If 'mode' is '0' then that indicates 'wait for completion'.
//------------------------------------------------------------------------------------------------------------------------------------------
CdlStatus LIBCD_CdSync([[maybe_unused]] const int32_t mode, uint8_t pResult[8]) noexcept {
    // Just step the CDROM a little in case this is being polled
    stepCdromWithCallbacks();

    // Give the caller the result of the last cd operation if required
    if (pResult) {
        std::memcpy(pResult, gLastCdCmdResult, sizeof(gLastCdCmdResult));
    }

    return CdlComplete;
}

void _thunk_LIBCD_CdSync() noexcept {
    v0 = LIBCD_CdSync(a0, vmAddrToPtr<uint8_t>(a1));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for cdrom data to be ready or check if it is ready.
// Mode '0' means block until data is ready, otherwise we simply return the current status.
//------------------------------------------------------------------------------------------------------------------------------------------
CdlStatus LIBCD_CdReady(const int32_t mode, uint8_t pResult[8]) noexcept {
    device::cdrom::CDROM& cdrom = *PsxVm::gpCdrom;

    const auto exitActions = finally([&](){
        // Copy the last command result on exit if the caller wants it
        if (pResult) {
            std::memcpy(pResult, gLastCdCmdResult, sizeof(gLastCdCmdResult));
        }
    });

    if (mode == 0) {
        // Block until there is data: see if there is some
        if (!cdrom.isBufferEmpty()) {
            return CdlDataReady;
        } else {
            // No data: issue a command to read some
            ASSERT_FAIL("TODO");
            return CdlDataReady;
        }
    }
    else {
        // Just querying whether there is data or not.
        // Emulate the CD a little in case this is being polled in a loop and return the status.
        stepCdromWithCallbacks();
        return (!cdrom.isBufferEmpty()) ? CdlDataReady : CdlNoIntr;
    }

    // TODO: REMOVE THIS
    #if 0
    // Speed up the emulation of the CD
    #if PC_PSX_DOOM_MODS
        emulate_cdrom();
    #endif

    LIBCD_CD_ready();
    #endif
}

void _thunk_LIBCD_CdReady() noexcept {
    v0 = LIBCD_CdReady(a0, vmAddrToPtr<uint8_t>(a1));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the callback for when a cd command completes and return the previous one
//------------------------------------------------------------------------------------------------------------------------------------------
CdlCB LIBCD_CdSyncCallback(const CdlCB syncCallback) noexcept {
    const CdlCB oldCallback = gpLIBCD_CD_cbsync;
    gpLIBCD_CD_cbsync = syncCallback;
    return oldCallback;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the callback for when a cd data sector is ready and return the previous one
//------------------------------------------------------------------------------------------------------------------------------------------
CdlCB LIBCD_CdReadyCallback(const CdlCB readyCallback) noexcept {
    const CdlCB oldCallback = gpLIBCD_CD_cbready;
    gpLIBCD_CD_cbready = readyCallback;
    return oldCallback;
}

bool LIBCD_CdControl(const CdlCmd cmd, const uint8_t* const pArgs, uint8_t pResult[8]) noexcept {
    return handleCdCmd(cmd, pArgs, pResult);

    // TODO : REMOVE
    #if 0
    a0 = cmd;
    a1 = ptrToVmAddr(pArgs);
    a2 = ptrToVmAddr(pResult);

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
    
    const CdlCB oldCbSync = gpLIBCD_CD_cbsync;

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
    gpLIBCD_CD_cbsync = nullptr;
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
    gpLIBCD_CD_cbsync = oldCbSync;
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
    gpLIBCD_CD_cbsync = oldCbSync;
    s7 = -1;                                            // Result = FFFFFFFF
    v0 = s7 + 1;                                        // Result = 00000000
loc_80054E90:
    ra = lw(sp + 0x30);
    s7 = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x38;
    return (v0 != 0);
    #endif
}

void _thunk_LIBCD_CdControl() noexcept {
    v0 = LIBCD_CdControl((CdlCmd) a0, vmAddrToPtr<const uint8_t>(a1), vmAddrToPtr<uint8_t>(a2));
}

bool LIBCD_CdControlF(const CdlCmd cmd, const uint8_t* const pArgs) noexcept {
    return handleCdCmd(cmd, pArgs, nullptr);

    // TODO : REMOVE
    #if 0
    a0 = cmd;
    a1 = ptrToVmAddr(pArgs);

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

    const CdlCB oldCbSync = gpLIBCD_CD_cbsync;

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
    gpLIBCD_CD_cbsync = nullptr;
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
    gpLIBCD_CD_cbsync = oldCbSync;
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
    gpLIBCD_CD_cbsync = oldCbSync;
    s6 = -1;                                            // Result = FFFFFFFF
    v0 = s6 + 1;                                        // Result = 00000000
loc_80054FA0:
    ra = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x30;
    return (v0 != 0);
    #endif
}

void _thunk_LIBCD_CdControlF() noexcept {
    v0 = LIBCD_CdControlF((CdlCmd) a0, vmAddrToPtr<const uint8_t>(a1));
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the requested number of 32-bit words from the CDROM's sector data buffer.
// Returns 'true' if successful, which will be always.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBCD_CdGetSector(void* const pDst, const int32_t readSizeInWords) noexcept {
    device::cdrom::CDROM& cdrom = *PsxVm::gpCdrom;
    
    // Figure out the size in bytes we want (request is for words)
    const int32_t readSizeInBytes = readSizeInWords * sizeof(uint32_t);
    uint8_t* const pDstBytes = (uint8_t*) pDst;

    // The previous PSYQ code wrote '0x80' 'to 0x1F801803' (The 'CD Request' register).
    // This means that we want data and if the data FIFO is at the end then we should reset it.
    if (cdrom.isBufferEmpty()) {
        cdrom.dataBuffer = cdrom.rawSector;
        cdrom.dataBufferPointer = 0;
        cdrom.status.dataFifoEmpty = 1;
    }

    // From Avocado, figure out the offset of the data in the sector buffer. 12 bytes are for sector header.
    //  Mode 0 - 2048 byte sectors
    //  Mode 1 - 2340 byte sectors (no sync bytes also)
    //
    const int32_t dataStart = (cdrom.mode.sectorSize == 0) ? 24 : 12;

    // Read the bytes: note that reading past the end of the buffer on the real hardware would do the following:
    //  "The PSX will repeat the byte at index [800h-8] or [924h-4] as padding value."
    // See: 
    //  https://problemkaputt.de/psx-spx.htm#cdromcontrollerioports
    //
    const int32_t sectorSize = (cdrom.mode.sectorSize == 0) ? 2048 : 2340;
    const int32_t sectorBytesLeft = (cdrom.dataBufferPointer < sectorSize) ? sectorSize - cdrom.dataBufferPointer : 0;

    const uint8_t* const pSrcBytes = cdrom.dataBuffer.data() + cdrom.dataBufferPointer + dataStart;

    if (readSizeInBytes <= sectorBytesLeft) {
        // Usual case: there is enough data in the cdrom buffer: just do a memcpy and move along the pointer
        std::memcpy(pDstBytes, pSrcBytes, readSizeInBytes);
        cdrom.dataBufferPointer += readSizeInBytes;
    }
    else {
        // Note enough bytes in the FIFO to complete the read, will read what we can then use the repeated byte value for the rest
        std::memcpy(pDstBytes, pSrcBytes, sectorBytesLeft);
        cdrom.dataBufferPointer = sectorSize;

        const std::uint8_t fillByte = (cdrom.mode.sectorSize == 0) ?
            cdrom.dataBuffer[dataStart + sectorSize - 8] :
            cdrom.dataBuffer[dataStart + sectorSize - 4];

        const int32_t bytesToFill = readSizeInBytes - sectorBytesLeft;
        std::memset(pDstBytes + sectorBytesLeft, fillByte, bytesToFill);
    }

    // Set this flag if the CDROM data FIFO has been drained
    if (cdrom.dataBufferPointer >= sectorSize) {
        cdrom.status.dataFifoEmpty = 0;
    }

    // No emulated command was issued for this read, clear the params and reponse FIFOs
    cdrom.CDROM_params.clear();
    cdrom.CDROM_response.clear();

    return true;    // According to the PsyQ docs this always returns '1' or 'true' for success (never fails)
}

void _thunk_LIBCD_CdGetSector() noexcept {
    v0 = LIBCD_CdGetSector(vmAddrToPtr<void>(a0), a1);
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
    if (gpLIBCD_CD_cbready == nullptr) goto loc_80055A70;
    a0 = lbu(s4);                                       // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    gpLIBCD_CD_cbready((CdlStatus) a0, vmAddrToPtr<const uint8_t>(a1));
loc_80055A70:
    v0 = s0 & 2;
loc_80055A74:
    if (v0 == 0) goto loc_80055A2C;
    if (gpLIBCD_CD_cbsync == nullptr) goto loc_80055A2C;
    a0 = lbu(s2);                                       // Load from: 800774CD
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    gpLIBCD_CD_cbsync((CdlStatus) a0, vmAddrToPtr<const uint8_t>(a1));
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
    if (gpLIBCD_CD_cbready == nullptr) goto loc_80055CEC;
    a0 = lbu(s6);                                       // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    gpLIBCD_CD_cbready((CdlStatus) a0, vmAddrToPtr<const uint8_t>(a1));
loc_80055CEC:
    v0 = s0 & 2;
loc_80055CF0:
    if (v0 == 0) goto loc_80055CA8;
    if (gpLIBCD_CD_cbsync == nullptr) goto loc_80055CA8;
    a0 = lbu(s2);                                       // Load from: 800774CD
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)
    gpLIBCD_CD_cbsync((CdlStatus) a0, vmAddrToPtr<const uint8_t>(a1));
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
                            if (gpLIBCD_CD_cbready) {
                                a0 = lbu(s4);               // Load from: 800774CE
                                a1 = 0x80086110;            // Result = LIBCD_BIOS_result[2] (80086110)
                                gpLIBCD_CD_cbready((CdlStatus) a0, vmAddrToPtr<const uint8_t>(a1));
                            }
                        }

                        v0 = s0 & 2;

                        if (v0 == 0)
                            continue;
                        
                        if (gpLIBCD_CD_cbsync == nullptr)
                            continue;

                        a0 = lbu(s2);               // Load from: 800774CD
                        a1 = 0x80086108;            // Result = LIBCD_BIOS_result[0] (80086108)
                        gpLIBCD_CD_cbsync((CdlStatus) a0, vmAddrToPtr<const uint8_t>(a1));
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
    const CdlCB oldCbSync = gpLIBCD_CD_cbsync;
    v0 = a1 & 0x10;
    if (v0 == 0) goto loc_80056318;
loc_800562C8:
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x2154;                                       // Result = STR_Sys_CD_open_Msg[0] (80012154)
    gpLIBCD_CD_cbsync = nullptr;
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
    gpLIBCD_CD_cbsync = oldCbSync;
    v0 = a1 & 0x10;
    if (v0 != 0) goto loc_800562C8;
loc_80056318:
    v0 = 0;                                             // Result = 00000000
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

    a0 = 0x80012168;            // Result = STR_Sys_CD_init_Msg[0] (80012168)
    a1 = 0x800774F8;            // Result = 800774F8
    LIBC2_printf();

    v1 = 0x800774D0;            // Result = 800774D0
    v0 = 9;
    a0 = -1;

    gpLIBCD_CD_cbready = nullptr;
    gpLIBCD_CD_cbsync = nullptr;

    sw(0, 0x80077204);          // Store to: gLIBCD_CD_status (80077204)

    do {
        sw(0, v1);
        v0--;
        v1 += 4;
    } while (v0 != a0);

    // TODO: REMOVE THIS
    v0 = 0;

    #if 0
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
    const CdlCB oldCbSync = gpLIBCD_CD_cbsync;
    v0 = a1 & 0x10;

    while (v0 != 0) {
        a0 = 0x80012154;                // Result = STR_Sys_CD_open_Msg[0] (80012154)
        gpLIBCD_CD_cbsync = nullptr;
        LIBC2_printf();

        a0 = 1;
        a1 = 0;
        a2 = 0;
        a3 = 0;
        LIBCD_CD_cw();

        a0 = 0x3C;
        _thunk_LIBETC_VSync();

        a1 = lw(0x80077204);            // Load from: gLIBCD_CD_status (80077204)
        gpLIBCD_CD_cbsync = oldCbSync;
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

    #endif

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
    if (gpLIBCD_CD_cbready == nullptr) goto loc_8005708C;
    a0 = lbu(s2);                                       // Load from: 800774CE
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6110;                                       // Result = LIBCD_BIOS_result[2] (80086110)
    gpLIBCD_CD_cbready((CdlStatus) a0, vmAddrToPtr<const uint8_t>(a1));
loc_8005708C:
    v0 = s0 & 2;
loc_80057090:
    if (v0 == 0) goto loc_80057048;
    if (gpLIBCD_CD_cbsync == nullptr) goto loc_80057048;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x74CD;                                       // Result = 800774CD
    a0 = lbu(v0);                                       // Load from: 800774CD
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x6108;                                       // Result = LIBCD_BIOS_result[0] (80086108)    
    gpLIBCD_CD_cbsync((CdlStatus) a0, vmAddrToPtr<const uint8_t>(a1));
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
