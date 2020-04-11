//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): mostly core API functions, including module (.WMD) file loading
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessapi.h"

#include "PcPsx/Finally.h"
#include "psxcmd.h"
#include "PsxVm/PsxVm.h"
#include "PcPsx/Endian.h"
#include "wessapi_t.h"
#include "wessarc.h"
#include "wessseq.h"

// 4 byte identifier for WMD (Williams Module) files: says 'SPSX'
static constexpr uint32_t WESS_MODULE_ID = Endian::littleToHost(0x58535053);

// Expected WMD (module) file version
static constexpr uint32_t WESS_MODULE_VER = 1;

// Minimum tracks in a sequence
static constexpr uint8_t MINIMUM_TRACK_INDXS_FOR_A_SEQUENCE = 4;

const VmPtr<bool32_t>                           gbWess_module_loaded(0x800758F8);       // If true then a WMD file (module) has been loaded
const VmPtr<VmPtr<master_status_structure>>     gpWess_pm_stat(0x800A8758);             // Master status structure for the entire sequencer system

static const VmPtr<bool32_t>            gbWess_sysinit(0x800758F4);             // Set to true once the WESS API has been initialized
static const VmPtr<bool32_t>            gbWess_early_exit(0x800758FC);          // Unused flag in PSX DOOM, I think to request the API to exit?
static const VmPtr<int32_t>             gWess_num_sd(0x800758E4);               // The number of sound drivers available
static const VmPtr<VmPtr<uint8_t>>      gpWess_wmd_mem(0x8007590C);             // The memory block that the module and all sequences are loaded into. Contains master status struct and it's children.
static const VmPtr<bool32_t>            gbWess_wmd_mem_is_mine(0x80075908);     // If true then the WESS API allocated the WMD memory block and thus is responsible for its cleanup
static const VmPtr<VmPtr<uint8_t>>      gpWess_wmd_end(0x80075910);             // Current end of the used portion of the WMD memory block
static const VmPtr<int32_t>             gWess_wmd_size(0x80075914);             // Size of the WMD memory block allocated, or upper limit on sequencer memory use
static const VmPtr<int32_t>             gWess_mem_limit(0x80075904);            // Size of the WMD memory block that the WESS API was INTENDING to allocate (alloc may have failed)
static const VmPtr<int32_t>             gWess_end_seq_num(0x80075900);          // Maximum sequence number (exclusive) in the loaded module file
static const VmPtr<VmPtr<PsxCd_File>>   gpWess_fp_wmd_file(0x800758F0);         // File object for the module (.WMD) file on-disc
static const VmPtr<VmPtr<uint8_t>>      gpWess_wmdFileBytesBeg(0x800758EC);     // Pointer to the start of a buffer containing the serialized .WMD file, as it was read from the disc
static const VmPtr<VmPtr<uint8_t>>      gpWess_curWmdFileBytes(0x800758E8);     // Location in the buffer containing the serialized .WMD file to read from next

// Unused error handling stuff.
// May have only been used in debug builds.
static int32_t (*gpWess_Error_func)(int32_t, int32_t) = nullptr;
static int32_t gWess_Error_module = 0;

