#pragma once

#include "Endian.h"

enum class CdFileId : int32_t;

// This is a mask to chop off the highest bit of the 1st 32-bit word in a lump name.
// That bit is not part of the name, it is used to indicate whether the lump is compressed or not.
static constexpr uint32_t NAME_WORD_MASK = Endian::isLittle() ? 0xFFFFFF7F : 0x7FFFFFFF;

// Format for a name in a lump file.
// Note: using a union here so we can do writes as words or chars in certain places without causing strict aliasing violations.
static constexpr uint32_t MAXLUMPNAME = 8;

union lumpname_t {
    char        chars[MAXLUMPNAME];
    uint32_t    words[2];
};

static_assert(sizeof(lumpname_t) == 8);

// Header for a lump in a WAD file
struct lumpinfo_t {
    uint32_t    filepos;
    uint32_t    size;
    lumpname_t  name;
};

static_assert(sizeof(lumpinfo_t) == 16);

extern int32_t      gNumLumps;
extern lumpinfo_t*  gpLumpInfo;
extern void**       gpLumpCache;
extern bool*        gpbIsUncompressedLump;
extern bool         gbIsLevelDataCached;

void W_Init() noexcept;
int32_t W_CheckNumForName(const char* const name) noexcept;
int32_t W_GetNumForName(const char* const name) noexcept;
int32_t W_LumpLength(const int32_t lumpNum) noexcept;
void W_ReadLump(const int32_t lumpNum, void* const pDest, const bool bDecompress) noexcept;
void* W_CacheLumpNum(const int32_t lumpNum, const int16_t allocTag, const bool bDecompress) noexcept;
void* W_CacheLumpName(const char* const name, const int16_t allocTag, const bool bDecompress) noexcept;
void* W_OpenMapWad(const CdFileId discFile) noexcept;
int32_t W_MapLumpLength(const int32_t lumpNum) noexcept;
int32_t W_MapCheckNumForName(const char* const name) noexcept;
void W_ReadMapLump(const int32_t lumpNum, void* const pDest, const bool bDecompress) noexcept;
void decode(const void* pSrc, void* pDst) noexcept;
uint32_t getDecodedSize(const void* const pSrc) noexcept;
