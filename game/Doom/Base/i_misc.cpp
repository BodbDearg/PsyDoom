#include "i_misc.h"

#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/in_main.h"
#include "i_drawcmds.h"
#include "i_main.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include <cstdio>

const fontchar_t gBigFontChars[NUM_BIG_FONT_CHARS] = {
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw a number using the large font at the specified pixel location
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawNumber(const int32_t x, const int32_t y, const int32_t value) noexcept {
    // Basic setup of the drawing primitive
    SPRT& spritePrim = *(SPRT*) LIBETC_getScratchAddr(128);

    #if PC_PSX_DOOM_MODS
        // Set these primitive properties prior to drawing rather than allowing them to be undefined as in the original code.
        // I think this drawing code just happened to work because of the types of operations which occurred just before it.
        // They must have produced primitive state changes that we actually desired for this function...
        // Relying on external draw code and ordering however is brittle, so be explicit here and set exactly what we need:
        {
            // Set the draw mode to disable wrapping (zero sized text window) and texture page to the STATUS graphic
            DR_MODE& drawModePrim = *(DR_MODE*) LIBETC_getScratchAddr(128);
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
        spritePrim.tu0 = gBigFontChars[BIG_FONT_DIGITS + digit].u;
        I_AddPrim(&spritePrim);

        curX -= 11;
    }

    // Print the minus symbol if the value was negative
    if (bNegativeVal) {
        spritePrim.x0 = (int16_t) curX;
        spritePrim.tu0 = gBigFontChars[BIG_FONT_EXCLAMATION].u;
        I_AddPrim(&spritePrim);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draw a string using the very small font used for hud status bar messages
//------------------------------------------------------------------------------------------------------------------------------------------
void I_DrawStringSmall(const int32_t x, const int32_t y, const char* const str) noexcept {
    // Common sprite setup for every character.
    //
    // Note: lots of stuff like shading, color etc. is deliberately LEFT ALONE and not defined/specified here.
    // It's up to external code to customize that if it wants.
    SPRT& spritePrim = *(SPRT*) LIBETC_getScratchAddr(128);
    LIBGPU_setWH(spritePrim, 8, 8);
    spritePrim.y0 = (int16_t) y;
    
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
            POLY_F4& polyPrim = *(POLY_F4*) LIBETC_getScratchAddr(128);
            LIBGPU_SetPolyF4(polyPrim);
            LIBGPU_setRGB0(polyPrim, 0, 0, 0);
            LIBGPU_setXY4(polyPrim,
                0,          0,
                SCREEN_W,   0,
                0,          SCREEN_H,
                SCREEN_W,   SCREEN_H
            );

            I_AddPrim(LIBETC_getScratchAddr(128));
        }
        
        I_VramViewerDraw(*gVramViewerTexPage);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Decide which palette to use for the 3d view, based on the player's current status and bonuses
//------------------------------------------------------------------------------------------------------------------------------------------
void I_UpdatePalette() noexcept {
    // Decide on red amount from current damage or scaled beserk time left (pick whichever is greatest)
    player_t& player = gPlayers[*gCurPlayerIndex];
    int32_t redAmount = player.damagecount;

    if (player.powers[pw_strength] != 0) {
        const int32_t berserkAmount = 12 - (player.powers[pw_strength] >> 6);
        
        if (berserkAmount > redAmount) {
            redAmount = berserkAmount;
        }
    }

    // Deciding on various palettes and effects based on the player status.
    // A lot of these powerups start blinking when the player gets to 4 seconds or less remaining.
    *gbDoViewLighting = true;
    uint32_t paletteIdx = MAINPAL;

    if ((player.powers[pw_infrared] > TICRATE * 4) || (player.powers[pw_infrared] & 8)) {
        *gbDoViewLighting = false;
    }

    if ((player.powers[pw_invulnerability] > TICRATE * 4) || (player.powers[pw_invulnerability] & 8)) {
        *gbDoViewLighting = false;
        paletteIdx = INVULNERABILITYPAL;
    }
    else if (redAmount != 0) {
        int32_t redPalIdx = (redAmount + 7) >> 3;
        
        if (redPalIdx >= NUMREDPALS) {
            redPalIdx = NUMREDPALS - 1;
        }

        paletteIdx = STARTREDPALS + redPalIdx;
    }
    else if ((player.powers[pw_ironfeet] > TICRATE * 4) || (player.powers[pw_ironfeet] & 8)) {
        paletteIdx = RADIATIONPAL;
    }
    else if (player.bonuscount != 0) {
        int32_t bonusPalIdx = (player.bonuscount + 7) >> 3;
        
        if (bonusPalIdx >= NUMBONUSPALS) {
            bonusPalIdx = NUMBONUSPALS - 1;
        }

        paletteIdx = STARTBONUSPALS + bonusPalIdx;
    }

    // Save the palette we decided on
    *g3dViewPaletteClutId = gPaletteClutIds[paletteIdx];
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

        const fontchar_t& fontchar = gBigFontChars[charIdx];
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
        DR_MODE& drawModePrim = *(DR_MODE*) LIBETC_getScratchAddr(128);

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
    SPRT& spritePrim = *(SPRT*) LIBETC_getScratchAddr(128);

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
        const fontchar_t& fontchar = gBigFontChars[charIdx];

        LIBGPU_setXY0(spritePrim, (int16_t) curX, (int16_t) curY);
        LIBGPU_setUV0(spritePrim, fontchar.u, fontchar.v);
        LIBGPU_setWH(spritePrim, fontchar.w, fontchar.h);
    
        I_AddPrim(&spritePrim);

        // Move past the drawn character
        curX += fontchar.w;
    }
}
