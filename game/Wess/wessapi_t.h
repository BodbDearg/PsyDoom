#pragma once

#include <cstdint>

struct track_status;
struct TriggerPlayAttr;

void updatetrackstat(track_status& trackStat, const TriggerPlayAttr* const pPlayAttribs) noexcept;
void wess_seq_trigger_type(const int32_t seqIdx, const uintptr_t seqType) noexcept;
void wess_seq_trigger_type_special(const int32_t seqIdx, const uintptr_t seqType, const TriggerPlayAttr* const pPlayAttribs) noexcept;
void wess_seq_update_type_special(const uintptr_t seqType, const TriggerPlayAttr* const pPlayAttribs) noexcept;
void wess_seq_stoptype(const uintptr_t seqType) noexcept;
