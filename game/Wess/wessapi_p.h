#pragma once

#include <cstdint>

struct SavedVoiceList;

void wess_seq_pause(const int32_t seqIdx, const bool bMute) noexcept;
void wess_seq_restart(const int32_t seqIdx) noexcept;
void wess_seq_pauseall(const bool bMute, SavedVoiceList* const pSavedVoices) noexcept;
void wess_seq_restartall(SavedVoiceList* const pPrevVoiceState) noexcept;
