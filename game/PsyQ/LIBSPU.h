#pragma once

#include <cstdint>

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

// Structure holding various reverb related settings
struct SpuReverbAttr {
    uint32_t    mask;           // Which reverb properties to set for commands that use this structure (e.g 'SPU_REV_DEPTHL' or 'SPU_REV_MODE')
    int32_t     mode;           // What type of reverb to use
    SpuVolume   depth;          // Depth/loudness of reverb echo
    int32_t     delay;          // Delay in reverb echo
    int32_t     feedback;       // Reverb echo feedback
};

// Return codes for some functions
static constexpr int32_t SPU_SUCCESS    = 0;
static constexpr int32_t SPU_ERROR      = -1;

// Generic on/off or true/false values
static constexpr int32_t SPU_OFF    = 0;
static constexpr int32_t SPU_ON     = 1;

// Parameter for various functions.
// It means make changes to ALL voices indicated by a bitmask, not just voices for enabled or disabled bits.
static constexpr int32_t SPU_BIT    = 8;

// Which attributes in 'SpuReverbAttr' to use
static constexpr uint32_t SPU_REV_MODE      = 0x01;
static constexpr uint32_t SPU_REV_DEPTHL    = 0x02;
static constexpr uint32_t SPU_REV_DEPTHR    = 0x04;
static constexpr uint32_t SPU_REV_DELAYTIME = 0x08;
static constexpr uint32_t SPU_REV_FEEDBACK  = 0x10;

void LIBSPU_SpuSetVoiceAttr() noexcept;
void LIBSPU__SpuSetVoiceAttr() noexcept;
void LIBSPU__spu_note2pitch() noexcept;
void LIBSPU_SpuSetReverbModeParam() noexcept;
void LIBSPU__SpuIsInAllocateArea_() noexcept;
void LIBSPU__spu_init() noexcept;
void LIBSPU__spu_writeByIO() noexcept;
void LIBSPU__spu_r() noexcept;
void LIBSPU__spu_ioctl() noexcept;
void LIBSPU__spu_setVoiceAttr() noexcept;
void LIBSPU__spu_setReverbAttr() noexcept;
void LIBSPU__spu_setCommonAttr() noexcept;
void LIBSPU__spu_getCommonAttr() noexcept;
void LIBSPU__SpuCallback() noexcept;
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
void LIBSPU_SpuSetKey() noexcept;
void LIBSPU_SpuGetAllKeysStatus() noexcept;
