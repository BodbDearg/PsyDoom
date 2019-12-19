#include "f_finale.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "PsxVm/PsxVm.h"
#include "Wess/psxcd.h"

void F1_Start() noexcept {
    sp -= 0x28;
    sw(s0, sp + 0x20);
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7A90;                                       // Result = gTexInfo_LOADING[0] (80097A90)
    a0 = s0;                                            // Result = gTexInfo_LOADING[0] (80097A90)
    a1 = 0x5F;                                          // Result = 0000005F
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    sw(ra, sp + 0x24);
    a2 = 0x6D;                                          // Result = 0000006D
    I_DrawPlaque();
    I_ResetTexCache();
    a0 = s0 - 0x80;                                     // Result = gTexInfo_BACK[0] (80097A10)
    I_CacheTex();
    a2 = 0;                                             // Result = 00000000
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x3E60);                               // Load from: CDTrackNum_Finale_Doom1 (80073E60)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75F8);                               // Load from: gCdMusicVol (800775F8)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x3E54);                               // Load from: CDTrackNum_Credits_Demo (80073E54)
    a3 = 0;                                             // Result = 00000000
    sw(0, gp + 0xB30);                                  // Store to: 80078110
    sw(0, gp + 0x978);                                  // Store to: 80077F58
    at = 0x800B0000;                                    // Result = 800B0000
    sb(0, at - 0x6FB8);                                 // Store to: 800A9048
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    sw(v0, sp + 0x10);
    sw(a1, sp + 0x14);
    psxcd_play_at_andloop();
loc_8003D750:
    psxcd_elapsed_sectors();
    if (v0 == 0) goto loc_8003D750;
    ra = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x28;
    return;
}

void F1_Stop() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7EC0);                                 // Store to: gbGamePaused (80077EC0)
    psxcd_stop();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void F1_Ticker() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    *gGameAction = ga_nothing;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gPlayerPadButtons[0] (80077F44)
    at += v0;
    s0 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gPlayerOldPadButtons[0] (80078214)
    at += v0;
    s1 = lw(at);
    P_CheckCheats();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EC0);                               // Load from: gbGamePaused (80077EC0)
    if (v0 == 0) goto loc_8003D80C;
    v0 = *gGameAction;
    goto loc_8003D8D8;
loc_8003D80C:
    a0 = lw(gp + 0xB30);                                // Load from: 80078110
    v0 = (i32(a0) < 0xB);
    if (v0 == 0) goto loc_8003D8C4;
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 1;
        if (bJump) goto loc_8003D8D4;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003D8D8;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x483C;                                       // Result = STR_Doom1_WinText_1[0] (8007483C)
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    v0 += a0;
    a2 = lw(gp + 0x978);                                // Load from: 80077F58
    a1 = v0 + v1;
    v0 = a1 + a2;
    v0 = lbu(v0);
    {
        const bool bJump = (v0 != 0);
        v0 = a0 + 1;
        if (bJump) goto loc_8003D88C;
    }
    sw(0, gp + 0x978);                                  // Store to: 80077F58
    sw(v0, gp + 0xB30);                                 // Store to: 80078110
    goto loc_8003D89C;
loc_8003D88C:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x6FB8;                                       // Result = 800A9048
    D_strncpy();
loc_8003D89C:
    v1 = lw(gp + 0x978);                                // Load from: 80077F58
    v0 = v1 + 1;
    sw(v0, gp + 0x978);                                 // Store to: 80077F58
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6FB8;                                       // Result = 800A9048
    at += v1;
    sb(0, at);
    v0 = 0;                                             // Result = 00000000
    goto loc_8003D8D8;
