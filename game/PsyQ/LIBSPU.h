#pragma once

#include <cstdint>

// The number of hardware SPU voices.
// Note: this is not an original PsyQ constant - this is one I've made up.
static constexpr uint32_t SPU_NUM_VOICES = 24;

// Mode of operation for 'LIBSPU_SpuIsTransferCompleted'
enum SpuTransferQuery : int32_t {
    SPU_TRANSFER_PEEK = 0,      // Only check whether the transfer has completed, do not block
    SPU_TRANSFER_WAIT = 1       // Wait until the transfer completes
};

// Transfer modes for for 'LIBSPU_SpuSetTransferMode'
enum SpuTransferMode : int32_t {
    SPU_TRANSFER_BY_DMA = 0,    // Transfer data to the SPU using DMA
    SPU_TRANSFER_BY_IO  = 1     // Transfer data to the SPU using synchronous writes by the CPU (slower)
};

// Holds volume levels for left and right channels
struct SpuVolume {
    int16_t     left;
    int16_t     right;
};

// Settings for an external input
struct SpuExtAttr {
    SpuVolume   volume;     // Volume level
    int32_t     reverb;     // Reverb enabled/disabled
    int32_t     mix;        // Mix enabled/disabled
};

// Common sound attributes/settings
struct SpuCommonAttr {
    uint32_t    mask;       // Bitmask for what attributes are being set/applied
    SpuVolume   mvol;       // Master volume setting
    SpuVolume   mvolmode;   // Master volume mode
    SpuVolume   mvolx;      // The current master volume level
    SpuExtAttr  cd;         // CD input settings
    SpuExtAttr  ext;        // External digital input settings
};

// Flags for specifying fields or sub-fields in 'SpuCommonAttr'
static constexpr uint32_t SPU_COMMON_MVOLL      = 0x00000001;   // Master volume (left channel)
static constexpr uint32_t SPU_COMMON_MVOLR      = 0x00000002;   // Master volume (right channel)
static constexpr uint32_t SPU_COMMON_MVOLMODEL  = 0x00000004;   // Master volume mode (left channel)
static constexpr uint32_t SPU_COMMON_MVOLMODER  = 0x00000008;   // Master volume mode (right channel)
static constexpr uint32_t SPU_COMMON_RVOLL      = 0x00000010;   // Reverb volume (left channel)
static constexpr uint32_t SPU_COMMON_RVOLR      = 0x00000020;   // Reverb volume (right channel)
static constexpr uint32_t SPU_COMMON_CDVOLL     = 0x00000040;   // CD input volume (left channel)
static constexpr uint32_t SPU_COMMON_CDVOLR     = 0x00000080;   // CD input volume (right channel)
static constexpr uint32_t SPU_COMMON_CDREV      = 0x00000100;   // Whether CD input reverb is enabled
static constexpr uint32_t SPU_COMMON_CDMIX      = 0x00000200;   // Whether CD input can be heard (is mixed with all other sounds)
static constexpr uint32_t SPU_COMMON_EXTVOLL    = 0x00000400;   // External input volume (left channel)
static constexpr uint32_t SPU_COMMON_EXTVOLR    = 0x00000800;   // External input volume (right channel)
static constexpr uint32_t SPU_COMMON_EXTREV     = 0x00001000;   // Whether reverb is enabled for external input
static constexpr uint32_t SPU_COMMON_EXTMIX     = 0x00002000;   // Whether external input can be heard (is mixed with all other sounds)

// Structure used for specifying voice attributes.
//
// ADSR envelope bit meanings (32-bits):
//  0-4     Sustain level
//  5-8     Decay rate
//  9-14    Attack rate
//  15      Attack rate mode (0 = linear, 1 = exponential)
//  16-20   Release rate
//  21      Release rate mode (0 = linear, 1 = exponential)
//  22-28   Sustain rate
//  29      Unused
//  30      Sustain rate sign (0 = positive, 1 = negative)
//  31      Sustain rate mode (0 = linear, 1 = exponential)
//
struct SpuVoiceAttr {
    uint32_t    voice_bits;     // Which voices to set the attributes for. 1-bit per voice, starting with the lowest bit.
    uint32_t    attr_mask;      // Which attributes to modify? 1-bit per attribute, starting with the lowest bit.
    SpuVolume   volume;         // Volume
    SpuVolume   volmode;        // Volume mode
    SpuVolume   volumex;        // Current volume
    uint16_t    pitch;          // Tone: pitch setting (set pitch/sample-rate directly)
    uint16_t    note;           // Tone: note setting (pitch is computed using this and the base note)
    uint16_t    sample_note;    // Waveform data: base note setting (the note which is regarded to be at 44,100 Hz)
    int16_t     envx;           // Envelope volume
    uint32_t    addr;           // Start address in SPU RAM for waveform data (note: this is rounded up to the next 8 byte boundary upon setting)
    uint32_t    loop_addr;      // Start address in SPU RAM for loop  (note: this is rounded up to the next 8 byte boundary upon setting)
    int32_t     a_mode;         // Attack rate volume mode
    int32_t     s_mode;         // Sustain rate volume mode
    int32_t     r_mode;         // Release rate volume mode
    uint16_t    ar;             // Attack rate
    uint16_t    dr;             // Decay rate
    uint16_t    sr;             // Sustain rate
    uint16_t    rr;             // Release rate
    uint16_t    sl;             // Sustain level
    uint16_t    adsr1;          // Envelope adsr (1st 16-bits, see above)
    uint16_t    adsr2;          // Envelope adsr (2nd 16-bits, see above)
};

