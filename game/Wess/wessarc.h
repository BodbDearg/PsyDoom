#pragma once

#include "psxcd.h"
#include "PcPsx/Types.h"

// Setting ids for sound hardware
enum SoundHardwareTags : int32_t {
    SNDHW_TAG_END               = 0,    // Marks the end of the list
    SNDHW_TAG_DRIVER_ID         = 1,    // What driver to use
    SNDHW_TAG_SOUND_EFFECTS     = 2,    // Sound effects enabled?
    SNDHW_TAG_MUSIC             = 3,    // Music enabled?
    SNDHW_TAG_DRUMS             = 4,    // Drums enabled?
    SNDHW_TAG_MAX               = 5     // Length of this list
};

// Sound driver ids
enum SoundDriverId : uint8_t {
    NoSound_ID  = 0,        // No sound driver
    PSX_ID      = 1,        // Sony PlayStation sound driver
    GENERIC_ID  = 50        // Generic hardware agnostic sound driver
};

// Sequence play mode
enum SeqPlayMode : uint8_t {
    SEQ_STATE_STOPPED   = 0,
    SEQ_STATE_PLAYING   = 1
};

// Driver & sequencer command ids.
// See the actual sequencer and driver implementations for these for more details on what each does.
// Note that some are not implemented by the driver and some are not implemented by the sequencer.
enum DriverCmd : uint8_t {
    DriverInit          = 0,
    DriverExit          = 1,
    DriverEntry1        = 2,
    DriverEntry2        = 3,
    DriverEntry3        = 4,
    TrkOff              = 5,
    TrkMute             = 6,
    PatchChg            = 7,
    PatchMod            = 8,
    PitchMod            = 9,
    ZeroMod             = 10,
    ModuMod             = 11,
    VolumeMod           = 12,
    PanMod              = 13,
    PedalMod            = 14,
    ReverbMod           = 15,
    ChorusMod           = 16,
    NoteOn              = 17,
    NoteOff             = 18,
    StatusMark          = 19,
    GateJump            = 20,
    IterJump            = 21,
    ResetGates          = 22,
    ResetIters          = 23,
    WriteIterBox        = 24,
    SeqTempo            = 25,
    SeqGosub            = 26,
    SeqJump             = 27,
    SeqRet              = 28,
    SeqEnd              = 29,
    TrkTempo            = 30,
    TrkGosub            = 31,
    TrkJump             = 32,
    TrkRet              = 33,
    TrkEnd              = 34,
    NullEvent           = 35
};

// Describes a patch/instrument: this is a collection of voices triggered in unison
struct patch {
    uint16_t    num_voices;         // How many voices to use for the patch
    uint16_t    first_voice_idx;    // Index of the first patch voice for the patch. Other patch voices follow contiguously in the voices list.
};

static_assert(sizeof(patch) == 4);

// Settings for one individual voice/sound in a patch/instrument
struct patch_voice {
    uint8_t     priority;           // Voice priority: used to determine what to kill when we're out of voices
    uint8_t     reverb;             // Appears to be unused: reverb for the voice
    uint8_t     volume;             // Volume to play the voice at
    uint8_t     pan;                // Voice pan 64 to 191 - 128 is the voice center. Values outside this range are clamped by the PSX driver.
    uint8_t     base_note;          // The 'base' note/semitone at which the sound sample is regarded to play back at 44,100 Hz (used to compute sample frequency for triggered notes)
    uint8_t     base_note_frac;     // The .8 fractional part of the base note
    uint8_t     note_min;           // Minimum semitone at which the voice can be played - does not sound below this
    uint8_t     note_max;           // Maximum semitone at which the voice can be played - does not sound above this
    uint8_t     pitchstep_down;     // How big each unit of pitch shift is when pitch shifting down (in 1/8192 units)
    uint8_t     pitchstep_up;       // How big each unit of pitch shift is when pitch shifting up (in 1/8192 units)
    uint16_t    sample_idx;         // The index of the patch sample to use for this voice
    uint16_t    adsr1;              // The first (low) 16-bits of the SPU voice envelope (see comments for 'SpuVoiceAttr' for more details)
    uint16_t    adsr2;              // The second (high) 16-bits of the SPU voice envelope (see comments for 'SpuVoiceAttr' for more details)
};

static_assert(sizeof(patch_voice) == 16);

