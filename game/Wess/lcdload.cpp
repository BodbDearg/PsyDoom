//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): LCD (sound samples file) loading code.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "lcdload.h"

#include "Doom/cdmaptbl.h"
#include "psxcd.h"
#include "psxspu.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBCD.h"
#include "PsyQ/LIBSPU.h"
#include "wessarc.h"

static const VmPtr<PsxCd_File>                      gWess_open_lcd_file(0x8007F2E0);                // The currently open LCD file
static const VmPtr<VmPtr<patch_group_data>>         gpWess_lcd_load_patchGrp(0x80075AD8);           // TODO: COMMENT
static const VmPtr<VmPtr<patches_header>>           gpWess_lcd_load_patches(0x80075AC8);            // TODO: COMMENT
static const VmPtr<VmPtr<patchmaps_header>>         gpWess_lcd_load_patchmaps(0x80075ACC);          // TODO: COMMENT
static const VmPtr<VmPtr<patchinfo_header>>         gpWess_lcd_load_patchInfos(0x80075AD0);         // TODO: COMMENT
static const VmPtr<VmPtr<drumpmaps_header>>         gpWess_lcd_load_drummaps(0x80075AD4);           // TODO: COMMENT
static const VmPtr<int32_t>                         gWess_lcd_load_soundBytesLeft(0x80075AE4);      // TODO: COMMENT
static const VmPtr<uint32_t>                        gWess_lcd_load_soundNum(0x80075AE0);            // TODO: COMMENT
static const VmPtr<uint16_t>                        gWess_lcd_load_numSounds(0x80075AE8);           // TODO: COMMENT
static const VmPtr<VmPtr<uint8_t>>                  gpWess_lcd_load_headerBuf(0x80075AEC);          // TODO: COMMENT
static const VmPtr<uint32_t>                        gWess_lcd_load_samplePos(0x80075ADC);           // TODO: COMMENT

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the LCD loader with the specified master status struct
//------------------------------------------------------------------------------------------------------------------------------------------
bool wess_dig_lcd_loader_init(master_status_structure* const pMStat) noexcept {
    // If there is no master status struct then we cannot initialize
    if (!pMStat)
        return false;
    
    // Try to find the patch group in the currently loaded module for the PlayStation
    module_data& mod_info = *pMStat->pmod_info;
    patch_group_data* pPatchGrp = nullptr;

    for (uint8_t patchGrpIdx = 0; patchGrpIdx < mod_info.mod_hdr.patch_types_infile; ++patchGrpIdx) {
        patch_group_data& patchGrp = pMStat->ppat_info[patchGrpIdx];

        if (patchGrp.pat_grp_hdr.patch_id == PSX_ID) {
            pPatchGrp = &patchGrp;
            break;
        }
    }

    // If we didn't find the patch group for the PSX driver then initialization failed
    *gpWess_lcd_load_patchGrp = pPatchGrp;

    if (!pPatchGrp)
        return false;

    // Save pointers to the various data structures for the patch group
    {
        uint8_t* pPatchData = pPatchGrp->ppat_data.get();

        *gpWess_lcd_load_patches = (patches_header*) pPatchData;
        pPatchData += sizeof(patches_header) * pPatchGrp->pat_grp_hdr.patches;
        
        *gpWess_lcd_load_patchmaps = (patchmaps_header*) pPatchData;
        pPatchData += sizeof (patchmaps_header) * pPatchGrp->pat_grp_hdr.patchmaps;

        *gpWess_lcd_load_patchInfos = (patchinfo_header*) pPatchData;
        pPatchData += sizeof(patchinfo_header) * pPatchGrp->pat_grp_hdr.patchinfo;

        *gpWess_lcd_load_drummaps = (drumpmaps_header*) pPatchData;
    }

    return true;
}

void wess_dig_set_sample_position() noexcept {
loc_80048FCC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5AD0);                               // Load from: 80075AD0
    v0 = a0 << 1;
    if (v1 == 0) goto loc_80048FF0;
    v0 += a0;
    v0 <<= 2;
    v0 += v1;
    sw(a1, v0 + 0x8);
