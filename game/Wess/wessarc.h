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

// Describes a group of voices used for a patch/instrument
struct patches_header {
    uint16_t    patchmap_cnt;       // How many voices to use for the patch
    uint16_t    patchmap_idx;       // Index of the first voice in the patchmaps list. The patchmaps for subsequent voices follow contiguously.
};

static_assert(sizeof(patches_header) == 4);

// Settings for one individual voice/sound in a patch/instrument
struct patchmaps_header {
    uint8_t     priority;           // Voice priority: used to determine what to kill when we're out of voices
    uint8_t     reverb;             // Appears to be unused: reverb for the voice
    uint8_t     volume;             // Volume to play the voice at
    uint8_t     pan;                // Voice pan 64 to 191 - 128 is the voice center. Values outside this range are clamped by the PSX driver.
    uint8_t     root_key;           // The 'base' note/semitone or the note at which the sound is played back at 44,100 Hz frequency
    uint8_t     fine_adj;           // The .8 fractional part of the base note
    uint8_t     note_min;           // Minimum semitone at which the voice can be played - does not sound below this
    uint8_t     note_max;           // Maximum semitone at which the voice can be played - does not sound above this
    uint8_t     pitchstep_min;      // How big each unit of pitch shift is when pitch shifting down (in 1/8192 units)
    uint8_t     pitchstep_max;      // How big each unit of pitch shift is when pitch shifting up (in 1/8192 units)
    uint16_t    sample_id;          // The index of the sound sample to use for this voice
    uint16_t    adsr1;              // The first 16-bits of the SPU voice envelope (see comments for 'SpuVoiceAttr' for more details)
    uint16_t    adsr2;              // The second (high) 16-bits of the SPU voice envelope (see comments for 'SpuVoiceAttr' for more details)
};

static_assert(sizeof(patchmaps_header) == 16);

// Holds details for a sound sample which can be loaded or is already loaded
struct patchinfo_header {
    uint32_t    sample_offset;      // Unused & unclear what the purpose of this is for. Increases with each patch in the module.
    uint32_t    sample_size;        // The size in bytes of the sound data for this patch
    uint32_t    sample_pos;         // Where in SPU RAM the patch is currently uploaded to. Set to '0' if not uploaded to the SPU.
};

static_assert(sizeof(patchinfo_header) == 12);

// Settings for an individual drum voice in a drum track
struct drumpmaps_header {
    uint16_t    patchnum;       // Watch patch to use for this drum
    uint16_t    note;           // What note/pitch to play the drum at
};

static_assert(sizeof(drumpmaps_header) == 4);

// Main header for a module (.WMD) file.
// Defines general/global info and settings for the module.
struct module_header {
    int32_t     module_id_text;         // Should be the string 'SPSX'
    uint32_t    module_version;         // Should be '1'
    uint16_t    sequences;              // How many sequences are in the module.
    uint8_t     patch_types_infile;     // How many sound driver types are supported by the module.
    uint8_t     seq_work_areas;         // The number of sequence work areas to allocate, or the maximum number of active sequences
    uint8_t     trk_work_areas;         // The number of track work areas to allocate, or the maximum number of active tracks
    uint8_t     gates_per_seq;          // Maximum number of 'gates' or on/off conditional jump switches per sequence
    uint8_t     iters_per_seq;          // Maximum number of iteration counts (iters) per sequence. These are used to do jumps a certain number of times.
    uint8_t     callback_areas;         // Maximum number of user callbacks that can be used
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
    SoundDriverId   voices_type;            // What sound driver the track should play with
    uint8_t         voices_max;             // Maximum number of voices that the track claims to use
    uint8_t         priority;               // Used for prioritizing voices when we are out of hardware voices
    uint8_t         lockchannel;            // Seems to be unused in this version of the WESS library, unclear what the purpose is
    SoundClass      voices_class;           // What rough class of sounds the track contains
    uint8_t         reverb;                 // The reverb level to initialize the track status with
    uint16_t        initpatchnum;           // Which patch to initially use for the track status unless manually overridden
    uint16_t        initpitch_cntrl;        // What pitch shift value to initially use for the track status unless manually overridden
    uint8_t         initvolume_cntrl;       // What volume to initially use for the track status unless manually overridden
    uint8_t         initpan_cntrl;          // What pan setting to initially use for the track status unless manually overridden
    uint8_t         substack_count;         // The maximum number of locations that can be remembered in the track's location stack (for save + return control flow)
    uint8_t         mutebits;               // Used to initialize the 'mutemask' track status parameter. Those mask bits define muting groups.
    uint16_t        initppq;                // Parts per quarter note: how many parts to subdivide each quarter note into for timing purposes. Affects timing precision.
    uint16_t        initqpm;                // The quarter notes per minute (beats per minute) value to initialize the track status with, unless manually overridden.
    uint16_t        labellist_count;        // How many jump/position labels the track defines
    uint32_t        data_size;              // The size of the data stream containing sequencer commands for the track
};

