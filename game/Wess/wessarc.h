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

// Driver/sequencer command ids
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

// TODO: COMMENT
struct patches_header {
    uint16_t    patchmap_cnt;       // 0x000    TODO: COMMENT
    uint16_t    patchmap_idx;       // 0x002    TODO: COMMENT
};

static_assert(sizeof(patches_header) == 4);

// TODO: COMMENT
struct patchmaps_header {
    uint8_t     priority;           // 0x000    TODO: COMMENT
    uint8_t     reverb;             // 0x001    TODO: COMMENT
    uint8_t     volume;             // 0x002    TODO: COMMENT
    uint8_t     pan;                // 0x003    TODO: COMMENT
    uint8_t     root_key;           // 0x004    TODO: COMMENT
    uint8_t     fine_adj;           // 0x005    TODO: COMMENT
    uint8_t     note_min;           // 0x006    TODO: COMMENT
    uint8_t     note_max;           // 0x007    TODO: COMMENT
    uint8_t     pitchstep_min;      // 0x008    TODO: COMMENT
    uint8_t     pitchstep_max;      // 0x009    TODO: COMMENT
    uint16_t    sample_id;          // 0x00A    TODO: COMMENT
    uint16_t    adsr1;              // 0x00C    TODO: COMMENT
    uint16_t    adsr2;              // 0x00E    TODO: COMMENT
};

static_assert(sizeof(patchmaps_header) == 16);

// TODO: COMMENT
struct patchinfo_header {
    uint32_t    sample_offset;      // 0x000    TODO: COMMENT
    uint32_t    sample_size;        // 0x004    TODO: COMMENT
    uint32_t    sample_pos;         // 0x008    TODO: COMMENT
};

static_assert(sizeof(patchinfo_header) == 12);

// Defines a drum type in a drunk track
struct drumpmaps_header {
    uint16_t    patchnum;       // Watch patch to use for this drum
    uint16_t    note;           // What note/pitch to play the drum at
};

static_assert(sizeof(drumpmaps_header) == 4);

// TODO: COMMENT
struct module_header {
    int32_t     module_id_text;         // 0x000    TODO: COMMENT
    uint32_t    module_version;         // 0x004    TODO: COMMENT
    uint16_t    sequences;              // 0x008    TODO: COMMENT
    uint8_t     patch_types_infile;     // 0x00A    TODO: COMMENT
    uint8_t     seq_work_areas;         // 0x00B    TODO: COMMENT
    uint8_t     trk_work_areas;         // 0x00C    TODO: COMMENT
    uint8_t     gates_per_seq;          // 0x00D    TODO: COMMENT
    uint8_t     iters_per_seq;          // 0x00E    TODO: COMMENT
    uint8_t     callback_areas;         // 0x00F    TODO: COMMENT
};

static_assert(sizeof(module_header) == 16);

// Classes of sounds/voices
enum SoundClass : uint8_t {
    SNDFX_CLASS     = 0,    // TODO: COMMENT
    MUSIC_CLASS     = 1,    // TODO: COMMENT
    DRUMS_CLASS     = 2,    // TODO: COMMENT
    SFXDRUMS_CLASS  = 3     // TODO: COMMENT
};

// TODO: COMMENT
struct track_header {
    SoundDriverId   voices_type;            // 0x000    TODO: COMMENT
    uint8_t         voices_max;             // 0x001    TODO: COMMENT
    uint8_t         priority;               // 0x002    TODO: COMMENT
    uint8_t         lockchannel;            // 0x003    TODO: COMMENT
    SoundClass      voices_class;           // 0x004    TODO: COMMENT
    uint8_t         reverb;                 // 0x005    TODO: COMMENT
    uint16_t        initpatchnum;           // 0x006    TODO: COMMENT
    uint16_t        initpitch_cntrl;        // 0x008    TODO: COMMENT
    uint8_t         initvolume_cntrl;       // 0x00A    TODO: COMMENT
    uint8_t         initpan_cntrl;          // 0x00B    TODO: COMMENT
    uint8_t         substack_count;         // 0x00C    TODO: COMMENT
    uint8_t         mutebits;               // 0x00D    TODO: COMMENT
    uint16_t        initppq;                // 0x00E    TODO: COMMENT
    uint16_t        initqpm;                // 0x010    TODO: COMMENT
    uint16_t        labellist_count;        // 0x012    TODO: COMMENT
    uint32_t        data_size;              // 0x014    TODO: COMMENT
};

static_assert(sizeof(track_header) == 24);

// TODO: COMMENT
struct track_data {
    track_header        trk_hdr;        // 0x000    TODO: COMMENT
    VmPtr<uint32_t>     plabellist;     // 0x018    TODO: COMMENT
    VmPtr<uint8_t>      ptrk_data;      // 0x01C    TODO: COMMENT
};

static_assert(sizeof(track_data) == 32);

