#pragma once

#include "WadFile.h"

#include <vector>

//------------------------------------------------------------------------------------------------------------------------------------------
// Provides transparent access to a list of WAD files, treating them as if they were all one combined WAD file.
// Allows user WADs to be loaded on top of the main PSXDOOM.WAD, enabling new textures to be easily added or existing ones replaced.
//------------------------------------------------------------------------------------------------------------------------------------------
class WadList {
public:
    WadList() noexcept;
    ~WadList() noexcept;

    void add(const char* const filePath) noexcept;
    void add(const CdFileId fileId) noexcept;
    void finalize() noexcept;

    int32_t findLumpIdx(const char* const lumpName, const int32_t searchStartIdx = 0) const noexcept;
    const WadFile::Lump& getLump(const int32_t lumpIdx) const noexcept;
    const WadFile::LumpName getLumpName(const int32_t lumpIdx) const noexcept;
    inline int32_t getNumLumps() const noexcept { return (int32_t) mLumpHandles.size(); }

    void purgeCachedLump(const int32_t lumpIdx) noexcept;
    void purgeAllLumps() noexcept;
    const WadFile::Lump& cacheLump(const int32_t lumpIdx, const int16_t allocTag, const bool bDecompress) noexcept;
    void readLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept;

private:
    // Describes a lump in the 'combined' WAD file.
    // Tells which WAD file it is located in and which lump index it has within that WAD file.
    // Also stores the 64-bit (8 character) name for quick search.
    struct LumpHandle {
        int32_t     wadFileIdx;
        int32_t     wadLumpIdx;
        uint64_t    nameWord;       // Note: has the 'compressed' flag removed for faster search
    };

    std::vector<WadFile>        mWadFiles;
    std::vector<LumpHandle>     mLumpHandles;
};
