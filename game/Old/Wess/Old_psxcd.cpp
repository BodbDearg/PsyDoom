#include "Old_psxcd.h"

#if !PC_PSX_DOOM_MODS

// Various flags
static bool gbPSXCD_init_pos;               // If this flag is false then we need to initialize the cd position for data reading (we don't know it), true if we initialized the cd position
static bool gbPSXCD_async_on;               // True when there is an asynchronous read happening in data mode

// Data reading mode stuff
static CdlLOC           gPSXCD_cur_io_loc;              // Current IO location on disc
static CdlLOC           gPSXCD_sectorbuf_contents;      // Where the current sector buffer contents came from on disc
static void*            gpPSXCD_lastdestptr;            // Async read: destination memory chunk being written to
static int32_t          gPSXCD_lastreadbytes;           // Async read: number of bytes being read
static PsxCd_File       gPSXCD_lastfilestruct;          // Async read: details for the file being read
static PsxCd_File       gPSXCD_newfilestruct;           // Async read: details for the file for which read is being retried
static int32_t          gPSXCD_cur_cmd;                 // Async read: index of the current command being issued in the loop iteration
static PsxCd_Command    gPSXCD_psxcd_cmds[5];           // Async read: commands issued to the cd

// Number of sectors read in data and audio mode respectively
static int32_t  gPSXCD_readcount;           // Number of data sectors read

// CD commands issued and results
static int32_t  gPSXCD_sync_intr;           // Int result of the last 'LIBCD_CdSync' call when waiting until command completion
static uint8_t  gPSXCD_sync_result[8];      // Result bytes for the last 'LIBCD_CdSync' call when waiting until command completion

