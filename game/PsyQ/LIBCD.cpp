//------------------------------------------------------------------------------------------------------------------------------------------
// Module containing a partial reimplementation of the PSY-Q 'LIBCD' library.
// These functions are not neccesarily faithful to the original code, and are reworked to make the game run in it's new environment.
//------------------------------------------------------------------------------------------------------------------------------------------

#include "LIBCD.h"

#include "LIBAPI.h"
#include "LIBETC.h"
#include "PcPsx/Finally.h"

BEGIN_THIRD_PARTY_INCLUDES

#include <device/cdrom/cdrom.h>
#include <device/spu/spu.h>
#include <disc/disc.h>

END_THIRD_PARTY_INCLUDES

// N.B: must be done LAST due to MIPS register macros
#include "PsxVm/PsxVm.h"

// CD-ROM constants
static constexpr int32_t CD_SECTORS_PER_SEC = 75;       // The number of CD sectors per second of audio
static constexpr int32_t CD_LEAD_SECTORS    = 150;      // How many sectors are assigned to the lead in track, which has the TOC for the disc

// Callbacks for when a command completes and when a data sector is ready
static CdlCB gpLIBCD_CD_cbsync;
static CdlCB gpLIBCD_CD_cbready;

// Result bytes for the most recent CD command
static uint8_t gLastCdCmdResult[8];

// Internal LIBCD init function prototypes
void LIBCD_CD_init() noexcept;
void LIBCD_CD_initvol() noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Call a libcd callback ('sync' or 'ready' callbacks)
//------------------------------------------------------------------------------------------------------------------------------------------
static void invokeCallback(
    const CdlCB callback,
    const CdlSyncStatus status,
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
    // Advance the cdrom emulation: this may result in a sector being read
    device::cdrom::CDROM& cdrom = *PsxVm::gpCdrom;
    const int32_t oldReadSector = cdrom.readSector;
    cdrom.step();

    // If we read a new sector then setup the data buffer fifo and let the callback know
    if (cdrom.readSector != oldReadSector) {
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
            if (pArgs) {
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the cdrom handling library: should be called before use
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBCD_CdInit() noexcept {
    // Note: the original code checked whether the reset operation succeeded here and retried up to 4x times.
    // In this emulated environment however we can simplify a bit and just assume that it works.
    LIBCD_CdReset(1);
    LIBCD_CdSyncCallback(nullptr);
    LIBCD_CdReadyCallback(nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reset the CD rom handling library.
// If mode is '1' then CD audio stuff is reset also, otherwise just the CD handling stuff is initialized.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBCD_CdReset(const int32_t mode) noexcept {
    LIBCD_CD_init();

    if (mode == 1) {
        LIBCD_CD_initvol();
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cancel the current cd command which is in-flight
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBCD_CdFlush() noexcept {
    // This doesn't need to do anything for this LIBCD reimplementation.
    // All commands are executed synchronously!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check for a cd command's completion or wait for it to complete.
// If 'mode' is '0' then that indicates 'wait for completion'.
//------------------------------------------------------------------------------------------------------------------------------------------
CdlSyncStatus LIBCD_CdSync([[maybe_unused]] const int32_t mode, uint8_t pResult[8]) noexcept {
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
CdlSyncStatus LIBCD_CdReady(const int32_t mode, uint8_t pResult[8]) noexcept {
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
            // No data: make a read of some happen immediately
            cdrom.bForceSectorRead = true;
            stepCdromWithCallbacks();
            ASSERT(!cdrom.isBufferEmpty());

            return CdlDataReady;
        }
    }
    else {
        // Just querying whether there is data or not.
        // Emulate the CD a little in case this is being polled in a loop and return the status.
        stepCdromWithCallbacks();
        return (!cdrom.isBufferEmpty()) ? CdlDataReady : CdlNoIntr;
    }
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Issue a command to the CDROM system, possibly with arguments and return the result in the given (optional) buffer.
// Returns 'true' if successful.
//
// In the original PsyQ SDK some of the commands would block, while others would not block.
// In this version however ALL commands are executed SYNCHRONOUSLY.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBCD_CdControl(const CdlCmd cmd, const uint8_t* const pArgs, uint8_t pResult[8]) noexcept {
    return handleCdCmd(cmd, pArgs, pResult);
}

void _thunk_LIBCD_CdControl() noexcept {
    v0 = LIBCD_CdControl((CdlCmd) a0, vmAddrToPtr<const uint8_t>(a1), vmAddrToPtr<uint8_t>(a2));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Issue a command to the CDROM system and return 'true' if successful.
//
// In the original PsyQ SDK this was similar to 'CdControl' except it was faster (though less safe) by bypassing some of the handshaking
// with the cdrom drive. In this reimplementation however it functions the same as the normal 'CdControl'.
// Note: all commands are also executed SYNCHRONOUSLY.
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBCD_CdControlF(const CdlCmd cmd, const uint8_t* const pArgs) noexcept {
    return handleCdCmd(cmd, pArgs, nullptr);
}

void _thunk_LIBCD_CdControlF() noexcept {
    v0 = LIBCD_CdControlF((CdlCmd) a0, vmAddrToPtr<const uint8_t>(a1));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the volume for CD audio going to the SPU
//------------------------------------------------------------------------------------------------------------------------------------------
bool LIBCD_CdMix(const CdlATV& vol) noexcept {
    device::cdrom::CDROM& cdrom = *PsxVm::gpCdrom;

    cdrom.volumeLeftToLeft = vol.l_to_l;
    cdrom.volumeLeftToRight = vol.l_to_r;
    cdrom.volumeRightToLeft = vol.r_to_l;
    cdrom.volumeRightToRight = vol.r_to_r;

    return true;    // Always successful
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the requested number of 32-bit words from the CDROM's sector data buffer.
// Returns 'true' if successful, which will be always.
//
// N.B: this function originally required that the dest pointer be 32-bit aligned, but I'm not requiring that for this reimplmentation.
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal initialization function: init the CDROM handling system
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBCD_CD_init() noexcept {
    // Clear callbacks
    gpLIBCD_CD_cbready = nullptr;
    gpLIBCD_CD_cbsync = nullptr;

    // Clear the data FIFO
    device::cdrom::CDROM& cdrom = *PsxVm::gpCdrom;

    cdrom.status.dataFifoEmpty = 0;
    cdrom.dataBuffer.clear();
    cdrom.dataBufferPointer = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal initialization function: sound stuff related to CD audio
//------------------------------------------------------------------------------------------------------------------------------------------
void LIBCD_CD_initvol() noexcept {
    // Set volume levels
    spu::SPU& spu = *PsxVm::gpSpu;

    if ((spu.mainVolume.left == 0) && (spu.mainVolume.right == 0)) {
        spu.mainVolume.left = 0x3FFF;
        spu.mainVolume.right = 0x3FFF;
    }
    
    spu.cdVolume.left = 0x3FFF;
    spu.cdVolume.right = 0x3FFF;

    // Set SPU status flags
    spu.control = {};
    spu.control.spuEnable = true;
    spu.control.unmute = true;
    spu.control.cdEnable = true;

    // Set the default CD to SPU mixing parameters
    CdlATV cdAudioVol = {};
    cdAudioVol.l_to_l = 128;
    cdAudioVol.r_to_r = 128;
    LIBCD_CdMix(cdAudioVol);
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
