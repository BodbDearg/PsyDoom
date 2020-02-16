#include "pw_main.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_password.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"
#include "o_main.h"

// The list of password characters
const char gPasswordChars[NUM_PW_CHARS + 1] = "bcdfghjklmnpqrstvwxyz0123456789!";

static const VmPtr<uint32_t>    gInvalidPasswordFlashTicsLeft(0x8007802C);  // How many ticks left to flash 'invalid password' for after entering a wrong password
static const VmPtr<int32_t>     gCurPasswordCharIdx(0x80078174);            // Which password character is currently selected/highlighted for input

// How many characters have been input for the current password sequence and the password input buffer
const VmPtr<int32_t>                gNumPasswordCharsEntered(0x80077C40);
const VmPtr<uint8_t[PW_SEQ_LEN]>    gPasswordCharBuffer(0x80096560);

void START_PasswordScreen() noexcept {
    sp -= 0x18;
    
    a0 = 0;    
    a1 = sfx_pistol;
    S_StartSound();

    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F44);                               // Load from: gTicButtons[0] (80077F44)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F48);                               // Load from: gTicButtons[1] (80077F48)
    *gInvalidPasswordFlashTicsLeft = 0;
    *gCurPasswordCharIdx = 0;
    gVBlanksUntilMenuMove[0] = 0;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7DEC);                                // Store to: gOldTicButtons[0] (80078214)
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7DE8);                                // Store to: gOldTicButtons[1] (80078218)

    sp += 0x18;
}

void STOP_PasswordScreen() noexcept {
    sp -= 0x18;
    a0 = 0;
    sw(ra, sp + 0x10);
    a1 = sfx_pistol;
    S_StartSound();
    v0 = 0x20;                                          // Result = 00000020
    *gCurPasswordCharIdx = v0;
    DRAW_PasswordScreen();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void TIC_PasswordScreen() noexcept {
    a0 = *gInvalidPasswordFlashTicsLeft;
    sp -= 0x28;
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    if (a0 == 0) goto loc_80036EF4;
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = a0 - 1;
        if (bJump) goto loc_80036EF4;
    }
    *gInvalidPasswordFlashTicsLeft = v0;
    v0 &= 7;
    v1 = 4;
    a0 = 0;
    if (v0 != v1) goto loc_80036EF4;
    a1 = sfx_itemup;
    S_StartSound();
loc_80036EF4:
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x7F44);                               // Load from: gTicButtons[0] (80077F44)
    s1 = 0x80080000;                                    // Result = 80080000
    s1 = lw(s1 - 0x7DEC);                               // Load from: gOldTicButtons[0] (80078214)
    v0 = s0 & 0xF000;
    {
        const bool bJump = (v0 != 0);
        v0 = s0 & 0x900;
        if (bJump) goto loc_80036F1C;
    }
    gVBlanksUntilMenuMove[0] = 0;
    goto loc_8003700C;
loc_80036F1C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7EF8;                                       // Result = gVBlanksUntilMenuMove (80077EF8)
    v0 = gVBlanksUntilMenuMove[0];
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FBC);                               // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v0 -= v1;
    gVBlanksUntilMenuMove[0] = v0;
    if (i32(v0) > 0) goto loc_80037008;
    gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;
    v0 = s0 & 0x1000;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x4000;
        if (bJump) goto loc_80036F70;
    }
    v1 = *gCurPasswordCharIdx;
    v0 = (i32(v1) < 8);
    {
        const bool bJump = (v0 != 0);
        v0 = s0 & 0x8000;
        if (bJump) goto loc_80036FA0;
    }
    v0 = v1 - 8;
    goto loc_80036F8C;
loc_80036F70:
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x8000;
        if (bJump) goto loc_80036FA0;
    }
    v1 = *gCurPasswordCharIdx;
    v0 = (i32(v1) < 0x18);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 + 8;
        if (bJump) goto loc_80036F9C;
    }
loc_80036F8C:
    *gCurPasswordCharIdx = v0;
    a0 = 0;
    a1 = sfx_pstop;
    S_StartSound();
loc_80036F9C:
    v0 = s0 & 0x8000;
