#pragma once

#include "PsyQ/LIBCD.h"

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

void PSXCD_psxcd_memcpy() noexcept;
void psxcd_sync() noexcept;
void psxcd_critical_sync() noexcept;
void PSXCD_cbcomplete() noexcept;
void PSXCD_cbready() noexcept;
void psxcd_disable_callbacks() noexcept;
void psxcd_enable_callbacks() noexcept;
void psxcd_init() noexcept;
void psxcd_exit() noexcept;
void psxcd_set_data_mode() noexcept;
void psxcd_open() noexcept;
void psxcd_init_pos() noexcept;
void psxcd_async_on() noexcept;
void psxcd_seeking_for_play() noexcept;
void psxcd_waiting_for_pause() noexcept;
void psxcd_read() noexcept;
void psxcd_async_read_cancel() noexcept;
void psxcd_async_read() noexcept;
void psxcd_seek() noexcept;
void psxcd_tell() noexcept;
void psxcd_close() noexcept;
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
