#include "Wess/lcdload.h"

#if !PC_PSX_DOOM_MODS

#include "PsyQ/LIBCD.h"

// The currently open LCD file
static PsxCd_File gWess_open_lcd_file;

//------------------------------------------------------------------------------------------------------------------------------------------
// Opens the specified file for the LCD loader.
// PC-PSX: removed to fix compiler warnings as it is unreferenced and no longer used.
//------------------------------------------------------------------------------------------------------------------------------------------
static PsxCd_File* wess_dig_lcd_data_open(const CdMapTbl_File file) noexcept {
    PsxCd_File* const pFile = psxcd_open(file);
    gWess_open_lcd_file = *pFile;
    return pFile;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait for up to 8 seconds for all CDROM commands to finish executing.
// Returns '1' if there was an error or timeout waiting for all commands to finish up, '0' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_dig_lcd_psxcd_sync() noexcept {
    const uint32_t timeoutMs = gWess_Millicount + 8000;
    
    while (gWess_Millicount < timeoutMs) {
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
        uint16_t* const pLcdHeader = (uint16_t*) gPSXCD_sectorbuf;

        gWess_lcd_load_soundNum = 0;
        gWess_lcd_load_soundBytesLeft = 0;
        
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
        LIBCD_CdGetSector(gPSXCD_sectorbuf, CD_SECTOR_SIZE / sizeof(uint32_t));
        gWess_lcd_load_numSounds = pLcdHeader[0];
        gpWess_lcd_load_headerBuf = gPSXCD_sectorbuf;

        // If the number of sounds exceeds the max then a read error must have happened.
        // In this case retry the read again...
        if (gWess_lcd_load_numSounds > MAX_LCD_SOUNDS)
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
            uint8_t* const pSectorBuffer = (bUseSectorBuffer2) ? gWess_sectorBuffer2 : gWess_sectorBuffer1;
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

#endif  // !PC_PSX_DOOM_MODS