// Holds details for a sound sample used by a patch voice which can be loaded or is already loaded
struct patch_sample {
    uint32_t    offset;         // Unused & unclear what the purpose of this is for. Increases with each patch sample in the module.
    uint32_t    size;           // The size in bytes of the sound data for this sample
    uint32_t    spu_addr;       // Where in SPU RAM the patch is currently uploaded to. Set to '0' if not uploaded to the SPU.
};

static_assert(sizeof(patch_sample) == 12);

// Special patch type used for drum tracks.
// Consists of a patch that is always played back using the same note.
struct drum_patch {
    uint16_t    patch_idx;      // Watch patch to use for this drum
    uint16_t    note;           // What note/pitch to play the drum patch at
};

static_assert(sizeof(drum_patch) == 4);

// Main header for a module (.WMD) file.
// Defines general/global info and settings for the module.
struct module_header {
    int32_t     module_id;              // Should be the string 'SPSX'
    uint32_t    module_version;         // Should be '1'
    uint16_t    num_sequences;          // How many sequences are in the module.
    uint8_t     num_patch_groups;       // How many driver specific patch groups are defined in the module
    uint8_t     max_active_sequences;   // The number of sequence work areas to allocate, or the maximum number of active sequences
    uint8_t     max_active_tracks;      // The number of track work areas to allocate, or the maximum number of active tracks
    uint8_t     max_gates_per_seq;      // Maximum number of 'gates' or on/off conditional jump switches per sequence
    uint8_t     max_iters_per_seq;      // Maximum number of iteration counts (iters) per sequence. These are used to do jumps a certain number of times.
    uint8_t     max_callbacks;          // Maximum number of user sequencer callbacks that can be used
};

static_assert(sizeof(module_header) == 16);

// Classes of sounds/voices
enum SoundClass : uint8_t {
    SNDFX_CLASS     = 0,    // Used for most game sounds
    MUSIC_CLASS     = 1,    // Used for music sounds
    DRUMS_CLASS     = 2,    // Similar to music sounds, but with more variety in which samples are used
    SFXDRUMS_CLASS  = 3     // I guess similar to sfx but setup to use lots of different samples (like drums)
};

// Basic description for a track in a sequence
struct track_header {
    SoundDriverId   driver_id;              // What sound driver the track should play with
    uint8_t         max_voices;             // Maximum number of voices that the track claims to use
    uint8_t         priority;               // Used for prioritizing voices when we are out of hardware voices
    uint8_t         lock_channel;           // Seems to be unused in this version of the WESS library: appears to be for assigning certain tracks to certain sound channels
    SoundClass      sound_class;            // What rough class of sounds the track contains
    uint8_t         init_reverb;            // The reverb level to initialize the track status with
    uint16_t        init_patch_idx;         // Which patch index to initially use for the track status unless manually overridden
    uint16_t        init_pitch_cntrl;       // What pitch shift value to initially use for the track status unless manually overridden
    uint8_t         init_volume_cntrl;      // What volume to initially use for the track status unless manually overridden
    uint8_t         init_pan_cntrl;         // What pan setting to initially use for the track status unless manually overridden
    uint8_t         loc_stack_size;         // The maximum number of track data locations that can be remembered in the track's location stack (for save + return control flow)
    uint8_t         init_mutegroups_mask;   // Used to initialize the 'mutegroups_mask' track status parameter. Those mask bits define muting groups.
    uint16_t        init_ppq;               // Parts per quarter note: how many parts to subdivide each quarter note into for timing purposes. Affects timing precision.
    uint16_t        init_qpm;               // The quarter notes per minute (beats per minute) value to initialize the track status with, unless manually overridden.
    uint16_t        num_labels;             // How many jump/position labels the track defines
    uint32_t        cmd_stream_size;        // The size of the stream containing sequencer commands and command timings for the track
};

static_assert(sizeof(track_header) == 24);

// Holds all of the read-only data for a track in a music sequence
struct track_data {
    track_header        hdr;            // Header for the track
    VmPtr<uint32_t>     plabels;        // A list of offsets into track data: used by control flow (i.e jump) sequencer commands
    VmPtr<uint8_t>      pcmd_stream;    // The actual byte stream containing sequencer commands for the track and delta timings in between them
};

static_assert(sizeof(track_data) == 32);

