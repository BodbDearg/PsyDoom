#include "w_wad.h"

#include "Doom/cdmaptbl.h"
#include "Doom/d_main.h"
#include "i_file.h"
#include "i_main.h"
#include "PcPsx/Utils.h"
#include "PsxVm/PsxVm.h"
#include "PsxVm/VmSVal.h"
#include "z_zone.h"

// WAD file header
struct wadinfo_t {
    char        fileid[4];      // Should always be "IWAD" for PSX DOOM
    int32_t     numlumps;       // The number of lumps in the WAD
    int32_t     infotableofs;   // Offset in the WAD of the lump infos array
};

static_assert(sizeof(wadinfo_t) == 12);

// Lump related state: the number of lumps, info on each lump, pointers to loaded lumps and
// whether each lump was loaded from the main IWAD or not.
const VmPtr<int32_t>                gNumLumps(0x800781EC);
const VmPtr<VmPtr<lumpinfo_t>>      gpLumpInfo(0x800781C4);
const VmPtr<VmPtr<VmPtr<void>>>     gpLumpCache(0x8007823C);
const VmPtr<VmPtr<bool>>            gpbIsUncompressedLump(0x800782F0);
const VmPtr<bool32_t>               gbIsLevelDataCached(0x80077BE4);

// Which of the open files is the main WAD file
static const VmPtr<uint32_t> gMainWadFileIdx(0x80078254);

