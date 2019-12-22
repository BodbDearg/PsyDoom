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
int32_t OpenFile(const uint32_t fileNum) noexcept {
    // Open the file and do a non recoverable error if it fails
    a0 = fileNum;
    psxcd_open();
    VmPtr<PsxCd_File> pOpenedFile = v0;
    
    if (!pOpenedFile) {
        a0 = 0x80011384;    // Result = STR_CannotOpen_Err[0] (80011384)                            
        a1 = fileNum;
        I_Error();
    }

    // Search for a free cd file slot and abort with an error if not found
    int32_t fileSlotIdx = 0;
    VmPtr<PsxCd_File> pFileSlot = gOpenPsxCdFiles;
    
    do {
        if (pFileSlot->file.size == 0)
            break;
        
        ++fileSlotIdx;
        ++pFileSlot;
    } while (fileSlotIdx < MAX_OPEN_FILES);
    
    if (fileSlotIdx >= MAX_OPEN_FILES) {
        a0 = 0x80011394;    // Result = STR_OpenFile_TooManyFiles_Err[0] (80011394)              
        I_Error();
    }
    
    // Save the opened file and return the opened file slot index
    *pFileSlot = *pOpenedFile;
    return fileSlotIdx;
}

void _thunk_OpenFile() noexcept {
    v0 = OpenFile(a0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Closes the previously opened file slot index
//------------------------------------------------------------------------------------------------------------------------------------------
void CloseFile(const int32_t fileSlotIdx) noexcept {
    VmPtr<PsxCd_File> pFile = &gOpenPsxCdFiles[fileSlotIdx];
    
    a0 = pFile;
    psxcd_close();

    pFile->file.size = 0;
}

void _thunk_CloseFile() noexcept {
    CloseFile(a0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek within the specified file using the given seek mode and offset.
// Returns the offset within the CD file after the seek.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t SeekAndTellFile(const int32_t fileSlotIdx, const int32_t offset, const PsxCd_SeekMode seekMode) noexcept {
    VmPtr<PsxCd_File> pFile = &gOpenPsxCdFiles[fileSlotIdx];

    a0 = pFile;
    a1 = offset;
    a2 = (uint32_t) seekMode;
    psxcd_seek();

    a0 = pFile;
    psxcd_tell();
    const int32_t newOffset = v0;

    return newOffset;
}

void _thunk_SeekAndTellFile() noexcept {
    v0 = SeekAndTellFile(a0, a1, (PsxCd_SeekMode) a2);
}

void ReadFile() noexcept {
loc_8003206C:
    sp -= 0x28;
    sw(s4, sp + 0x20);
    s4 = a1;
    sw(s2, sp + 0x18);
    s2 = a2;
    v0 = a0 << 2;
    v0 += a0;
    v0 <<= 3;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x6270;                                       // Result = gOpenPsxCdFiles[0] (800A9D90)
    sw(s1, sp + 0x14);
    s1 = v0 + v1;
    a0 = s1;
    sw(ra, sp + 0x24);
    sw(s3, sp + 0x1C);
    sw(s0, sp + 0x10);
    psxcd_tell();
    s3 = v0;
    v0 = (s2 < 0x2001);
    s0 = s2;
    if (v0 != 0) goto loc_800320C4;
    s0 = 0x2000;                                        // Result = 00002000
loc_800320C4:
    a0 = s1;
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    psxcd_seek();
    a0 = s4;
    a1 = s0;
    a2 = s1;
    psxcd_read();
    a0 = s1;
    a1 = s3;
    a2 = 0;                                             // Result = 00000000
    psxcd_seek();
    a0 = s4;
    a1 = s2;
    a2 = s1;
    psxcd_read();
    s0 = v0;
    a1 = s0;
    if (s0 == s2) goto loc_80032120;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x13B4;                                       // Result = STR_ReadFile_Read_Err[0] (800113B4)
    a2 = s2;
    I_Error();
loc_80032120:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}
