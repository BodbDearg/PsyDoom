#pragma once

#include <cstdint>

enum cheatseq_t : int32_t;

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

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the cheat sequence to execute on the pause menu using helper shortcut keys in development mode.
// These save the pain of inputting button sequences and make it easy to cheat.
//------------------------------------------------------------------------------------------------------------------------------------------
cheatseq_t getDevCheatSequenceToExec() noexcept;
