#pragma once

#include <cstdint>

void wess_seq_load_err() noexcept;
void wess_seq_loader_install_error_handler() noexcept;
bool Is_Seq_Seq_Num_Valid(const int32_t seqIdx) noexcept;
void open_sequence_data() noexcept;
void close_sequence_data() noexcept;
void load_sequence_data() noexcept;
void wess_seq_loader_init() noexcept;
void wess_seq_loader_exit() noexcept;
void wess_seq_sizeof() noexcept;
void wess_seq_load() noexcept;
void wess_seq_free() noexcept;
