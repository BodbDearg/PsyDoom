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

// Return codes for some functions
static constexpr int32_t SPU_SUCCESS    = 0;
static constexpr int32_t SPU_ERROR      = -1;

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
void LIBSPU_SpuSetReverbDepth() noexcept;
void LIBSPU_SpuSetReverbVoice() noexcept;
void LIBSPU_SpuInit() noexcept;
void LIBSPU_SpuSetReverb() noexcept;
void LIBSPU_SpuQuit() noexcept;

bool LIBSPU_SpuIsTransferCompleted(const SpuTransferQuery mode) noexcept;
void _thunk_LIBSPU_SpuIsTransferCompleted() noexcept;

void LIBSPU_SpuInitMalloc() noexcept;
void LIBSPU_SpuSetTransferMode(const SpuTransferMode mode) noexcept;

uint32_t LIBSPU_SpuSetTransferStartAddr(const uint32_t addr) noexcept;
void _thunk_LIBSPU_SpuSetTransferStartAddr() noexcept;

uint32_t LIBSPU_SpuWrite(const void* const pData, const uint32_t size) noexcept;
void _thunk_LIBSPU_SpuWrite() noexcept;

void LIBSPU_SpuSetKeyOnWithAttr() noexcept;
void LIBSPU_SpuSetKey() noexcept;
void LIBSPU_SpuGetAllKeysStatus() noexcept;