// Flags for specifying fields or sub-fields in 'SpuVoiceAttr'
static constexpr uint32_t SPU_VOICE_VOLL        = 0x00000001;
static constexpr uint32_t SPU_VOICE_VOLR        = 0x00000002;
static constexpr uint32_t SPU_VOICE_VOLMODEL    = 0x00000004;
static constexpr uint32_t SPU_VOICE_VOLMODER    = 0x00000008;
static constexpr uint32_t SPU_VOICE_PITCH       = 0x00000010;
static constexpr uint32_t SPU_VOICE_NOTE        = 0x00000020;
static constexpr uint32_t SPU_VOICE_SAMPLE_NOTE = 0x00000040;   // Set the base note (the 44,100 Hz note)
static constexpr uint32_t SPU_VOICE_WDSA        = 0x00000080;   // Waveform data start address
static constexpr uint32_t SPU_VOICE_ADSR_AMODE  = 0x00000100;
static constexpr uint32_t SPU_VOICE_ADSR_SMODE  = 0x00000200;
static constexpr uint32_t SPU_VOICE_ADSR_RMODE  = 0x00000400;
static constexpr uint32_t SPU_VOICE_ADSR_AR     = 0x00000800;
static constexpr uint32_t SPU_VOICE_ADSR_DR     = 0x00001000;
static constexpr uint32_t SPU_VOICE_ADSR_SR     = 0x00002000;
static constexpr uint32_t SPU_VOICE_ADSR_RR     = 0x00004000;
static constexpr uint32_t SPU_VOICE_ADSR_SL     = 0x00008000;
static constexpr uint32_t SPU_VOICE_LSAX        = 0x00010000;   // Loop address
static constexpr uint32_t SPU_VOICE_ADSR_ADSR1  = 0x00020000;
static constexpr uint32_t SPU_VOICE_ADSR_ADSR2  = 0x00040000;

