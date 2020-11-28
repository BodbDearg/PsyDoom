//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): LCD (Linear CD sound samples file) loading code.
// These are the files which contain all of the sounds used for the game and for music instruments.
//
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "lcdload.h"

#include "Finally.h"
#include "PcPsx/ProgArgs.h"
#include "psxspu.h"
#include "PsyQ/LIBSPU.h"
#include "wessapi.h"
#include "wessarc.h"

#include <cstring>

// Maximum number of sounds that can be in an LCD file
static constexpr uint32_t MAX_LCD_SOUNDS = 100;

static patch_group_data*    gpWess_lcd_load_patchGroup;         // Saved reference to the PSX driver patch group
static patch*               gpWess_lcd_load_patches;            // Saved reference to the master status patches list
static patch_voice*         gpWess_lcd_load_patchVoices;        // Saved reference to the master status patch voices list
static patch_sample*        gpWess_lcd_load_patchSamples;       // Saved reference to the master status patch samples list
static drum_patch*          gpWess_lcd_load_drumPatches;        // Saved reference to the master status drum patches list
static int32_t              gWess_lcd_load_soundBytesLeft;      // How many bytes there are to left to upload to the SPU for the current sound being loaded
static uint32_t             gWess_lcd_load_soundNum;            // Which sound number in the LCD file is being loaded
static uint16_t             gWess_lcd_load_numSounds;           // How many sounds are being loaded in total from the LCD file
static uint8_t*             gpWess_lcd_load_headerBuf;          // This buffer contains the LCD file header (2048 bytes max)
static uint32_t             gWess_lcd_load_samplePos;           // This is the current address in SPU RAM to upload sound to: incremented as we load

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the LCD loader with the specified master status struct
//------------------------------------------------------------------------------------------------------------------------------------------
bool wess_dig_lcd_loader_init(master_status_structure* const pMStat) noexcept {
    // If there is no master status struct then we cannot initialize
    if (!pMStat)
        return false;
    
    // Try to find the patch group in the currently loaded module for the PlayStation
    module_data& module = *pMStat->pmodule;
    patch_group_data* pPatchGroup = nullptr;

    for (uint8_t patchGrpIdx = 0; patchGrpIdx < module.hdr.num_patch_groups; ++patchGrpIdx) {
        patch_group_data& patchGrp = pMStat->ppatch_groups[patchGrpIdx];

        if (patchGrp.hdr.driver_id == PSX_ID) {
            pPatchGroup = &patchGrp;
            break;
        }
    }

    // If we didn't find the patch group for the PSX driver then initialization failed
    gpWess_lcd_load_patchGroup = pPatchGroup;

    if (!pPatchGroup)
        return false;

    // Save pointers to the various data structures for the patch group
    {
        uint8_t* pPatchesData = pPatchGroup->pdata;
        gpWess_lcd_load_patches = (patch*) pPatchesData;
        pPatchesData += sizeof(patch) * pPatchGroup->hdr.num_patches;
        
        wess_align_byte_ptr(pPatchesData, alignof(patch_voice));
        gpWess_lcd_load_patchVoices = (patch_voice*) pPatchesData;
        pPatchesData += sizeof (patch_voice) * pPatchGroup->hdr.num_patch_voices;

        wess_align_byte_ptr(pPatchesData, alignof(patch_sample));
        gpWess_lcd_load_patchSamples = (patch_sample*) pPatchesData;
        pPatchesData += sizeof(patch_sample) * pPatchGroup->hdr.num_patch_samples;

        wess_align_byte_ptr(pPatchesData, alignof(drum_patch));
        gpWess_lcd_load_drumPatches = (drum_patch*) pPatchesData;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the SPU address for the given patch sample.
// This is used by DOOM to 'unload' sounds by setting them to address '0'.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_dig_set_sample_position(const int32_t patchSampleIdx, const uint32_t spuAddr) noexcept {
    if (gpWess_lcd_load_patchSamples) {
        gpWess_lcd_load_patchSamples[patchSampleIdx].spu_addr = spuAddr;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// The name of this function is somewhat confusing...
// Uploads/writes sound data from a buffered CD sector (2048 bytes) to SPU RAM at the specified address.
//
// Also handles transitioning from one sound in the LCD file to the next as each sound upload is completed.
// Optionally, details about the uploaded sound(s) are saved to the given sample block upon successful upload.
// The details for previous (still valid) uploads can also be forcefully overwritten by the 'bOverride' flag.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_dig_lcd_data_read(
    uint8_t* const pSectorData,
    const uint32_t destSpuAddr,
    SampleBlock* const pSampleBlock,
    const bool bOverride
) noexcept {
    // Some helpers to make the code easier below
    patch_sample* const pPatchSamples = gpWess_lcd_load_patchSamples;
    const uint16_t* pPatchSampleIndices = (const uint16_t*) gpWess_lcd_load_headerBuf;

    // Loop vars: number of bytes left to read from the input sector buffer, number of bytes written to the SPU and offset in the sector data
    uint32_t sectorBytesLeft = CDROM_SECTOR_SIZE;
    int32_t bytesWritten = 0;
    uint32_t sndDataOffset = 0;
    
    // Continue uploading sound bytes to the SPU until we have exhausted all bytes in the given input sector buffer
    while (sectorBytesLeft > 0) {
        // Do we need to switch over to uploading a new sound?
        // If we've uploaded all the bytes from the current sound then do that:
        if (gWess_lcd_load_soundBytesLeft == 0) {
            // Commit the details for the sound we just finished uploading, unless we haven't yet started on uploading a sound.
            // Sound number '0' is invalid and the LCD file header at this location contains the number of sounds, not the patch sample index for a sound.
            if (gWess_lcd_load_soundNum > 0) {
                const uint16_t patchSampleIdx = pPatchSampleIndices[gWess_lcd_load_soundNum];
                patch_sample& patchSample = pPatchSamples[patchSampleIdx];

                // Only save the details of the sound if there was not a previous sound saved, or if we are forcing it
                if ((patchSample.spu_addr == 0) || bOverride) {
                    patchSample.spu_addr = gWess_lcd_load_samplePos;

                    // Save the details of the sound to the sample block also, if it's given
                    if (pSampleBlock) {
                        pSampleBlock->patch_sample_idx[pSampleBlock->num_samples] = patchSampleIdx;
                        pSampleBlock->sample_spu_addr_8[pSampleBlock->num_samples] = (uint16_t)(patchSample.spu_addr / 8);
                        pSampleBlock->num_samples++;
                    }
                }
            }

            // If we have finished uploading all sounds then we are done
            if (gWess_lcd_load_soundNum >= gWess_lcd_load_numSounds)
                return bytesWritten;
            
            // Otherwise switch over to reading and uploading the next sound.
            // Set the number of bytes to read, and where to upload it to in SPU ram when we are done.
            gWess_lcd_load_soundNum++;

            const int32_t nextPatchSampleIdx = pPatchSampleIndices[gWess_lcd_load_soundNum];
            patch_sample& nextPatchSample = pPatchSamples[nextPatchSampleIdx];

            gWess_lcd_load_soundBytesLeft = nextPatchSample.size;
            gWess_lcd_load_samplePos = destSpuAddr + sndDataOffset;

            // If uploading this sound would cause us to go beyond the bounds of SPU ram then do not try to upload this sound
            #if PSYDOOM_MODS
                // PsyDoom: I think this condition was slightly wrong?
                const bool bInsufficientSpuRam = ((int32_t) destSpuAddr + sndDataOffset + nextPatchSample.size  > (int32_t) gPsxSpu_sram_end);
            #else
                const bool bInsufficientSpuRam = ((int32_t) destSpuAddr + nextPatchSample.size > (int32_t)(gPsxSpu_sram_end + sndDataOffset));
            #endif

            if (bInsufficientSpuRam) {
                gWess_lcd_load_soundBytesLeft = 0;
                return bytesWritten;
            }

            // If there are no more bytes left to read then we are done: this check here is actually not neccessary?
            if (sectorBytesLeft == 0)
                return bytesWritten;
        }

        // Upload to the SPU whatever samples we can from the sector.
        // Either upload the whole sector or just a part of it (if it's bigger than the rest of the sound)
        const uint16_t patchSampleIdx = pPatchSampleIndices[gWess_lcd_load_soundNum];
        const patch_sample& patchSample = pPatchSamples[patchSampleIdx];

        const uint32_t sndBytesLeft = (uint32_t) gWess_lcd_load_soundBytesLeft;
        const uint32_t writeSize = (sectorBytesLeft > sndBytesLeft) ? sndBytesLeft : sectorBytesLeft;

        if ((patchSample.spu_addr == 0) || bOverride) {
            LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
            LIBSPU_SpuSetTransferStartAddr(destSpuAddr + sndDataOffset);
            LIBSPU_SpuWrite(pSectorData + sndDataOffset, writeSize);
            bytesWritten += writeSize;
        }

        gWess_lcd_load_soundBytesLeft -= writeSize;
        sndDataOffset += writeSize;
        sectorBytesLeft -= writeSize;
    }

    return bytesWritten;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads the specified LCD sound file to SPU RAM starting at the given address.
// Returns the number of bytes uploaded to the SPU or '0' on failure.
// Optionally, details for the uploaded sounds can be saved to the given sample block.
// The 'override' flag also specifies whether existing sound patches are to have their details overwritten or not.
//
// PsyDoom: this function has been rewritten to get rid of low level I/O stuff, for the original version see the 'Old' folder.
// The goal of the rewrite is to enable modding of the game with new .LCD files for custom maps, since the 'psxcd' I/O functions support
// replacing original game files with alternate versions on the user's computer.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_dig_lcd_load(
    const CdFileId lcdFileToLoad,
    const uint32_t destSpuAddr,
    SampleBlock* const pSampleBlock,
    const bool bOverride
) noexcept {
    // Ignore this command in headless mode
    if (ProgArgs::gbHeadlessMode)
        return 0;

    // Open the LCD file firstly and abort if that fails or the file handle returned is invalid
    PsxCd_File* const pLcdFile = psxcd_open(lcdFileToLoad);

    if (!pLcdFile)
        return 0;

    auto closeLcdFileOnExit = finally([&]() noexcept {
        psxcd_close(*pLcdFile);
    });

    // Read the LCD file header to sector buffer 1
    struct LCDHeader {
        uint16_t    numPatchSamples;
        uint16_t    patchSampleIndices[MAX_LCD_SOUNDS];
    };

    LCDHeader* const pLcdHeader = (LCDHeader*) gWess_sectorBuffer1;

    if (psxcd_read(pLcdHeader, sizeof(LCDHeader), *pLcdFile) != sizeof(LCDHeader))
        return 0;

    // If the number of sounds is not valid then abort
    if (pLcdHeader->numPatchSamples > MAX_LCD_SOUNDS)
        return 0;

    // Set the number of sounds to load globally and which buffer contains the LCD header.
    // Also initialize other variables used by 'wess_dig_lcd_data_read':
    gWess_lcd_load_numSounds = pLcdHeader->numPatchSamples;
    gpWess_lcd_load_headerBuf = gWess_sectorBuffer1;

    gWess_lcd_load_soundNum = 0;
    gWess_lcd_load_samplePos = destSpuAddr;
    gWess_lcd_load_soundBytesLeft = 0;

    // Seek to the first sound data sector in the file and continue reading sound data until we are done
    if (psxcd_seek(*pLcdFile, CDROM_SECTOR_SIZE, PsxCd_SeekMode::SET) != 0)
        return 0;

    // Read all of the sound data and upload to the SPU using sector buffer 2 as a temporary.
    // Note: we've already consumed 1 sector from the file, so the byte count left is adjusted accordingly.
    int32_t lcdBytesLeft = pLcdFile->size - CDROM_SECTOR_SIZE;
    int32_t numSpuBytesWritten = 0;

    while (lcdBytesLeft > 0) {
        // Read this sector from the LCD file and the number of bytes left is smaller then read that amount instead
        uint8_t* const sectorBuffer = gWess_sectorBuffer2;
        const int32_t readSize = (lcdBytesLeft < CDROM_SECTOR_SIZE) ? lcdBytesLeft : CDROM_SECTOR_SIZE;
        psxcd_read(sectorBuffer, readSize, *pLcdFile);

        if (readSize < CDROM_SECTOR_SIZE) {
            // When we are not filling part of the buffer then zero it out just for consistency
            std::memset(sectorBuffer + readSize, 0, (size_t) CDROM_SECTOR_SIZE - readSize);
        }

        numSpuBytesWritten += wess_dig_lcd_data_read(sectorBuffer, destSpuAddr + numSpuBytesWritten, pSampleBlock, bOverride);
        lcdBytesLeft -= readSize;
    }

    return numSpuBytesWritten;
}
