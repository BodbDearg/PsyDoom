#include "WadFile.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"

#include <cctype>

//------------------------------------------------------------------------------------------------------------------------------------------
// Header for a WAD file: constains high level information about the contents of the WAD
//------------------------------------------------------------------------------------------------------------------------------------------
struct WadHdr {
    char        fileid[4];          // Should always be "IWAD" for PSX DOOM
    int32_t     numLumps;           // The number of lumps in the WAD
    int32_t     lumpHdrsOffset;     // Offset in the WAD of the array of lump headers
};

static_assert(sizeof(WadHdr) == 12);

//------------------------------------------------------------------------------------------------------------------------------------------
// Header for a lump in a WAD file
//------------------------------------------------------------------------------------------------------------------------------------------
struct WadLumpHdr {
    uint32_t            wadFileOffset;
    uint32_t            decompressedSize;
    WadFile::LumpName   name;
};

static_assert(sizeof(WadLumpHdr) == 16);

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates a WAD file that has not yet been opened
//------------------------------------------------------------------------------------------------------------------------------------------
WadFile::WadFile() noexcept
    : mNumLumps(0)
    , mLumpNames{}
    , mLumps{}
    , mFileReader()
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Move a WAD file from one object to another
//------------------------------------------------------------------------------------------------------------------------------------------
WadFile::WadFile(WadFile&& other) noexcept 
    : mNumLumps(other.mNumLumps)
    , mLumpNames(std::move(other.mLumpNames))
    , mLumps(std::move(other.mLumps))
    , mFileReader(std::move(other.mFileReader))
{
    other.mNumLumps = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frees up the WAD file and it's resources
//------------------------------------------------------------------------------------------------------------------------------------------
WadFile::~WadFile() noexcept {
    close();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes up the WAD file and releases all resources used
//------------------------------------------------------------------------------------------------------------------------------------------
void WadFile::close() noexcept {
    purgeAllLumps();

    mFileReader.close();
    mLumps.reset();
    mLumpNames.reset();
    mNumLumps = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Opens the specified WAD file on disk.
// Note: if a WAD file is currently opened then it is closed first.
//------------------------------------------------------------------------------------------------------------------------------------------
void WadFile::open(const char* const filePath) noexcept {
    close();
    mFileReader.open(filePath);
    initAfterOpen();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Opens the specified WAD file from the game's CD.
// Note: if a WAD file is currently opened then it is closed first.
//------------------------------------------------------------------------------------------------------------------------------------------
void WadFile::open(const CdFileId fileId) noexcept {
    close();
    mFileReader.open(fileId);
    initAfterOpen();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the index of the lump with the specified name, starting the search from the specified lump index.
// If not found then '-1' will be returned.
// Note that name comparison is case insensitive and only up to 8 characters of the lump name will be used, the rest will be ignored.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t WadFile::findLumpIdx(const char* const lumpName, const int32_t searchStartIdx) const noexcept {
    const uint64_t lumpNameWord = makeLumpNameWord(lumpName);

    const int32_t numLumps = mNumLumps;
    const LumpName* const pLumpNames = mLumpNames.get();

    for (int32_t i = searchStartIdx; i < numLumps; ++i) {
        if ((pLumpNames[i].word & LUMPNAME_MASK) == lumpNameWord)
            return i;
    }

    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Evict a single lump from the cache (if cached) and free the memory used to hold it's data
//------------------------------------------------------------------------------------------------------------------------------------------
void WadFile::purgeCachedLump(const int32_t lumpIdx) noexcept {
    ASSERT((lumpIdx >= 0) && (lumpIdx < mNumLumps));
    Lump& lump = mLumps[lumpIdx];
    void* const pCachedData = lump.pCachedData;

    if (pCachedData) {
        Z_Free2(*gpMainMemZone, pCachedData);
        ASSERT_LOG(!lump.pCachedData, "Z_Free2 should clear the pointer field pointing to the freed memory block!");
        lump.bIsDecompressed = false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Evicts all lumps from the cache and frees the memory used to hold their data
//------------------------------------------------------------------------------------------------------------------------------------------
void WadFile::purgeAllLumps() noexcept {
    Lump* const pLumps = mLumps.get();
    const int32_t numLumps = mNumLumps;
    memzone_t& mainMemZone = *gpMainMemZone;

    for (int32_t i = 0; i < numLumps; ++i) {
        void* const pCachedData = pLumps[i].pCachedData;

        if (pCachedData) {
            Z_Free2(mainMemZone, pCachedData);
            ASSERT_LOG(!pLumps[i].pCachedData, "Z_Free2 should clear the pointer field pointing to the freed memory block!");
            pLumps[i].bIsDecompressed = false;
        }
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
const WadFile::Lump& WadFile::cacheLump(const int32_t lumpIdx, const int16_t allocTag, const bool bDecompress) noexcept {
    // Sanity check the lump number is range.
    // Note: we disallow reading the last lump since the next lump after the current is required to exist.
    // We require the next lump to exist in order to determine the lump's compressed size in the WAD, since the lump header has the uncompressed size.
    if (lumpIdx + 1 >= mNumLumps) {
        I_Error("WadFile::cacheLump: %i + 1 >= numLumps", lumpIdx);
    }

    // See if the lump is already loaded.
    // If that is the case then this call might just be a no-op.
    Lump& lump = mLumps[lumpIdx];

    if (lump.pCachedData) {
        // Handle the situation where the caller wanted the data uncompressed when it's already loaded and compressed.
        // In this scenario decompress the currently loaded lump and have it replace the previously compressed version of the lump.
        // The original game did not trigger this case but now with PsyDoom it's possible, so we must fix.
        if (bDecompress && (!lump.bIsDecompressed)) {
            void* const pCompressedLump = lump.pCachedData;
            
            Z_SetUser(pCompressedLump, nullptr); // N.B: Doing this to avoid wiping the cache entry on 'Z_Free'
            Z_Malloc(*gpMainMemZone, lump.decompressedSize, allocTag, &lump.pCachedData);
            decompressLump(pCompressedLump, lump.pCachedData);
            Z_Free2(*gpMainMemZone, pCompressedLump);

            lump.bIsDecompressed = true;
        }

        return lump;
    }

    // If we get to here then the lump is not cached and will have to be loaded.
    // Note that originally the PSX engine disallowed loading lumps during gameplay due to slow CD-ROM I/O, but PsyDoom waives this restriction.
    // This change means that levels no longer need to ship with 'MAPSPR--.IMG' and 'MAPTEX--.IMG' files and can load resources on the fly.
    int32_t sizeToRead;

    if (bDecompress) {
        sizeToRead = lump.decompressedSize;
    } else {
        // If not decompressing we must use the distance to the next lump in the file to get the compressed size (it's not specified explicitly):
        const Lump& nextLump = mLumps[lumpIdx + 1];
        const int32_t lumpCompressedSize = nextLump.wadFileOffset - lump.wadFileOffset;
        sizeToRead = lumpCompressedSize;
    }

    // Alloc RAM for the lump and read it
    Z_Malloc(*gpMainMemZone, sizeToRead, allocTag, &lump.pCachedData);
    readLump(lumpIdx, lump.pCachedData, bDecompress);

    // Save whether the lump is compressed or not.
    // If the lump is compressed then the highest bit of the first character in the name will be set:
    if ((uint8_t) mLumpNames[lumpIdx].chars[0] & 0x80) {
        lump.bIsDecompressed = bDecompress;
    } else {
        lump.bIsDecompressed = true;
    }

    return lump;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the requested lump index into the given buffer.
// The buffer must be big enough to accomodate the data and (optionally) decompression can be disabled.
//------------------------------------------------------------------------------------------------------------------------------------------
void WadFile::readLump(const int32_t lumpIdx, void* const pDest, const bool bDecompress) noexcept {
    // Sanity check the lump number is range.
    // Note: we disallow reading the last lump since the next lump after the current is required to exist.
    // We require the next lump to exist in order to determine the lump's compressed size in the WAD, since the lump header has the uncompressed size.
    if (lumpIdx + 1 >= mNumLumps) {
        I_Error("WadFile::readLump: %i + 1 >= numLumps", lumpIdx);
    }

    // Get info for the lump requested and the one following it
    const Lump& lump = mLumps[lumpIdx];
    const Lump& nextLump = mLumps[lumpIdx + 1];

    const LumpName& lumpName = mLumpNames[lumpIdx];

    // Do we need to decompress?
    // Decompress if specified and if the lumpname has the special bit set in the first character, indicating that the lump is compressed.
    const uint32_t sizeToRead = nextLump.wadFileOffset - lump.wadFileOffset;
    const bool bIsLumpCompressed = ((uint8_t) lumpName.chars[0] & 0x80u);

    if (bDecompress && bIsLumpCompressed) {
        // Decompression needed, must alloc a temp buffer for the compressed data before reading and decompressing!
        void* const pTmpBuffer = Z_EndMalloc(*gpMainMemZone, sizeToRead, PU_STATIC, nullptr);

        mFileReader.seekAbsolute(lump.wadFileOffset);
        mFileReader.read(pTmpBuffer, sizeToRead);
        ASSERT(getDecompressedLumpSize(pTmpBuffer) == lump.decompressedSize);   // Sanity check the WAD data in debug mode
        decompressLump(pTmpBuffer, pDest);

        Z_Free2(*gpMainMemZone, pTmpBuffer);
    } else {
        // No decompression needed, can just read straight into the output buffer
        mFileReader.seekAbsolute(lump.wadFileOffset);
        mFileReader.read(pDest, sizeToRead);

        // Sanity check the WAD data in debug mode
        ASSERT((!bIsLumpCompressed) || getDecompressedLumpSize(pDest) == lump.decompressedSize);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decompresses the given compressed lump data into the given output buffer.
// The compression algorithm used is a form of LZSS.
// Assumes the output buffer is sized big enough to hold all of the decompressed data.
//------------------------------------------------------------------------------------------------------------------------------------------
void WadFile::decompressLump(const void* const pSrc, void* const pDst) noexcept {
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
// Similar to 'decompressLump' except that it does not do any decompression.
// Instead this function returns the decompressed size of the lump data, given just the data itself.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t WadFile::getDecompressedLumpSize(const void* const pSrc) noexcept {
    // This code is pretty much a replica of 'decompressLump()' - see that function for more details/comments.
    // The only difference here is that we don't save the decompressed data and just count the number of output bytes instead.
    const uint8_t* pSrcByte = (uint8_t*) pSrc;
    int32_t size = 0;

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
            // We are only interested in the byte count for this function.
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Converts a name into a 64-bit integer (8 characters maximum, and ASCII uppercase)
//------------------------------------------------------------------------------------------------------------------------------------------
uint64_t WadFile::makeLumpNameWord(const char* const name) noexcept {
    // Get up to 8 characters of the name in uppercase
    union {
        char        nameUChars[MAX_LUMPNAME] = {};
        uint64_t    nameWord;
    };

    for (int i = 0; i < MAX_LUMPNAME; ++i) {
        const char c = name[i];

        if (c == 0)
            break;

        if (c >= 'a' && c <= 'z') {
            nameUChars[i] = c - 32;
        } else {
            nameUChars[i] = c;
        }
    }

    return nameWord;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs WAD initialization after the file has been opened
//------------------------------------------------------------------------------------------------------------------------------------------
void WadFile::initAfterOpen() noexcept {
    readLumpInfo();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// WAD file initialization step: reads the header for the WAD, then reads information for all lumps and sets up lump entries
//------------------------------------------------------------------------------------------------------------------------------------------
void WadFile::readLumpInfo() noexcept {
    // Read and ensure the WAD file header is valid.
    // Note: allow 'IWAD' or 'PWAD' as the file id, originally just 'IWAD' was allowed.
    WadHdr wadHdr = {};
    mFileReader.read(wadHdr);

    const bool bIsIWAD = (D_strncasecmp(wadHdr.fileid, "IWAD", sizeof(wadHdr.fileid)) == 0);
    const bool bIsPWAD = (D_strncasecmp(wadHdr.fileid, "PWAD", sizeof(wadHdr.fileid)) == 0);

    if ((!bIsIWAD) && (!bIsPWAD)) {
        I_Error("WadFile::initAfterOpen: invalid file id for WAD! Must be 'IWAD' or 'PWAD'!");
    }

    // Read the array of lump headers
    std::unique_ptr<WadLumpHdr[]> lumpHdrs(new WadLumpHdr[wadHdr.numLumps]);
    mFileReader.seekAbsolute(wadHdr.lumpHdrsOffset);
    mFileReader.read(lumpHdrs.get(), wadHdr.numLumps * sizeof(WadLumpHdr));

    // Setup the list of lump names and lumps using the lump headers
    mNumLumps = wadHdr.numLumps;
    mLumpNames.reset(new LumpName[wadHdr.numLumps]);
    mLumps.reset(new Lump[wadHdr.numLumps]);
    
    std::memset(mLumps.get(), 0, sizeof(Lump) * wadHdr.numLumps);

    for (int32_t i = 0; i < wadHdr.numLumps; ++i) {
        const WadLumpHdr& lumpHdr = lumpHdrs[i];
        Lump& lump = mLumps[i];

        mLumpNames[i] = lumpHdr.name;
        lump.wadFileOffset = lumpHdr.wadFileOffset;
        lump.decompressedSize = lumpHdr.decompressedSize;
    }
}