loc_8003D8C4:
    v0 = s0 & 0xF0;
    if (s0 == s1) goto loc_8003D8D4;
    {
        const bool bJump = (v0 != 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_8003D8D8;
    }
loc_8003D8D4:
    v0 = 0;                                             // Result = 00000000
loc_8003D8D8:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void F1_Drawer() noexcept {
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    I_IncDrawnFrameCount();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7A10;                                       // Result = gTexInfo_BACK[0] (80097A10)
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F7C);                               // Load from: gPaletteClutId_Main (800A9084)
    s1 = 0x2D;                                          // Result = 0000002D
    I_CacheAndDrawSprite();
    v0 = lw(gp + 0xB30);                                // Load from: 80078110
    s0 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_8003D968;
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x483C;                                       // Result = STR_Doom1_WinText_1[0] (8007483C)
loc_8003D940:
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = s1;
    a2 = s2;
    I_DrawString();
    s1 += 0xE;
    v0 = lw(gp + 0xB30);                                // Load from: 80078110
    s0++;
    v0 = (i32(s0) < i32(v0));
    s2 += 0x19;
    if (v0 != 0) goto loc_8003D940;
loc_8003D968:
    a0 = -1;                                            // Result = FFFFFFFF
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 -= 0x6FB8;                                       // Result = 800A9048
    a1 = s1;
    I_DrawString();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EC0);                               // Load from: gbGamePaused (80077EC0)
    if (v0 == 0) goto loc_8003D998;
    I_DrawPausedOverlay();
loc_8003D998:
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void F2_Start() noexcept {
    sp -= 0x68;
    sw(s0, sp + 0x60);
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7A90;                                       // Result = gTexInfo_LOADING[0] (80097A90)
    a0 = s0;                                            // Result = gTexInfo_LOADING[0] (80097A90)
    a1 = 0x5F;                                          // Result = 0000005F
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    sw(ra, sp + 0x64);
    a2 = 0x6D;                                          // Result = 0000006D
    I_DrawPlaque();
    I_ResetTexCache();
    a0 = s0 + 0x120;                                    // Result = gTexInfo_DEMON[0] (80097BB0)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7D44;                                       // Result = STR_LumpName_DEMON[0] (80077D44)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    a0 = 4;                                             // Result = 00000004
    P_LoadBlocks();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x4A68);                               // Load from: CastInfo_1_ZombieMan[1] (80074A68)
    at = 0x800B0000;                                    // Result = 800B0000
    sb(0, at - 0x6FB8);                                 // Store to: 800A9048
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1FB8;                                       // Result = MObjInfo_MT_PLAYER[3] (8005E048)
    at += v0;
    v0 = lw(at);
    sw(0, gp + 0xB8C);                                  // Store to: 8007816C
    sw(0, gp + 0xB30);                                  // Store to: 80078110
    sw(0, gp + 0x978);                                  // Store to: 80077F58
    sw(0, gp + 0xCA8);                                  // Store to: 80078288
    sw(0, gp + 0x998);                                  // Store to: 80077F78
    sw(0, gp + 0xAA8);                                  // Store to: 80078088
    sw(0, gp + 0xB88);                                  // Store to: 80078168
    v1 = v0 << 3;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v1 += v0;
    a1 = lw(v1 + 0x8);
    v0 = 0x2D;                                          // Result = 0000002D
    sw(v0, gp + 0x930);                                 // Store to: 80077F10
    sw(v1, gp + 0x9C8);                                 // Store to: 80077FA8
    sw(a1, gp + 0x8F0);                                 // Store to: 80077ED0
    a0 = 0x3C;                                          // Result = 0000003C
    S_LoadSoundAndMusic();
    a2 = 0;                                             // Result = 00000000
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x3E64);                               // Load from: CDTrackNum_Finale_Doom2 (80073E64)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75F8);                               // Load from: gCdMusicVol (800775F8)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x3E54);                               // Load from: CDTrackNum_Credits_Demo (80073E54)
    a3 = 0;                                             // Result = 00000000
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    sw(v0, sp + 0x10);
    sw(a1, sp + 0x14);
    psxcd_play_at_andloop();
