#pragma once

#include <cstdint>

struct TriggerPlayAttr;

void updatetrackstat() noexcept;
void wess_seq_trigger_type(const int32_t seqNum, const uint32_t seqType) noexcept;
void wess_seq_trigger_type_special(const int32_t seqNum, const uint32_t seqType, const TriggerPlayAttr* const pPlayAttribs) noexcept;
void queue_wess_seq_update_type_special() noexcept;
void wess_seq_stoptype(const uint32_t seqType) noexcept;
