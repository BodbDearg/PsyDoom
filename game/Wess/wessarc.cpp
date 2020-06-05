//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): platform specific utilities
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessarc.h"

#include "psxcmd.h"
#include "psxspu.h"
#include "PsyQ/LIBAPI.h"
#include "PsyQ/LIBSPU.h"
#include "wessseq.h"

const WessDriverFunc* const gWess_CmdFuncArr[10] = {
    gWess_DrvFunctions,
    gWess_drv_cmds,
    gWess_DrvFunctions,
    gWess_DrvFunctions,
    gWess_DrvFunctions,
    gWess_DrvFunctions,
    gWess_DrvFunctions,
    gWess_DrvFunctions,
    gWess_DrvFunctions,
    gWess_DrvFunctions
};

// Keeps track of global time (MS) for the sequencer and other operations.
// The fractional part is used for greater precision in carrying over fractional parts of milliseconds.
const VmPtr<uint32_t>   gWess_Millicount(0x80075954);
const VmPtr<uint32_t>   gWess_Millicount_Frac(0x80075958);

// True if the 'WessInterruptHandler' function is active and receiving periodic callbacks
const VmPtr<bool32_t>   gbWess_WessTimerActive(0x8007594C);

// Temporary buffers used for holding a sector worth of data
const VmPtr<uint8_t[CD_SECTOR_SIZE]> gWess_sectorBuffer1(0x8009656C);
const VmPtr<uint8_t[CD_SECTOR_SIZE]> gWess_sectorBuffer2(0x80096D7C);

// True when the sequencer is enabled and can tick
const VmPtr<bool32_t> gbWess_SeqOn(0x80075948);

static const VmPtr<uint32_t>    gWess_T2counter(0x80075950);            // Tracks the number of PSX timer 2 interrupts or calls to 'WessInterruptHandler'
static const VmPtr<uint32_t>    gWess_EV2(0x8007595C);                  // Hardware event handle for the PSX timer 2 interrupt event which drives the music system
static const VmPtr<PsxCd_File>  gWess_module_fileref(0x8007EFFC);       // Holds the current file open by the module loader
static const VmPtr<PsxCd_File>  gWess_data_fileref(0x8007F024);         // Holds the current file open by the data loader
static const VmPtr<bool32_t>    gbWess_ReadChunk1(0x8007F04C);          // If true we can read data to sector buffer 1
static const VmPtr<bool32_t>    gbWess_ReadChunk2(0x8007EFF8);          // If true we can read data to sector buffer 2