loc_8003DACC:
    psxcd_elapsed_sectors();
    if (v0 == 0) goto loc_8003DACC;
    ra = lw(sp + 0x64);
    s0 = lw(sp + 0x60);
    sp += 0x68;
    return;
}

void F2_Stop() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7EC0);                                 // Store to: gbGamePaused (80077EC0)
    psxcd_stop();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void F2_Ticker() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    *gGameAction = ga_nothing;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gPlayerPadButtons[0] (80077F44)
    at += v0;
    s0 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gPlayerOldPadButtons[0] (80078214)
    at += v0;
    s1 = lw(at);
    P_CheckCheats();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EC0);                               // Load from: gbGamePaused (80077EC0)
    s2 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8003DB8C;
    v0 = *gGameAction;
    goto loc_8003E30C;
loc_8003DB8C:
    v1 = lw(gp + 0xB8C);                                // Load from: 8007816C
    v0 = (i32(v1) < 2);
    if (v1 == s2) goto loc_8003DC80;
    if (v0 == 0) goto loc_8003DBB4;
    v0 = 0;                                             // Result = 00000000
    if (v1 == 0) goto loc_8003DBC8;
    goto loc_8003E30C;
loc_8003DBB4:
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (v1 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003DCA8;
    }
    goto loc_8003E30C;
loc_8003DBC8:
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 & 1;
        if (bJump) goto loc_8003E308;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003E30C;
    }
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x4950;                                       // Result = STR_Doom2_WinText_1[0] (80074950)
    a0 = lw(gp + 0xB30);                                // Load from: 80078110
    a2 = lw(gp + 0x978);                                // Load from: 80077F58
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    v0 += a0;
    a1 = v0 + v1;
    v0 = a1 + a2;
    v0 = lbu(v0);
    {
        const bool bJump = (v0 != 0);
        v0 = a0 + 1;
        if (bJump) goto loc_8003DC48;
    }
    sw(v0, gp + 0xB30);                                 // Store to: 80078110
    v0 = (i32(v0) < 0xB);
    sw(0, gp + 0x978);                                  // Store to: 80077F58
    if (v0 != 0) goto loc_8003DC58;
    sw(s2, gp + 0xB8C);                                 // Store to: 8007816C
    goto loc_8003DC58;
loc_8003DC48:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x6FB8;                                       // Result = 800A9048
    D_strncpy();
loc_8003DC58:
    v1 = lw(gp + 0x978);                                // Load from: 80077F58
    v0 = v1 + 1;
    sw(v0, gp + 0x978);                                 // Store to: 80077F58
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6FB8;                                       // Result = 800A9048
    at += v1;
    sb(0, at);
    v0 = 0;                                             // Result = 00000000
    goto loc_8003E30C;
