#pragma once

#include "Asserts.h"
#include "Endian.h"
#include "GameFileReader.h"
#include "SmallString.h"

#include <memory>

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds details about one lump in a wad file (except for the name)
//------------------------------------------------------------------------------------------------------------------------------------------
struct WadLump {
    void*       pCachedData;            // The data for the lump cached into memory, if 'nullptr' then the lump has not been loaded yet.
    bool        bIsUncompressed;        // Only has meaning if the lump is cached. If 'true' then the cached data is uncompressed, otherwise it is compressed.
    int32_t     wadFileOffset;          // Offset of the lump in the WAD file.
    int32_t     uncompressedSize;       // Original size of the lump in bytes, before any compression.
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Format for the name of a lump in a WAD file, as stored on disk
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int32_t MAX_WAD_LUMPNAME = String8::MAX_LEN;
typedef String8 WadLumpName;
static_assert(sizeof(WadLumpName) == 8);

//------------------------------------------------------------------------------------------------------------------------------------------
// This is a mask to chop off the highest bit of the 1st byte in a 64-bit WAD lump name.
// That bit is not part of the name, it is used to indicate whether the WAD lump is compressed or not.
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint64_t WAD_LUMPNAME_MASK = Endian::isLittle() ? 0xFFFFFFFFFFFFFF7F : 0x7FFFFFFFFFFFFFFF;

//------------------------------------------------------------------------------------------------------------------------------------------
// Type for a function that remaps a WAD lump name to (potentially) be some other name.
// This can be supplied as an optional extra when opening a WAD to change the names of certain lumps.
//------------------------------------------------------------------------------------------------------------------------------------------
typedef WadLumpName (*RemapWadLumpNameFn)(const WadLumpName& oldName) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a WAD file: provides access to the lumps within the file and maintains an open file handle to the WAD.
// This is part of the replacement for the old WAD handling code for PsyDoom.
//------------------------------------------------------------------------------------------------------------------------------------------
class WadFile {
public:
    WadFile() noexcept;
    WadFile(WadFile&& other) noexcept;
    ~WadFile() noexcept;

    void close() noexcept;
    void open(const char* const filePath, const RemapWadLumpNameFn lumpNameRemapFn = nullptr) noexcept;
    void open(const CdFileId fileId, const RemapWadLumpNameFn lumpNameRemapFn = nullptr) noexcept;

    inline bool isValidLumpIdx(const int32_t lumpIdx) const noexcept {
        return ((lumpIdx >= 0) && (lumpIdx < mNumLumps));
    }

    inline const WadLump& getLump(const int32_t lumpIdx) const noexcept {
        ASSERT(isValidLumpIdx(lumpIdx));
        return mLumps[lumpIdx];
    }

    inline WadLumpName getLumpName(const int32_t lumpIdx) const noexcept {
        ASSERT(isValidLumpIdx(lumpIdx));
        return mLumpNames[lumpIdx];
    }

    inline int32_t getNumLumps() const noexcept {
        return mNumLumps;
    }

    int32_t findLumpIdx(const WadLumpName lumpName, const int32_t searchStartIdx = 0) const noexcept;

    void purgeCachedLump(const int32_t lumpIdx) noexcept;
    void purgeAllLumps() noexcept;
    int32_t getRawSize(const int32_t lumpIdx) noexcept;
    const WadLump& cacheLump(const int32_t lumpIdx, const int16_t allocTag, const bool bDecompress) noexcept;
    void readLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept;

private:
    WadFile(const WadFile& other) = delete;
    WadFile& operator = (const WadFile& other) = delete;
    WadFile& operator = (WadFile&& other) = delete;

    void initAfterOpen(const RemapWadLumpNameFn lumpNameRemapFn) noexcept;
    void readLumpInfo(const RemapWadLumpNameFn lumpNameRemapFn) noexcept;

    int32_t                         mNumLumps;          // The number of lumps in the WAD
    int32_t                         mSizeInBytes;       // The total size (in bytes) of the entire WAD file
    std::unique_ptr<WadLumpName[]>  mLumpNames;         // Store names in their own list for cache-friendly search
    std::unique_ptr<WadLump[]>      mLumps;             // The details and data for each lump
    GameFileReader                  mFileReader;        // Responsible for reading from the WAD file
};
