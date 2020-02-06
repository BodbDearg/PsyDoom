#pragma once

#include <cstdint>

void I_DrawNumber(const int32_t x, const int32_t y, const int32_t value) noexcept;
void _thunk_I_DrawNumber() noexcept;

void I_DrawStringSmall() noexcept;
void I_DrawPausedOverlay() noexcept;
void I_UpdatePalette() noexcept;

void I_DrawString(const int32_t x, const int32_t y, const char* const str) noexcept;
void _thunk_I_DrawString() noexcept;
