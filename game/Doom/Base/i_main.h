#pragma once

#include "PsxVm/VmPtr.h"

// Size of the temporary buffer that is used for WAD loading and other stuff - 64 KiB
static constexpr uint32_t TMP_BUFFER_SIZE = 0x10000;

extern const VmPtr<std::byte[TMP_BUFFER_SIZE]>  gTmpBuffer;
extern const VmPtr<uint32_t>                    gTotalVBlanks;
extern const VmPtr<uint32_t>                    gLastTotalVBlanks;
extern const VmPtr<uint32_t>                    gElapsedVBlanks;
extern const VmPtr<uint32_t>                    gLockedTexPagesMask;

void I_Main() noexcept;
void I_PSXInit() noexcept;
[[noreturn]] void I_Error(const char* const fmtMsg, ...) noexcept;
void I_ReadGamepad() noexcept;
void I_CacheTexForLumpName() noexcept;
void I_CacheAndDrawSprite() noexcept;
void I_DrawSprite() noexcept;
void I_DrawPlaque() noexcept;
void I_IncDrawnFrameCount() noexcept;
void I_DrawPresent() noexcept;
void I_VsyncCallback() noexcept;
void I_Init() noexcept;
void I_CacheTex() noexcept;
void I_RemoveTexCacheEntry() noexcept;
void I_ResetTexCache() noexcept;
void I_VramViewerDraw() noexcept;
void I_NetSetup() noexcept;
void I_NetUpdate() noexcept;
void I_NetHandshake() noexcept;
void I_NetSendRecv() noexcept;
void I_SubmitGpuCmds() noexcept;
void I_LocalButtonsToNet() noexcept;
void I_NetButtonsToLocal() noexcept;
