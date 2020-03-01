#pragma once

#include "PsxVm/VmPtr.h"

// Maximum number of tracks on a CD
static constexpr int32_t CdlMAXTOC = 100;

// Describes a track location or file position on a CD.
// The location is described in audio terms.
struct CdlLOC {
    uint8_t minute;     // Note: in binary coded decimal form
    uint8_t second;     // Note: in binary coded decimal form
    uint8_t sector;     // Note: in binary coded decimal form
    uint8_t track;      // Unused in this PsyQ SDK version: normally '0'
    
    // Conversion to and from a single 32-bit int
    void operator = (const uint32_t loc32) noexcept {
        minute = (uint8_t)(loc32 >> 0);
        second = (uint8_t)(loc32 >> 8);
        sector = (uint8_t)(loc32 >> 16);
        track = (uint8_t)(loc32 >> 24);
    }

    inline operator uint32_t () const noexcept {
        return (
            ((uint32_t) minute << 0 ) |
            ((uint32_t) second << 8 ) |
            ((uint32_t) sector << 16) |
            ((uint32_t) track  << 24)
        );
    }
};

static_assert(sizeof(CdlLOC) == 4);

// Describes a file on a CD, including filename, location on disc and size
struct CdlFILE {
    CdlLOC      pos;
    uint32_t    size;
    char        name[16];
};

static_assert(sizeof(CdlFILE) == 24);

// Enum representing a command code issued to the CDROM
enum CdlCmd : uint8_t {
    CdlNop          = 0x01,     // NOP
    CdlSetloc       = 0x02,     // Set sector location (seek)
    CdlPlay         = 0x03,     // Play CD audio
    CdlForward      = 0x04,     // Fast forward audio
    CdlBackward     = 0x05,     // Rewind audio
    CdlReadN        = 0x06,     // Read some data (with retry)
    CdlStandby      = 0x07,     // Wait but continue to rotate the disc
    CdlStop         = 0x08,     // Stop the disc from spinning completely
    CdlPause        = 0x09,     // Stop the cdrom at the current location
    CdlMute         = 0x0B,     // Mute CD audio
    CdlDemute       = 0x0C,     // Un-mute CD audio
    CdlSetfilter    = 0x0D,     // Select an ADPCM audio sector to play
    CdlSetmode      = 0x0E,     // Set the current mode for the CD (audio or data)
    CdlGetparam     = 0x0F,     // Get the current mode for the CD (audio or data)
    CdlGetlocL      = 0x10,     // Get current logical disc location (data sector)
    CdlGetlocP      = 0x11,     // Get current physical disc location (audio sector)
    CdlGetTN        = 0x13,     // Get the number of tracks in the disc
    CdlGetTD        = 0x14,     // Get location information for a track on the disc
    CdlSeekL        = 0x15,     // Seek to a logical sector (data sector)
    CdlSeekP        = 0x16,     // Seek to a phyiscal sector (audio sector)
    CdlReadS        = 0x1B,     // Read some data (no retry)
};

// Status code sent to callback functions and returned by 'LIBCD_CdSync'
enum CdlStatus : uint8_t {
    CdlNoIntr       = 0x00,     // No result or currently executing a cd operation
    CdlDataReady    = 0x01,     // There is data ready for reading
    CdlComplete     = 0x02,     // The cd operation completed successfully
    CdlAcknowledge  = 0x03,     // Unused/reserved
    CdlDataEnd      = 0x04,     // Reached the end of data
    CdlDiskError    = 0x05      // An error occurred
};

// Callback type for 'CdReadyCallback', and 'CdSyncCallback'
typedef void (*CdlCB)(const CdlStatus status, const uint8_t pResult[8]);

void LIBCD_CdInit() noexcept;
void LIBCD_CdReset() noexcept;
void LIBCD_CdFlush() noexcept;

CdlStatus LIBCD_CdSync([[maybe_unused]] const int32_t mode, uint8_t pResult[8]) noexcept;
void _thunk_LIBCD_CdSync() noexcept;

CdlStatus LIBCD_CdReady(const int32_t mode, uint8_t pResult[8]) noexcept;
void _thunk_LIBCD_CdReady() noexcept;

CdlCB LIBCD_CdSyncCallback(const CdlCB syncCallback) noexcept;
CdlCB LIBCD_CdReadyCallback(const CdlCB readyCallback) noexcept;

bool LIBCD_CdControl(const CdlCmd cmd, const uint8_t* const pArgs, uint8_t pResult[8]) noexcept;
void _thunk_LIBCD_CdControl() noexcept;

bool LIBCD_CdControlF(const CdlCmd cmd, const uint8_t* const pArgs) noexcept;
void _thunk_LIBCD_CdControlF() noexcept;

void LIBCD_CdMix() noexcept;

bool LIBCD_CdGetSector(void* const pDst, const int32_t sizeInWords) noexcept;
void _thunk_LIBCD_CdGetSector() noexcept;

CdlLOC& LIBCD_CdIntToPos(const int32_t sectorNum, CdlLOC& pos) noexcept;
void _thunk_LIBCD_CdIntToPos() noexcept;

int32_t LIBCD_CdPosToInt(const CdlLOC& pos) noexcept;
void _thunk_LIBCD_CdPosToInt() noexcept;

void LIBCD_BIOS_getintr() noexcept;
void LIBCD_CD_sync() noexcept;
void LIBCD_CD_ready() noexcept;
void LIBCD_CD_cw() noexcept;
void LIBCD_CD_vol() noexcept;
void LIBCD_CD_shell() noexcept;
void LIBCD_CD_flush() noexcept;
void LIBCD_CD_init() noexcept;
void LIBCD_CD_initvol() noexcept;
void LIBCD_BIOS_callback() noexcept;
int32_t LIBCD_CdGetToc(CdlLOC trackLocs[CdlMAXTOC]) noexcept;
