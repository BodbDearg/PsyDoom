#pragma once

#include <cstdint>

// The number of hardware SPU voices.
// Note: this is not an original PsyQ constant - this is one I've made up.
static constexpr uint32_t SPU_NUM_VOICES = 24;

// Mode of operation for 'LIBSPU_SpuIsTransferCompleted'
enum SpuTransferQuery : uint32_t {
    SPU_TRANSFER_PEEK = 0,      // Only check whether the transfer has completed, do not block
    SPU_TRANSFER_WAIT = 1       // Wait until the transfer completes
};

// Transfer modes for for 'LIBSPU_SpuSetTransferMode'
enum SpuTransferMode : uint32_t {
    SPU_TRANSFER_BY_DMA = 0,    // Transfer data to the SPU using DMA
    SPU_TRANSFER_BY_IO  = 1     // Transfer data to the SPU using synchronous writes by the CPU (slower)
};

// Holds volume levels for left and right channels
struct SpuVolume {
    int16_t     left;
    int16_t     right;
};

// Structure used for specifying voice attributes
struct SpuVoiceAttr {
    uint32_t    voice_bits;     // Which voices to set the attributes for. 1-bit per voice, starting with the lowest bit.
    uint32_t    attr_mask;      // Which attributes to modify? 1-bit per attribute, starting with the lowest bit.
    SpuVolume   volume;         // Volume
    SpuVolume   volmode;        // Volume mode
    SpuVolume   volumex;        // Current volume
    uint16_t    pitch;          // Tone: pitch setting
    uint16_t    note;           // Tone: note setting
    uint16_t    sample_note;    // Waveform data: note setting
    int16_t     envx;           // Envelope volume
    uint32_t    addr;           // Start address in SPU RAM for waveform data
    uint32_t    loop_addr;      // Start address in SPU RAM for loop
    int32_t     a_mode;         // Attack rate mode
    int32_t     s_mode;         // Sustain rate mode
    int32_t     r_mode;         // Release rate mode
    uint16_t    ar;             // Attack rate
    uint16_t    dr;             // Decay rate
    uint16_t    sr;             // Sustain rate
    uint16_t    rr;             // Release rate
    uint16_t    sl;             // Sustain level
    uint16_t    adsr1;          // Envelope 1 adsr
    uint16_t    adsr2;          // Envelope 2 adsr
};

// Flags for specifying fields in 'SpuVoiceAttr'
static constexpr uint32_t SPU_VOICE_VOLL        = 0x00000001;
static constexpr uint32_t SPU_VOICE_VOLR        = 0x00000002;
static constexpr uint32_t SPU_VOICE_VOLMODEL    = 0x00000004;
static constexpr uint32_t SPU_VOICE_VOLMODER    = 0x00000008;
static constexpr uint32_t SPU_VOICE_PITCH       = 0x00000010;
static constexpr uint32_t SPU_VOICE_NOTE        = 0x00000020;
static constexpr uint32_t SPU_VOICE_SAMPLE_NOTE = 0x00000040;
static constexpr uint32_t SPU_VOICE_WDSA        = 0x00000080;
static constexpr uint32_t SPU_VOICE_ADSR_AMODE  = 0x00000100;
static constexpr uint32_t SPU_VOICE_ADSR_SMODE  = 0x00000200;
static constexpr uint32_t SPU_VOICE_ADSR_RMODE  = 0x00000400;
static constexpr uint32_t SPU_VOICE_ADSR_AR     = 0x00000800;
static constexpr uint32_t SPU_VOICE_ADSR_DR     = 0x00001000;
static constexpr uint32_t SPU_VOICE_ADSR_SR     = 0x00002000;
static constexpr uint32_t SPU_VOICE_ADSR_RR     = 0x00004000;
static constexpr uint32_t SPU_VOICE_ADSR_SL     = 0x00008000;
static constexpr uint32_t SPU_VOICE_LSAX        = 0x00010000;
static constexpr uint32_t SPU_VOICE_ADSR_ADSR1  = 0x00020000;
static constexpr uint32_t SPU_VOICE_ADSR_ADSR2  = 0x00040000;

// Spu reverb modes
enum SpuReverbMode : uint32_t {
    SPU_REV_MODE_OFF        = 0,
    SPU_REV_MODE_ROOM       = 1,
    SPU_REV_MODE_STUDIO_A   = 2,
    SPU_REV_MODE_STUDIO_B   = 3,
    SPU_REV_MODE_STUDIO_C   = 4,
    SPU_REV_MODE_HALL       = 5,
    SPU_REV_MODE_SPACE      = 6,
    SPU_REV_MODE_ECHO       = 7,
    SPU_REV_MODE_DELAY      = 8,
    SPU_REV_MODE_PIPE       = 9,
    SPU_REV_MODE_MAX        = 10
};

// A flag appended to 'SpuReverbMode' to indicate that the reverb working area in SPU RAM should be cleared when setting the reverb mode
static constexpr uint32_t SPU_REV_MODE_CLEAR_WA = 0x100;

