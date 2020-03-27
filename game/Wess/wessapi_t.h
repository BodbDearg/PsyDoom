#pragma once

#include <cstdint>

void updatetrackstat() noexcept;
void wess_seq_trigger_type(const int32_t seqNum, const int32_t seqType) noexcept;
void wess_seq_trigger_type_special() noexcept;
void queue_wess_seq_update_type_special() noexcept;
void wess_seq_stoptype() noexcept;
