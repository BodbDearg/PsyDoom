//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): PlayStation CD-ROM handling utilities
//------------------------------------------------------------------------------------------------------------------------------------------
#include "psxcd.h"

#include "PcPsx/DiscReader.h"
#include "PcPsx/FatalErrors.h"
#include "PcPsx/ModMgr.h"
#include "PcPsx/ProgArgs.h"
#include "PcPsx/PsxVm.h"
#include "PcPsx/Utils.h"
#include "psxspu.h"

#if PSYDOOM_MODS
    // Maximum number of open files
    static constexpr int32_t MAX_OPEN_FILES = 4;
#endif

// Time it takes to fade out CD audio (milliseconds)
static constexpr int32_t FADE_TIME_MS = 250;

// Locations on the disc for all CD tracks.
// This is used to determine where to seek to for cd audio playback.
CdlLOC gTrackCdlLOC[CdlMAXTOC];

// Various flags
static bool gbPSXCD_IsCdInit;               // If true then the 'psxcd' module has been initialized
static bool gbPSXCD_cb_enable_flag;         // True if callbacks are currently enabled
static bool gbPSXCD_playflag;               // If true then we are playing cd audio
static bool gbPSXCD_loopflag;               // If true then the currently played cd audio track will be looped
static bool gbPSXCD_seeking_for_play;       // If true then we are currently seeking to the location where the cd audio track being played will start
static bool gbPSXCD_waiting_for_pause;      // If true then we are waiting for a cd 'pause' operation to complete
static bool gbPSXCD_critical_error;         // True if a critical error occurred

// Whether the cdrom is currently in data or audio mode.
// 0 = audio mode, 1 = data mode, -1 = undefined.
static int32_t gPSXCD_psxcd_mode = -1;

// Data reading mode stuff
static PsxCd_File gPSXCD_cdfile;            // Used to hold a file temporarily after opening

// Audio mode stuff
static CdlLOC   gPSXCD_lastloc;             // The last valid intended cd-audio disc seek location
static CdlLOC   gPSXCD_newloc;              // Last known location for cd audio playback, this gets continously saved to so we can restore if we want to pause
static CdlLOC   gPSXCD_beginloc;            // Start sector of the current audio track
static CdlLOC   gPSXCD_cdloc;               // Temporary CdlLOC variable used in various places
static int32_t  gPSXCD_playvol;             // Specified playback volume for cd audio
static int32_t  gPSXCD_loopvol;             // Volume to playback cd audio when looping around again
static CdlATV   gPSXCD_cdatv;               // The volume mixing levels of cd audio sent to the SPU for output
static int32_t  gPSXCD_playfadeuptime;      // If > 0 then fade up cd audio volume in this amount of time (MS) when beginning playback
static int32_t  gPSXCD_loopfadeuptime;      // Same as the regular 'fade up time', but used when we loop
static int32_t  gPSXCD_looptrack;           // What track to loop after the current one ends
static int32_t  gPSXCD_loopsectoroffset;    // The sector offset to begin the loop track at

// Number of sectors read audio mode
static int32_t  gPSXCD_playcount;

// CD commands issued and results
static CdlCmd   gPSXCD_cdl_com = CdlPause;      // The last command issued to the cdrom via 'LIBCD_CdControl'
static CdlCmd   gPSXCD_cdl_err_com;             // The last command issued to the cdrom via 'LIBCD_CdControl' which was an error
static uint8_t  gPSXCD_cd_param[8];             // Parameters for the last command issued to the cdrom via 'LIBCD_CdControl' (if there were parameters)
static int32_t  gPSXCD_cdl_intr;                // Int result of the last read command issued to the cdrom
static int32_t  gPSXCD_check_intr;              // Int result of the last 'LIBCD_CdSync' call when querying the status of something
static int32_t  gPSXCD_cdl_err_intr;            // Int result of the last 'LIBCD_CdSync' call with an error
static uint8_t  gPSXCD_check_result[8];         // Result bytes for the last 'LIBCD_CdSync' call when querying the status of something
static int32_t  gPSXCD_cdl_err_count;           // A count of how many cd errors that occurred
static uint8_t  gPSXCD_cdl_stat;                // The first result byte (status byte) for the last read command
static uint8_t  gPSXCD_cdl_err_stat;            // The first result byte (status byte) for when the last error which occurred

