//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): mostly core API functions, including module (.WMD) file loading
//------------------------------------------------------------------------------------------------------------------------------------------
#include "wessapi.h"

#include "Asserts.h"
#include "Endian.h"
#include "wessapi_t.h"
#include "wessseq.h"

static constexpr uint32_t WESS_MODULE_ID = Endian::littleToHost(0x58535053);        // 4 byte identifier for WMD (Williams Module) files: says 'SPSX'
static constexpr uint32_t WESS_MODULE_VER = 1;                                      // Expected WMD (module) file version
static constexpr uint8_t MINIMUM_TRACK_INDXS_FOR_A_SEQUENCE = 4;                    // Minimum tracks in a sequence

bool                        gbWess_module_loaded;       // If true then a WMD file (module) has been loaded
master_status_structure*    gpWess_pm_stat;             // Master status structure for the entire sequencer system

static bool             gbWess_sysinit;             // Set to true once the WESS API has been initialized
static bool             gbWess_early_exit;          // Unused flag in PSX DOOM, I think to request the API to exit?
static int32_t          gWess_num_snd_drv;          // The number of sound drivers available
static uint8_t*         gpWess_wmd_mem;             // The memory block that the module and all sequences are loaded into. Contains master status struct and it's children.
static bool             gbWess_wmd_mem_is_mine;     // If true then the WESS API allocated the WMD memory block and thus is responsible for its cleanup
static uint8_t*         gpWess_wmd_end;             // Current end of the used portion of the WMD memory block
static int32_t          gWess_wmd_size;             // Size of the WMD memory block allocated, or upper limit on sequencer memory use
static int32_t          gWess_mem_limit;            // Size of the WMD memory block that the WESS API was INTENDING to allocate (alloc may have failed)
static int32_t          gWess_end_seq_num;          // Maximum sequence number (exclusive) in the loaded module file
static PsxCd_File*      gpWess_fp_wmd_file;         // File object for the module (.WMD) file on-disc
static const uint8_t*   gpWess_wmdFileBytesBeg;     // Pointer to the start of a buffer containing the serialized .WMD file, as it was read from the disc
static const uint8_t*   gpWess_curWmdFileBytes;     // Location in the buffer containing the serialized .WMD file to read from next

// Unused error handling stuff: may have only been used in debug builds originally
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
    return gpWess_pm_stat;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the WESS API has been initialized
