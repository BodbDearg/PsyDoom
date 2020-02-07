#include "i_misc.h"

#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/in_main.h"
#include "Doom/UI/st_main.h"
#include "i_drawcmds.h"
#include "i_main.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBC2.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include <cstdio>

// Where on the UI texture are each of the font characters located and the size of each character
struct fontchar_t {
    std::uint8_t u;
    std::uint8_t v;
    std::uint8_t w;
    std::uint8_t h;
};

static constexpr fontchar_t BIG_FONT_CHARS[] = {
    {   0, 195,  11,  16 }, // 0 - 0
    {  12, 195,  11,  16 }, // 1 - 1
    {  24, 195,  11,  16 }, // 2 - 2
    {  36, 195,  11,  16 }, // 3 - 3
    {  48, 195,  11,  16 }, // 4 - 4
    {  60, 195,  11,  16 }, // 5 - 5
    {  72, 195,  11,  16 }, // 6 - 6
    {  84, 195,  11,  16 }, // 7 - 7
    {  96, 195,  11,  16 }, // 8 - 8
    { 108, 195,  11,  16 }, // 9 - 9
    { 232, 195,  11,  16 }, // - - 10
    { 120, 195,  11,  15 }, // % - 11
    {   0, 211,   7,  16 }, // ! - 12
    {   8, 211,   7,  16 }, // . - 13
    {  16, 211,  15,  16 }, // A - 14
    {  32, 211,  13,  16 }, // B - 15
    {  46, 211,  12,  16 }, // C - 16
    {  60, 211,  13,  16 }, // D - 17
    {  74, 211,  13,  16 }, // E - 18
    {  88, 211,  13,  16 }, // F - 19
    { 102, 211,  13,  16 }, // G - 20
    { 116, 211,  13,  16 }, // H - 21
    { 130, 211,   6,  16 }, // I - 22
    { 136, 211,  12,  16 }, // J - 23
    { 148, 211,  14,  16 }, // K - 24
    { 162, 211,  13,  16 }, // L - 25
    { 176, 211,  15,  16 }, // M - 26
    { 192, 211,  15,  16 }, // N - 27
    { 208, 211,  13,  16 }, // O - 28
    { 222, 211,  13,  16 }, // P - 29
    { 236, 211,  13,  16 }, // Q - 30
    {   0, 227,  13,  16 }, // R - 31
    {  14, 227,  13,  16 }, // S - 32
    {  28, 227,  14,  16 }, // T - 33
    {  42, 227,  13,  16 }, // U - 34
    {  56, 227,  15,  16 }, // V - 35
    {  72, 227,  15,  16 }, // W - 36
    {  88, 227,  15,  16 }, // X - 37
    { 104, 227,  13,  16 }, // Y - 38
    { 118, 227,  13,  16 }, // Z - 39
    { 132, 230,  13,  13 }, // a - 40
    { 146, 230,  12,  13 }, // b - 41
    { 158, 230,  11,  13 }, // c - 42
    { 170, 230,  11,  13 }, // d - 43
    { 182, 230,  10,  13 }, // e - 44
    { 192, 230,  11,  13 }, // f - 45
    { 204, 230,  11,  13 }, // g - 46
    { 216, 230,  12,  13 }, // h - 47
    { 228, 230,   5,  13 }, // i - 48
    { 234, 230,  10,  13 }, // j - 49
    {   0, 243,  12,  13 }, // k - 50
    {  12, 243,   9,  13 }, // l - 51
    {  22, 243,  13,  13 }, // m - 52
    {  36, 243,  13,  13 }, // n - 53
    {  50, 243,  11,  13 }, // o - 54
    {  62, 243,  11,  13 }, // p - 55
    {  74, 243,  11,  13 }, // q - 56
    {  86, 243,  11,  13 }, // r - 57
    {  98, 243,  12,  13 }, // s - 58
    { 112, 243,  11,  13 }, // t - 59
    { 124, 243,  11,  13 }, // u - 60
    { 136, 243,  13,  13 }, // v - 61
    { 150, 243,  13,  13 }, // w - 62
    { 164, 243,  13,  13 }, // x - 63
    { 178, 243,  13,  13 }, // y - 64
    { 192, 243,  13,  13 }  // z - 65
};

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

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw a number using the large font at the specified pixel location
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawNumber(const int32_t x, const int32_t y, const int32_t value) noexcept {
    // Basic setup of the drawing primitive
    SPRT& spritePrim = *(SPRT*) getScratchAddr(128);

    #if PC_PSX_DOOM_MODS
        // Set these primitive properties prior to drawing rather than allowing them to be undefined as in the original code.
        // I think this drawing code just happened to work because of the types of operations which occurred just before it.
        // They must have produced primitive state changes that we actually desired for this function...
        // Relying on external draw code and ordering however is brittle, so be explicit here and set exactly what we need:    
        {
            // Set the draw mode to disable wrapping (zero sized text window) and texture page to the STATUS graphic
            DR_MODE& drawModePrim = *(DR_MODE*) getScratchAddr(128);
            const RECT texWindow = { 0, 0, 0, 0 };
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, &texWindow);
            I_AddPrim(&drawModePrim);
        }

        LIBGPU_SetSprt(spritePrim);
        LIBGPU_SetShadeTex(&spritePrim, true);
        spritePrim.clut = gPaletteClutIds[UIPAL];
    #endif

    spritePrim.y0 = (int16_t) y;            // Always on the same row
    spritePrim.tv0 = 195;                   // Digits are all on the same line in VRAM
    LIBGPU_setWH(spritePrim, 11, 16);       // Digits are always this size

    // Work with unsigned while we are printing, until the end
    bool bNegativeVal;
    int32_t valueAbs;

    if (value >= 0) {
        bNegativeVal = false;
        valueAbs = value;
    } else {
        bNegativeVal = true;
        valueAbs = -value;
    }

    // Figure out what digits to print
    constexpr uint32_t MAX_DIGITS = 16;
    int32_t digits[MAX_DIGITS];
    int32_t digitIdx = 0;

    while (digitIdx < MAX_DIGITS) {
        digits[digitIdx] = valueAbs % 10;
        valueAbs /= 10;

        if (valueAbs <= 0)
            break;

        ++digitIdx;
    }

    // Print the digits, starting with the least significant and move backwards across the screen
    const int32_t numDigits = digitIdx + 1;
    int32_t curX = x;

    for (digitIdx = 0; digitIdx < numDigits; ++digitIdx) {
        const int32_t digit = digits[digitIdx];

        spritePrim.x0 = (int16_t) curX;
        spritePrim.tu0 = BIG_FONT_CHARS[BIG_FONT_DIGITS + digit].u;
        I_AddPrim(&spritePrim);

        curX -= 11;
    }

    // Print the minus symbol if the value was negative
    if (bNegativeVal) {
        spritePrim.x0 = (int16_t) curX;
        spritePrim.tu0 = BIG_FONT_CHARS[BIG_FONT_EXCLAMATION].u;
        I_AddPrim(&spritePrim);
    }
}

