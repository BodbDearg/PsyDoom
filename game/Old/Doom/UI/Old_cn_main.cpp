// PsyDoom: no longer using the controller configuration screen
#if !PSYDOOM_MODS

#include "Old_cn_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "m_main.h"
#include "o_main.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/PsxPadButtons.h"

// Names for all the control bindings
static const char gCtrlBindingNames[NUM_CTRL_BINDS][16] = {
    { "Attack"              },
    { "Use"                 },
    { "Strafe On"           },
    { "Speed"               },
    { "Strafe Left"         },
    { "Strafe Right"        },
    { "Weapon Backward"     },
    { "Weapon Forward"      }
};

// The default control bindings for all actions
static const padbuttons_t gDefaultCtrlBindings[NUM_CTRL_BINDS] = {
    PAD_TRIANGLE,   // cbind_attack
    PAD_CIRCLE,     // cbind_use
    PAD_CROSS,      // cbind_strafe
    PAD_SQUARE,     // cbind_run
    PAD_L1,         // cbind_strafe_left
    PAD_R1,         // cbind_strafe_right
    PAD_L2,         // cbind_prev_weapon
    PAD_R2          // cbind_next_weapon
};

// Graphic containing sprites for all of the 8 bindable buttons
texture_t gTex_BUTTONS;

