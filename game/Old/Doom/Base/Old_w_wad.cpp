#if !PSYDOOM_MODS

#include "w_wad.h"

#include "Asserts.h"
#include "Doom/cdmaptbl.h"
#include "Doom/d_main.h"
#include "i_file.h"
#include "i_main.h"
#include "z_zone.h"

// WAD file header
struct wadinfo_t {
    char        fileid[4];      // Should always be "IWAD" for PSX DOOM
    int32_t     numlumps;       // The number of lumps in the WAD
    int32_t     infotableofs;   // Offset in the WAD of the lump infos array
};

static_assert(sizeof(wadinfo_t) == 12);

// Main IWAD lump related state: the number of lumps, info on each lump, pointers to loaded lumps and
// whether each lump was loaded from the main IWAD or not.
int32_t         gNumLumps;
lumpinfo_t*     gpLumpInfo;
void**          gpLumpCache;
bool*           gpbIsUncompressedLump;
bool            gbIsLevelDataCached;

// Which of the open files is the main WAD file
static uint32_t gMainWadFileIdx;

// Map WAD stuff: pointer to the loaded WAD in memory, number of map WAD lumps, pointer to the map WAD info tables (the WAD directory).
static void*    gpMapWadFileData;
static int32_t  gNumMapWadLumps;
lumpinfo_t*     gpMapWadLumpInfo;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the WAD file management system.
// Opens up the main WAD file and verifies it is valid, and then reads all of the header info for all of the lumps.
//------------------------------------------------------------------------------------------------------------------------------------------
void W_Init() noexcept {
    // Initialize the list of open file slots and open the main IWAD file
    InitOpenFileSlots();
    gMainWadFileIdx = OpenFile(CdFileId::PSXDOOM_WAD);

    // Read the header for the IWAD and ensure it has the correct id/magic
    wadinfo_t wadinfo = {};
    ReadFile(gMainWadFileIdx, &wadinfo, sizeof(wadinfo_t));

    if (D_strncasecmp(wadinfo.fileid, "IWAD", sizeof(wadinfo.fileid)) != 0) {
        I_Error("W_Init: invalid main IWAD id");
    }

    // Save the number of lumps and alloc the lump info array
    gNumLumps = wadinfo.numlumps;
    gpLumpInfo = (lumpinfo_t*) Z_Malloc(*gpMainMemZone, gNumLumps * sizeof(lumpinfo_t), PU_STATIC, nullptr);

    // Read the lump info array
    SeekAndTellFile(gMainWadFileIdx, wadinfo.infotableofs, PsxCd_SeekMode::SET);
    ReadFile(gMainWadFileIdx, gpLumpInfo, gNumLumps * sizeof(lumpinfo_t));

    // Alloc and zero init the lump cache pointers list and an array of bools to say whether each lump is compressed or not.
    static_assert(sizeof(bool) == 1, "Expect bool to be 1 byte!");

    gpLumpCache = (void**) Z_Malloc(*gpMainMemZone, gNumLumps * sizeof(void*), PU_STATIC, nullptr);
    gpbIsUncompressedLump = (bool*) Z_Malloc(*gpMainMemZone, gNumLumps * sizeof(bool), PU_STATIC, nullptr);

    D_memset(gpLumpCache, std::byte(0), gNumLumps * sizeof(void*));
    D_memset(gpbIsUncompressedLump, std::byte(0), gNumLumps * sizeof(bool));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the lump index for the given main IWAD lump name (case insensitive) or -1 if not found
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_CheckNumForName(const char* const name) noexcept {
    // Copy the given name and uppercase for case insensitivity
    char nameUpper[MAXLUMPNAME + 1];
    D_memset(nameUpper, std::byte(0), sizeof(nameUpper));
    D_strncpy(nameUpper, name, MAXLUMPNAME);
    nameUpper[MAXLUMPNAME] = 0;
    D_strupr(nameUpper);

    // Get the two words of the name for faster comparison
    const uint32_t findNameW1 = (uint32_t&) nameUpper[0];
    const uint32_t findNameW2 = (uint32_t&) nameUpper[4];

    // Try to find the given lump name and compare names using 32-bit words rather than single chars
    lumpinfo_t* pLump = gpLumpInfo;
    int32_t lumpIdx = 0;

    while (lumpIdx < gNumLumps) {
        // Note: must mask the highest bit of the first character of the lump name.
        // This bit is used to indicate whether the lump is compressed or not.
        const uint32_t lumpNameW1 = ((uint32_t&) pLump->name.chars[0]) & NAME_WORD_MASK;
        const uint32_t lumpNameW2 = ((uint32_t&) pLump->name.chars[4]);

        if (lumpNameW1 == findNameW1 && lumpNameW2 == findNameW2)
            return lumpIdx;

        ++lumpIdx;
        ++pLump;
    };

    // If we get to here then the lump name is not found
    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the lump index for the given main IWAD lump name (case insensitive) or fail with a fatal error if not found
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_GetNumForName(const char* const name) noexcept {
    const int32_t lumpIdx = W_CheckNumForName(name);

    if (lumpIdx >= 0) {
        return lumpIdx;
    } else {
        I_Error("W_GetNumForName: %s not found!", name);
        return -1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Give the size in bytes of the given lump number from the main IWAD.
// Issues a fatal error if the lump is not found.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_LumpLength(const int32_t lumpNum) noexcept {
    if (lumpNum >= gNumLumps) {
        I_Error("W_LumpLength: %i >= numlumps", lumpNum);
    }

    const lumpinfo_t& lump = gpLumpInfo[lumpNum];
    return lump.size;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the requested lump number from the main IWAD into the given buffer.
// The buffer must be big enough to accomodate the data.
// Optionally, decompression can be disabled.
//------------------------------------------------------------------------------------------------------------------------------------------
void W_ReadLump(const int32_t lumpNum, void* const pDest, const bool bDecompress) noexcept {
    // Sanity check the lump number is range
    if (lumpNum >= gNumLumps) {
        I_Error("W_ReadLump: %i >= numlumps", lumpNum);
    }

    // The lump requested and the next one after that
    const lumpinfo_t& lump = gpLumpInfo[lumpNum];
    const lumpinfo_t& nextLump = gpLumpInfo[lumpNum + 1];

    // Do we need to decompress? Decompress if specified and if the lumpname has the special bit
    // set in the first character, indicating that the lump is compressed.
    const uint32_t sizeToRead = nextLump.filepos - lump.filepos;

    if (bDecompress && (((uint8_t) lump.name.chars[0] & 0x80u) != 0)) {
        // Decompression needed, must alloc a temp buffer for the compressed data before reading and decompressing!
        void* const pTmpBuffer = Z_EndMalloc(*gpMainMemZone, lump.size, PU_STATIC, nullptr);

        SeekAndTellFile(gMainWadFileIdx, lump.filepos, PsxCd_SeekMode::SET);
        ReadFile(gMainWadFileIdx, pTmpBuffer, sizeToRead);
        decode(pTmpBuffer, pDest);

        Z_Free2(*gpMainMemZone, pTmpBuffer);
    } else {
        // No decompression needed, can just read straight into the output buffer
        SeekAndTellFile(gMainWadFileIdx, lump.filepos, PsxCd_SeekMode::SET);
        ReadFile(gMainWadFileIdx, pDest, sizeToRead);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cache/load the specified lump number and return it's pointer.
// If the lump is already cached does nothing.
// Optionally, lump decompression can be skipped.
// Note that lump caching is *NOT* permitted during regular gameplay!
//------------------------------------------------------------------------------------------------------------------------------------------
void* W_CacheLumpNum(const int32_t lumpNum, const int16_t allocTag, const bool bDecompress) noexcept {
    // Sanity check the lump number is in range
    if (lumpNum >= gNumLumps) {
        I_Error("W_CacheLumpNum: %i >= numlumps", lumpNum);
    }

    // If the lump is already loaded then we don't need to do anything
    void*& lumpCacheEntry = gpLumpCache[lumpNum];

    if (!lumpCacheEntry) {
        // If the level loading is done then we should NOT be loading lumps during gameplay.
        // Because CD-ROM I/O is so slow, this would cause very serious stalls and slowdowns during gameplay, so consider it a fatal error.
        // Unlike PC DOOM the PSX version cannot simply stream in lumps on the fly...
        if (gbIsLevelDataCached) {
            I_Error("cache miss on lump %i", lumpNum);
        }

        // Figure out how much data we will need to allocate. If we are decompressing then this will be the decompressed size, otherwise
        // it will be the actual size of the lump before decompression is applied:
        const lumpinfo_t& lumpInfo = gpLumpInfo[lumpNum];
        int32_t sizeToRead;

        if (bDecompress) {
            sizeToRead = lumpInfo.size;
        } else {
            const lumpinfo_t& nextLumpInfo = gpLumpInfo[lumpNum + 1];
            const int32_t lumpStorageSize = nextLumpInfo.filepos - lumpInfo.filepos;
            sizeToRead = lumpStorageSize;
        }

        // Alloc RAM for the lump and read it
        Z_Malloc(*gpMainMemZone, sizeToRead, allocTag, &lumpCacheEntry);
        W_ReadLump(lumpNum, lumpCacheEntry, bDecompress);

        // Save whether the lump is compressed or not.
        // If the lump is compressed then the highest bit of the first character in the name will be set:
        if ((lumpInfo.name.chars[0] & 0x80) != 0) {
            gpbIsUncompressedLump[lumpNum] = bDecompress;
        } else {
            gpbIsUncompressedLump[lumpNum] = true;
        }
    }

    return lumpCacheEntry;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper that caches a specified lump name from the main IWAD
//------------------------------------------------------------------------------------------------------------------------------------------
void* W_CacheLumpName(const char* const name, const int16_t allocTag, const bool bDecompress) noexcept {
    return W_CacheLumpNum(W_GetNumForName(name), allocTag, bDecompress);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open the specified map wad file, load it entirely into memory and return a pointer to it's data
//------------------------------------------------------------------------------------------------------------------------------------------
void* W_OpenMapWad(const CdFileId discFile) noexcept {
    // Load the entire map WAD into memory
    {
        const uint32_t openFileIdx = OpenFile(discFile);
        const int32_t wadSize = SeekAndTellFile(openFileIdx, 0, PsxCd_SeekMode::END);
        gpMapWadFileData = Z_EndMalloc(*gpMainMemZone, wadSize, PU_STATIC, nullptr);

        SeekAndTellFile(openFileIdx, 0, PsxCd_SeekMode::SET);
        ReadFile(openFileIdx, gpMapWadFileData, wadSize);
        CloseFile(openFileIdx);
    }

    // Make sure the file id is valid.
    // Note that PSX DOOM expects the identifier "IWAD" also in it's map wads, rather then "PWAD".
    const wadinfo_t& wadinfo = *(wadinfo_t*) gpMapWadFileData;
    const bool bIsValidWad = (D_strncasecmp(wadinfo.fileid, "IWAD", sizeof(wadinfo.fileid)) == 0);

    if (!bIsValidWad) {
        I_Error("W_OpenMapWad: invalid map IWAD id");
    }

    // Finish up by saving some high level map wad info
    gNumMapWadLumps = wadinfo.numlumps;
    gpMapWadLumpInfo = (lumpinfo_t*)((std::byte*) gpMapWadFileData + wadinfo.infotableofs);
    return gpMapWadFileData;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Give the size in bytes of the given lump number from the currently loaded map WAD.
// Issues a fatal error if the lump is not found.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_MapLumpLength(const int32_t lumpNum) noexcept {
    if (lumpNum >= gNumMapWadLumps) {
        I_Error("W_MapLumpLength: %i out of range", lumpNum);
    }

    const lumpinfo_t& lump = gpMapWadLumpInfo[lumpNum];
    return lump.size;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the lump index for the given map lump name (case insensitive) or -1 if not found
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_MapCheckNumForName(const char* const name) noexcept {
    // Copy the given name and uppercase for case insensitivity
    char nameUpper[MAXLUMPNAME + 1];
    D_memset(nameUpper, std::byte(0), sizeof(nameUpper));
    D_strncpy(nameUpper, name, MAXLUMPNAME);
    nameUpper[MAXLUMPNAME] = 0;
    D_strupr(nameUpper);

    // Get the two words of the name for faster comparison
    const uint32_t findNameW1 = (uint32_t&) nameUpper[0];
    const uint32_t findNameW2 = (uint32_t&) nameUpper[4];

    // Try to find the given lump name and compare names using 32-bit words rather than single chars
    lumpinfo_t* pLump = gpMapWadLumpInfo;
    int32_t lumpIdx = 0;

    while (lumpIdx < gNumMapWadLumps) {
        // Note: must mask the highest bit of the first character of the lump name.
        // This bit is used to indicate whether the lump is compressed or not.
        const uint32_t lumpNameW1 = ((uint32_t&) pLump->name.chars[0]) & NAME_WORD_MASK;
        const uint32_t lumpNameW2 = ((uint32_t&) pLump->name.chars[4]);

        if (lumpNameW1 == findNameW1 && lumpNameW2 == findNameW2)
            return lumpIdx;

        ++lumpIdx;
        ++pLump;
    };

    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the requested lump number from the currently open map WAD into the given buffer.
// The buffer must be big enough to accomodate the data.
// Optionally, decompression can be disabled.
//------------------------------------------------------------------------------------------------------------------------------------------
void W_ReadMapLump(const int32_t lumpNum, void* const pDest, const bool bDecompress) noexcept {
    // Sanity check the lump number is range
    if (lumpNum >= gNumMapWadLumps) {
        I_Error("W_ReadMapLump: lump %d out of range", lumpNum);
    }

    // Do we need to decompress? Decompress if specified and if the lumpname has the special bit
    // set in the first character, indicating that the lump is compressed.
    const lumpinfo_t& lump = gpMapWadLumpInfo[lumpNum];
    const std::byte* const pLumpBytes = (std::byte*) gpMapWadFileData + lump.filepos;

    if (bDecompress && (((uint8_t) lump.name.chars[0] & 0x80u) != 0)) {
        // Decompression needed: decompress to the given output buffer
        decode(pLumpBytes, pDest);
    } else {
        // No decompression needed, can just copy straight into the output buffer
        const lumpinfo_t& nextLump = gpMapWadLumpInfo[lumpNum + 1];
        const uint32_t sizeToCopy = nextLump.filepos - lump.filepos;
        D_memcpy(pDest, pLumpBytes, sizeToCopy);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decode the given compressed data into the given output buffer.
// The compression algorithm used is a form of LZSS.
//------------------------------------------------------------------------------------------------------------------------------------------
void decode(const void* pSrc, void* pDst) noexcept {
    const uint8_t* pSrcByte = (const uint8_t*) pSrc;
    uint8_t* pDstByte = (uint8_t*) pDst;

    uint32_t idByte = 0;        // Controls whether there is compressed or uncompressed data ahead
    uint32_t haveIdByte = 0;    // Controls when to read an id byte, when '0' we need to read another one

    while (true) {
        // Read the id byte if required.
        // We need 1 id byte for every 8 bytes of uncompressed output, or every 8 runs of compressed data.
        if (haveIdByte == 0) {
            idByte = *pSrcByte;
            ++pSrcByte;
        }

        haveIdByte = (haveIdByte + 1) & 7;

        if (idByte & 1) {
            // Compressed data ahead: the first 12-bits tells where to take repeated data from.
            // The remaining 4-bits tell how many bytes of repeated data to take.
            const uint32_t srcByte1 = pSrcByte[0];
            const uint32_t srcByte2 = pSrcByte[1];
            pSrcByte += 2;

            const int32_t srcOffset = ((srcByte1 << 4) | (srcByte2 >> 4)) + 1;
            const int32_t numRepeatedBytes = (srcByte2 & 0xF) + 1;

            // A value of '1' is a special value and means we have reached the end of the compressed stream
            if (numRepeatedBytes == 1)
                break;

            const uint8_t* const pRepeatedBytes = pDstByte - srcOffset;

            for (int32_t i = 0; i < numRepeatedBytes; ++i) {
                *pDstByte = pRepeatedBytes[i];
                ++pDstByte;
            }
        } else {
            // Uncompressed data: just copy the input byte
            *pDstByte = *pSrcByte;
            ++pSrcByte;
            ++pDstByte;
        }

        idByte >>= 1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Similar to 'decode' except does not do any decompression.
// Instead, this function returns the decompressed size of the data.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t getDecodedSize(const void* const pSrc) noexcept {
    // This code is pretty much a replica of 'decode()' - see that function for more details/comments.
    // The only difference here is that we don't save the decompressed data and just count the number of output bytes instead.
    const uint8_t* pSrcByte = (uint8_t*) pSrc;
    uint32_t size = 0;

    uint32_t idByte = 0;
    uint32_t haveIdByte = 0;

    while (true) {
        if (haveIdByte == 0) {
            idByte = *pSrcByte;
            ++pSrcByte;
        }

        haveIdByte = (haveIdByte + 1) & 7;

        if (idByte & 1) {
            // Note: not bothering to read the byte containing only positional information for the replicated data.
            // We are only interested in the byte count in this instance.
            const uint32_t srcByte2 = pSrcByte[1];
            pSrcByte += 2;
            const uint32_t numRepeatedBytes = (srcByte2 & 0xF) + 1;

            if (numRepeatedBytes == 1)
                break;

            size += numRepeatedBytes;
        } else {
            ++size;
            ++pSrcByte;
        }

        idByte >>= 1;
    }

    return size;
}

#endif  #if !PSYDOOM_MODS
