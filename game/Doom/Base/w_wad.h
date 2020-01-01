#pragma once

#include "PcPsx/Types.h"
#include "PsxVm/VmPtr.h"

enum class CdMapTbl_File : uint32_t;

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
extern const VmPtr<bool32_t>            gbIsLevelDataCached;

void W_Init() noexcept;
int32_t W_CheckNumForName(const char* const name) noexcept;
int32_t W_GetNumForName(const char* const name) noexcept;
int32_t W_LumpLength(const int32_t lumpNum) noexcept;
void W_ReadLump(const int32_t lumpNum, void* const pDest, const bool bDecompress) noexcept;

void* W_CacheLumpNum(const int32_t lumpNum, const int16_t allocTag, const bool bDecompress) noexcept;
void _thunk_W_CacheLumpNum() noexcept;

void* W_CacheLumpName(const char* const name, const int16_t allocTag, const bool bDecompress) noexcept;
void _thunk_W_CacheLumpName() noexcept;

void* W_OpenMapWad(const CdMapTbl_File discFile) noexcept;

int32_t W_MapLumpLength(const int32_t lumpNum) noexcept;
void _thunk_W_MapLumpLength() noexcept;

int32_t W_MapCheckNumForName(const char* const name) noexcept;

void W_ReadMapLump(const int32_t lumpNum, void* const pDest, const bool bDecompress) noexcept;
void _thunk_W_ReadMapLump() noexcept;

void decode(const void* pSrc, void* pDst) noexcept;
void _thunk_decode() noexcept;

uint32_t getDecodedSize(const void* const pSrc) noexcept;
