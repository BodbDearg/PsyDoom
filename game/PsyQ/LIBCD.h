#pragma once

#include <cstdint>

// Describes a track location or file position on a CD.
// The location is described in audio terms.
struct CdlLOC {
    uint8_t minute;
    uint8_t second;
    uint8_t sector;
    uint8_t track;      // Unused
    
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

void LIBCD_CdInit() noexcept;
void LIBCD_EVENT_def_cbsync() noexcept;
void LIBCD_EVENT_def_cbready() noexcept;
void LIBCD_EVENT_def_cbread() noexcept;
void LIBCD_CdStatus() noexcept;
void LIBCD_CdLastCom() noexcept;
void LIBCD_CdReset() noexcept;
void LIBCD_CdFlush() noexcept;
void LIBCD_CdSetDebug() noexcept;
void LIBCD_CdComstr() noexcept;
void LIBCD_CdIntstr() noexcept;
void LIBCD_CdSync() noexcept;
void LIBCD_CdReady() noexcept;
void LIBCD_CdSyncCallback() noexcept;
void LIBCD_CdReadyCallback() noexcept;
void LIBCD_CdReadCallback() noexcept;
void LIBCD_CdControl() noexcept;
void LIBCD_CdControlF() noexcept;
void LIBCD_CdControlB() noexcept;
void LIBCD_CdMix() noexcept;
void LIBCD_CdGetSector() noexcept;
void LIBCD_CdDataCallback() noexcept;
void LIBCD_CdDataSync() noexcept;
void LIBCD_CdReadSync() noexcept;
void LIBCD_CdRead() noexcept;
void LIBCD_CdIntToPos() noexcept;
void LIBCD_CdPosToInt() noexcept;
void LIBCD_BIOS_getintr() noexcept;
void LIBCD_CD_sync() noexcept;
void LIBCD_CD_ready() noexcept;
void LIBCD_CD_cw() noexcept;
void LIBCD_CD_vol() noexcept;
void LIBCD_CD_shell() noexcept;
void LIBCD_CD_flush() noexcept;
void LIBCD_CD_init() noexcept;
void LIBCD_CD_initvol() noexcept;
void LIBCD_BIOS_cd_read_retry() noexcept;
void LIBCD_CD_readm() noexcept;
void LIBCD_CD_readsync() noexcept;
void LIBCD_CD_datasync() noexcept;
void LIBCD_CD_getsector() noexcept;
void LIBCD_CD_set_test_parmnum() noexcept;
void LIBCD_BIOS_callback() noexcept;
void LIBCD_BIOS_cb_read() noexcept;
void LIBCD_CdGetToc() noexcept;
void LIBCD_CdGetToc2() noexcept;
