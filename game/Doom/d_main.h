#pragma once

#include "doomdef.h"

#include <cstddef>

// PsyDoom: 'CdFileId' has changed in format
#if PSYDOOM_MODS
    union String16;
    typedef String16 CdFileId;
#else 
    typedef int32_t CdFileId;
#endif

extern int32_t      gTicCon;
extern int32_t      gPlayersElapsedVBlanks[MAXPLAYERS];

#if PSYDOOM_MODS
    extern int32_t  gNextPlayerElapsedVBlanks;
#endif

extern uint32_t*    gpDemoBuffer;
extern uint32_t*    gpDemo_p;
extern skill_t      gStartSkill;
extern int32_t      gStartMapOrEpisode;
extern gametype_t   gStartGameType;
extern bool         gbDidAbortGame;

#if PSYDOOM_MODS
    extern bool         gbIsFirstTick;
    extern bool         gbKeepInputEvents;
    extern uint32_t*    gpDemoBufferEnd;
    extern bool         gbDoInPlaceLevelReload;
    extern fixed_t      gInPlaceReloadPlayerX;
    extern fixed_t      gInPlaceReloadPlayerY;
    extern fixed_t      gInPlaceReloadPlayerZ;
    extern angle_t      gInPlaceReloadPlayerAng;
#endif

void D_DoomMain() noexcept;
gameaction_t RunLegals() noexcept;
gameaction_t RunTitle() noexcept;
gameaction_t RunDemo(const CdFileId file) noexcept;

#if PSYDOOM_MODS
    gameaction_t RunDemoAtPath(const char* const filePath) noexcept;
#endif

gameaction_t RunCredits() noexcept;

void I_SetDebugDrawStringPos(const int32_t x, const int32_t y) noexcept;
void I_DebugDrawString(const char* const fmtMsg, ...) noexcept;

void D_memset(void* const pDst, const std::byte fillByte, const uint32_t count) noexcept;
void D_memcpy(void* const pDst, const void* const pSrc, const uint32_t numBytes) noexcept;
void D_strncpy(char* dst, const char* src, uint32_t maxChars) noexcept;
int32_t D_strncasecmp(const char* str1, const char* str2, int32_t maxCount) noexcept;
void D_strupr(char* str) noexcept;

gameaction_t MiniLoop(
    void (*const pStart)(),
    void (*const pStop)(const gameaction_t exitAction),
    gameaction_t (*const pTicker)(),
    void (*const pDrawer)()
) noexcept;
