#include "i_file.h"

#include "i_main.h"
#include "PsxVm/PsxVm.h"
#include "Wess/psxcd.h"

static constexpr int32_t MAX_OPEN_FILES = 4;

static const VmPtr<PsxCd_File[MAX_OPEN_FILES]> gOpenPsxCdFiles(0x800A9D90);

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the open file records for the game.
// There is a maximum of 4 files that can be open at once.
//------------------------------------------------------------------------------------------------------------------------------------------
void InitOpenFileSlots() noexcept {
    VmPtr<PsxCd_File> pFile = &gOpenPsxCdFiles[MAX_OPEN_FILES - 1];

    do {
        pFile->file.size = 0;
        --pFile;
    } while (pFile >= gOpenPsxCdFiles);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open the specified file number in the array of predefined CD files.
// Returns the index of the file in the list file slots.
// Kills the program with an error on failure to open.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t OpenFile(const CdMapTbl_File discFile) noexcept {
    // Open the file and do a non recoverable error if it fails
    const PsxCd_File* const pOpenedFile = psxcd_open(discFile);

    if (!pOpenedFile) {
        I_Error("Cannot open %u", (uint32_t) discFile);
    }

    // Search for a free cd file slot and abort with an error if not found
    int32_t fileSlotIdx = 0;
    VmPtr<PsxCd_File> pFileSlot = gOpenPsxCdFiles.get();
    
    do {
        if (pFileSlot->file.size == 0)
            break;
        
        ++fileSlotIdx;
        ++pFileSlot;
    } while (fileSlotIdx < MAX_OPEN_FILES);
    
    if (fileSlotIdx >= MAX_OPEN_FILES) {
        I_Error("OpenFile: Too many open files!");
    }
    
    // Save the opened file and return the opened file slot index
    *pFileSlot = *pOpenedFile;
    return fileSlotIdx;
}

void _thunk_OpenFile() noexcept {
    v0 = OpenFile((CdMapTbl_File) a0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes the previously opened file slot index
//------------------------------------------------------------------------------------------------------------------------------------------
void CloseFile(const int32_t fileSlotIdx) noexcept {
    PsxCd_File& file = gOpenPsxCdFiles[fileSlotIdx];
    psxcd_close(file);
    file.file.size = 0;
}

void _thunk_CloseFile() noexcept {
    CloseFile(a0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek within the specified file using the given seek mode and offset.
// Returns the offset within the CD file after the seek.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t SeekAndTellFile(const int32_t fileSlotIdx, const int32_t offset, const PsxCd_SeekMode seekMode) noexcept {
    PsxCd_File& file = gOpenPsxCdFiles[fileSlotIdx];
    psxcd_seek(file, offset, seekMode);
    return psxcd_tell(file);
}

void _thunk_SeekAndTellFile() noexcept {
    v0 = SeekAndTellFile(a0, a1, (PsxCd_SeekMode) a2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the specified number of bytes from the given file.
// If the read fails then the program dies with an error.
//------------------------------------------------------------------------------------------------------------------------------------------
void ReadFile(const int32_t fileSlotIdx, void* const pBuffer, const uint32_t size) noexcept {
    // Grab the file being read and see what offset we are at in the file.
    // We will seek to that offset before the read:
    PsxCd_File& file = gOpenPsxCdFiles[fileSlotIdx];
    const int32_t curFileOffset = psxcd_tell(file);

    // This is really strange... PSX DOOM appears to do some sort of dummy read of 8192 bytes at the start of the file
    // before reading the actual data that has been requested. I don't know why this was done, maybe perhaps to flush
    // out whatever the CD-ROM is currently doing with audio or something like that?
    //
    // I tried disabling this previously because I thought it would be useless in an emulated PSX environment but
    // it turns out that it actually DOES cause problems if I omit this code.
    // Will need to read up more on the PSX hardware to find out why this is required.
    {
        constexpr uint32_t WARMUP_READ_MAX_SIZE = 8192;
        uint32_t warmupReadSize = size;
    
        if (size > WARMUP_READ_MAX_SIZE) {
            warmupReadSize = WARMUP_READ_MAX_SIZE;
        }

        psxcd_seek(file, 0, PsxCd_SeekMode::SET);
        psxcd_read(pBuffer, warmupReadSize, file);
    }

    // This is where we actually seek to the file offset and read it
    psxcd_seek(file, curFileOffset, PsxCd_SeekMode::SET);
    const int32_t numBytesRead = psxcd_read(pBuffer, (int32_t) size, file);

    // If the read failed then kill the program with an error
    if (numBytesRead != (int32_t) size) {
        I_Error("ReadFile: error reading %d of %u bytes\n", numBytesRead, size);
    }
}

void _thunk_ReadFile() noexcept {
    ReadFile(a0, vmAddrToPtr<void>(a1), a2);
}
