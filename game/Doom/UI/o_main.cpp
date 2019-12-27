#include "o_main.h"

#include "ct_main.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "PsxVm/PsxVm.h"
#include "pw_main.h"

void O_Init() noexcept {
    sp -= 0x18;
    a0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x10);
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    a1 = 0;                                             // Result = 00000000
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7EF8;                                       // Result = gVBlanksUntilMenuMove (80077EF8)
    v1 = 0x80080000;                                    // Result = 80080000
    v1 -= 0x8000;                                       // Result = gCursorPos (80078000)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7E28);                                 // Store to: gCursorFrame (800781D8)
loc_8003E940:
    sw(0, v1);
    sw(0, a0);
    a0 += 4;
    a1++;
    v0 = (i32(a1) < 2);
    v1 += 4;
    if (v0 != 0) goto loc_8003E940;
    v0 = *gNetGame;
    if (v0 == 0) goto loc_8003E984;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x4BD0;                                       // Result = OptionsMenuEntries_NetGame[0] (80074BD0)
    sw(v0, gp + 0xCE0);                                 // Store to: gpCurOptionsMenuEntries (800782C0)
    v0 = 4;                                             // Result = 00000004
    goto loc_8003E9BC;
loc_8003E984:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EC0);                               // Load from: gbGamePaused (80077EC0)
    if (v0 == 0) goto loc_8003E9AC;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x4B88;                                       // Result = OptionsMenuEntries_InGame[0] (80074B88)
    sw(v0, gp + 0xCE0);                                 // Store to: gpCurOptionsMenuEntries (800782C0)
    v0 = 6;                                             // Result = 00000006
    goto loc_8003E9BC;
loc_8003E9AC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x4B4C;                                       // Result = OptionsMenuEntries_MainMenu[0] (80074B4C)
    sw(v0, gp + 0xCE0);                                 // Store to: gpCurOptionsMenuEntries (800782C0)
    v0 = 5;                                             // Result = 00000005
loc_8003E9BC:
    sw(v0, gp + 0xB38);                                 // Store to: gCurNumOptionsMenuEntries (80078118)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void O_Shutdown() noexcept {
    v1 = 1;                                             // Result = 00000001
    v0 = 0x80080000;                                    // Result = 80080000
    v0 -= 0x7FFC;                                       // Result = DefaultCursorPos (80078004)
loc_8003E9DC:
    sw(0, v0);
    v1--;
    v0 -= 4;
    if (i32(v1) >= 0) goto loc_8003E9DC;
    return;
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
    s5 = 0x80080000;                                    // Result = 80080000
    s5 -= 0x8000;                                       // Result = gCursorPos (80078000)
    s4 = s5 + 4;                                        // Result = DefaultCursorPos (80078004)
    s1 = 4;                                             // Result = 00000004
loc_8003EA6C:
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7F54;                                       // Result = gbPlayerInGame[0] (800780AC)
    at += s1;
    v0 = lw(at);
    if (v0 == 0) goto loc_8003EE8C;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gPlayerPadButtons[0] (80077F44)
    at += s1;
    s0 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gPlayerOldPadButtons[0] (80078214)
    at += s1;
    v0 = lw(at);
    {
        const bool bJump = (s0 == v0);
        v0 = s0 & 0x900;
        if (bJump) goto loc_8003EACC;
    }
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8003EACC;
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = 9;                                             // Result = 00000009
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
    v0 = lw(gp + 0xB38);                                // Load from: gCurNumOptionsMenuEntries (80078118)
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
    v0 = lw(gp + 0xB38);                                // Load from: gCurNumOptionsMenuEntries (80078118)
    v0--;
    sw(v0, v1);
loc_8003EB80:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a0 = 0;                                             // Result = 00000000
    if (s2 != v0) goto loc_8003EB9C;
    a1 = 0x12;                                          // Result = 00000012
    S_StartSound();
loc_8003EB9C:
    v1 = lw(s4);
    v0 = v1 << 1;
    v0 += v1;
    v1 = lw(gp + 0xCE0);                                // Load from: gpCurOptionsMenuEntries (800782C0)
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
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
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
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x75F4);                               // Load from: gOptionsMusVol (800775F4)
    v1 = v0 + 1;
    v0 = (i32(v1) < 0x65);
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x75F4);                                // Store to: gOptionsMusVol (800775F4)
    {
        const bool bJump = (v0 != 0);
        v0 = v1 << 7;
        if (bJump) goto loc_8003EC74;
    }
    v0 = 0x64;                                          // Result = 00000064
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x75F4);                                // Store to: gOptionsMusVol (800775F4)
    goto loc_8003ECB0;
