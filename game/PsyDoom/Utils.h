#pragma once

#include "Macros.h"

#include <cstdint>
#include <memory>
#include <string>

enum SequenceStatus : uint8_t;
struct DiscInfo;
struct IsoFileSys;

BEGIN_NAMESPACE(Utils)

//------------------------------------------------------------------------------------------------------------------------------------------
// A simple container for data read from a file on the game disc.
// Holds the byte array and the number of bytes for the data.
//------------------------------------------------------------------------------------------------------------------------------------------
struct DiscFileData {
    std::unique_ptr<std::byte[]>    pBytes;
    uint32_t                        numBytes;
};

void installFatalErrorHandler() noexcept;
void uninstallFatalErrorHandler() noexcept;
std::string getOrCreateUserDataFolder() noexcept;
void doPlatformUpdates() noexcept;
bool waitForSeconds(const float seconds) noexcept;
bool waitForCdAudioPlaybackStart() noexcept;
bool waitUntilSeqEnteredStatus(const int32_t sequenceIdx, const SequenceStatus status) noexcept;
bool waitUntilSeqExitedStatus(const int32_t sequenceIdx, const SequenceStatus status) noexcept;
bool waitForCdAudioFadeOut() noexcept;
void threadYield() noexcept;
void onBeginUIDrawing() noexcept;
void checkForRendererToggleInput() noexcept;

DiscFileData getDiscFileData(
    const DiscInfo& discInfo,
    const IsoFileSys& isoFileSys,
    const char* const filePath,
    const uint32_t readOffset = 0,
    const int32_t numBytesToRead = -1
) noexcept;

bool getDiscFileMD5Hash(
    const DiscInfo& discInfo,
    const IsoFileSys& isoFileSys,
    const char* const filePath,
    uint64_t& hashWord1,
    uint64_t& hashWord2
) noexcept;

bool checkDiscFileMD5Hash(
    const DiscInfo& discInfo,
    const IsoFileSys& isoFileSys,
    const char* const filePath,
    const uint64_t checkHashWord1,
    const uint64_t checkHashWord2
) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a pad button has just been pressed by examining the currently pressed pad buttons versus the last pressed
//------------------------------------------------------------------------------------------------------------------------------------------
inline constexpr bool padBtnJustPressed(const uint32_t btn, const uint32_t curPadBtns, const uint32_t oldPadBtns) noexcept {
    if (curPadBtns & btn) {
        return ((oldPadBtns & btn) == 0);
    } else {
        return false;
    }
}

END_NAMESPACE(Utils)
