#pragma once

#include "doomdef.h"

#include <cstddef>
#include <cstring>

// PsyDoom: 'CdFileId' has changed in format
#if PSYDOOM_MODS
    struct String16;
    typedef String16 CdFileId;
#else 
    typedef int32_t CdFileId;
#endif

extern int32_t      gTicCon;
extern int32_t      gPlayersElapsedVBlanks[MAXPLAYERS];

#if PSYDOOM_MODS
    extern int32_t  gNextPlayerElapsedVBlanks;
#endif

extern std::byte*   gpDemoBuffer;
extern std::byte*   gpDemo_p;
extern skill_t      gStartSkill;
extern int32_t      gStartMapOrEpisode;
extern gametype_t   gStartGameType;
extern bool         gbDidAbortGame;

#if PSYDOOM_MODS
    extern bool         gbIsFirstTick;
    extern bool         gbKeepInputEvents;
    extern std::byte*   gpDemoBufferEnd;
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

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Helper function to check if the specified number of values can be read from the demo buffer
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline bool Demo_CanRead(const uint32_t numElems = 1) noexcept {
    const size_t numBytes = sizeof(T) * numElems;
    return (gpDemo_p + numBytes <= gpDemoBufferEnd);
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper function for reading a single value from the demo buffer.
// This encapsulates some of the original logic in a way that is also more flexible for the new demo format.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline T Demo_Read() noexcept {
    constexpr size_t NUM_BYTES = sizeof(T);

    // For PsyDoom I'm adding additional safety and zeroing out of bounds reads
    #if PSYDOOM_MODS
        if (gpDemo_p + NUM_BYTES > gpDemoBufferEnd)
            return {};
    #endif

    T value;
    std::memcpy(&value, gpDemo_p, NUM_BYTES);
    gpDemo_p += NUM_BYTES;
    return value;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper function for reading multiple values from the demo buffer.
// This encapsulates some of the original logic in a way that is also more flexible for the new demo format.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline void Demo_Read(T* const pElems, const uint32_t numElems) noexcept {
    const size_t numBytes = sizeof(T) * numElems;

    // For PsyDoom I'm adding additional safety and zeroing out of bounds reads
    #if PSYDOOM_MODS
        if (gpDemo_p + numBytes > gpDemoBufferEnd) {
            std::memset(pElems, 0, numBytes);
            return;
        }
    #endif

    std::memcpy(pElems, gpDemo_p, numBytes);
    gpDemo_p += numBytes;
}
