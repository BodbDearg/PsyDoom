#pragma once

#if !PSYDOOM_MODS

#include "PsyQ/LIBCD.h"

// Tracks an open file on the CD-ROM.
// Contains info about the file as well as the current IO location and status.
struct PsxCd_File {
    CdlFILE     file;               // Details about the file itself
    CdlLOC      new_io_loc;         // Current I/O location
    uint32_t    io_block_offset;    // Offset within the current sector for IO
    uint8_t     io_result[8];       // Result bytes from LIBCD for the last CD operation
};

void psxcd_sync() noexcept;
bool psxcd_critical_sync() noexcept;
bool psxcd_async_on() noexcept;
void psxcd_init_pos() noexcept;

#endif  // #if !PSYDOOM_MODS
