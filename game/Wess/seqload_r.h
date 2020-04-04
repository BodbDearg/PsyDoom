#pragma once

#include <cstdint>

int32_t wess_seq_range_sizeof(const int32_t firstSeqIdx, const int32_t numSeqs) noexcept;
void wess_seq_range_load() noexcept;
void wess_seq_range_free() noexcept;
