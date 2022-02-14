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

    void add(const char* const filePath, const RemapWadLumpNameFn lumpNameRemapFn = nullptr) noexcept;
    void add(const CdFileId fileId, const RemapWadLumpNameFn lumpNameRemapFn = nullptr) noexcept;
    void finalize() noexcept;
    void clear() noexcept;

    inline bool isValidLumpIdx(const int32_t lumpIdx) const noexcept {
        return ((lumpIdx >= 0) && (lumpIdx < getNumLumps()));
    }

    int32_t findLumpIdx(const WadLumpName lumpName, const int32_t searchStartIdx = 0) const noexcept;
    const WadLump& getLump(const int32_t lumpIdx) const noexcept;
    const WadLumpName getLumpName(const int32_t lumpIdx) const noexcept;
    inline int32_t getNumLumps() const noexcept { return (int32_t) mLumpHandles.size(); }

    void purgeCachedLump(const int32_t lumpIdx) noexcept;
    void purgeAllLumps() noexcept;
    const WadLump& cacheLump(const int32_t lumpIdx, const int16_t allocTag, const bool bDecompress) noexcept;
    void readLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept;

private:
    // Describes a lump in the 'combined' WAD file.
    // Tells which WAD file it is located in and which lump index it has within that WAD file.
    // Also stores the 64-bit (8 character) name for quick search.
    struct LumpHandle {
        int32_t         wadFileIdx;
        int32_t         wadLumpIdx;
        WadLumpName     name;           // Note: has the 'compressed' flag removed for faster search
    };

    std::vector<WadFile>        mWadFiles;
    std::vector<LumpHandle>     mLumpHandles;
};
