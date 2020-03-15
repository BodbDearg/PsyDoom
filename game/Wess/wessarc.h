#pragma once

#include "psxcd.h"
#include "PcPsx/Types.h"

extern const VmPtr<uint32_t>                    gWess_Millicount;
extern const VmPtr<bool32_t>                    gbWess_WessTimerActive;
extern const VmPtr<uint8_t[CD_SECTOR_SIZE]>     gWess_sectorBuffer1;
extern const VmPtr<uint8_t[CD_SECTOR_SIZE]>     gWess_sectorBuffer2;
extern const VmPtr<bool32_t>                    gbWess_SeqOn;

int16_t GetIntsPerSec() noexcept;
void CalcPartsPerInt() noexcept;
void WessInterruptHandler() noexcept;
void init_WessTimer() noexcept;
void exit_WessTimer() noexcept;
void Wess_init_for_LoadFileData() noexcept;
void module_open() noexcept;
void module_read() noexcept;
void module_seek() noexcept;
void module_tell() noexcept;
void module_close() noexcept;
void get_num_Wess_Sound_Drivers() noexcept;
void data_open() noexcept;
void data_read_chunk() noexcept;
void data_read() noexcept;
void data_close() noexcept;
void wess_low_level_init() noexcept;
void wess_low_level_exit() noexcept;
void* wess_malloc() noexcept;
void wess_free(void* const pMem) noexcept;