// Structure holding various reverb related settings
struct SpuReverbAttr {
    uint32_t        mask;           // Which reverb properties to set for commands that use this structure (e.g 'SPU_REV_DEPTHL' or 'SPU_REV_MODE')
    SpuReverbMode   mode;           // What type of reverb to use
    SpuVolume       depth;          // Depth/loudness of reverb echo
    int32_t         delay;          // Delay in reverb echo
    int32_t         feedback;       // Reverb echo feedback
};

// An internal LIBSPU struct used to hold a definition for a reverb type, before sending the settings to the SPU.
// All of these fields (except the 'paramBits' field) will map directly to SPU registers controlling reverb.
// For more details on what the reverb fields mean, see the NO$PSX specs:
//  https://problemkaputt.de/psx-spx.htm#spureverbregisters
//
struct SpuReverbDef {
    uint32_t    fieldBits;      // Not a reverb setting: defines which of the reverb registers should be set (1 bit flag for each of the 32 registers)
    uint16_t    apfOffset1;
    uint16_t    apfOffset2;
    uint16_t    reflectionVolume1;
    uint16_t    combVolume1;
    uint16_t    combVolume2;
    uint16_t    combVolume3;
    uint16_t    combVolume4;
    uint16_t    reflectionVolume2;
    uint16_t    apfVolume1;
    uint16_t    apfVolume2;
    uint16_t    sameSideRefractAddr1Left;
    uint16_t    sameSideRefractAddr1Right;
    uint16_t    combAddr1Left;
    uint16_t    combAddr1Right;
    uint16_t    combAddr2Left;
    uint16_t    combAddr2Right;
    uint16_t    sameSideRefractAddr2Left;
    uint16_t    sameSideRefractAddr2Right;
    uint16_t    diffSideReflectAddr1Left;
    uint16_t    diffSideReflectAddr1Right;
    uint16_t    combAddr3Left;
    uint16_t    combAddr3Right;
    uint16_t    combAddr4Left;
    uint16_t    combAddr4Right;
    uint16_t    diffSideReflectAddr2Left;
    uint16_t    diffSideReflectAddr2Right;
    uint16_t    apfAddr1Left;
    uint16_t    apfAddr1Right;
    uint16_t    apfAddr2Left;
    uint16_t    apfAddr2Right;
    uint16_t    inputVolLeft;
    uint16_t    inputVolRight;
};

// Bit flags representing each of the reverb fields in 'SpuReverbDef' (except 'fieldBits')
static constexpr uint32_t SPU_REVF_APF_OFFSET_1                 = 0x00000001;
static constexpr uint32_t SPU_REVF_APF_OFFSET_2                 = 0x00000002;
static constexpr uint32_t SPU_REVF_REFLECTION_VOLUME_1          = 0x00000004;
static constexpr uint32_t SPU_REVF_COMB_VOLUME_1                = 0x00000008;
static constexpr uint32_t SPU_REVF_COMB_VOLUME_2                = 0x00000010;
static constexpr uint32_t SPU_REVF_COMB_VOLUME_3                = 0x00000020;
static constexpr uint32_t SPU_REVF_COMB_VOLUME_4                = 0x00000040;
static constexpr uint32_t SPU_REVF_REFLECTION_VOLUME_2          = 0x00000080;
static constexpr uint32_t SPU_REVF_APF_VOLUME_1                 = 0x00000100;
static constexpr uint32_t SPU_REVF_APF_VOLUME_2                 = 0x00000200;
static constexpr uint32_t SPU_SAME_SIDE_REFRACT_ADDR_1_LEFT     = 0x00000400;
static constexpr uint32_t SPU_SAME_SIDE_REFRACT_ADDR_1_RIGHT    = 0x00000800;
static constexpr uint32_t SPU_COMB_ADDR_1_LEFT                  = 0x00001000;
static constexpr uint32_t SPU_COMB_ADDR_1_RIGHT                 = 0x00002000;
static constexpr uint32_t SPU_COMB_ADDR_2_LEFT                  = 0x00004000;
static constexpr uint32_t SPU_COMB_ADDR_2_RIGHT                 = 0x00008000;
static constexpr uint32_t SPU_SAME_SIDE_REFRACT_ADDR_2_LEFT     = 0x00010000;
static constexpr uint32_t SPU_SAME_SIDE_REFRACT_ADDR_2_RIGHT    = 0x00020000;
static constexpr uint32_t SPU_DIFF_SIDE_REFLECT_ADDR_1_LEFT     = 0x00040000;
static constexpr uint32_t SPU_DIFF_SIDE_REFLECT_ADDR_1_RIGHT    = 0x00080000;
static constexpr uint32_t SPU_COMB_ADDR_3_LEFT                  = 0x00100000;
static constexpr uint32_t SPU_COMB_ADDR_3_RIGHT                 = 0x00200000;
static constexpr uint32_t SPU_COMB_ADDR_4_LEFT                  = 0x00400000;
static constexpr uint32_t SPU_COMB_ADDR_4_RIGHT                 = 0x00800000;
static constexpr uint32_t SPU_DIFF_SIDE_REFLECT_ADDR_2_LEFT     = 0x01000000;
static constexpr uint32_t SPU_DIFF_SIDE_REFLECT_ADDR_2_RIGHT    = 0x02000000;
static constexpr uint32_t SPU_APF_ADDR_1_LEFT                   = 0x04000000;
static constexpr uint32_t SPU_APF_ADDR_1_RIGHT                  = 0x08000000;
static constexpr uint32_t SPU_APF_ADDR_2_LEFT                   = 0x10000000;
static constexpr uint32_t SPU_APF_ADDR_2_RIGHT                  = 0x20000000;
static constexpr uint32_t SPU_INPUT_VOL_LEFT                    = 0x40000000;
static constexpr uint32_t SPU_INPUT_VOL_RIGHT                   = 0x80000000;

