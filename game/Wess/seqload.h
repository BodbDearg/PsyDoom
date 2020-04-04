#pragma once

#include <cstdint>

enum class CdMapTbl_File : uint32_t;
struct master_status_structure;

// Types of sequence loader errors
enum Seq_Load_Error : int32_t {
    SEQLOAD_NO_ERROR,
    SEQLOAD_FOPEN,
    SEQLOAD_FREAD,
    SEQLOAD_FSEEK
};

typedef void (*SeqLoaderErrorHandler)(const int32_t module, const Seq_Load_Error errorCode) noexcept;

void wess_seq_load_err(const Seq_Load_Error errorCode) noexcept;
void wess_seq_loader_install_error_handler(const SeqLoaderErrorHandler handler, const int32_t module) noexcept;
bool Is_Seq_Seq_Num_Valid(const int32_t seqIdx) noexcept;
bool open_sequence_data() noexcept;
void close_sequence_data() noexcept;
void load_sequence_data() noexcept;
bool wess_seq_loader_init(master_status_structure* const pMStat, const CdMapTbl_File moduleFileId, const bool bOpenModuleFile) noexcept;
void wess_seq_loader_exit() noexcept;
int32_t wess_seq_sizeof(const int32_t seqIdx) noexcept;
void wess_seq_load() noexcept;
bool wess_seq_free(const int32_t seqIdx) noexcept;
