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
#include "o_main.h"
#include "PsyQ/LIBETC.h"
#include "PsyQ/LIBGPU.h"

// The list of password characters
const char gPasswordChars[NUM_PW_CHARS + 1] = "bcdfghjklmnpqrstvwxyz0123456789!";

static uint32_t     gInvalidPasswordFlashTicsLeft;      // How many ticks left to flash 'invalid password' for after entering a wrong password
static int32_t      gCurPasswordCharIdx;                // Which password character is currently selected/highlighted for input

int32_t     gNumPasswordCharsEntered;               // How many characters have been input for the current password sequence
uint8_t     gPasswordCharBuffer[PW_SEQ_LEN];        // The password input buffer
bool        gbUsingAPassword;                       // True if a valid password is currently being used

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the password screen
//------------------------------------------------------------------------------------------------------------------------------------------
void START_PasswordScreen() noexcept {
    S_StartSound(nullptr, sfx_pistol);

    // Reset control related stuff and password entry status
    gVBlanksUntilMenuMove[0] = 0;
    gOldTicButtons[0] = gTicButtons[0];
    gOldTicButtons[1] = gTicButtons[1];

    gInvalidPasswordFlashTicsLeft = 0;
    gCurPasswordCharIdx = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the password screen
//------------------------------------------------------------------------------------------------------------------------------------------
void STOP_PasswordScreen([[maybe_unused]] const gameaction_t exitAction) noexcept {
    S_StartSound(nullptr, sfx_pistol);

    // Draw the screen one more time before we transition
    gCurPasswordCharIdx = 32;
    DRAW_PasswordScreen();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the password screen, entering password characters and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t TIC_PasswordScreen() noexcept {
    // Do invalid password flash sfx every so often if currently active
    if (gInvalidPasswordFlashTicsLeft != 0) {
        if (gGameTic > gPrevGameTic) {
            gInvalidPasswordFlashTicsLeft -= 1;
            
            if ((gInvalidPasswordFlashTicsLeft & 7) == 4) {
                S_StartSound(nullptr, sfx_itemup);
            }
        }
    }

    // Handle up/down/left/right movements
    const padbuttons_t ticButtons = gTicButtons[0];
    const padbuttons_t oldTicButtons = gOldTicButtons[0];

    if ((ticButtons & PAD_DIRECTION_BTNS) == 0) {
        // If there are no direction buttons pressed this frame then we can move immediately next time
        gVBlanksUntilMenuMove[0] = 0;
    } else {
        // Direction buttons pressed, see if it is time to move:
        gVBlanksUntilMenuMove[0] -= gPlayersElapsedVBlanks[0];

        if (gVBlanksUntilMenuMove[0] <= 0) {
            gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;
            
            if (ticButtons & PAD_UP) {
                if (gCurPasswordCharIdx >= 8) {
                    gCurPasswordCharIdx -= 8;
                    S_StartSound(nullptr, sfx_pstop);
                }
            }
            else if (ticButtons & PAD_DOWN) {
                if (gCurPasswordCharIdx + 8 < NUM_PW_CHARS) {
                    gCurPasswordCharIdx += 8;
                    S_StartSound(nullptr, sfx_pstop);
                }
            }

            if (ticButtons & PAD_LEFT) {
                gCurPasswordCharIdx -= 1;

                if (gCurPasswordCharIdx < 0) {
                    gCurPasswordCharIdx = 0;
                } else {
                    S_StartSound(nullptr, sfx_pstop);
                }
            }
            else if (ticButtons & PAD_RIGHT) {
                gCurPasswordCharIdx += 1;

                if (gCurPasswordCharIdx >= NUM_PW_CHARS) {
                    gCurPasswordCharIdx = NUM_PW_CHARS - 1;
                } else {
                    S_StartSound(nullptr, sfx_pstop);
                }
            }
        }
    }

    // Exit this screen if start or select are pressed
    if (ticButtons & (PAD_START | PAD_SELECT))
        return ga_exit;

    // Nothing more to do if new buttons are not pressed
    if (ticButtons == oldTicButtons)
        return ga_nothing;

    if (ticButtons & (PAD_SQUARE | PAD_CROSS | PAD_CIRCLE)) {
        // Entering a password character (if there is a free slot)
        S_StartSound(nullptr, sfx_swtchx);

        if (gNumPasswordCharsEntered < PW_SEQ_LEN) {
            gPasswordCharBuffer[gNumPasswordCharsEntered] = (uint8_t) gCurPasswordCharIdx;
            gNumPasswordCharsEntered++;
            
            // If the password sequence is not yet complete then there is nothing more to do
            if (gNumPasswordCharsEntered < PW_SEQ_LEN)
                return ga_nothing;
        }
        
        // Process the password once it is complete
        int32_t mapNum = {};
        skill_t skill = {};
        
        if (P_ProcessPassword(gPasswordCharBuffer, mapNum, skill, nullptr)) {
            // Valid password entered, begin warping to the destination map
            gbUsingAPassword = true;
            gGameMap = mapNum;
            gStartMapOrEpisode = mapNum;
            gGameSkill = skill;
            gStartSkill = skill;

            return ga_warped;
        }
        else {
            // Invalid password entered, flash the message for a bit
            gInvalidPasswordFlashTicsLeft = 16;
        }
    }
    else if (ticButtons & PAD_TRIANGLE) {
        // Delete a password sequence character and reset it's value in the buffer to the default
        S_StartSound(nullptr, sfx_swtchx);
        gNumPasswordCharsEntered--;

        if (gNumPasswordCharsEntered < 0) {
            gNumPasswordCharsEntered = 0;
        }

        gPasswordCharBuffer[gNumPasswordCharsEntered] = 0;
    }

    return ga_nothing;
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
        DR_MODE& drawModePrim = *(DR_MODE*) LIBETC_getScratchAddr(128);

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
    SPRT& spritePrim = *(SPRT*) LIBETC_getScratchAddr(128);

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
        if (gCurPasswordCharIdx == pwCharIdx) {
            if (gTicCon & 4)
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
        
            for (int32_t charIdx = 0; charIdx < gNumPasswordCharsEntered; ++charIdx) {
                const uint8_t pwCharIdx = gPasswordCharBuffer[charIdx];
                charStr[0] = gPasswordChars[pwCharIdx];
                I_DrawString(xpos, 160, charStr);
                xpos += CHARS_SPACING;
            }
        }
        
        // Draw password characters that need to be input
        {
            int32_t xpos = gNumPasswordCharsEntered * CHARS_SPACING + CHARS_START_X;

            for (int32_t charIdx = gNumPasswordCharsEntered; charIdx < PW_SEQ_LEN; ++charIdx) {
                I_DrawString(xpos, 160, ".");
                xpos += CHARS_SPACING;
            }
        }
        
        // Flash the invalid password message if a password was entered wrong
        if (gInvalidPasswordFlashTicsLeft & 4) {
            I_DrawString(-1, 200, "Invalid Password");
        }
    }

    // Finish up the draw
    I_SubmitGpuCmds();
    I_DrawPresent();
}
