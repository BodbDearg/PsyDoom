#pragma once

#include "psxcd.h"
#include "PcPsx/Types.h"

// Setting ids for sound hardware
enum SoundHardwareTags : uint32_t {
    SNDHW_TAG_END               = 0,
    SNDHW_TAG_DRIVER_ID         = 1,
    SNDHW_TAG_SOUND_EFFECTS     = 2,
    SNDHW_TAG_MUSIC             = 3,
    SNDHW_TAG_DRUMS             = 4,
    SNDHW_TAG_MAX               = 5
};

// Sound driver ids
enum SoundDriverIds : uint8_t {
    NoSound_ID  = 0,        // No sound driver
    PSX_ID      = 1,        // Sony PlayStation sound driver
    GENERIC_ID  = 50        // Generic hardware agnostic sound driver
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

// TODO: COMMENT
struct track_header {
    uint8_t     voices_type;            // 0x000    TODO: COMMENT
    uint8_t     voices_max;             // 0x001    TODO: COMMENT
    uint8_t     priority;               // 0x002    TODO: COMMENT
    uint8_t     lockchannel;            // 0x003    TODO: COMMENT
    uint8_t     voices_class;           // 0x004    TODO: COMMENT
    uint8_t     reverb;                 // 0x005    TODO: COMMENT
    uint16_t    initpatchnum;           // 0x006    TODO: COMMENT
    uint16_t    initpitch_cntrl;        // 0x008    TODO: COMMENT
    uint8_t     initvolume_cntrl;       // 0x00A    TODO: COMMENT
    uint8_t     initpan_cntrl;          // 0x00B    TODO: COMMENT
    uint8_t     substack_count;         // 0x00C    TODO: COMMENT
    uint8_t     mutebits;               // 0x00D    TODO: COMMENT
    uint16_t    initppq;                // 0x00E    TODO: COMMENT
    uint16_t    initqpm;                // 0x010    TODO: COMMENT
    uint16_t    labellist_count;        // 0x012    TODO: COMMENT
    uint32_t    data_size;              // 0x014    TODO: COMMENT
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
    uint16_t    unk1;       // 0x002    TODO: COMMENT
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
typedef VmPtr<void()> callfunc_t;

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
    uint32_t    load_flags;         // 0x000    TODO: COMMENT
    uint8_t     patch_id;           // 0x004    TODO: COMMENT
    uint8_t     hw_voice_limit;     // 0x005    TODO: COMMENT
    uint16_t    pad1;               // 0x006    TODO: COMMENT
    uint16_t    patches;            // 0x008    TODO: COMMENT
    uint16_t    patch_size;         // 0x00A    TODO: COMMENT
    uint16_t    patchmaps;          // 0x00C    TODO: COMMENT
    uint16_t    patchmap_size;      // 0x00E    TODO: COMMENT
    uint16_t    patchinfo;          // 0x010    TODO: COMMENT
    uint16_t    patchinfo_size;     // 0x012    TODO: COMMENT
    uint16_t    drummaps;           // 0x014    TODO: COMMENT
    uint16_t    drummap_size;       // 0x016    TODO: COMMENT
    uint32_t    extra_data_size;    // 0x018    TODO: COMMENT
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
    uint8_t         flags;                  // 0x000    TODO: COMMENT
    uint8_t         playmode;               // 0x001    TODO: COMMENT
    uint16_t        seq_num;                // 0x002    TODO: COMMENT
    uint8_t         tracks_active;          // 0x004    TODO: COMMENT
    uint8_t         tracks_playing;         // 0x005    TODO: COMMENT
    uint8_t         volume;                 // 0x006    TODO: COMMENT
    uint8_t         pan;                    // 0x007    TODO: COMMENT
    uint32_t        seq_type;               // 0x008    TODO: COMMENT
    VmPtr<char>     ptrk_indxs;             // 0x00C    TODO: COMMENT
    VmPtr<char>     pgates;                 // 0x010    TODO: COMMENT
    VmPtr<char>     piters;                 // 0x014    TODO: COMMENT
};

static_assert(sizeof(sequence_status) == 24);

// TODO: COMMENT
struct track_status {
    uint8_t             flags;                  // 0x000    TODO: COMMENT
    uint8_t             refindx;                // 0x001    TODO: COMMENT
    uint8_t             seq_owner;              // 0x002    TODO: COMMENT
    uint8_t             patchtype;              // 0x003    TODO: COMMENT
    uint32_t            deltatime;              // 0x004    TODO: COMMENT
    uint8_t             priority;               // 0x008    TODO: COMMENT
    uint8_t             reverb;                 // 0x009    TODO: COMMENT
    uint16_t            patchnum;               // 0x00A    TODO: COMMENT
    uint8_t             volume_cntrl;           // 0x00C    TODO: COMMENT
    uint8_t             pan_cntrl;              // 0x00D    TODO: COMMENT
    int16_t             pitch_cntrl;            // 0x00E    TODO: COMMENT
    uint8_t             voices_active;          // 0x010    TODO: COMMENT
    uint8_t             voices_max;             // 0x011    TODO: COMMENT
    uint8_t             mutemask;               // 0x012    TODO: COMMENT
    uint8_t             sndclass;               // 0x013    TODO: COMMENT
    uint16_t            ppq;                    // 0x014    TODO: COMMENT
    uint16_t            qpm;                    // 0x016    TODO: COMMENT
    uint16_t            labellist_count;        // 0x018    TODO: COMMENT
    uint16_t            labellist_max;          // 0x01A    TODO: COMMENT
    uint32_t            ppi;                    // 0x01C    TODO: COMMENT
    uint32_t            starppi;                // 0x020    TODO: COMMENT
    uint32_t            accppi;                 // 0x024    TODO: COMMENT
    uint32_t            totppi;                 // 0x028    TODO: COMMENT
    uint32_t            endppi;                 // 0x02C    TODO: COMMENT
    VmPtr<uint8_t>      pstart;                 // 0x030    TODO: COMMENT
    VmPtr<uint8_t>      ppos;                   // 0x034    TODO: COMMENT
    VmPtr<uint32_t>     plabellist;             // 0x038    TODO: COMMENT
    VmPtr<uint32_t>     psubstack;              // 0x03C    TODO: COMMENT
    VmPtr<uint32_t>     psp;                    // 0x040    TODO: COMMENT
    VmPtr<uint32_t>     pstackend;              // 0x044    TODO: COMMENT
    uint32_t            data_size;              // 0x048    TODO: COMMENT
    uint32_t            data_space;             // 0x04C    TODO: COMMENT
};

static_assert(sizeof(track_status) == 80);

// TODO: COMMENT
struct voice_status {
    uint8_t                     flags;          // 0x000    TODO: COMMENT
    uint8_t                     patchtype;      // 0x001    TODO: COMMENT
    uint8_t                     refindx;        // 0x002    TODO: COMMENT
    uint8_t                     track;          // 0x003    TODO: COMMENT
    uint8_t                     priority;       // 0x004    TODO: COMMENT
    uint8_t                     keynum;         // 0x005    TODO: COMMENT
    uint8_t                     velnum;         // 0x006    TODO: COMMENT
    uint8_t                     sndtype;        // 0x007    TODO: COMMENT
    VmPtr<patchmaps_header>     patchmaps;      // 0x008    TODO: COMMENT
    VmPtr<patchinfo_header>     patchinfo;      // 0x00C    TODO: COMMENT
    uint32_t                    pabstime;       // 0x010    TODO: COMMENT
    uint32_t                    adsr2;          // 0x014    TODO: COMMENT
};

static_assert(sizeof(voice_status) == 24);

// TODO: COMMENT
struct master_status_structure {
    VmPtr<uint32_t>             pabstime;                   // 0x000    TODO: COMMENT
    uint8_t                     seqs_active;                // 0x004    TODO: COMMENT
    uint8_t                     trks_active;                // 0x005    TODO: COMMENT
    uint8_t                     voices_active;              // 0x006    TODO: COMMENT
    uint8_t                     voices_total;               // 0x007    TODO: COMMENT
    uint8_t                     patch_types_loaded;         // 0x008    TODO: COMMENT
    uint8_t                     unk1;                       // 0x009    TODO: COMMENT
    uint8_t                     callbacks_active;           // 0x00A    TODO: COMMENT
    uint8_t                     unk3;                       // 0x00B    TODO: COMMENT
    VmPtr<module_data>          pmod_info;                  // 0x00C    TODO: COMMENT
    VmPtr<callback_status>      pcalltable;                 // 0x010    TODO: COMMENT
    VmPtr<uint8_t>              pmaster_volume;             // 0x014    TODO: COMMENT
    VmPtr<patch_group_data>     ppat_info;                  // 0x018    TODO: COMMENT
    uint32_t                    max_trks_perseq;            // 0x01C    TODO: COMMENT
    VmPtr<sequence_status>      pseqstattbl;                // 0x020    TODO: COMMENT
    uint32_t                    max_substack_pertrk;        // 0x024    TODO: COMMENT
    VmPtr<track_status>         ptrkstattbl;                // 0x028    TODO: COMMENT
    uint32_t                    max_voices_pertrk;          // 0x02C    TODO: COMMENT
    VmPtr<voice_status>         pvoicestattbl;              // 0x030    TODO: COMMENT
    uint32_t                    fp_module;                  // 0x034    TODO: COMMENT
};

static_assert(sizeof(master_status_structure) == 56);

extern const VmPtr<uint32_t>                    gWess_Millicount;
extern const VmPtr<uint32_t>                    gWess_Millicount_Frac;
extern const VmPtr<bool32_t>                    gbWess_WessTimerActive;
extern const VmPtr<uint8_t[CD_SECTOR_SIZE]>     gWess_sectorBuffer1;
extern const VmPtr<uint8_t[CD_SECTOR_SIZE]>     gWess_sectorBuffer2;
extern const VmPtr<bool32_t>                    gbWess_SeqOn;

int16_t GetIntsPerSec() noexcept;
void CalcPartsPerInt() noexcept;
int32_t WessInterruptHandler() noexcept;
void init_WessTimer() noexcept;
void exit_WessTimer() noexcept;
bool Wess_init_for_LoadFileData() noexcept;
PsxCd_File* module_open(const CdMapTbl_File fileId) noexcept;
int32_t module_read(void* const pDest, const int32_t numBytes, PsxCd_File& file) noexcept;
int32_t module_seek(PsxCd_File& file, const int32_t seekPos, const PsxCd_SeekMode seekMode) noexcept;
int32_t module_tell(const PsxCd_File& file) noexcept;
void module_close(PsxCd_File& file) noexcept;
int32_t get_num_Wess_Sound_Drivers(VmPtr<int32_t>* const pSettingsTagLists) noexcept;
PsxCd_File* data_open(const CdMapTbl_File fileId) noexcept;
int32_t data_read(PsxCd_File& file, const int32_t destSpuAddr, const int32_t numBytes, const int32_t fileOffset) noexcept;
void data_close(PsxCd_File& file) noexcept;
void wess_low_level_init() noexcept;
void wess_low_level_exit() noexcept;
void* wess_malloc(const int32_t size) noexcept;
void wess_free(void* const pMem) noexcept;
