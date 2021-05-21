#pragma once

#include <cstdint>

// PsyDoom: 'CdFileId' has changed in format
#if PSYDOOM_MODS
    union String16;
    typedef String16 CdFileId;
#else 
    typedef int32_t CdFileId;
#endif

struct master_status_structure;

// Types of sequence loader errors
enum Seq_Load_Error : int32_t {
    SEQLOAD_NO_ERROR,
    SEQLOAD_FOPEN,
    SEQLOAD_FREAD,
    SEQLOAD_FSEEK
};

// Form for a function which is called when a sequence loading error happens
typedef void (*SeqLoaderErrorHandler)(const int32_t module, const Seq_Load_Error errorCode) noexcept;

extern bool gbWess_seq_loader_enable;

void wess_seq_loader_install_error_handler(const SeqLoaderErrorHandler handler, const int32_t module) noexcept;
bool Is_Seq_Seq_Num_Valid(const int32_t seqIdx) noexcept;
bool open_sequence_data() noexcept;
void close_sequence_data() noexcept;
bool wess_seq_loader_init(master_status_structure* const pMStat, const CdFileId moduleFileId, const bool bOpenModuleFile) noexcept;
void wess_seq_loader_exit() noexcept;
int32_t wess_seq_sizeof(const int32_t seqIdx) noexcept;
int32_t wess_seq_load(const int32_t seqIdx, void* const pSeqMem) noexcept;
bool wess_seq_free(const int32_t seqIdx) noexcept;