// This is a mask to chop off the highest bit of the 1st 32-bit word in a lump name.
// That bit is not part of the name, it is used to indicate whether the lump is compressed or not.
static constexpr uint32_t NAME_WORD_MASK = isHostCpuLittleEndian() ? 0xFFFFFF7F : 0x7FFFFFFF;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the WAD file management system.
// Opens up the main WAD file and verifies it is valid, and then reads all of the header info for all of the lumps.
//------------------------------------------------------------------------------------------------------------------------------------------
void W_Init() noexcept {
    // Initialize the list of open file slots and open the main IWAD file
    InitOpenFileSlots();
    *gMainWadFileIdx = OpenFile(CdMapTbl_File::PSXDOOM_WAD);

    // Read the header for the IWAD and ensure it has the correct id/magic
    VmSVal<wadinfo_t> wadinfo;
    ReadFile(*gMainWadFileIdx, wadinfo.get(), sizeof(wadinfo_t));

    a0 = ptrToVmAddr(wadinfo->fileid);
    a1 = 0x80077BE8;        // Result = STR_IWAD[0] (80077BE8)
    a2 = sizeof(wadinfo->fileid);
    D_strncasecmp();

    if (v0 != 0) {
        I_Error("W_Init: invalid main IWAD id");
    }

    // Save the number of lumps and alloc the lump info array
    *gNumLumps = wadinfo->numlumps;
    *gpLumpInfo = (lumpinfo_t*) Z_Malloc(**gpMainMemZone, *gNumLumps * sizeof(lumpinfo_t), PU_STATIC, nullptr);

    // Read the lump info array
    SeekAndTellFile(*gMainWadFileIdx, wadinfo->infotableofs, PsxCd_SeekMode::SET);
    ReadFile(*gMainWadFileIdx, gpLumpInfo->get(), *gNumLumps * sizeof(lumpinfo_t));

    // Alloc and zero init the lump cache pointers list and an array of bools to say whether each lump is compressed or not.
    static_assert(sizeof(bool) == 1, "Expect bool to be 1 byte!");    

    *gpLumpCache = (VmPtr<void>*) Z_Malloc(**gpMainMemZone, *gNumLumps * sizeof(VmPtr<void>), PU_STATIC, nullptr);
    *gpbIsUncompressedLump = (bool*) Z_Malloc(**gpMainMemZone, *gNumLumps * sizeof(bool), PU_STATIC, nullptr);

    D_memset(gpLumpCache->get(), std::byte(0), *gNumLumps * sizeof(VmPtr<void>));
    D_memset(gpbIsUncompressedLump->get(), std::byte(0), *gNumLumps * sizeof(bool));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the lump index for the given lump name (case insensitive) or -1 if not found
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t W_CheckNumForName(const char* const name) noexcept {
    // Copy the given name and uppercase for case insensitivity
    char nameUpper[MAXLUMPNAME + 1];
    D_memset(nameUpper, (std::byte) 0, sizeof(nameUpper));
    D_strncpy(nameUpper, name, MAXLUMPNAME);
    nameUpper[MAXLUMPNAME] = 0;
    strupr(nameUpper);

    // Get the two words of the name for faster comparison
    const uint32_t findNameW1 = (uint32_t&) nameUpper[0];
    const uint32_t findNameW2 = (uint32_t&) nameUpper[4];

    // Try to find the given lump name and compare names using 32-bit words rather than single chars
    lumpinfo_t* pLump = (*gpLumpInfo).get();
    int32_t lumpIdx = 0;

    while (lumpIdx < *gNumLumps) {
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
// Return the lump index for the given lump name (case insensitive) or fail with a fatal error if not found
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
    if (lumpNum >= *gNumLumps) {
        I_Error("W_LumpLength: %i >= numlumps", lumpNum);
    }

    const lumpinfo_t& lump = (*gpLumpInfo)[lumpNum];
    return lump.size;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the requested lump number from the main IWAD into the given buffer.
// The buffer must be big enough to accomodate the data.
// Optionally, decompression can be disabled.
//------------------------------------------------------------------------------------------------------------------------------------------
void W_ReadLump(const int32_t lumpNum, void* const pDest, const bool bDecompress) noexcept {    
    // Sanity check the lump number is range.
    // Note: modified this check to disallow reading the last lump.
    // This code relies on getting the NEXT lump after the requested one in order to determine sizes.
    // We shouldn't be reading the last WAD lump anyway as it's an end marker...
    #if PC_PSX_DOOM_MODS
        if (lumpNum + 1 >= *gNumLumps) {
            I_Error("W_ReadLump: %i + 1 >= numlumps", lumpNum);
        }
    #else
        if (lumpNum >= *gNumLumps) {
            I_Error("W_ReadLump: %i >= numlumps", lumpNum);
        }
    #endif
    
    // The lump requested and the next one after that
    const lumpinfo_t& lump = (*gpLumpInfo)[lumpNum];
    const lumpinfo_t& nextLump = (*gpLumpInfo)[lumpNum + 1];

    // Do we need to decompress? Decompress if specified and if the lumpname has the special bit
    // set in the first character, indicating that the lump is compressed.
    const uint32_t sizeToRead = nextLump.filepos - lump.filepos;

    if (bDecompress && (((uint8_t) lump.name.chars[0] & 0x80u) != 0)) {
        // Decompression needed, must alloc a temp buffer for the compressed data before reading and decompressing!
        void* const pTmpBuffer = Z_EndMalloc(**gpMainMemZone, lump.size, PU_STATIC, nullptr);        
        
        SeekAndTellFile(*gMainWadFileIdx, lump.filepos, PsxCd_SeekMode::SET);
        ReadFile(*gMainWadFileIdx, pTmpBuffer, sizeToRead);
        decode(pTmpBuffer, pDest);

        Z_Free2(**gpMainMemZone, pTmpBuffer);
    } else {
        // No decompression needed, can just read straight into the output buffer
        SeekAndTellFile(*gMainWadFileIdx, lump.filepos, PsxCd_SeekMode::SET);
        ReadFile(*gMainWadFileIdx, pDest, sizeToRead);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cache/load the specified lump number and return it's pointer.
// If the lump is already cached does nothing.
// Optionally, lump decompression can be skipped.
// Note that lump caching is *NOT* permitted during regular gameplay!
//------------------------------------------------------------------------------------------------------------------------------------------
void* W_CacheLumpNum(const int32_t lumpNum, const int16_t allocTag, const bool bDecompress) noexcept {
    // Sanity check the lump number is in range.
    // Note: modified this check to disallow caching the last lump.
    // This code relies on getting the NEXT lump after the requested one in order to determine sizes.
    // We shouldn't be reading the last WAD lump anyway as it's an end marker...
    #if PC_PSX_DOOM_MODS
        if (lumpNum + 1 >= *gNumLumps) {
            I_Error("W_CacheLumpNum: %i + 1 >= numlumps", lumpNum);
        }
    #else
        if (lumpNum >= *gNumLumps) {
            I_Error("W_CacheLumpNum: %i >= numlumps", lumpNum);
        }
    #endif

    // If the lump is already loaded then we don't need to do anything
    VmPtr<void>& lumpCacheEntry = (*gpLumpCache)[lumpNum];
    
    if (!lumpCacheEntry) {
        // If the level loading is done then we should NOT be loading lumps during gameplay.
        // Because CD-ROM I/O is so slow, this would cause very serious stalls and slowdowns during gameplay, so consider it a fatal error.
        // Unlike PC DOOM the PSX version cannot simply stream in lumps on the fly...
        if (*gbIsLevelDataCached) {
            I_Error("cache miss on lump %i", lumpNum);
        }
    
        // Figure out how much data we will need to allocate. If we are decompressing then this will be the decompressed size, otherwise
        // it will be the actual size of the lump before decompression is applied:
        lumpinfo_t& lumpInfo = (*gpLumpInfo)[lumpNum];
        int32_t sizeToRead;
        
        if (bDecompress) {
            sizeToRead = lumpInfo.size;
        } else {
            lumpinfo_t& nextLumpInfo = (*gpLumpInfo)[lumpNum + 1];
            const int32_t lumpStorageSize = nextLumpInfo.filepos - lumpInfo.filepos;
            sizeToRead = lumpStorageSize;
        }

        // Alloc RAM for the lump and read it
        Z_Malloc(**gpMainMemZone, sizeToRead, allocTag, &lumpCacheEntry);
        W_ReadLump(lumpNum, lumpCacheEntry.get(), bDecompress);

        // Save whether the lump is compressed or not.
        // If the lump is compressed then the highest bit of the first character in the name will be set:
        if ((lumpInfo.name.chars[0] & 0x80) != 0) {
            (*gpbIsUncompressedLump)[lumpNum] = bDecompress;
        } else {
            (*gpbIsUncompressedLump)[lumpNum] = true;
        }
    }

    return lumpCacheEntry.get();
}

void _thunk_W_CacheLumpNum() noexcept {
    v0 = ptrToVmAddr(W_CacheLumpNum((int32_t) a0, (int16_t) a1, (a2 != 0)));
}

void W_CacheLumpName() noexcept {
loc_800319E4:
    sp -= 0x30;
    t1 = a0;
    sw(s1, sp + 0x24);
    s1 = a1;
    sw(s2, sp + 0x28);
    s2 = a2;
    a0 = sp + 0x10;
    a1 = t1;
    a2 = sp + 0x18;
    sw(ra, sp + 0x2C);
    sw(s0, sp + 0x20);
    sw(0, sp + 0x10);
    sw(0, sp + 0x14);
loc_80031A18:
    v0 = lbu(a1);
    v1 = v0;
    if (v0 == 0) goto loc_80031A50;
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    a1++;
    if (v0 == 0) goto loc_80031A3C;
    v1 -= 0x20;
loc_80031A3C:
    sb(v1, a0);
    a0++;
    v0 = (i32(a0) < i32(a2));
    if (v0 != 0) goto loc_80031A18;
loc_80031A50:
    a3 = lw(sp + 0x10);
    a2 = lw(sp + 0x14);
    v1 = *gNumLumps;
    v0 = *gpLumpInfo;
    a0 = 0;
    if (i32(v1) <= 0) goto loc_80031AA8;
    t0 = -0x81;                                         // Result = FFFFFF7F
    a1 = v1;
    v1 = v0 + 8;
loc_80031A74:
    v0 = lw(v1 + 0x4);
    if (v0 != a2) goto loc_80031A98;
    v0 = lw(v1);
    v0 &= t0;
    if (v0 == a3) goto loc_80031AC0;
loc_80031A98:
    a0++;
    v0 = (i32(a0) < i32(a1));
    v1 += 0x10;
    if (v0 != 0) goto loc_80031A74;
loc_80031AA8:
    v1 = -1;                                            // Result = FFFFFFFF
loc_80031AAC:
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 == v0) goto loc_80031AC8;
    s0 = v1;
    goto loc_80031AD8;
loc_80031AC0:
    v1 = a0;
    goto loc_80031AAC;
loc_80031AC8:
    I_Error("W_GetNumForName: %s not found!", vmAddrToPtr<const char>(t1));
loc_80031AD8:
    a0 = s0;
    a1 = s1;
    a2 = s2;
    _thunk_W_CacheLumpNum();
    ra = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void W_OpenMapWad() noexcept {
loc_80031B04:
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    _thunk_OpenFile();
    s0 = v0;
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    a2 = 2;                                             // Result = 00000002
    _thunk_SeekAndTellFile();
    s1 = v0;
    a1 = s1;
    a2 = 1;                                             // Result = 00000001
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_EndMalloc();
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    sw(v0, gp + 0x938);                                 // Store to: gpMapWadFileData (80077F18)
    a2 = 0;                                             // Result = 00000000
    _thunk_SeekAndTellFile();
    a0 = s0;
    a1 = lw(gp + 0x938);                                // Load from: gpMapWadFileData (80077F18)
    a2 = s1;
    _thunk_ReadFile();
    a0 = s0;
    _thunk_CloseFile();
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7BE8;                                       // Result = STR_IWAD[0] (80077BE8)
    a0 = lw(gp + 0x938);                                // Load from: gpMapWadFileData (80077F18)
    a2 = 4;                                             // Result = 00000004
    D_strncasecmp();
    if (v0 == 0) goto loc_80031BA0;
    I_Error("W_OpenMapWad: invalid map IWAD id");
loc_80031BA0:
    v0 = lw(gp + 0x938);                                // Load from: gpMapWadFileData (80077F18)
    v1 = lw(v0 + 0x4);
    a0 = lw(v0 + 0x8);
    sw(v1, gp + 0xB3C);                                 // Store to: gNumMapWadLumps (8007811C)
    v1 = v0 + a0;
    sw(v1, gp + 0xAE8);                                 // Store to: gpMapWadDirectory (800780C8)
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void W_MapLumpLength() noexcept {
loc_80031BD4:
    v0 = lw(gp + 0xB3C);                                // Load from: gNumMapWadLumps (8007811C)
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    v0 = (i32(s0) < i32(v0));
    sw(ra, sp + 0x14);
    if (v0 != 0) goto loc_80031C00;
    I_Error("W_MapLumpLength: %i out of range", (int32_t) s0);
loc_80031C00:
    v1 = lw(gp + 0xAE8);                                // Load from: gpMapWadDirectory (800780C8)
    v0 = s0 << 4;
    v0 += v1;
    v0 = lw(v0 + 0x4);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void W_MapGetNumForName() noexcept {
loc_80031C24:
    sp -= 0x10;
    a1 = sp;
    a2 = sp + 8;
    sw(0, sp);
    sw(0, sp + 0x4);
loc_80031C38:
    v0 = lbu(a0);
    if (v0 == 0) goto loc_80031C78;
    v1 = lbu(a0);
    v0 = v1 - 0x61;
    v0 = (v0 < 0x1A);
    a0++;
    if (v0 == 0) goto loc_80031C64;
    v1 -= 0x20;
loc_80031C64:
    sb(v1, a1);
    a1++;
    v0 = (i32(a1) < i32(a2));
    if (v0 != 0) goto loc_80031C38;
loc_80031C78:
    a3 = lw(sp);
    a2 = lw(sp + 0x4);
    v1 = lw(gp + 0xB3C);                                // Load from: gNumMapWadLumps (8007811C)
    v0 = lw(gp + 0xAE8);                                // Load from: gpMapWadDirectory (800780C8)
    a0 = 0;                                             // Result = 00000000
    if (i32(v1) <= 0) goto loc_80031CD0;
    t0 = -0x81;                                         // Result = FFFFFF7F
    a1 = v1;
    v1 = v0 + 8;
loc_80031C9C:
    v0 = lw(v1 + 0x4);
    if (v0 != a2) goto loc_80031CC0;
    v0 = lw(v1);
    v0 &= t0;
    {
        const bool bJump = (v0 == a3);
        v0 = a0;
        if (bJump) goto loc_80031CD4;
    }
loc_80031CC0:
    a0++;
    v0 = (i32(a0) < i32(a1));
    v1 += 0x10;
    if (v0 != 0) goto loc_80031C9C;
loc_80031CD0:
    v0 = -1;                                            // Result = FFFFFFFF
loc_80031CD4:
    sp += 0x10;
    return;
}

void W_ReadMapLump() noexcept {
loc_80031CE0:
    v0 = lw(gp + 0xB3C);                                // Load from: gNumMapWadLumps (8007811C)
    sp -= 0x30;
    sw(s0, sp + 0x20);
    s0 = a0;
    sw(s2, sp + 0x28);
    s2 = a1;
    sw(s1, sp + 0x24);
    s1 = a2;
    v0 = (i32(s0) < i32(v0));
    sw(ra, sp + 0x2C);
    if (v0 != 0) goto loc_80031D1C;
    I_Error("W_ReadMapLump: lump %d out of range", (int32_t) s0);
loc_80031D1C:
    v1 = lw(gp + 0xAE8);                                // Load from: gpMapWadDirectory (800780C8)
    v0 = s0 << 4;
    v1 += v0;
    if (s1 == 0) goto loc_80031D58;
    v0 = lbu(v1 + 0x8);
    v0 &= 0x80;
    a1 = s2;
    if (v0 == 0) goto loc_80031D58;
    v0 = lw(gp + 0x938);                                // Load from: gpMapWadFileData (80077F18)
    a0 = lw(v1);
    a0 += v0;
    _thunk_decode();
    goto loc_80031D74;
loc_80031D58:
    a0 = s2;
    a1 = lw(gp + 0x938);                                // Load from: gpMapWadFileData (80077F18)
    v0 = lw(v1);
    a2 = lw(v1 + 0x10);
    a1 += v0;
    a2 -= v0;
    _thunk_D_memcpy();
loc_80031D74:
    ra = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
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

void _thunk_decode() noexcept {
    decode(vmAddrToPtr<const uint8_t>(a0), vmAddrToPtr<uint8_t>(a1));
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
            const int32_t numRepeatedBytes = (srcByte2 & 0xF) + 1;

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
