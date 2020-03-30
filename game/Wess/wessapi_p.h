#pragma once

#include <cstdint>

struct NoteState;

void wess_seq_pause(const int32_t seqNum, const bool bMute) noexcept;
void wess_seq_restart(const int32_t seqNum) noexcept;
void wess_seq_pauseall(const bool bMute, NoteState* const pNoteState) noexcept;
void wess_seq_restartall(NoteState* const pPrevNoteState) noexcept;