static_assert(sizeof(track_header) == 24);

// Holds all of the read-only data for a track in a music sequence
struct track_data {
    track_header        trk_hdr;        // Header for the track
    VmPtr<uint32_t>     plabellist;     // A list of offsets into track data: used by control flow (i.e jump) sequencer commands
    VmPtr<uint8_t>      ptrk_data;      // The actual byte stream containing sequencer commands for the track
};

static_assert(sizeof(track_data) == 32);

// Basic information for a sequence
struct seq_header {
    uint16_t    tracks;         // How many tracks are in the sequence
    uint16_t    _unused1;       // Field appears to be unused - it's purpose cannot be determined because of that
};

static_assert(sizeof(seq_header) == 4);

// Holds all of the read-only data for a music sequence
struct sequence_data {
    seq_header          seq_hdr;            // General info for the sequence: always remains loaded in memory
    VmPtr<track_data>   ptrk_info;          // The loaded list of tracks for the sequence: loaded and unloaded on demand for music tracks (to save memory)
    uint32_t            fileposition;       // The offset of this sequence's data in the module file
    uint32_t            trkinfolength;      // Total size of all the tracks in the sequence: this field is populated when the module is first loaded
    uint32_t            trkstoload;         // Total number of tracks in the sequence: this field is populated when the module is first loaded
};

static_assert(sizeof(sequence_data) == 20);

// Contains the header for the module and the read-only data for all loaded sequences
struct module_data {
    module_header           mod_hdr;        // Header or basic info for the module
    VmPtr<sequence_data>    pseq_info;      // All of the tracks in the module: this list is loaded at all times, but tracks for individual sequences might not be
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
    uint16_t    curval;         // Current 'value' being passed to the callback (defined by the sequencer command)
    callfunc_t  callfunc;       // The actual function to call
};

static_assert(sizeof(callback_status) == 8);

// Flags specifying what types of patch group data gets loaded
enum patch_grp_load_flags : uint32_t {
    LOAD_PATCHES    = 0x01,
    LOAD_PATCHMAPS  = 0x02,
    LOAD_PATCHINFO  = 0x04,
    LOAD_DRUMMAPS   = 0x08,
    LOAD_EXTRADATA  = 0x10
};

// Definition for a 'patch group'.
// A patch group is a group of patches/instruments for a specific sound driver, like the PSX sound driver.
struct patch_group_header {
    patch_grp_load_flags    load_flags;             // Determines what elements this driver is interested in loading
    SoundDriverId           patch_id;               // Driver id
    uint8_t                 hw_voice_limit;         // How many voices does this driver support
    uint16_t                _pad;                   // Unused
    uint16_t                patches;                // How many patches or instruments does this patch group define
    uint16_t                patch_size;             // Should be 'sizeof(patches_header)'
    uint16_t                patchmaps;              // How many patchmap (individual patch voice) entries there are
    uint16_t                patchmap_size;          // Should be 'sizeof(patchmaps_header)'
    uint16_t                patchinfo;              // How many patchinfo (patch sample definition) entries there are
    uint16_t                patchinfo_size;         // Should be 'sizeof(patchinfo_header)'
    uint16_t                drummaps;               // How many drummap entries thre are (individual drum voice)
    uint16_t                drummap_size;           // Should be 'sizeof(drumpmaps_header)'
    uint32_t                extra_data_size;        // The size of extra driver specific data in the module file
};

static_assert(sizeof(patch_group_header) == 28);

// Determines what types of tracks a sound driver is interested in loading it
struct hardware_table_list {
    SoundDriverId   hardware_ID;            // Driver id to use for filtering tracks by driver
    int32_t         sfxload : 1;            // Does the driver load sfx tracks?
    int32_t         musload : 1;            // Does the driver load muisc tracks?
    int32_t         drmload : 1;            // Does the driver load drum tracks?
    int32_t         _unused_flags : 29;     // These bit flags are not used
};

static_assert(sizeof(hardware_table_list) == 8);