// Return codes for some functions
static constexpr int32_t SPU_SUCCESS    = 0;
static constexpr int32_t SPU_ERROR      = -1;

// Generic on/off or true/false values
static constexpr int32_t SPU_OFF    = 0;
static constexpr int32_t SPU_ON     = 1;

// Returned for each voice by 'LIBSPU_SpuGetAllKeysStatus' to indicate a voice + envelope status
static constexpr int32_t SPU_OFF_ENV_ON = 2;
static constexpr int32_t SPU_ON_ENV_OFF = 3;

// Parameter for various functions.
// It means make changes to ALL voices indicated by a bitmask, not just voices for enabled or disabled bits.
static constexpr int32_t SPU_BIT    = 8;

// Which attributes in 'SpuReverbAttr' to use
static constexpr uint32_t SPU_REV_MODE      = 0x01;
static constexpr uint32_t SPU_REV_DEPTHL    = 0x02;
static constexpr uint32_t SPU_REV_DEPTHR    = 0x04;
static constexpr uint32_t SPU_REV_DELAYTIME = 0x08;
static constexpr uint32_t SPU_REV_FEEDBACK  = 0x10;

// Definitions for the 10 available reverb modes in the PsyQ SDK
extern const SpuReverbDef gReverbDefs[SPU_REV_MODE_MAX];

// The base address of the reverb working area for all 10 reverb modes, divided by '8'.
// Multiply by '8' to get the real address in SPU RAM where the reverb work area is located.
// Note that everything past that address in SPU RAM is reserved for reverb.
extern const uint16_t gReverbWorkAreaBaseAddrs[SPU_REV_MODE_MAX];

void LIBSPU_SpuSetVoiceAttr() noexcept;
void LIBSPU__SpuSetVoiceAttr() noexcept;
int32_t LIBSPU_SpuSetReverbModeParam(const SpuReverbAttr& reverbAttr) noexcept;
void LIBSPU__spu_init() noexcept;
void LIBSPU__spu_writeByIO() noexcept;
void LIBSPU__spu_setVoiceAttr() noexcept;
void LIBSPU_SpuSetCommonAttr() noexcept;
uint32_t LIBSPU_SpuGetReverbOffsetAddr() noexcept;
int32_t LIBSPU_SpuClearReverbWorkArea() noexcept;
void LIBSPU__SpuInit() noexcept;
void LIBSPU_SpuStart() noexcept;
void LIBSPU_SpuSetReverbDepth(const SpuReverbAttr& reverb) noexcept;
int32_t LIBSPU_SpuSetReverbVoice(const int32_t onOff, const int32_t voiceBits) noexcept;
void LIBSPU_SpuInit() noexcept;
int32_t LIBSPU_SpuSetReverb(const int32_t onOff) noexcept;
void LIBSPU_SpuQuit() noexcept;

bool LIBSPU_SpuIsTransferCompleted(const SpuTransferQuery mode) noexcept;
void _thunk_LIBSPU_SpuIsTransferCompleted() noexcept;

void LIBSPU_SpuInitMalloc(const int32_t maxAllocs, uint8_t* const pMemMangementTbl) noexcept;
void LIBSPU_SpuSetTransferMode(const SpuTransferMode mode) noexcept;

uint32_t LIBSPU_SpuSetTransferStartAddr(const uint32_t addr) noexcept;
void _thunk_LIBSPU_SpuSetTransferStartAddr() noexcept;

uint32_t LIBSPU_SpuWrite(const void* const pData, const uint32_t size) noexcept;
void _thunk_LIBSPU_SpuWrite() noexcept;

void LIBSPU_SpuSetKeyOnWithAttr() noexcept;
void LIBSPU_SpuSetKey(const int32_t onOff, const uint32_t voiceBits) noexcept;
void LIBSPU_SpuGetAllKeysStatus(uint8_t statuses[SPU_NUM_VOICES]) noexcept;
