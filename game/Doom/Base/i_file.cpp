#include "i_file.h"

#include "i_main.h"
#include "String8_16.h"

static constexpr int32_t MAX_OPEN_FILES = 4;

static PsxCd_File gOpenPsxCdFiles[MAX_OPEN_FILES];

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the open file records for the game.
// There is a maximum of 4 files that can be open at once.
//------------------------------------------------------------------------------------------------------------------------------------------
void InitOpenFileSlots() noexcept {
    for (int32_t i = 0; i < MAX_OPEN_FILES; ++i) {
        // PsyDoom: the 'PsxCd_File' struct has changed layout & contents
        #if PSYDOOM_MODS
            gOpenPsxCdFiles[i] = {};
        #else
            gOpenPsxCdFiles[i].file.size = 0;
        #endif
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open the specified file number in the array of predefined CD files.
// Returns the index of the file in the list file slots.
// Kills the program with an error on failure to open.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t OpenFile(const CdFileId discFile) noexcept {
    // Open the file and do a non recoverable error if it fails
    const PsxCd_File* const pOpenedFile = psxcd_open(discFile);

    if (!pOpenedFile) {
        // PsyDoom: the file id is now a fixed size string
        #if PSYDOOM_MODS
            const auto fileName = discFile.c_str();
            I_Error("Cannot open %s!", fileName.data());
        #else
            I_Error("Cannot open %u", (uint32_t) discFile);
        #endif
    }

    // Search for a free cd file slot and abort with an error if not found
    int32_t fileSlotIdx = 0;

    for (; fileSlotIdx < MAX_OPEN_FILES; ++fileSlotIdx) {
        // PsyDoom: the 'PsxCd_File' struct has changed layout & contents
        #if PSYDOOM_MODS
            if (gOpenPsxCdFiles[fileSlotIdx].size == 0)
                break;
        #else
            if (gOpenPsxCdFiles[fileSlotIdx].file.size == 0)
                break;
        #endif
    }

    if (fileSlotIdx >= MAX_OPEN_FILES) {
        I_Error("OpenFile: Too many open files!");
    }

    // Save the opened file and return the opened file slot index
    gOpenPsxCdFiles[fileSlotIdx] = *pOpenedFile;
    return fileSlotIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes the previously opened file slot index
//------------------------------------------------------------------------------------------------------------------------------------------
void CloseFile(const int32_t fileSlotIdx) noexcept {
    PsxCd_File& file = gOpenPsxCdFiles[fileSlotIdx];
    psxcd_close(file);

    // PsyDoom: the 'PsxCd_File' struct has changed layout & contents
    #if PSYDOOM_MODS
        file = {};
    #else
        file.file.size = 0;
    #endif
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the specified number of bytes from the given file.
// If the read fails then the program dies with an error.
//------------------------------------------------------------------------------------------------------------------------------------------
void ReadFile(const int32_t fileSlotIdx, void* const pBuffer, const uint32_t size) noexcept {
    PsxCd_File& file = gOpenPsxCdFiles[fileSlotIdx];

    // PsyDoom: this is no longer neccessary
    #if !PSYDOOM_MODS
    {
        // This is really strange... PSX DOOM appears to do some sort of dummy read of 8192 bytes at the start of the file
        // before reading the actual data that has been requested. I don't know why this was done, maybe perhaps to flush
        // out whatever the CD-ROM is currently doing with audio or something like that?
        const int32_t curFileOffset = psxcd_tell(file);

        {
            constexpr uint32_t WARMUP_READ_MAX_SIZE = 8192;
            uint32_t warmupReadSize = size;

            if (size > WARMUP_READ_MAX_SIZE) {
                warmupReadSize = WARMUP_READ_MAX_SIZE;
            }

            psxcd_seek(file, 0, PsxCd_SeekMode::SET);
            psxcd_read(pBuffer, warmupReadSize, file);
        }

        // Restore the proper offset
        psxcd_seek(file, curFileOffset, PsxCd_SeekMode::SET);
    }
    #endif

    const int32_t numBytesRead = psxcd_read(pBuffer, (int32_t) size, file);

    // If the read failed then kill the program with an error
    if (numBytesRead != (int32_t) size) {
        I_Error("ReadFile: error reading %d of %u bytes\n", numBytesRead, size);
    }
}