// Holds all of the read-only data for a patch group (group of patches specific to a sound driver)
struct patch_group_data {
    patch_group_header      pat_grp_hdr;                        // General info for the patch group
    VmPtr<uint8_t>          ppat_data;                          // Contains the lists of 'patches_header', 'patchmaps_header', 'patchinfo_header' and 'drumpmaps_header' data structures (in that order)
    int32_t                 data_fileposition;                  // Where in the module file this patch group is found
    int32_t                 sndhw_tags[SNDHW_TAG_MAX * 2];      // Holds a copy of the settings list that the driver was initialized with
    hardware_table_list     hw_tl_list;                         // What types of tracks the driver wants to load
};

static_assert(sizeof(patch_group_data) == 84);

// Holds the current state for a music/sound sequence
struct sequence_status {
    uint8_t         active : 1;             // If true then this sequence status structure is allocated and actually in use
    uint8_t         handle : 1;             // If set then the sequence must manually be started at a later time after it is allocated
    uint8_t         _unusedFlagBits : 6;    // Unused flag bits
    SeqPlayMode     playmode;               // Is the sequence playing or stopped?
    int16_t         seq_idx;                // The index of the sequence using this status structure
    uint8_t         tracks_active;          // How many track statuses this sequence currently has allocated
    uint8_t         tracks_playing;         // How many tracks this sequence is currently playing
    uint8_t         volume;                 // Default initialized to upon allocating the sequence status but doesn't seem to be used appart from that?
    uint8_t         pan;                    // Default initialized to upon allocating the sequence status but doesn't seem to be used appart from that?
    uint32_t        seq_type;               // A user supplied handle: used to identify individual sequences for the purposes of stopping or updating them
    VmPtr<uint8_t>  ptrk_indxs;             // A sparse list of active track status indices for the sequence. If a slot is '0xFF' then that slot does not to refer to a track status.
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
    uint8_t                 refindx;                // This is the index of the track status in the list of track statuses (the track work areas list)
    uint8_t                 seq_owner;              // The index of the sequence status this track status belongs to
    SoundDriverId           patchtype;              // What driver is using this track
    uint32_t                deltatime;              // How many quarter note parts must pass before the next sequencer command is executed (time delta till next command)
    uint8_t                 priority;               // Priority level of the track for voice prioritization (when we're out of voices)
    uint8_t                 reverb;                 // If non zero then reverb is enabled for the track, otherwise disabled
    uint16_t                patchnum;               // The index of the patch that this track uses
    uint8_t                 volume_cntrl;           // Volume level to use for track voices
    uint8_t                 pan_cntrl;              // Pan modifier to use for track voices
    int16_t                 pitch_cntrl;            // Pitch modifier to use for track voices
    uint8_t                 voices_active;          // How many voices is the track currently using?
    uint8_t                 voices_max;             // The maximum number of voices this track can use: copied from the track header
    uint8_t                 mutemask;               // A set of bit flags for what muting groups the track belongs to. Doesn't seem to affect anything ultimately though?
    SoundClass              sndclass;               // The sound class this track falls into
    uint16_t                ppq;                    // Timing: parts or subdivisions per quarter note - determines timing precision
    uint16_t                qpm;                    // Timing: quarter notes per minute, or beats per minute (BPM)
    uint16_t                labellist_count;        // Copied from the track header: number of labels the track defines
    uint16_t                labellist_max;          // Copied from the track header: number of labels the track defines
    uint32_t                ppi;                    // How many quarter note parts to advance track timing by per hardware timer interrupt (in 16.16 fixed point format)
    uint32_t                starppi;                // Timing: current fractional (.16 fixed point) accumulated quarter note parts count
    uint32_t                accppi;                 // Timing: accumulated whole quarter note parts before the next sequencer command is executed. Decremented when a sequencer command is done.
    uint32_t                totppi;                 // Timing: accumulated whole quarter note parts, absolute or total value since the track was started
    uint32_t                endppi;                 // Timing: absolute quarter note part count to end the track at (if the 'timed' flag is set for manual track end time)
    VmPtr<uint8_t>          pstart;                 // Points to the first byte of track sequencer command data
    VmPtr<uint8_t>          ppos;                   // Track location: current pointer within the track sequencer command data
    VmPtr<uint32_t>         plabellist;             // Convenience pointer to the label list for the track
    VmPtr<uint32_t>         psubstack;              // A stack of track locations that can be used by sequencer commands which save and restore the current track data location
    VmPtr<VmPtr<uint8_t>>   psp;                    // Points to the next stack location to use from the track location stack
    VmPtr<uint32_t>         pstackend;              // Points to the end of the track's location stack
    uint32_t                data_size;              // How big the track sequencer/command data is (in bytes)
    uint32_t                data_space;             // How big the track sequencer/command data is (in bytes)
};

