#include "o_main.h"

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
#include "PsxVm/PsxVm.h"
#include "pw_main.h"

// Available options and their names
enum option_t : uint32_t {
    opt_music,
    opt_sound,
    opt_password,
    opt_config,
    opt_main_menu,
    opt_restart
};

const char gOptionNames[][16] = {
    { "Music Volume"    },
    { "Sound Volume"    },
    { "Password"        },
    { "Configuration"   },
    { "Main Menu"       },
    { "Restart Level"   }
};

// The layout for the options menu: outside of gameplay, in gameplay (single player) and in gameplay (multiplayer)
struct menuitem_t {
    option_t    option;
    int32_t     x;
    int32_t     y;
};

static const menuitem_t gOptMenuItems_MainMenu[] = {
    { opt_music,        62, 65  },
    { opt_sound,        62, 105 },
    { opt_password,     62, 145 },
    { opt_config,       62, 170 },
    { opt_main_menu,    62, 195 },
};

static const menuitem_t gOptMenuItems_Single[] = {
    { opt_music,        62, 50  },
    { opt_sound,        62, 90  },
    { opt_password,     62, 130 },
    { opt_config,       62, 155 },
    { opt_main_menu,    62, 180 },
    { opt_restart,      62, 205 },
};

static const menuitem_t gOptMenuItems_NetGame[] = {
    { opt_music,        62, 70  },
    { opt_sound,        62, 110 },
    { opt_main_menu,    62, 150 },
    { opt_restart,      62, 175 },
};

// Currently in-use options menu layout: items list and size
static const VmPtr<int32_t>             gOptionsMenuSize(0x80078118);
static const VmPtr<VmPtr<menuitem_t>>   gpOptionsMenuItems(0x800782C0);

// The marble floor texture used as a background for the options menu
const VmPtr<texture_t> gTex_MARB01(0x80097AB0);

// Current options music and sound volume
const VmPtr<int32_t> gOptionsSndVol(0x800775F0);
const VmPtr<int32_t> gOptionsMusVol(0x800775F4);

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the options menu
//------------------------------------------------------------------------------------------------------------------------------------------
void O_Init() noexcept {
    // BAM!
    a0 = 0;
    a1 = sfx_pistol;
    S_StartSound();

    // Initialize cursor position and vblanks until move for all players
    *gCursorFrame = 0;

    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        gCursorPos[playerIdx] = 0;
        gVBlanksUntilMenuMove[playerIdx] = 0;
    }

    // Set what menu layout to use
    if (*gNetGame != gt_single) {
        *gpOptionsMenuItems = 0x80074BD0;   // TODO: OptionsMenuEntries_NetGame[0] (80074BD0)
        *gOptionsMenuSize = 4;
    }    
    else if (*gbGamePaused) {
        *gpOptionsMenuItems = 0x80074B88;   // TODO: OptionsMenuEntries_InGame[0] (80074B88)
        *gOptionsMenuSize = 6;
    }
    else {
        *gpOptionsMenuItems = 0x80074B4C;   // TODO: OptionsMenuEntries_MainMenu[0] (80074B4C)
        *gOptionsMenuSize = 5;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down the options menu
//------------------------------------------------------------------------------------------------------------------------------------------
void O_Shutdown([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // Reset the cursor position for all players
    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        gCursorPos[playerIdx] = 0;
    }
}

void _thunk_O_Shutdown() noexcept {
    O_Shutdown((gameaction_t) a0);
}

void O_Control() noexcept {
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    sp -= 0x30;
    sw(ra, sp + 0x28);
    sw(s5, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = (i32(v0) < i32(v1));
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_8003EA50;
    v0 = v1 & 3;
    s2 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_8003EA54;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7E28);                               // Load from: gCursorFrame (800781D8)
    v0 ^= 1;
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7E28);                                // Store to: gCursorFrame (800781D8)
loc_8003EA50:
    s2 = 1;                                             // Result = 00000001
loc_8003EA54:
    s3 = 0x51EB0000;                                    // Result = 51EB0000
    s3 |= 0x851F;                                       // Result = 51EB851F
    s5 = gCursorPos;
    s4 = s5 + 4;                                        // Result = DefaultCursorPos (80078004)
    s1 = 4;                                             // Result = 00000004
loc_8003EA6C:
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7F54;                                       // Result = gbPlayerInGame[0] (800780AC)
    at += s1;
    v0 = lw(at);
    if (v0 == 0) goto loc_8003EE8C;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gTicButtons[0] (80077F44)
    at += s1;
    s0 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gOldTicButtons[0] (80078214)
    at += s1;
    v0 = lw(at);
    {
        const bool bJump = (s0 == v0);
        v0 = s0 & 0x900;
        if (bJump) goto loc_8003EACC;
    }
    a0 = 0;
    if (v0 == 0) goto loc_8003EACC;
    a1 = sfx_pistol;
    S_StartSound();
    v0 = 9;
    goto loc_8003EEA0;
loc_8003EACC:
    v0 = s0 & 0xF000;
    if (v0 != 0) goto loc_8003EAF0;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7EF8;                                       // Result = gVBlanksUntilMenuMove (80077EF8)
    at += s1;
    sw(0, at);
    goto loc_8003EB9C;
loc_8003EAF0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x7EF8;                                       // Result = gVBlanksUntilMenuMove (80077EF8)
    a0 = s1 + v0;
    v1 = lw(a0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7FBC);                               // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v1 -= v0;
    sw(v1, a0);
    if (i32(v1) > 0) goto loc_8003EB9C;
    v0 = 0xF;                                           // Result = 0000000F
    sw(v0, a0);
    v0 = s0 & 0x4000;
    a0 = s1 + s5;
    if (v0 == 0) goto loc_8003EB50;
    v1 = lw(a0);
    v0 = *gOptionsMenuSize;
    v1++;
    v0--;
    v0 = (i32(v0) < i32(v1));
    sw(v1, a0);
    if (v0 == 0) goto loc_8003EB80;
    sw(0, a0);
    goto loc_8003EB80;
loc_8003EB50:
    v0 = s0 & 0x1000;
    v1 = s1 + s5;
    if (v0 == 0) goto loc_8003EB9C;
    v0 = lw(v1);
    v0--;
    sw(v0, v1);
    if (i32(v0) >= 0) goto loc_8003EB80;
    v0 = *gOptionsMenuSize;
    v0--;
    sw(v0, v1);
loc_8003EB80:
    v0 = *gCurPlayerIndex;
    a0 = 0;
    if (s2 != v0) goto loc_8003EB9C;
    a1 = sfx_pstop;
    S_StartSound();
loc_8003EB9C:
    v1 = lw(s4);
    v0 = v1 << 1;
    v0 += v1;
    v1 = *gpOptionsMenuItems;
    v0 <<= 2;
    v0 += v1;
    v1 = lw(v0);
    v0 = (v1 < 6);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_8003EE8C;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x1748;                                       // Result = JumpTable_O_Control[0] (80011748)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x8003EBE8: goto loc_8003EBE8;
        case 0x8003ECF8: goto loc_8003ECF8;
        case 0x8003EDC8: goto loc_8003EDC8;
        case 0x8003EE10: goto loc_8003EE10;
        case 0x8003EE4C: goto loc_8003EE4C;
        case 0x8003EE6C: goto loc_8003EE6C;
        default: jump_table_err(); break;
    }
