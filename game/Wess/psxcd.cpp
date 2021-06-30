//------------------------------------------------------------------------------------------------------------------------------------------
// Williams Entertainment Sound System (WESS): PlayStation CD-ROM handling utilities
//
// Note: this module has been almost completely rewritten for PsyDoom due to massive differences in how file I/O and CD audio playback work
// for this port. A lot of the old PsyQ 'LIBCD' code simply did not make sense anymore, hence this is all marked as 'PsyDoom modifications'.
// For the original code consult the version of this file in the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "psxcd.h"

#include "Asserts.h"
#include "FatalErrors.h"
#include "psxspu.h"
#include "PsyDoom/DiscInfo.h"
#include "PsyDoom/DiscReader.h"
#include "PsyDoom/ModMgr.h"
#include "PsyDoom/ProgArgs.h"
#include "PsyDoom/PsxVm.h"
#include "PsyDoom/Utils.h"
#include "Spu.h"

#include <mutex>

static constexpr int32_t MAX_OPEN_FILES     = 4;        // Maximum number of open files
static constexpr int32_t FADE_TIME_MS       = 250;      // Time it takes to fade out CD audio (milliseconds)
static constexpr int32_t CDDA_SECTOR_SIZE   = 2352;     // Size of of a CD digital audio sector

// If true then the 'psxcd' module has been initialized
static bool gbPSXCD_IsCdInit;

// Used to hold a file temporarily after opening
static PsxCd_File gPSXCD_cdfile;

// CD audio playback related state.
// Access to all of this is controlled by the CD player mutex.
static struct {
    DiscReader  discReader          = { PsxVm::gDiscInfo };     // The disc reader used to stream the audio
    bool        bPlay               = false;                    // If 'false' then playback is either paused or stopped (stopped if the disc reader doesn't have a track)
    bool        bLoop               = false;                    // If 'true' then playback is looped upon reaching the end
    int32_t     bufferOffset        = 0;                        // Where we are in the audio buffer
    int32_t     loopTrack           = 0;                        // The track to play when looping
    int32_t     loopSectorOffset    = 0;                        // Offset (in sectors) to start at in the track when looping

    // The CD audio buffer: we read CD audio in chunks
    int16_t buffer[CDDA_SECTOR_SIZE / sizeof(int16_t)];
} gCdPlayer;

// The lock for the CD player and a helper to lock/unlock via RAII.
// N.B: this *CANNOT* be held the same time as the SPU lock, otherwise deadlock MIGHT occur!
// The SPU can request audio from the CD and thus needs access to the cd player lock also.
static std::recursive_mutex gCdPlayerMutex;

struct LockCdPlayer {
    LockCdPlayer() noexcept { gCdPlayerMutex.lock(); }
    ~LockCdPlayer() noexcept { gCdPlayerMutex.unlock(); }
};

// Disc readers used for each open file
static DiscReader gFileDiscReaders[MAX_OPEN_FILES] = { PsxVm::gDiscInfo, PsxVm::gDiscInfo, PsxVm::gDiscInfo, PsxVm::gDiscInfo };