void _thunk_I_DrawNumber() noexcept {
    I_DrawNumber(a0, a1, a2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw a string using the very small font used for hud status bar messages
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawStringSmall(const int32_t x, const int32_t y, const char* const str) noexcept {
    // PC-PSX: this draw state was not defined in the original game: explicitly make sure it is setup correctly here.
    // Do not rely on previous drawing code to set it correctly!
    #if PC_PSX_DOOM_MODS
    {
        DR_MODE& drawModePrim = *(DR_MODE*) getScratchAddr(128);
        const RECT texWindow = { 0, 0, 0, 0 };
        LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, &texWindow);
        I_AddPrim(&drawModePrim);
    }
    #endif

    // Common sprite setup for every character
    SPRT& spritePrim = *(SPRT*) getScratchAddr(128);    
    LIBGPU_setWH(spritePrim, 8, 8);
    spritePrim.y0 = (int16_t) y;

    // PC-PSX: this draw state was not defined in the original game: explicitly make sure it is setup correctly here.
    // Do not rely on previous drawing code to set it correctly!
    #if PC_PSX_DOOM_MODS
        LIBGPU_SetSprt(spritePrim);
        LIBGPU_SetShadeTex(&spritePrim, true);
        spritePrim.clut = gPaletteClutIds[UIPAL];
    #endif
    
    // Draw each visible character in the string
    int32_t curX = x;
    const char* pCurChar = str;

    for (char c = *pCurChar; c != 0; ++pCurChar, c = *pCurChar) {
        // Uppercase the character if lowercase: the small font only has uppercase defined
        if (c >= 'a' && c <= 'z') {
            c += 'A' - 'a';
        }

        // Figure out the font character index.
        // Note that the first 33 characters (control characters and space) are just printed as 8px whitespace.
        int32_t charIdx = (int32_t) c - 33;

        // Only 64 ascii characters (after whitespace) are supported by this font
        if (charIdx >= 0 && charIdx < 64) {
            // Figure out the u and v texture coordinates to use based on the font character: each character is 8x8 px
            const int32_t fontRow = charIdx / 32;
            const int32_t fontCol = charIdx - fontRow * 32;
            const int32_t texU = fontCol * 8;
            const int32_t texV = fontRow * 8 - 88;

            // Populate and submit the primitive
            LIBGPU_setUV0(spritePrim, (uint8_t) texU, (uint8_t) texV);
            spritePrim.x0 = (int16_t) curX;

            I_AddPrim(&spritePrim);
        }

        curX += 8;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw pause screen elements, including the plaque and level warp and vram viewer cheats
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawPausedOverlay() noexcept {
    // Draw the paused plaque unless disabled
    const player_t& player = gPlayers[*gCurPlayerIndex];

    if ((player.cheats & CF_NOPAUSEMSG) == 0) {
        I_CacheAndDrawSprite(*gTex_PAUSE, 107, 108, gPaletteClutIds[MAINPAL]);
    }

    if (player.cheats & CF_WARPMENU) {
        // Draw the level warp menu
        char warpmsg[64];
        std::sprintf(warpmsg, "warp to level %d", *gMapNumToCheatWarpTo);
        I_DrawString(-1, 40, warpmsg);
        I_DrawString(-1, 60, gMapNames[*gMapNumToCheatWarpTo - 1]);
    }
    else if (player.cheats & CF_VRAMVIEWER) {
        // Draw the vram viewer: first clear the background to black and then draw it
        {
            POLY_F4& polyPrim = *(POLY_F4*) getScratchAddr(128);
            LIBGPU_SetPolyF4(polyPrim);
            LIBGPU_setRGB0(polyPrim, 0, 0, 0);
            LIBGPU_setXY4(polyPrim,
                0,          0,
                SCREEN_W,   0,
                0,          SCREEN_H,
                SCREEN_W,   SCREEN_H
            );

            I_AddPrim(getScratchAddr(128));
        }
        
        a0 = *gVramViewerTexPage;
        I_VramViewerDraw();
    }
}

void I_UpdatePalette() noexcept {
loc_8003B0F0:
    v1 = *gCurPlayerIndex;
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    a1 = v1 + v0;
    v0 = lw(a1 + 0x34);
    a2 = lw(a1 + 0xD8);
    {
        const bool bJump = (v0 == 0);
        v0 = u32(i32(v0) >> 6);
        if (bJump) goto loc_8003B144;
    }
    v1 = 0xC;                                           // Result = 0000000C
    v1 -= v0;
    v0 = (i32(a2) < i32(v1));
    if (v0 == 0) goto loc_8003B144;
    a2 = v1;
loc_8003B144:
    a0 = lw(a1 + 0x44);
    *gbDoViewLighting = true;
    v0 = (i32(a0) < 0x3D);
    v1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8003B16C;
    v0 = a0 & 8;
    if (v0 == 0) goto loc_8003B174;
loc_8003B16C:
    *gbDoViewLighting = false;
loc_8003B174:
    a0 = lw(a1 + 0x30);
    v0 = (i32(a0) < 0x3D);
    {
        const bool bJump = (v0 == 0);
        v0 = a0 & 8;
        if (bJump) goto loc_8003B190;
    }
    if (v0 == 0) goto loc_8003B1A0;
loc_8003B190:
    *gbDoViewLighting = false;
    v1 = 0xE;                                           // Result = 0000000E
    goto loc_8003B210;
loc_8003B1A0:
    v0 = a2 + 7;
    if (a2 == 0) goto loc_8003B1C4;
    v1 = u32(i32(v0) >> 3);
    v0 = (i32(v1) < 8);
    if (v0 != 0) goto loc_8003B1BC;
    v1 = 7;                                             // Result = 00000007
loc_8003B1BC:
    v1++;
    goto loc_8003B210;
loc_8003B1C4:
    a0 = lw(a1 + 0x3C);
    v0 = (i32(a0) < 0x3D);
    {
        const bool bJump = (v0 == 0);
        v0 = a0 & 8;
        if (bJump) goto loc_8003B1E0;
    }
    if (v0 == 0) goto loc_8003B1E8;
loc_8003B1E0:
    v1 = 0xD;                                           // Result = 0000000D
    goto loc_8003B210;
loc_8003B1E8:
    v0 = lw(a1 + 0xDC);
    {
        const bool bJump = (v0 == 0);
        v0 += 7;
        if (bJump) goto loc_8003B210;
    }
    v1 = u32(i32(v0) >> 3);
    v0 = (i32(v1) < 4);
    v1 += 9;
    if (v0 != 0) goto loc_8003B210;
    v1 = 3;                                             // Result = 00000003
    v1 += 9;                                            // Result = 0000000C
loc_8003B210:
    v0 = v1 << 1;
    at = gPaletteClutIds;
    at += v0;
    v0 = lhu(at);
    at = 0x80070000;                                    // Result = 80070000
    *g3dViewPaletteClutId = (uint16_t) v0;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// For the given string returns the 'x' coordinate to draw it in the center of the screen.
// Assumes the big font is being used.
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t I_GetStringXPosToCenter(const char* const str) noexcept {
    // Go through the entire string and get the width of all characters that would be drawn
    int32_t width = 0;
    const char* pCurChar = str;

    for (char c = *pCurChar; c != 0; ++pCurChar, c = *pCurChar) {
        // Figure out which font character to use, and y positioning
        int32_t charIdx = 0;

        if ((c >= 'A') && (c <= 'Z')) {
            charIdx = BIG_FONT_UCASE_ALPHA + (c - 'A');
        }
        else if ((c >= 'a') && (c <= 'z')) {
            charIdx = BIG_FONT_LCASE_ALPHA + (c - 'a');
        }
        else if ((c >= '0') && (c <= '9')) {
            charIdx = BIG_FONT_DIGITS + (c - '0');
        }
        else if (c == '%') {
            charIdx = BIG_FONT_PERCENT;
        }
        else if (c == '!') {
            charIdx = BIG_FONT_EXCLAMATION;
        }
        else if (c == '.') {
            charIdx = BIG_FONT_PERIOD;
        }
        else if (c == '-') {
            charIdx = BIG_FONT_MINUS;
        }
        else {
            width += 6;     // Whitespace
            continue;
        }

        const fontchar_t& fontchar = BIG_FONT_CHARS[charIdx];
        width += fontchar.w;
    }

    // Figure out an x position to center this string in the middle of the screen
    return (SCREEN_W - width) / 2;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw the given string using the big font at the given pixel location.
// If '-1' is specified for the x coordinate, the string is drawn centered horizontally in the middle of the screen.
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawString(const int32_t x, const int32_t y, const char* const str) noexcept {
    // Set the draw mode to remove the current texture window and texture page to the STATUS graphic
    {
        DR_MODE& drawModePrim = *(DR_MODE*) getScratchAddr(128);

        // PC-PSX: define the texture window while we are at, rather than relying on the one set externally
        #if PC_PSX_DOOM_MODS
            const RECT texWindow = { 0, 0, 0, 0 };
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, &texWindow);
        #else
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, nullptr);
        #endif

        I_AddPrim(&drawModePrim);
    }

    // Some basic setup of the sprite primitive for all characters
    SPRT& spritePrim = *(SPRT*) getScratchAddr(128);

    LIBGPU_SetSprt(spritePrim);
    LIBGPU_SetShadeTex(&spritePrim, true);
    spritePrim.clut = gPaletteClutIds[UIPAL];

    // Decide on starting x position: can either be so the string is centered in the screen, or just the value verbatim
    int32_t curX = (x != -1) ? x : I_GetStringXPosToCenter(str);

    // Draw all the characters in the string
    const char* pCurChar = str;

    for (char c = *pCurChar; c != 0; ++pCurChar, c = *pCurChar) {
        // Figure out which font character to use, and y positioning
        int32_t curY = y;
        int32_t charIdx = 0;

        if ((c >= 'A') && (c <= 'Z')) {
            charIdx = BIG_FONT_UCASE_ALPHA + (c - 'A');
        }
        else if ((c >= 'a') && (c <= 'z')) {
            charIdx = BIG_FONT_LCASE_ALPHA + (c - 'a');
            curY += 3;
        }
        else if ((c >= '0') && (c <= '9')) {
            charIdx = BIG_FONT_DIGITS + (c - '0');
        }
        else if (c == '%') {
            charIdx = BIG_FONT_PERCENT;
        }
        else if (c == '!') {
            charIdx = BIG_FONT_EXCLAMATION;
        }
        else if (c == '.') {
            charIdx = BIG_FONT_PERIOD;
        }
        else if (c == '-') {
            charIdx = BIG_FONT_MINUS;
        }
        else {
            curX += 6;      // Whitespace
            continue;
        }

        // Populate and submit the sprite primitive
        const fontchar_t& fontchar = BIG_FONT_CHARS[charIdx];

        LIBGPU_setXY0(spritePrim, (int16_t) curX, (int16_t) curY);
        LIBGPU_setUV0(spritePrim, fontchar.u, fontchar.v);
        LIBGPU_setWH(spritePrim, fontchar.w, fontchar.h);
    
        I_AddPrim(&spritePrim);

        // Move past the drawn character
        curX += fontchar.w;
    }
}

void _thunk_I_DrawString() noexcept {
    I_DrawString(a0, a1, vmAddrToPtr<const char>(a2));
}
