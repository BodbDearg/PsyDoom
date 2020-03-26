#pragma once

#include "PcPsx/Types.h"
#include "PsxVm/VmPtr.h"

struct master_status_structure;
struct sequence_data;
struct track_data;
struct track_status;

// Enum representing the current high level state of a sequence
enum SequenceStatus : uint8_t {
    SEQUENCE_INVALID    = 0,    // Invalid sequence number
    SEQUENCE_INACTIVE   = 1,    // Not paused or playing
    SEQUENCE_STOPPED    = 2,    // Paused
    SEQUENCE_PLAYING    = 3     // Playing
};

// Mask flags for 'TriggerPlayAttr': defines which fields are to be used from that struct
static constexpr uint32_t TRIGGER_VOLUME    = 0x1;
static constexpr uint32_t TRIGGER_PAN       = 0x2;
static constexpr uint32_t TRIGGER_PATCH     = 0x4;
static constexpr uint32_t TRIGGER_PITCH     = 0x8;
static constexpr uint32_t TRIGGER_MUTEMODE  = 0x10;
static constexpr uint32_t TRIGGER_TEMPO     = 0x20;
static constexpr uint32_t TRIGGER_TIMED     = 0x40;
static constexpr uint32_t TRIGGER_LOOPED    = 0x80;
static constexpr uint32_t TRIGGER_REVERB    = 0x100;

// Struct defining custom parameters for when triggering a sequence.
// The 'mask' field determines what parameters are customized.
struct TriggerPlayAttr {
    uint32_t    mask;
    uint8_t     volume;         // Range: 0-127
    uint8_t     pan;            // Range: 0-127, with '64' being the center
    int16_t     patch;          // Range: 0-32767
    int16_t     pitch;          // Range: -8192 to 8191
    uint8_t     mutemode;       // Range: 0-7
    uint8_t     reverb;
    uint16_t    tempo;
    uint16_t    _padding;
    uint32_t    timeppq;
};

static_assert(sizeof(TriggerPlayAttr) == 20);

extern const VmPtr<bool32_t>    gbWess_module_loaded;

void zeroset(void* const pDest, const uint32_t numBytes) noexcept;
void wess_install_error_handler(int32_t (* const pErrorFunc)(int32_t, int32_t), const int32_t module) noexcept;
master_status_structure* wess_get_master_status() noexcept;
bool Is_System_Active() noexcept;
bool Is_Module_Loaded() noexcept;
bool Is_Seq_Num_Valid(const int32_t seqNum) noexcept;
void Register_Early_Exit() noexcept;
void wess_install_handler() noexcept;
void wess_restore_handler() noexcept;
bool wess_init() noexcept;
void wess_exit(bool bForceRestoreTimerHandler) noexcept;
uint8_t* wess_get_wmd_start() noexcept;
uint8_t* wess_get_wmd_end() noexcept;
void free_mem_if_mine() noexcept;
void wess_unload_module() noexcept;

int32_t wess_load_module(
    const void* const pWmdFile,
    void* const pDestMem,
    const int32_t memoryAllowance,
    VmPtr<int32_t>* const pSettingTagLists
) noexcept;

void filltrackstat(track_status& trackStat, const track_data& trackInfo, const TriggerPlayAttr* const pAttribs) noexcept;
void assigntrackstat(track_status& trackStat, const track_data& trackInfo) noexcept;

int32_t wess_seq_structrig(
    const sequence_data& seqInfo,
    const int32_t seqNum,
    const int32_t seqType,
    const int32_t getHandle,
    const TriggerPlayAttr* pPlayAttribs
) noexcept;

void wess_seq_trigger() noexcept;
void wess_seq_trigger_special() noexcept;
SequenceStatus wess_seq_status(const int32_t seqNum) noexcept;
void wess_seq_stop(const int32_t seqNum) noexcept;
void wess_seq_stopall() noexcept;