loc_80036FA0:
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x2000;
        if (bJump) goto loc_80036FCC;
    }
    v0 = *gCurPasswordCharIdx;
    v0--;
    *gCurPasswordCharIdx = v0;
    a0 = 0;                                             // Result = 00000000
    if (i32(v0) >= 0) goto loc_80037000;
    *gCurPasswordCharIdx = 0;
    v0 = s0 & 0x900;
    goto loc_8003700C;
loc_80036FCC:
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x900;
        if (bJump) goto loc_8003700C;
    }
    v0 = *gCurPasswordCharIdx;
    v0++;
    *gCurPasswordCharIdx = v0;
    v0 = (i32(v0) < 0x20);
    a0 = 0;
    if (v0 != 0) goto loc_80037000;
    v0 = 0x1F;
    *gCurPasswordCharIdx = v0;
    v0 = s0 & 0x900;
    goto loc_8003700C;
loc_80037000:
    a1 = sfx_pstop;
    S_StartSound();
loc_80037008:
    v0 = s0 & 0x900;
loc_8003700C:
    {
        const bool bJump = (v0 != 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_8003711C;
    }
    v0 = s0 & 0xE0;
    if (s0 == s1) goto loc_80037118;
    a0 = 0;
    if (v0 == 0) goto loc_800370D0;
    a1 = sfx_swtchx;
    S_StartSound();
    a0 = *gNumPasswordCharsEntered;
    v0 = (i32(a0) < 0xA);
    a2 = sp + 0x14;
    if (v0 == 0) goto loc_80037070;
    v1 = *gCurPasswordCharIdx;
    v0 = a0 + 1;
    *gNumPasswordCharsEntered = v0;
    at = gPasswordCharBuffer;
    at += a0;
    sb(v1, at);
    v0 = *gNumPasswordCharsEntered;
    v0 = (i32(v0) < 0xA);
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003711C;
    }
loc_80037070:
    a0 = gPasswordCharBuffer;
    a1 = sp + 0x10;
    a3 = 0;                                             // Result = 00000000
    P_ProcessPassword();
    a0 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_800370C0;
    v1 = lw(sp + 0x10);
    a1 = lw(sp + 0x14);
    sw(a0, gp + 0x65C);                                 // Store to: gbUsingAPassword (80077C3C)
    *gGameMap = v1;
    *gStartMapOrEpisode = v1;
    *gGameSkill = (skill_t) a1;
    *gStartSkill = (skill_t) a1;
    v0 = 4;
    goto loc_8003711C;
loc_800370C0:
    v0 = 0x10;                                          // Result = 00000010
    *gInvalidPasswordFlashTicsLeft = v0;
    v0 = 0;                                             // Result = 00000000
    goto loc_8003711C;
loc_800370D0:
    v0 = s0 & 0x10;
    {
        const bool bJump = (v0 == 0);
        v0 = 0;
        if (bJump) goto loc_8003711C;
    }
    a1 = sfx_swtchx;
    S_StartSound();
    v0 = *gNumPasswordCharsEntered;
    v0--;
    *gNumPasswordCharsEntered = v0;
    if (i32(v0) >= 0) goto loc_80037100;
    *gNumPasswordCharsEntered = 0;
loc_80037100:
    v0 = *gNumPasswordCharsEntered;
    at = gPasswordCharBuffer;
    at += v0;
    sb(0, at);
loc_80037118:
    v0 = 0;                                             // Result = 00000000