loc_8003DC80:
    v0 = lw(gp + 0x930);                                // Load from: 80077F10
    v0--;
    sw(v0, gp + 0x930);                                 // Store to: 80077F10
    v0 = (i32(v0) < -0xC8);
    {
        const bool bJump = (v0 == 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8003E308;
    }
    sw(v0, gp + 0xB8C);                                 // Store to: 8007816C
    v0 = 0;                                             // Result = 00000000
    goto loc_8003E30C;
loc_8003DCA8:
    v0 = lw(gp + 0x998);                                // Load from: 80077F78
    if (v0 != 0) goto loc_8003DDA0;
    v0 = s0 & 0xF0;
    if (s0 == s1) goto loc_8003DDA0;
    if (v0 == 0) goto loc_8003DDA0;
    a0 = 0;                                             // Result = 00000000
    a1 = 8;                                             // Result = 00000008
    S_StartSound();
    v0 = lw(gp + 0xCA8);                                // Load from: 80078288
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F8C;                                       // Result = MObjInfo_MT_PLAYER[E] (8005E074)
    at += v0;
    a1 = lw(at);
    if (a1 == 0) goto loc_8003DD2C;
    a0 = 0;                                             // Result = 00000000
    S_StartSound();
loc_8003DD2C:
    v0 = lw(gp + 0xCA8);                                // Load from: 80078288
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F94;                                       // Result = MObjInfo_MT_PLAYER[C] (8005E06C)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    v1 = lw(v0 + 0x8);
    sw(s2, gp + 0x998);                                 // Store to: 80077F78
    sw(0, gp + 0xAA8);                                  // Store to: 80078088
    sw(v0, gp + 0x9C8);                                 // Store to: 80077FA8
    sw(v1, gp + 0x8F0);                                 // Store to: 80077ED0
loc_8003DDA0:
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003E30C;
    }
    v0 = lw(gp + 0x998);                                // Load from: 80077F78
    if (v0 == 0) goto loc_8003DEC0;
    v0 = lw(gp + 0x9C8);                                // Load from: 80077FA8
    v0 = lw(v0 + 0x10);
    if (v0 != 0) goto loc_8003DEC0;
    v0 = lw(gp + 0xCA8);                                // Load from: 80078288
    v0++;
    v1 = v0 << 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A64;                                       // Result = CastInfo_1_ZombieMan[0] (80074A64)
    at += v1;
    v1 = lw(at);
    sw(0, gp + 0x998);                                  // Store to: 80077F78
    sw(v0, gp + 0xCA8);                                 // Store to: 80078288
    if (v1 != 0) goto loc_8003DE1C;
    sw(0, gp + 0xCA8);                                  // Store to: 80078288
loc_8003DE1C:
    a0 = lw(gp + 0xCA8);                                // Load from: 80078288
    a0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += a0;
    v1 = lw(at);
    a1 = 0x80060000;                                    // Result = 80060000
    a1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    sw(0, gp + 0xAA8);                                  // Store to: 80078088
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1FB8;                                       // Result = MObjInfo_MT_PLAYER[3] (8005E048)
    at += v0;
    v0 = lw(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += a0;
    a0 = lw(at);
    v1 = v0 << 3;
    v1 -= v0;
    v1 <<= 2;
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 2;
    v0 -= a0;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1FB4;                                       // Result = MObjInfo_MT_PLAYER[4] (8005E04C)
    at += v0;
    v0 = lw(at);
    v1 += a1;
    sw(v1, gp + 0x9C8);                                 // Store to: 80077FA8
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8003E098;
    a1 = v0;
    goto loc_8003E090;
loc_8003DEC0:
    v0 = lw(gp + 0x8F0);                                // Load from: 80077ED0
    v0--;
    sw(v0, gp + 0x8F0);                                 // Store to: 80077ED0
    {
        const bool bJump = (i32(v0) > 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003E30C;
    }
    v0 = lw(gp + 0x9C8);                                // Load from: 80077FA8
    v1 = lw(v0 + 0x10);
    v0 = 0x167;                                         // Result = 00000167
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0x168);
        if (bJump) goto loc_8003E054;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x10A;                                     // Result = 0000010A
        if (bJump) goto loc_8003DF94;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0x10B);
        if (bJump) goto loc_8003E04C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0xDB;                                      // Result = 000000DB
        if (bJump) goto loc_8003DF4C;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0xDC);
        if (bJump) goto loc_8003E03C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0xA0;                                      // Result = 000000A0
        if (bJump) goto loc_8003DF30;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 0xBE;                                      // Result = 000000BE
        if (bJump) goto loc_8003E034;
    }
    a1 = 7;                                             // Result = 00000007
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003DF30:
    v0 = 0x106;                                         // Result = 00000106
    {
        const bool bJump = (v1 == v0);
        v0 = 0x108;                                     // Result = 00000108
        if (bJump) goto loc_8003E044;
    }
    a1 = 0x50;                                          // Result = 00000050
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003DF4C:
    v0 = 0x12F;                                         // Result = 0000012F
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0x130);
        if (bJump) goto loc_8003E064;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x129;                                     // Result = 00000129
        if (bJump) goto loc_8003DF78;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 0x12C;                                     // Result = 0000012C
        if (bJump) goto loc_8003E064;
    }
    a1 = 0x4A;                                          // Result = 0000004A
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003DF78:
    v0 = (v1 < 0x14C);
    {
        const bool bJump = (v0 == 0);
        v0 = (v1 < 0x149);
        if (bJump) goto loc_8003E084;
    }
    a1 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_8003E088;
    a1 = 7;                                             // Result = 00000007
    goto loc_8003E088;
