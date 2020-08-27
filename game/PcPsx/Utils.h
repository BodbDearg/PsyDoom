#pragma once

#include "Macros.h"

#include <cstdint>
#include <string>

enum cheatseq_t : int32_t;
enum SequenceStatus : uint8_t;

BEGIN_NAMESPACE(Utils)

std::string getOrCreateUserDataFolder() noexcept;
void doPlatformUpdates() noexcept;
void waitForSeconds(float seconds) noexcept;
void waitForCdAudioPlaybackStart() noexcept;
void waitUntilSeqEnteredStatus(const int32_t sequenceIdx, const SequenceStatus status) noexcept;
void waitUntilSeqExitedStatus(const int32_t sequenceIdx, const SequenceStatus status) noexcept;
void waitForCdAudioFadeOut() noexcept;
void threadYield() noexcept;
cheatseq_t getDevCheatSequenceToExec() noexcept;

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
