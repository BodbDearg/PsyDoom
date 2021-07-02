#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Note: WAD file handling has been more or less rewritten to support PsyDoom's modding goals.
// For the original code, see the 'Old' code folder.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS

#include "PsyDoom/WadUtils.h"

// PsyDoom: 'CdFileId' has changed in format
#if PSYDOOM_MODS
    struct String16;
    typedef String16 CdFileId;
#else 
    typedef int32_t CdFileId;
#endif

class WadFile;

extern bool gbIsLevelDataCached;

void W_Init() noexcept;
void W_Shutdown() noexcept;

#if PSYDOOM_MODS
    int32_t W_NumLumps() noexcept;
#endif

const WadLump& W_GetLump(const int32_t lumpIdx) noexcept;
const WadLumpName W_GetLumpName(const int32_t lumpIdx) noexcept;
int32_t W_CheckNumForName(const WadLumpName lumpName, const int32_t searchStartIdx = 0) noexcept;
int32_t W_GetNumForName(const WadLumpName lumpName) noexcept;
int32_t W_LumpLength(const int32_t lumpIdx) noexcept;
void W_ReadLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept;
const WadLump& W_CacheLumpNum(const int32_t lumpIdx, const int16_t allocTag, const bool bDecompress) noexcept;
const WadLump& W_CacheLumpName(const WadLumpName lumpName, const int16_t allocTag, const bool bDecompress) noexcept;
void W_OpenMapWad(const CdFileId fileId) noexcept;
void W_CloseMapWad() noexcept;
int32_t W_MapCheckNumForName(const WadLumpName lumpName) noexcept;
int32_t W_MapGetNumForName(const WadLumpName lumpName) noexcept;
int32_t W_MapLumpLength(const int32_t lumpIdx) noexcept;
void W_ReadMapLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept;
void decode(const void* pSrc, void* pDst) noexcept;
uint32_t getDecodedSize(const void* const pSrc) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Convenience overloads that work the old way: uppercases the given lump name - assumes all lumps in the WAD are uppercase.
// If the string is constant then these may now be able to make a 'WadLumpName' at compile time.
//------------------------------------------------------------------------------------------------------------------------------------------
inline int32_t W_CheckNumForName(const char* const name, const int32_t searchStartIdx = 0) noexcept {
    const WadLumpName ucaseName = WadUtils::makeUppercaseLumpName(name);
    return W_CheckNumForName(ucaseName, searchStartIdx);
}

inline int32_t W_GetNumForName(const char* const name) noexcept {
    const WadLumpName ucaseName = WadUtils::makeUppercaseLumpName(name);
    return W_GetNumForName(ucaseName);
}

inline const WadLump& W_CacheLumpName(const char* const name, const int16_t allocTag, const bool bDecompress) noexcept {
    const WadLumpName ucaseName = WadUtils::makeUppercaseLumpName(name);
    return W_CacheLumpName(ucaseName, allocTag, bDecompress);
}

inline int32_t W_MapCheckNumForName(const char* const name) noexcept {
    const WadLumpName ucaseName = WadUtils::makeUppercaseLumpName(name);
    return W_MapCheckNumForName(ucaseName);
}

inline int32_t W_MapGetNumForName(const char* const name) noexcept {
    const WadLumpName ucaseName = WadUtils::makeUppercaseLumpName(name);
    return W_MapGetNumForName(ucaseName);
}

#endif  // #if PSYDOOM_MODS