loc_8003DF94:
    v0 = (v1 < 0x1E7);
    {
        const bool bJump = (v0 == 0);
        v0 = (v1 < 0x1E5);
        if (bJump) goto loc_8003DFEC;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x1A5;                                     // Result = 000001A5
        if (bJump) goto loc_8003E06C;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0x1A6);
        if (bJump) goto loc_8003E064;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x182;                                     // Result = 00000182
        if (bJump) goto loc_8003DFD0;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 0x18F;                                     // Result = 0000018F
        if (bJump) goto loc_8003E05C;
    }
    a1 = 0x4A;                                          // Result = 0000004A
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003DFD0:
    v0 = 0x1BB;                                         // Result = 000001BB
    {
        const bool bJump = (v1 == v0);
        v0 = 0x1CB;                                     // Result = 000001CB
        if (bJump) goto loc_8003E064;
    }
    a1 = 0x3B;                                          // Result = 0000003B
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003DFEC:
    v0 = 0x225;                                         // Result = 00000225
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 0x226);
        if (bJump) goto loc_8003E07C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x205;                                     // Result = 00000205
        if (bJump) goto loc_8003E018;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 0x223;                                     // Result = 00000223
        if (bJump) goto loc_8003E074;
    }
    a1 = 0xF;                                           // Result = 0000000F
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003E018:
    v0 = 0x227;                                         // Result = 00000227
    {
        const bool bJump = (v1 == v0);
        v0 = 0x23C;                                     // Result = 0000023C
        if (bJump) goto loc_8003E07C;
    }
    a1 = 0x3B;                                          // Result = 0000003B
    if (v1 == v0) goto loc_8003E088;
    a1 = 0;                                             // Result = 00000000
    goto loc_8003E088;
loc_8003E034:
    a1 = 0x1D;                                          // Result = 0000001D
    goto loc_8003E088;
loc_8003E03C:
    a1 = 8;                                             // Result = 00000008
    goto loc_8003E088;
loc_8003E044:
    a1 = 0x4F;                                          // Result = 0000004F
    goto loc_8003E088;
loc_8003E04C:
    a1 = 0x4E;                                          // Result = 0000004E
    goto loc_8003E088;
loc_8003E054:
    a1 = 0x2E;                                          // Result = 0000002E
    goto loc_8003E088;
loc_8003E05C:
    a1 = 0x35;                                          // Result = 00000035
    goto loc_8003E088;
loc_8003E064:
    a1 = 0x4A;                                          // Result = 0000004A
    goto loc_8003E088;
loc_8003E06C:
    a1 = 7;                                             // Result = 00000007
    goto loc_8003E088;
loc_8003E074:
    a1 = 9;                                             // Result = 00000009
    goto loc_8003E088;
loc_8003E07C:
    a1 = 0xF;                                           // Result = 0000000F
    goto loc_8003E088;
loc_8003E084:
    a1 = 0;                                             // Result = 00000000
loc_8003E088:
    a0 = 0;                                             // Result = 00000000
    if (a1 == 0) goto loc_8003E098;
loc_8003E090:
    S_StartSound();
