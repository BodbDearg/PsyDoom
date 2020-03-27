#pragma once

#include <cstdint>

void wess_seq_pause(const int32_t seqNum, const bool bMute) noexcept;
void queue_wess_seq_restart() noexcept;
void queue_wess_seq_pauseall() noexcept;
void queue_wess_seq_restartall() noexcept;
