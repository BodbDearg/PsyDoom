#pragma once

#include "PsyQ/LIBCD.h"

enum class CdMapTbl_File : uint32_t;

// Number of bytes in a CD-ROM sector
static constexpr int32_t CD_SECTOR_SIZE = 2048;

// Tracks an open file on the CD-ROM.
// Contains info about the file as well as the current IO location and status.
struct PsxCd_File {
    CdlFILE     file;
    CdlLOC      new_io_loc;
    uint32_t    io_block_offset;
    uint8_t     io_result[8];
};

static_assert(sizeof(PsxCd_File) == 40);

// Seek mode for seeking: similar to the C standard library seek modes
enum class PsxCd_SeekMode : uint32_t {
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

void PSXCD_psxcd_memcpy() noexcept;
void psxcd_sync() noexcept;
void psxcd_critical_sync() noexcept;

void PSXCD_cbcomplete(const CdlStatus status, const uint8_t pResult[8]) noexcept;
void PSXCD_cbready(const CdlStatus status, const uint8_t pResult[8]) noexcept;

void psxcd_disable_callbacks() noexcept;
void psxcd_enable_callbacks() noexcept;
void psxcd_init() noexcept;
void psxcd_exit() noexcept;
void psxcd_set_data_mode() noexcept;

PsxCd_File* psxcd_open(const CdMapTbl_File discFile) noexcept;
void _thunk_psxcd_open() noexcept;

void psxcd_init_pos() noexcept;
void psxcd_async_on() noexcept;
void psxcd_seeking_for_play() noexcept;
void psxcd_waiting_for_pause() noexcept;

int32_t psxcd_read(void* const pDest, int32_t numBytes, PsxCd_File& file) noexcept;
void _thunk_psxcd_read() noexcept;

int32_t psxcd_seek(PsxCd_File& file, int32_t offset, const PsxCd_SeekMode mode) noexcept;
void _thunk_psxcd_seek() noexcept;

int32_t psxcd_tell(PsxCd_File& file) noexcept;
void _thunk_psxcd_tell() noexcept;

void psxcd_close(PsxCd_File& file) noexcept;
void _thunk_psxcd_close() noexcept;

void psxcd_set_audio_mode() noexcept;
void psxcd_set_loop_volume() noexcept;
void psxcd_play_at_andloop() noexcept;
void psxcd_play_at() noexcept;
void psxcd_play() noexcept;
void psxcd_seek_for_play_at() noexcept;
void psxcd_seek_for_play() noexcept;
void psxcd_play_status() noexcept;
void psxcd_stop() noexcept;
void psxcd_pause() noexcept;
void psxcd_restart() noexcept;
void psxcd_elapsed_sectors() noexcept;
void psxcd_set_stereo() noexcept;