// Basic information for a sequence
struct sequence_header {
    uint16_t    num_tracks;     // How many tracks are in the sequence
    uint16_t    _unused;        // Field appears to be unused - it's purpose cannot be determined because of that
};

static_assert(sizeof(sequence_header) == 4);

// Holds all of the read-only data for a music sequence
struct sequence_data {
    sequence_header     hdr;                // General info for the sequence: always remains loaded in memory
    VmPtr<track_data>   ptracks;            // The loaded list of tracks for the sequence: loaded and unloaded on demand for music tracks (to save memory)
    uint32_t            modfile_offset;     // The offset of this sequence's data in the module file
    uint32_t            track_data_size;    // Total size of all the loaded tracks in the sequence (some tracks may be skipped)
    uint32_t            num_tracks;         // Total number of loaded tracks in the sequence (some tracks may be skipped)
};

static_assert(sizeof(sequence_data) == 20);

// Contains the header for the module and the read-only data for all loaded sequences and tracks
struct module_data {
    module_header           hdr;            // Header or basic info for the module
    VmPtr<sequence_data>    psequences;     // All of the tracks in the module: this list is loaded at all times, but tracks for individual sequences might not be
};

static_assert(sizeof(module_data) == 20);

// Format for a user callback function.
// Receives the callback type and value, both of which are defined by the sequencer command invoking the callback.
typedef VmPtr<void(uint8_t callbackType, int16_t value)> callfunc_t;

// Holds information for a user callback.
// User callbacks can be triggered by the 'Eng_StatusMark' sequencer command.
struct callback_status {
    uint8_t     active;         // True if this callback object has been allocated
    uint8_t     type;           // Used to filter whether the 'Eng_StatusMark' command triggers this callback (if the type matches)
    uint16_t    cur_value;      // Current 'value' being passed to the callback (defined by the sequencer command)
    callfunc_t  pfunc;          // The actual function to call
};

static_assert(sizeof(callback_status) == 8);

// Flags specifying what types of patch group data gets loaded
enum patch_group_load_flags : int32_t {
    LOAD_PATCHES        = 0x01,
    LOAD_PATCH_VOICES   = 0x02,
    LOAD_PATCH_SAMPLES  = 0x04,
    LOAD_DRUM_PATCHES   = 0x08,
    LOAD_EXTRA_DATA     = 0x10
};

// Definition for a 'patch group'.
// A patch group is a group of patches/instruments for a specific sound driver, like the PSX sound driver.
struct patch_group_header {
    patch_group_load_flags  load_flags;             // Determines what elements this driver is interested in loading
    SoundDriverId           driver_id;              // Driver id
    uint8_t                 hw_voice_limit;         // How many voices does this driver support
    uint16_t                _pad;                   // Unused
    uint16_t                num_patches;            // How many 'patch' entries there are in the patch group
    uint16_t                patch_size;             // Should be 'sizeof(patch)'
    uint16_t                num_patch_voices;       // How many 'patch_voice' entries there are in the patch group
    uint16_t                patch_voice_size;       // Should be 'sizeof(patch_voice)'
    uint16_t                num_patch_samples;      // How many 'patch_sample' entries there are in the patch group
    uint16_t                patch_sample_size;      // Should be 'sizeof(patch_sample)'
    uint16_t                num_drum_patches;       // How many 'drum_patch' entries there are in the patch group
    uint16_t                drum_patch_size;        // Should be 'sizeof(drum_patch)'
    uint32_t                extra_data_size;        // The size of extra driver specific data in the module file
};

static_assert(sizeof(patch_group_header) == 28);

// Determines what types of tracks a sound driver is interested in loading it
struct hardware_table_list {
    SoundDriverId   driver_id;              // Driver id to use for filtering tracks by driver
    int32_t         sfxload : 1;            // Does the driver load sfx tracks?
    int32_t         musload : 1;            // Does the driver load muisc tracks?
    int32_t         drmload : 1;            // Does the driver load drum tracks?
    int32_t         _unused_flags : 29;     // These bit flags are not used
};

static_assert(sizeof(hardware_table_list) == 8);

// Holds all of the read-only data for a patch group (group of patches specific to a sound driver)
struct patch_group_data {
    patch_group_header      hdr;                                // General info for the patch group
    VmPtr<uint8_t>          pdata;                              // Contains the lists of patches, patch voices and patch samples etc. (all patch group instrument data)
    int32_t                 modfile_offset;                     // Where in the module file this patch group is found
    int32_t                 sndhw_tags[SNDHW_TAG_MAX * 2];      // Holds a copy of the settings list (sound hardware tags) that the driver was initialized with
    hardware_table_list     hw_table_list;                      // What types of tracks the driver wants to load
};

