#pragma once

#include "doomdef.h"
#include "PcPsx/Types.h"
#include "PsxVm/VmPtr.h"

extern VmPtr<int32_t> gTicCon;

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

gameaction_t MiniLoop(
    void (*const pStart)(),
    void (*const pStop)(),
    void (*const pTicker)(),
    void (*const pDrawer)()
) noexcept;
