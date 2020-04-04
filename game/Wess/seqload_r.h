#pragma once

#include <cstdint>

int32_t wess_seq_range_sizeof(const int32_t firstSeqIdx, const int32_t numSeqs) noexcept;
int32_t wess_seq_range_load(const int32_t firstSeqIdx, const int32_t numSeqs, void* const pSeqMem) noexcept;
bool wess_seq_range_free(const int32_t firstSeqIdx, const int32_t numSeqs) noexcept;
