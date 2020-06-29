#include "Old_psxcd.h"

#if !PC_PSX_DOOM_MODS

static int32_t  gPSXCD_sync_intr;           // Int result of the last 'LIBCD_CdSync' call when waiting until command completion
static uint8_t  gPSXCD_sync_result[8];      // Result bytes for the last 'LIBCD_CdSync' call when waiting until command completion

// Function forward declarations
static int32_t psxcd_async_read(void* const pDest, const int32_t numBytes, PsxCd_File& file) noexcept;

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

#endif  // #if !PC_PSX_DOOM_MODS
