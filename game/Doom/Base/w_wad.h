#pragma once

#include "PsxVm/VmPtr.h"

// Format for a name in a lump file
static constexpr uint32_t MAXLUMPNAME = 8;

struct lumpname_t {
    char chars[MAXLUMPNAME];
};

static_assert(sizeof(lumpname_t) == 8);

// Header for a lump in a WAD file
struct lumpinfo_t {
    uint32_t    filepos;
    uint32_t    size;
    lumpname_t  name;
};

static_assert(sizeof(lumpinfo_t) == 16);

extern const VmPtr<int32_t>             gNumLumps;
extern const VmPtr<VmPtr<lumpinfo_t>>   gpLumpInfo;
extern const VmPtr<VmPtr<VmPtr<void>>>  gpLumpCache;
extern const VmPtr<VmPtr<bool>>         gpbIsUncompressedLump;

void W_Init() noexcept;
int32_t W_CheckNumForName(const char* const name) noexcept;
int32_t W_GetNumForName(const char* const name) noexcept;
int32_t W_LumpLength(const int32_t lumpNum) noexcept;
void W_ReadLump(const int32_t lumpNum, void* const pDest, const bool bDecompress) noexcept;
void W_CacheLumpNum() noexcept;
void W_CacheLumpName() noexcept;
void W_OpenMapWad() noexcept;
void W_MapLumpLength() noexcept;
void W_MapGetNumForName() noexcept;
void W_ReadMapLump() noexcept;

void decode(const void* pSrc, void* pDst) noexcept;
void _thunk_decode() noexcept;

uint32_t getDecodedSize(const void* const pSrc) noexcept;