loc_8003711C:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the password screen
//------------------------------------------------------------------------------------------------------------------------------------------
void DRAW_PasswordScreen() noexcept {
    // Increment the frame count for the texture cache and draw the background using the 'MARB01' sprite
    I_IncDrawnFrameCount();

    for (int16_t y = 0; y < 4; ++y) {
        for (int16_t x = 0; x < 4; ++x) {
            I_CacheAndDrawSprite(*gTex_MARB01, x * 64, y * 64, gPaletteClutIds[MAINPAL]);
        }
    }

    // Setup the draw mode
    {
        DR_MODE& drawModePrim = *(DR_MODE*) getScratchAddr(128);

        // PC-PSX: explicitly clear the texture window here also to disable wrapping - don't rely on previous drawing code to do that
        #if PC_PSX_DOOM_MODS
            RECT texWindow = { 0, 0, 0, 0 };
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, &texWindow);
        #else
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS->texPageId, nullptr);
        #endif

        I_AddPrim(&drawModePrim);
    }

    // Common sprite setup for all the password chars
    SPRT& spritePrim = *(SPRT*) getScratchAddr(128);

    LIBGPU_SetSprt(spritePrim);
    spritePrim.clut = gPaletteClutIds[UIPAL];

    // Draw the array of password characters
    for (int32_t pwCharIdx = 0; pwCharIdx < NUM_PW_CHARS; ++pwCharIdx) {
        // Determine where to place the character
        constexpr int32_t CHARS_PER_ROW = 8;
        constexpr int32_t CHAR_SPACING = 20;

        const int32_t charRow = pwCharIdx / CHARS_PER_ROW;
        const int32_t charCol = pwCharIdx - charRow * CHARS_PER_ROW;

        int16_t charX = (int16_t)((charCol * CHAR_SPACING) + 48);
        int16_t charY = (int16_t)((charRow * CHAR_SPACING) + 60);

        // Flash (don't display every 4 frames) the currently selected password character and make it over-bright
        if (*gCurPasswordCharIdx == pwCharIdx) {
            if ((*gTicCon) & 4)
                continue;
            
            LIBGPU_setRGB0(spritePrim, 255, 0, 0);          // Selected characters are color multiplied almost 2x and only red
        } else {
            LIBGPU_setRGB0(spritePrim, 128, 128, 128);      // Regular characters have normal brightness
        }

        // Figure out which big font character to display
        const char pwChar = gPasswordChars[pwCharIdx];
        int32_t bigFontCharIdx = 0;

        if (pwChar >= 'a' && pwChar <= 'z') {
            bigFontCharIdx = BIG_FONT_LCASE_ALPHA + (pwChar - 'a');
            charY = (int16_t)((charRow * CHAR_SPACING) + 63);           // Tweak y position for smaller lowercase chars
        } 
        else if (pwChar >= '0' && pwChar <= '9') {
            bigFontCharIdx = BIG_FONT_DIGITS + (pwChar - '0');
        } 
        else if (pwChar == '!') {
            bigFontCharIdx = BIG_FONT_EXCLAMATION;
        }

        const fontchar_t& fontChar = gBigFontChars[bigFontCharIdx];

        // Setup and submit the sprite primitive for this password character
        LIBGPU_setXY0(spritePrim, charX, charY);
        LIBGPU_setUV0(spritePrim, fontChar.u, fontChar.v);
        LIBGPU_setWH(spritePrim, fontChar.w, fontChar.h);
        
        I_AddPrim(&spritePrim);
    }

    // Draw the screen title
    I_DrawString(-1, 20, "Password");

    // Draw password characters that were input
    {
        constexpr int32_t CHARS_START_X = 58;
        constexpr int32_t CHARS_SPACING = 14;

        {
            char charStr[2];
            charStr[1] = 0;

            int32_t xpos = CHARS_START_X;
        
            for (int32_t charIdx = 0; charIdx < *gNumPasswordCharsEntered; ++charIdx) {
                const uint8_t pwCharIdx = gPasswordCharBuffer[charIdx];
                charStr[0] = gPasswordChars[pwCharIdx];
                I_DrawString(xpos, 160, charStr);
                xpos += CHARS_SPACING;
            }
        }
        
        // Draw password characters that need to be input
        {
            int32_t xpos = (*gNumPasswordCharsEntered) * CHARS_SPACING + CHARS_START_X;

            for (int32_t charIdx = *gNumPasswordCharsEntered; charIdx < PW_SEQ_LEN; ++charIdx) {
                I_DrawString(xpos, 160, ".");
                xpos += CHARS_SPACING;
            }
        }
        
        // Flash the invalid password message if a password was entered wrong
        if (*gInvalidPasswordFlashTicsLeft & 4) {
            I_DrawString(-1, 200, "Invalid Password");
        }
    }

    // Finish up the draw
    I_SubmitGpuCmds();
    I_DrawPresent();
}