// TODO: COMMENT
struct seq_header {
    uint16_t    tracks;     // 0x000    TODO: COMMENT
    uint16_t    _unused1;       // 0x002    TODO: COMMENT
};

static_assert(sizeof(seq_header) == 4);

// TODO: COMMENT
struct sequence_data {
    seq_header          seq_hdr;            // 0x000    TODO: COMMENT
    VmPtr<track_data>   ptrk_info;          // 0x004    TODO: COMMENT
    uint32_t            fileposition;       // 0x008    TODO: COMMENT
    uint32_t            trkinfolength;      // 0x00C    TODO: COMMENT
    uint32_t            trkstoload;         // 0x010    TODO: COMMENT
};

static_assert(sizeof(sequence_data) == 20);

// TODO: COMMENT
struct module_data {
    module_header           mod_hdr;        // 0x000    TODO: COMMENT
    VmPtr<sequence_data>    pseq_info;      // 0x010    TODO: COMMENT
};

static_assert(sizeof(module_data) == 20);

// TODO: COMMENT
typedef VmPtr<void(uint8_t callbackType, int16_t value)> callfunc_t;

// TODO: COMMENT
struct callback_status {
    uint8_t     active;     // 0x000    TODO: COMMENT
    uint8_t     type;       // 0x001    TODO: COMMENT
    uint16_t    curval;     // 0x002    TODO: COMMENT
    callfunc_t  callfunc;   // 0x004    TODO: COMMENT
};

static_assert(sizeof(callback_status) == 8);

// TODO: COMMENT
struct patch_group_header {
    uint32_t        load_flags;         // 0x000    TODO: COMMENT
    SoundDriverId   patch_id;           // 0x004    TODO: COMMENT
    uint8_t         hw_voice_limit;     // 0x005    TODO: COMMENT
    uint16_t        pad1;               // 0x006    TODO: COMMENT
    uint16_t        patches;            // 0x008    TODO: COMMENT
    uint16_t        patch_size;         // 0x00A    TODO: COMMENT
    uint16_t        patchmaps;          // 0x00C    TODO: COMMENT
    uint16_t        patchmap_size;      // 0x00E    TODO: COMMENT
    uint16_t        patchinfo;          // 0x010    TODO: COMMENT
    uint16_t        patchinfo_size;     // 0x012    TODO: COMMENT
    uint16_t        drummaps;           // 0x014    TODO: COMMENT
    uint16_t        drummap_size;       // 0x016    TODO: COMMENT
    uint32_t        extra_data_size;    // 0x018    TODO: COMMENT
};

static_assert(sizeof(patch_group_header) == 28);

// TODO: COMMENT
struct hardware_table_list {
    int32_t     hardware_ID;            // 0x000    TODO: COMMENT
    int32_t     sfxload : 1;            // 0x004    TODO: COMMENT
    int32_t     musload : 1;            // 0x004    TODO: COMMENT
    int32_t     drmload : 1;            // 0x004    TODO: COMMENT
    int32_t     _unused_flags : 29;     // 0x004    TODO: COMMENT
};

static_assert(sizeof(hardware_table_list) == 8);

// TODO: COMMENT
struct patch_group_data {
    patch_group_header      pat_grp_hdr;                        // 0x000    TODO: COMMENT
    VmPtr<uint8_t>          ppat_data;                          // 0x01C    TODO: COMMENT
    int32_t                 data_fileposition;                  // 0x020    TODO: COMMENT
    int32_t                 sndhw_tags[SNDHW_TAG_MAX * 2];      // 0x024    TODO: COMMENT
    hardware_table_list     hw_tl_list;                         // 0x04C    TODO: COMMENT
};

static_assert(sizeof(patch_group_data) == 84);

// TODO: COMMENT
struct sequence_status {
    uint8_t         active : 1;             // 0x000    TODO: COMMENT
    uint8_t         handle : 1;             // 0x000    TODO: COMMENT
    uint8_t         _unusedFlagBits : 6;    // 0x000    TODO: COMMENT
    SeqPlayMode     playmode;               // Is the sequence playing or stopped?
    int16_t         seq_num;                // 0x002    TODO: COMMENT
    uint8_t         tracks_active;          // 0x004    TODO: COMMENT
    uint8_t         tracks_playing;         // 0x005    TODO: COMMENT
    uint8_t         volume;                 // 0x006    TODO: COMMENT
    uint8_t         pan;                    // 0x007    TODO: COMMENT
    uint32_t        seq_type;               // 0x008    TODO: COMMENT
    VmPtr<uint8_t>  ptrk_indxs;             // 0x00C    TODO: COMMENT
    VmPtr<uint8_t>  pgates;                 // 0x010    TODO: COMMENT
    VmPtr<uint8_t>  piters;                 // 0x014    TODO: COMMENT
};

static_assert(sizeof(sequence_status) == 24);

