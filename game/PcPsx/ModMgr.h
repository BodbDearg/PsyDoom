#pragma once

#include "Doom/cdmaptbl.h"
#include "Macros.h"
#include "Wess/psxcd.h"

class WadList;

BEGIN_NAMESPACE(ModMgr)

void init() noexcept;
void shutdown() noexcept;
void addUserWads(WadList& wadList) noexcept;

// File overrides mechanism
bool areOverridesAvailableForFile(const CdFileId discFile) noexcept;
bool isFileOverriden(const PsxCd_File& file) noexcept;
bool openOverridenFile(const CdFileId discFile, PsxCd_File& fileOut) noexcept;
void closeOverridenFile(PsxCd_File& file) noexcept;
int32_t readFromOverridenFile(void* const pDest, int32_t numBytes, PsxCd_File& file) noexcept;
int32_t seekForOverridenFile(PsxCd_File& file, int32_t offset, const PsxCd_SeekMode mode) noexcept;
int32_t tellForOverridenFile(const PsxCd_File& file) noexcept;
int32_t getOverridenFileSize(const CdFileId discFile) noexcept;

END_NAMESPACE(ModMgr)
