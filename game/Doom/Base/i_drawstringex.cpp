//------------------------------------------------------------------------------------------------------------------------------------------
// New for PsyDoom: a module containing an extended, more flexible string drawing function.
// This function gives more control over the appearance and positioning of the text.
// It also allows line-breaks via the special '\n' character pair.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "i_drawstringex.h"

#if PSYDOOM_MODS

#include "Doom/Renderer/r_data.h"
#include "i_drawcmds.h"
#include "i_main.h"
#include "i_misc.h"
#include "PsyDoom/Game.h"
#include "PsyQ/LIBGPU.h"

#include <algorithm>

DrawStringLine      gDrawStringEx_lines[DRAW_STRING_EX_MAX_LINES];      // The lines for the text to be drawn by 'I_DrawStringEx'
DrawStringMetrics   gDrawStringEx_metrics;                              // The metrics for the text to be drawn by 'I_DrawStringEx'

//------------------------------------------------------------------------------------------------------------------------------------------
// Finds the end of the first line in the given string.
// The end of lines are delimited by the '\n' character pair.
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* FindLineEnd(const char* const str) noexcept {
    const char* pCurChar = str;
    
    while (*pCurChar) {
        if ((pCurChar[0] == '\\') && (pCurChar[1] == 'n'))
            return pCurChar + 2;

        pCurChar++;
    }

    return pCurChar;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines the number of actual characters in the specified text line, excluding any '\n' pair (newline instruction) at the end
//------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t GetRealLineLength(const char* const pCharsBeg, const char* const pCharsEnd) noexcept {
    const uint32_t numRawChars = (uint32_t)(pCharsEnd - pCharsBeg);
    const bool bEndsWithNewline = ((numRawChars >= 2) && (pCharsEnd[-2] == '\\') && (pCharsEnd[-1] == 'n'));
    return (bEndsWithNewline) ? numRawChars - 2 : numRawChars;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the index of which glyph in the big font the specified character should use.
// Returns '-1' if the character does not exist in the big font.
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t GetBigFontCharIndex(const char c) noexcept {
    if ((c >= 'A') && (c <= 'Z'))
        return (int32_t)(BIG_FONT_UCASE_ALPHA + (c - 'A'));
    
    if ((c >= 'a') && (c <= 'z'))
        return (int32_t)(BIG_FONT_LCASE_ALPHA + (c - 'a'));
    
    if ((c >= '0') && (c <= '9'))
        return (int32_t)(BIG_FONT_DIGITS + (c - '0'));
    
    if (c == '%')
        return BIG_FONT_PERCENT;
    
    if (c == '!')
        return BIG_FONT_EXCLAMATION;
    
    if (c == '.')
        return BIG_FONT_PERIOD;
    
    if (c == '-')
        return BIG_FONT_MINUS;
    
    // This character does not exist in the big font!
    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines the width of the specified line of text
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t DetermineLineWidth(const char* const str, const uint32_t numChars, const DrawStringParams& params) noexcept {
    // Easy case: small font has a fixed character width
    if (params.bUseSmallFont)
        return numChars * SMALL_FONT_SIZE;

    // Harder case: the big font has variable character widths
    int32_t width = 0;

    for (uint32_t i = 0; i < numChars; ++i) {
        const int32_t fontCharIdx = GetBigFontCharIndex(str[i]);
        width += (fontCharIdx >= 0) ? gBigFontChars[fontCharIdx].w : BIG_FONT_WHITESPACE_CHAR_WIDTH;
    }

    return width;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws a single line of text using the big font and the specified sprite primitive.
// The current draw mode and most shared draw state of the sprite primitive is expected to have been setup prior to calling.
//------------------------------------------------------------------------------------------------------------------------------------------
static void DrawStringLine_BigFont(const DrawStringLine& line, const DrawStringParams& params, SPRT& spritePrim) noexcept {
    int32_t curX = line.posX;

    for (uint32_t i = 0; i < line.length; ++i) {
        const char theChar = line.text[i];

        // Treat line break commands ("\n") as spaces if the draw mode specifies we should
        if (params.bRemoveNewlines && (theChar == '\\') && (line.text[i + 1] == 'n')) {
            curX += BIG_FONT_WHITESPACE_CHAR_WIDTH;
            ++i; // Consuming 2 characters rather than just 1
            continue;
        }

        // Get the glyph to use and skip past it if it's a space or unsupported character
        const int32_t charIdx = GetBigFontCharIndex(theChar);

        if (charIdx < 0) {
            curX += BIG_FONT_WHITESPACE_CHAR_WIDTH;
            continue;
        }

        // Lowercase characters get nudged down slightly
        const int32_t yAdjust = (charIdx >= BIG_FONT_LCASE_ALPHA) ? 3 : 0;

        // Populate and submit the draw primitive
        const fontchar_t& fontchar = gBigFontChars[charIdx];

        LIBGPU_setXY0(spritePrim, (int16_t) curX, (int16_t)(line.posY + yAdjust));
        LIBGPU_setUV0(spritePrim, fontchar.u, fontchar.v);
        LIBGPU_setWH(spritePrim, fontchar.w, fontchar.h);
        I_AddPrim(spritePrim);

        // Move past the drawn character
        curX += fontchar.w;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws a single line of text using the small font and the specified sprite primitive.
// The current draw mode and most shared draw state of the sprite primitive is expected to have been setup prior to calling.
//------------------------------------------------------------------------------------------------------------------------------------------
void DrawStringLine_SmallFont(const DrawStringLine& line, const DrawStringParams& params, SPRT& spritePrim) noexcept {
    spritePrim.y0 = (int16_t) line.posY;
    int32_t curX = line.posX;

    for (uint32_t i = 0; i < line.length; ++i) {
        char theChar = line.text[i];

        // Treat line break commands ("\n") as spaces if the draw mode specifies we should
        if (params.bRemoveNewlines && (theChar == '\\') && (line.text[i + 1] == 'n')) {
            curX += SMALL_FONT_SIZE;
            ++i; // Consuming 2 characters rather than just 1
            continue;
        }

        // Force the character to draw to be uppercase: the small font only has uppercase defined!
        if ((theChar >= 'a') && (theChar <= 'z')) {
            theChar += 'A' - 'a';
        }

        // Figure out the font character index.
        // Note that the first 33 characters (control characters and space) are just printed as 8px whitespace.
        const int32_t charIdx = (int32_t) theChar - 33;

        // Only 64 ascii characters (after whitespace) are supported by this font
        if (charIdx >= 0 && charIdx < 64) {
            // Figure out the u and v texture coordinates to use based on the font character: each character is 8x8 px
            const int32_t fontRow = charIdx / 32;
            const int32_t fontCol = charIdx - fontRow * 32;
            const int32_t texU = fontCol * SMALL_FONT_SIZE;
            const int32_t texV = fontRow * SMALL_FONT_SIZE + SMALL_FONT_V_MIN;

            // Populate and submit the draw primitive
            spritePrim.x0 = (int16_t) curX;
            LIBGPU_setUV0(spritePrim, (uint8_t) texU, (uint8_t) texV);
            I_AddPrim(spritePrim);
        }

        curX += SMALL_FONT_SIZE;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does layout for a string to be drawn and then draws it using the specified parameters
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawStringEx(
    const int32_t x,
    const int32_t y,
    const DrawStringParams& params,
    const char* const str
) noexcept {
    I_DrawStringEx_LayoutText(x, y, params, str);
    I_DrawStringEx_PostLayoutDraw(params);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs the layout required for 'I_DrawStringEx'.
// Can also be used to determine the metrics for text prior to render.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawStringEx_LayoutText(
    const int32_t x,
    const int32_t y,
    const DrawStringParams& params,
    const char* const str
) noexcept {
    // No lines to draw initially
    DrawStringMetrics& metrics = gDrawStringEx_metrics;
    metrics = {};
    
    // Split up the text into lines (unless converting newlines to spaces) and do horizontal layout for each line
    const char* pLineBeg = str;
    const char* pLineEnd = (params.bRemoveNewlines) ? pLineBeg + std::strlen(pLineBeg) : FindLineEnd(pLineBeg);
    
    while ((pLineBeg != pLineEnd) && (metrics.numLines < DRAW_STRING_EX_MAX_LINES)) {
        DrawStringLine& line = gDrawStringEx_lines[metrics.numLines];
        metrics.numLines++;

        // Determine line length and width
        line.text = pLineBeg;
        line.length = GetRealLineLength(pLineBeg, pLineEnd);
        line.sizeX = DetermineLineWidth(pLineBeg, line.length, params);

        // Do horizontal layout
        if (params.xAnchorMode == DrawStringAnchorMode::CENTER) {
            line.posX = x - line.sizeX / 2;
        } else if (params.xAnchorMode == DrawStringAnchorMode::MAX) {
            line.posX = x - line.sizeX;
        } else {
            line.posX = x;
        }

        // Move onto the next line
        pLineBeg = pLineEnd;
        pLineEnd = FindLineEnd(pLineBeg);
    }

    // No lines of text?
    const int32_t numLines = metrics.numLines;

    if (numLines <= 0)
        return;

    // How high are all the lines?
    const int32_t lineHeight = (params.bUseSmallFont) ? SMALL_FONT_SIZE : BIG_FONT_LINE_HEIGHT;
    const int32_t textHeight = numLines * lineHeight;

    // Do vertical layout
    int32_t curY;

    if (params.yAnchorMode == DrawStringAnchorMode::CENTER) {
        curY = y - textHeight / 2;
    } else if (params.yAnchorMode == DrawStringAnchorMode::MAX) {
        curY = y - textHeight;
    } else {
        curY = y;
    }

    for (int32_t i = 0; i < numLines; ++i) {
        gDrawStringEx_lines[i].posY = curY;
        curY += lineHeight;
    }

    // Determine text metrics
    DrawStringLine& firstLine = gDrawStringEx_lines[0];
    metrics.begX = firstLine.posX;
    metrics.endX = firstLine.posX + firstLine.sizeX;
    metrics.begY = firstLine.posY;
    metrics.endY = firstLine.posY + textHeight;

    for (int32_t i = 1; i < numLines; ++i) {
        DrawStringLine& line = gDrawStringEx_lines[i];
        metrics.begX = std::min(metrics.begX, line.posX);
        metrics.endX = std::max(metrics.endX, line.posX + line.sizeX);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs the actual drawing after layout has been done.
// Assumes all the strings references generated during layout are still valid.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawStringEx_PostLayoutDraw(const DrawStringParams& params) noexcept {
    // Set the draw mode to remove the current texture window (set it to the max size).
    // Also set the current texture page to that of the STATUS graphic.
    {
        DR_MODE drawModePrim = {};
        const SRECT texWindow = { (int16_t) gTex_STATUS.texPageCoordX, (int16_t) gTex_STATUS.texPageCoordY, 256, 256 };
        
        LIBGPU_SetDrawMode(
            drawModePrim,
            false,
            false,
            gTex_STATUS.texPageId | LIBGPU_GetTPageSemiTransBits(params.semiTransMode),
            &texWindow
        );

        I_AddPrim(drawModePrim);
    }

    // Some basic setup of the sprite primitive for all characters
    SPRT spritePrim = {};
    LIBGPU_SetSprt(spritePrim);
    LIBGPU_SetShadeTex(spritePrim, (!params.bEnableShading));
    LIBGPU_setRGB0(spritePrim, params.colR, params.colG, params.colB);
    LIBGPU_SetSemiTrans(spritePrim, params.bSemiTransparent);
    spritePrim.clut = (params.clutId == 0) ? Game::getTexPalette_STATUS() : params.clutId;

    if (params.bUseSmallFont) {
        LIBGPU_setWH(spritePrim, 8, 8);
    }

    // Line drawing loop
    const DrawStringMetrics& metrics = gDrawStringEx_metrics;

    if (params.bUseSmallFont) {
        for (int32_t i = 0; i < metrics.numLines; ++i) {
            DrawStringLine_SmallFont(gDrawStringEx_lines[i], params, spritePrim);
        }
    }
    else {
        for (int32_t i = 0; i < metrics.numLines; ++i) {
            DrawStringLine_BigFont(gDrawStringEx_lines[i], params, spritePrim);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Counts the number of lines ("\n") in the specified string to be drawn.
// Useful for layout purposes.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t I_DrawStringEx_CountNumLines(const char* const str) noexcept {
    const char* pCurChar = str;
    int32_t numLines = 0;

    while (*pCurChar) {
        numLines++;
        pCurChar = FindLineEnd(pCurChar);
    }

    return numLines;
}

#endif  // #if PSYDOOM_MODS
