#pragma once

#include "PcPsx/Types.h"
#include "PsxVm/VmPtr.h"

struct master_status_structure;

extern const VmPtr<bool32_t>    gbWess_module_loaded;

void zeroset(void* const pDest, const uint32_t numBytes) noexcept;
void wess_install_error_handler(int32_t (* const pErrorFunc)(int32_t, int32_t), const int32_t module) noexcept;
master_status_structure* wess_get_master_status() noexcept;
bool Is_System_Active() noexcept;
bool Is_Module_Loaded() noexcept;
bool Is_Seq_Num_Valid(const int32_t seqNum) noexcept;
void Register_Early_Exit() noexcept;
void wess_install_handler() noexcept;
void wess_restore_handler() noexcept;
bool wess_init() noexcept;
void wess_exit(bool bForceRestoreTimerHandler) noexcept;
uint8_t* wess_get_wmd_start() noexcept;
uint8_t* wess_get_wmd_end() noexcept;
void free_mem_if_mine() noexcept;
void wess_unload_module() noexcept;

int32_t wess_load_module(
    const void* const pWmdFile,
    void* const pDestMem,
    const int32_t memoryAllowance,
    VmPtr<int32_t>* const pSettingTagLists
) noexcept;

void filltrackstat() noexcept;
void assigntrackstat() noexcept;
void wess_seq_structrig() noexcept;
void wess_seq_trigger() noexcept;
void wess_seq_trigger_special() noexcept;
void wess_seq_status() noexcept;
void wess_seq_stop() noexcept;
void wess_seq_stopall() noexcept;