static_assert(sizeof(patch_group_data) == 84);

// Holds the current state for a music/sound sequence
struct sequence_status {
    uint8_t         active : 1;             // If true then this sequence status structure is allocated and actually in use
    uint8_t         handle : 1;             // If set then the sequence must manually be started at a later time after it is allocated
    uint8_t         _unusedFlagBits : 6;    // Unused flag bits
    SeqPlayMode     playmode;               // Is the sequence playing or stopped?
    int16_t         seq_idx;                // The index of the sequence using this status structure
    uint8_t         num_tracks_active;      // How many track statuses this sequence currently has allocated
    uint8_t         num_tracks_playing;     // How many tracks this sequence is currently playing
    uint8_t         volume;                 // Default initialized to upon allocating the sequence status but doesn't seem to be used appart from that?
    uint8_t         pan;                    // Default initialized to upon allocating the sequence status but doesn't seem to be used appart from that?
    uint32_t        type;                   // A user supplied handle: used to identify individual sequences for the purposes of stopping or updating them
    VmPtr<uint8_t>  ptrackstat_indices;     // A sparse list of active track status indices for the sequence. If a slot is '0xFF' then that slot does not to refer to a track status.
    VmPtr<uint8_t>  pgates;                 // The value for all on/off gates for the sequence. If the value is '0xFF' then the gate has not been initialized.
    VmPtr<uint8_t>  piters;                 // The value for all iteration counters for the sequence. If the value is '0xFF' then the counter has not been initialized.
};

static_assert(sizeof(sequence_status) == 24);

// Holds the current state for an individual track in a sequence
struct track_status {
    uint8_t                 active : 1;             // If true then this status structure is allocated and in use
    uint8_t                 mute : 1;               // A flag set to indicate that the track is muted: doesn't seem to actually affect anything though
    uint8_t                 handled : 1;            // When set the track has to be manually started, and manually deallocated
    uint8_t                 stopped : 1;            // This flag is set when playback of the track is paused
    uint8_t                 timed : 1;              // If set then the track is ended at a fixed and manually specified time
    uint8_t                 looped : 1;             // If set then the track loops when ended
    uint8_t                 skip : 1;               // An instruction for the sequencer not to determine the next sequencer command automatically: used by flow control commands to take control
    uint8_t                 off : 1;                // Set to indicate the track is being turned off
    uint8_t                 ref_idx;                // Reference index: this is the index of the track status in the list of track statuses (the track work areas list)
    uint8_t                 seqstat_idx;            // The index of the sequence status this track status belongs to
    SoundDriverId           driver_id;              // What driver is using this track
    uint32_t                qnp_till_next_cmd;      // How many quarter note parts must pass before the next sequencer command is executed (time delta till next command)
    uint8_t                 priority;               // Priority level of the track for voice prioritization (when we're out of voices)
    uint8_t                 reverb;                 // If non zero then reverb is enabled for the track, otherwise disabled
    uint16_t                patch_idx;              // The index of the patch that this track uses
    uint8_t                 volume_cntrl;           // Volume level to use for track voices
    uint8_t                 pan_cntrl;              // Pan modifier to use for track voices
    int16_t                 pitch_cntrl;            // Pitch modifier to use for track voices
    uint8_t                 num_active_voices;      // How many voices is the track currently using?
    uint8_t                 max_voices;             // The maximum number of voices this track can use: copied from the track header
    uint8_t                 mutegroups_mask;        // A set of bit flags for what muting groups the track belongs to. Doesn't seem to affect anything ultimately though?
    SoundClass              sound_class;            // The sound class this track falls into
    uint16_t                tempo_ppq;              // Tempo: parts (or subdivisions) per quarter note - affects timing precision
    uint16_t                tempo_qpm;              // Tempo: quarter notes per minute, or beats per minute (BPM)
    uint16_t                num_labels;             // Copied from the track header: number of labels the track defines
    uint16_t                end_label_idx;          // Copied from the track header: number of labels the track defines
    uint32_t                tempo_ppi_frac;         // Tempo: How many quarter note parts to advance track timing by per hardware timer interrupt (in 16.16 fixed point format)
    uint32_t                deltatime_qnp_frac;     // Delta timing: fractional (.16 fixed point) quarter note parts since the previous sequencer commmand
    uint32_t                deltatime_qnp;          // Delta timing: whole quarter note parts since the previous sequencer commmand
    uint32_t                abstime_qnp;            // Absolute timing: total whole quarter note parts since the track was started
    uint32_t                end_abstime_qnp;        // Timing: absolute quarter note part count to end the track at (if the 'timed' flag is set for manual track end time)
    VmPtr<uint8_t>          pcmds_start;            // Points to the start of the command stream for the track: this contains all of the track's sequencer commands and the time between them
    VmPtr<uint8_t>          pcur_cmd;               // Current track location: a pointer within the track's command stream to the next upcoming sequencer command
    VmPtr<uint32_t>         plabels;                // Convenience pointer to the label list for the track
    VmPtr<VmPtr<uint8_t>>   ploc_stack;             // A stack of track locations that can be used by sequencer commands which save and restore the current track data location
    VmPtr<VmPtr<uint8_t>>   ploc_stack_cur;         // Points to the next stack location to use from the track location stack
    VmPtr<VmPtr<uint8_t>>   ploc_stack_end;         // Points to the end of the track's location stack
    uint32_t                cmd_stream_size;        // How big the track's command stream is (in bytes)
    uint32_t                cmd_stream_capacity;    // How big the memory block for the track's command stream is (in bytes)
};