//------------------------------------------------------------------------------------------------------------------------------------------
// Startup/initialization logic for the control configuration screen
//------------------------------------------------------------------------------------------------------------------------------------------
void START_ControlsScreen() noexcept {
    S_StartSound(nullptr, sfx_pistol);

    gCursorFrame = 0;
    gCursorPos[0] = 0;

    I_LoadAndCacheTexLump(gTex_BUTTONS, "BUTTONS", 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown/cleanup logic for the control configuration screen
//------------------------------------------------------------------------------------------------------------------------------------------
void STOP_ControlsScreen([[maybe_unused]] const gameaction_t exitAction) noexcept {
    S_StartSound(nullptr, sfx_pistol);

    gCursorPos[0] = 3;  // Point to the 'controls' option of the options menu
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the control configuration screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t TIC_ControlsScreen() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0)
            return ga_nothing;
    #endif

    // Animate the cursor every so often
    if ((gGameTic > gPrevGameTic) && ((gGameTic & 3) == 0)) {
        gCursorFrame ^= 1;
    }

    // Do menu up/down movements
    #if PSYDOOM_MODS
        const TickInputs& tickInputs = gTickInputs[0];
        const TickInputs& oldTickInputs = gOldTickInputs[0];

        const bool bMenuUp = tickInputs.bMenuUp;
        const bool bMenuDown = tickInputs.bMenuDown;
        const bool bMenuMove = (bMenuUp || bMenuDown || tickInputs.bMenuLeft || tickInputs.bMenuRight);
        const bool bMenuStart = tickInputs.bMenuStart;
        const bool bMenuBack = tickInputs.bMenuBack;
        const bool bMenuOk = (tickInputs.bMenuOk && (!oldTickInputs.bMenuOk));
    #else
        const uint32_t ticButtons = gTicButtons[0];
        const uint32_t oldTicButtons = gOldTicButtons[0];

        const bool bMenuUp = (ticButtons & PAD_UP);
        const bool bMenuDown = (ticButtons & PAD_DOWN);
        const bool bMenuMove = (ticButtons & PAD_DIRECTION_BTNS);
        const bool bMenuStart = (ticButtons & PAD_START);
        const bool bMenuBack = (ticButtons & PAD_SELECT);
        const bool bMenuOk = ((ticButtons != oldTicButtons) && PAD_ACTION_BTNS);
    #endif

    if (!bMenuMove) {
        // If no buttons are currently pressed then you can move immediately next time they are
        gVBlanksUntilMenuMove[0] = 0;
    } else {
        // Check to see if we can move up/down in the menu now.
        // The delay controls how often movement is repeated when up or down is held.
        gVBlanksUntilMenuMove[0] -= gPlayersElapsedVBlanks[0];

        if (gVBlanksUntilMenuMove[0] <= 0) {
            gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;

            if (bMenuDown) {
                // Down is pressed and movement is allowed: move, wraparound (if required) and play a sound
                gCursorPos[0]++;

                if (gCursorPos[0] > NUM_BINDABLE_BTNS) {
                    gCursorPos[0] = 0;
                }

                S_StartSound(nullptr, sfx_pstop);
            }
            else if (bMenuUp) {
                // Up is pressed and movement is allowed: move, wraparound (if required) and play a sound
                gCursorPos[0]--;

                if (gCursorPos[0] < 0) {
                    gCursorPos[0] = NUM_BINDABLE_BTNS;
                }

                S_StartSound(nullptr, sfx_pstop);
            }
        }
    }

    // Exit out of the controls screen if start or select are pressed
    if (bMenuStart || bMenuBack)
        return ga_exit;

    // PsyDoom: moved this check to where we are actually trying to bind a PSX button
    #if !PSYDOOM_MODS
        // Check for inputs to change control bindings if no new buttons are pressed just finish up now
        if (ticButtons == gOldTicButtons[0])
            return ga_nothing;
    #endif

    if (gCursorPos[0] < 8) {
        // PsyDoom: only getting the PSX buttons at this point and only do a PSX binding if there is a change.
        // This controls menu is purely concerned with the original PSX controller inputs, not anything the PC might be using.
        // To bind PC inputs to game actions, the configuration .ini file must be used.
        #if PSYDOOM_MODS
            const uint32_t ticButtons = gTicButtons;
            const uint32_t oldTicButtons = gOldTicButtons;

            if (ticButtons == oldTicButtons)
                return ga_nothing;
        #endif

        // Skull cursor is over a bindable action slot.
        // See if any of the buttons which are bindable have been pressed:
        for (int16_t btnIdx = 0; btnIdx < NUM_BINDABLE_BTNS; ++btnIdx) {
            const uint32_t btnMask = gBtnMasks[btnIdx];

            if (ticButtons & btnMask) {
                // This button has been pressed - assign it to the current action:
                gCtrlBindings[gCursorPos[0]] = btnMask;

                // Play a sound for the assignment and finish up button search
                S_StartSound(nullptr, sfx_swtchx);
                break;
            }
        }
    }
    else if (bMenuOk) {
        // One of the right action buttons is pressed on the default configuration slot.
        // Restore the control bindings to their defaults and play a sound to acknowledge the change:
        D_memcpy(gCtrlBindings, gDefaultCtrlBindings, sizeof(gDefaultCtrlBindings));
        S_StartSound(nullptr, sfx_swtchx);
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the control configuration screen
//------------------------------------------------------------------------------------------------------------------------------------------
void DRAW_ControlsScreen() noexcept {
    // Increment the frame count for the texture cache and draw the background using the 'MARB01' sprite
    I_IncDrawnFrameCount();

    {
        const uint16_t bgPaletteClutId = Game::getTexPalette_OptionsBg();

        for (int32_t y = 0; y < 4; ++y) {
            for (int32_t x = 0; x < 4; ++x) {
                I_CacheAndDrawSprite(gTex_OptionsBg, (int16_t) x * 64, (int16_t) y * 64, bgPaletteClutId);
            }
        }
    }

    // Screen header text
    I_DrawString(-1, 20, "Configuration");

    // Draw the skull cursor
    I_DrawSprite(
        gTex_STATUS.texPageId,
        gPaletteClutIds[UIPAL],
        12,
        (int16_t) gCursorPos[0] * 20 + 43,
        M_SKULL_TEX_U + (uint8_t) gCursorFrame * M_SKULL_W,
        M_SKULL_TEX_V,
        M_SKULL_W,
        M_SKULL_H
    );

    // Draw which button each action is bound to
    {
        const padbuttons_t* pCtrlBinding = gCtrlBindings;
        int16_t ypos = 45;

        for (int16_t ctrlBindIdx = 0; ctrlBindIdx < NUM_CTRL_BINDS; ++ctrlBindIdx, ++pCtrlBinding) {
            // Try to find which button this action is mapped to
            const padbuttons_t bindingButton = *pCtrlBinding;
            int16_t bindableBtnIdx = 0;

            {
                const padbuttons_t* pBtnMask = gBtnMasks;

                while (bindableBtnIdx < NUM_BINDABLE_BTNS) {
                    if (bindingButton == *pBtnMask)
                        break;

                    ++pBtnMask;
                    ++bindableBtnIdx;
                }
            }

            // Draw the button, unless the cursor is on it and it is flashing and currently invisible (it's flashed every 8 ticks)
            if ((gCursorPos[0] != ctrlBindIdx) || ((gTicCon & 8) == 0)) {
                constexpr uint8_t BTN_SPRITE_SIZE = 16;

                I_DrawSprite(
                    gTex_BUTTONS.texPageId,
                    Game::getTexPalette_BUTTONS(),
                    38,
                    ypos,
                    gTex_BUTTONS.texPageCoordX + (uint8_t) bindableBtnIdx * BTN_SPRITE_SIZE,
                    gTex_BUTTONS.texPageCoordY,
                    BTN_SPRITE_SIZE,
                    BTN_SPRITE_SIZE
                );
            }

            ypos += 20;
        }
    }

    // Draw the control binding names
    {
        int32_t ypos = 45;

        for (int32_t ctrlBindIdx = 0; ctrlBindIdx < NUM_CTRL_BINDS; ++ctrlBindIdx) {
            I_DrawString(65, ypos, gCtrlBindingNames[ctrlBindIdx]);
            ypos += 20;
        }
    }

    // Draw 'Default' (reset controls to default) menu option
    constexpr int32_t menuOptIdx = NUM_CTRL_BINDS;

    if ((gCursorPos[0] != menuOptIdx) || ((gTicCon & 8) == 0)) {
        I_DrawString(65, NUM_CTRL_BINDS * 20 + 45, "Default");
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

#endif  // #if !PSYDOOM_MODS
