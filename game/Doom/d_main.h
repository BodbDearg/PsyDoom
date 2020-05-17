#pragma once

#include "doomdef.h"
#include "PcPsx/Types.h"
#include "PsxVm/VmPtr.h"

enum class CdMapTbl_File : int32_t;

extern const VmPtr<int32_t>                 gTicCon;
extern const VmPtr<int32_t[MAXPLAYERS]>     gPlayersElapsedVBlanks;

extern uint32_t*    gpDemoBuffer;
extern uint32_t*    gpDemo_p;

#if PC_PSX_DOOM_MODS
    extern uint32_t*    gpDemoBufferEnd;
#endif

extern const VmPtr<skill_t>                 gStartSkill;
extern const VmPtr<int32_t>                 gStartMapOrEpisode;
extern const VmPtr<gametype_t>              gStartGameType;
extern const VmPtr<bool32_t>                gbDidAbortGame;

void D_DoomMain() noexcept;
gameaction_t RunLegals() noexcept;
gameaction_t RunTitle() noexcept;
gameaction_t RunDemo(const CdMapTbl_File file) noexcept;

#if PC_PSX_DOOM_MODS
    gameaction_t RunDemoAtPath(const char* const filePath) noexcept;
#endif

gameaction_t RunCredits() noexcept;
void I_SetDebugDrawStringPos(const int32_t x, const int32_t y) noexcept;
void I_DebugDrawString(const char* const fmtMsg, ...) noexcept;

void D_memset(void* const pDst, const std::byte fillByte, const uint32_t count) noexcept;
void _thunk_D_memset() noexcept;

void D_memcpy(void* const pDst, const void* const pSrc, const uint32_t numBytes) noexcept;
void _thunk_D_memcpy() noexcept;

void D_strncpy(char* dst, const char* src, uint32_t maxChars) noexcept;

int32_t D_strncasecmp(const char* str1, const char* str2, int32_t maxCount) noexcept;
void _thunk_D_strncasecmp() noexcept;

void D_strupr(char* str) noexcept;

gameaction_t MiniLoop(
    void (*const pStart)(),
    void (*const pStop)(const gameaction_t exitAction),
    gameaction_t (*const pTicker)(),
    void (*const pDrawer)()
) noexcept;
