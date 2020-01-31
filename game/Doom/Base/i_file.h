#pragma once

#include "Wess/psxcd.h"

void InitOpenFileSlots() noexcept;

int32_t OpenFile(const CdMapTbl_File discFile) noexcept;
void CloseFile(const int32_t fileSlotIdx) noexcept;

int32_t SeekAndTellFile(const int32_t fileSlotIdx, const int32_t offset, const PsxCd_SeekMode seekMode) noexcept;
void ReadFile(const int32_t fileSlotIdx, void* const pBuffer, const uint32_t size) noexcept;
