#pragma once

#include "PsyQ/LIBCD.h"

enum class CdMapTbl_File : int32_t;

// Number of bytes in a CD-ROM sector
static constexpr int32_t CD_SECTOR_SIZE = 2048;

// Tracks an open file on the CD-ROM.
// Contains info about the file as well as the current IO location and status.
struct PsxCd_File {
    CdlFILE     file;               // Details about the file itself
    CdlLOC      new_io_loc;         // Current I/O location
    uint32_t    io_block_offset;    // Offset within the current sector for IO
    uint8_t     io_result[8];       // Result bytes from LIBCD for the last CD operation
};

static_assert(sizeof(PsxCd_File) == 40);

// Seek mode for seeking: similar to the C standard library seek modes
enum class PsxCd_SeekMode : int32_t {
    SET = 0,    // Set the offset
    CUR = 1,    // Seek relative to the current position
    END = 2     // Seek relative to the end
};

// Stores the location and size of a file on the CD-ROM.
// Used by the retail version of the game for fast file access, presumably not used during development as that would be painful.
// The location is stored in terms of start sector (2048 byte sector) number.
struct PsxCd_MapTblEntry {
    uint32_t    startSector;
    uint32_t    size;
};

static_assert(sizeof(PsxCd_MapTblEntry) == 8);

// Sector buffer for when we are reading data
extern const VmPtr<uint8_t[CD_SECTOR_SIZE]> gPSXCD_sectorbuf;

void psxcd_sync() noexcept;
bool psxcd_critical_sync() noexcept;
void PSXCD_cbcomplete(const CdlSyncStatus status, const uint8_t pResult[8]) noexcept;
void PSXCD_cbready(const CdlSyncStatus status, const uint8_t pResult[8]) noexcept;
void psxcd_disable_callbacks() noexcept;
void psxcd_enable_callbacks() noexcept;
void psxcd_init() noexcept;
void psxcd_exit() noexcept;
void psxcd_set_data_mode() noexcept;
PsxCd_File* psxcd_open(const CdMapTbl_File discFile) noexcept;
void psxcd_init_pos() noexcept;
bool psxcd_async_on() noexcept;
bool psxcd_seeking_for_play() noexcept;
bool psxcd_waiting_for_pause() noexcept;
int32_t psxcd_read(void* const pDest, int32_t numBytes, PsxCd_File& file) noexcept;
int32_t psxcd_seek(PsxCd_File& file, int32_t offset, const PsxCd_SeekMode mode) noexcept;
int32_t psxcd_tell(const PsxCd_File& file) noexcept;
void psxcd_close(PsxCd_File& file) noexcept;
void psxcd_set_audio_mode() noexcept;
void psxcd_set_loop_volume(const int32_t vol) noexcept;

void psxcd_play_at_andloop(
    const int32_t track,
    const int32_t vol,
    const int32_t sectorOffset,
    const int32_t fadeUpTime,
    const int32_t loopTrack,
    const int32_t loopVol,
    const int32_t loopSectorOffest,
    const int32_t loopFadeUpTime
) noexcept;

void psxcd_play_at(const int32_t track, const int32_t vol, const int32_t sectorOffset) noexcept;
void psxcd_play(const int32_t track, const int32_t vol) noexcept;
void psxcd_seek_for_play_at(const int32_t track, const int32_t sectorOffset) noexcept;
void psxcd_seek_for_play(const int32_t track) noexcept;
bool psxcd_play_status() noexcept;
void psxcd_stop() noexcept;
void psxcd_pause() noexcept;
void psxcd_restart(const int32_t vol) noexcept;
int32_t psxcd_elapsed_sectors() noexcept;
void psxcd_set_stereo(const bool bStereo) noexcept;