// Function forward declarations
static int32_t psxcd_async_read(void* const pDest, const int32_t numBytes, PsxCd_File& file) noexcept;
void psxcd_async_read_cancel() noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Simple memcpy() operation
//------------------------------------------------------------------------------------------------------------------------------------------
static void PSXCD_psxcd_memcpy(void* const pDst, const void* const pSrc, uint32_t count) noexcept {
    uint8_t* const pDstBytes = (uint8_t*) pDst;
    const uint8_t* const pSrcBytes = (const uint8_t*) pSrc;

    for (uint32_t i = 0; i < count; ++i) {
        pDstBytes[i] = pSrcBytes[i];
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait until the current CD command has completed successfully (with a timeout)
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_sync() noexcept {
    const uint32_t timeoutMs = gWess_Millicount + 8000;

    while (gWess_Millicount < timeoutMs) {
        gPSXCD_sync_intr = LIBCD_CdSync(1, gPSXCD_sync_result);

        if (gPSXCD_sync_intr == CdlDiskError) {
            // Cancel the current command if there was a problem and try again
            LIBCD_CdFlush();
            gPSXCD_cdl_err_count++;
            gPSXCD_cdl_err_intr = gPSXCD_sync_intr + 80;    // '+': Just to make the codes more unique, so their source is known
            gPSXCD_cdl_err_com = gPSXCD_cdl_com;
            gPSXCD_cdl_err_stat = gPSXCD_sync_result[0];
        }

        if (gPSXCD_sync_intr == CdlComplete)
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Wait until the current CD command has completed successfully.
// Do so with a timeout, or give up if there is an error.
// Return 'true' if the CD command succeeded.
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_critical_sync() noexcept {
    const uint32_t timeoutMs = gWess_Millicount + 8000;

    while (gWess_Millicount < timeoutMs) {
        gPSXCD_sync_intr = LIBCD_CdSync(1, gPSXCD_sync_result);

        if (gPSXCD_sync_intr == CdlDiskError) {
            // Cancel the current command if there was a problem
            LIBCD_CdFlush();
            gPSXCD_cdl_err_count++;
            gPSXCD_cdl_err_intr = gPSXCD_sync_intr + 70;    // '+': Just to make the codes more unique, so their source is known
            gPSXCD_cdl_err_com = gPSXCD_cdl_com;
            gPSXCD_cdl_err_stat = gPSXCD_sync_result[0];

            return false;   // Give up if an error happens!
        }

        if (gPSXCD_sync_intr == CdlComplete)
            return true;
    }

    return false;   // Timeout!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Callback invoked by the PsyQ SDK (and ultimately by the hardware) for when we are finished reading a cd sector
//------------------------------------------------------------------------------------------------------------------------------------------
void PSXCD_cbready(const CdlSyncStatus status, const uint8_t pResult[8]) noexcept {
    // Are callbacks disabled currently? If so then ignore...
    if (!gbPSXCD_cb_enable_flag)
        return;
    
    // Save the status bits and int results for the command
    gPSXCD_cdl_stat = pResult[0];
    gPSXCD_cdl_intr = status;
    
    if ((pResult[0] & CdlStatRead) && gbPSXCD_async_on && (gPSXCD_cdl_com == CdlReadN)) {
        // Update read stats
        gPSXCD_readcount++;
        
        // Is there data ready for reading?
        if (status == CdlDataReady) {
            if (gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command == PSXCD_COMMAND_READ) {
                // Executing a command to read an entire sector to the output/destination location.
                //
                // Optimization: if the destination pointer is 32-bit aligned then we can just copy directly to the output location.
                // If the pointer is 32-bit aligned also then we assume the data is padded to at least 32-bit boundaries.
                if (((uintptr_t) gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].pdest & 3) == 0) {
                    // Yay, it's all aligned: we can copy the data directly to the output location!
                    LIBCD_CdGetSector(gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].pdest, CD_SECTOR_SIZE / sizeof(uint32_t));
                } else {
                    // Unaligned destination: have to take the scenic route...
                    LIBCD_CdGetSector(gPSXCD_sectorbuf, CD_SECTOR_SIZE / sizeof(uint32_t));
                    PSXCD_psxcd_memcpy(gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].pdest, gPSXCD_sectorbuf, CD_SECTOR_SIZE);
                }

                // One less sector to read
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].pdest = gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].pdest + CD_SECTOR_SIZE;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].amount -= 1;

                // If there are no more sectors to read for this command then move onto the next command
                if (gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].amount == 0) {
                    gPSXCD_cur_cmd++;
                }
            }
            else if (gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command == PSXCD_COMMAND_READCOPY) {
                // Executing a command to read a partial sector.
                // Copy the entire sector to the sector buffer, then just copy out the bits we need:
                LIBCD_CdGetSector(gPSXCD_sectorbuf, CD_SECTOR_SIZE / sizeof(uint32_t));
                PSXCD_psxcd_memcpy(
                    gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].pdest,
                    gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].psrc,
                    gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].amount
                );

                // This command is just one read sector operation, move onto the next
                gPSXCD_cur_cmd++;
            }
            else {
                // Don't know what this command is, so terminate the command list
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command = PSXCD_COMMAND_END;
            }
            
            // Are we finishing up the command list? If so then pause the cdrom:
            if (gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command == PSXCD_COMMAND_END) {
                gbPSXCD_async_on = false;
                gbPSXCD_waiting_for_pause = true;
                gPSXCD_cdl_com = CdlPause;
                LIBCD_CdControlF(CdlPause, nullptr);
            }

            return;
        }
        
        // Did an error happen? If so then cancel the current command
        if (status == CdlDiskError) {
            LIBCD_CdFlush();
            gbPSXCD_critical_error = true;      // This is a bad one!
        }

        gPSXCD_cdl_err_com = gPSXCD_cdl_com;
        gPSXCD_cdl_err_intr = status;
    }
    else if (pResult[0] & CdlStatPlay) {
        // If we are playing cd audio and there is new data ready then record the updated cd location
        if (status == CdlDataReady) {
            // I'm not sure what the hell this flag is - I couldn't find any mention of it anywhere.
            // This bit shouldn't be set even when the encoding is BCD because second count shouldn't exceed 60?
            if ((pResult[4] & 0x80) == 0) {
                gPSXCD_playcount++;
                gPSXCD_newloc.minute = pResult[3];
                gPSXCD_newloc.second = pResult[4];
                gPSXCD_newloc.sector = pResult[5];
                gPSXCD_newloc.track = (pResult[1] >> 4) * 10 + (pResult[1] & 0x0F);     // Convert from BCD to binary
            }

            return;
        }
        
        // If we have reached the end then we might want to loop around again
        if (status == CdlDataEnd) {
            // Clear the command and location if we have reached the end
            gPSXCD_cdl_com = CdlNop;
            gPSXCD_lastloc = 0;

            // Are we to loop and play again?
            if (gbPSXCD_playflag || gbPSXCD_loopflag) {
                psxcd_play_at_andloop(
                    gPSXCD_looptrack,
                    gPSXCD_loopvol,
                    gPSXCD_loopsectoroffset,
                    gPSXCD_loopfadeuptime,
                    gPSXCD_looptrack,
                    gPSXCD_loopvol,
                    gPSXCD_loopsectoroffset,
                    gPSXCD_loopfadeuptime
                );
            }

            return;
        }
        
        // If there is a disk error cancel any current command
        if (status == CdlDiskError) {
            LIBCD_CdFlush();
        }

        // Error or unknown situation:
        gPSXCD_cdl_err_intr = status + 20;      // '+': Just to make the codes more unique, so their source is known
        gPSXCD_cdl_err_com = gPSXCD_cdl_com;
    }
    else {
        // Error or unknown situation:
        gPSXCD_cdl_err_intr = status + 30;      // '+': Just to make the codes more unique, so their source is known
        gPSXCD_cdl_err_com = gPSXCD_cdl_com;
    }

    // This point is only reached if there is an error!
    gPSXCD_cdl_err_count++;
    gPSXCD_cdl_err_stat = pResult[0];
    gPSXCD_cdl_com = gPSXCD_cdl_err_com;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the WESS (Williams Entertainment Sound System) CD handling module
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_init() noexcept {
    // If we've already done this then just no-op
    if (gbPSXCD_IsCdInit)
        return;

    // Initialize LIBCD
    LIBCD_CdInit();
    gbPSXCD_IsCdInit = true;

    // Clear the locations for all CD tracks
    for (int32_t trackIdx = 0; trackIdx < CdlMAXTOC; ++trackIdx) {
        CdlLOC& trackpos = gTrackCdlLOC[trackIdx];
        trackpos.minute = 0;
        trackpos.second = 0;
        trackpos.sector = 0;
        trackpos.track = 0;
    }

    // Initialize the SPU
    psxspu_init();

    // Get the locations of all CD tracks
    const int32_t trackCount = LIBCD_CdGetToc(gTrackCdlLOC);

    if (trackCount != 0) {
        gbPSXCD_init_pos = false;
        gbPSXCD_async_on = false;
        
        // Set command complete and data callbacks and save the old ones for later restoring
        gPSXCD_cbsyncsave = LIBCD_CdSyncCallback(PSXCD_cbcomplete);
        gPSXCD_cbreadysave = LIBCD_CdReadyCallback(PSXCD_cbready);

        psxcd_enable_callbacks();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Switches the cdrom drive into data reading mode
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_set_data_mode() noexcept {
    if (gPSXCD_psxcd_mode != 1) {
        // Currently in audio mode: begin fading out audio and clear playback flags
        gbPSXCD_playflag = false;
        gbPSXCD_loopflag = false;
        gbPSXCD_seeking_for_play = false;

        const int32_t cdVol = psxspu_get_cd_vol();

        if (cdVol != 0) {
            psxspu_start_cd_fade(FADE_TIME_MS, 0);
            Utils::waitForCdAudioFadeOut();
        }

        // Disable cd audio mixing
        psxspu_setcdmixoff();
        
        // Ensure no pending commands and set the cdrom mode to double speed data
        psxcd_sync();

        gPSXCD_psxcd_mode = 1;
        gPSXCD_cdl_com = CdlSetmode;
        gPSXCD_cd_param[0] = CdlModeSpeed;  // Operate the CDROM at 2x speed
        LIBCD_CdControl(CdlSetmode, gPSXCD_cd_param, nullptr);
        psxcd_sync();
    } else {
        // Already in data mode: just cancel any reads or commands in flight
        if (gbPSXCD_async_on) {
            psxcd_async_read_cancel();
        }

        psxcd_sync();
        
        if (gPSXCD_cdl_com == CdlPause) {
            LIBCD_CdFlush();
        }
    }
    
    // Stop the cdrom at the current location when we are done to finish up
    gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControl(CdlPause, nullptr, nullptr);
    psxcd_sync();
    LIBCD_CdFlush();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open a specified CD file for reading
//------------------------------------------------------------------------------------------------------------------------------------------
PsxCd_File* psxcd_open(const CdMapTbl_File discFile) noexcept {
    // Figure out where the file is on disc and save it's size
    const PsxCd_MapTblEntry& fileTableEntry = CD_MAP_TBL[(uint32_t) discFile];
    LIBCD_CdIntToPos(fileTableEntry.startSector, gPSXCD_cdfile.file.pos);
    gPSXCD_cdfile.file.size = fileTableEntry.size;

    // Initialize file IO position and status
    gPSXCD_cdfile.new_io_loc = gPSXCD_cdfile.file.pos;
    gPSXCD_cdfile.io_block_offset = 0;

    for (uint8_t& statusByte : gPSXCD_cdfile.io_result) {
        statusByte = 0;
    }

    return &gPSXCD_cdfile;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes some positional related variables
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_init_pos() noexcept {
    gbPSXCD_seeking_for_play = false;
    gbPSXCD_waiting_for_pause = false;
    gbPSXCD_init_pos = false;
    gbPSXCD_critical_error = false;
    gbPSXCD_playflag = false;
    gbPSXCD_loopflag = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Queries if there is an asynchronous read still happening and returns 'true' if that is the case.
// Also retries the current async read, if there is an error detected.
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_async_on() noexcept {
    // If we are not doing any async reading then the answer is simple
    if (!gbPSXCD_async_on)
        return false;

    // Otherwise do a status check on the health of the read to make sure it is going ok
    gPSXCD_check_intr = LIBCD_CdSync(1, gPSXCD_check_result);

    // If a problem happened then retry the current read.
    // Retry if we encounter a critical error, a disk error or the motor stops rotating - which can happen if the shell is opened.
    if (gbPSXCD_critical_error || (gPSXCD_check_intr == CdlDiskError) || ((gPSXCD_check_result[0] & CdlStatStandby) == 0)) {
        // A problem happened! Record the details of the error and stop any executing cd commands:
        LIBCD_CdFlush();

        gPSXCD_cdl_err_count++;
        gPSXCD_cdl_err_com = gPSXCD_cdl_com;
        gPSXCD_cdl_err_intr = gPSXCD_check_intr + 100;      // '+': Just to make the codes more unique, so their source is known
        gPSXCD_cdl_err_stat = gPSXCD_check_result[0];

        // Clear the error flag and retry the last read command
        gbPSXCD_critical_error = false;
        gPSXCD_newfilestruct = gPSXCD_lastfilestruct;
        psxcd_async_read(gpPSXCD_lastdestptr, gPSXCD_lastreadbytes, gPSXCD_newfilestruct);
    }

    // Still reading...
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the cdrom is currently seeking to a location for audio playback
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_seeking_for_play() noexcept {
    // If we are not seeking then the answer is simple
    if (!gbPSXCD_seeking_for_play)
        return false;
    
    // The cdrom is still busy seeking: check to make sure there was not an error
    gPSXCD_check_intr = LIBCD_CdSync(1, gPSXCD_check_result);
    
    if ((gPSXCD_check_intr == CdlDiskError) || ((gPSXCD_check_result[0] & CdlStatStandby) == 0)) {
        // Some sort of error happened and not on standby: cancel the current command and record the command details
        LIBCD_CdFlush();

        gPSXCD_cdl_err_count++;
        gPSXCD_cdl_err_intr = gPSXCD_check_intr + 110;      // '+': Just to make the codes more unique, so their source is known
        gPSXCD_cdl_err_com = gPSXCD_cdl_com;
        gPSXCD_cdl_err_stat = gPSXCD_check_result[0];

        // Try to seek to the last valid intended cd audio location
        psxcd_sync();
        gPSXCD_cdl_com = CdlSeekP;
        LIBCD_CdControlF(CdlSeekP, (uint8_t*) &gPSXCD_lastloc);
    }
    
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the cdrom is currently in the process of pausing
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_waiting_for_pause() noexcept {
    // If we are not pausing then the answer is simple
    if (!gbPSXCD_waiting_for_pause)
        return false;

    // The cdrom is still busy pausing: check to make sure there was not an error
    gPSXCD_check_intr = LIBCD_CdSync(1, gPSXCD_check_result);

    if ((gPSXCD_check_intr == CdlDiskError) || ((gPSXCD_check_result[0] & CdlStatStandby) == 0)) {
        // Some sort of error happened and not on standby: cancel the current command and record the command details
        LIBCD_CdFlush();

        gPSXCD_cdl_err_count++;
        gPSXCD_cdl_err_intr = gPSXCD_check_intr + 120;      // '+': Just to make the codes more unique, so their source is known
        gPSXCD_cdl_err_com = gPSXCD_cdl_com;
        gPSXCD_cdl_err_stat = gPSXCD_check_result[0];

        // Retry the pause command
        psxcd_sync();
        gPSXCD_cdl_com = CdlPause;
        LIBCD_CdControlF(CdlPause, nullptr);
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the specified number of bytes synchronously from the given CD file and returns the number of bytes read.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_read(void* const pDest, int32_t numBytes, PsxCd_File& file) noexcept {
    // Kick off the async read.
    // Note: number of bytes read will not match request if there was an error!
    const int32_t retBytesRead = psxcd_async_read(pDest, numBytes, file);

    // Continue reading until done.
    while (psxcd_async_on()) {}
    return retBytesRead;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cancels the currently active async read, if any
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_async_read_cancel() noexcept {
    // If we are not doing an async read then there is nothing to do
    if (!gbPSXCD_async_on)
        return;

    // Finish up any current commands and mark the data position as uninitialized
    gbPSXCD_async_on = false;
    gbPSXCD_init_pos = false;
    psxcd_sync();

    // Pause the cdrom
    gbPSXCD_waiting_for_pause = true;
    gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControlF(CdlPause, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Submit commands to the cdrom to do an asynchronous read of the specified number of bytes
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t psxcd_async_read(void* const pDest, const int32_t numBytes, PsxCd_File& file) noexcept {
    // If no bytes are being read or the file is invalid then there is nothing to do
    if ((numBytes == 0) || (file.file.pos == 0))
        return 0;

    bool bReadError;

    do {
        // Clear the error flag and ensure we are in data mode
        bReadError = false;
        psxcd_set_data_mode();
        
        // Save read details
        gpPSXCD_lastdestptr = pDest;
        gPSXCD_lastreadbytes = numBytes;
        gPSXCD_lastfilestruct = file;

        // We start putting commands at the beginning of the command list
        gPSXCD_cur_cmd = 0;

        // First handle the read location being misaligned with sector boundaries and queue read/copy commands for up until the end of the start sector.
        // This is done to get I/O to sector align so we can read a series of whole sectors in one continous operation (below).
        int32_t numBytesLeft = numBytes;
        uint8_t* pDestBytes = (uint8_t*) pDest;

        if ((file.io_block_offset != 0) && (numBytesLeft != 0)) {
            // Do we need to seek to the read location in the file? If so then queue up a seek command
            if ((!gbPSXCD_init_pos) || (gPSXCD_cur_io_loc != file.new_io_loc)) {
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command = PSXCD_COMMAND_SEEK;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].io_loc = file.new_io_loc;
                gPSXCD_cur_cmd++;

                // We know where we are going to be now
                gPSXCD_cur_io_loc = file.new_io_loc;
                gbPSXCD_init_pos = true;
            }
            
            // Decide how many bytes do we want to read from the sector
            const int32_t sectorBytesLeft = CD_SECTOR_SIZE - file.io_block_offset;
            const int32_t bytesToCopy = (numBytesLeft <= sectorBytesLeft) ? numBytesLeft : sectorBytesLeft;

            // Do we already have the required sector in the sector buffer?
            // If that is the case we can just copy the buffer contents, otherwise we need to read from the CD and then copy.
            if ((gPSXCD_cur_cmd == PSXCD_COMMAND_END) && (gPSXCD_sectorbuf_contents == gPSXCD_cur_io_loc)) {
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command = PSXCD_COMMAND_COPY;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].psrc = gPSXCD_sectorbuf + file.io_block_offset;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].pdest = pDestBytes;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].amount = bytesToCopy;

                gPSXCD_cur_cmd++;
            } else {
                // Haven't got the sector buffered, need to read first before we can copy
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command = PSXCD_COMMAND_READCOPY;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].psrc = gPSXCD_sectorbuf + file.io_block_offset;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].pdest = pDestBytes;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].amount = bytesToCopy;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].io_loc = gPSXCD_cur_io_loc;

                gPSXCD_sectorbuf_contents = gPSXCD_cur_io_loc;
                gPSXCD_cur_cmd++;
            }

            // Move along past the bytes read
            numBytesLeft -= bytesToCopy;
            pDestBytes += bytesToCopy;
            file.io_block_offset = file.io_block_offset + bytesToCopy;

            // Move onto a new sector if required
            if (file.io_block_offset == CD_SECTOR_SIZE) {
                const int32_t curSector = LIBCD_CdPosToInt(gPSXCD_cur_io_loc);
                LIBCD_CdIntToPos(curSector + 1, gPSXCD_cur_io_loc);
                file.new_io_loc = gPSXCD_cur_io_loc;
                file.io_block_offset = 0;
            }
        }
        
        // Next queue commands to read as many whole sectors as we can in one read operation.
        const int32_t wholeSectorsToRead = numBytesLeft / CD_SECTOR_SIZE;

        if (wholeSectorsToRead > 0) {
            // Do we need to seek to the read location in the file? If so then queue up a seek command
            if ((!gbPSXCD_init_pos) || (gPSXCD_cur_io_loc != file.new_io_loc)) {
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command = PSXCD_COMMAND_SEEK;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].io_loc = file.new_io_loc;
                gPSXCD_cur_cmd++;

                // We know where we are going to be now
                gPSXCD_cur_io_loc = file.new_io_loc;
                gbPSXCD_init_pos = true;
            }

            // How many bytes would be read by this operation?
            const int32_t wholeSectorBytes = wholeSectorsToRead * CD_SECTOR_SIZE;
            
            // Queue the read command
            gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command = PSXCD_COMMAND_READ;
            gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].amount = wholeSectorsToRead;
            gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].pdest = pDestBytes;

            // If the destination pointer is not 32-bit aligned then when reading it we must first take the extra step of copying the sector
            // data to the PSXCD sector buffer and THEN copy from there to the intended destination. This is because 'LIBCD_CdGetSector'
            // works on 32-bit word units only and requires 32-bit aligned pointers. Note: if the output data is 32-bit aligned then it is
            // assumed to be padded to at least 32-bits in size.
            //
            // So... if we are going to be dealing with unaligned data, make a note of what CD location will be going into the sector buffer:
            if ((uintptr_t) pDestBytes & 3) {
                const int32_t curSector = LIBCD_CdPosToInt(gPSXCD_cur_io_loc);
                LIBCD_CdIntToPos(curSector + wholeSectorsToRead - 1, gPSXCD_sectorbuf_contents);
            }

            gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].io_loc = gPSXCD_cur_io_loc;
            gPSXCD_cur_cmd++;

            // Move along in the ouput
            pDestBytes += wholeSectorBytes;
            numBytesLeft -= wholeSectorBytes;

            // Figure out which sector to go to next
            {
                const int32_t curSector = LIBCD_CdPosToInt(gPSXCD_cur_io_loc);
                LIBCD_CdIntToPos(curSector + wholeSectorsToRead, gPSXCD_cur_io_loc);
            }

            file.io_block_offset = 0;
            file.new_io_loc = gPSXCD_cur_io_loc;
        }

        // Queue commands to read the remaining few bits of data
        if (numBytesLeft != 0) {
            // Do we need to seek to the read location in the file? If so then queue up a seek command
            if ((!gbPSXCD_init_pos) || (gPSXCD_cur_io_loc != file.new_io_loc)) {
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command = PSXCD_COMMAND_SEEK;
                gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].io_loc = file.new_io_loc;
                gPSXCD_cur_cmd++;

                // We know where we are going to be now
                gPSXCD_cur_io_loc = file.new_io_loc;
                gbPSXCD_init_pos = true;
            }

            // Queue the read and copy command
            gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command = PSXCD_COMMAND_READCOPY;
            gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].psrc = gPSXCD_sectorbuf;
            gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].pdest = pDestBytes;
            gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].amount = numBytesLeft;
            gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].io_loc = gPSXCD_cur_io_loc;
            gPSXCD_cur_cmd++;

            // Move along the I/O location
            gPSXCD_sectorbuf_contents = gPSXCD_cur_io_loc;
            file.io_block_offset = numBytesLeft;
        }

        // Terminate the command list.
        // After this all 'commands' are queued and we can begin issuing them to the CDROM via LIBCD.
        gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command = PSXCD_COMMAND_END;
        gPSXCD_cur_cmd = 0;

        // Issue command: are we to copy bytes to the destination buffer?
        if (gPSXCD_psxcd_cmds[0].command == PSXCD_COMMAND_COPY) {
            PSXCD_psxcd_memcpy(gPSXCD_psxcd_cmds[0].pdest, gPSXCD_psxcd_cmds[0].psrc, gPSXCD_psxcd_cmds[0].amount);
            gPSXCD_cur_cmd++;

            // No more commands?
            if (gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command == PSXCD_COMMAND_END)
                return numBytes;
        }

        // Issue command: are we to seek to a location?
        if (gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command == PSXCD_COMMAND_SEEK) {
            psxcd_sync();
            gPSXCD_cdl_com = CdlSetloc;
            LIBCD_CdControl(CdlSetloc, (const uint8_t*) &gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].io_loc, nullptr);
            gPSXCD_cur_cmd++;

            // No more commands?
            if (gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command == PSXCD_COMMAND_END)
                return numBytes;
        }

        // Issue whatever remains in the command list at this point
        switch (gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].command) {
            // No more commands?
            case PSXCD_COMMAND_END:
                return numBytes;

            // Are we to read? If so then kick that off
            case PSXCD_COMMAND_READ:
            case PSXCD_COMMAND_READCOPY: {
                if (psxcd_critical_sync()) {
                    gPSXCD_cdl_com = CdlReadN;
                    LIBCD_CdControl(CdlReadN, (const uint8_t*) &gPSXCD_psxcd_cmds[gPSXCD_cur_cmd].io_loc, nullptr);
                    
                    if (psxcd_critical_sync()) {
                        // We are now doing an asynchronous read.
                        // The flag won't be cleared until it is done or cancelled.
                        gbPSXCD_async_on = true;
                    } else {
                        bReadError = true;
                    }
                } else {
                    bReadError = true;
                }
            }   break;

            // Unknown/unexpected command here: read failed!
            default:
                return 0;
        }

        // If a read error happened then try redo the entire read
        if (bReadError) {
            file = gPSXCD_lastfilestruct;
        }
    } while (bReadError);
    
    return numBytes;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek to a specified position in a file, relatively or absolutely.
