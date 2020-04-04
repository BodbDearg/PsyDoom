//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): sequence/song loader.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "seqload.h"

#include "PsxVm/PsxVm.h"
#include "wessarc.h"

static const VmPtr<int32_t>                             gWess_num_sequences(0x8007596C);            // TODO: COMMENT
static const VmPtr<int32_t>                             gWess_seqld_moduleRefCount(0x80075974);     // TODO: COMMENT
static const VmPtr<VmPtr<PsxCd_File>>                   gpWess_seqld_moduleFile(0x80075980);        // TODO: COMMENT
static const VmPtr<CdMapTbl_File>                       gWess_seqld_moduleFileId(0x80075964);       // TODO: COMMENT
static const VmPtr<bool32_t>                            gbWess_seq_loader_enable(0x80075960);       // TODO: COMMENT
static const VmPtr<VmPtr<master_status_structure>>      gpWess_seqld_mstat(0x80075968);             // TODO: COMMENT
static const VmPtr<track_header>                        gWess_seqld_seqTrackHdr(0x8007F050);        // TODO: COMMENT
static const VmPtr<track_header>                        gWess_seqld_emptyTrackHdr(0x8007F068);      // TODO: COMMENT

static SeqLoaderErrorHandler    gpWess_seqld_errorHandler;      // Callback invoked if there are problems loading sequences
static int32_t                  gWess_seqld_errorModule;        // Module value passed to the error handling callback