//------------------------------------------------------------------------------------------------------------------------------------------
bool Is_System_Active() noexcept {
    return gbWess_sysinit;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a module file (.WMD) file has been loaded
//------------------------------------------------------------------------------------------------------------------------------------------
bool Is_Module_Loaded() noexcept {
    return gbWess_module_loaded;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given sequence index is valid and loaded
//------------------------------------------------------------------------------------------------------------------------------------------
bool Is_Seq_Num_Valid(const int32_t seqIdx) noexcept {
    if ((seqIdx >= 0) && (seqIdx < gWess_end_seq_num)) {
        return (gpWess_pm_stat->pmodule->psequences[seqIdx].ptracks != nullptr);
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Not sure what this function is for, but it appears unused in the retail game.
// I think it is to request the WESS API to finish up early, possibly a leftover from PC code?
//------------------------------------------------------------------------------------------------------------------------------------------
void Register_Early_Exit() noexcept {
    if (!gbWess_early_exit) {
        gbWess_early_exit = true;
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
    if (gbWess_sysinit)
        return false;

    // Ensure the sequencer is initially disabled
    gbWess_SeqOn = false;

    // Install the timing handler/callback, init hardware specific stuff and mark the module as initialized
    if (!gbWess_WessTimerActive) {
        wess_install_handler();
    }
    
    wess_low_level_init();
    gbWess_sysinit = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the WESS API
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_exit(bool bForceRestoreTimerHandler) noexcept {
    // Must be initialized to shut down
    if ((!Is_System_Active()) || (!gbWess_sysinit))
        return;

    // Unload the current module and do hardware specific shutdown
    if (gbWess_module_loaded) {
        wess_unload_module();
    }

    wess_low_level_exit();

    // Mark the API as not initialized and restore the previous timer handler if appropriate
    gbWess_sysinit = false;

    if (bForceRestoreTimerHandler || gbWess_WessTimerActive) {
        wess_restore_handler();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return a pointer to the start of memory used by the loaded .WMD (Williams module) file and for loaded sequences
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t* wess_get_wmd_start() noexcept {
    return gpWess_wmd_mem;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return a pointer to the end of occupied memory used by the loaded .WMD (Williams module) file and for loaded sequences
//------------------------------------------------------------------------------------------------------------------------------------------
uint8_t* wess_get_wmd_end() noexcept {
    return gpWess_wmd_end;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Frees all memory used by the currently loaded module, if the memory was allocated by the WESS API
//------------------------------------------------------------------------------------------------------------------------------------------
void free_mem_if_mine() noexcept {
    if (gbWess_wmd_mem_is_mine) {
        if (gpWess_wmd_mem) {
            wess_free(gpWess_wmd_mem);
            gpWess_wmd_mem = nullptr;
        }

        gbWess_wmd_mem_is_mine = false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// If a module is currently loaded, unloads it and frees any memory allocated during module load.
// This also shuts down the sequencer.
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_unload_module() noexcept {
    // If nothing is loaded then there is nothing to do
    if (!gbWess_module_loaded)
        return;

    // Shutdown the sequencer engine
    wess_seq_stopall();
    gbWess_SeqOn = false;

    master_status_structure& mstat = *gpWess_pm_stat;
    gWess_CmdFuncArr[NoSound_ID][DriverExit](mstat);

    // Shutdown loaded sound drivers for each patch group
    const int32_t numPatchGroups = mstat.num_patch_groups;

    for (int32_t patchGroupIdx = 0; patchGroupIdx < numPatchGroups; ++patchGroupIdx) {
        patch_group_data& patchGroup = mstat.ppatch_groups[patchGroupIdx];

        const bool bWasDriverInitialized = (
            patchGroup.hw_table_list.sfxload ||
            patchGroup.hw_table_list.musload ||
            patchGroup.hw_table_list.drmload
        );

        if (bWasDriverInitialized) {
            gWess_CmdFuncArr[patchGroup.hw_table_list.driver_id][DriverExit](mstat);
        }
    }

    // Free any module memory allocated and mark unloaded
    free_mem_if_mine();
    gbWess_module_loaded = false;
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
        wess_memcpy(pDstMemPtr, gpWess_curWmdFileBytes, readSize);
        
        pDstMemPtr += readSize;
        pDstMemPtr += (uintptr_t) pDstMemPtr & 1;       // 32-bit align the pointer after the read...
        pDstMemPtr += (uintptr_t) pDstMemPtr & 2;       // 32-bit align the pointer after the read...
    }

    gpWess_curWmdFileBytes += readSize;
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
    gWess_mem_limit = memoryAllowance;
    
    if (gbWess_module_loaded) {
        wess_unload_module();
    }

    // Figure out how many sound drivers are available (and thus how many patch groups to allocate)
    const int32_t numPatchGroups = get_num_Wess_Sound_Drivers(pSettingTagLists);
    gWess_num_snd_drv = numPatchGroups;
    
    // Allocate the required amount of memory or just save what we were given (if given memory)
    if (pDestMem) {
        gbWess_wmd_mem_is_mine = false;
        gpWess_wmd_mem = (uint8_t*) pDestMem;
    } else {
        gbWess_wmd_mem_is_mine = true;
        gpWess_wmd_mem = (uint8_t*) wess_malloc(memoryAllowance);

        // If malloc failed then we can't load the module
        if (!gpWess_wmd_mem) {
            return gbWess_module_loaded;
        }
    }
    
    // Zero initialize the loaded module memory
    gWess_wmd_size = memoryAllowance;
    zeroset(gpWess_wmd_mem, memoryAllowance);

    // No sequences initially
    gWess_end_seq_num = 0;
    
    // If the WESS API has not been initialized or an input WMD file is not supplied then we can't load the module
    if ((!Is_System_Active()) || (!pWmdFile)) {
        free_mem_if_mine();
        return gbWess_module_loaded;
    }
   
    // Input file pointers
    gpWess_curWmdFileBytes = (const uint8_t*) pWmdFile;
    gpWess_wmdFileBytesBeg = (const uint8_t*) pWmdFile;

    // Destination pointer to allocate from in the loaded WMD memory block.
    // The code below allocates from this chunk linearly.
    //
    // PsyDoom: added alignment code to align the destination pointer to prevent undefined behavior for various types.
    // This may increase memory usage in the WMD.
    uint8_t* pCurDestBytes = (uint8_t*) pDestMem;
    
    // Alloc the root master status structure
    wess_align_byte_ptr(pCurDestBytes, alignof(master_status_structure));
    master_status_structure& mstat = *(master_status_structure*) pCurDestBytes;
    pCurDestBytes += sizeof(master_status_structure);
    gpWess_pm_stat = &mstat;

    // Master status: setting up various fields
    mstat.pmodule_file = gpWess_fp_wmd_file;
    mstat.pabstime_ms = &gWess_Millicount;
    mstat.num_patch_groups = (uint8_t) numPatchGroups;

    // Alloc the module info struct and link to the master status struct
    wess_align_byte_ptr(pCurDestBytes, alignof(module_data));
    module_data& module = *(module_data*) pCurDestBytes;
    pCurDestBytes += sizeof(module_data);
    mstat.pmodule = &module;

    // Read the module header and verify it has the expected id and version.
    // If this operation fails then free any memory allocated (if owned):
    module_header& moduleHdr = module.hdr;
    wess_memcpy(&moduleHdr, gpWess_curWmdFileBytes, sizeof(module_header));
    gpWess_curWmdFileBytes += sizeof(module_header);

    if ((moduleHdr.module_id != WESS_MODULE_ID) || (moduleHdr.module_version != WESS_MODULE_VER)) {
        free_mem_if_mine();
        return false;
    }
    
    // Alloc the sequence status structs and link to the master status struct
    wess_align_byte_ptr(pCurDestBytes, alignof(sequence_status));
    sequence_status* const pSeqStat = (sequence_status*) pCurDestBytes;
    mstat.psequence_stats = pSeqStat;
    pCurDestBytes += sizeof(sequence_status) * moduleHdr.max_active_sequences;

    // Alloc the array of track status structs and link to the master status struct
    wess_align_byte_ptr(pCurDestBytes, alignof(track_status));
    track_status* const pTrackStat = (track_status*) pCurDestBytes;
    mstat.ptrack_stats = pTrackStat;
    pCurDestBytes += sizeof(track_status) * moduleHdr.max_active_tracks;

    // Alloc the list of master volumes for each sound driver and link to the master status struct.
    // Note that the pointer must be 32-bit aligned afterwords, as it might end up on an odd address:
    uint8_t* const pMasterVols = (uint8_t*) pCurDestBytes;
    mstat.pmaster_vols = pMasterVols;
    pCurDestBytes += sizeof(uint8_t) * numPatchGroups;
    pCurDestBytes += (uintptr_t) pCurDestBytes & 1;         // Align to the next 32-bit boundary...
    pCurDestBytes += (uintptr_t) pCurDestBytes & 2;         // Align to the next 32-bit boundary...

    // Initialize master volumes for each sound driver
    for (int32_t patchGroupIdx = 0; patchGroupIdx < numPatchGroups; ++patchGroupIdx) {
        pMasterVols[patchGroupIdx] = 128;
    }

    // Alloc the list of patch group info structs for each sound driver and link to the master status struct
    wess_align_byte_ptr(pCurDestBytes, alignof(patch_group_data));
    patch_group_data* const pPatchGroups = (patch_group_data*) pCurDestBytes;
    pCurDestBytes += sizeof(patch_group_data) * numPatchGroups;
    mstat.ppatch_groups = pPatchGroups;

    // Load the settings for each sound driver (if given)
    if (pSettingTagLists) {
        for (int32_t patchGroupIdx = 0; patchGroupIdx < numPatchGroups; ++patchGroupIdx) {
            // Iterate through the key/value pairs of setting types and values for this driver
            for (int32_t kvpIdx = 0; pSettingTagLists[patchGroupIdx][kvpIdx + 0] != SNDHW_TAG_END; kvpIdx += 2) {
                // Save the key value pair in the patch group info
                patch_group_data& patchGroup = pPatchGroups[patchGroupIdx];
                patchGroup.sndhw_tags[kvpIdx + 0] = pSettingTagLists[patchGroupIdx][kvpIdx + 0];
                patchGroup.sndhw_tags[kvpIdx + 1] = pSettingTagLists[patchGroupIdx][kvpIdx + 1];

                // Process this key/value settings pair
                const int32_t key = patchGroup.sndhw_tags[kvpIdx + 0];
                const int32_t value = patchGroup.sndhw_tags[kvpIdx + 1];

                if (key == SNDHW_TAG_DRIVER_ID) {
                    patchGroup.hw_table_list.driver_id = (SoundDriverId) value;
                } else if (key == SNDHW_TAG_SOUND_EFFECTS) {
                    patchGroup.hw_table_list.sfxload |= (value & 1);
                } else if (key == SNDHW_TAG_MUSIC) {
                    patchGroup.hw_table_list.musload |= (value & 1);
                } else if (key == SNDHW_TAG_DRUMS) {
                    patchGroup.hw_table_list.drmload |= (value & 1);
                }
            }
        }
    }

    // Read patch group info for each sound driver and figure out the total number of voices for all patch groups
    mstat.max_voices = 0;

    for (int32_t filePatchGrpIdx = 0; filePatchGrpIdx < module.hdr.num_patch_groups; ++filePatchGrpIdx) {
        // Read the patch group header.
        // Note: in the original code this data structure was a global, but I've made it a local here...
        patch_group_header patchGroupHdr;
        wess_memcpy(&patchGroupHdr, gpWess_curWmdFileBytes, sizeof(patch_group_header));
        gpWess_curWmdFileBytes += sizeof(patch_group_header);

        // Try to match against one of the sound drivers loaded
        for (int32_t patchGroupIdx = 0; patchGroupIdx < mstat.num_patch_groups; ++patchGroupIdx) {
            // Does this patch group play with this sound hardware? If it doesn't then skip over it:
            patch_group_data& patchGroup = pPatchGroups[patchGroupIdx];

            if (patchGroupHdr.driver_id != patchGroup.hw_table_list.driver_id)
                continue;
            
            // Save the header, pointer to patch data and offset, and increment the total voice count.
            // PsyDoom: also ensure the destination bytes pointer is properly aligned.
            wess_align_byte_ptr(pCurDestBytes, alignof(patch));
            
            patchGroup.hdr = patchGroupHdr;
            patchGroup.pdata = pCurDestBytes;
            patchGroup.modfile_offset = (int32_t)(gpWess_curWmdFileBytes - gpWess_wmdFileBytesBeg);

            mstat.max_voices += patchGroupHdr.hw_voice_limit;

            // Load the various types of patch group data
            {
                const bool bReadSuccess = conditional_read(
                    patchGroupHdr.load_flags & LOAD_PATCHES,
                    pCurDestBytes,
                    (int32_t) patchGroupHdr.num_patches * patchGroupHdr.patch_size
                );

                if (!bReadSuccess)
                    return false;
            }

            {
                wess_align_byte_ptr(pCurDestBytes, alignof(patch_voice));
                
                const bool bReadSuccess = conditional_read(
                    patchGroupHdr.load_flags & LOAD_PATCH_VOICES,
                    pCurDestBytes,
                    (int32_t) patchGroupHdr.num_patch_voices * patchGroupHdr.patch_voice_size
                );

                if (!bReadSuccess)
                    return false;
            }
            
            {
                wess_align_byte_ptr(pCurDestBytes, alignof(patch_sample));
            
                const bool bReadSuccess = conditional_read(
                    patchGroupHdr.load_flags & LOAD_PATCH_SAMPLES,
                    pCurDestBytes,
                    (int32_t) patchGroupHdr.num_patch_samples * patchGroupHdr.patch_sample_size
                );
                
                if (!bReadSuccess)
                    return false;
            }
            
            {
                wess_align_byte_ptr(pCurDestBytes, alignof(drum_patch));
            
                const bool bReadSuccess = conditional_read(
                    patchGroupHdr.load_flags & LOAD_DRUM_PATCHES,
                    pCurDestBytes,
                    (int32_t) patchGroupHdr.num_drum_patches * patchGroupHdr.drum_patch_size
                );
                
                if (!bReadSuccess)
                    return false;
            }

            {
                wess_align_byte_ptr(pCurDestBytes, alignof(void*));
            
                const bool bReadSuccess = conditional_read(
                    patchGroupHdr.load_flags & LOAD_EXTRA_DATA,
                    pCurDestBytes,
                    patchGroupHdr.extra_data_size
                );
                
                if (!bReadSuccess)
                    return false;
            }

            // Found the sound driver for this patch group, don't need to search the rest
            break;
        }
    }

    // Alloc the list of voice status structs and link to the master status struct
    wess_align_byte_ptr(pCurDestBytes, alignof(voice_status));
    voice_status* const pVoiceStat = (voice_status*) pCurDestBytes;
    mstat.pvoice_stats = pVoiceStat;
    pCurDestBytes += sizeof(voice_status) * mstat.max_voices;

    // Assign hardware voices for each sound driver to the voice status structures allocated previously
    if (mstat.num_patch_groups > 0) {
        int32_t patchGroupsLeft = mstat.num_patch_groups;
        const patch_group_data* pPatchGroup = pPatchGroups;

        int32_t hwVoicesLeft = pPatchGroup->hdr.hw_voice_limit;
        uint8_t hwVoiceIdx = 0;

        for (int32_t voiceIdx = 0; voiceIdx < mstat.max_voices;) {
            // Have we assigned voices for all drivers? If so then we are done...
            if (patchGroupsLeft <= 0)
                break;

            // Move onto the next sound driver if all voices for this driver have been assigned
            if (hwVoicesLeft <= 0) {
                if (patchGroupsLeft > 0) {
                    --patchGroupsLeft;
                    ++pPatchGroup;
                    hwVoicesLeft = pPatchGroup->hdr.hw_voice_limit;
                    hwVoiceIdx = 0;
                }
            }

            // If there are any hardware voices left to be assigned then assign them to this status struct
            voice_status& voice = mstat.pvoice_stats[voiceIdx];

            if (hwVoicesLeft > 0) {
                --hwVoicesLeft;
                voice.driver_id = pPatchGroup->hdr.driver_id;
                voice.ref_idx = hwVoiceIdx;
                ++hwVoiceIdx;

                // PsyDoom: fix a small logic issue, which shouldn't be a problem in practice.
                // Move onto the next voice status struct *ONLY* if we actually assigned it to a hardware voice.
                // Previously, if a driver had '0' hardware voices then we might leak or leave unused one 'voice status' struct:
                #if PSYDOOM_MODS
                    ++voiceIdx;
                #endif
            }

            // PsyDoom: part of the fix mentioned above, only preform this increment conditionally now
            #if !PSYDOOM_MODS
                ++voiceIdx;
            #endif
        }
    }

    // Alloc the list of sequence info structs and link to the master status struct
    wess_align_byte_ptr(pCurDestBytes, alignof(sequence_data));
    sequence_data* const pSequence = (sequence_data*) pCurDestBytes;
    pCurDestBytes += sizeof(sequence_data) * module.hdr.num_sequences;
    module.psequences = pSequence;

    // These stats hold the maximums for all sequences
    uint8_t maxTracksPerSeq = MINIMUM_TRACK_INDXS_FOR_A_SEQUENCE;
    uint8_t maxVoicesPerTrack = 0;
    uint8_t maxLocStackSizePerTrack = 0;
    
    // Determine track stats and sequence headers for all sequences
    for (int32_t seqIdx = 0; seqIdx < module.hdr.num_sequences; ++seqIdx) {
        // Read the sequence header, save the sequence position in the file and move past it
        sequence_data& sequence = module.psequences[seqIdx];
        wess_memcpy(&sequence.hdr, gpWess_curWmdFileBytes, sizeof(sequence_header));

        sequence.modfile_offset = (uint32_t)(gpWess_curWmdFileBytes - gpWess_wmdFileBytesBeg);
        gpWess_curWmdFileBytes += sizeof(sequence_header);

        // Run through all tracks in the sequence and figure out the stats (size etc.) for what will be loaded
        uint8_t numTracksToload = 0;
        uint32_t tracksTotalSize = 0;

        for (int32_t trackIdx = 0; trackIdx < sequence.hdr.num_tracks; ++trackIdx) {
            // Read the track header and move on in the file.
            // Note: in the original code this data structure was a global, but I've made it a local here...
            track_header trackHdr;
            wess_memcpy(&trackHdr, gpWess_curWmdFileBytes, sizeof(track_header));
            gpWess_curWmdFileBytes += sizeof(track_header);

            // Decide whether the track is to be loaded for this sound driver
            bool bLoadTrack = false;

            if ((trackHdr.driver_id == NoSound_ID) || (trackHdr.driver_id == GENERIC_ID)) {
                // This track is not associated with any sound driver or works with any sound driver: load always
                bLoadTrack = true;
            }
            else {
                // Not doing an unconditional load of this track.
                // Only load it if it is for one of the loaded sound drivers and loading this track type is allowed:
                for (int32_t patchGroupIdx = 0; patchGroupIdx < mstat.num_patch_groups; ++patchGroupIdx) {
                    patch_group_data& patchGroup = pPatchGroups[patchGroupIdx];

                    // Is the track for this sound driver?
                    if (trackHdr.driver_id != patchGroup.hw_table_list.driver_id)
                        continue;

                    // Only load the track if it's a known voice class and the driver wants to load that voice class
                    if ((trackHdr.sound_class == SNDFX_CLASS) || (trackHdr.sound_class == SFXDRUMS_CLASS)) {
                        if (patchGroup.hw_table_list.sfxload) {
                            bLoadTrack = true;
                            break;
                        }
                    }

                    if (trackHdr.sound_class == MUSIC_CLASS) {
                        if (patchGroup.hw_table_list.musload) {
                            bLoadTrack = true;
                            break;
                        }
                    }

                    if (trackHdr.sound_class == DRUMS_CLASS) {
                        if (patchGroup.hw_table_list.drmload) {
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
                tracksTotalSize += (uint32_t) trackHdr.num_labels * sizeof(uint32_t);
                tracksTotalSize += trackHdr.cmd_stream_size;
                tracksTotalSize += tracksTotalSize & 1;         // Added size due to 32-bit align..
                tracksTotalSize += tracksTotalSize & 2;         // Added size due to 32-bit align..

                // Incorporate track stats into the maximum for all sequences
                if (trackHdr.max_voices > maxVoicesPerTrack) {
                    maxVoicesPerTrack = trackHdr.max_voices;
                }

                if (trackHdr.loc_stack_size > maxLocStackSizePerTrack) {
                    maxLocStackSizePerTrack = trackHdr.loc_stack_size;
                }
            }

            // Move past this track in the WMD file
            gpWess_curWmdFileBytes += (uint32_t) trackHdr.num_labels * sizeof(uint32_t);
            gpWess_curWmdFileBytes += trackHdr.cmd_stream_size;
        }
        
        // Incorporate track count into the global max
        if (numTracksToload > maxTracksPerSeq) {
            maxTracksPerSeq = numTracksToload;
        }

        // If no tracks are in the sequence then we still allocate a small amount of track info.
        // This is for the 'dummy' track that gets created when the sequence is loaded.
        if (numTracksToload == 0) {
            tracksTotalSize = sizeof(track_data) + 4;   // +2 bytes for dummy track commands, +2 bytes to align afterwards...
        }

        // Save sequence stats
        sequence.num_tracks = numTracksToload;
        sequence.track_data_size = tracksTotalSize;
    }

    // Save the sequence & track limits figured out in the loop above
    mstat.max_tracks_per_seq = maxTracksPerSeq;
    mstat.max_voices_per_track = maxVoicesPerTrack;
    mstat.max_track_loc_stack_size = maxLocStackSizePerTrack;

    // Alloc the list of callback status structs and link to the master status struct
    wess_align_byte_ptr(pCurDestBytes, alignof(callback_status));
    callback_status* const pCallbackStats = (callback_status*) pCurDestBytes;
    pCurDestBytes += sizeof(callback_status) * module.hdr.max_callbacks;
    mstat.pcallback_stats = pCallbackStats;

    // Allocate the sequence work areas
    for (int32_t seqStatIdx = 0; seqStatIdx < module.hdr.max_active_sequences; ++seqStatIdx) {
        sequence_status& seqStat = mstat.psequence_stats[seqStatIdx];

        // Allocate the gates array for this sequence and 32-bit align afterwards
        seqStat.pgates = (uint8_t*) pCurDestBytes;

        pCurDestBytes += sizeof(uint8_t) * module.hdr.max_gates_per_seq;
        pCurDestBytes += (uintptr_t) pCurDestBytes & 1;     // Added size due to 32-bit align..
        pCurDestBytes += (uintptr_t) pCurDestBytes & 2;     // Added size due to 32-bit align..

        // Alloc the iters array for this sequence and 32-bit align afterwards
        seqStat.piters = (uint8_t*) pCurDestBytes;

        pCurDestBytes += sizeof(uint8_t) * module.hdr.max_iters_per_seq;
        pCurDestBytes += (uintptr_t) pCurDestBytes & 1;     // Added size due to 32-bit align..
        pCurDestBytes += (uintptr_t) pCurDestBytes & 2;     // Added size due to 32-bit align..

        // Alloc the track indexes array for this sequence and 32-bit align afterwards
        seqStat.ptrackstat_indices = (uint8_t*) pCurDestBytes;

        pCurDestBytes += sizeof(uint8_t) * maxTracksPerSeq;
        pCurDestBytes += (uintptr_t) pCurDestBytes & 1;     // Added size due to 32-bit align..
        pCurDestBytes += (uintptr_t) pCurDestBytes & 2;     // Added size due to 32-bit align..

        // Initialize the track indexes array for the sequence
        for (int32_t trackSlotIdx = 0; trackSlotIdx < maxTracksPerSeq; ++trackSlotIdx) {
            seqStat.ptrackstat_indices[trackSlotIdx] = 0xFF;
        }
    }

    // Allocate the sub-stacks for each track work area.
    // These are used to hold a stack track locations that can be pushed and popped to by sequencer commands for save/return behavior.
    wess_align_byte_ptr(pCurDestBytes, alignof(void*));
    
    for (uint8_t trackIdx = 0; trackIdx < module.hdr.max_active_tracks; ++trackIdx) {
        track_status& trackStat = pTrackStat[trackIdx];

        trackStat.ref_idx = trackIdx;
        trackStat.ploc_stack = (uint8_t**) pCurDestBytes;
        pCurDestBytes += sizeof(uint8_t*) * mstat.max_track_loc_stack_size;
        trackStat.ploc_stack_end = (uint8_t**) pCurDestBytes;
    }

    // Initialize the sequencer
    gWess_CmdFuncArr[NoSound_ID][DriverInit](mstat);
    
    // Initialize loaded drivers
    for (int32_t patchGroupIdx = 0; patchGroupIdx < mstat.num_patch_groups; ++ patchGroupIdx) {
        patch_group_data& patchGroup = pPatchGroups[patchGroupIdx];
        
        // Initialize the driver if we are loading any voices and tracks relating to it
        const bool bInitDriver = (
            patchGroup.hw_table_list.sfxload ||
            patchGroup.hw_table_list.musload ||
            patchGroup.hw_table_list.drmload
        );

        if (bInitDriver) {
            // Initialize the driver
            gWess_CmdFuncArr[patchGroup.hw_table_list.driver_id][DriverInit](mstat);
        }
    }

    // The module is now loaded and the sequencer is enabled
    gbWess_module_loaded = true;
    gbWess_SeqOn = true;

    // Save the end pointer for the loaded module and ensure 32-bit aligned
    pCurDestBytes += (uintptr_t) pCurDestBytes & 1;
    pCurDestBytes += (uintptr_t) pCurDestBytes & 2;
    gpWess_wmd_end = pCurDestBytes;

    // This is maximum sequence number that can be triggered
    gWess_end_seq_num = module.hdr.num_sequences;

    // PsyDoom: sanity check we haven't overflowed module memory
    #if PSYDOOM_MODS
        ASSERT(gpWess_wmd_end - gpWess_wmd_mem <= gWess_mem_limit);
    #endif
    
    // Load was a success!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs initialization of most of the track status struct, given the specified track info and optional play attributes
//------------------------------------------------------------------------------------------------------------------------------------------
void filltrackstat(track_status& trackStat, const track_data& track, const TriggerPlayAttr* const pAttribs) noexcept {
    // Basic track status initialization
    trackStat.active = true;
    trackStat.timed = false;
    trackStat.looped = false;
    trackStat.skip = false;
    trackStat.off = false;

    trackStat.driver_id = track.hdr.driver_id;
    trackStat.priority = track.hdr.priority;
    trackStat.sound_class = track.hdr.sound_class;
    trackStat.num_active_voices = 0;
    trackStat.max_voices = track.hdr.max_voices;
    trackStat.tempo_ppq = track.hdr.init_ppq;
    trackStat.deltatime_qnp_frac = 0;
    trackStat.deltatime_qnp = 0;
    trackStat.abstime_qnp = 0;
    trackStat.ploc_stack_cur = trackStat.ploc_stack;
    trackStat.num_labels = track.hdr.num_labels;
    trackStat.cmd_stream_size = track.hdr.cmd_stream_size;
    trackStat.mutegroups_mask = track.hdr.init_mutegroups_mask;

    // If no play attributes are given then the attribute mask to customize is simply zero
    const uint32_t attribsMask = (pAttribs) ? pAttribs->attribs_mask : 0;

    // Set all of the track attributes that can be customized, using any customizations given
    if (attribsMask & TRIGGER_VOLUME) {
        trackStat.volume_cntrl = pAttribs->volume_cntrl;
    } else {
        trackStat.volume_cntrl = track.hdr.init_volume_cntrl;
    }
    
    if (attribsMask & TRIGGER_PAN) {
        trackStat.pan_cntrl = pAttribs->pan_cntrl;
    } else {
        trackStat.pan_cntrl = track.hdr.init_pan_cntrl;
    }
    
    if (attribsMask & TRIGGER_PATCH) {
        trackStat.patch_idx = pAttribs->patch_idx;
    } else {
        trackStat.patch_idx = track.hdr.init_patch_idx;
    }
    
    if (attribsMask & TRIGGER_PITCH) {
        trackStat.pitch_cntrl = pAttribs->pitch_cntrl;
    } else {
        trackStat.pitch_cntrl = track.hdr.init_pitch_cntrl;
    }
    
    if ((attribsMask & TRIGGER_MUTEMODE) && (trackStat.mutegroups_mask & (1 << pAttribs->mutegroup))) {
        trackStat.mute = true;
    } else {
        trackStat.mute = false;
    }
    
    if (attribsMask & TRIGGER_TEMPO) {
        trackStat.tempo_qpm = pAttribs->tempo_qpm;
    } else {
        trackStat.tempo_qpm = track.hdr.init_qpm;
    }

    const int16_t interruptsPerSec = GetIntsPerSec();
    trackStat.tempo_ppi_frac = CalcPartsPerInt(interruptsPerSec, trackStat.tempo_ppq, trackStat.tempo_qpm);     // Compute how much to advance the track's timing for each hardware timer interrupt

    if (attribsMask & TRIGGER_TIMED) {
        trackStat.timed = true;
        trackStat.end_abstime_qnp = trackStat.abstime_qnp + pAttribs->playtime_qnp;
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
        trackStat.reverb = track.hdr.init_reverb;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sets up some basic position and count related info for the given track status using the track info.
// Determines the time until the 1st sequencer command also.
//------------------------------------------------------------------------------------------------------------------------------------------
void assigntrackstat(track_status& trackStat, const track_data& track) noexcept {
    trackStat.cmd_stream_capacity = track.hdr.cmd_stream_size;
    trackStat.end_label_idx = track.hdr.num_labels;
    trackStat.pcmds_start = track.pcmd_stream;
    trackStat.pcur_cmd = Read_Vlq(track.pcmd_stream, trackStat.qnp_till_next_cmd);
    trackStat.plabels = track.plabels;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Allocates and sets up sequence and track status structs for the sequence to be played.
// The given play attributes etc. can be used to customize sequence playback.
// On success the index of the sequence allocated (+1) is returned, otherwise '0' is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_seq_structrig(
    const sequence_data& sequence,
    const int32_t seqIdx,
    const uintptr_t seqType,
    const bool bGetHandle,
    const TriggerPlayAttr* pPlayAttribs
) noexcept {
    // Can't play the sequence if the number is not valid
    if (!Is_Seq_Num_Valid(seqIdx))
        return 0;

    // Disable sequencer ticking temporarily (to avoid hardware timer interrupts) while we setup all this
    gbWess_SeqOn = false;

    master_status_structure& mstat = *gpWess_pm_stat;
    module_data& module = *mstat.pmodule;

    const uint8_t maxActiveSeqs = module.hdr.max_active_sequences;
    const uint8_t maxActiveTracks = module.hdr.max_active_tracks;

    // Find a free sequence status structure that we can use to play this sequence
    uint8_t allocSeqStatIdx = 0;

    while ((allocSeqStatIdx < maxActiveSeqs) && (mstat.psequence_stats[allocSeqStatIdx].active)) {
        ++allocSeqStatIdx;
    }

    // If we failed to allocate a sequence status structure then we can't play anything
    if (allocSeqStatIdx >= maxActiveSeqs) {
        gbWess_SeqOn = true;
        return 0;
    }

    // Try to allocate free tracks for all of the tracks that the sequence needs to play.
    // Note that this loop assumes each sequence will always have at least 1 track to play.
    sequence_status& seqStat = mstat.psequence_stats[allocSeqStatIdx];
    uint8_t* const pTrackStatIndices = seqStat.ptrackstat_indices;

    uint8_t numTracksPlayed = 0;
    uint8_t numTracksToPlay = (uint8_t) sequence.hdr.num_tracks;

    for (uint8_t trackStatIdx = 0; trackStatIdx < maxActiveTracks; ++trackStatIdx) {
        // Ensure this track is not in use before we do anything
        track_status& trackStat = mstat.ptrack_stats[trackStatIdx];

        if (trackStat.active)
            continue;

        // Great! We can use this track for one of the sequence tracks.
        // Set it's owner and start filling in the track status:
        trackStat.seqstat_idx = allocSeqStatIdx;

        track_data& track = sequence.ptracks[numTracksPlayed];
        filltrackstat(trackStat, track, pPlayAttribs);
        assigntrackstat(trackStat, track);

        if (bGetHandle) {
            trackStat.handled = true;
            trackStat.stopped = true;
        } else {
            trackStat.handled = false;
            trackStat.stopped = false;
            seqStat.num_tracks_playing++;
        }

        // There is one more track playing the sequence and globally
        seqStat.num_tracks_active += 1;
        mstat.num_active_tracks += 1;

        // Remember what track is being used on the sequence
        pTrackStatIndices[numTracksPlayed] = trackStatIdx;
        numTracksPlayed++;

        // If we have allocated all the tracks in the sequence then we are done
        if (numTracksPlayed >= numTracksToPlay)
            break;
    }

    // Setup various sequence bits if we actually started playing some tracks
    if (numTracksPlayed > 0) {
        seqStat.seq_idx = (int16_t) seqIdx;
        seqStat.type = seqType;

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
        mstat.num_active_seqs += 1;
    }

    // Renable the sequencer's timer interrupts/updates and return what sequence index we started playing (if any)
    gbWess_SeqOn = true;

    if (numTracksPlayed > 0) {
        return (int32_t) allocSeqStatIdx + 1;
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
    master_status_structure& mstat = *gpWess_pm_stat;
    wess_seq_structrig(mstat.pmodule->psequences[seqIdx], seqIdx, 0, false, pPlayAttribs);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the current state of the specified sequence
//------------------------------------------------------------------------------------------------------------------------------------------
SequenceStatus wess_seq_status(const int32_t seqIdx) noexcept {
    // Is the sequence number a valid one?
    if (!Is_Seq_Num_Valid(seqIdx))
        return SEQUENCE_INVALID;
    
    // Try to find the specified sequence number among all the sequences
    master_status_structure& mstat = *gpWess_pm_stat;
    const int32_t maxSeqs = mstat.pmodule->hdr.max_active_sequences;

    for (uint8_t statIdx = 0; statIdx < maxSeqs; ++statIdx) {
        sequence_status& seqStat = mstat.psequence_stats[statIdx];

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
    gbWess_SeqOn = false;
    
    // Run through all of the sequences searching for the one we are interested in
    master_status_structure& mstat = *gpWess_pm_stat;

    const uint8_t maxSeqs = mstat.pmodule->hdr.max_active_sequences;
    const uint32_t maxTracksPerSeq = mstat.max_tracks_per_seq;
    uint8_t numActiveSeqsToVisit = mstat.num_active_seqs;

    for (uint8_t seqStatIdx = 0; seqStatIdx < maxSeqs; ++seqStatIdx) {
        // If there are no more active sequences to visit then we are done
        if (numActiveSeqsToVisit == 0)
            break;

        // Only bother if the sequence is loaded/active
        sequence_status& seqStat = mstat.psequence_stats[seqStatIdx];

        if (!seqStat.active)
            continue;
        
        if (seqStat.seq_idx == seqIdx) {
            // This is the sequence we want, go through all the tracks and stop each one
            uint32_t numActiveTracksToVisit = seqStat.num_tracks_active;
            uint8_t* const pTrackStatIndices = seqStat.ptrackstat_indices;

            for (uint32_t trackSlotIdx = 0; trackSlotIdx < maxTracksPerSeq; ++trackSlotIdx) {
                // Is this track index valid and in use?
                const uint8_t trackStatIdx = pTrackStatIndices[trackSlotIdx];

                if (trackStatIdx == 0xFF)
                    continue;
                
                // Call the driver function to turn off the track
                track_status& trackStat = mstat.ptrack_stats[trackStatIdx];
                gWess_CmdFuncArr[trackStat.driver_id][TrkOff](trackStat);

                // If there are no more tracks left active then we are done
                --numActiveTracksToVisit;

                if (numActiveTracksToVisit == 0)
                    break;
            }
        }

        --numActiveSeqsToVisit;
    }

    // Re-enable the sequencer
    gbWess_SeqOn = true;
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
    gbWess_SeqOn = false;

    // Grab some basic info from the master status
    master_status_structure& mstat = *gpWess_pm_stat;

    const uint8_t maxSeqs = mstat.pmodule->hdr.max_active_sequences;
    const uint32_t maxTracksPerSeq = mstat.max_tracks_per_seq;
    uint8_t numActiveSeqsToVisit = mstat.num_active_seqs;

    if (numActiveSeqsToVisit > 0) {
        // Run through all of the active sequences and stop them all
        for (uint8_t seqStatIdx = 0; seqStatIdx < maxSeqs; ++seqStatIdx) {
            sequence_status& seqStat = mstat.psequence_stats[seqStatIdx];

            if (!seqStat.active)
                continue;

            // Run through all of the active tracks in the sequence and stop them all
            uint32_t numActiveTracksToVisit = seqStat.num_tracks_active;
            uint8_t* const pTrackStatIndices = seqStat.ptrackstat_indices;

            for (uint32_t trackSlotIdx = 0; trackSlotIdx < maxTracksPerSeq; ++trackSlotIdx) {
                // Is this sequence track slot actually in use? Skip if not:
                const uint8_t trackStatIdx = pTrackStatIndices[trackSlotIdx];

                if (trackStatIdx == 0xFF)
                    continue;
                
                // Call the driver function to turn off the track
                track_status& trackStat = mstat.ptrack_stats[trackStatIdx];
                gWess_CmdFuncArr[trackStat.driver_id][TrkOff](trackStat);

                // If there are no more tracks left active then we are done
                numActiveTracksToVisit--;

                if (numActiveTracksToVisit == 0)
                    break;
            }

            // If there are no more active sequences to visit then we are done
            numActiveSeqsToVisit--;

            if (numActiveSeqsToVisit == 0)
                break;
        }
    }

    // Re-enable the sequencer
    gbWess_SeqOn = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition that aligns a byte pointer forward until it matches the given alignment
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_align_byte_ptr(uint8_t*& ptr, const size_t alignment) noexcept {
    #if PSYDOOM_MODS
        ASSERT(alignment > 0);
        ptr = (uint8_t*)((((uintptr_t) ptr + alignment - 1u) / alignment) * alignment);
    #endif
}
