//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): LCD (sound samples file) loading code.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "lcdload.h"

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
    const PsxCd_File* const pFile = psxcd_open(file);
    *gWess_open_lcd_file = *pFile;
    return gWess_open_lcd_file.get();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Uploads sound data from a CD sized sector (2048 bytes) to the SPU at the specified address.
// Also handles transitioning from one sound in the LCD file to the next as each sound upload is completed.
// Optionally, details about the uploaded sound(s) are saved to the given sample block upon successful upload.
// The details for previous (still valid) uploads can also be forcefully overwritten by the 'bOverride' flag.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_dig_lcd_data_read(uint8_t* const pSectorData, const uint32_t destSpuAddr, SampleBlock* const pSampleBlock, const bool bOverride) noexcept {
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
                const bool bOutOfSpuRam = ((int32_t) destSpuAddr + nextPatchInfo.sample_size + sndDataOffset > (int32_t) *gPsxSpu_sram_end);
            #else
                const bool bOutOfSpuRam = ((int32_t) destSpuAddr + nextPatchInfo.sample_size > (int32_t)(*gPsxSpu_sram_end + sndDataOffset));
            #endif

            if (bOutOfSpuRam) {
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

void wess_dig_lcd_psxcd_sync() noexcept {
loc_800493AC:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    s0 += 0x1F40;
    v0 = (v0 < s0);
    sw(s1, sp + 0x14);
    if (v0 == 0) goto loc_80049434;
    s2 = 5;                                             // Result = 00000005
    s1 = 2;                                             // Result = 00000002
loc_800493E4:
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x5AF8;                                       // Result = 80075AF8
    a0 = 1;                                             // Result = 00000001
    _thunk_LIBCD_CdSync();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AF4);                                // Store to: 80075AF4
    if (v0 != s2) goto loc_80049414;
    LIBCD_CdFlush();
    v0 = 1;                                             // Result = 00000001
    goto loc_80049438;
loc_80049414:
    {
        const bool bJump = (v0 == s1);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80049438;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5954);                               // Load from: gWess_Millicount (80075954)
    v0 = (v0 < s0);
    if (v0 != 0) goto loc_800493E4;
loc_80049434:
    v0 = 1;                                             // Result = 00000001
loc_80049438:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void wess_dig_lcd_load() noexcept {
loc_80049454:
    sp -= 0x40;
    sw(fp, sp + 0x38);
    fp = a0;
    sw(s6, sp + 0x30);
    s6 = a2;
    sw(s7, sp + 0x34);
    s7 = a3;
    sw(ra, sp + 0x3C);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    sw(a1, sp + 0x10);
    psxcd_disable_callbacks();
loc_80049494:
    s5 = 1;                                             // Result = 00000001
    s4 = 5;                                             // Result = 00000005
loc_8004949C:
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5AE0);                                 // Store to: 80075AE0
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x5AE4);                                 // Store to: 80075AE4
    s3 = 0;                                             // Result = 00000000
    psxcd_init_pos();
    psxcd_set_data_mode();
    a0 = fp;
    v0 = ptrToVmAddr(wess_dig_lcd_data_open((CdMapTbl_File) a0));
    s0 = v0;
    v0 = lw(s0);
    s2 = lw(sp + 0x10);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800497A4;
    }
    s1 = lw(s0 + 0x4);
    a0 = s0;
    _thunk_LIBCD_CdPosToInt();
    a0 = v0;
    s0 += 0x18;
    a1 = s0;
    _thunk_LIBCD_CdIntToPos();
    a0 = 2;                                             // Result = 00000002
    a1 = s0;
    a2 = 0;                                             // Result = 00000000
    _thunk_LIBCD_CdControl();
    a0 = 6;                                             // Result = 00000006
    a1 = s0;
    a2 = 0;                                             // Result = 00000000
    _thunk_LIBCD_CdControl();
    a0 = 1;                                             // Result = 00000001
loc_80049518:
    a1 = 0;                                             // Result = 00000000
    _thunk_LIBCD_CdReady();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AF0);                                // Store to: 80075AF0
    if (v0 == s5) goto loc_80049540;
    a0 = 1;                                             // Result = 00000001
    if (v0 != s4) goto loc_80049518;
    LIBCD_CdFlush();
loc_80049540:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AF0);                               // Load from: 80075AF0
    if (v0 == s4) goto loc_8004949C;
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6AE8;                                       // Result = gPSXCD_sectorbuf[0] (800A9518)
    a0 = s0;                                            // Result = gPSXCD_sectorbuf[0] (800A9518)
    a1 = 0x200;                                         // Result = 00000200
    _thunk_LIBCD_CdGetSector();
    v0 = lhu(s0);                                       // Load from: gPSXCD_sectorbuf[0] (800A9518)
    at = 0x80070000;                                    // Result = 80070000
    sh(v0, at + 0x5AE8);                                // Store to: 80075AE8
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lhu(v0 + 0x5AE8);                              // Load from: 80075AE8
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x5AEC);                                // Store to: 80075AEC
    v0 = (v0 < 0x65);
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s1) < 0x800);
        if (bJump) goto loc_8004949C;
    }
    s1 -= 0x800;
    if (v0 == 0) goto loc_800495A0;
    s1 = 0;                                             // Result = 00000000
