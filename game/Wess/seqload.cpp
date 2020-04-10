//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): sequence/song loader.
// Many thanks to Erick Vasquez Garcia (author of 'PSXDOOM-RE') for his reconstruction this module, upon which this interpretation is based.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "seqload.h"

#include "wessarc.h"

// Has the sequence loader been initialized?
const VmPtr<bool32_t>   gbWess_seq_loader_enable(0x80075960);

static const VmPtr<int32_t>                             gWess_num_sequences(0x8007596C);            // The number of sequences in the loaded module file
static const VmPtr<int32_t>                             gWess_seqld_moduleRefCount(0x80075974);     // Reference count for the opened module file - closed upon reaching '0'
static const VmPtr<VmPtr<PsxCd_File>>                   gpWess_seqld_moduleFile(0x80075980);        // The module file from which sequences are loaded
static const VmPtr<CdMapTbl_File>                       gWess_seqld_moduleFileId(0x80075964);       // File id for the module file
static const VmPtr<VmPtr<master_status_structure>>      gpWess_seqld_mstat(0x80075968);             // Saved reference to the master status structure
static const VmPtr<track_header>                        gWess_seqld_seqTrackHdr(0x8007F050);        // Track header for the current sequence track being loaded
static const VmPtr<track_header>                        gWess_seqld_emptyTrackHdr(0x8007F068);      // Track header for a generated dummy track for empty sequences with no tracks

static SeqLoaderErrorHandler    gpWess_seqld_errorHandler;      // Callback invoked if there are problems loading sequences
static int32_t                  gWess_seqld_errorModule;        // Module value passed to the error handling callback

//------------------------------------------------------------------------------------------------------------------------------------------
// Invokes the sequence loader error handler with the given error code
//------------------------------------------------------------------------------------------------------------------------------------------
static void wess_seq_load_err(const Seq_Load_Error errorCode) noexcept {
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

    // Fill in the header for the default empty track.
    // This is used to create a 'dummy' track when a sequence contains no tracks - so a sequence always has 1 track.
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Load the given sequence index into the given memory buffer, which is expected to be big enough to hold the sequence.
// Returns the number of bytes used from the given buffer, or '0' on failure or if the sequence is already loaded.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t wess_seq_load(const int32_t seqIdx, void* const pSeqMem) noexcept {
    // Don't load if the sequence index is invalid or the sequence loader not initialized
    if ((!*gbWess_seq_loader_enable) || (!Is_Seq_Seq_Num_Valid(seqIdx)))
        return 0;

    // Only load the sequence if not already loaded, otherwise we return '0' for using zero bytes from the given buffer
    master_status_structure& mstat = *gpWess_seqld_mstat->get();
    sequence_data& seqInfo = mstat.pmod_info->pseq_info[seqIdx];
    return (!seqInfo.ptrk_info) ? load_sequence_data(seqIdx, pSeqMem) : 0;
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
