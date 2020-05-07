//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): LCD (Linear CD sound samples file) loading code.
// These are the files which contain all of the sounds used for the game and for music instruments.
//
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "lcdload.h"

#include "Doom/cdmaptbl.h"
#include "PcPsx/Finally.h"
#include "PcPsx/ProgArgs.h"
#include "psxcd.h"
#include "psxspu.h"
#include "PsyQ/LIBCD.h"
#include "PsyQ/LIBSPU.h"
#include "wessarc.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <cstring>
END_THIRD_PARTY_INCLUDES

// Maximum number of sounds that can be in an LCD file
static constexpr uint32_t MAX_LCD_SOUNDS = 100;

static const VmPtr<PsxCd_File>                  gWess_open_lcd_file(0x8007F2E0);                // The currently open LCD file
static const VmPtr<VmPtr<patch_group_data>>     gpWess_lcd_load_patchGroup(0x80075AD8);         // Saved reference to the PSX driver patch group
static const VmPtr<VmPtr<patch>>                gpWess_lcd_load_patches(0x80075AC8);            // Saved reference to the master status patches list
static const VmPtr<VmPtr<patch_voice>>          gpWess_lcd_load_patchVoices(0x80075ACC);        // Saved reference to the master status patch voices list
static const VmPtr<VmPtr<patch_sample>>         gpWess_lcd_load_patchSamples(0x80075AD0);       // Saved reference to the master status patch samples list
static const VmPtr<VmPtr<drum_patch>>           gpWess_lcd_load_drumPatches(0x80075AD4);        // Saved reference to the master status drum patches list
static const VmPtr<int32_t>                     gWess_lcd_load_soundBytesLeft(0x80075AE4);      // How many bytes there are to left to upload to the SPU for the current sound being loaded
static const VmPtr<uint32_t>                    gWess_lcd_load_soundNum(0x80075AE0);            // Which sound number in the LCD file is being loaded
static const VmPtr<uint16_t>                    gWess_lcd_load_numSounds(0x80075AE8);           // How many sounds are being loaded in total from the LCD file
static const VmPtr<VmPtr<uint8_t>>              gpWess_lcd_load_headerBuf(0x80075AEC);          // This buffer contains the LCD file header (2048 bytes max)
static const VmPtr<uint32_t>                    gWess_lcd_load_samplePos(0x80075ADC);           // This is the current address in SPU RAM to upload sound to: incremented as we load

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
    *gpWess_lcd_load_patchGroup = pPatchGroup;

    if (!pPatchGroup)
        return false;

    // Save pointers to the various data structures for the patch group
    {
        uint8_t* pPatchesData = pPatchGroup->pdata.get();

        *gpWess_lcd_load_patches = (patch*) pPatchesData;
        pPatchesData += sizeof(patch) * pPatchGroup->hdr.num_patches;
        
        *gpWess_lcd_load_patchVoices = (patch_voice*) pPatchesData;
        pPatchesData += sizeof (patch_voice) * pPatchGroup->hdr.num_patch_voices;

        *gpWess_lcd_load_patchSamples = (patch_sample*) pPatchesData;
        pPatchesData += sizeof(patch_sample) * pPatchGroup->hdr.num_patch_samples;

        *gpWess_lcd_load_drumPatches = (drum_patch*) pPatchesData;
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets the SPU address for the given patch sample.
// This is used by DOOM to 'unload' sounds by setting them to address '0'.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_dig_set_sample_position(const int32_t patchSampleIdx, const uint32_t spuAddr) noexcept {
    if (gpWess_lcd_load_patchSamples) {
        gpWess_lcd_load_patchSamples->get()[patchSampleIdx].spu_addr = spuAddr;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Opens the specified file for the LCD loader.
// PC-PSX: removed to fix compiler warnings as it is unreferenced and no longer used.
//------------------------------------------------------------------------------------------------------------------------------------------
#if !PC_PSX_DOOM_MODS
static PsxCd_File* wess_dig_lcd_data_open(const CdMapTbl_File file) noexcept {
    PsxCd_File* const pFile = psxcd_open(file);
    *gWess_open_lcd_file = *pFile;
    return pFile;
}
#endif

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
    patch_sample* const pPatchSamples = gpWess_lcd_load_patchSamples->get();
    const uint16_t* pPatchSampleIndices = (const uint16_t*) gpWess_lcd_load_headerBuf->get();

    // Loop vars: number of bytes left to read from the input sector buffer, number of bytes written to the SPU and offset in the sector data
    uint32_t sectorBytesLeft = CD_SECTOR_SIZE;
    int32_t bytesWritten = 0;
    uint32_t sndDataOffset = 0;
    
    // Continue uploading sound bytes to the SPU until we have exhausted all bytes in the given input sector buffer
    while (sectorBytesLeft > 0) {
        // Do we need to switch over to uploading a new sound?
        // If we've uploaded all the bytes from the current sound then do that:
        if (*gWess_lcd_load_soundBytesLeft == 0) {
            // Commit the details for the sound we just finished uploading, unless we haven't yet started on uploading a sound.
            // Sound number '0' is invalid and the LCD file header at this location contains the number of sounds, not the patch index for a sound.
            if (*gWess_lcd_load_soundNum > 0) {
                const uint16_t patchSampleIdx = pPatchSampleIndices[*gWess_lcd_load_soundNum];
                patch_sample& patchSample = pPatchSamples[patchSampleIdx];

                // Only save the details of the sound if there was not a previous sound saved, or if we are forcing it
                if ((patchSample.spu_addr == 0) || bOverride) {
                    patchSample.spu_addr = *gWess_lcd_load_samplePos;

                    // Save the details of the sound to the sample block also, if it's given
                    if (pSampleBlock) {
                        pSampleBlock->patch_sample_idx[pSampleBlock->num_samples] = patchSampleIdx;
                        pSampleBlock->sample_spu_addr_8[pSampleBlock->num_samples] = (uint16_t)(patchSample.spu_addr / 8);
                        pSampleBlock->num_samples++;
                    }
                }
            }

            // If we have finished uploading all sounds then we are done
            if (*gWess_lcd_load_soundNum >= *gWess_lcd_load_numSounds)
                return bytesWritten;
            
            // Otherwise switch over to reading and uploading the next sound.
            // Set the number of bytes to read, and where to upload it to in SPU ram when we are done.
            *gWess_lcd_load_soundNum += 1;

            const int32_t nextPatchSampleIdx = pPatchSampleIndices[*gWess_lcd_load_soundNum];
            patch_sample& nextPatchSample = pPatchSamples[nextPatchSampleIdx];

            *gWess_lcd_load_soundBytesLeft = nextPatchSample.size;
            *gWess_lcd_load_samplePos = destSpuAddr + sndDataOffset;

            // If uploading this sound would cause us to go beyond the bounds of SPU ram then do not try to upload this sound
            #if PC_PSX_DOOM_MODS
                // PC-PSX: I think this condition was slightly wrong?
                const bool bInsufficientSpuRam = ((int32_t) destSpuAddr + sndDataOffset + nextPatchSample.size  > (int32_t) *gPsxSpu_sram_end);
            #else
                const bool bInsufficientSpuRam = ((int32_t) destSpuAddr + nextPatchSample.size > (int32_t)(*gPsxSpu_sram_end + sndDataOffset));
            #endif

            if (bInsufficientSpuRam) {
                *gWess_lcd_load_soundBytesLeft = 0;
                return bytesWritten;
            }

            // If there are no more bytes left to read then we are done: this check here is actually not neccessary?
            if (sectorBytesLeft == 0)
                return bytesWritten;
        }

        // Upload to the SPU whatever samples we can from the sector.
        // Either upload the whole sector or just a part of it (if it's bigger than the rest of the sound)
        const uint16_t patchSampleIdx = pPatchSampleIndices[*gWess_lcd_load_soundNum];
        const patch_sample& patchSample = pPatchSamples[patchSampleIdx];

        const uint32_t sndBytesLeft = (uint32_t) *gWess_lcd_load_soundBytesLeft;
        const uint32_t writeSize = (sectorBytesLeft > sndBytesLeft) ? sndBytesLeft : sectorBytesLeft;

        if ((patchSample.spu_addr == 0) || bOverride) {
            LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
            LIBSPU_SpuSetTransferStartAddr(destSpuAddr + sndDataOffset);
            LIBSPU_SpuWrite(pSectorData + sndDataOffset, writeSize);
            bytesWritten += writeSize;
        }

        *gWess_lcd_load_soundBytesLeft -= writeSize;
        sndDataOffset += writeSize;
        sectorBytesLeft -= writeSize;
    }

    return bytesWritten;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for up to 8 seconds for all CDROM commands to finish executing.
// Returns '1' if there was an error or timeout waiting for all commands to finish up, '0' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_dig_lcd_psxcd_sync() noexcept {
    // PC-PSX: don't need to sync with the CDROM anymore since all commands execute synchronously.
    // Just no-op and return '0' for success...
    #if PC_PSX_DOOM_MODS
        return 0;
    #else
        const uint32_t timeoutMs = *gWess_Millicount + 8000;
    
        while (*gWess_Millicount < timeoutMs) {
            // Note: the sync result was a global but it wasn't used anywhere else so I made it local instead...
            uint8_t syncResult[8];
            const CdlSyncStatus status = LIBCD_CdSync(1, syncResult);

            if (status == CdlDiskError) {
                LIBCD_CdFlush();
                return 1;
            }

            if (status == CdlComplete)
                return 0;
        }

        return 1;
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads the specified LCD sound file to SPU RAM starting at the given address.
// Returns the number of bytes uploaded to the SPU or '0' on failure.
// Optionally, details for the uploaded sounds can be saved to the given sample block.
// The 'override' flag also specifies whether existing sound patches are to have their details overwritten or not.
//
// Note: I've completely rewritten this function for PsyDoom to get rid of all the low level I/O stuff.
// The goal of the rewrite is to enable modding of the game with new .LCD files for custom maps, since the 'psxcd' I/O functions support
// replacing original game files with alternate versions on the user's computer.
//------------------------------------------------------------------------------------------------------------------------------------------
#if PC_PSX_DOOM_MODS

int32_t wess_dig_lcd_load(
    const CdMapTbl_File lcdFileToLoad,
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

    if (pLcdFile->file.pos == 0)
        return 0;

    // Read the LCD file header to sector buffer 1
    struct LCDHeader {
        uint16_t    numPatchSamples;
        uint16_t    patchSampleIndices[MAX_LCD_SOUNDS];
    };

    LCDHeader* const pLcdHeader = (LCDHeader*) gWess_sectorBuffer1.get();

    if (psxcd_read(pLcdHeader, sizeof(LCDHeader), *pLcdFile) != sizeof(LCDHeader))
        return 0;

    // If the number of sounds is not valid then abort
    if (pLcdHeader->numPatchSamples > MAX_LCD_SOUNDS)
        return 0;

    // Set the number of sounds to load globally and which buffer contains the LCD header.
    // Also initialize other variables used by 'wess_dig_lcd_data_read':
    *gWess_lcd_load_numSounds = pLcdHeader->numPatchSamples;
    *gpWess_lcd_load_headerBuf = gWess_sectorBuffer1.get();

    *gWess_lcd_load_soundNum = 0;
    *gWess_lcd_load_samplePos = destSpuAddr;
    *gWess_lcd_load_soundBytesLeft = 0;

    // Seek to the first sound data sector in the file and continue reading sound data until we are done
    if (psxcd_seek(*pLcdFile, CD_SECTOR_SIZE, PsxCd_SeekMode::SET) != 0)
        return 0;

    // Read all of the sound data and upload to the SPU using sector buffer 2 as a temporary
    int32_t lcdBytesLeft = pLcdFile->file.size;
    int32_t numSpuBytesWritten = 0;

    while (lcdBytesLeft > 0) {
        // Read this sector from the LCD file and the number of bytes left is smaller then read that amount instead
        uint8_t* const sectorBuffer = gWess_sectorBuffer2.get();
        const int32_t readSize = (lcdBytesLeft < CD_SECTOR_SIZE) ? lcdBytesLeft : CD_SECTOR_SIZE;
        psxcd_read(sectorBuffer, readSize, *pLcdFile);

        if (readSize < CD_SECTOR_SIZE) {
            // When we are not filling part of the buffer then zero it out just for consistency
            std::memset(sectorBuffer + readSize, 0, (size_t) CD_SECTOR_SIZE - readSize);
        }

        numSpuBytesWritten += wess_dig_lcd_data_read(sectorBuffer, destSpuAddr + numSpuBytesWritten, pSampleBlock, bOverride);
        lcdBytesLeft -= readSize;
    }

    return numSpuBytesWritten;
}
#endif  // #if PC_PSX_DOOM_MODS

#if !PC_PSX_DOOM_MODS
// This is the original version of this function, which is much more complex and contains a lot of low-level IO
int32_t wess_dig_lcd_load(
    const CdMapTbl_File lcdFileToLoad,
    const uint32_t destSpuAddr,
    SampleBlock* const pSampleBlock,
    const bool bOverride
) noexcept {
    // Turn off internal cd related callbacks temporarily while this is happening
    psxcd_disable_callbacks();

    // Keep trying to read the LCD file until there is success
    while (true) {
        // Try to read the header sector for the LCD file: begin by opening the requested LCD file and initializing sound related variables
        uint16_t* const pLcdHeader = (uint16_t*) gPSXCD_sectorbuf.get();

        *gWess_lcd_load_soundNum = 0;
        *gWess_lcd_load_soundBytesLeft = 0;
        
        psxcd_init_pos();
        psxcd_set_data_mode();
        PsxCd_File* const pLcdFile = wess_dig_lcd_data_open(lcdFileToLoad);

        // If the file returned is not present (PC-PSX added check) or invalid then just stop now
        #if PC_PSX_DOOM_MODS
            if ((!pLcdFile) || (pLcdFile->file.pos == 0))
                return 0;
        #else
            if (pLcdFile->file.pos == 0)
                return 0;
        #endif
        
        // Begin reading the LCD file, starting with the first sector, which contains the header.
        // Also initialize the I/O location before the read.
        const int32_t lcdFileSize = pLcdFile->file.size;

        LIBCD_CdIntToPos(LIBCD_CdPosToInt(pLcdFile->file.pos), pLcdFile->new_io_loc);
        LIBCD_CdControl(CdlSetloc, (uint8_t*) &pLcdFile->new_io_loc, nullptr);
        LIBCD_CdControl(CdlReadN, (uint8_t*) &pLcdFile->new_io_loc, nullptr);

        // Wait until the header sector is read or an error occurs.
        // If there was an error then abort all active CDROM commands and try everything again.
        CdlSyncStatus cdStatus = CdlNoIntr;

        do {
            cdStatus = LIBCD_CdReady(1, nullptr);
        } while ((cdStatus != CdlDataReady) && (cdStatus != CdlDiskError));

        if (cdStatus == CdlDiskError) {
            LIBCD_CdFlush();
            continue;
        }

        // Grab the sector buffer and get the number of sounds to load from it
        LIBCD_CdGetSector(gPSXCD_sectorbuf.get(), CD_SECTOR_SIZE / sizeof(uint32_t));
        *gWess_lcd_load_numSounds = pLcdHeader[0];
        *gpWess_lcd_load_headerBuf = gPSXCD_sectorbuf.get();

        // If the number of sounds exceeds the max then a read error must have happened.
        // In this case retry the read again...
        if (*gWess_lcd_load_numSounds > MAX_LCD_SOUNDS)
            continue;

        // Figure out how many sound bytes there is to read.
        // The first sector is discounted, as that is the LCD header sector:
        int32_t lcdSoundBytesLeft = lcdFileSize - CD_SECTOR_SIZE;
        
        if (lcdSoundBytesLeft < 0) {
            lcdSoundBytesLeft = 0;
        }

        // Main sound reading loop:
        // Reads all of the sectors in the LCD file and uploads all of the sound data to the SPU.
        int32_t numSpuBytesWritten = 0;
        uint32_t curDestSpuAddr = destSpuAddr;

        bool bDidPrevUploadToSpu = false;
        bool bUseSectorBuffer2 = false;

        while (lcdSoundBytesLeft != 0) {
            // Wait until a data sector is read or an error occurs.
            // If there was an error then abort all active CDROM commands and try everything all over again (header included).
            do {
                cdStatus = LIBCD_CdReady(1, nullptr);
            } while ((cdStatus != CdlDataReady) && (cdStatus != CdlDiskError));

            if (cdStatus == CdlDiskError) {
                LIBCD_CdFlush();
                break;
            }
            
            // Grab the contents of this CD sector
            uint8_t* const pSectorBuffer = (bUseSectorBuffer2) ? gWess_sectorBuffer2.get() : gWess_sectorBuffer1.get();
            LIBCD_CdGetSector(pSectorBuffer, CD_SECTOR_SIZE / sizeof(uint32_t));
            lcdSoundBytesLeft -= CD_SECTOR_SIZE;
            
            if (lcdSoundBytesLeft < 0) {
                lcdSoundBytesLeft = 0;
            }

            // If we previously started a transfer to the SPU, wait for it to finish before we do another one
            if (bDidPrevUploadToSpu) {
                LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
            }

            // Upload the sound contents of this data sector to the SPU
            const int32_t bytesUploadedToSpu = wess_dig_lcd_data_read(pSectorBuffer, curDestSpuAddr, pSampleBlock, bOverride);
            numSpuBytesWritten += bytesUploadedToSpu;
            curDestSpuAddr += bytesUploadedToSpu;

            // Flip sector buffers and set the flag indicating we've uploaded something to the SPU
            bUseSectorBuffer2 = (!bUseSectorBuffer2);
            bDidPrevUploadToSpu = true;
        }

        // If all of the LCD bytes were read successfully then we can finish up, otherwise let the loop try again
        if (cdStatus != CdlDiskError) {
            LIBCD_CdControl(CdlPause, nullptr, nullptr);
            
            if (wess_dig_lcd_psxcd_sync() == 0) {
                LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
                psxcd_enable_callbacks();
                return numSpuBytesWritten;
            }
        }
    }

    // Should never reach this point, but if we do then consider it a failure!
    return 0;
}
#endif  // #if !PC_PSX_DOOM_MODS
