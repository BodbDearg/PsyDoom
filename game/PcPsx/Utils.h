#pragma once

#include "Macros.h"
#include <cstdint>

enum cheatseq_t : int32_t;

BEGIN_NAMESPACE(Utils)

void do_platform_updates() noexcept;
void wait_for_seconds(float seconds) noexcept;
void thread_yield() noexcept;
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
