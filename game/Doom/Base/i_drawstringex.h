#pragma once

#include <cstdint>

#if PSYDOOM_MODS
    // Text positioning/anchor mode for the x or y axis when using PsyDoom's new 'I_DrawStringEx()' function.
    enum class DrawStringAnchorMode : uint8_t {
        MIN,        // The x or y coordinate describes where the minimum extent of the text should be placed on that axis
        CENTER,     // The x or y coordinate describes where the center of the text should be placed on that axis
        MAX         // The x or y coordinate describes where the maximum extent of the text should be placed on that axis
    };

    // Style settings for PsyDoom's new 'I_DrawStringEx()' function.
    // Note: by default initializing this struct you also set it to the default settings.
    struct DrawStringParams {
        uint16_t                clutId;             // Which palette clut to use: if '0' automatically choose
        bool                    bEnableShading;     // Allow the text to be colored? (affects usage of the rgb components)
        bool                    bSemiTransparent;   // If 'true' then semi-transparency is enabled for the text
        uint8_t                 semiTransMode;      // 0-3: which semi-transparency blending mode to use: see 'LIBGPU_getTPage' for more details.
        uint8_t                 colR;               // If shading is enabled, which color (red) to use
        uint8_t                 colG;               // If shading is enabled, which color (green) to use
        uint8_t                 colB;               // If shading is enabled, which color (blue) to use
        DrawStringAnchorMode    xAnchorMode;        // Horizontal text alignment mode
        DrawStringAnchorMode    yAnchorMode;        // Vertical text alignment mode
        bool                    bUseSmallFont;      // If 'true' use the small font instead of the big one
        bool                    bRemoveNewlines;    // If 'true' then convert newlines to spaces: useful to force everything onto a single line
    };

    // Stores a single line of text to be rendered by 'I_DrawStringEx'
    struct DrawStringLine {
        const char*     text;       // The text itself
        uint32_t        length;     // Number of characters in the text
        int32_t         posX;       // What position to draw the text at (left x)
        int32_t         posY;       // What position to draw the text at (top y)
        int32_t         sizeX;      // The size of the text line: x-axis
    };

    // Stores the results of laying out text for 'I_DrawStringEx'
    struct DrawStringMetrics {
        int32_t     numLines;
        int32_t     begX, endX;
        int32_t     begY, endY;
    };

    // The maximum number of lines that can be drawn by 'I_DrawStringEx'
    static constexpr int32_t DRAW_STRING_EX_MAX_LINES = 256;

    extern DrawStringLine       gDrawStringEx_lines[DRAW_STRING_EX_MAX_LINES];
    extern DrawStringMetrics    gDrawStringEx_metrics;

    void I_DrawStringEx(
        const int32_t x,
        const int32_t y,
        const DrawStringParams& params,
        const char* const str
    ) noexcept;

    void I_DrawStringEx_LayoutText(
        const int32_t x,
        const int32_t y,
        const DrawStringParams& params,
        const char* const str
    ) noexcept;

    void I_DrawStringEx_PostLayoutDraw(const DrawStringParams& params) noexcept;
    int32_t I_DrawStringEx_CountNumLines(const char* const str) noexcept;

#endif  // #if PSYDOOM_MODS