// Spu reverb modes
enum SpuReverbMode : int32_t {
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
static constexpr uint32_t SPU_REVF_APF_OFFSET_1                     = 0x00000001;
static constexpr uint32_t SPU_REVF_APF_OFFSET_2                     = 0x00000002;
static constexpr uint32_t SPU_REVF_REFLECTION_VOLUME_1              = 0x00000004;
static constexpr uint32_t SPU_REVF_COMB_VOLUME_1                    = 0x00000008;
static constexpr uint32_t SPU_REVF_COMB_VOLUME_2                    = 0x00000010;
static constexpr uint32_t SPU_REVF_COMB_VOLUME_3                    = 0x00000020;
static constexpr uint32_t SPU_REVF_COMB_VOLUME_4                    = 0x00000040;
static constexpr uint32_t SPU_REVF_REFLECTION_VOLUME_2              = 0x00000080;
static constexpr uint32_t SPU_REVF_APF_VOLUME_1                     = 0x00000100;
static constexpr uint32_t SPU_REVF_APF_VOLUME_2                     = 0x00000200;
static constexpr uint32_t SPU_REVF_SAME_SIDE_REFRACT_ADDR_1_LEFT    = 0x00000400;
static constexpr uint32_t SPU_REVF_SAME_SIDE_REFRACT_ADDR_1_RIGHT   = 0x00000800;
static constexpr uint32_t SPU_REVF_COMB_ADDR_1_LEFT                 = 0x00001000;
static constexpr uint32_t SPU_REVF_COMB_ADDR_1_RIGHT                = 0x00002000;
static constexpr uint32_t SPU_REVF_COMB_ADDR_2_LEFT                 = 0x00004000;
static constexpr uint32_t SPU_REVF_COMB_ADDR_2_RIGHT                = 0x00008000;
static constexpr uint32_t SPU_REVF_SAME_SIDE_REFRACT_ADDR_2_LEFT    = 0x00010000;
static constexpr uint32_t SPU_REVF_SAME_SIDE_REFRACT_ADDR_2_RIGHT   = 0x00020000;
static constexpr uint32_t SPU_REVF_DIFF_SIDE_REFLECT_ADDR_1_LEFT    = 0x00040000;
static constexpr uint32_t SPU_REVF_DIFF_SIDE_REFLECT_ADDR_1_RIGHT   = 0x00080000;
static constexpr uint32_t SPU_REVF_COMB_ADDR_3_LEFT                 = 0x00100000;
static constexpr uint32_t SPU_REVF_COMB_ADDR_3_RIGHT                = 0x00200000;
static constexpr uint32_t SPU_REVF_COMB_ADDR_4_LEFT                 = 0x00400000;
static constexpr uint32_t SPU_REVF_COMB_ADDR_4_RIGHT                = 0x00800000;
static constexpr uint32_t SPU_REVF_DIFF_SIDE_REFLECT_ADDR_2_LEFT    = 0x01000000;
static constexpr uint32_t SPU_REVF_DIFF_SIDE_REFLECT_ADDR_2_RIGHT   = 0x02000000;
static constexpr uint32_t SPU_REVF_APF_ADDR_1_LEFT                  = 0x04000000;
static constexpr uint32_t SPU_REVF_APF_ADDR_1_RIGHT                 = 0x08000000;
static constexpr uint32_t SPU_REVF_APF_ADDR_2_LEFT                  = 0x10000000;
static constexpr uint32_t SPU_REVF_APF_ADDR_2_RIGHT                 = 0x20000000;
static constexpr uint32_t SPU_REVF_INPUT_VOL_LEFT                   = 0x40000000;
static constexpr uint32_t SPU_REVF_INPUT_VOL_RIGHT                  = 0x80000000;

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

// Volume change modes or rates of change for volume envelopes
static constexpr int32_t SPU_VOICE_DIRECT     = 0;      // Directly set volume to a fixed amount. If amount is negative then the phase for the wave is inverted.
static constexpr int32_t SPU_VOICE_LINEARIncN = 1;      // Linear increase (normal phase): when volume is positive, increase linearly to the maximum amount.
static constexpr int32_t SPU_VOICE_LINEARIncR = 2;      // Linear increase (revese phase): when volume is negative, increase linearly to the maximum amount (with phase inverted)
static constexpr int32_t SPU_VOICE_LINEARDecN = 3;      // Linear decrease (normal phase): when volume is positive, decrease linearly to the minimum amount.
static constexpr int32_t SPU_VOICE_LINEARDecR = 4;      // Linear decrease (revese phase): when volume is negative, decrease linearly to the minimum amount (with phase inverted)
static constexpr int32_t SPU_VOICE_EXPIncN    = 5;      // Exponential increase (normal phase): when volume is positive, increase exponentially to the maximum amount.
static constexpr int32_t SPU_VOICE_EXPIncR    = 6;      // Exponential increase (reverse phase): when volume is negative, increase exponentially to the maximum amount.
static constexpr int32_t SPU_VOICE_EXPDec     = 7;      // Exponential decrease: when volume is positive or negative, decrease exponentially to the mininum amount.

// A bitmask representing the bits for all 24 channels
static constexpr int32_t SPU_ALLCH = 0x00FFFFFF;

// Size of a malloc record for 'LIBSPU_SpuMalloc' (function which is not present here)
static constexpr int32_t SPU_MALLOC_RECSIZ = 8;

void LIBSPU_SpuSetVoiceAttr(const SpuVoiceAttr& attribs) noexcept;
int32_t LIBSPU_SpuSetReverbModeParam(const SpuReverbAttr& reverbAttr) noexcept;
void LIBSPU_SpuSetCommonAttr(const SpuCommonAttr& attribs) noexcept;
uint32_t LIBSPU_SpuGetReverbOffsetAddr() noexcept;
int32_t LIBSPU_SpuClearReverbWorkArea() noexcept;
void LIBSPU_SpuStart() noexcept;
void LIBSPU_SpuSetReverbDepth(const SpuReverbAttr& reverb) noexcept;
int32_t LIBSPU_SpuSetReverbVoice(const int32_t onOff, const int32_t voiceBits) noexcept;
void LIBSPU_SpuInit() noexcept;
int32_t LIBSPU_SpuSetReverb(const int32_t onOff) noexcept;
void LIBSPU_SpuQuit() noexcept;
bool LIBSPU_SpuIsTransferCompleted(const SpuTransferQuery mode) noexcept;
void LIBSPU_SpuInitMalloc(const int32_t maxAllocs, uint8_t* const pMemMangementTbl) noexcept;
void LIBSPU_SpuSetTransferMode(const SpuTransferMode mode) noexcept;
uint32_t LIBSPU_SpuSetTransferStartAddr(const uint32_t addr) noexcept;
uint32_t LIBSPU_SpuWrite(const void* const pData, const uint32_t size) noexcept;
void LIBSPU_SpuSetKeyOnWithAttr(const SpuVoiceAttr& attribs) noexcept;
void LIBSPU_SpuSetKey(const int32_t onOff, const uint32_t voiceBits) noexcept;
void LIBSPU_SpuGetAllKeysStatus(uint8_t statuses[SPU_NUM_VOICES]) noexcept;