loc_8003E098:
    v0 = lw(gp + 0x9C8);                                // Load from: 80077FA8
    v1 = lw(gp + 0xAA8);                                // Load from: 80078088
    a0 = lw(v0 + 0x10);
    v1++;
    sw(v1, gp + 0xAA8);                                 // Store to: 80078088
    v0 = a0 << 3;
    v0 -= a0;
    v0 <<= 2;
    a0 = 0x80060000;                                    // Result = 80060000
    a0 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += a0;
    sw(v0, gp + 0x9C8);                                 // Store to: 80077FA8
    v0 = 0xC;                                           // Result = 0000000C
    if (v1 != v0) goto loc_8003E25C;
    v0 = lw(gp + 0xB88);                                // Load from: 80078168
    if (v0 == 0) goto loc_8003E130;
    v0 = lw(gp + 0xCA8);                                // Load from: 80078288
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F9C;                                       // Result = MObjInfo_MT_PLAYER[A] (8005E064)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 3;
    goto loc_8003E17C;
loc_8003E130:
    v0 = lw(gp + 0xCA8);                                // Load from: 80078288
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F98;                                       // Result = MObjInfo_MT_PLAYER[B] (8005E068)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 3;
loc_8003E17C:
    v0 -= v1;
    v0 <<= 2;
    v0 += a0;
    sw(v0, gp + 0x9C8);                                 // Store to: 80077FA8
    v0 = lw(gp + 0xB88);                                // Load from: 80078168
    a0 = lw(gp + 0x9C8);                                // Load from: 80077FA8
    v1 = v0 ^ 1;
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    sw(v1, gp + 0xB88);                                 // Store to: 80078168
    if (a0 != v0) goto loc_8003E25C;
    if (v1 == 0) goto loc_8003E200;
    v0 = lw(gp + 0xCA8);                                // Load from: 80078288
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F9C;                                       // Result = MObjInfo_MT_PLAYER[A] (8005E064)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 3;
    goto loc_8003E24C;
loc_8003E200:
    v0 = lw(gp + 0xCA8);                                // Load from: 80078288
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1F98;                                       // Result = MObjInfo_MT_PLAYER[B] (8005E068)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 3;
loc_8003E24C:
    v0 -= v1;
    v0 <<= 2;
    v0 += a0;
    sw(v0, gp + 0x9C8);                                 // Store to: 80077FA8
loc_8003E25C:
    v1 = lw(gp + 0xAA8);                                // Load from: 80078088
    v0 = 0x18;                                          // Result = 00000018
    if (v1 == v0) goto loc_8003E280;
    v1 = lw(gp + 0x9C8);                                // Load from: 80077FA8
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x619C;                                       // Result = State_S_PLAY[0] (80059E64)
    if (v1 != v0) goto loc_8003E2E4;