loc_80048FF0:
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Opens the specified file for the LCD loader
//------------------------------------------------------------------------------------------------------------------------------------------
static PsxCd_File* wess_dig_lcd_data_open(const CdMapTbl_File file) noexcept {
    PsxCd_File* const pFile = psxcd_open(file);
    *gWess_open_lcd_file = *pFile;
    return pFile;
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
    patchinfo_header* const pPatchInfos = gpWess_lcd_load_patchInfos->get();
    const uint16_t* pSampleIndexes = (const uint16_t*) gpWess_lcd_load_headerBuf->get();

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
                const uint16_t patchIdx = pSampleIndexes[*gWess_lcd_load_soundNum];
                patchinfo_header& patchInfo = pPatchInfos[patchIdx];

                // Only save the details of the sound if there was not a previous sound saved, or if we are forcing it
                if ((patchInfo.sample_pos == 0) || bOverride) {
                    patchInfo.sample_pos = *gWess_lcd_load_samplePos;

                    // Save the details of the sound to the sample block also, if it's given
                    if (pSampleBlock) {
                        pSampleBlock->sampindx[pSampleBlock->numsamps] = patchIdx;
                        pSampleBlock->samppos[pSampleBlock->numsamps] = (uint16_t)(patchInfo.sample_pos / 8);
                        pSampleBlock->numsamps++;
                    }
                }
            }

            // If we have finished uploading all sounds then we are done
            if (*gWess_lcd_load_soundNum >= *gWess_lcd_load_numSounds)
                return bytesWritten;
            
            // Otherwise switch over to reading and uploading the next sound.
            // Set the number of bytes to read, and where to upload it to in SPU ram when we are done.
            *gWess_lcd_load_soundNum += 1;

            const int32_t nextPatchIdx = pSampleIndexes[*gWess_lcd_load_soundNum];
            patchinfo_header& nextPatchInfo = pPatchInfos[nextPatchIdx];

            *gWess_lcd_load_soundBytesLeft = nextPatchInfo.sample_size;
            *gWess_lcd_load_samplePos = destSpuAddr + sndDataOffset;

            // If uploading this sound would cause us to go beyond the bounds of SPU ram then do not try to upload this sound
            #if PC_PSX_DOOM_MODS
                // PC-PSX: I think this condition was slightly wrong?
                const bool bInsufficientSpuRam = ((int32_t) destSpuAddr + nextPatchInfo.sample_size + sndDataOffset > (int32_t) *gPsxSpu_sram_end);
            #else
                const bool bInsufficientSpuRam = ((int32_t) destSpuAddr + nextPatchInfo.sample_size > (int32_t)(*gPsxSpu_sram_end + sndDataOffset));
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
        // Either upload the whole sector or just a part of it (if it's bigger than the rest of thhe sound)
        const uint16_t sampleIdx = pSampleIndexes[*gWess_lcd_load_soundNum];
        const patchinfo_header& patchInfo = pPatchInfos[sampleIdx];

        const uint32_t sndBytesLeft = (uint32_t) *gWess_lcd_load_soundBytesLeft;
        const uint32_t writeSize = (sectorBytesLeft > sndBytesLeft) ? sndBytesLeft : sectorBytesLeft;

        if ((patchInfo.sample_pos == 0) || bOverride) {
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
//------------------------------------------------------------------------------------------------------------------------------------------
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

        // If the number of sounds exceeds 100 then a read error must have happened.
        // In this case retry the read again...
        if (*gWess_lcd_load_numSounds > 100)
            continue;

        // Figure out how many sound bytes there is to read.
        // The first sector is discounted, as that is the LCD header sector:
        int32_t lcdSoundBytesLeft = lcdFileSize - CD_SECTOR_SIZE;
        
        if (lcdSoundBytesLeft < 0) {
            lcdSoundBytesLeft = 0;
        }

        // Main sound reading loop:
        // Reads all of the sectors in the LCD file and uploads all of the sound data to the SPU.
        int32_t totalBytesUploadedToSpu = 0;
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
            totalBytesUploadedToSpu += bytesUploadedToSpu;
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
                return totalBytesUploadedToSpu;
            }
        }
    }

    // Should never reach this point, but if we do then consider it a failure!
    return 0;
}