//------------------------------------------------------------------------------------------------------------------------------------------
// Gives the number of ticks or interrupts per second the music system uses.
// In PSX DOOM the sequencer runs at roughly 120 Hz intervals.
//------------------------------------------------------------------------------------------------------------------------------------------
int16_t GetIntsPerSec() noexcept {
    return 120;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Calculates the number of quarter note per hardware timer interrupt, in 16.16 fixed point format.
// This is used to determine how fast to pace/advance the sequencer.
//
// Params:
//  intsPerSec      :   How many hardware interrupts happen a second. Approx '120' for PSX DOOM.
//  partsPerQNote   :   How many parts to subdivide each quarter note into. Affects timing precision.
//  qnotesPerMin    :   Track tempo in quarter notes per minute. This typically known as 'beats per minute'.
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t CalcPartsPerInt(const int16_t intsPerSec, const int16_t partsPerQNote, const int16_t qnotesPerMin) noexcept {
    const uint32_t intsPerMin = intsPerSec * 60;                                                // Number of interrupts per minute
    const uint32_t qnotePerMinFrac = (uint32_t) qnotesPerMin << 16;                             // Number of quarter notes per minute in 16.16 format
    const uint32_t qnotesPerIntRoundUp = intsPerSec * 30 + 30;                                  // Helps round up the number of quarter notes per interrupt (ceil)
    const uint32_t qnotesPerIntFrac = (qnotePerMinFrac + qnotesPerIntRoundUp) / intsPerMin;     // Number of quarter notes per interrupt (16.16)
    const uint32_t partsPerInt = qnotesPerIntFrac * partsPerQNote;                              // Number of quarter note 'parts' per interrupt (16.16)
    return partsPerInt;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// This is the root update/step function for the music and sound sequencer.
// Originally on the actual PlayStation this would have been triggered via hardware timer interrupts at approximately 121.9284 Hz.
// For all code dealing with the timer however, it's just assumed to be firing at 120 Hz.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t WessInterruptHandler() noexcept {
    // Increment the number of interrupts
    *gWess_T2counter += 1;

    // How much to step the millicount each time, in 16.16 format.
    // This is approximately 1000/120 or 8.33333...
    constexpr uint32_t MS_FRAC_STEP = 0x85555;

    // Advance the millicount, both the whole and fractional parts
    const uint32_t elapsedMillicount_Frac = *gWess_Millicount_Frac + MS_FRAC_STEP;
    
    *gWess_Millicount += elapsedMillicount_Frac >> 16;
    *gWess_Millicount_Frac = elapsedMillicount_Frac & 0xFFFF;

    // Update SPU volume fades
    psxspu_fadeengine();

    // Execute the sequencer engine if it is enabled
    if (*gbWess_SeqOn) {
        SeqEngine();
    }

    // Not sure what the return value is used for, I couldn't find any documentation on it anywhere.
    // It may have been unused and certainly IS unused in this port...
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up timer interrupts and the timer interrupt handler which drive the entire music and sound sequencer
//------------------------------------------------------------------------------------------------------------------------------------------
void init_WessTimer() noexcept {
    // The sequencer is disabled while doing this as are interrupts
    *gbWess_SeqOn = false;
    LIBAPI_EnterCriticalSection();
    
    // Create and enable a hardware event to handle interrupts generated by the PlayStation's 'root counter 2' timer
    *gWess_EV2 = LIBAPI_OpenEvent(RCntCNT2, EvSpINT, EvMdINTR, WessInterruptHandler);
    LIBAPI_EnableEvent(*gWess_EV2);

    // Set the timer to fire at roughly 121.9284 Hz intervals.
    // Each unit for root counter 2 is 8 CPU cycles and the PlayStation's CPU runs at approximately 33,868,800 Hz - hence the 121.9284 Hz figure.
    // I guess approximately 120 Hz intervals was actually intended though?
    LIBAPI_SetRCnt(RCntCNT2, 34722, RCntMdINTR);

    // Start the timer generating interrupt events
    LIBAPI_StartRCnt(RCntCNT2);

    // The timer handler is now installed, and we can re-enable interrupts
    *gbWess_WessTimerActive = true;
    LIBAPI_ExitCriticalSection();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shut down the timer interrupts powering the sequencer system
//------------------------------------------------------------------------------------------------------------------------------------------
void exit_WessTimer() noexcept {
    // Make sure volume levels are actually what we think they are
    const int32_t masterVol = psxspu_get_master_vol();
    psxspu_set_master_vol(masterVol);

    const int32_t cdVol = psxspu_get_cd_vol();
    psxspu_set_cd_vol(cdVol);

    // Disable interrupts, remove the timer event and renable interrupts
    LIBAPI_EnterCriticalSection();

    *gbWess_WessTimerActive =  false;
    LIBAPI_DisableEvent(*gWess_EV2);
    LIBAPI_CloseEvent(*gWess_EV2);
    
    LIBAPI_ExitCriticalSection();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do platform specific initialization for loading files and return 'true' if initialization was successful.
// This did nothing for PSX DOOM and was never called...
//------------------------------------------------------------------------------------------------------------------------------------------
bool Wess_init_for_LoadFileData() noexcept {
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform I/O while reading a module file: open the specified module file for reading and return it's pointer
//------------------------------------------------------------------------------------------------------------------------------------------
PsxCd_File* module_open(const CdMapTbl_File fileId) noexcept {
    PsxCd_File* const pFile = psxcd_open(fileId);
    *gWess_module_fileref = *pFile;
    return gWess_module_fileref.get();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform I/O while reading a module file: read the specified number of bytes from the given file.
// Returns the number of bytes read.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t module_read(void* const pDest, const int32_t numBytes, PsxCd_File& file) noexcept {
    return psxcd_read(pDest, numBytes, file);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform I/O while reading a module file: seek to the specified position in the given file.
// Returns '0' on success, any other value on failure.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t module_seek(PsxCd_File& file, const int32_t seekPos, const PsxCd_SeekMode seekMode) noexcept {
    return psxcd_seek(file, seekPos, seekMode);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform I/O while reading a module file: returns the current IO offset within the given file
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t module_tell(const PsxCd_File& file) noexcept {
    return psxcd_tell(file);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform I/O while reading a module file: close up the given file
//------------------------------------------------------------------------------------------------------------------------------------------
void module_close(PsxCd_File& file) noexcept {
    psxcd_close(file);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the number of sound drivers available
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t get_num_Wess_Sound_Drivers([[maybe_unused]] const int32_t* const* const pSettingTagLists) noexcept {
    return 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open the given sound file for reading
//------------------------------------------------------------------------------------------------------------------------------------------
PsxCd_File* data_open(const CdMapTbl_File fileId) noexcept {
    const PsxCd_File* const pFile = psxcd_open(fileId);
    *gWess_data_fileref = *pFile;
    return gWess_data_fileref.get();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a chunk of sound data from the given file and upload to the spu at the given address
//------------------------------------------------------------------------------------------------------------------------------------------
static void data_read_chunk(PsxCd_File& file, const int32_t chunkSize, const uint32_t spuDestAddr) noexcept {
    // This code alternates between reading to sector buffers 1 & 2.
    // While it is reading from the CD it is hoped that other data is uploading to the SPU.
    // In all cases after switching buffers and writing the new data to the SPU, wait for the previous transfer to complete (if there is one).
    if (*gbWess_ReadChunk1) {
        // Reading to chunk 1: very first read.
        // Don't need to wait on a previous transfer in this case because there is none.
        psxcd_read(gWess_sectorBuffer1.get(), chunkSize, file);
        LIBSPU_SpuSetTransferStartAddr(spuDestAddr);
        LIBSPU_SpuWrite(gWess_sectorBuffer1.get(), chunkSize);
        *gbWess_ReadChunk2 = true;
        *gbWess_ReadChunk1 = false;
    } else {
        if (*gbWess_ReadChunk2) {
            // Reading to chunk 2
            psxcd_read(gWess_sectorBuffer2.get(), chunkSize, file);
            LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
            LIBSPU_SpuSetTransferStartAddr(spuDestAddr);
            LIBSPU_SpuWrite(gWess_sectorBuffer2.get(), chunkSize);
            *gbWess_ReadChunk2 = false;
        } else {
            // Reading to chunk 1: not the first read
            psxcd_read(gWess_sectorBuffer1.get(), chunkSize, file);
            LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
            LIBSPU_SpuSetTransferStartAddr(spuDestAddr);
            LIBSPU_SpuWrite(gWess_sectorBuffer1.get(), chunkSize);
            *gbWess_ReadChunk2 = true;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read a number of bytes from the given sound file to the specified address in SPU RAM.
// Returns the number of bytes that were written to SPU RAM.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t data_read(PsxCd_File& file, const int32_t destSpuAddr, const int32_t numBytes, const int32_t fileOffset) noexcept {
    // Will the transfer fit? Ignore if it doesn't...
    const int32_t maxBytes = gPsxSpu_sram_end - destSpuAddr;

    if (numBytes > maxBytes)
        return 0;

    // Seek to the specified location and begin reading sector sized chunks
    psxcd_seek(file, fileOffset, PsxCd_SeekMode::SET);

    *gbWess_ReadChunk1 = true;
    uint32_t numBytesLeft = numBytes;
    uint32_t curDestSpuAddr = destSpuAddr;

    while (numBytesLeft >= CD_SECTOR_SIZE) {
        data_read_chunk(file, CD_SECTOR_SIZE, curDestSpuAddr);
        numBytesLeft -= CD_SECTOR_SIZE;
        curDestSpuAddr += CD_SECTOR_SIZE;
    }

    // Read what remains, if anything
    if (numBytesLeft > 0) {
        data_read_chunk(file, numBytesLeft, curDestSpuAddr);
    }

    // Wait for the SPU transfer to finish before exiting
    LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
    return numBytes;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Close the given sound file that was opened for reading
//------------------------------------------------------------------------------------------------------------------------------------------
void data_close(PsxCd_File& file) noexcept {
    psxcd_close(file);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialization function to setup platform specific WESS stuff
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_low_level_init() noexcept {
    psxspu_init();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleanup function to shut down platform specific WESS stuff
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_low_level_exit() noexcept {
    // Didn't do anything...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocate memory for the sound system
//------------------------------------------------------------------------------------------------------------------------------------------
void* wess_malloc([[maybe_unused]] const int32_t size) noexcept {
    // Not implemented in PSX DOOM - always returned 'null'
    return nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Free memory for the sound system
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_free([[maybe_unused]] void* const pMem) noexcept {
    // Not implemented in PSX DOOM because 'wess_malloc' wasn't implemented...
}
