#include "WadList.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the WAD list with no WADS opened
//------------------------------------------------------------------------------------------------------------------------------------------
WadList::WadList() noexcept
    : mWadFiles()
    , mLumpHandles()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the list of WADS and releases all resources used
//------------------------------------------------------------------------------------------------------------------------------------------
WadList::~WadList() noexcept {
    // Nothing to do right now...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds a new WAD at the specified file path to the list.
// Note: the 'finalize()' call must be done before the WAD's resources are made visible.
//------------------------------------------------------------------------------------------------------------------------------------------
void WadList::add(const char* const filePath, const RemapWadLumpNameFn lumpNameRemapFn) noexcept {
    WadFile& wadFile = mWadFiles.emplace_back();
    wadFile.open(filePath, lumpNameRemapFn);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Adds a new WAD for the specified game file to the list.
// Note: the 'finalize()' call must be done before the WAD's resources are made visible.
//------------------------------------------------------------------------------------------------------------------------------------------
void WadList::add(const CdFileId fileId, const RemapWadLumpNameFn lumpNameRemapFn) noexcept {
    WadFile& wadFile = mWadFiles.emplace_back();
    wadFile.open(fileId, lumpNameRemapFn);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Makes the resources of all WADS in the list visible
//------------------------------------------------------------------------------------------------------------------------------------------
void WadList::finalize() noexcept {
    // How many lumps are there in total among all the wad files?
    // Reserve enough room to hold all of their handles.
    mLumpHandles.clear();
    int32_t totalLumps = 0;

    for (WadFile& wadFile : mWadFiles) {
        totalLumps += wadFile.getNumLumps();
    }

    mLumpHandles.reserve(totalLumps);

    // Create all of the lump handles
    for (int32_t wadFileIndex = 0; wadFileIndex < (int32_t) mWadFiles.size(); ++wadFileIndex) {
        WadFile& wadFile = mWadFiles[wadFileIndex];
        const int32_t numWadLumps = wadFile.getNumLumps();

        for (int32_t lumpIdx = 0; lumpIdx < numWadLumps; ++lumpIdx) {
            const WadLumpName lumpName = wadFile.getLumpName(lumpIdx);
            mLumpHandles.push_back({ wadFileIndex, lumpIdx, lumpName.word() & WAD_LUMPNAME_MASK });     // Note: remove the special 'compressed' flag bit to make later search a bit faster
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears the WAD list and unloads all WADs
//------------------------------------------------------------------------------------------------------------------------------------------
void WadList::clear() noexcept {
    mLumpHandles.clear();
    mWadFiles.clear();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the index of the lump with the specified name, starting the search from the specified lump index.
// If not found then '-1' will be returned.
// Note: when searching the 'compressed' flag bit in the 1st byte of candidate lump names is ignored.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t WadList::findLumpIdx(const WadLumpName lumpName, const int32_t searchStartIdx) const noexcept {
    const int32_t numLumps = (int32_t) mLumpHandles.size();
    const LumpHandle* const pLumpHandles = mLumpHandles.data();

    for (int32_t i = searchStartIdx; i < numLumps; ++i) {
        if (pLumpHandles[i].name.word() == lumpName.word())
            return i;
    }

    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns a specified lump in the combined WAD lump list
//------------------------------------------------------------------------------------------------------------------------------------------
const WadLump& WadList::getLump(const int32_t lumpIdx) const noexcept {
    ASSERT(isValidLumpIdx(lumpIdx));
    const LumpHandle& lumpHandle = mLumpHandles[lumpIdx];
    const WadFile& wadFile = mWadFiles[lumpHandle.wadFileIdx];
    return wadFile.getLump(lumpHandle.wadLumpIdx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the name of the specified lump
//------------------------------------------------------------------------------------------------------------------------------------------
const WadLumpName WadList::getLumpName(const int32_t lumpIdx) const noexcept {
    ASSERT(isValidLumpIdx(lumpIdx));
    const LumpHandle& lumpHandle = mLumpHandles[lumpIdx];
    const WadFile& wadFile = mWadFiles[lumpHandle.wadFileIdx];
    return wadFile.getLumpName(lumpHandle.wadLumpIdx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Evict a single lump from the cache (if cached) and free the memory used to hold it's data
//------------------------------------------------------------------------------------------------------------------------------------------
void WadList::purgeCachedLump(const int32_t lumpIdx) noexcept {
    ASSERT(isValidLumpIdx(lumpIdx));
    const LumpHandle& lumpHandle = mLumpHandles[lumpIdx];
    WadFile& wadFile = mWadFiles[lumpHandle.wadFileIdx];
    wadFile.purgeCachedLump(lumpHandle.wadLumpIdx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Evicts all lumps from the cache and frees the memory used to hold their data
//------------------------------------------------------------------------------------------------------------------------------------------
void WadList::purgeAllLumps() noexcept {
    for (WadFile& wadFile : mWadFiles) {
        wadFile.purgeAllLumps();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cache/load the specified lump index and return all of the lump information, including the cached data for the lump.
// Optionally, the lump can also be decompressed.
// 
// Notes:
//  (1) If the lump is already cached then this call is a no-op, unless if the cached lump is compressed and decompression is required.
//  (2) If decompression is not required then an uncompressed lump data may still be returned. This can happen if the data in the WAD file
//      is already decompressed or if the lump was cached in a decompressed state previously.
//------------------------------------------------------------------------------------------------------------------------------------------
const WadLump& WadList::cacheLump(const int32_t lumpIdx, const int16_t allocTag, const bool bDecompress) noexcept {
    ASSERT(isValidLumpIdx(lumpIdx));
    const LumpHandle& lumpHandle = mLumpHandles[lumpIdx];
    WadFile& wadFile = mWadFiles[lumpHandle.wadFileIdx];
    return wadFile.cacheLump(lumpHandle.wadLumpIdx, allocTag, bDecompress);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the requested lump index into the given buffer.
// The buffer must be big enough to accomodate the data and (optionally) decompression can be disabled.
//------------------------------------------------------------------------------------------------------------------------------------------
void WadList::readLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept {
    ASSERT(isValidLumpIdx(lumpIdx));
    const LumpHandle& lumpHandle = mLumpHandles[lumpIdx];
    WadFile& wadFile = mWadFiles[lumpHandle.wadFileIdx];
    wadFile.readLump(lumpHandle.wadLumpIdx, pDest, bDecompress);
}
