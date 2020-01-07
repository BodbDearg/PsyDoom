#pragma once

#include "doomdef.h"
#include "PcPsx/Types.h"
#include "PsxVm/VmPtr.h"

enum class CdMapTbl_File : uint32_t;

extern const VmPtr<int32_t>                 gTicCon;
extern const VmPtr<int32_t[MAXPLAYERS]>     gPlayersElapsedVBlanks;
extern const VmPtr<VmPtr<uint32_t>>         gpDemoBuffer;
extern const VmPtr<VmPtr<uint32_t>>         gpDemo_p;
extern const VmPtr<skill_t>                 gStartSkill;
extern const VmPtr<int32_t>                 gStartMapOrEpisode;
extern const VmPtr<gametype_t>              gStartGameType;

void D_DoomMain() noexcept;
gameaction_t RunLegals() noexcept;
gameaction_t RunTitle() noexcept;
gameaction_t RunDemo(const CdMapTbl_File file) noexcept;
gameaction_t RunCredits() noexcept;
void I_SetDebugDrawStringPos() noexcept;
void I_DebugDrawString(const char* const fmtMsg, ...) noexcept;

void D_memset(void* const pDst, const std::byte fillByte, const uint32_t count) noexcept;
void _thunk_D_memset() noexcept;

void D_memcpy(void* const pDst, const void* const pSrc, const uint32_t numBytes) noexcept;
void _thunk_D_memcpy() noexcept;

void D_strncpy(char* dst, const char* src, uint32_t maxChars) noexcept;

int32_t D_strncasecmp(const char* str1, const char* str2, int32_t maxCount) noexcept;
void _thunk_D_strncasecmp() noexcept;

void strupr(char* str) noexcept;

gameaction_t MiniLoop(
    void (*const pStart)(),
    void (*const pStop)(),
    void (*const pTicker)(),
    void (*const pDrawer)()
) noexcept;
