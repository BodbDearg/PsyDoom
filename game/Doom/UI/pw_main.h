#pragma once

#include "PsxVm/VmPtr.h"

// How many password characters are available for the password system
static constexpr int32_t NUM_PW_CHARS = 32;

// The length of a password sequence
static constexpr int32_t PW_SEQ_LEN = 10;

extern const char                           gPasswordChars[NUM_PW_CHARS + 1];
extern const VmPtr<int32_t>                 gNumPasswordCharsEntered;
extern const VmPtr<uint8_t[PW_SEQ_LEN]>     gPasswordCharBuffer;

void START_PasswordScreen() noexcept;
void STOP_PasswordScreen() noexcept;
void TIC_PasswordScreen() noexcept;
void DRAW_PasswordScreen() noexcept;
