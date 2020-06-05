#pragma once

#include "Doom/doomdef.h"

// How many password characters are available for the password system
static constexpr int32_t NUM_PW_CHARS = 32;

// The length of a password sequence
static constexpr int32_t PW_SEQ_LEN = 10;

extern const char   gPasswordChars[NUM_PW_CHARS + 1];
extern int32_t      gNumPasswordCharsEntered;
extern uint8_t      gPasswordCharBuffer[PW_SEQ_LEN];
extern bool         gbUsingAPassword;

void START_PasswordScreen() noexcept;
void STOP_PasswordScreen(const gameaction_t exitAction) noexcept;
gameaction_t TIC_PasswordScreen() noexcept;
void DRAW_PasswordScreen() noexcept;