// Returns '0' on success, any other value on failure.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_seek(PsxCd_File& file, int32_t offset, const PsxCd_SeekMode mode) noexcept {
    // Is this an actual valid file? If not then just NOP the call and return '0' for 'success':
    if (file.file.pos == CdlLOC{ 0, 0, 0, 0 })
        return 0;

    if (mode == PsxCd_SeekMode::SET) {
        // Seek to an absolute position in the file: figure out the sector for the requested location within the file
        const int32_t fileStartSec = LIBCD_CdPosToInt(file.file.pos);
        const int32_t sectorInFile = ((offset < 0) ? offset + (CD_SECTOR_SIZE - 1) : offset) / CD_SECTOR_SIZE;
        const int32_t newDiscSector = fileStartSec + sectorInFile;
        LIBCD_CdIntToPos(newDiscSector, file.new_io_loc);

        // Figure out the offset within the destination sector we want to go to
        file.io_block_offset = (uint32_t)(offset - sectorInFile * CD_SECTOR_SIZE);
    }
    else if (mode == PsxCd_SeekMode::CUR) {
        // Seek relative to the current IO position: figure out the sector for the requested relative offset
        const int32_t curIoSec = LIBCD_CdPosToInt(gPSXCD_cur_io_loc);
        const int32_t secOffset = (file.io_block_offset + offset) / CD_SECTOR_SIZE;
        const int32_t newDiscSector = curIoSec + secOffset;
        LIBCD_CdIntToPos(newDiscSector, file.new_io_loc);
        
        // Figure out the offset within the destination sector we want to go to
        file.io_block_offset = ((uint32_t)(file.io_block_offset + offset)) % CD_SECTOR_SIZE;
    }
    else {
        // Seek relative to the end: figure out the disc sector to go to for the requested location within the file
        const int32_t fileStartSec = LIBCD_CdPosToInt(file.file.pos);
        const int32_t sectorInFile = (file.file.size - offset) / CD_SECTOR_SIZE;
        const int32_t newDiscSector = fileStartSec + sectorInFile;
        LIBCD_CdIntToPos(newDiscSector, file.new_io_loc);

        // Figure out the offset within the destination sector we want to go to
        file.io_block_offset = ((uint32_t)(file.file.size - offset) % CD_SECTOR_SIZE);
    }

    // Return '0' for success - this never actually fails for the retail game.
    // All we are doing here really is modifying the file datastructure.
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the current IO offset within the given file.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_tell(const PsxCd_File& file) noexcept {
    // Is this a real file descriptor or just a dummy one?
    // If it's real figure out the current io offset within the file, otherwise just return '0':
    if (file.file.pos != 0) {
        const int32_t curSec = LIBCD_CdPosToInt(file.new_io_loc);
        const int32_t fileStartSec = LIBCD_CdPosToInt(file.file.pos);
        const int32_t sectorInFile = curSec - fileStartSec;
        const int32_t curOffset = sectorInFile * CD_SECTOR_SIZE + file.io_block_offset;
        return curOffset;
    } else {
        return 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Placeholder for 'closing' a cd file - didn't need to do anything for retail PSX DOOM.
// In the retail .exe an open cd file is simply a struct describing the current IO location, filename etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_close([[maybe_unused]] PsxCd_File& file) noexcept {
    // Nothing to do...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Switches the cdrom into audio playback mode if in data mode
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_set_audio_mode() noexcept {
    if (gPSXCD_psxcd_mode != 0) {
        // Currently in data mode: cancel any async reads
        if (gbPSXCD_async_on) {
            psxcd_async_read_cancel();
        }

        // No longer know what position the cdrom is on for data reading
        gbPSXCD_init_pos = false;

        // Switch to audio mode
        gPSXCD_cd_param[0] = CdlModeRept | CdlModeAP | CdlModeDA;
        gPSXCD_cdl_com = CdlSetmode;
        LIBCD_CdControl(CdlSetmode, gPSXCD_cd_param, nullptr);
        gPSXCD_psxcd_mode = 0;

        // Finish up the last command and pause
        psxcd_sync();
        gPSXCD_cdl_com = CdlPause;
        LIBCD_CdControl(CdlPause, nullptr, nullptr);
        psxcd_sync();

        // Cancel any other commands in flight
        LIBCD_CdFlush();
    } else {
        // Already in audio mode: just complete any pending commands
        psxcd_sync();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pause cd audio playback and make a note of where we paused at
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_pause() noexcept {
    // No longer playing anything
    gbPSXCD_playflag = false;
    gbPSXCD_seeking_for_play = false;

    // If we did not start playback of anything (never set a sector location) then we don't need to do anything else
    if (gPSXCD_lastloc == 0)
        return;

    // Save the last location we read from so we can restore in 'psxcd_restart'
    gPSXCD_lastloc = gPSXCD_newloc;

    // Quickly fade out cd audio if playing
    const int32_t startCdVol = psxspu_get_cd_vol();

    if (startCdVol != 0) {
        psxspu_start_cd_fade(FADE_TIME_MS, 0);
    }
    
    // Ensure no active commands, and issue the pause command
    psxcd_sync();

    gbPSXCD_waiting_for_pause = true;
    gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControlF(CdlPause, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pause cd audio playback and make a note of where we paused at
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_pause() noexcept {
    // No longer playing anything
    gbPSXCD_playflag = false;
    gbPSXCD_seeking_for_play = false;

    // If we did not start playback of anything (never set a sector location) then we don't need to do anything else
    if (gPSXCD_lastloc == 0)
        return;

    // Save the last location we read from so we can restore in 'psxcd_restart'
    gPSXCD_lastloc = gPSXCD_newloc;

    // Quickly fade out cd audio if playing
    const int32_t startCdVol = psxspu_get_cd_vol();

    if (startCdVol != 0) {
        psxspu_start_cd_fade(FADE_TIME_MS, 0);
    }
    
    // Ensure no active commands, and issue the pause command
    psxcd_sync();

    gbPSXCD_waiting_for_pause = true;
    gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControlF(CdlPause, nullptr);
}

#endif  // #if !PC_PSX_DOOM_MODS
