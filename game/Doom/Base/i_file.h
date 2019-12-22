#pragma once

#include <cstdint>

void InitOpenFileSlots() noexcept;

int32_t OpenFile(const uint32_t fileNum) noexcept;
void _thunk_OpenFile() noexcept;

void CloseFile() noexcept;
void SeekAndTellFile() noexcept;
void ReadFile() noexcept;
