#pragma once

#include "Wess/psxcd.h"

void InitOpenFileSlots() noexcept;

int32_t OpenFile(const uint32_t fileNum) noexcept;
void _thunk_OpenFile() noexcept;

void CloseFile(const int32_t fileSlotIdx) noexcept;
void _thunk_CloseFile() noexcept;

int32_t SeekAndTellFile(const int32_t fileSlotIdx, const int32_t offset, const PsxCd_SeekMode seekMode) noexcept;
void _thunk_SeekAndTellFile() noexcept;

void ReadFile() noexcept;