//------------------------------------------------------------------------------------------------------------------------------------------
// A callback invoked by the SPU when it wants audio from the CD player - returns a single sample.
//------------------------------------------------------------------------------------------------------------------------------------------
static Spu::StereoSample SpuAudioCallback([[maybe_unused]] void* pUserData) noexcept {
    // Lock the CD player while we are doing this.
    // Note that this thread also has the SPU lock at this point too.
    // Therefore the main thread must NOT lock both the CD player and the SPU at the same time, or otherwise a deadlock might occur.
    LockCdPlayer cdPlayerLock;

    // If the CD player is not currently active then return silence
    if ((!gCdPlayer.bPlay) || (!gCdPlayer.discReader.isTrackOpen()))
        return Spu::StereoSample{};

    // Check if we have any data left in the buffer firstly
    constexpr int16_t SAMPLE_SIZE = sizeof(int16_t);
    constexpr int32_t NUM_BUFFER_SAMPLES = sizeof(gCdPlayer.buffer) / SAMPLE_SIZE;
    static_assert(NUM_BUFFER_SAMPLES % 2 == 0);

    DiscReader& disc = gCdPlayer.discReader;

    if (gCdPlayer.bufferOffset + 1 >= NUM_BUFFER_SAMPLES) {
        // Get the size of the track and where we are at in it
        const DiscTrack* pTrack = disc.getOpenTrack();
        int32_t trackSize = pTrack->trackPayloadSize;
        int32_t trackOffset = disc.tell();

        // See if there is any data left in the track to read
        if (trackOffset >= trackSize) {
            // We reached the end, do we loop back around again?
            if (gCdPlayer.bLoop) {
                // Looping: rewind back to the start plus any additional offset.
                // Change tracks also if we need to.
                if (disc.getTrackNum() != gCdPlayer.loopTrack) {
                    disc.setTrackNum(gCdPlayer.loopTrack);

                    // Need to re-fetch this info when changing tracks
                    pTrack = disc.getOpenTrack();
                    trackSize = pTrack->trackPayloadSize;
                }

                if (gCdPlayer.loopSectorOffset > 0) {
                    disc.trackSeekAbs(CDDA_SECTOR_SIZE * gCdPlayer.loopSectorOffset);
                } else {
                    disc.trackSeekAbs(0);
                }

                trackOffset = disc.tell();
            }
            else {
                // No looping, mark the CD player as no longer playing and return an empty sample
                gCdPlayer.bPlay = false;
                return Spu::StereoSample{};
            }
        }

        // Read what we can and zero anything we can't (in case the last sector is short for some reason)
        const int32_t samplesToRead = std::min<int32_t>((trackSize - trackOffset) / SAMPLE_SIZE, NUM_BUFFER_SAMPLES);
        const int32_t samplesToZero = NUM_BUFFER_SAMPLES - samplesToRead;
        disc.read(gCdPlayer.buffer, samplesToRead * SAMPLE_SIZE);

        if (samplesToZero > 0) {
            std::memset(gCdPlayer.buffer + samplesToRead * SAMPLE_SIZE, 0, (size_t) samplesToZero * SAMPLE_SIZE);
        }

        gCdPlayer.bufferOffset = 0;
    }

    // Should have samples in the buffer at this point, return the requested samples
    ASSERT(gCdPlayer.bufferOffset + 2 <= NUM_BUFFER_SAMPLES);

    Spu::StereoSample sample = { gCdPlayer.buffer[gCdPlayer.bufferOffset], gCdPlayer.buffer[gCdPlayer.bufferOffset + 1] };
    gCdPlayer.bufferOffset += 2;
    return sample;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize the WESS (Williams Entertainment Sound System) CD handling module.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_init() noexcept {
    // If we've already done this then just no-op
    if (gbPSXCD_IsCdInit)
        return;

    gbPSXCD_IsCdInit = true;

    // Initialize the SPU and install the CD player as an external input to the SPU
    psxspu_init();

    {
        PsxVm::LockSpu spuLock;
        PsxVm::gSpu.pExtInputCallback = SpuAudioCallback;
        PsxVm::gSpu.pExtInputUserData = nullptr;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shut down the WESS (Williams Entertainment Sound System) CD handling module
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_exit() noexcept {
    // Uninstall the CD player as an external input to the SPU
    {
        PsxVm::LockSpu spuLock;
        PsxVm::gSpu.pExtInputCallback = nullptr;
        PsxVm::gSpu.pExtInputUserData = nullptr;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Open a specified CD file for reading
//------------------------------------------------------------------------------------------------------------------------------------------
PsxCd_File* psxcd_open(const CdFileId discFile) noexcept {
    // Zero init the temporary file structure
    gPSXCD_cdfile = {};

    // Modding mechanism: allow files to be overridden with user files in a specified directory.
    // Note that we do this check BEFORE validating if the file exists on-disc because PsyDoom now allows Doom format maps (.WAD)
    // to override maps in Final Doom (.ROM files). The .WAD map files of course won't exist on-disc in the case of Final Doom.
    if (ModMgr::areOverridesAvailableForFile(discFile))
        return (ModMgr::openOverridenFile(discFile, gPSXCD_cdfile)) ? &gPSXCD_cdfile : nullptr;

    // Figure out where the file is on disc and sanity check the file is valid
    const PsxCd_MapTblEntry fileTableEntry = CdMapTbl_GetEntry(discFile);

    if (fileTableEntry == PsxCd_MapTblEntry{}) {
        FatalErrors::raise("psxcd_open: invalid file specified!");
    }

    // Find a free disc reader slot to accomodate this file
    int32_t discReaderIdx = -1;

    for (int32_t i = 0; i < MAX_OPEN_FILES; ++i) {
        if (!gFileDiscReaders[i].isTrackOpen()) {
            discReaderIdx = i;
            break;
        }
    }

    if (discReaderIdx < 0) {
        FatalErrors::raise("psxcd_open: out of file handles!");
    }

    // Open up the disc reader for it and save it's details
    DiscReader& discReader = gFileDiscReaders[discReaderIdx];

    if (!discReader.setTrackNum(1)) {
        FatalErrors::raise("psxcd_open: failed to open a disc reader for the data track!");
    }

    if (!discReader.trackSeekAbs(fileTableEntry.startSector * CDROM_SECTOR_SIZE)) {
        FatalErrors::raise("psxcd_open: failed to seek to the specified file!");
    }

    gPSXCD_cdfile.size = fileTableEntry.size;
    gPSXCD_cdfile.startSector = fileTableEntry.startSector;
    gPSXCD_cdfile.fileHandle = discReaderIdx + 1;
    return &gPSXCD_cdfile;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the cdrom is currently seeking to a location for audio playback; the answer for this is always 'false' now for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
bool psxcd_seeking_for_play() noexcept { return false; }

//------------------------------------------------------------------------------------------------------------------------------------------
// Read the specified number of bytes synchronously from the given CD file and returns the number of bytes read
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_read(void* const pDest, int32_t numBytes, PsxCd_File& file) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    if (ModMgr::isFileOverriden(file))
        return ModMgr::readFromOverridenFile(pDest, numBytes, file);

    // If the file does not have a valid handle then the read fails
    if ((file.fileHandle <= 0) || (file.fileHandle > MAX_OPEN_FILES))
        return -1;

    // Verify that the read is in bounds for the file and fail if it isn't
    DiscReader& reader = gFileDiscReaders[file.fileHandle - 1];

    const int32_t fileBegByteIdx = file.startSector * CDROM_SECTOR_SIZE;
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
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_seek(PsxCd_File& file, int32_t offset, const PsxCd_SeekMode mode) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    if (ModMgr::isFileOverriden(file))
        return ModMgr::seekForOverridenFile(file, offset, mode);

    // If the file handle is invalid then the seek fails
    if ((file.fileHandle <= 0) || (file.fileHandle > MAX_OPEN_FILES))
        return -1;

    DiscReader& reader = gFileDiscReaders[file.fileHandle - 1];

    if (mode == PsxCd_SeekMode::SET) {
        // Seek to an absolute position in the file: make sure the offset is valid and try to go to it
        if ((offset < 0) || (offset > file.size))
            return -1;

        return (reader.trackSeekAbs(file.startSector * CDROM_SECTOR_SIZE + offset)) ? 0 : -1;
    }
    else if (mode == PsxCd_SeekMode::CUR) {
        // Seek relative to the current IO position: make sure the offset is valid and try to go to it
        const int32_t curOffset = reader.tell() - file.startSector * CDROM_SECTOR_SIZE;
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

        return (reader.trackSeekAbs(file.startSector * CDROM_SECTOR_SIZE + newOffset)) ? 0 : -1;
    }

    return -1;  // Bad seek mode!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the current IO offset within the given file
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_tell(const PsxCd_File& file) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    if (ModMgr::isFileOverriden(file))
        return ModMgr::tellForOverridenFile(file);

    // If the file handle is invalid then the tell fails
    if ((file.fileHandle <= 0) || (file.fileHandle > MAX_OPEN_FILES))
        return -1;

    // Tell where we are in the file
    DiscReader& reader = gFileDiscReaders[file.fileHandle - 1];
    return reader.tell() - file.startSector * CDROM_SECTOR_SIZE;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Close a CD file and free up the file slot
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_close([[maybe_unused]] PsxCd_File& file) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    if (ModMgr::isFileOverriden(file)) {
        ModMgr::closeOverridenFile(file);
        return;
    }

    // If it's a file on the game CD then close out any open disc readers it has and then zero the struct
    if ((file.fileHandle > 0) && (file.fileHandle <= MAX_OPEN_FILES)) {
        gFileDiscReaders[file.fileHandle - 1].closeTrack();
    }

    file = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Internal helper to eliminate the redundancy between 'psxcd_play_at_andloop' and 'psxcd_play_at'.
// This is a new addition for PsyDoom.
//------------------------------------------------------------------------------------------------------------------------------------------
static void psxcd_play_internal(
    const int32_t track,
    const int32_t vol,
    const int32_t sectorOffset,
    const int32_t fadeUpTime,
    const bool bLoop,
    const int32_t loopTrack,
    const int32_t loopSectorOffset
) noexcept {
    // Ignore the command in headless mode
    if (ProgArgs::gbHeadlessMode)
        return;

    // Switch to the specified track and temporarily pause: if it fails then stop and abort
    bool setTrackOk;

    {
        // N.B: don't hold this lock in the main thread at the same time as the SPU lock - otherwise deadlock might occur!
        LockCdPlayer cdPlayerLock;
        gCdPlayer.bPlay = false;
        setTrackOk = gCdPlayer.discReader.setTrackNum(track);
    }

    if (!setTrackOk) {
        psxcd_stop();
        return;
    }

    // Start mixing in CD audio set the volume level and start fading (if requested)
    psxspu_setcdmixon();

    if (fadeUpTime <= 0) {
        psxspu_set_cd_vol(vol);
        psxspu_stop_cd_fade();
    } else {
        psxspu_set_cd_vol(0);
        psxspu_start_cd_fade(fadeUpTime, vol);
    }

    // Skip the requested number of sectors
    {
        // N.B: don't hold this lock in the main thread at the same time as the SPU lock - otherwise deadlock might occur!
        LockCdPlayer cdPlayerLock;

        if (sectorOffset > 0) {
            gCdPlayer.discReader.trackSeekAbs(CDDA_SECTOR_SIZE * sectorOffset);
        }

        // Mark the player as playing and save loop parameters
        gCdPlayer.bPlay = true;
        gCdPlayer.bufferOffset = CDDA_SECTOR_SIZE / sizeof(int16_t);    // Need to read a sector
        gCdPlayer.bLoop = bLoop;
        gCdPlayer.loopTrack = loopTrack;
        gCdPlayer.loopSectorOffset = loopSectorOffset;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the given cd track and loop another track afterwards using the specified parameters.
//
//  track:              The track to play
//  vol:                Track volume
//  sectorOffset:       To start past the normal track start
//  fadeUpTime:         Milliseconds to fade in the track, or '0' if instant play.
//  loopTrack:          What track to play in loop after this track ends
//  loopVol:            What volume to play that looped track at.
//                          NOTE: This is IGNORED by PsyDoom and was always the same as the original volume in PSX DOOM.
//  loopSectorOffset:   What sector offset to use for the looped track
//  loopFadeUpTime:     Fade up time for the looped track.
//                          NOTE: This is IGNORED by PsyDoom and was always '0' in PSX Doom.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_play_at_andloop(
    const int32_t track,
    const int32_t vol,
    const int32_t sectorOffset,
    const int32_t fadeUpTime,
    const int32_t loopTrack,
    [[maybe_unused]] const int32_t loopVol,
    const int32_t loopSectorOffset,
    [[maybe_unused]] const int32_t loopFadeUpTime
) noexcept {
    // PsyDoom: to simplify threading and very messy synchronization in the CD audio callback these fields are no longer supported.
    // Setting them to values other than this will no longer work! That's OK because Doom always followed these usage patterns:
    ASSERT(loopVol == vol);
    ASSERT(loopFadeUpTime == 0);

    psxcd_play_internal(track, vol, sectorOffset, fadeUpTime, true, loopTrack, loopSectorOffset);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Begin playint the specified cd track at the given volume level.
// A sector offset can also be specified to begin from a certain location in the track.
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_play_at(const int32_t track, const int32_t vol, const int32_t sectorOffset) noexcept {
    psxcd_play_internal(track, vol, sectorOffset, 0, false, 0, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Play the given audio track at the specified volume level
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_play(const int32_t track, const int32_t vol) noexcept {
    psxcd_play_at(track, vol, 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Stop playback of cd audio; unlike 'psxcd_pause' playback CANNOT be resumed by calling 'psxcd_restart' afterwards
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_stop() noexcept {
    // Quickly fade out cd audio if playing
    bool bMightNeedFade = false;

    {
        // N.B: don't hold this lock in the main thread at the same time as the SPU lock - otherwise deadlock might occur!
        LockCdPlayer cdPlayerLock;
        bMightNeedFade = (gCdPlayer.discReader.isTrackOpen() && gCdPlayer.bPlay);
    }

    if (bMightNeedFade) {
        const int32_t startCdVol = psxspu_get_cd_vol();

        if (startCdVol != 0) {
            psxspu_start_cd_fade(FADE_TIME_MS, 0);
            Utils::waitForCdAudioFadeOut();
        }
    }

    // Close the disc and zero out everything
    {
        // N.B: don't hold this lock in the main thread at the same time as the SPU lock - otherwise deadlock might occur!
        LockCdPlayer cdPlayerLock;

        gCdPlayer.discReader.closeTrack();
        gCdPlayer.bPlay = false;
        gCdPlayer.bLoop = false;
        gCdPlayer.bufferOffset = 0;
        gCdPlayer.loopSectorOffset = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Pause cd audio playback and make a note of where we paused at
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_pause() noexcept {
    // Quickly fade out cd audio if playing
    bool bMightNeedFade = false;

    {
        // N.B: don't hold this lock in the main thread at the same time as the SPU lock - otherwise deadlock might occur!
        LockCdPlayer cdPlayerLock;
        bMightNeedFade = (gCdPlayer.discReader.isTrackOpen() && gCdPlayer.bPlay);
    }

    if (bMightNeedFade) {
        const int32_t startCdVol = psxspu_get_cd_vol();

        if (startCdVol != 0) {
            psxspu_start_cd_fade(FADE_TIME_MS, 0);
            Utils::waitForCdAudioFadeOut();
        }
    }

    // Mark as no longer playing
    {
        // N.B: don't hold this lock in the main thread at the same time as the SPU lock - otherwise deadlock might occur!
        LockCdPlayer cdPlayerLock;
        gCdPlayer.bPlay = false;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Restart cd audio playback: playback resumes from where the cd was last paused
//------------------------------------------------------------------------------------------------------------------------------------------
void psxcd_restart(const int32_t vol) noexcept {
    // Only Do this if we are actually playing a track
    {
        // N.B: don't hold this lock in the main thread at the same time as the SPU lock - otherwise deadlock might occur!
        LockCdPlayer cdPlayerLock;

        if (!gCdPlayer.discReader.isTrackOpen())
            return;

        // Begin playing again
        gCdPlayer.bPlay = true;
    }

    // Set the audio volume
    psxspu_set_cd_vol(vol);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells how many sectors have elapsed during cd playback
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_elapsed_sectors() noexcept {
    // N.B: don't hold this lock in the main thread at the same time as the SPU lock - otherwise deadlock might occur!
    LockCdPlayer cdPlayerLock;
    return (gCdPlayer.discReader.isTrackOpen()) ? gCdPlayer.discReader.tell() / CDDA_SECTOR_SIZE : 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the size of a file
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t psxcd_get_file_size(const CdFileId discFile) noexcept {
    // Modding mechanism: allow files to be overriden with user files in a specified directory
    if (ModMgr::areOverridesAvailableForFile(discFile))
        return ModMgr::getOverridenFileSize(discFile);

    return CdMapTbl_GetEntry(discFile).size;
}
