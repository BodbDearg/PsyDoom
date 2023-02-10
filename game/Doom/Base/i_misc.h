#pragma once

#include <cstdint>

// Definition for a big font character: where it is on the 'STATUS' texture atlas and it's size
struct fontchar_t {
    std::uint8_t u;
    std::uint8_t v;
    std::uint8_t w;
    std::uint8_t h;
};

// The list of big font characters
static constexpr int32_t NUM_BIG_FONT_CHARS = 66;
extern const fontchar_t gBigFontChars[NUM_BIG_FONT_CHARS];

// Starting indices for various individual and groups of big font chars
enum : int32_t {
    BIG_FONT_DIGITS         = 0,
    BIG_FONT_MINUS          = 10,
    BIG_FONT_PERCENT        = 11,
    BIG_FONT_EXCLAMATION    = 12,
    BIG_FONT_PERIOD         = 13,
    BIG_FONT_UCASE_ALPHA    = 14,
    BIG_FONT_LCASE_ALPHA    = 40
};

// The size in pixels (width and height) of the small font and top V coordinate for all it's characters
static constexpr int32_t SMALL_FONT_SIZE = 8;
static constexpr int32_t SMALL_FONT_V_MIN = 168;

// How high each line is for the big font and the width of a single space
static constexpr int32_t BIG_FONT_LINE_HEIGHT = 16;
static constexpr int32_t BIG_FONT_WHITESPACE_CHAR_WIDTH = 6;

void I_DrawNumber(const int32_t x, const int32_t y, const int32_t value) noexcept;

#if PSYDOOM_MODS
    void I_DrawStringSmall(
        const int32_t x,
        const int32_t y,
        const char* const str,
        const uint16_t clutId,
        const uint8_t r,
        const uint8_t g,
        const uint8_t b,
        const bool bSemiTransparent,
        const bool bDisableShading
    ) noexcept;
#else
    void I_DrawStringSmall(const int32_t x, const int32_t y, const char* const str) noexcept;
#endif

void I_DrawPausedOverlay() noexcept;
void I_UpdatePalette() noexcept;

#if PSYDOOM_MODS
    int32_t I_GetStringWidth(const char* const str) noexcept;
#endif

void I_DrawString(const int32_t x, const int32_t y, const char* const str) noexcept;