loc_8003E280:
    v0 = lw(gp + 0xCA8);                                // Load from: 80078288
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A68;                                       // Result = CastInfo_1_ZombieMan[1] (80074A68)
    at += v0;
    v1 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at -= 0x1FB8;                                       // Result = MObjInfo_MT_PLAYER[3] (8005E048)
    at += v0;
    v1 = lw(at);
    sw(0, gp + 0xAA8);                                  // Store to: 80078088
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, gp + 0x9C8);                                 // Store to: 80077FA8
loc_8003E2E4:
    v0 = lw(gp + 0x9C8);                                // Load from: 80077FA8
    v1 = lw(v0 + 0x8);
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v1, gp + 0x8F0);                                 // Store to: 80077ED0
    {
        const bool bJump = (v1 != v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8003E30C;
    }
    v0 = 0xF;                                           // Result = 0000000F
    sw(v0, gp + 0x8F0);                                 // Store to: 80077ED0
loc_8003E308:
    v0 = 0;                                             // Result = 00000000
loc_8003E30C:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void F2_Drawer() noexcept {
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    I_IncDrawnFrameCount();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7BB0;                                       // Result = gTexInfo_DEMON[0] (80097BB0)
    a1 = 0;                                             // Result = 00000000
    s2 = 0x800B0000;                                    // Result = 800B0000
    s2 -= 0x6F7C;                                       // Result = gPaletteClutId_Main (800A9084)
    a3 = lh(s2);                                        // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0;                                             // Result = 00000000
    I_CacheAndDrawSprite();
    v1 = lw(gp + 0xB8C);                                // Load from: 8007816C
    v0 = (i32(v1) < 2);
    if (i32(v1) < 0) goto loc_8003E8C8;
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8003E388;
    }
    if (v1 == v0) goto loc_8003E3DC;
    goto loc_8003E8C8;
loc_8003E388:
    v0 = lw(gp + 0xB30);                                // Load from: 80078110
    s0 = lw(gp + 0x930);                                // Load from: 80077F10
    s1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_8003E3C8;
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x4950;                                       // Result = STR_Doom2_WinText_1[0] (80074950)
loc_8003E3A0:
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = s0;
    a2 = s2;
    I_DrawString();
    s0 += 0xE;
    v0 = lw(gp + 0xB30);                                // Load from: 80078110
    s1++;
    v0 = (i32(s1) < i32(v0));
    s2 += 0x19;
    if (v0 != 0) goto loc_8003E3A0;
loc_8003E3C8:
    a0 = -1;                                            // Result = FFFFFFFF
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 -= 0x6FB8;                                       // Result = 800A9048
    a1 = s0;
    goto loc_8003E8C0;
loc_8003E3DC:
    v0 = lw(gp + 0x9C8);                                // Load from: 80077FA8
    a0 = lw(v0);
    v1 = lw(v0 + 0x4);
    a0 <<= 3;
    v1 &= 0x7FFF;
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 2;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x6BFC;                                       // Result = 80066BFC
    at += a0;
    v1 = lw(at);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FEC);                               // Load from: gFirstSpriteLumpNum (80078014)
    v0 += v1;
    v1 = lw(v0 + 0x4);
    s0 = lbu(v0 + 0x24);
    v1 -= a0;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7EC4);                               // Load from: gpSprites (80077EC4)
    v1 <<= 5;
    s1 = v1 + a0;
    a0 = s1;
    I_CacheTex();
    v0 = 9;                                             // Result = 00000009
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v1 = lhu(s2);                                       // Load from: gPaletteClutId_Main (800A9084)
    v0 = 0x2D;                                          // Result = 0000002D
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x20E);                                 // Store to: 1F80020E
    v0 = lhu(s1 + 0xA);
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x216);                                 // Store to: 1F800216
    v1 = lh(s1 + 0x2);
    v0 = 0xB4;                                          // Result = 000000B4
    a0 = v0 - v1;
    if (s0 != 0) goto loc_8003E500;
    v0 = lbu(s1 + 0x8);
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
    v0 = lbu(s1 + 0x8);
    v1 = lbu(s1 + 0x4);
    v0 += v1;
    v0--;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x214);                                 // Store to: 1F800214
    v0 = lbu(s1 + 0x8);
    v1 = lbu(s1 + 0x4);
    v0 += v1;
    v0--;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x224);                                 // Store to: 1F800224
    v0 = lbu(s1 + 0x8);
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x21C);                                 // Store to: 1F80021C
    v1 = lh(s1);
    v0 = 0x80;                                          // Result = 00000080
    v1 = v0 - v1;
    goto loc_8003E568;
loc_8003E4F4:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_8003E778;
loc_8003E500:
    v0 = lbu(s1 + 0x8);
    v1 = lbu(s1 + 0x4);
    v0 += v1;
    v0--;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
    v0 = lbu(s1 + 0x8);
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x214);                                 // Store to: 1F800214
    v0 = lbu(s1 + 0x8);
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x224);                                 // Store to: 1F800224
    v0 = lbu(s1 + 0x8);
    v1 = lbu(s1 + 0x4);
    v0 += v1;
    v0--;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x21C);                                 // Store to: 1F80021C
    v0 = lh(s1 + 0x4);
    v1 = lh(s1);
    v0 -= 0x80;
    v1 -= v0;