loc_800495A0:
    s0 = 1;                                             // Result = 00000001
    v0 = 1;                                             // Result = 00000001
    if (s1 == 0) goto loc_80049760;
loc_800495AC:
    a0 = 1;                                             // Result = 00000001
    if (s0 == 0) goto loc_8004963C;
loc_800495B4:
    a1 = 0;                                             // Result = 00000000
    _thunk_LIBCD_CdReady();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AF0);                                // Store to: 80075AF0
    if (v0 == s5) goto loc_800495DC;
    a0 = 1;                                             // Result = 00000001
    if (v0 != s4) goto loc_800495B4;
    LIBCD_CdFlush();
loc_800495DC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AF0);                               // Load from: 80075AF0
    if (v0 == s4) goto loc_80049494;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = 0x200;                                         // Result = 00000200
    _thunk_LIBCD_CdGetSector();
    v0 = (i32(s1) < 0x800);
    s1 -= 0x800;
    if (v0 == 0) goto loc_80049610;
    s1 = 0;                                             // Result = 00000000
loc_80049610:
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = s2;
    a2 = s6;
    a3 = s7;
    v0 = wess_dig_lcd_data_read(vmAddrToPtr<uint8_t>(a0), a1, vmAddrToPtr<SampleBlock>(a2), (a3 != 0));
    s3 += v0;
    s2 += v0;
    v0 = 1;                                             // Result = 00000001
    s0 = 0;                                             // Result = 00000000
    goto loc_80049758;
loc_8004963C:
    if (v0 == 0) goto loc_800496D0;
loc_80049644:
    a1 = 0;                                             // Result = 00000000
    _thunk_LIBCD_CdReady();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AF0);                                // Store to: 80075AF0
    if (v0 == s5) goto loc_8004966C;
    a0 = 1;                                             // Result = 00000001
    if (v0 != s4) goto loc_80049644;
    LIBCD_CdFlush();
loc_8004966C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AF0);                               // Load from: 80075AF0
    if (v0 == s4) goto loc_80049494;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x6D7C;                                       // Result = gWess_data_read_chunk2[0] (80096D7C)
    a1 = 0x200;                                         // Result = 00000200
    _thunk_LIBCD_CdGetSector();
    v0 = (i32(s1) < 0x800);
    s1 -= 0x800;
    if (v0 == 0) goto loc_800496A0;
    s1 = 0;                                             // Result = 00000000
loc_800496A0:
    v0 = LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x6D7C;                                       // Result = gWess_data_read_chunk2[0] (80096D7C)
    a1 = s2;
    a2 = s6;
    a3 = s7;
    v0 = wess_dig_lcd_data_read(vmAddrToPtr<uint8_t>(a0), a1, vmAddrToPtr<SampleBlock>(a2), (a3 != 0));
    s3 += v0;
    s2 += v0;
    v0 = 0;                                             // Result = 00000000
    goto loc_80049758;
loc_800496D0:
    a1 = 0;                                             // Result = 00000000
    _thunk_LIBCD_CdReady();
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x5AF0);                                // Store to: 80075AF0
    if (v0 == s5) goto loc_800496F8;
    a0 = 1;                                             // Result = 00000001
    if (v0 != s4) goto loc_800496D0;
    LIBCD_CdFlush();
loc_800496F8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AF0);                               // Load from: 80075AF0
    if (v0 == s4) goto loc_80049494;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = 0x200;                                         // Result = 00000200
    _thunk_LIBCD_CdGetSector();
    v0 = (i32(s1) < 0x800);
    s1 -= 0x800;
    if (v0 == 0) goto loc_8004972C;
    s1 = 0;                                             // Result = 00000000
loc_8004972C:
    v0 = LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x656C;                                       // Result = gWess_data_read_chunk1[0] (8009656C)
    a1 = s2;
    a2 = s6;
    a3 = s7;
    v0 = wess_dig_lcd_data_read(vmAddrToPtr<uint8_t>(a0), a1, vmAddrToPtr<SampleBlock>(a2), (a3 != 0));
    s3 += v0;
    s2 += v0;
    v0 = 1;                                             // Result = 00000001
loc_80049758:
    if (s1 != 0) goto loc_800495AC;
loc_80049760:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5AF0);                               // Load from: 80075AF0
    a0 = 9;                                             // Result = 00000009
    if (v0 == s4) goto loc_8004949C;
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    _thunk_LIBCD_CdControl();
    wess_dig_lcd_psxcd_sync();
    if (v0 != 0) goto loc_8004949C;
    v0 = LIBSPU_SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
    psxcd_enable_callbacks();
    v0 = s3;
loc_800497A4:
    ra = lw(sp + 0x3C);
    fp = lw(sp + 0x38);
    s7 = lw(sp + 0x34);
    s6 = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x40;
    return;
}