//------------------------------------------------------------------------------------------------------------------------------------------
// Zero fill a region of memory
//------------------------------------------------------------------------------------------------------------------------------------------
void zeroset(void* const pDest, const uint32_t numBytes) noexcept {
    uint8_t* const pDestBytes = (uint8_t*) pDest;

    for (uint32_t byteIdx = 0; byteIdx < numBytes; ++byteIdx) {
        pDestBytes[byteIdx] = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unused function in the retail PSX DOOM - possibly only called in debug builds: install and save an error handling function.
// Again, this error handler is unused in this build but may have been used in non-release builds.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_install_error_handler(int32_t (* const pErrorFunc)(int32_t, int32_t), const int32_t module) noexcept {
    gpWess_Error_func = pErrorFunc;
    gWess_Error_module = module;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return a pointer to the master status structure for the loaded module
//------------------------------------------------------------------------------------------------------------------------------------------
master_status_structure* wess_get_master_status() noexcept {
    return gpWess_pm_stat->get();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the WESS API has been initialized
//------------------------------------------------------------------------------------------------------------------------------------------
bool Is_System_Active() noexcept {
    return *gbWess_sysinit;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a module file (.WMD) file has been loaded
//------------------------------------------------------------------------------------------------------------------------------------------
bool Is_Module_Loaded() noexcept {
    return *gbWess_module_loaded;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given sequence number is valid
//------------------------------------------------------------------------------------------------------------------------------------------
bool Is_Seq_Num_Valid(const int32_t seqIdx) noexcept {
    if ((seqIdx >= 0) && (seqIdx < *gWess_end_seq_num)) {
        return ((*gpWess_pm_stat)->pmod_info->pseq_info[seqIdx].ptrk_info != nullptr);
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Not sure what this function is for, but it appears unused in the retail game.
// I think it is to request the WESS API to finish up early, possibly a leftover from PC code?
//------------------------------------------------------------------------------------------------------------------------------------------
void Register_Early_Exit() noexcept {
    if (!*gbWess_early_exit) {
        *gbWess_early_exit = true;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Install the timing handler used by the sound system.
// This handler steps the sequencer periodically.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_install_handler() noexcept {
    init_WessTimer();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Removes the timing handler used by the sound system.
// Timing handling is restored to the system.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_restore_handler() noexcept {
    exit_WessTimer();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the WESS API and returns 'true' if an initialization was actually done
//------------------------------------------------------------------------------------------------------------------------------------------
bool wess_init() noexcept {
    // If we don't need to initialize then don't...
    if (*gbWess_sysinit)
        return false;

    // Ensure the sequencer is initially disabled
    *gbWess_SeqOn = false;

    // Install the timing handler/callback, init hardware specific stuff and mark the module as initialized
    if (!*gbWess_WessTimerActive) {
        wess_install_handler();
    }
    
    wess_low_level_init();
    *gbWess_sysinit = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the WESS API
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_exit(bool bForceRestoreTimerHandler) noexcept {
    // Must be initialized to shut down
    if ((!Is_System_Active()) || (!*gbWess_sysinit))
        return;

    // Unload the current module and do hardware specific shutdown
    if (*gbWess_module_loaded) {
        wess_unload_module();
    }

    wess_low_level_exit();

    // Mark the API as not initialized and restore the previous timer handler if appropriate
    *gbWess_sysinit = false;

    if (bForceRestoreTimerHandler || *gbWess_WessTimerActive) {
        wess_restore_handler();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return a pointer to the start of memory used by the loaded .WMD (Williams module) file and for loaded sequences
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t* wess_get_wmd_start() noexcept {
    return gpWess_wmd_mem->get();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return a pointer to the end of occupied memory used by the loaded .WMD (Williams module) file and for loaded sequences
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t* wess_get_wmd_end() noexcept {
    return gpWess_wmd_end->get();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frees all memory used by the currently loaded module, if the memory was allocated by the WESS API
//------------------------------------------------------------------------------------------------------------------------------------------
void free_mem_if_mine() noexcept {
    if (*gbWess_wmd_mem_is_mine) {
        if (gpWess_wmd_mem->get()) {
            wess_free(gpWess_wmd_mem->get());
            *gpWess_wmd_mem = nullptr;
        }

        *gbWess_wmd_mem_is_mine = false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// If a module is currently loaded, unloads it and frees any memory allocated during module load.
// This also shuts down the sequencer.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_unload_module() noexcept {
    // If nothing is loaded then there is nothing to do
    if (!*gbWess_module_loaded)
        return;

    // Shutdown the sequencer engine
    wess_seq_stopall();
    *gbWess_SeqOn = false;

    master_status_structure& mstat = *gpWess_pm_stat->get();
    gWess_CmdFuncArr[NoSound_ID][DriverExit](mstat);

    // Shutdown loaded sound drivers
    const int32_t numSndDrv = (*gpWess_pm_stat)->patch_types_loaded;

    for (int32_t drvIdx = 0; drvIdx < numSndDrv; ++drvIdx) {
        patch_group_data& patch_grp = mstat.ppat_info[drvIdx];

        const bool bWasDriverInitialized = (
            patch_grp.hw_tl_list.sfxload ||
            patch_grp.hw_tl_list.musload ||
            patch_grp.hw_tl_list.drmload
        );

        if (bWasDriverInitialized) {
            gWess_CmdFuncArr[patch_grp.hw_tl_list.hardware_ID][DriverExit](mstat);
        }
    }

    // Free any module memory allocated and mark unloaded
    free_mem_if_mine();
    *gbWess_module_loaded = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does what it says on the tin...
//------------------------------------------------------------------------------------------------------------------------------------------
static void wess_memcpy(void* const pDst, const void* const pSrc, const uint32_t numBytes) noexcept {
    const uint8_t* const pSrcBytes = (const uint8_t*) pSrc;
    uint8_t* const pDstBytes = (uint8_t*) pDst;

    for (uint32_t byteIdx = 0; byteIdx < numBytes; ++byteIdx) {
        pDstBytes[byteIdx] = pSrcBytes[byteIdx];
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads a certain number of bytes (or skips over it) from the currently open module file.
// After the read, the location of the destination pointer is 32-bit aligned.
// Returns 'false' if the read fails.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool conditional_read(const uint32_t readFlag, uint8_t*& pDstMemPtr, const int32_t readSize) noexcept {
    // Either skip over the memory block or read it
    if (readFlag) {
        wess_memcpy(pDstMemPtr, gpWess_curWmdFileBytes->get(), readSize);
        
        pDstMemPtr += readSize;
        pDstMemPtr += (uintptr_t) pDstMemPtr & 1;       // 32-bit align the pointer after the read...
        pDstMemPtr += (uintptr_t) pDstMemPtr & 2;       // 32-bit align the pointer after the read...
    }

    *gpWess_curWmdFileBytes += readSize;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads a .WMD (Williams module file) from the given buffer in RAM.
//
// Params:
//  (1) pWmdFile            : The module file to load from, which has been buffered into RAM.
//  (2) pDestMem            : The memory block to use for the loaded WMD file.
//  (3) memoryAllowance     : The size of the given memory block to use for the loaded WMD.
//  (4) pSettingTagLists    : Lists of key value pairs of settings for each sound driver.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_load_module(
    const void* const pWmdFile,
    void* const pDestMem,
    const int32_t memoryAllowance,
    const int32_t* const* const pSettingTagLists
) noexcept {
    // Save the maximum memory limit and unload the current module (if loaded)
    *gWess_mem_limit = memoryAllowance;
    
    if (*gbWess_module_loaded) {
        wess_unload_module();
    }

    // Figure out how many sound drivers are available
    const int32_t numSoundDrivers = get_num_Wess_Sound_Drivers(pSettingTagLists);
    *gWess_num_sd = numSoundDrivers;
    
    // Allocate the required amount of memory or just save what we were given (if given memory)
    if (pDestMem) {
        *gbWess_wmd_mem_is_mine = false;
        *gpWess_wmd_mem = ptrToVmAddr(pDestMem);
    } else {
        *gbWess_wmd_mem_is_mine = true;
        *gpWess_wmd_mem = ptrToVmAddr(wess_malloc(memoryAllowance));

        // If malloc failed then we can't load the module
        if (!*gpWess_wmd_mem) {
            return *gbWess_module_loaded;
        }
    }
    
    // Zero initialize the loaded module memory
    *gWess_wmd_size = memoryAllowance;
    zeroset(gpWess_wmd_mem->get(), memoryAllowance);

    // No sequences initially
    *gWess_end_seq_num = 0;
    
    // If the WESS API has not been initialized or an input WMD file is not supplied then we can't load the module
    if ((!Is_System_Active()) || (!pWmdFile)) {
        free_mem_if_mine();
        return *gbWess_module_loaded;
    }
   
    // Input file pointers
    *gpWess_curWmdFileBytes = ptrToVmAddr(pWmdFile);
    *gpWess_wmdFileBytesBeg = ptrToVmAddr(pWmdFile);

    // Destination pointer to allocate from in the loaded WMD memory block.
    // The code below allocates from this chunk linearly.
    uint8_t* pCurDestBytes = (uint8_t*) pDestMem;

    // Alloc the root master status structure
    master_status_structure& mstat = *(master_status_structure*) pCurDestBytes;
    pCurDestBytes += sizeof(master_status_structure);
    *gpWess_pm_stat = &mstat;

    // Master status: setting up various fields
    mstat.fp_module = *gpWess_fp_wmd_file;
    mstat.pabstime = gWess_Millicount.get();
    mstat.patch_types_loaded = (uint8_t) numSoundDrivers;

    // Alloc the module info struct and link to the master status struct
    module_data& mod_info = *(module_data*) pCurDestBytes;
    pCurDestBytes += sizeof(module_data);
    mstat.pmod_info = &mod_info;

    // Read the module header and verify it has the expected id and version.
    // If this operation fails then free any memory allocated (if owned):
    module_header& mod_hdr = mod_info.mod_hdr;
    wess_memcpy(&mod_hdr, gpWess_curWmdFileBytes->get(), sizeof(module_header));
    *gpWess_curWmdFileBytes += sizeof(module_header);

    if ((mod_hdr.module_id_text != WESS_MODULE_ID) || (mod_hdr.module_version != WESS_MODULE_VER)) {
        free_mem_if_mine();
        return false;
    }
    
    // Alloc the sequence status structs and link to the master status struct
    sequence_status* const pSeqStat = (sequence_status*) pCurDestBytes;
    mstat.pseqstattbl = pSeqStat;
    pCurDestBytes += sizeof(sequence_status) * mod_hdr.seq_work_areas;

    // Alloc the array of track status structs and link to the master status struct
    track_status* const pTrackStat = (track_status*) pCurDestBytes;
    mstat.ptrkstattbl = pTrackStat;
    pCurDestBytes += sizeof(track_status) * mod_hdr.trk_work_areas;

    // Alloc the list of master volumes for each sound driver and link to the master status struct.
    // Note that the pointer must be 32-bit aligned afterwords, as it might end up on an odd address:
    uint8_t* const pMasterVols = (uint8_t*) pCurDestBytes;
    mstat.pmaster_volume = pMasterVols;
    pCurDestBytes += sizeof(uint8_t) * numSoundDrivers;
    pCurDestBytes += (uintptr_t) pCurDestBytes & 1;         // Align to the next 32-bit boundary...
    pCurDestBytes += (uintptr_t) pCurDestBytes & 2;         // Align to the next 32-bit boundary...

    // Initialize master volumes for each sound driver
    for (int32_t drvIdx = 0; drvIdx < numSoundDrivers; ++drvIdx) {
        pMasterVols[drvIdx] = 128;
    }

    // Alloc the list of patch group info structs for each sound driver and link to the master status struct
    patch_group_data* const pPatchGrpInfo = (patch_group_data*) pCurDestBytes;
    pCurDestBytes += sizeof(patch_group_data) * numSoundDrivers;
    mstat.ppat_info = pPatchGrpInfo;

    // Load the settings for each sound driver (if given)
    if (pSettingTagLists) {
        for (int32_t drvIdx = 0; drvIdx < numSoundDrivers; ++drvIdx) {
            // Iterate through the key/value pairs of setting types and values for this driver
            for (int32_t kvpIdx = 0; pSettingTagLists[drvIdx][kvpIdx + 0] != SNDHW_TAG_END; kvpIdx += 2) {
                // Save the key value pair in the patch group info
                patch_group_data& patch_info = pPatchGrpInfo[drvIdx];
                patch_info.sndhw_tags[kvpIdx + 0] = pSettingTagLists[drvIdx][kvpIdx + 0];
                patch_info.sndhw_tags[kvpIdx + 1] = pSettingTagLists[drvIdx][kvpIdx + 1];

                // Process this key/value settings pair
                const int32_t key = patch_info.sndhw_tags[kvpIdx + 0];
                const int32_t value = patch_info.sndhw_tags[kvpIdx + 1];

                if (key == SNDHW_TAG_DRIVER_ID) {
                    patch_info.hw_tl_list.hardware_ID = (SoundDriverId) value;
                } else if (key == SNDHW_TAG_SOUND_EFFECTS) {
                    patch_info.hw_tl_list.sfxload |= (value & 1);
                } else if (key == SNDHW_TAG_MUSIC) {
                    patch_info.hw_tl_list.musload |= (value & 1);
                } else if (key == SNDHW_TAG_DRUMS) {
                    patch_info.hw_tl_list.drmload |= (value & 1);
                }
            }
        }
    }

    // Read patch group info for each sound driver and figure out the total number of voices for all patch groups
    mstat.voices_total = 0;

    for (int32_t patchIdx = 0; patchIdx < mod_info.mod_hdr.patch_types_infile; ++patchIdx) {
        // Read the patch group header.
        // Note: in the original code this data structure was a global, but I've made it a local here...
        patch_group_header patch_grp_hdr;
        wess_memcpy(&patch_grp_hdr, gpWess_curWmdFileBytes->get(), sizeof(patch_group_header));
        *gpWess_curWmdFileBytes += sizeof(patch_group_header);

        // Try to match against one of the sound drivers loaded
        for (int32_t drvIdx = 0; drvIdx < mstat.patch_types_loaded; ++drvIdx) {
            // This this patch group play with this sound hardware? If it doesn't then skip over it:
            patch_group_data& patch_grp = pPatchGrpInfo[drvIdx];

            if (patch_grp_hdr.patch_id != patch_grp.hw_tl_list.hardware_ID)
                continue;
            
            // Save the header, pointer to patch data and offset, and increment the total voice count
            patch_grp.pat_grp_hdr = patch_grp_hdr;            
            patch_grp.ppat_data = pCurDestBytes;
            patch_grp.data_fileposition = (int32_t)(gpWess_curWmdFileBytes->get() - gpWess_wmdFileBytesBeg->get());

            mstat.voices_total += patch_grp_hdr.hw_voice_limit;

            // Load the various types of patch group data
            {
                const bool bReadSuccess = conditional_read(
                    patch_grp_hdr.load_flags & LOAD_PATCHES,
                    pCurDestBytes,
                    (int32_t) patch_grp_hdr.patches * patch_grp_hdr.patch_size
                );

                if (!bReadSuccess)
                    return false;
            }

            {
                const bool bReadSuccess = conditional_read(
                    patch_grp_hdr.load_flags & LOAD_PATCHMAPS,
                    pCurDestBytes,
                    (int32_t) patch_grp_hdr.patchmaps * patch_grp_hdr.patchmap_size
                );

                if (!bReadSuccess)
                    return false;
            }
            
            {
                const bool bReadSuccess = conditional_read(
                    patch_grp_hdr.load_flags & LOAD_PATCHINFO,
                    pCurDestBytes,
                    (int32_t) patch_grp_hdr.patchinfo * patch_grp_hdr.patchinfo_size
                );
                
                if (!bReadSuccess)
                    return false;
            }
            
            {
                const bool bReadSuccess = conditional_read(
                    patch_grp_hdr.load_flags & LOAD_DRUMMAPS,
                    pCurDestBytes,
                    (int32_t) patch_grp_hdr.drummaps * patch_grp_hdr.drummap_size
                );
                
                if (!bReadSuccess)
                    return false;
            }

            {
                const bool bReadSuccess = conditional_read(
                    patch_grp_hdr.load_flags & LOAD_EXTRADATA,
                    pCurDestBytes,
                    patch_grp_hdr.extra_data_size
                );
                
                if (!bReadSuccess)
                    return false;
            }

            // Found the sound driver for this patch group, don't need to search the rest
            break;
        }
    }

    // Alloc the list of voice status structs and link to the master status struct
    voice_status* const pVoiceStat = (voice_status*) pCurDestBytes;
    mstat.pvoicestattbl = pVoiceStat;
    pCurDestBytes += sizeof(voice_status) * mstat.voices_total;

    // Assign hardware voices for each sound driver to the voice status structures allocated previously
    if (mstat.patch_types_loaded > 0) {
        int32_t patchGrpsLeft = mstat.patch_types_loaded;
        const patch_group_data* pPatchGrp = pPatchGrpInfo;

        int32_t hwVoicesLeft = pPatchGrp->pat_grp_hdr.hw_voice_limit;
        uint8_t hwVoiceIdx = 0;

        for (int32_t voiceIdx = 0; voiceIdx < mstat.voices_total;) {
            // Have we assigned voices for all drivers? If so then we are done...
            if (patchGrpsLeft <= 0)
                break;

            // Move onto the next sound driver if all voices for this driver have been assigned
            if (hwVoicesLeft <= 0) {
                if (patchGrpsLeft > 0) {
                    --patchGrpsLeft;
                    ++pPatchGrp;
                    hwVoicesLeft = pPatchGrp->pat_grp_hdr.hw_voice_limit;
                    hwVoiceIdx = 0;
                }
            }

            // If there are any hardware voices left to be assigned then assign them to this status struct
            voice_status& voice = mstat.pvoicestattbl[voiceIdx];

            if (hwVoicesLeft > 0) {
                --hwVoicesLeft;
                voice.patchtype = pPatchGrp->pat_grp_hdr.patch_id;
                voice.refindx = hwVoiceIdx;
                ++hwVoiceIdx;

                // PC-PSX: fix a small logic issue, which shouldn't be a problem in practice.
                // Move onto the next voice status struct *ONLY* if we actually assigned it to a hardware voice.
                // Previously, if a driver had '0' hardware voices then we might leak or leave unused one 'voice status' struct:
                #if PC_PSX_DOOM_MODS
                    ++voiceIdx;
                #endif
            }

            // PC-PSX: part of the fix mentioned above, only preform this increment conditionally now
            #if !PC_PSX_DOOM_MODS
                ++voiceIdx;
            #endif
        }
    }

    // Alloc the list of sequence info structs and link to the master status struct
    sequence_data* const pSeqInfo = (sequence_data*) pCurDestBytes;
    pCurDestBytes += sizeof(sequence_data) * mod_info.mod_hdr.sequences;
    mod_info.pseq_info = pSeqInfo;

    // These stats hold the maximums for all sequences
    uint8_t maxSeqTracks = MINIMUM_TRACK_INDXS_FOR_A_SEQUENCE;
    uint8_t maxSeqVoices = 0;
    uint8_t maxSeqSubstackCount = 0;
    
    // Determine track stats and sequence headers for all sequences
    for (int32_t seqIdx = 0; seqIdx < mod_info.mod_hdr.sequences; ++seqIdx) {
        // Read the sequence header, save the sequence position in the file and move past it
        sequence_data& seq_info = mod_info.pseq_info[seqIdx];
        wess_memcpy(&seq_info.seq_hdr, gpWess_curWmdFileBytes->get(), sizeof(seq_header));

        seq_info.fileposition = (uint32_t)(gpWess_curWmdFileBytes->get() - gpWess_wmdFileBytesBeg->get());
        *gpWess_curWmdFileBytes += sizeof(seq_header);

        // Run through all tracks in the sequence and figure out the stats (size etc.) for what will be loaded
        uint8_t numTracksToload = 0;
        uint32_t tracksTotalSize = 0;

        for (int32_t trackIdx = 0; trackIdx < seq_info.seq_hdr.tracks; ++trackIdx) {
            // Read the track header and move on in the file.
            // Note: in the original code this data structure was a global, but I've made it a local here...
            track_header track_hdr;
            wess_memcpy(&track_hdr, gpWess_curWmdFileBytes->get(), sizeof(track_header));
            *gpWess_curWmdFileBytes += sizeof(track_header);

            // Decide whether the track is to be loaded for this sound driver
            bool bLoadTrack = false;

            if ((track_hdr.voices_type == NoSound_ID) || (track_hdr.voices_type == GENERIC_ID)) {
                // This track is not associated with any sound driver or works with any sound driver: load always
                bLoadTrack = true;
            } 
            else {
                // Not doing an unconditional load of this track.
                // Only load it if it is for one of the loaded sound drivers and loading this track type is allowed:
                for (int32_t drvIdx = 0; drvIdx < mstat.patch_types_loaded; ++drvIdx) {
                    patch_group_data& patch_grp = pPatchGrpInfo[drvIdx];

                    // Is the track for this sound driver?
                    if (track_hdr.voices_type != patch_grp.hw_tl_list.hardware_ID)
                        continue;

                    // Only load the track if it's a known voice class and the driver wants to load that voice class
                    if ((track_hdr.voices_class == SNDFX_CLASS) || (track_hdr.voices_class == SFXDRUMS_CLASS)) {
                        if (patch_grp.hw_tl_list.sfxload) {
                            bLoadTrack = true;
                            break;
                        }
                    }

                    if (track_hdr.voices_class == MUSIC_CLASS) {
                        if (patch_grp.hw_tl_list.musload) {
                            bLoadTrack = true;
                            break;
                        }
                    }

                    if (track_hdr.voices_class == DRUMS_CLASS) {
                        if (patch_grp.hw_tl_list.drmload) {
                            bLoadTrack = true;
                            break;
                        }
                    }
                }
            }

            // If the track is to be loaded figure out how much memory it would use and add to the total for the sequence.
            // Also update the maximum number of voices required for all sequences.
            if (bLoadTrack) {
                ++numTracksToload;

                // Track is to be loaded: incorporate this track's size into the total size for the sequence and 32-bit align size
                tracksTotalSize += sizeof(track_data);
                tracksTotalSize += (uint32_t) track_hdr.labellist_count * sizeof(uint32_t);
                tracksTotalSize += track_hdr.data_size;
                tracksTotalSize += tracksTotalSize & 1;     // Added size due to 32-bit align..
                tracksTotalSize += tracksTotalSize & 2;     // Added size due to 32-bit align..

                // Incorporate track stats into the maximum for all sequences
                if (track_hdr.voices_max > maxSeqVoices) {
                    maxSeqVoices = track_hdr.voices_max;
                }

                if (track_hdr.substack_count > maxSeqSubstackCount) {
                    maxSeqSubstackCount = track_hdr.substack_count;
                }
            }

            // Move past this track in the WMD file
            *gpWess_curWmdFileBytes += (uint32_t) track_hdr.labellist_count * sizeof(uint32_t);
            *gpWess_curWmdFileBytes += track_hdr.data_size;
        }
        
        // Incorporate track count into the global max
        if (numTracksToload > maxSeqTracks) {
            maxSeqTracks = numTracksToload;
        }

        // If no tracks are in the sequence then we still allocate a small amount of track info.
        // This is for the 'dummy' track that gets created when the sequence is loaded.
        if (numTracksToload == 0) {
            tracksTotalSize = sizeof(track_data) + 4;   // +2 bytes for dummy track commands, +2 bytes to align afterwards...
        }

        // Save sequence stats
        seq_info.trkstoload = numTracksToload;
        seq_info.trkinfolength = tracksTotalSize;
    }

    // Save the sequence & track limits figured out in the loop above
    mstat.max_trks_perseq = maxSeqTracks;
    mstat.max_voices_pertrk = maxSeqVoices;
    mstat.max_substack_pertrk = maxSeqSubstackCount;

    // Alloc the list of callback status structs and link to the master status struct
    callback_status* const pCallbackStat = (callback_status*) pCurDestBytes;
    pCurDestBytes += sizeof(callback_status) * mod_info.mod_hdr.callback_areas;
    mstat.pcalltable = pCallbackStat;

    // Allocate the sequence work areas
    for (int32_t seqIdx = 0; seqIdx < mod_info.mod_hdr.seq_work_areas; ++seqIdx) {
        sequence_status& seq_stat = mstat.pseqstattbl[seqIdx];

        // Allocate the gates array for this sequence and 32-bit align afterwards        
        seq_stat.pgates = (uint8_t*) pCurDestBytes;

        pCurDestBytes += sizeof(uint8_t) * mod_info.mod_hdr.gates_per_seq;
        pCurDestBytes += (uintptr_t) pCurDestBytes & 1;     // Added size due to 32-bit align..
        pCurDestBytes += (uintptr_t) pCurDestBytes & 2;     // Added size due to 32-bit align..

        // Alloc the iters array for this sequence and 32-bit align afterwards
        seq_stat.piters = (uint8_t*) pCurDestBytes;

        pCurDestBytes += sizeof(uint8_t) * mod_info.mod_hdr.iters_per_seq;
        pCurDestBytes += (uintptr_t) pCurDestBytes & 1;     // Added size due to 32-bit align..
        pCurDestBytes += (uintptr_t) pCurDestBytes & 2;     // Added size due to 32-bit align..

        // Alloc the track indexes array for this sequence and 32-bit align afterwards
        seq_stat.ptrk_indxs = (uint8_t*) pCurDestBytes;

        pCurDestBytes += sizeof(uint8_t) * maxSeqTracks;
        pCurDestBytes += (uintptr_t) pCurDestBytes & 1;     // Added size due to 32-bit align..
        pCurDestBytes += (uintptr_t) pCurDestBytes & 2;     // Added size due to 32-bit align..

        // Initialize the track indexes array for the sequence
        for (int32_t trackIdx = 0; trackIdx < maxSeqTracks; ++trackIdx) {
            seq_stat.ptrk_indxs[trackIdx] = 0xFF;
        }
    }

    // Allocate the sub-stacks for each track work area.
    // These are used to hold a stack track locations that can be pushed and popped to by sequencer commands for save/return behavior.
    for (uint8_t trackIdx = 0; trackIdx < mod_info.mod_hdr.trk_work_areas; ++trackIdx) {
        track_status& track_stat = pTrackStat[trackIdx];

        track_stat.refindx = trackIdx;
        track_stat.psubstack = (uint32_t*) pCurDestBytes;

        pCurDestBytes += sizeof(VmPtr<void>) * mstat.max_substack_pertrk;
        track_stat.pstackend = (uint32_t*) pCurDestBytes;
    }

    // Initialize the sequencer
    gWess_CmdFuncArr[NoSound_ID][DriverInit](mstat);
    
    // Initialize loaded drivers
    for (int32_t drvIdx = 0; drvIdx < mstat.patch_types_loaded; ++ drvIdx) {
        patch_group_data& patch_grp = pPatchGrpInfo[drvIdx];
        
        // Initialize the driver if we are loading any voices and tracks relating to it
        const bool bInitDriver = (
            patch_grp.hw_tl_list.sfxload ||
            patch_grp.hw_tl_list.musload ||
            patch_grp.hw_tl_list.drmload
        );

        if (bInitDriver) {
            // Initialize the driver
            gWess_CmdFuncArr[patch_grp.hw_tl_list.hardware_ID][DriverInit](mstat);
        }
    }

    // The module is now loaded and the sequencer is enabled
    *gbWess_module_loaded = true;
    *gbWess_SeqOn = true;

    // Save the end pointer for the loaded module and ensure 32-bit aligned
    pCurDestBytes += (uintptr_t) pCurDestBytes & 1;
    pCurDestBytes += (uintptr_t) pCurDestBytes & 2;
    *gpWess_wmd_end = pCurDestBytes;

    // This is maximum sequence number that can be triggered
    *gWess_end_seq_num = mod_info.mod_hdr.sequences;

    // PC-PSX: sanity check we haven't overflowed module memory
    #if PC_PSX_DOOM_MODS
        ASSERT(gpWess_wmd_end->get() - gpWess_wmd_mem->get() <= *gWess_mem_limit);
    #endif
    
    // Load was a success!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs initialization of most of the track status struct, given the specified track info and optional play attributes
//------------------------------------------------------------------------------------------------------------------------------------------
void filltrackstat(track_status& trackStat, const track_data& trackInfo, const TriggerPlayAttr* const pAttribs) noexcept {
    // Basic track status initialization
    trackStat.active = true;
    trackStat.timed = false;
    trackStat.looped = false;
    trackStat.skip = false;
    trackStat.off = false;

    trackStat.patchtype = trackInfo.trk_hdr.voices_type;
    trackStat.priority = trackInfo.trk_hdr.priority;
    trackStat.sndclass = trackInfo.trk_hdr.voices_class;
    trackStat.voices_active = 0;
    trackStat.voices_max = trackInfo.trk_hdr.voices_max;
    trackStat.ppq = trackInfo.trk_hdr.initppq;
    trackStat.starppi = 0;
    trackStat.accppi = 0;
    trackStat.totppi = 0;
    trackStat.psp = trackStat.psubstack;
    trackStat.labellist_count = trackInfo.trk_hdr.labellist_count;
    trackStat.data_size = trackInfo.trk_hdr.data_size;
    trackStat.mutemask = trackInfo.trk_hdr.mutebits;

    // If no play attributes are given then the attribute mask to customize is simply zero
    const uint32_t attribsMask = (pAttribs) ? pAttribs->mask : 0;

    // Set all of the track attributes that can be customized, using any customizations given
    if (attribsMask & TRIGGER_VOLUME) {
        trackStat.volume_cntrl = pAttribs->volume;
    } else {
        trackStat.volume_cntrl = trackInfo.trk_hdr.initvolume_cntrl;
    }
    
    if (attribsMask & TRIGGER_PAN) {
        trackStat.pan_cntrl = pAttribs->pan;
    } else {
        trackStat.pan_cntrl = trackInfo.trk_hdr.initpan_cntrl;
    }
    
    if (attribsMask & TRIGGER_PATCH) {
        trackStat.patchnum = pAttribs->patch;
    } else {
        trackStat.patchnum = trackInfo.trk_hdr.initpatchnum;
    }
    
    if (attribsMask & TRIGGER_PITCH) {
        trackStat.pitch_cntrl = pAttribs->pitch;
    } else {
        trackStat.pitch_cntrl = trackInfo.trk_hdr.initpitch_cntrl;
    }
    
    if ((attribsMask & TRIGGER_MUTEMODE) && (trackStat.mutemask & (1 << pAttribs->mutemode))) {
        trackStat.mute = true;
    } else {
        trackStat.mute = false;
    }
    
    if (attribsMask & TRIGGER_TEMPO) {
        trackStat.qpm = pAttribs->tempo;
    } else {
        trackStat.qpm = trackInfo.trk_hdr.initqpm;
    }

    const int16_t interruptsPerSec = GetIntsPerSec();
    trackStat.ppi = CalcPartsPerInt(interruptsPerSec, trackStat.ppq, trackStat.qpm);    // Compute how much to advance the track's timing for each hardware timer interrupt

    if (attribsMask & TRIGGER_TIMED) {
        trackStat.timed = true;
        trackStat.endppi = pAttribs->timeppq;
    } else {
        trackStat.timed = false;
    }

    if (attribsMask & TRIGGER_LOOPED) {
        trackStat.looped = true;   
    } else {
        trackStat.looped = false;
    }
    
    if (attribsMask & TRIGGER_REVERB) {
        trackStat.reverb = pAttribs->reverb;
    } else {
        trackStat.reverb = trackInfo.trk_hdr.reverb;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up some basic position and count related info for the given track status using the track info.
// Determines the time until the 1st sequencer command also.
//------------------------------------------------------------------------------------------------------------------------------------------
void assigntrackstat(track_status& trackStat, const track_data& trackInfo) noexcept {
    trackStat.data_space = trackInfo.trk_hdr.data_size;
    trackStat.labellist_max = trackInfo.trk_hdr.labellist_count;
    trackStat.pstart = trackInfo.ptrk_data.get();
    trackStat.ppos = Read_Vlq(trackInfo.ptrk_data.get(), trackStat.deltatime);
    trackStat.plabellist = trackInfo.plabellist;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocates and sets up sequence and track status structs for the sequence to be played.
// The given play attributes etc. can be used to customize sequence playback.
// On success the index of the sequence allocated (+1) is returned, otherwise '0' is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_seq_structrig(
    const sequence_data& seqInfo,
    const int32_t seqIdx,
    const uint32_t seqType,
    const bool bGetHandle,
    const TriggerPlayAttr* pPlayAttribs
) noexcept {
    // Can't play the sequence if the number is not valid
    if (!Is_Seq_Num_Valid(seqIdx))
        return 0;

    // Disable sequencer ticking temporarily (to avoid hardware timer interrupts) while we setup all this
    *gbWess_SeqOn = false;

    master_status_structure& mstat = *gpWess_pm_stat->get();
    module_data& modInfo = *mstat.pmod_info;

    const uint8_t maxSeqs = modInfo.mod_hdr.seq_work_areas;
    const uint8_t maxTracks = modInfo.mod_hdr.trk_work_areas;

    // Find a free sequence status structure that we can use to play this sequence
    uint8_t allocSeqIdx = 0;

    while ((allocSeqIdx < maxSeqs) && (mstat.pseqstattbl[allocSeqIdx].active)) {
        ++allocSeqIdx;
    }

    // If we failed to allocate a sequence status structure then we can't play anything
    if (allocSeqIdx >= maxSeqs) {
        *gbWess_SeqOn = true;
        return 0;
    }

    // Try to allocate free tracks for all of the tracks that the sequence needs to play.
    // Note that this loop assumes each sequence will always have at least 1 track to play.
    sequence_status& seqStat = mstat.pseqstattbl[allocSeqIdx];
    uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

    uint8_t numTracksPlayed = 0;
    uint8_t numTracksToPlay = (uint8_t) seqInfo.seq_hdr.tracks;

    for (uint8_t trackIdx = 0; trackIdx < maxTracks; ++trackIdx) {
        // Ensure this track is not in use before we do anything
        track_status& trackStat = mstat.ptrkstattbl[trackIdx];

        if (trackStat.active)
            continue;

        // Great! We can use this track for one of the sequence tracks.
        // Set it's owner and start filling in the track status:
        trackStat.seq_owner = allocSeqIdx;

        track_data& trackInfo = seqInfo.ptrk_info[numTracksPlayed];
        filltrackstat(trackStat, trackInfo, pPlayAttribs);
        assigntrackstat(trackStat, trackInfo);

        if (bGetHandle) {
            trackStat.handled = true;
            trackStat.stopped = true;
        } else {
            trackStat.handled = false;
            trackStat.stopped = false;
            seqStat.tracks_playing++;
        }

        // There is one more track playing the sequence and globally
        seqStat.tracks_active += 1;
        mstat.trks_active += 1;

        // Remember what track is being used on the sequence
        pSeqTrackIndexes[numTracksPlayed] = trackIdx;
        numTracksPlayed++;

        // If we have allocated all the tracks in the sequence then we are done
        if (numTracksPlayed >= numTracksToPlay)
            break;
    }

    // Setup various sequence bits if we actually started playing some tracks
    if (numTracksPlayed > 0) {
        seqStat.seq_idx = (int16_t) seqIdx;
        seqStat.seq_type = seqType;

        if (bGetHandle) {
            seqStat.handle = true;
            seqStat.playmode = SEQ_STATE_STOPPED;
        } else {
            seqStat.handle = false;
            seqStat.playmode = SEQ_STATE_PLAYING;
        }

        seqStat.volume = 128;
        seqStat.pan = 64;
        seqStat.active = true;
        mstat.seqs_active += 1;
    }

    // Renable the sequencer's timer interrupts/updates and return what sequence index we started playing (if any)
    *gbWess_SeqOn = true;

    if (numTracksPlayed > 0) {
        return (int32_t) allocSeqIdx + 1;
    } else {
        return 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger the specified sequence number and assign it the type number '0'
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_trigger(const int32_t seqIdx) noexcept {
    wess_seq_trigger_type(seqIdx, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger the specified sequence number with the specified custom play attributes.
// The sequence is assigned the type number '0'.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_trigger_special(const int32_t seqIdx, const TriggerPlayAttr* const pPlayAttribs) noexcept {
    master_status_structure& mstat = *gpWess_pm_stat->get();
    wess_seq_structrig(mstat.pmod_info->pseq_info[seqIdx], seqIdx, 0, false, pPlayAttribs);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the current state of the specified sequence
//------------------------------------------------------------------------------------------------------------------------------------------
SequenceStatus wess_seq_status(const int32_t seqIdx) noexcept {
    // Emulate sound a little in case calling code is polling in a loop waiting for status to change
    #if PC_PSX_DOOM_MODS
        emulate_sound_if_required();
    #endif

    // Is the sequence number a valid one?
    if (!Is_Seq_Num_Valid(seqIdx))
        return SEQUENCE_INVALID;
    
    // Try to find the specified sequence number among all the sequences
    master_status_structure& mstat = *gpWess_pm_stat->get();
    const int32_t maxSeqs = mstat.pmod_info->mod_hdr.seq_work_areas;

    for (uint8_t statIdx = 0; statIdx < maxSeqs; ++statIdx) {
        sequence_status& seqStat = mstat.pseqstattbl[statIdx];

        // Make sure this sequence is in use and that it's the one we want
        if ((!seqStat.active) || (seqStat.seq_idx != seqIdx))
            continue;

        // Determine status from play mode
        if (seqStat.playmode == SEQ_STATE_STOPPED) {
            return SEQUENCE_STOPPED;
        } else if (seqStat.playmode == SEQ_STATE_PLAYING) {
            return SEQUENCE_PLAYING;
        } else {
            return SEQUENCE_INACTIVE;   // Invalid/unknown play mode!
        }
    }
    
    // If the sequence number was not found assume not playing and not paused
    return SEQUENCE_INACTIVE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stops the specified sequence number
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_stop(const int32_t seqIdx) noexcept {
    // Don't bother if the sequence number is not valid
    if (!Is_Seq_Num_Valid(seqIdx))
        return;

    // Temporarily disable the sequencer while we do this.
    // It was originally fired by hardware timer interrupts, so this step was required.
    *gbWess_SeqOn = false;
    
    // Run through all of the sequences searching for the one we are interested in
    master_status_structure& mstat = *gpWess_pm_stat->get();

    const uint8_t maxSeqs = mstat.pmod_info->mod_hdr.seq_work_areas;
    const uint32_t maxTracksPerSeq = mstat.max_trks_perseq;
    uint8_t numActiveSeqsToVisit = mstat.seqs_active;

    for (uint8_t statIdx = 0; statIdx < maxSeqs; ++statIdx) {
        // If there are no more active sequences to visit then we are done
        if (numActiveSeqsToVisit == 0)
            break;

        // Only bother if the sequence is loaded/active
        sequence_status& seqStat = mstat.pseqstattbl[statIdx];

        if (!seqStat.active)
            continue;
        
        if (seqStat.seq_idx == seqIdx) {
            // This is the sequence we want, go through all the tracks and stop each one
            uint32_t numSeqTracksActive = seqStat.tracks_active;
            uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

            for (uint32_t i = 0; i < maxTracksPerSeq; ++i) {
                // Is this track index valid and in use?
                const uint8_t trackIdx = pSeqTrackIndexes[i];

                if (trackIdx == 0xFF)
                    continue;
                
                // Call the driver function to turn off the track
                track_status& trackStat = mstat.ptrkstattbl[trackIdx];
                gWess_CmdFuncArr[trackStat.patchtype][TrkOff](trackStat);

                // If there are no more tracks left active then we are done
                --numSeqTracksActive;

                if (numSeqTracksActive == 0)
                    break;
            }
        }

        --numActiveSeqsToVisit;
    }

    // Re-enable the sequencer
    *gbWess_SeqOn = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stops all active sound sequences
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_stopall() noexcept {
    // Don't bother if there is no module loaded
    if (!Is_Module_Loaded())
        return;

    // Temporarily disable the sequencer while we do this.
    // It was originally fired by hardware timer interrupts, so this step was required.
    *gbWess_SeqOn = false;

    // Grab some basic info from the master status
    master_status_structure& mstat = *gpWess_pm_stat->get();

    const uint8_t maxSeqs = mstat.pmod_info->mod_hdr.seq_work_areas;
    const uint32_t maxTracksPerSeq = mstat.max_trks_perseq;
    uint8_t numActiveSeqsToVisit = mstat.seqs_active;

    if (numActiveSeqsToVisit > 0) {
        // Run through all of the active sequences and stop them all
        for (uint8_t seqIdx = 0; seqIdx < maxSeqs; ++seqIdx) {
            sequence_status& seqStat = mstat.pseqstattbl[seqIdx];

            if (!seqStat.active)
                continue;

            // Run through all of the active tracks in the sequence and stop them all
            uint32_t numSeqTracksActive = seqStat.tracks_active;
            uint8_t* const pSeqTrackIndexes = seqStat.ptrk_indxs.get();

            for (uint32_t i = 0; i < maxTracksPerSeq; ++i) {
                // Is this sequence track slot actually in use? Skip if not:
                const uint8_t trackIdx = pSeqTrackIndexes[i];

                if (trackIdx == 0xFF)
                    continue;
                
                // Call the driver function to turn off the track
                track_status& trackStat = mstat.ptrkstattbl[trackIdx];
                gWess_CmdFuncArr[trackStat.patchtype][TrkOff](trackStat);

                // If there are no more tracks left active then we are done
                numSeqTracksActive--;

                if (numSeqTracksActive == 0)
                    break;
            }

            // If there are no more active sequences to visit then we are done
            numActiveSeqsToVisit--;

            if (numActiveSeqsToVisit == 0)
                break;
        }
    }

    // Re-enable the sequencer
    *gbWess_SeqOn = true;
}
