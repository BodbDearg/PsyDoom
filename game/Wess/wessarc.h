#pragma once

#include "psxcd.h"
#include "PcPsx/Types.h"

extern const VmPtr<uint32_t>                    gWess_Millicount;
extern const VmPtr<uint32_t>                    gWess_Millicount_Frac;
extern const VmPtr<bool32_t>                    gbWess_WessTimerActive;
extern const VmPtr<uint8_t[CD_SECTOR_SIZE]>     gWess_sectorBuffer1;
extern const VmPtr<uint8_t[CD_SECTOR_SIZE]>     gWess_sectorBuffer2;
extern const VmPtr<bool32_t>                    gbWess_SeqOn;

int16_t GetIntsPerSec() noexcept;
void CalcPartsPerInt() noexcept;
int32_t WessInterruptHandler() noexcept;
void init_WessTimer() noexcept;
void exit_WessTimer() noexcept;
bool Wess_init_for_LoadFileData() noexcept;
PsxCd_File* module_open(const CdMapTbl_File fileId) noexcept;
int32_t module_read(void* const pDest, const int32_t numBytes, PsxCd_File& file) noexcept;
int32_t module_seek(PsxCd_File& file, const int32_t seekPos, const PsxCd_SeekMode seekMode) noexcept;
int32_t module_tell(const PsxCd_File& file) noexcept;
void module_close(PsxCd_File& file) noexcept;
int32_t get_num_Wess_Sound_Drivers() noexcept;
PsxCd_File* data_open(const CdMapTbl_File fileId) noexcept;
int32_t data_read(PsxCd_File& file, const int32_t destSpuAddr, const int32_t numBytes, const int32_t fileOffset) noexcept;
void data_close(PsxCd_File& file) noexcept;
void wess_low_level_init() noexcept;
void wess_low_level_exit() noexcept;
void* wess_malloc(const int32_t size) noexcept;
void wess_free(void* const pMem) noexcept;