loc_8003EC3C:
    if (v0 == 0) goto loc_8003EE8C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x75F4);                               // Load from: gOptionsMusVol (800775F4)
    v1 = v0 - 1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x75F4);                                // Store to: gOptionsMusVol (800775F4)
    v0 = v1 << 7;
    if (i32(v1) >= 0) goto loc_8003EC74;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x75F4);                                 // Store to: gOptionsMusVol (800775F4)
    goto loc_8003ECB0;
loc_8003EC74:
    v0 -= v1;
    mult(v0, s3);
    v0 = u32(i32(v0) >> 31);
    a0 = hi;
    a0 = u32(i32(a0) >> 5);
    a0 -= v0;
    S_SetMusicVolume();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x75F4);                               // Load from: gOptionsMusVol (800775F4)
    v0 &= 1;
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8003ECB0;
    a1 = 0x15;                                          // Result = 00000015
    S_StartSound();
loc_8003ECB0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x75F4);                               // Load from: gOptionsMusVol (800775F4)
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
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x75F8);                                // Store to: gCdMusicVol (800775F8)
    s4 -= 4;
    goto loc_8003EE90;
loc_8003ECF8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
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
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x75F0);                               // Load from: gOptionsSndVol (800775F0)
    v1 = v0 + 1;
    v0 = (i32(v1) < 0x65);
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x75F0);                                // Store to: gOptionsSndVol (800775F0)
    {
        const bool bJump = (v0 != 0);
        v0 = v1 << 7;
        if (bJump) goto loc_8003ED84;
    }
    v0 = 0x64;                                          // Result = 00000064
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x75F0);                                // Store to: gOptionsSndVol (800775F0)
    s4 -= 4;
    goto loc_8003EE90;
loc_8003ED4C:
    if (v0 == 0) goto loc_8003EE8C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x75F0);                               // Load from: gOptionsSndVol (800775F0)
    v1 = v0 - 1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x75F0);                                // Store to: gOptionsSndVol (800775F0)
    v0 = v1 << 7;
    if (i32(v1) >= 0) goto loc_8003ED84;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x75F0);                                 // Store to: gOptionsSndVol (800775F0)
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
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x75F0);                               // Load from: gOptionsSndVol (800775F0)
    v0 &= 1;
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8003EE8C;
    a1 = 0x15;                                          // Result = 00000015
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
    v0 = MiniLoop(START_ControlsScreen, STOP_ControlsScreen, TIC_ControlsScreen, DRAW_ControlsScreen);
    s2--;

    goto loc_8003EE94;
loc_8003EE4C:
    v0 = s0 & 0xF0;
    s4 -= 4;
    if (v0 == 0) goto loc_8003EE90;
    a0 = 0;                                             // Result = 00000000
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = 5;                                             // Result = 00000005
    goto loc_8003EEA0;
loc_8003EE6C:
    v0 = s0 & 0xF0;
    s4 -= 4;
    if (v0 == 0) goto loc_8003EE90;
    a0 = 0;                                             // Result = 00000000
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = 8;                                             // Result = 00000008
    goto loc_8003EEA0;
loc_8003EE8C:
    s4 -= 4;
loc_8003EE90:
    s2--;
loc_8003EE94:
    s1 -= 4;
    if (i32(s2) >= 0) goto loc_8003EA6C;
    v0 = 0;                                             // Result = 00000000
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

void O_Drawer() noexcept {
loc_8003EEC8:
    sp -= 0x50;
    sw(ra, sp + 0x48);
    sw(s7, sp + 0x44);
    sw(s6, sp + 0x40);
    sw(s5, sp + 0x3C);
    sw(s4, sp + 0x38);
    sw(s3, sp + 0x34);
    sw(s2, sp + 0x30);
    sw(s1, sp + 0x2C);
    sw(s0, sp + 0x28);
    I_IncDrawnFrameCount();
    s1 = 0;                                             // Result = 00000000
loc_8003EEF8:
    s0 = 0;                                             // Result = 00000000
loc_8003EEFC:
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7AB0;                                       // Result = gTexInfo_MARB01[0] (80097AB0)
    a1 = s0 << 6;
    a2 = s1 << 6;
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F7C);                               // Load from: gPaletteClutId_Main (800A9084)
    s0++;
    I_CacheAndDrawSprite();
    v0 = (i32(s0) < 4);
    if (v0 != 0) goto loc_8003EEFC;
    s1++;                                               // Result = 00000001
    v0 = (i32(s1) < 4);                                 // Result = 00000001
    if (v0 != 0) goto loc_8003EEF8;
    v0 = *gGameAction;
    a0 = -1;                                            // Result = FFFFFFFF
    if (v0 != ga_nothing) goto loc_8003F0F4;
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D54;                                       // Result = STR_Options[0] (80077D54)
    a1 = 0x14;                                          // Result = 00000014
    I_DrawString();
    v0 = lw(gp + 0xB38);                                // Load from: gCurNumOptionsMenuEntries (80078118)
    s1 = lw(gp + 0xCE0);                                // Load from: gpCurOptionsMenuEntries (800782C0)
    s2 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_8003F074;
    s7 = 0x800B0000;                                    // Result = 800B0000
    s7 -= 0x6B0E;                                       // Result = gTexInfo_STATUS[2] (800A94F2)
    s6 = 0x800B0000;                                    // Result = 800B0000
    s6 -= 0x6F5C;                                       // Result = gPaletteClutId_UI (800A90A4)
    s5 = 0xB8;                                          // Result = 000000B8
    s4 = 0x6C;                                          // Result = 0000006C
    s3 = 0xB;                                           // Result = 0000000B
    s0 = s1 + 8;