// Previous 'CdReadyCallback' and 'CDSyncCallback' functions used by LIBCD prior to initializing this module.
// Used for restoring once we shutdown this module.
static CdlCB gPSXCD_cbsyncsave;
static CdlCB gPSXCD_cbreadysave;

#if PSYDOOM_MODS
    static DiscReader   gDiscReaders[MAX_OPEN_FILES] = { PsxVm::gDiscInfo, PsxVm::gDiscInfo, PsxVm::gDiscInfo, PsxVm::gDiscInfo };
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Callback invoked by the PsyQ libraries when a command to the CDROM is complete
//------------------------------------------------------------------------------------------------------------------------------------------
void PSXCD_cbcomplete(const CdlSyncStatus status, const uint8_t pResult[8]) noexcept {
    if (!gbPSXCD_cb_enable_flag)
        return;

    // Did the command complete OK?
    if (status == CdlComplete) {
        if (gPSXCD_cdl_com == CdlSeekP) {
            // Just finished a seek
            gbPSXCD_seeking_for_play = false;

            // Intending to play cd music? If so then start mixing in the audio and begin fading (if required)
            if (gbPSXCD_playflag) {
                psxspu_setcdmixon();

                if (gPSXCD_playfadeuptime == 0) {
                    psxspu_set_cd_vol(gPSXCD_playvol);
                } else {
                    psxspu_set_cd_vol(0);
                    psxspu_start_cd_fade(gPSXCD_playfadeuptime, gPSXCD_playvol);
                    gPSXCD_playfadeuptime = 0;
                }

                // Begin playback of the cd audio
                gPSXCD_cdl_com = CdlPlay;
                LIBCD_CdControlF(CdlPlay, nullptr);
            }
        } else if (gPSXCD_cdl_com == CdlPause) {
            // Just finished a pause
            gbPSXCD_waiting_for_pause = false;
        }
    } else {
        // An error happened - record the details
        gPSXCD_cdl_err_count++;
        gPSXCD_cdl_err_intr = status + 10;      // '+': Just to make the codes more unique, so their source is known
        gPSXCD_cdl_err_com = gPSXCD_cdl_com;
        gPSXCD_cdl_err_stat = pResult[0];
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Callback invoked by the PsyQ SDK (and ultimately by the hardware) for when we are finished reading a cd sector.
// PsyDoom: this function has been cut down somewhat, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void PSXCD_cbready(const CdlSyncStatus status, const uint8_t pResult[8]) noexcept {
    // Are callbacks disabled currently? If so then ignore...
    if (!gbPSXCD_cb_enable_flag)
        return;
    
    // Save the status bits and int results for the command
    gPSXCD_cdl_stat = pResult[0];
    gPSXCD_cdl_intr = status;
    
    if (pResult[0] & CdlStatPlay) {
        // If we are playing cd audio and there is new data ready then record the updated cd location
        if (status == CdlDataReady) {
            // I'm not sure what the hell this flag is - I couldn't find any mention of it anywhere.
            // This bit shouldn't be set even when the encoding is BCD because second count shouldn't exceed 60?
            if ((pResult[4] & 0x80) == 0) {
                gPSXCD_playcount++;
                gPSXCD_newloc.minute = pResult[3];
                gPSXCD_newloc.second = pResult[4];
                gPSXCD_newloc.sector = pResult[5];
                gPSXCD_newloc.track = (pResult[1] >> 4) * 10 + (pResult[1] & 0x0F);     // Convert from BCD to binary
            }

            return;
        }
        
        // If we have reached the end then we might want to loop around again
        if (status == CdlDataEnd) {
            // Clear the command and location if we have reached the end
            gPSXCD_cdl_com = CdlNop;
            gPSXCD_lastloc = 0;

            // Are we to loop and play again?
            if (gbPSXCD_playflag || gbPSXCD_loopflag) {
                psxcd_play_at_andloop(
                    gPSXCD_looptrack,
                    gPSXCD_loopvol,
                    gPSXCD_loopsectoroffset,
                    gPSXCD_loopfadeuptime,
                    gPSXCD_looptrack,
                    gPSXCD_loopvol,
                    gPSXCD_loopsectoroffset,
                    gPSXCD_loopfadeuptime
                );
            }

            return;
        }
        
        // Error or unknown situation:
        gPSXCD_cdl_err_intr = status + 20;      // '+': Just to make the codes more unique, so their source is known
        gPSXCD_cdl_err_com = gPSXCD_cdl_com;
    }
    else {
        // Error or unknown situation:
        gPSXCD_cdl_err_intr = status + 30;      // '+': Just to make the codes more unique, so their source is known
        gPSXCD_cdl_err_com = gPSXCD_cdl_com;
    }

    // This point is only reached if there is an error!
    gPSXCD_cdl_err_count++;
    gPSXCD_cdl_err_stat = pResult[0];
    gPSXCD_cdl_com = gPSXCD_cdl_err_com;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Disable/ignore internal cd related callbacks
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_disable_callbacks() noexcept {
    gbPSXCD_cb_enable_flag = false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Enable internal cd related callbacks
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_enable_callbacks() noexcept {
    gbPSXCD_cb_enable_flag = true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the WESS (Williams Entertainment Sound System) CD handling module.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_init() noexcept {
    // If we've already done this then just no-op
    if (gbPSXCD_IsCdInit)
        return;

    // Initialize LIBCD
    LIBCD_CdInit();
    gbPSXCD_IsCdInit = true;

    // Clear the locations for all CD tracks
    for (int32_t trackIdx = 0; trackIdx < CdlMAXTOC; ++trackIdx) {
        CdlLOC& trackpos = gTrackCdlLOC[trackIdx];
        trackpos.minute = 0;
        trackpos.second = 0;
        trackpos.sector = 0;
        trackpos.track = 0;
    }

    // Initialize the SPU
    psxspu_init();

    // Get the locations of all CD tracks
    const int32_t trackCount = LIBCD_CdGetToc(gTrackCdlLOC);

    if (trackCount != 0) {
        // Set command complete and data callbacks and save the old ones for later restoring
        gPSXCD_cbsyncsave = LIBCD_CdSyncCallback(PSXCD_cbcomplete);
        gPSXCD_cbreadysave = LIBCD_CdReadyCallback(PSXCD_cbready);

        psxcd_enable_callbacks();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shut down the WESS (Williams Entertainment Sound System) CD handling module
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_exit() noexcept {
    // Restore these previous callbacks
    LIBCD_CdSyncCallback(gPSXCD_cbsyncsave);
    LIBCD_CdReadyCallback(gPSXCD_cbreadysave);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Switches the cdrom drive into data reading mode.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_set_data_mode() noexcept {
    if (gPSXCD_psxcd_mode != 1) {
        // Currently in audio mode: begin fading out audio and clear playback flags
        gbPSXCD_playflag = false;
        gbPSXCD_loopflag = false;
        gbPSXCD_seeking_for_play = false;

        const int32_t cdVol = psxspu_get_cd_vol();

        if (cdVol != 0) {
            psxspu_start_cd_fade(FADE_TIME_MS, 0);
            Utils::waitForCdAudioFadeOut();
        }

        // Disable cd audio mixing and mark us in 'data' mode
        psxspu_setcdmixoff();
        gPSXCD_psxcd_mode = 1;
    }
    
    // Stop the cdrom at the current location when we are done to finish up
    gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControl(CdlPause, nullptr, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open a specified CD file for reading.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
PsxCd_File* psxcd_open(const CdFileId discFile) noexcept {
    // Zero init the temporary file structure and sanity check the file is valid!
    gPSXCD_cdfile = {};

    if (((int32_t) discFile < 0) || (discFile >= CdFileId::END)) {
        FatalErrors::raise("psxcd_open: invalid file specified!");
    }

    // Modding mechanism: allow files to be overriden with user files in a specified directory.
    if (ModMgr::areOverridesAvailableForFile(discFile))
        return (ModMgr::openOverridenFile(discFile, gPSXCD_cdfile)) ? &gPSXCD_cdfile : nullptr;

    // Find a free disc reader slot to accomodate this file
    int32_t discReaderIdx = -1;

    for (int32_t i = 0; i < MAX_OPEN_FILES; ++i) {
        if (!gDiscReaders[i].isTrackOpen()) {
            discReaderIdx = i;
            break;
        }
    }
    
    if (discReaderIdx < 0) {
        FatalErrors::raise("psxcd_open: out of file handles!");
    }

    // Figure out where the file is on disc, open up the disc reader for it and save it's details
    const PsxCd_MapTblEntry& fileTableEntry = gCdMapTbl[(uint32_t) discFile];
    DiscReader& discReader = gDiscReaders[discReaderIdx];

    if (!discReader.setTrackNum(1)) {
        FatalErrors::raise("psxcd_open: failed to open a disc reader for the data track!");
    }

    if (!discReader.trackSeekAbs(fileTableEntry.startSector * CD_SECTOR_SIZE)) {
        FatalErrors::raise("psxcd_open: failed to seek to the specified file!");
    }

    gPSXCD_cdfile.size = fileTableEntry.size;
    gPSXCD_cdfile.startSector = fileTableEntry.startSector;
    gPSXCD_cdfile.fileHandle = discReaderIdx + 1;
    return &gPSXCD_cdfile;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the cdrom is currently seeking to a location for audio playback.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_seeking_for_play() noexcept { return gbPSXCD_seeking_for_play; }

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the cdrom is currently in the process of pausing.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_waiting_for_pause() noexcept { return gbPSXCD_waiting_for_pause; }

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the specified number of bytes synchronously from the given CD file and returns the number of bytes read.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_read(void* const pDest, int32_t numBytes, PsxCd_File& file) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    if (ModMgr::isFileOverriden(file))
        return ModMgr::readFromOverridenFile(pDest, numBytes, file);

    // If the file does not have a valid handle then the read fails
    if ((file.fileHandle <= 0) || (file.fileHandle > MAX_OPEN_FILES))
        return -1;

    // Verify that the read is in bounds for the file and fail if it isn't
    DiscReader& reader = gDiscReaders[file.fileHandle - 1];

    const int32_t fileBegByteIdx = file.startSector * CD_SECTOR_SIZE;
    const int32_t fileEndByteIdx = fileBegByteIdx + file.size;
    const int32_t curByteIdx = reader.tell();

    if ((curByteIdx < fileBegByteIdx) || (curByteIdx + numBytes > fileEndByteIdx))
        return -1;

    // Do the actual read and return the number of bytes read
    return (reader.read(pDest, numBytes)) ? numBytes : -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek to a specified position in a file, relatively or absolutely.
// Returns '0' on success, any other value on failure.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_seek(PsxCd_File& file, int32_t offset, const PsxCd_SeekMode mode) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    if (ModMgr::isFileOverriden(file))
        return ModMgr::seekForOverridenFile(file, offset, mode);

    // If the file handle is invalid then the seek fails
    if ((file.fileHandle <= 0) || (file.fileHandle > MAX_OPEN_FILES))
        return -1;

    DiscReader& reader = gDiscReaders[file.fileHandle - 1];

    if (mode == PsxCd_SeekMode::SET) {
        // Seek to an absolute position in the file: make sure the offset is valid and try to go to it
        if ((offset < 0) || (offset > file.size))
            return -1;

        return (reader.trackSeekAbs(file.startSector * CD_SECTOR_SIZE + offset)) ? 0 : -1;
    }
    else if (mode == PsxCd_SeekMode::CUR) {
        // Seek relative to the current IO position: make sure the offset is valid and try to go to it
        const int32_t curOffset = reader.tell() - file.startSector * CD_SECTOR_SIZE;
        const int32_t newOffset = curOffset + offset;

        if ((newOffset < 0) || (newOffset > file.size))
            return -1;

        return (reader.trackSeekRel(offset)) ? 0 : -1;
    }
    else if (mode == PsxCd_SeekMode::END) {
        // Seek relative to the end: make sure the offset is valid and try to go to it
        const int32_t newOffset = file.size - offset;

        if ((newOffset < 0) || (newOffset > file.size))
            return -1;

        return (reader.trackSeekAbs(file.startSector * CD_SECTOR_SIZE + newOffset)) ? 0 : -1;
    }

    return -1;  // Bad seek mode!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the current IO offset within the given file.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_tell(const PsxCd_File& file) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    if (ModMgr::isFileOverriden(file))
        return ModMgr::tellForOverridenFile(file);

    // If the file handle is invalid then the tell fails
    if ((file.fileHandle <= 0) || (file.fileHandle > MAX_OPEN_FILES))
        return -1;

    // Tell where we are in the file
    DiscReader& reader = gDiscReaders[file.fileHandle - 1];
    return reader.tell() - file.startSector * CD_SECTOR_SIZE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Close a CD file and free up the file slot.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_close([[maybe_unused]] PsxCd_File& file) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    if (ModMgr::isFileOverriden(file)) {
        ModMgr::closeOverridenFile(file);
        return;
    }

    // If it's a file on the game CD then close out any open disc readers it has and then zero the struct
    if ((file.fileHandle > 0) && (file.fileHandle <= MAX_OPEN_FILES)) {
        gDiscReaders[file.fileHandle - 1].closeTrack();
    }

    file = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Switches the cdrom into audio playback mode if in data mode.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_set_audio_mode() noexcept {
    if (gPSXCD_psxcd_mode != 0) {
        // In data mode: switch to audio mode
        gPSXCD_cd_param[0] = CdlModeRept | CdlModeAP | CdlModeDA;
        gPSXCD_cdl_com = CdlSetmode;
        LIBCD_CdControl(CdlSetmode, gPSXCD_cd_param, nullptr);
        gPSXCD_psxcd_mode = 0;

        // Pause the disc drive
        gPSXCD_cdl_com = CdlPause;
        LIBCD_CdControl(CdlPause, nullptr, nullptr);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set the volume to playback cd audio when looping around again
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_set_loop_volume(const int32_t vol) noexcept {
    gPSXCD_loopvol = vol;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the given cd track and loop another track afterwards using the specified parameters.
//
//  track:              The track to play
//  vol:                Track volume
//  sectorOffset:       To start past the normal track start
//  fadeUpTime:         Milliseconds to fade in the track, or '0' if instant play
//  loopTrack:          What track to play in loop after this track ends
//  loopVol:            What volume to play that looped track at
//  loopSectorOffest:   What sector offset to use for the looped track
//  loopFadeUpTime:     Fade up time for the looped track
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_play_at_andloop(
    const int32_t track,
    const int32_t vol,
    const int32_t sectorOffset,
    const int32_t fadeUpTime,
    const int32_t loopTrack,
    const int32_t loopVol,
    const int32_t loopSectorOffest,
    const int32_t loopFadeUpTime
) noexcept {
    // PsyDoom: ignore command in headless mode
    #if PSYDOOM_MODS
        if (ProgArgs::gbHeadlessMode)
            return;
    #endif

    // If this is an invalid track then do nothing
    const CdlLOC& trackLoc = gTrackCdlLOC[track];

    if (trackLoc == 0)
        return;

    // Ensure we are in audio mode before beginning playback
    psxcd_set_audio_mode();

    // Figure out the position to begin playback from
    const int32_t trackBegSector = LIBCD_CdPosToInt(trackLoc);
    LIBCD_CdIntToPos(trackBegSector + sectorOffset, gPSXCD_cdloc);

    // Init playback stats
    gbPSXCD_playflag = true;
    gbPSXCD_loopflag = true;
    gPSXCD_playvol = vol;
    gPSXCD_playfadeuptime = fadeUpTime;
    gPSXCD_playcount = 0;
    
    // Save the parameters for when we loop
    gPSXCD_looptrack = loopTrack;
    gPSXCD_loopvol = loopVol;
    gPSXCD_loopsectoroffset = loopSectorOffest;
    gPSXCD_loopfadeuptime = loopFadeUpTime;

    // Seek to the track start
    gbPSXCD_seeking_for_play = true;
    gPSXCD_cdl_com = CdlSeekP;
    LIBCD_CdControlF(CdlSeekP, (const uint8_t*) &gPSXCD_cdloc);

    // Remember the track start location as the begin, current and last valid intended location.
    //
    // This looks like a BUG in the original code to me? Should it not be saving 'gPSXCD_cdloc'?
    // Otherwise the sector offset specified is forgotten about...
    gPSXCD_newloc = trackLoc;
    gPSXCD_lastloc = trackLoc;
    gPSXCD_beginloc = trackLoc;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begin playint the specified cd track at the given volume level.
// A sector offset can also be specified to begin from a certain location in the track.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_play_at(const int32_t track, const int32_t vol, const int32_t sectorOffset) noexcept {
    // PsyDoom: ignore command in headless mode
    #if PSYDOOM_MODS
        if (ProgArgs::gbHeadlessMode)
            return;
    #endif

    // If this is an invalid track then do nothing
    const CdlLOC& trackLoc = gTrackCdlLOC[track];

    if (trackLoc == 0)
        return;
    
    // Ensure we are in audio mode before beginning playback
    gbPSXCD_playflag = false;
    gbPSXCD_loopflag = false;
    gbPSXCD_seeking_for_play = false;
    psxcd_set_audio_mode();

    // Figure out the position to begin playback from
    const int32_t trackBegSector = LIBCD_CdPosToInt(trackLoc);
    LIBCD_CdIntToPos(trackBegSector + sectorOffset, gPSXCD_cdloc);

    // Init some playback stats and begin seeking to the specified track location
    gbPSXCD_playflag = true;
    gPSXCD_playvol = vol;
    gbPSXCD_seeking_for_play = true;
    gPSXCD_playcount = 0;
    gbPSXCD_loopflag = false;       // Don't loop
    gPSXCD_playfadeuptime = 0;      // Don't fade in

    gPSXCD_cdl_com = CdlSeekP;
    LIBCD_CdControlF(CdlSeekP, (const uint8_t*) &gPSXCD_cdloc);

    // Remember the track start location as the begin, current and last valid intended location.
    //
    // This looks like a BUG in the original code to me? Should it not be saving 'gPSXCD_cdloc'?
    // Otherwise the sector offset specified is forgotten about...
    gPSXCD_newloc = trackLoc;
    gPSXCD_lastloc = trackLoc;
    gPSXCD_beginloc = trackLoc;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the given audio track at the specified volume level
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_play(const int32_t track, const int32_t vol) noexcept {
    psxcd_play_at(track, vol, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek to the specified offset within a cd audio track in preparation for playback
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_seek_for_play_at(const int32_t track, const int32_t sectorOffset) noexcept {
    // PsyDoom: ignore command in headless mode
    #if PSYDOOM_MODS
        if (ProgArgs::gbHeadlessMode)
            return;
    #endif

    // If this is an invalid track then do nothing
    const CdlLOC& trackLoc = gTrackCdlLOC[track];

    if (trackLoc == 0)
        return;

    // Ensure we are in audio mode before doing the seek
    gbPSXCD_playflag = false;
    gbPSXCD_loopflag = false;
    gbPSXCD_seeking_for_play = false;
    psxcd_set_audio_mode();

    // Figure out the position to go to
    const uint32_t trackBegSector = LIBCD_CdPosToInt(trackLoc);
    LIBCD_CdIntToPos(trackBegSector + sectorOffset, gPSXCD_cdloc);

    // Init some playback stats and begin seeking to the specified track location
    gbPSXCD_playflag = false;
    gPSXCD_playcount = 0;
    gbPSXCD_loopflag = false;
    gbPSXCD_seeking_for_play = true;

    gPSXCD_cdl_com = CdlSeekP;
    LIBCD_CdControlF(CdlSeekP, (const uint8_t*) &gPSXCD_cdloc);

    // Remember the track start location as the begin, current and last valid intended location.
    //
    // This looks like a BUG in the original code to me? Should it not be saving 'gPSXCD_cdloc'?
    // Otherwise the sector offset specified is forgotten about...
    gPSXCD_newloc = trackLoc;
    gPSXCD_lastloc = trackLoc;
    gPSXCD_beginloc = trackLoc;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Seek to the beginning of a cd audio track in preparation for playback
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_seek_for_play(const int32_t track) noexcept {
    psxcd_seek_for_play_at(track, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if cd audio is currently playing, returning 'true' if that is the case
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_play_status() noexcept {
    // Was the last command to play or to seek to a physical (audio) location?
    // If not then we cannot be playing:
    if ((gPSXCD_cdl_com == CdlPlay) || (gPSXCD_cdl_com == CdlSeekP)) {
        gPSXCD_check_intr = LIBCD_CdSync(1, gPSXCD_check_result);

        // Make sure playback is ok: if there was an error or the motor stopped then stop current commands and record the error
        if ((gPSXCD_check_intr == CdlDiskError) || ((gPSXCD_check_result[0] & CdlStatStandby) == 0)) {
            LIBCD_CdFlush();

            gPSXCD_cdl_err_count++;
            gPSXCD_cdl_err_com = gPSXCD_cdl_com;
            gPSXCD_cdl_err_intr = gPSXCD_check_intr + 90;       // '+': Just to make the codes more unique, so their source is known
            gPSXCD_cdl_err_stat = gPSXCD_check_result[0];

            return false;   // Bah... Take better care of your discs! :P
        }

        // Playing cd audio without any issues!
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop playback of cd audio.
// Unlike 'psxcd_pause' playback CANNOT be resumed by calling 'psxcd_restart' afterwards.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_stop() noexcept {
    // No longer playing anything
    gbPSXCD_playflag = false;
    gbPSXCD_loopflag = false;
    gbPSXCD_seeking_for_play = false;

    // Unlike 'psxcd_pause' DON'T remember where to resume playback from, this is a 'stop' command
    gPSXCD_lastloc = 0;

    // Quickly fade out cd audio if playing
    const int32_t startCdVol = psxspu_get_cd_vol();

    if (startCdVol != 0) {
        psxspu_start_cd_fade(FADE_TIME_MS, 0);
        Utils::waitForCdAudioFadeOut();
    }

    // Issue the pause command
    gbPSXCD_waiting_for_pause = true;
    gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControlF(CdlPause, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pause cd audio playback and make a note of where we paused at.
// PsyDoom: this function has been rewritten, for the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_pause() noexcept {
    // No longer playing anything
    gbPSXCD_playflag = false;
    gbPSXCD_seeking_for_play = false;

    // If we did not start playback of anything (never set a sector location) then we don't need to do anything else
    if (gPSXCD_lastloc == 0)
        return;

    // Save the last location we read from so we can restore in 'psxcd_restart'
    gPSXCD_lastloc = gPSXCD_newloc;

    // Quickly fade out cd audio if playing
    const int32_t startCdVol = psxspu_get_cd_vol();

    if (startCdVol != 0) {
        psxspu_start_cd_fade(FADE_TIME_MS, 0);
        Utils::waitForCdAudioFadeOut();
    }
    
    // Issue the pause command
    gbPSXCD_waiting_for_pause = true;
    gPSXCD_cdl_com = CdlPause;
    LIBCD_CdControlF(CdlPause, nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Restart cd audio playback: playback resumes from where the cd was last paused
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_restart(const int32_t vol) noexcept {
    // Only do the restart if we had paused previously (have a saved sector position)
    if (gPSXCD_lastloc == 0)
        return;

    // Switch to audio mode
    psxcd_set_audio_mode();

    // Reset all these variables
    gPSXCD_cdloc = gPSXCD_lastloc;
    gbPSXCD_playflag = true;
    gbPSXCD_seeking_for_play = true;
    gPSXCD_playcount = 0;
    gPSXCD_playvol = vol;

    // Seek to the last intended cd location
    gPSXCD_cdl_com = CdlSeekP;
    LIBCD_CdControlF(CdlSeekP, (const uint8_t*) &gPSXCD_cdloc);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells how many sectors have elapsed during cd playback
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_elapsed_sectors() noexcept {
    // If we haven't started playback then no sectors are elapsed
    if (gPSXCD_beginloc == 0)
        return 0;

    // Return the difference between the current and the track start sector
    const int32_t begSector = LIBCD_CdPosToInt(gPSXCD_beginloc);
    const int32_t curSector = LIBCD_CdPosToInt(gPSXCD_newloc);
    return curSector - begSector;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set whether cd audio is mixed as stereo or mono
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_set_stereo(const bool bStereo) noexcept {
    if (bStereo) {
        gPSXCD_cdatv.l_to_l = 127;
        gPSXCD_cdatv.l_to_r = 0;
        gPSXCD_cdatv.r_to_r = 127;
        gPSXCD_cdatv.r_to_l = 0;
    } else {
        gPSXCD_cdatv.l_to_l = 63;
        gPSXCD_cdatv.l_to_r = 63;
        gPSXCD_cdatv.r_to_r = 63;
        gPSXCD_cdatv.r_to_l = 63;
    }

    LIBCD_CdMix(gPSXCD_cdatv);
}

#if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the size of a file
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_get_file_size(const CdFileId discFile) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    if (ModMgr::areOverridesAvailableForFile(discFile))
        return ModMgr::getOverridenFileSize(discFile);

    return gCdMapTbl[(int32_t) discFile].size;
}

#endif  // #if PSYDOOM_MODS