loc_8003EBE8:
    v0 = *gCurPlayerIndex;
    {
        const bool bJump = (s2 != v0);
        v0 = s0 & 0x2000;
        if (bJump) goto loc_8003EE8C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x8000;
        if (bJump) goto loc_8003EC3C;
    }
    v0 = *gOptionsMusVol;
    v1 = v0 + 1;
    v0 = (i32(v1) < 0x65);
    *gOptionsMusVol = v1;
    {
        const bool bJump = (v0 != 0);
        v0 = v1 << 7;
        if (bJump) goto loc_8003EC74;
    }
    v0 = 0x64;                                          // Result = 00000064
    *gOptionsMusVol = v0;
    goto loc_8003ECB0;
loc_8003EC3C:
    if (v0 == 0) goto loc_8003EE8C;
    v0 = *gOptionsMusVol;
    v1 = v0 - 1;
    *gOptionsMusVol = v1;
    v0 = v1 << 7;
    if (i32(v1) >= 0) goto loc_8003EC74;
    *gOptionsMusVol = 0;
    goto loc_8003ECB0;
loc_8003EC74:
    v0 -= v1;
    mult(v0, s3);
    v0 = u32(i32(v0) >> 31);
    a0 = hi;
    a0 = u32(i32(a0) >> 5);
    a0 -= v0;
    S_SetMusicVolume();
    v0 = *gOptionsMusVol;
    v0 &= 1;
    a0 = 0;
    if (v0 == 0) goto loc_8003ECB0;
    a1 = sfx_stnmov;
    S_StartSound();
loc_8003ECB0:
    v0 = *gOptionsMusVol;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v1 += v0;
    v1 <<= 8;
    v1 -= v0;
    mult(v1, s3);
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 = u32(i32(v0) >> 5);
    v0 -= v1;
    *gCdMusicVol = v0;
    s4 -= 4;
    goto loc_8003EE90;
loc_8003ECF8:
    v0 = *gCurPlayerIndex;
    {
        const bool bJump = (s2 != v0);
        v0 = s0 & 0x2000;
        if (bJump) goto loc_8003EE8C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = s0 & 0x8000;
        if (bJump) goto loc_8003ED4C;
    }
    v0 = *gOptionsSndVol;
    v1 = v0 + 1;
    v0 = (i32(v1) < 0x65);
    *gOptionsSndVol = v1;
    {
        const bool bJump = (v0 != 0);
        v0 = v1 << 7;
        if (bJump) goto loc_8003ED84;
    }
    v0 = 0x64;                                          // Result = 00000064
    *gOptionsSndVol = v0;
    s4 -= 4;
    goto loc_8003EE90;
