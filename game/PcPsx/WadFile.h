#pragma once

#include "Asserts.h"
#include "Endian.h"
#include "GameFileReader.h"

#include <memory>

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a WAD file: provides access to the lumps within the file and maintains an open file handle to the WAD.
// This is part of the replacement for the old WAD handling code for PsyDoom.
//------------------------------------------------------------------------------------------------------------------------------------------
class WadFile {
public:
    //------------------------------------------------------------------------------------------------------------------------------------------
    // Format for a name in a lump file, as stored on disk.
    // Note: use a union so we can access as a 64-bit word or individual chars in certain places without causing strict aliasing violations.
    //------------------------------------------------------------------------------------------------------------------------------------------
    static constexpr int32_t MAX_LUMPNAME = 8;

    union LumpName {
        char        chars[MAX_LUMPNAME];
        uint64_t    word;
    };

    static_assert(sizeof(LumpName) == 8);

    //------------------------------------------------------------------------------------------------------------------------------------------
    // This is a mask to chop off the highest bit of the 1st byte in a 64-bit lump name.
    // That bit is not part of the name, it is used to indicate whether the lump is compressed or not.
    //------------------------------------------------------------------------------------------------------------------------------------------
    static constexpr uint64_t LUMPNAME_MASK = Endian::isLittle() ? 0xFFFFFFFFFFFFFF7F : 0x7FFFFFFFFFFFFFFF;

    //------------------------------------------------------------------------------------------------------------------------------------------
    // Holds details about one lump in a wad file
    //------------------------------------------------------------------------------------------------------------------------------------------
    struct Lump {
        void*       pCachedData;            // The data for the lump cached into memory, if 'nullptr' then the lump has not been loaded yet.
        bool        bIsDecompressed;        // Only has meaning if the lump is cached. If 'true' then the cached data is decompressed, otherwise it is compressed.
        int32_t     wadFileOffset;          // Offset of the lump in the WAD file.
        int32_t     decompressedSize;       // Original size of the lump in bytes, before any compression.
    };

    WadFile() noexcept;
    WadFile(WadFile&& other) noexcept;
    ~WadFile() noexcept;

    void close() noexcept;
    void open(const char* const filePath) noexcept;
    void open(const CdFileId fileId) noexcept;

    inline const Lump& getLump(const int32_t lumpIdx) const noexcept {
        ASSERT((lumpIdx >= 0) && (lumpIdx < mNumLumps));
        return mLumps[lumpIdx];
    }

    inline LumpName getLumpName(const int32_t lumpIdx) const noexcept {
        ASSERT((lumpIdx >= 0) && (lumpIdx < mNumLumps));
        return mLumpNames[lumpIdx];
    }

    inline int32_t getNumLumps() const noexcept {
        return mNumLumps;
    }

    int32_t findLumpIdx(const char* const lumpName, const int32_t searchStartIdx = 0) const noexcept;

    void purgeCachedLump(const int32_t lumpIdx) noexcept;
    void purgeAllLumps() noexcept;
    const Lump& cacheLump(const int32_t lumpIdx, const int16_t allocTag, const bool bDecompress) noexcept;
    void readLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept;

    static uint64_t makeLumpNameWord(const char* const name) noexcept;
    static void decompressLump(const void* const pSrc, void* const pDst) noexcept;
    static int32_t getDecompressedLumpSize(const void* const pSrc) noexcept;

private:
    WadFile(const WadFile& other) = delete;
    WadFile& operator = (const WadFile& other) = delete;
    WadFile& operator = (WadFile&& other) = delete;

    void initAfterOpen() noexcept;
    void readLumpInfo() noexcept;

    int32_t                         mNumLumps;          // The number of lumps in the WAD
    std::unique_ptr<LumpName[]>     mLumpNames;         // Store names in their own list for cache-friendly search
    std::unique_ptr<Lump[]>         mLumps;             // The details and data for each lump
    GameFileReader                  mFileReader;        // Responsible for reading from the WAD file
};
