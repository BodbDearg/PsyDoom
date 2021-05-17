#pragma once

#include "Asserts.h"
#include "Endian.h"
#include "GameFileReader.h"

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
// Format for the name of a lump in a WAD file, as stored on disk.
// Note: use a union so we can access as a 64-bit word or individual chars in certain places without causing strict aliasing violations.
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr int32_t MAX_WAD_LUMPNAME = 8;

union WadLumpName {
    uint64_t word;

    // This structure is a workaround for the fact that we can't initialize an array at compile time in a constexpr context
    struct {
        char c0, c1, c2, c3, c4, c5, c6, c7;

        inline constexpr char& operator[](const size_t i) noexcept { return (&c0)[i]; }
        inline constexpr const char& operator[](const size_t i) const noexcept { return (&c0)[i]; }
    } chars;

    inline constexpr WadLumpName() noexcept : word(0) {}
    inline constexpr WadLumpName(const uint64_t word) noexcept : word(word) {}

    inline constexpr WadLumpName(const char c0, const char c1, const char c2, const char c3, const char c4, const char c5, const char c6, const char c7) noexcept
        : chars{}
    {
        chars.c0 = c0;
        chars.c1 = c1;
        chars.c2 = c2;
        chars.c3 = c3;
        chars.c4 = c4;
        chars.c5 = c5;
        chars.c6 = c6;
        chars.c7 = c7;
    }
};

static_assert(sizeof(WadLumpName) == 8);

//------------------------------------------------------------------------------------------------------------------------------------------
// This is a mask to chop off the highest bit of the 1st byte in a 64-bit WAD lump name.
// That bit is not part of the name, it is used to indicate whether the WAD lump is compressed or not.
//------------------------------------------------------------------------------------------------------------------------------------------
static constexpr uint64_t WAD_LUMPNAME_MASK = Endian::isLittle() ? 0xFFFFFFFFFFFFFF7F : 0x7FFFFFFFFFFFFFFF;

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
    void open(const char* const filePath) noexcept;
    void open(const CdFileId fileId) noexcept;

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
    const WadLump& cacheLump(const int32_t lumpIdx, const int16_t allocTag, const bool bDecompress) noexcept;
    void readLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept;

private:
    WadFile(const WadFile& other) = delete;
    WadFile& operator = (const WadFile& other) = delete;
    WadFile& operator = (WadFile&& other) = delete;

    void initAfterOpen() noexcept;
    void readLumpInfo() noexcept;

    int32_t                         mNumLumps;          // The number of lumps in the WAD
    std::unique_ptr<WadLumpName[]>  mLumpNames;         // Store names in their own list for cache-friendly search
    std::unique_ptr<WadLump[]>      mLumps;             // The details and data for each lump
    GameFileReader                  mFileReader;        // Responsible for reading from the WAD file
};
