#pragma once

#include "PsxVm/VmPtr.h"

// Format for a name in a lump file
struct lumpname_t {
    char chars[8];
};

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
void W_CheckNumForName() noexcept;
void W_GetNumForName() noexcept;
void W_LumpLength() noexcept;
void W_ReadLump() noexcept;
void W_CacheLumpNum() noexcept;
void W_CacheLumpName() noexcept;
void W_OpenMapWad() noexcept;
void W_MapLumpLength() noexcept;
void W_MapGetNumForName() noexcept;
void W_ReadMapLump() noexcept;
void decode() noexcept;
void getDecodedSize() noexcept;
