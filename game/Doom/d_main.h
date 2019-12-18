#pragma once

#include "PsxVm/VmPtr.h"
#include "PcPsx/Types.h"

extern VmPtr<int32_t>   gGameTic;
extern VmPtr<int32_t>   gPrevGameTic;
extern VmPtr<int32_t>   gLastTgtGameTicCount;
extern VmPtr<int32_t>   gTicCon;
extern VmPtr<bool32_t>  gbDemoPlayback;
extern VmPtr<bool32_t>  gbDemoRecording;

void D_DoomMain() noexcept;
void RunLegals() noexcept;
void RunTitle() noexcept;
void RunDemo() noexcept;
void RunCredits() noexcept;
void I_SetDebugDrawStringPos() noexcept;
void I_DebugDrawString() noexcept;

void D_memset(void* const pDst, const std::byte fillByte, const uint32_t count) noexcept;
void _thunk_D_memset() noexcept;

void D_memcpy(void* const pDst, const void* const pSrc, const uint32_t numBytes) noexcept;
void _thunk_D_memcpy() noexcept;

void D_strncpy() noexcept;
void D_strncasecmp() noexcept;
void strupr() noexcept;
void MiniLoop() noexcept;
