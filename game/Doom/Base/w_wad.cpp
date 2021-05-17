//------------------------------------------------------------------------------------------------------------------------------------------
// Note: WAD file handling has been more or less rewritten to support PsyDoom's modding goals.
// For the original code, see the 'Old' code folder.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "w_wad.h"

#if PSYDOOM_MODS

#include "Doom/cdmaptbl.h"
#include "i_main.h"
#include "PcPsx/WadList.h"

// A flag set to true once data for the current map has been loaded.
// Has very little purpose anymore in PsyDoom; was originally used to ensure the game was not loading resources on-the-fly off the CD-ROM.
// In PsyDoom however on-the-fly resource loading is allowed, so this flag's purpose is diminished.
bool gbIsLevelDataCached;

// A list of WAD files that are used to source all data for the game, except for map data which is found in individual map WAD files.
// At a minimum this will contain just 'PSXDOOM.WAD' but may also include other user WADs.
static WadList gMainWadList;

// The currently open map WAD.
// This is only used to load level data, and nothing else.
static WadFile gMapWad;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the WAD file management system.
// Opens up the main WAD files and verifies they are valid.
//------------------------------------------------------------------------------------------------------------------------------------------
void W_Init() noexcept {
    // Open all main WAD files and finalize the list.
    // TODO: support user WAD files here.
    gMainWadList.add(CdFileId::PSXDOOM_WAD);
    gMainWadList.finalize();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the WAD file management system and cleans up resources used
//------------------------------------------------------------------------------------------------------------------------------------------
void W_Shutdown() noexcept {
    gMapWad.close();
    gMainWadList.clear();
    gbIsLevelDataCached = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the details for the specified main WAD lump (except the name)
//------------------------------------------------------------------------------------------------------------------------------------------
const WadLump& W_GetLump(const int32_t lumpIdx) noexcept {
    return gMainWadList.getLump(lumpIdx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the name for the specified main WAD lump
//------------------------------------------------------------------------------------------------------------------------------------------
const WadLumpName W_GetLumpName(const int32_t lumpIdx) noexcept {
    return gMainWadList.getLumpName(lumpIdx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the lump index for the given main WAD lump name or -1 if not found.
// Optionally, the search can start at a lump index other than '0' in order to support sequential processing of repeated lump names.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_CheckNumForName(const WadLumpName lumpName, const int32_t searchStartIdx) noexcept {
    return gMainWadList.findLumpIdx(lumpName, searchStartIdx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the lump index for the given main WAD lump name or fail with a fatal error if not found
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_GetNumForName(const WadLumpName lumpName) noexcept {
    // Search for the lump name and return it if found
    const int32_t lumpIdx = W_CheckNumForName(lumpName);

    if (lumpIdx >= 0)
        return lumpIdx;

    // Otherwise issue an error for the missing lump name.
    // Note that the string might not be null terminated, so have to ensure that here.
    char lumpNameCStr[MAX_WAD_LUMPNAME + 1];
    std::memcpy(lumpNameCStr, &lumpName.chars, MAX_WAD_LUMPNAME);
    lumpNameCStr[MAX_WAD_LUMPNAME] = 0;

    I_Error("W_GetNumForName: %s not found!", lumpNameCStr);
    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gives the decompressed size (in bytes) of the given main WAD lump (specified by lump index).
// Issues a fatal error if the lump index is invalid.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_LumpLength(const int32_t lumpIdx) noexcept {
    const WadLump& lump = gMainWadList.getLump(lumpIdx);
    return lump.uncompressedSize;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the requested main WAD lump (specified by lump index) into the given buffer.
// The buffer must be big enough to accomodate the data.
// Optionally, decompression can be disabled.
//------------------------------------------------------------------------------------------------------------------------------------------
void W_ReadLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept {
    gMainWadList.readLump(lumpIdx, pDest, bDecompress);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cache/load the desired main WAD lump (specified by lump index) and return information for the lump, including it's data.
// If the lump is already cached does nothing. Optionally, lump decompression can be skipped.
//------------------------------------------------------------------------------------------------------------------------------------------
const WadLump& W_CacheLumpNum(const int32_t lumpIdx, const int16_t allocTag, const bool bDecompress) noexcept {
    return gMainWadList.cacheLump(lumpIdx, allocTag, bDecompress);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper that caches a specified main WAD lump (specified by name)
//------------------------------------------------------------------------------------------------------------------------------------------
const WadLump& W_CacheLumpName(const WadLumpName lumpName, const int16_t allocTag, const bool bDecompress) noexcept {
    return W_CacheLumpNum(W_GetNumForName(lumpName), allocTag, bDecompress);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open the specified map wad file for reading.
// Note: if a map WAD is already opened then it will be closed by this operation.
//------------------------------------------------------------------------------------------------------------------------------------------
void W_OpenMapWad(const CdFileId fileId) noexcept {
    gMapWad.open(fileId);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes the currently open map WAD, if any
//------------------------------------------------------------------------------------------------------------------------------------------
void W_CloseMapWad() noexcept {
    gMapWad.close();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Give the decompressed size in bytes of the specified lump index from the currently loaded map WAD.
// Issues a fatal error if the lump is not found.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_MapLumpLength(const int32_t lumpIdx) noexcept {
    const WadLump& lump = gMapWad.getLump(lumpIdx);
    return lump.uncompressedSize;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the lump index for the given map lump name or -1 if not found
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_MapCheckNumForName(const WadLumpName lumpName) noexcept {
    return gMapWad.findLumpIdx(lumpName);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the requested lump index from the currently open map WAD into the given buffer.
// The buffer must be big enough to accomodate the data. Optionally, decompression can be disabled.
//------------------------------------------------------------------------------------------------------------------------------------------
void W_ReadMapLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept {
    gMapWad.readLump(lumpIdx, pDest, bDecompress);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// These functions have been relocated to the new WAD handling code.
// Keeping these calls here however so previous code using them can still work without changes.
//------------------------------------------------------------------------------------------------------------------------------------------
void decode(const void* pSrc, void* pDst) noexcept {
    WadUtils::decompressLump(pSrc, pDst);
}

uint32_t getDecodedSize(const void* const pSrc) noexcept {
    return WadUtils::getDecompressedLumpSize(pSrc);
}

#endif  // #if PSYDOOM_MODS