// TODO: COMMENT
struct track_status {
    uint8_t                 active : 1;             // 0x000    TODO: COMMENT
    uint8_t                 mute : 1;               // 0x000    TODO: COMMENT
    uint8_t                 handled : 1;            // When set the track has to be manually started, and manually deallocated
    uint8_t                 stopped : 1;            // 0x000    TODO: COMMENT
    uint8_t                 timed : 1;              // 0x000    TODO: COMMENT
    uint8_t                 looped : 1;             // 0x000    TODO: COMMENT
    uint8_t                 skip : 1;               // 0x000    TODO: COMMENT
    uint8_t                 off : 1;                // 0x000    TODO: COMMENT
    uint8_t                 refindx;                // 0x001    TODO: COMMENT
    uint8_t                 seq_owner;              // 0x002    TODO: COMMENT
    SoundDriverId           patchtype;              // 0x003    TODO: COMMENT
    uint32_t                deltatime;              // 0x004    TODO: COMMENT
    uint8_t                 priority;               // 0x008    TODO: COMMENT
    uint8_t                 reverb;                 // 0x009    TODO: COMMENT
    uint16_t                patchnum;               // 0x00A    TODO: COMMENT
    uint8_t                 volume_cntrl;           // 0x00C    TODO: COMMENT
    uint8_t                 pan_cntrl;              // 0x00D    TODO: COMMENT
    int16_t                 pitch_cntrl;            // 0x00E    TODO: COMMENT
    uint8_t                 voices_active;          // 0x010    TODO: COMMENT
    uint8_t                 voices_max;             // 0x011    TODO: COMMENT
    uint8_t                 mutemask;               // 0x012    TODO: COMMENT
    SoundClass              sndclass;               // 0x013    TODO: COMMENT
    uint16_t                ppq;                    // 0x014    TODO: COMMENT
    uint16_t                qpm;                    // 0x016    TODO: COMMENT
    uint16_t                labellist_count;        // 0x018    TODO: COMMENT
    uint16_t                labellist_max;          // 0x01A    TODO: COMMENT
    uint32_t                ppi;                    // 0x01C    TODO: COMMENT
    uint32_t                starppi;                // 0x020    TODO: COMMENT
    uint32_t                accppi;                 // 0x024    TODO: COMMENT
    uint32_t                totppi;                 // 0x028    TODO: COMMENT
    uint32_t                endppi;                 // 0x02C    TODO: COMMENT
    VmPtr<uint8_t>          pstart;                 // 0x030    TODO: COMMENT
    VmPtr<uint8_t>          ppos;                   // 0x034    TODO: COMMENT
    VmPtr<uint32_t>         plabellist;             // 0x038    TODO: COMMENT
    VmPtr<uint32_t>         psubstack;              // 0x03C    TODO: COMMENT
    VmPtr<VmPtr<uint8_t>>   psp;                    // 0x040    TODO: COMMENT
    VmPtr<uint32_t>         pstackend;              // 0x044    TODO: COMMENT
    uint32_t                data_size;              // 0x048    TODO: COMMENT
    uint32_t                data_space;             // 0x04C    TODO: COMMENT
};

static_assert(sizeof(track_status) == 80);

// Holds state for one hardware voice in the sequencer system
struct voice_status {
    uint8_t                         active : 1;             // '1' if the voice has been allocated
    uint8_t                         release : 1;            // '1' if the voice is being released or 'keyed off'
    uint8_t                         _unusedFlagBits : 6;    // These flag fields are unused
    SoundDriverId                   patchtype;              // Which sound driver this voice is used with
    uint8_t                         refindx;                // Index of the voice in it's sound driver
    uint8_t                         track;                  // What track is using the voice
    uint8_t                         priority;               // Inherited from the parent track: used to determine when voices are 'stolen' or not when the voice limit is reached
    uint8_t                         keynum;                 // Which note (semitone) the voice is to play
    uint8_t                         velnum;                 // Volume the voice is to play at
    SoundClass                      sndtype;                // What broad class of sound the voice is being used for (MUSIC, SFX etc.)
    VmPtr<const patchmaps_header>   patchmaps;              // 0x008    TODO: COMMENT
    VmPtr<const patchinfo_header>   patchinfo;              // 0x00C    TODO: COMMENT
    uint32_t                        pabstime;               // When the voice should be released and fully shut off (in absolute time)
    uint32_t                        adsr2;                  // 0x014    TODO: COMMENT
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
    VmPtr<module_data>          pmod_info;                  // 0x00C    TODO: COMMENT
    VmPtr<callback_status>      pcalltable;                 // A list of user callbacks which can be invoked by 'Eng_StatusMark' commands
    VmPtr<uint8_t>              pmaster_volume;             // Master volume level for the sequencer: written to but appears otherwise unused
    VmPtr<patch_group_data>     ppat_info;                  // 0x018    TODO: COMMENT
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
