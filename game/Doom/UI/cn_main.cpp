#include "cn_main.h"

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
#include "PsxVm/PsxVm.h"

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

// Graphic containing sprites for all of the 8 bindable buttons
const VmPtr<texture_t> gTex_BUTTONS(0x80097AD0);

void START_ControlsScreen() noexcept {
    sp -= 0x18;
    a0 = 0;
    sw(ra, sp + 0x10);
    a1 = sfx_pistol;
    S_StartSound();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7AD0;                                       // Result = gTex_BUTTONS[0] (80097AD0)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C5C;                                       // Result = STR_LumpName_BUTTONS[0] (80077C5C)
    sw(0, gp + 0xBF8);                                  // Store to: gCursorFrame (800781D8)
    gCursorPos[0] = 0;
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void STOP_ControlsScreen() noexcept {
    sp -= 0x18;
    a0 = 0;
    sw(ra, sp + 0x10);
    a1 = sfx_pistol;
    S_StartSound();
    v0 = 3;                                             // Result = 00000003
    gCursorPos[0] = v0;
    ra = lw(sp + 0x10);
    sp += 0x18;
}

void TIC_ControlsScreen() noexcept {
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = (i32(v0) < i32(v1));
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_800379F0;
    v0 = v1 & 3;
    if (v0 != 0) goto loc_800379F0;
    v0 = lw(gp + 0xBF8);                                // Load from: gCursorFrame (800781D8)
    v0 ^= 1;
    sw(v0, gp + 0xBF8);                                 // Store to: gCursorFrame (800781D8)
loc_800379F0:
    s0 = 0x80070000;                                    // Result = 80070000
    s0 = lw(s0 + 0x7F44);                               // Load from: gTicButtons[0] (80077F44)
    s1 = 0x80080000;                                    // Result = 80080000
    s1 = lw(s1 - 0x7DEC);                               // Load from: gOldTicButtons[0] (80078214)
    v0 = s0 & 0xF000;
    {
        const bool bJump = (v0 != 0);
        v0 = s0 & 0x900;
        if (bJump) goto loc_80037A18;
    }
    
    *gVBlanksUntilMenuMove = 0;
    goto loc_80037AB8;
loc_80037A18:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7EF8;                                       // Result = gVBlanksUntilMenuMove (80077EF8)
    v0 = *gVBlanksUntilMenuMove;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FBC);                               // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v0 -= v1;
    *gVBlanksUntilMenuMove = v0;
    if (i32(v0) > 0) goto loc_80037AB4;
    *gVBlanksUntilMenuMove = MENU_MOVE_VBLANK_DELAY;
    v0 = s0 & 0x4000;
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x1000;
        if (bJump) goto loc_80037A7C;
    }
    v1 = gCursorPos;
    v0 = gCursorPos[0];
    v0++;
    sw(v0, v1);                                         // Store to: gCursorPos (80078000)
    v0 = (i32(v0) < 9);
    a0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80037AAC;
    sw(0, v1);                                          // Store to: gCursorPos (80078000)
    goto loc_80037AAC;
loc_80037A7C:
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x900;
        if (bJump) goto loc_80037AB8;
    }
    v1 = gCursorPos;
    v0 = gCursorPos[0];
    v0--;
    sw(v0, v1);                                         // Store to: gCursorPos (80078000)
    if (i32(v0) >= 0) goto loc_80037AA8;
    v0 = 8;                                             // Result = 00000008
    sw(v0, v1);                                         // Store to: gCursorPos (80078000)
loc_80037AA8:
    a0 = 0;                                             // Result = 00000000
loc_80037AAC:
    a1 = sfx_pstop;
    S_StartSound();
loc_80037AB4:
    v0 = s0 & 0x900;
loc_80037AB8:
    {
        const bool bJump = (v0 != 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_80037B6C;
    }
    v0 = 0;                                             // Result = 00000000
    if (s0 == s1) goto loc_80037B6C;
    v0 = gCursorPos[0];
    v0 = (i32(v0) < 8);
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80037B34;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x3DEC;                                       // Result = gBtnSprite_Triangle_ButtonMask (80073DEC)
loc_80037AE4:
    a2 = lw(v1);
    v0 = s0 & a2;
    a0++;
    if (v0 != 0) goto loc_80037B0C;
    v0 = (i32(a0) < 8);
    v1 += 4;
    if (v0 != 0) goto loc_80037AE4;
    v0 = 0;                                             // Result = 00000000
    goto loc_80037B6C;
loc_80037B0C:
    a0 = 0;                                             // Result = 00000000
    v0 = gCursorPos[0];
    v0 <<= 2;
    at = gCtrlBindings;
    at += v0;
    sw(a2, at);
    a1 = 0x17;
    goto loc_80037B60;
loc_80037B34:
    v0 = s0 & 0xF0;
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80037B6C;
    }
    a0 = gCtrlBindings;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x3E2C;                                       // Result = DefaultBtnBinding_Attack (80073E2C)
    a2 = 0x20;
    _thunk_D_memcpy();
    a0 = 0;
    a1 = sfx_swtchx;
loc_80037B60:
    S_StartSound();
    v0 = 0;
loc_80037B6C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Renders the control configuration screen
//------------------------------------------------------------------------------------------------------------------------------------------
void DRAW_ControlsScreen() noexcept {
    // Increment the frame count for the texture cache and draw the background using the 'MARB01' sprite
    I_IncDrawnFrameCount();

    for (int32_t y = 0; y < 4; ++y) {
        for (int32_t x = 0; x < 4; ++x) {
            I_CacheAndDrawSprite(*gTex_MARB01, (int16_t) x * 64, (int16_t) y * 64, gPaletteClutIds[MAINPAL]);
        }
    }

    // Screen header text
    I_DrawString(-1, 20, "Configuration");

    // Draw the skull cursor    
    I_DrawSprite(
        gTex_STATUS->texPageId,
        gPaletteClutIds[UIPAL],
        12,
        (int16_t) gCursorPos[0] * 20 + 43,
        M_SKULL_TEX_U + (uint8_t)(*gCursorFrame) * M_SKULL_W,
        M_SKULL_TEX_V,
        M_SKULL_W,
        M_SKULL_H
    );

    // Draw which button each action is bound to
    {
        const padbuttons_t* pCtrlBinding = gCtrlBindings.get();
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
            if ((gCursorPos[0] != ctrlBindIdx) || ((*gTicCon & 8) == 0)) {
                constexpr uint8_t BTN_SPRITE_SIZE = 16;

                I_DrawSprite(
                    gTex_BUTTONS->texPageId,
                    gPaletteClutIds[MAINPAL],
                    38,
                    ypos,
                    gTex_BUTTONS->texPageCoordX + (uint8_t) bindableBtnIdx * BTN_SPRITE_SIZE,
                    gTex_BUTTONS->texPageCoordY,
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

    if ((gCursorPos[0] != menuOptIdx) || ((*gTicCon & 8) == 0)) {
        I_DrawString(65, NUM_CTRL_BINDS * 20 + 45, "Default");
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}
