//------------------------------------------------------------------------------------------------------------------------------------------
// Handles the display and logic for the password input screen.
// Note: this version is mostly based on the tweaked/improved Final Doom password screen, which includes a nice "Exit" option at the bottom.
//------------------------------------------------------------------------------------------------------------------------------------------
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
#include "PsyDoom/Controls.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/PlayerPrefs.h"
#include "PsyDoom/PsxPadButtons.h"
#include "PsyDoom/Utils.h"
#include "PsyQ/LIBGPU.h"

#include <algorithm>

// The list of password characters
const char gPasswordChars[NUM_PW_CHARS + 1] = "bcdfghjklmnpqrstvwxyz0123456789!";

static uint32_t     gInvalidPasswordFlashTicsLeft;      // How many ticks left to flash 'invalid password' for after entering a wrong password
static int32_t      gCurPasswordCharIdx;                // Which password character is currently selected/highlighted for input

int32_t     gNumPasswordCharsEntered;               // How many characters have been input for the current password sequence
uint8_t     gPasswordCharBuffer[PW_SEQ_LEN];        // The password input buffer
bool        gbUsingAPassword;                       // True if a valid password is currently being used

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: pop and return the next password character index which the user has typed in via the keyboard for the password screen.
// Returns '-1' if the user hasn't typed in a password character.
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t popTypedPasswordChar() noexcept {
    // See if any valid password characters were typed
    const char typedChar = (char) std::tolower(Input::popTypedChar());

    switch (typedChar) {
        case 'b':   return 0;
        case 'c':   return 1;
        case 'd':   return 2;
        case 'f':   return 3;
        case 'g':   return 4;
        case 'h':   return 5;
        case 'j':   return 6;
        case 'k':   return 7;
        case 'l':   return 8;
        case 'm':   return 9;
        case 'n':   return 10;
        case 'p':   return 11;
        case 'q':   return 12;
        case 'r':   return 13;
        case 's':   return 14;
        case 't':   return 15;
        case 'v':   return 16;
        case 'w':   return 17;
        case 'x':   return 18;
        case 'y':   return 19;
        case 'z':   return 20;
        case '0':   return 21;
        case '1':   return 22;
        case '2':   return 23;
        case '3':   return 24;
        case '4':   return 25;
        case '5':   return 26;
        case '6':   return 27;
        case '7':   return 28;
        case '8':   return 29;
        case '9':   return 30;
        case '!':   return 31;
    }

    return -1;  // Not a valid password char!
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the password screen
//------------------------------------------------------------------------------------------------------------------------------------------
void START_PasswordScreen() noexcept {
    S_StartSound(nullptr, sfx_pistol);

    // Reset control related stuff and password entry status
    gVBlanksUntilMenuMove[0] = 0;

    #if PSYDOOM_MODS
        for (int32_t i = 0; i < MAXPLAYERS; ++i) {
            gOldTickInputs[i] = gTickInputs[i];
        }

        gOldTicButtons = gTicButtons;
    #else
        for (int32_t i = 0; i < MAXPLAYERS; ++i) {
            gOldTicButtons[i] = gTicButtons[i];
        }
    #endif

    gInvalidPasswordFlashTicsLeft = 0;
    gCurPasswordCharIdx = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the password screen
//------------------------------------------------------------------------------------------------------------------------------------------
void STOP_PasswordScreen([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // PsyDoom: if quitting the app then exit immediately, don't play any sounds etc.
    #if PSYDOOM_MODS
        if (Input::isQuitRequested())
            return;
    #endif

    S_StartSound(nullptr, sfx_pistol);

    // Draw the screen one more time before we transition
    gCurPasswordCharIdx = 32;
    DRAW_PasswordScreen();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the password screen, entering password characters and so on...
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t TIC_PasswordScreen() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0) {
            gbKeepInputEvents = true;   // Don't consume 'key pressed' etc. events yet, not ticking...
            return ga_nothing;
        }
    #endif

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
    #if PSYDOOM_MODS
        const TickInputs& inputs = gTickInputs[0];
        const TickInputs& oldInputs = gOldTickInputs[0];

        const bool bMenuUp = inputs.fMenuUp();
        const bool bMenuDown = inputs.fMenuDown();
        const bool bMenuLeft = inputs.fMenuLeft();
        const bool bMenuRight = inputs.fMenuRight();
        const bool bMenuStart = inputs.fMenuStart();
        const bool bMenuBack = inputs.fMenuBack();
        const bool bMenuMove = (bMenuUp || bMenuDown || bMenuLeft || bMenuRight);
    #else
        const padbuttons_t ticButtons = gTicButtons[0];
        const padbuttons_t oldTicButtons = gOldTicButtons[0];

        const bool bMenuUp = (ticButtons & PAD_UP);
        const bool bMenuDown = (ticButtons & PAD_DOWN);
        const bool bMenuLeft = (ticButtons & PAD_LEFT);
        const bool bMenuRight = (ticButtons & PAD_RIGHT);
        const bool bMenuStart = (ticButtons & PAD_START);
        const bool bMenuBack = (ticButtons & PAD_SELECT);
        const bool bMenuMove = (ticButtons & PAD_DIRECTION_BTNS);
    #endif

    if (!bMenuMove) {
        // If there are no direction buttons pressed this frame then we can move immediately next time
        gVBlanksUntilMenuMove[0] = 0;
    } else {
        // Direction buttons pressed, see if it is time to move:
        gVBlanksUntilMenuMove[0] -= gPlayersElapsedVBlanks[0];

        if (gVBlanksUntilMenuMove[0] <= 0) {
            gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;

            if (bMenuUp) {
                if (gCurPasswordCharIdx >= 8) {
                    gCurPasswordCharIdx -= 8;
                    S_StartSound(nullptr, sfx_pstop);
                }
            }
            else if (bMenuDown) {
                // Note: the menu position is allowed to go onto index 'NUM_PW_CHARS', which is the 'Exit' option
                if (gCurPasswordCharIdx < NUM_PW_CHARS) {
                    gCurPasswordCharIdx = std::min(gCurPasswordCharIdx + 8, NUM_PW_CHARS);
                    S_StartSound(nullptr, sfx_pstop);
                }
            }

            if (bMenuLeft) {
                gCurPasswordCharIdx -= 1;

                if (gCurPasswordCharIdx < 0) {
                    gCurPasswordCharIdx = 0;
                } else {
                    S_StartSound(nullptr, sfx_pstop);
                }
            }
            else if (bMenuRight) {
                // Note: the menu position is allowed to go onto index 'NUM_PW_CHARS', which is the 'Exit' option
                gCurPasswordCharIdx += 1;

                if (gCurPasswordCharIdx > NUM_PW_CHARS) {
                    gCurPasswordCharIdx = NUM_PW_CHARS;
                } else {
                    S_StartSound(nullptr, sfx_pstop);
                }
            }
        }
    }

    // Exit this screen if start or select are pressed
    if (bMenuStart | bMenuBack)
        return ga_exit;

    // Nothing more to do if new buttons are not pressed
    #if !PSYDOOM_MODS
        if (ticButtons == oldTicButtons)
            return ga_nothing;
    #endif

    // Entering or deleting a password character, or pressing the 'menu ok' command?
    // PsyDoom: allow password characters to be typed with the keyboard also.
    #if PSYDOOM_MODS
        const int32_t typedPasswordCharIdx = popTypedPasswordChar();
        const bool bMenuOkPressed = (inputs.fMenuOk() && (!oldInputs.fMenuOk()));
        const bool bEnterPasswordChar = (
            (inputs.fEnterPasswordChar() && (!oldInputs.fEnterPasswordChar())) ||
            (typedPasswordCharIdx >= 0)
        );
        const bool bDeletePasswordChar = (inputs.fDeletePasswordChar() && (!oldInputs.fDeletePasswordChar()));
    #else
        const bool bEnterPasswordChar = (ticButtons & (PAD_SQUARE | PAD_CROSS | PAD_CIRCLE));
        const bool bDeletePasswordChar = (ticButtons & PAD_TRIANGLE);
    #endif

    // If we are on the 'Exit' option then handle that specially
    if (gCurPasswordCharIdx == NUM_PW_CHARS) {
        return (bMenuOkPressed) ? ga_exit : ga_nothing;
    }

    // Handle entering or deleting a password character otherwise 
    if (bEnterPasswordChar) {
        // Entering a password character (if there is a free slot)
        S_StartSound(nullptr, sfx_swtchx);

        if (gNumPasswordCharsEntered < PW_SEQ_LEN) {
            // PsyDoom: allow passwords to be typed in with the keyboard also
            #if PSYDOOM_MODS
                if (typedPasswordCharIdx >= 0) {
                    gPasswordCharBuffer[gNumPasswordCharsEntered] = (uint8_t) typedPasswordCharIdx;
                } else {
                    gPasswordCharBuffer[gNumPasswordCharsEntered] = (uint8_t) gCurPasswordCharIdx;
                }
            #else
                gPasswordCharBuffer[gNumPasswordCharsEntered] = (uint8_t) gCurPasswordCharIdx;
            #endif

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

            // PsyDoom: remember this password in player prefs, so it's restored on relaunch
            #if PSYDOOM_MODS
                PlayerPrefs::pullLastPassword();
            #endif

            return ga_warped;
        }
        else {
            // Invalid password entered, flash the message for a bit
            gInvalidPasswordFlashTicsLeft = 16;
        }
    }
    else if (bDeletePasswordChar) {
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

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    {
        const uint16_t bgPaletteClutId = Game::getTexPalette_OptionsBg();

        for (int16_t y = 0; y < 4; ++y) {
            for (int16_t x = 0; x < 4; ++x) {
                I_CacheAndDrawSprite(gTex_OptionsBg, x * 64, y * 64, bgPaletteClutId);
            }
        }
    }

    // Setup the draw mode
    {
        // PsyDoom: explicitly clear the texture window here also to disable wrapping - don't rely on previous drawing code to do that
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        #if PSYDOOM_MODS
            DR_MODE drawModePrim = {};
            const SRECT texWindow = { (int16_t) gTex_STATUS.texPageCoordX, (int16_t) gTex_STATUS.texPageCoordY, 256, 256 };
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS.texPageId, &texWindow);
        #else
            DR_MODE& drawModePrim = *(DR_MODE*) LIBETC_getScratchAddr(128);
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS.texPageId, nullptr);
        #endif

        I_AddPrim(drawModePrim);
    }

    // Common sprite setup for all the password chars
    #if PSYDOOM_MODS
        // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
        SPRT spritePrim = {};
    #else
        SPRT& spritePrim = *(SPRT*) LIBETC_getScratchAddr(128);
    #endif

    LIBGPU_SetSprt(spritePrim);
    spritePrim.clut = Game::getTexPalette_STATUS();

    // Draw the array of password characters
    for (int32_t pwCharIdx = 0; pwCharIdx < NUM_PW_CHARS; ++pwCharIdx) {
        // Determine where to place the character
        constexpr int32_t CHARS_PER_ROW = 8;
        constexpr int32_t CHAR_SPACING = 20;

        const int32_t charRow = pwCharIdx / CHARS_PER_ROW;
        const int32_t charCol = pwCharIdx - charRow * CHARS_PER_ROW;

        int16_t charX = (int16_t)((charCol * CHAR_SPACING) + 48);
        int16_t charY = (int16_t)((charRow * CHAR_SPACING) + 60);

        // Flash (don't display every 8 frames) the currently selected password character and make it over-bright
        if (gCurPasswordCharIdx == pwCharIdx) {
            if (gTicCon & 8)
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

        I_AddPrim(spritePrim);
    }

    // Draw the screen title
    I_DrawString(-1, 20, "Password");

    if (gInvalidPasswordFlashTicsLeft & 4) {
        // Flash the invalid password message if a password was entered wrong
        I_DrawString(-1, 160, "Invalid Password");
    }
    else {
        // Draw password characters that were input
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
    }

    // Draw the exit option
    if ((gCurPasswordCharIdx != NUM_PW_CHARS) || ((gTicCon & 8) == 0)) {
        I_DrawString(-1, 200, "Exit");
    }

    // Finish up the draw
    I_SubmitGpuCmds();
    I_DrawPresent();
}