loc_8003ED4C:
    if (v0 == 0) goto loc_8003EE8C;
    v0 = *gOptionsSndVol;
    v1 = v0 - 1;
    *gOptionsSndVol = v1;
    v0 = v1 << 7;
    if (i32(v1) >= 0) goto loc_8003ED84;
    *gOptionsSndVol = 0;
    s4 -= 4;
    goto loc_8003EE90;
loc_8003ED84:
    v0 -= v1;
    mult(v0, s3);
    v0 = u32(i32(v0) >> 31);
    a0 = hi;
    a0 = u32(i32(a0) >> 5);
    a0 -= v0;
    S_SetSfxVolume();
    v0 = *gOptionsSndVol;
    v0 &= 1;
    a0 = 0;
    if (v0 == 0) goto loc_8003EE8C;
    a1 = sfx_stnmov;
    S_StartSound();
    s4 -= 4;
    goto loc_8003EE90;
loc_8003EDC8:
    v0 = s0 & 0xF0;
    if (v0 == 0) goto loc_8003EE8C;
    
    v0 = MiniLoop(START_PasswordScreen, STOP_PasswordScreen, TIC_PasswordScreen, DRAW_PasswordScreen);
    v1 = 4;

    s4 -= 4;
    if (v0 != v1) goto loc_8003EE90;
    v0 = 4;                                             // Result = 00000004
    goto loc_8003EEA0;
loc_8003EE10:
    v0 = s0 & 0xF0;
    if (v0 == 0) goto loc_8003EE8C;
    
    s4 -= 4;
    v0 = MiniLoop(START_ControlsScreen, _thunk_STOP_ControlsScreen, _thunk_TIC_ControlsScreen, DRAW_ControlsScreen);
    s2--;

    goto loc_8003EE94;
loc_8003EE4C:
    v0 = s0 & 0xF0;
    s4 -= 4;
    if (v0 == 0) goto loc_8003EE90;
    a0 = 0;
    a1 = sfx_pistol;
    S_StartSound();
    v0 = 5;
    goto loc_8003EEA0;
loc_8003EE6C:
    v0 = s0 & 0xF0;
    s4 -= 4;
    if (v0 == 0) goto loc_8003EE90;
    a0 = 0;
    a1 = sfx_pistol;
    S_StartSound();
    v0 = 8;
    goto loc_8003EEA0;
loc_8003EE8C:
    s4 -= 4;
loc_8003EE90:
    s2--;
loc_8003EE94:
    s1 -= 4;
    if (i32(s2) >= 0) goto loc_8003EA6C;
    v0 = 0;
loc_8003EEA0:
    ra = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x30;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the options menu
//------------------------------------------------------------------------------------------------------------------------------------------
void O_Drawer() noexcept {
    // Increment the frame count for the texture cache and draw the background using the 'MARB01' sprite
    I_IncDrawnFrameCount();

    for (int16_t y = 0; y < 4; ++y) {
        for (int16_t x = 0; x < 4; ++x) {
            I_CacheAndDrawSprite(*gTex_MARB01, x * 64, y * 64, gPaletteClutIds[MAINPAL]);
        }
    }

    // Don't do any rendering if we are about to exit the menu
    if (*gGameAction == ga_nothing) {
        // Menu title
        I_DrawString(-1, 20, "Options");

        // Draw each menu item for the current options screen layout.
        // The available options will vary depending on game mode.
        const menuitem_t* pMenuItem = gpOptionsMenuItems->get();
        
        for (int32_t optIdx = 0; optIdx < *gOptionsMenuSize; ++optIdx, ++pMenuItem) {
            // Draw the option label
            I_DrawString(pMenuItem->x, pMenuItem->y, gOptionNames[pMenuItem->option]);

            // If the option has a slider associated with it, draw that too
            if (pMenuItem->option <= opt_sound) {
                // Draw the slider backing/container
                I_DrawSprite(
                    gTex_STATUS->texPageId,
                    gPaletteClutIds[UIPAL],
                    (int16_t) pMenuItem->x + 13,
                    (int16_t) pMenuItem->y + 20,
                    0,
                    184,
                    108,
                    11
                );

                // Draw the slider handle
                const int32_t sliderVal = (pMenuItem->option == opt_sound) ? *gOptionsSndVol : *gOptionsMusVol;

                I_DrawSprite(
                    gTex_STATUS->texPageId,
                    gPaletteClutIds[UIPAL],
                    (int16_t)(pMenuItem->x + 14 + sliderVal),
                    (int16_t)(pMenuItem->y + 20),
                    108,
                    184,
                    6,
                    11
                );
            }
        }

        // Draw the skull cursor
        const int32_t cursorPos = gCursorPos[*gCurPlayerIndex];
        const menuitem_t& menuItem = (*gpOptionsMenuItems)[cursorPos];

        I_DrawSprite(
            gTex_STATUS->texPageId,
            gPaletteClutIds[UIPAL],
            (int16_t) menuItem.x - 24,
            (int16_t) menuItem.y - 2,
            M_SKULL_TEX_U + (uint8_t)(*gCursorFrame) * M_SKULL_W,
            M_SKULL_TEX_V,
            M_SKULL_W,
            M_SKULL_H
        );
    }

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}