loc_8003EF8C:
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x4AEC;                                       // Result = STR_Menu_MusicVolume[0] (80074AEC)
    a0 = lw(s0 - 0x4);
    v0 = lw(s1);
    a1 = lw(s0);
    v0 <<= 4;
    a2 += v0;
    I_DrawString();
    v0 = lw(s1);
    v0 = (v0 < 2);
    if (v0 == 0) goto loc_8003F058;
    a2 = lw(s0 - 0x4);
    a3 = lw(s0);
    sw(0, sp + 0x10);
    sw(s5, sp + 0x14);
    sw(s4, sp + 0x18);
    sw(s3, sp + 0x1C);
    a0 = lhu(s7);                                       // Load from: gTexInfo_STATUS[2] (800A94F2)
    a1 = lh(s6);                                        // Load from: gPaletteClutId_UI (800A90A4)
    a2 += 0xD;
    a3 += 0x14;
    I_DrawSprite();
    v0 = lw(s1);
    {
        const bool bJump = (v0 != 0);
        v0 = 6;                                         // Result = 00000006
        if (bJump) goto loc_8003F020;
    }
    v1 = lw(s0 - 0x4);
    a3 = lw(s0);
    sw(s4, sp + 0x10);
    sw(s5, sp + 0x14);
    a0 = lhu(s7);                                       // Load from: gTexInfo_STATUS[2] (800A94F2)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x75F4);                               // Load from: gOptionsMusVol (800775F4)
    goto loc_8003F03C;
loc_8003F020:
    v1 = lw(s0 - 0x4);
    a3 = lw(s0);
    sw(s4, sp + 0x10);
    sw(s5, sp + 0x14);
    a0 = lhu(s7);                                       // Load from: gTexInfo_STATUS[2] (800A94F2)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x75F0);                               // Load from: gOptionsSndVol (800775F0)
loc_8003F03C:
    sw(v0, sp + 0x18);
    sw(s3, sp + 0x1C);
    a1 = lh(s6);                                        // Load from: gPaletteClutId_UI (800A90A4)
    a2 += v1;
    a2 += 0xE;
    a3 += 0x14;
    I_DrawSprite();
loc_8003F058:
    s2++;
    s0 += 0xC;
    v0 = lw(gp + 0xB38);                                // Load from: gCurNumOptionsMenuEntries (80078118)
    v0 = (i32(s2) < i32(v0));
    s1 += 0xC;
    if (v0 != 0) goto loc_8003EF8C;
loc_8003F074:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lhu(a0 - 0x6B0E);                              // Load from: gTexInfo_STATUS[2] (800A94F2)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lh(a1 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x8000;                                       // Result = gCursorPos (80078000)
    at += v0;
    v1 = lw(at);
    a2 = lw(gp + 0xCE0);                                // Load from: gpCurOptionsMenuEntries (800782C0)
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 += a2;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7E28);                               // Load from: gCursorFrame (800781D8)
    a2 = lw(v0 + 0x4);
    a3 = lw(v0 + 0x8);
    v0 = 0xC0;                                          // Result = 000000C0
    sw(v0, sp + 0x14);
    v0 = 0x10;                                          // Result = 00000010
    sw(v0, sp + 0x18);
    v0 = 0x12;                                          // Result = 00000012
    sw(v0, sp + 0x1C);
    v1 <<= 4;
    v1 += 0x84;
    a2 -= 0x18;
    a3 -= 2;
    sw(v1, sp + 0x10);
    I_DrawSprite();
loc_8003F0F4:
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x48);
    s7 = lw(sp + 0x44);
    s6 = lw(sp + 0x40);
    s5 = lw(sp + 0x3C);
    s4 = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x50;
    return;
}