static_assert(sizeof(track_status) == 80);

// Holds state for one hardware voice in the sequencer system
struct voice_status {
    uint8_t                         active : 1;             // '1' if the voice has been allocated
    uint8_t                         release : 1;            // '1' if the voice is being released or 'keyed off'
    uint8_t                         _unusedFlagBits : 6;    // These flag fields are unused
    SoundDriverId                   patchtype;              // Which sound driver this voice is used with
    uint8_t                         refindx;                // Index of the voice in it's sound driver
    uint8_t                         track_idx;              // What track index is using the voice
    uint8_t                         priority;               // Inherited from the parent track: used to determine when voices are 'stolen' or not when the voice limit is reached
    uint8_t                         note;                   // Which note (semitone) the voice is to play
    uint8_t                         volume;                 // Volume the voice is to play at
    SoundClass                      sndtype;                // What broad class of sound the voice is being used for (MUSIC, SFX etc.)
    VmPtr<const patchmaps_header>   patchmaps;              // The patch voice which uses this voice
    VmPtr<const patchinfo_header>   patchinfo;              // Details for the actual sound sample to use for the voice
    uint32_t                        pabstime;               // When the voice should be released and fully shut off (in absolute time)
    uint32_t                        adsr2;                  // How long it takes for the voice to fade out after being released
};

static_assert(sizeof(voice_status) == 24);

// The master status structure: this is the root data structure for the entire sequencer system, holding most of it's state.
// It's populated with information from the module file (.WMD) and contains state for all of the active sequences,
// tracks within sequences and individual voices. It also holds volume levels, driver info and callbacks etc.
struct master_status_structure {
    VmPtr<uint32_t>             pabstime;                   // Pointer to the current absolute sequencer time (MS)
    uint8_t                     seqs_active;                // How many sequences are currently in allocated
    uint8_t                     trks_active;                // How many tracks are currently allocated among all sequences
    uint8_t                     voices_active;              // How many voices are currently allocated among all tracks
    uint8_t                     voices_total;               // The total number of voice statuses (max active voices)
    uint8_t                     patch_types_loaded;         // How many sound drivers there are - should just be '1' (PSX) for DOOM
    uint8_t                     _unused1;                   // An unused field: its purpose cannot be inferred as it is never used
    uint8_t                     callbacks_active;           // How many user callbacks are allocated: never written to in DOOM, other than zero initialized
    uint8_t                     _unused2;                   // An unused field: its purpose cannot be inferred as it is never used
    VmPtr<module_data>          pmod_info;                  // Module header and sequence read-only data
    VmPtr<callback_status>      pcalltable;                 // A list of user callbacks which can be invoked by 'Eng_StatusMark' commands
    VmPtr<uint8_t>              pmaster_volume;             // Master volume level for the sequencer: written to but appears otherwise unused
    VmPtr<patch_group_data>     ppat_info;                  // A collection of patch group settings and data for each sound driver: instrument definitions live here
    uint32_t                    max_trks_perseq;            // Maximum number of tracks per sequence for all sequences in the module
    VmPtr<sequence_status>      pseqstattbl;                // Status structures or 'work areas' for tracker sequences. The size of this list is specified by the module header.
    uint32_t                    max_substack_pertrk;        // Maximum number of track data locations that can be saved per track for later restoring (location stack)
    VmPtr<track_status>         ptrkstattbl;                // Track statuses or 'work areas' for all tracks in the sequencer. The size of this list is specified by the module header.
    uint32_t                    max_voices_pertrk;          // Maximum number of voices used by any track in any sequence
    VmPtr<voice_status>         pvoicestattbl;              // Status structures or 'work areas' for voices sequences. The size of this list is determined by sound drivers.
    VmPtr<PsxCd_File>           fp_module;                  // Pointer to the module file itself
};

static_assert(sizeof(master_status_structure) == 56);

extern const VmPtr<uint32_t>                    gWess_Millicount;
extern const VmPtr<uint32_t>                    gWess_Millicount_Frac;
extern const VmPtr<bool32_t>                    gbWess_WessTimerActive;
extern const VmPtr<uint8_t[CD_SECTOR_SIZE]>     gWess_sectorBuffer1;
extern const VmPtr<uint8_t[CD_SECTOR_SIZE]>     gWess_sectorBuffer2;
extern const VmPtr<bool32_t>                    gbWess_SeqOn;

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