//------------------------------------------------------------------------------------------------------------------------------------------
// Invokes the sequence loader error handler with the given error code
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_load_err(const Seq_Load_Error errorCode) noexcept {
    if (gpWess_seqld_errorHandler) {
        gpWess_seqld_errorHandler(gWess_seqld_errorModule, errorCode);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Install an error callback which can be invoked if errors occur with the sequence loader
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_loader_install_error_handler(const SeqLoaderErrorHandler handler, const int32_t module) noexcept {
    gpWess_seqld_errorHandler = handler;
    gWess_seqld_errorModule = module;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given sequence index is valid for the open module (.WMD) file
//------------------------------------------------------------------------------------------------------------------------------------------
bool Is_Seq_Seq_Num_Valid(const int32_t seqIdx) noexcept {
    return ((seqIdx >= 0) && (seqIdx < *gWess_num_sequences));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Retain or add a reference to the open module file used to load sequences.
// If the file is not opened yet then it is opened.
// Returns 'true' if the file is successfully opened.
//------------------------------------------------------------------------------------------------------------------------------------------
bool open_sequence_data() noexcept {
    if (*gWess_seqld_moduleRefCount == 0) {
        *gpWess_seqld_moduleFile = module_open(*gWess_seqld_moduleFileId);

        if (!gpWess_seqld_moduleFile->get()) {
            wess_seq_load_err(SEQLOAD_FOPEN);
            return false;
        }
    }

    *gWess_seqld_moduleRefCount += 1;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Release a reference to the open module file used to load sequences.
// If the reference count falls to '0' then the file is closed.
//------------------------------------------------------------------------------------------------------------------------------------------
void close_sequence_data() noexcept {
    if (*gWess_seqld_moduleRefCount == 1) {
        module_close(*gpWess_seqld_moduleFile->get());
    }

    if (*gWess_seqld_moduleRefCount > 0) {
        *gWess_seqld_moduleRefCount -= 1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads the given sequence into the given memory block, which is expected to be big enough.
// Returns the number of bytes used from the given memory block, or '0' on failure.
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t load_sequence_data(const int32_t seqNum, void* const pSeqMem) noexcept {
    // Can't load if the sequence loader wasn't initialized or the sequence index is invalid
    if ((!*gbWess_seq_loader_enable) || (!Is_Seq_Seq_Num_Valid(seqNum)))
        return 0;

    // Make sure the module file containing the sequence is open
    if (!open_sequence_data()) {
        wess_seq_load_err(SEQLOAD_FOPEN);
        return 0;
    }

    // Allocate room for all the track infos in the sequence.
    // If there are no tracks in the sequence then alloc room for one default empty track:
    master_status_structure& mstat = *gpWess_seqld_mstat->get();
    module_data& modInfo = *mstat.pmod_info;
    sequence_data& seqInfo = modInfo.pseq_info[seqNum];
    
    uint8_t* pCurSeqMem = (uint8_t*) pSeqMem;
    seqInfo.ptrk_info = (track_data*) pSeqMem;

    if (seqInfo.trkstoload == 0) {
        pCurSeqMem += sizeof(track_data);
    } else {
        pCurSeqMem += sizeof(track_data) * seqInfo.trkstoload;
    }

    // Go to where the sequence is located in the module file
    PsxCd_File& moduleFile = *gpWess_seqld_moduleFile->get();

    if (module_seek(moduleFile, seqInfo.fileposition, PsxCd_SeekMode::SET) != 0) {
        wess_seq_load_err(SEQLOAD_FSEEK);
        return 0;
    }

    // Read the sequence header
    if (module_read(&seqInfo.seq_hdr, sizeof(seq_header), moduleFile) != sizeof(seq_header)) {
        wess_seq_load_err(SEQLOAD_FREAD);
        return 0;
    }

    // Read all of the tracks in the sequence
    for (uint16_t trackIdx = 0; trackIdx < seqInfo.seq_hdr.tracks; ++trackIdx) {
        // Read the track header firstly
        track_header& trackHdr = *gWess_seqld_seqTrackHdr;

        if (module_read(&trackHdr, sizeof(track_header), moduleFile) != sizeof(track_header)) {
            wess_seq_load_err(SEQLOAD_FREAD);
            return 0;
        }

        // Determine whether we want to load the track or not depending on what driver it is for or what sound classes we want to load
        bool bLoadTrack = false;

        if ((trackHdr.voices_type == NoSound_ID) || (trackHdr.voices_type == GENERIC_ID)) {
            // Track is for no particular sound driver or a generic one: always load
            bLoadTrack = true;
        } else {
            // Track is for a hardware sound driver: determine if it matches any of the drivers loaded and if those drivers want this track:
            for (uint32_t driverIdx = 0; driverIdx < mstat.patch_types_loaded; ++driverIdx) {
                patch_group_data& patchGrp = mstat.ppat_info[driverIdx];

                // Don't load if it's not for this driver
                if (trackHdr.voices_type != patchGrp.hw_tl_list.hardware_ID)
                    continue;

                // Only load if the flags specify to for whatever voice class it is
                if (patchGrp.hw_tl_list.sfxload && ((trackHdr.voices_class == SNDFX_CLASS) || (trackHdr.voices_class == SFXDRUMS_CLASS))) {
                    bLoadTrack = true;
                } else if (patchGrp.hw_tl_list.musload && (trackHdr.voices_class == MUSIC_CLASS)) {
                    bLoadTrack = true;
                } else if (patchGrp.hw_tl_list.drmload && (trackHdr.voices_class == DRUMS_CLASS)) {
                    bLoadTrack = true;
                }
            }
        }

        // If we are not loading the track try to skip past it
        if (!bLoadTrack) {
            const uint32_t bytesToSkip = trackHdr.labellist_count * sizeof(uint32_t) + trackHdr.data_size;

            if (module_seek(moduleFile, bytesToSkip, PsxCd_SeekMode::CUR) != 0) {
                wess_seq_load_err(SEQLOAD_FSEEK);
                return 0;
            }

            continue;
        }

        // Loading the track: save the header
        track_data& trackInfo = seqInfo.ptrk_info[trackIdx];
        trackInfo.trk_hdr = trackHdr;

        // Assign the sound driver to use for this track
        if (trackHdr.voices_type == GENERIC_ID) {
            trackInfo.trk_hdr.voices_type = SoundDriverId::NoSound_ID;

            if ((trackHdr.voices_class == SNDFX_CLASS) || (trackHdr.voices_class == SFXDRUMS_CLASS)) {
                // Track is SFX: find a driver which wants to load SFX
                for (uint32_t driverIdx = 0; driverIdx < mstat.patch_types_loaded; ++driverIdx) {
                    patch_group_data& patchGrp = mstat.ppat_info[driverIdx];
                    
                    if (patchGrp.hw_tl_list.sfxload) {
                        trackInfo.trk_hdr.voices_type = (SoundDriverId) patchGrp.hw_tl_list.hardware_ID;
                        break;
                    }
                }
            } 
            else if (trackHdr.voices_class == MUSIC_CLASS) {
                // Track is music: find a driver which wants to load music
                for (uint32_t driverIdx = 0; driverIdx < mstat.patch_types_loaded; ++driverIdx) {
                    patch_group_data& patchGrp = mstat.ppat_info[driverIdx];
                    
                    if (patchGrp.hw_tl_list.musload) {
                        trackInfo.trk_hdr.voices_type = (SoundDriverId) patchGrp.hw_tl_list.hardware_ID;
                        break;
                    }
                }
            } 
            else if (trackHdr.voices_class == 2) {
                // Track is drums: find a driver which wants to load drums
                for (uint32_t driverIdx = 0; driverIdx < mstat.patch_types_loaded; ++driverIdx) {
                    patch_group_data& patchGrp = mstat.ppat_info[driverIdx];
                    
                    if (patchGrp.hw_tl_list.drmload) {
                        trackInfo.trk_hdr.voices_type = (SoundDriverId) patchGrp.hw_tl_list.hardware_ID;
                        break;
                    }
                }
            }
        }

        // Alloc the label list and read it
        const uint16_t labelListCount = trackInfo.trk_hdr.labellist_count;
        const int32_t labelListSize = labelListCount * sizeof(uint32_t);

        trackInfo.plabellist = (uint32_t*) pCurSeqMem;
        pCurSeqMem += labelListSize;

        if (module_read(trackInfo.plabellist.get(), labelListSize, moduleFile) != labelListSize) {
            wess_seq_load_err(SEQLOAD_FREAD);
            return 0;
        }

        // Alloc the track data and 32-bit align the pointer afterwords since the track data might not be dword size aligned
        trackInfo.ptrk_data = pCurSeqMem;
        const int32_t trackDataSize = trackInfo.trk_hdr.data_size;

        pCurSeqMem += trackDataSize;
        pCurSeqMem += ((uintptr_t) pCurSeqMem) & 1;
        pCurSeqMem += ((uintptr_t) pCurSeqMem) & 2;
        
        // Read the track data
        if (module_read(trackInfo.ptrk_data.get(), trackDataSize, moduleFile) != trackDataSize) {
            wess_seq_load_err(SEQLOAD_FREAD);
            return 0;
        }
    }

    // If the sequence did not contain any tracks then initialize the default empty track we allocated earlier
    if (seqInfo.trkstoload == 0) {
        // Set the track header and label list pointer
        track_data& trackInfo = seqInfo.ptrk_info[0];
        trackInfo.trk_hdr = *gWess_seqld_emptyTrackHdr;
        trackInfo.plabellist = (uint32_t*) pCurSeqMem;
        
        // Init and alloc the track data with two commands.
        // Note that the memory pointer must be aligned after because it could wind up on an unaligned address.
        seqInfo.seq_hdr.tracks = 1;

        trackInfo.ptrk_data = pCurSeqMem;
        pCurSeqMem[0] = DriverInit;
        pCurSeqMem[1] = ResetGates;

        pCurSeqMem += trackInfo.trk_hdr.data_size;
        pCurSeqMem += ((uintptr_t) pCurSeqMem) & 1;
        pCurSeqMem += ((uintptr_t) pCurSeqMem) & 2;
    } else {
        seqInfo.seq_hdr.tracks = (uint16_t) seqInfo.trkstoload;
    }

    // Finish up the load by closing the module file and return the amount of memory used for the sequence
    close_sequence_data();
    return (int32_t)(pCurSeqMem - (uint8_t*) pSeqMem);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the sequencer loader using the given master status structure and module file, returns 'true' on success.
// Optionally the module file can be pre-opened in preparation for access later.
//------------------------------------------------------------------------------------------------------------------------------------------
bool wess_seq_loader_init(master_status_structure* const pMStat, const CdMapTbl_File moduleFileId, const bool bOpenModuleFile) noexcept {    
    // Some very basic initialization
    *gbWess_seq_loader_enable = false;
    *gWess_seqld_moduleFileId = moduleFileId;
    *gpWess_seqld_mstat = pMStat;

    // If there is no master stat then this fails
    if (!pMStat)
        return false;

    // Save the number of sequences in the module
    *gbWess_seq_loader_enable = true;
    *gWess_num_sequences = pMStat->pmod_info->mod_hdr.sequences;

    // Fill in the header for the default empty track (used for when a sequence has no tracks).
    //
    // TODO: is this what is used for game SFX?
    track_header& emptyTrackHdr = *gWess_seqld_emptyTrackHdr;
    emptyTrackHdr.priority = 128;
    emptyTrackHdr.initvolume_cntrl = 127;
    emptyTrackHdr.initpan_cntrl = 64;
    emptyTrackHdr.initppq = 120;
    emptyTrackHdr.initqpm = 120;
    emptyTrackHdr.voices_type = SoundDriverId::NoSound_ID;
    emptyTrackHdr.voices_max = 0;
    emptyTrackHdr.reverb = 0;
    emptyTrackHdr.voices_class = SoundClass::SNDFX_CLASS;
    emptyTrackHdr.initpatchnum = 0;
    emptyTrackHdr.initpitch_cntrl = 0;
    emptyTrackHdr.substack_count = 0;
    emptyTrackHdr.mutebits = 0;
    emptyTrackHdr.labellist_count = 0;
    emptyTrackHdr.data_size = 2;            // 2 commands of a single byte each in the default track
    
    // If requested, pre-open the module file also to have it ready
    if (bOpenModuleFile) {
        if (!open_sequence_data()) {
            wess_seq_load_err(SEQLOAD_FOPEN);
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the sequence loader
//------------------------------------------------------------------------------------------------------------------------------------------
void wess_seq_loader_exit() noexcept {
    close_sequence_data();
    *gbWess_seq_loader_enable = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns how many bytes still need to be loaded for the given sequence, which will be '0' when the sequence is loaded.
// Returns '0' also if the sequence loader is not initialized or the sequence number invalid.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_seq_sizeof(const int32_t seqIdx) noexcept {
    if ((*gbWess_seq_loader_enable) && Is_Seq_Seq_Num_Valid(seqIdx)) {
        master_status_structure& mstat = *gpWess_seqld_mstat->get();
        sequence_data& seqInfo = mstat.pmod_info->pseq_info[seqIdx];
        return (seqInfo.ptrk_info) ? 0 : seqInfo.trkinfolength;
    }

    return 0;
}

void wess_seq_load() noexcept {
loc_800451F4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x5960);                               // Load from: gbWess_seq_loader_enable (80075960)
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s1, sp + 0x14);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x1C);
    if (v0 == 0) goto loc_80045278;
    v0 = Is_Seq_Seq_Num_Valid(a0);
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 2;
        if (bJump) goto loc_80045238;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_8004527C;
loc_80045238:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x5968);                               // Load from: gpWess_seq_loader_pm_stat (80075968)
    v1 = lw(v1 + 0xC);
    v0 += s0;
    v1 = lw(v1 + 0x10);
    v0 <<= 2;
    v0 += v1;
    v0 = lw(v0 + 0x4);
    {
        const bool bJump = (v0 != 0);
        v0 = s1;                                        // Result = 00000000
        if (bJump) goto loc_8004527C;
    }
    a0 = s0;
    a1 = s2;
    v0 = load_sequence_data(a0, vmAddrToPtr<void>(a1));
    s1 = v0;
loc_80045278:
    v0 = s1;
loc_8004527C:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Clear the reference to the specified sequence's data and return 'true' if a reference was actually cleared.
// Depsite what the name implies, this does not actually free any memory - it just clears a pointer reference to the sequence data.
//------------------------------------------------------------------------------------------------------------------------------------------
bool wess_seq_free(const int32_t seqIdx) noexcept {    
    if ((!*gbWess_seq_loader_enable) || (!Is_Seq_Seq_Num_Valid(seqIdx)))
        return false;

    master_status_structure& mstat = *gpWess_seqld_mstat->get();
    sequence_data& seqInfo = mstat.pmod_info->pseq_info[seqIdx];

    if (seqInfo.ptrk_info) {
        seqInfo.ptrk_info = nullptr;
        return true;
    }

    return false;
}