static_assert(sizeof(track_status) == 80);

// Holds state for one hardware voice in the sequencer system
struct voice_status {
    uint8_t                         active : 1;             // '1' if the voice has been allocated
    uint8_t                         release : 1;            // '1' if the voice is being released or 'keyed off'
    uint8_t                         _unusedFlagBits : 6;    // These flag fields are unused
    SoundDriverId                   driver_id;              // Which sound driver this voice is used with
    uint8_t                         ref_idx;                // Reference index: index of this voice in it's parent patch group (hardware voice index)
    uint8_t                         trackstat_idx;          // The index of the track status which is using this voice
    uint8_t                         priority;               // Inherited from the parent track: used to determine when voices are 'stolen' or not when the voice limit is reached
    uint8_t                         note;                   // Which note (semitone) the voice is to play
    uint8_t                         volume;                 // Volume the voice is to play at
    SoundClass                      sound_class;            // What broad class of sound the voice is being used for (MUSIC, SFX etc.)
    VmPtr<const patch_voice>        ppatch_voice;           // The patch voice which used by this hardware voice
    VmPtr<const patch_sample>       ppatch_sample;          // Details for the sound sample used by the hardware voice
    uint32_t                        onoff_abstime_ms;       // When the voice was keyed on or started, or when it fully ends and should be deallocated (if being keyed off). The time is absolute time in MS since the sequencer started.
    uint32_t                        release_time_ms;        // How long it takes for the voice to fade out after being released
};

static_assert(sizeof(voice_status) == 24);

