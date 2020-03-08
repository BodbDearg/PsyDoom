#pragma once

#include "PsxVm/VmPtr.h"

extern const VmPtr<uint32_t> gWess_Millicount;

void GetIntsPerSec() noexcept;
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
void wess_malloc() noexcept;
void wess_free() noexcept;