loc_8003E568:
    t2 = 0x1F800000;                                    // Result = 1F800000
    t2 += 0x204;                                        // Result = 1F800204
    t3 = 0xFF0000;                                      // Result = 00FF0000
    t3 |= 0xFFFF;                                       // Result = 00FFFFFF
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x208);                                 // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sh(a0, at + 0x20A);                                 // Store to: 1F80020A
    v0 = lhu(s1 + 0x4);
    s2 = s0 & t3;                                       // Result = 00086550
    at = 0x1F800000;                                    // Result = 1F800000
    sh(a0, at + 0x212);                                 // Store to: 1F800212
    v0 += v1;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    v0 = lhu(s1 + 0x4);
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    v0 += v1;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x220);                                 // Store to: 1F800220
    v0 = lhu(s1 + 0x6);
    t7 = 0x4000000;                                     // Result = 04000000
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x218);                                 // Store to: 1F800218
    v0 += a0;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x222);                                 // Store to: 1F800222
    v0 = lhu(s1 + 0x6);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += a0;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x21A);                                 // Store to: 1F80021A
    v0 = lbu(s1 + 0x9);
    t5 = 0x80000000;                                    // Result = 80000000
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20D);                                 // Store to: 1F80020D
    v0 = lbu(s1 + 0x9);
    t4 = -1;                                            // Result = FFFFFFFF
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x215);                                 // Store to: 1F800215
    v0 = lbu(s1 + 0x9);
    v1 = lbu(s1 + 0x6);
    t1 = t0 << 2;
    v0 += v1;
    v0--;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x225);                                 // Store to: 1F800225
    v0 = lbu(s1 + 0x9);
    v1 = lbu(s1 + 0x6);
    t6 = t1 + 4;
    v0 += v1;
    v0--;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x21D);                                 // Store to: 1F80021D
loc_8003E650:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8003E6B4;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_8003E4F4;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s2;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003E6B4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003E768;
    if (v1 == a0) goto loc_8003E650;
loc_8003E6D8:
    v0 = lw(gp + 0x754);                                // Load from: GPU_REG_GP1 (80077D34)
    v0 = lw(v0);
    v0 &= t7;
    if (v0 == 0) goto loc_8003E650;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t3;
    v0 |= t5;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_8003E744;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003E728:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x750);                                // Load from: GPU_REG_GP0 (80077D30)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003E728;
loc_8003E744:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003E650;
    goto loc_8003E6D8;
loc_8003E768:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t6;
loc_8003E778:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a3);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= a1;
    v1 |= v0;
    sw(v1, a3);
    sb(t0, a3 + 0x3);
    t0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a3 += 4;
    if (t0 == v0) goto loc_8003E7D8;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003E7C0:
    v0 = lw(t2);
    t2 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_8003E7C0;
loc_8003E7D8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_8003E88C;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003E804:
    v0 = lw(gp + 0x754);                                // Load from: GPU_REG_GP1 (80077D34)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8003E88C;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= a3;
    v0 |= t1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t0) goto loc_8003E870;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003E854:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x750);                                // Load from: GPU_REG_GP0 (80077D30)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003E854;
loc_8003E870:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003E804;
loc_8003E88C:
    a0 = -1;                                            // Result = FFFFFFFF
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1734;                                       // Result = STR_CastOfCharacters[0] (80011734)
    a1 = 0x14;                                          // Result = 00000014
    I_DrawString();
    v0 = lw(gp + 0xCA8);                                // Load from: 80078288
    a0 = -1;                                            // Result = FFFFFFFF
    v0 <<= 3;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x4A64;                                       // Result = CastInfo_1_ZombieMan[0] (80074A64)
    at += v0;
    a2 = lw(at);
    a1 = 0xD0;                                          // Result = 000000D0
loc_8003E8C0:
    I_DrawString();
loc_8003E8C8:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EC0);                               // Load from: gbGamePaused (80077EC0)
    if (v0 == 0) goto loc_8003E8E4;
    I_DrawPausedOverlay();
loc_8003E8E4:
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}