// The master status structure: this is the root data structure for the entire sequencer system, holding most of it's read-only data and state.
// It's populated with information from the module file (.WMD) and contains state for all of the active sequences, tracks within sequences and
// individual voices. It also holds volume levels, driver info and callbacks etc.
struct master_status_structure {
    uint32_t*                   pabstime_ms;                // Pointer to the current absolute sequencer time (MS)
    uint8_t                     num_active_seqs;            // How many sequence statuses are currently allocated from the pool of sequence statuses
    uint8_t                     num_active_tracks;          // How many track statuses are currently allocated from the pool of track statuses
    uint8_t                     num_active_voices;          // How many voices are currently allocated among all tracks
    uint8_t                     max_voices;                 // The total number of voice statuses for all patch groups (max active voices)
    uint8_t                     num_patch_groups;           // How many patch groups or patches for a sound driver there are - should just be '1' (PSX) for DOOM
    uint8_t                     _unused1;                   // An unused field: its purpose cannot be inferred as it is never used
    uint8_t                     num_active_callbacks;       // How many user callbacks are allocated: never written to in DOOM, other than zero initialized
    uint8_t                     _unused2;                   // An unused field: its purpose cannot be inferred as it is never used
    VmPtr<module_data>          pmodule;                    // Module header, sequences and track data (for sequences that are loaded)
    VmPtr<callback_status>      pcallback_stats;            // A pool of callback statuses which can be user allocated and invoked by 'Eng_StatusMark' commands
    VmPtr<uint8_t>              pmaster_vols;               // Master volume levels for each patch group or sound driver: written to but appears otherwise unused
    VmPtr<patch_group_data>     ppatch_groups;              // A collection of patch group settings and data for each sound driver: instrument definitions live here
    uint32_t                    max_tracks_per_seq;         // Maximum number of tracks per sequence for all sequences in the module
    VmPtr<sequence_status>      psequence_stats;            // Sequence statuses: a pool from which all sequence statuses are allocated. The size of this list is specified by the module header.
    uint32_t                    max_track_loc_stack_size;   // Maximum number of track data locations that can be saved per track for later restoring (location stack)
    VmPtr<track_status>         ptrack_stats;               // Track statuses: a pool from which all track statuses are allocated. The size of this list is specified by the module header.
    uint32_t                    max_voices_per_track;       // Maximum number of voices used by any track in any sequence
    VmPtr<voice_status>         pvoice_stats;               // Status structures or 'work areas' for voices sequences. The size of this list is determined by sound drivers.
    VmPtr<PsxCd_File>           pmodule_file;               // Pointer to the module file itself
};

extern uint32_t     gWess_Millicount;
extern uint32_t     gWess_Millicount_Frac;
extern bool         gbWess_WessTimerActive;
extern uint8_t      gWess_sectorBuffer1[CD_SECTOR_SIZE];
extern uint8_t      gWess_sectorBuffer2[CD_SECTOR_SIZE];
extern bool         gbWess_SeqOn;

// Type for a driver or sequencer command function.
// The format of the function depends on the particular command function invoked.
struct WessDriverFunc {
    // These are the three acceptable formats for the driver function
    union {
        void (*pNoArg)() noexcept;
        void (*pMasterStatArg)(master_status_structure& mstat) noexcept;
        void (*pTrackStatArg)(track_status& trackStat) noexcept;
    };

    inline WessDriverFunc(void (*pNoArg)() noexcept) noexcept : pNoArg(pNoArg) {}
    inline WessDriverFunc(void (*pMasterStatArg)(master_status_structure&) noexcept) noexcept : pMasterStatArg(pMasterStatArg) {}
    inline WessDriverFunc(void (*pTrackStatArg)(track_status&) noexcept) noexcept : pTrackStatArg(pTrackStatArg) {}

    // Invoke the function using one of the following overloads
    inline void operator()() const noexcept { pNoArg(); }
    inline void operator()(master_status_structure& mstat) const noexcept { pMasterStatArg(mstat); }
    inline void operator()(track_status& trackStat) const noexcept { pTrackStatArg(trackStat); }
};

// Lists of command handling functions for each driver.
// Up to 10 driver slots are available.
extern const WessDriverFunc* const gWess_CmdFuncArr[10];

int16_t GetIntsPerSec() noexcept;
uint32_t CalcPartsPerInt(const int16_t intsPerSec, const int16_t partsPerQNote, const int16_t qnotesPerMin) noexcept;
int32_t WessInterruptHandler() noexcept;
void init_WessTimer() noexcept;
void exit_WessTimer() noexcept;
bool Wess_init_for_LoadFileData() noexcept;
PsxCd_File* module_open(const CdMapTbl_File fileId) noexcept;
int32_t module_read(void* const pDest, const int32_t numBytes, PsxCd_File& file) noexcept;
int32_t module_seek(PsxCd_File& file, const int32_t seekPos, const PsxCd_SeekMode seekMode) noexcept;
int32_t module_tell(const PsxCd_File& file) noexcept;
void module_close(PsxCd_File& file) noexcept;
int32_t get_num_Wess_Sound_Drivers(const int32_t* const* const pSettingTagLists) noexcept;
PsxCd_File* data_open(const CdMapTbl_File fileId) noexcept;
int32_t data_read(PsxCd_File& file, const int32_t destSpuAddr, const int32_t numBytes, const int32_t fileOffset) noexcept;
void data_close(PsxCd_File& file) noexcept;
void wess_low_level_init() noexcept;
void wess_low_level_exit() noexcept;
void* wess_malloc(const int32_t size) noexcept;
void wess_free(void* const pMem) noexcept;
