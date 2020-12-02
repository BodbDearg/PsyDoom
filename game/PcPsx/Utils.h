#pragma once

#include "Macros.h"

#include <cstdint>
#include <string>

enum SequenceStatus : uint8_t;

BEGIN_NAMESPACE(Utils)

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
