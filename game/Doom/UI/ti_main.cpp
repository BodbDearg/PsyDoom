#include "ti_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/w_wad.h"
#include "Doom/d_main.h"
#include "Doom/Game/p_firesky.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_sky.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"

void START_Title() noexcept {
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    I_PurgeTexCache();
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x7C4C;                                       // Result = STR_LumpName_LOADING[0] (80077C4C)
    a0 = s1;                                            // Result = STR_LumpName_LOADING[0] (80077C4C)
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    _thunk_W_CacheLumpName();
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7A90;                                       // Result = gTexInfo_LOADING[0] (80097A90)
    a0 = s0;                                            // Result = gTexInfo_LOADING[0] (80097A90)
    a1 = s1;                                            // Result = STR_LumpName_LOADING[0] (80077C4C)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    a0 = s0;                                            // Result = gTexInfo_LOADING[0] (80097A90)
    a1 = 0x5F;                                          // Result = 0000005F
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    a2 = 0x6D;                                          // Result = 0000006D
    _thunk_I_DrawLoadingPlaque();
    a0 = 0;                                             // Result = 00000000
    S_LoadSoundAndMusic();
    s4 = 0x80070000;                                    // Result = 80070000
    s4 += 0x7C54;                                       // Result = STR_LumpName_MARB01[0] (80077C54)
    a0 = s4;                                            // Result = STR_LumpName_MARB01[0] (80077C54)
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    _thunk_W_CacheLumpName();
    s3 = 0x80070000;                                    // Result = 80070000
    s3 += 0x7C5C;                                       // Result = STR_LumpName_BUTTONS[0] (80077C5C)
    a0 = s3;                                            // Result = STR_LumpName_BUTTONS[0] (80077C5C)
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    _thunk_W_CacheLumpName();
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x7C64;                                       // Result = STR_LumpName_NETERR[0] (80077C64)
    a0 = s2;                                            // Result = STR_LumpName_NETERR[0] (80077C64)
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    _thunk_W_CacheLumpName();
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x7C6C;                                       // Result = STR_LumpName_PAUSE[0] (80077C6C)
    a0 = s1;                                            // Result = STR_LumpName_PAUSE[0] (80077C6C)
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    _thunk_W_CacheLumpName();
    a0 = s0 + 0x20;                                     // Result = gTexInfo_MARB01[0] (80097AB0)
    a1 = s4;                                            // Result = STR_LumpName_MARB01[0] (80077C54)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    a0 = s0 + 0x40;                                     // Result = gTexInfo_BUTTONS[0] (80097AD0)
    a1 = s3;                                            // Result = STR_LumpName_BUTTONS[0] (80077C5C)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    a0 = s0 + 0x60;                                     // Result = gTexInfo_NETERR[0] (80097AF0)
    a1 = s2;                                            // Result = STR_LumpName_NETERR[0] (80077C64)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    a0 = s0 - 0x20;                                     // Result = gTexInfo_PAUSE[0] (80097A70)
    a1 = s1;                                            // Result = STR_LumpName_PAUSE[0] (80077C6C)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    a0 = s0 - 0x60;                                     // Result = gTexInfo_TITLE[0] (80097A30)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7C74;                                       // Result = STR_LumpName_TITLE[0] (80077C74)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 += 0x7C7C;                                       // Result = STR_LumpName_SKY09[0] (80077C7C)
    R_TextureNumForName();
    a1 = 0x20;                                          // Result = 00000020
    v1 = *gpTextures;
    v0 <<= 5;
    v0 += v1;
    a0 = lh(v0 + 0x10);
    *gpSkyTexture = v0;
    a2 = 1;                                             // Result = 00000001
    _thunk_W_CacheLumpNum();
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lhu(v0 - 0x6F5E);                              // Load from: gPaletteClutId_Sky (800A90A2)
    a0 = *gpSkyTexture;
    *gPaletteClutId_CurMapSky = (uint16_t) v0;
    _thunk_I_CacheTex();
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x3E4C);                               // Load from: CDTrackNum_TitleScreen (80073E4C)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75F8);                               // Load from: gCdMusicVol (800775F8)
    v0 = 0xFA;                                          // Result = 000000FA
    sw(v0, gp + 0xBB0);                                 // Store to: gTitleScreenSpriteY (80078190)
    psxcd_play();
loc_80035234:
    psxcd_elapsed_sectors();
    if (v0 == 0) goto loc_80035234;
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void STOP_Title() noexcept {
    a0 = 0;
    a1 = sfx_barexp;
    S_StartSound();

    psxcd_stop();
}

void TIC_Title() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F44);                               // Load from: gTicButtons[0] (80077F44)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_800352B0;
    v0 = 9;                                             // Result = 00000009
    goto loc_800353FC;
loc_800352B0:
    v0 = *gCurPlayerIndex;
    v1 = lw(gp + 0x6A4);                                // Load from: gVBlanksUntilTitleSprMove (80077C84)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    v1 -= v0;
    sw(v1, gp + 0x6A4);                                 // Store to: gVBlanksUntilTitleSprMove (80077C84)
    v0 = 2;                                             // Result = 00000002
    if (i32(v1) > 0) goto loc_80035310;
    v1 = lw(gp + 0xBB0);                                // Load from: gTitleScreenSpriteY (80078190)
    sw(v0, gp + 0x6A4);                                 // Store to: gVBlanksUntilTitleSprMove (80077C84)
    v0 = v1 - 1;
    if (v1 == 0) goto loc_80035310;
    sw(v0, gp + 0xBB0);                                 // Store to: gTitleScreenSpriteY (80078190)
    if (v0 != 0) goto loc_80035310;
    v0 = *gTicCon;
    sw(v0, gp + 0x92C);                                 // Store to: gMenuTimeoutStartTicCon (80077F0C)
loc_80035310:
    v0 = *gCurPlayerIndex;
    v1 = lw(gp + 0x6A8);                                // Load from: gVBlanksUntilTitleFireMove (80077C88)
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    v1 -= v0;
    sw(v1, gp + 0x6A8);                                 // Store to: gVBlanksUntilTitleFireMove (80077C88)
    {
        const bool bJump = (i32(v1) > 0);
        v1 = 2;                                         // Result = 00000002
        if (bJump) goto loc_800353C8;
    }
    v0 = lw(gp + 0xBB0);                                // Load from: gTitleScreenSpriteY (80078190)
    sw(v1, gp + 0x6A8);                                 // Store to: gVBlanksUntilTitleFireMove (80077C88)
    v1 = (i32(v0) < 0x32);
    v0 ^= 1;
    v0 &= 1;
    v1 &= v0;
    if (v1 == 0) goto loc_800353B8;
    v0 = *gpSkyTexture;
    v0 = lh(v0 + 0x10);
    v1 = *gpLumpCache;
    v0 <<= 2;
    v0 += v1;
    a1 = lw(v0);
    v0 = lbu(a1 + 0x1FC8);
    a0 = v0 - 1;
    v1 = 0x3F;                                          // Result = 0000003F
    if (i32(a0) >= 0) goto loc_800353A4;
    a0 = 0;                                             // Result = 00000000
loc_800353A4:
    v0 = a1 + 0x2007;
loc_800353A8:
    sb(a0, v0);
    v1--;
    v0--;
    if (i32(v1) >= 0) goto loc_800353A8;
loc_800353B8:
    a0 = *gpSkyTexture;
    P_UpdateFireSky();
loc_800353C8:
    v1 = lw(gp + 0xBB0);                                // Load from: gTitleScreenSpriteY (80078190)
    v0 = 0;                                             // Result = 00000000
    if (v1 != 0) goto loc_800353FC;
    v0 = *gTicCon;
    v1 = lw(gp + 0x92C);                                // Load from: gMenuTimeoutStartTicCon (80077F0C)
    v0 -= v1;
    v0 = (i32(v0) < 0x708);
    v0 ^= 1;
    v0 = -v0;
    v0 &= 7;
loc_800353FC:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void DRAW_Title() noexcept {
    sp -= 0x48;
    sw(ra, sp + 0x44);
    sw(s0, sp + 0x40);
    I_IncDrawnFrameCount();
    t0 = 9;                                             // Result = 00000009
    t1 = sp + 0x14;
    t3 = 0x24;                                          // Result = 00000024
    t4 = 0x28;                                          // Result = 00000028
    t2 = 0xFF0000;                                      // Result = 00FF0000
    t2 |= 0xFFFF;                                       // Result = 00FFFFFF
    t8 = 0x80080000;                                    // Result = 80080000
    t8 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s0 = t8 & t2;                                       // Result = 00086550
    t7 = 0x4000000;                                     // Result = 04000000
    t6 = 0x80000000;                                    // Result = 80000000
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 = 0xFF;                                          // Result = 000000FF
    sb(v1, sp + 0x24);
    sb(v1, sp + 0x34);
    v1 = lhu(gp + 0xBB0);                               // Load from: gTitleScreenSpriteY (80078190)
    v0 = 9;                                             // Result = 00000009
    sb(v0, sp + 0x13);
    v0 = 0x2C;                                          // Result = 0000002C
    sb(v0, sp + 0x17);
    v0 = 0xFF;                                          // Result = 000000FF
    sh(v0, sp + 0x20);
    sh(v0, sp + 0x30);
    v0 = 0xEF;                                          // Result = 000000EF
    sb(v0, sp + 0x2D);
    sb(v0, sp + 0x35);
    v0 = 0x80;                                          // Result = 00000080
    sb(v0, sp + 0x14);
    sb(v0, sp + 0x15);
    sb(v0, sp + 0x16);
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lhu(v0 + 0x7A3A);                              // Load from: gTexInfo_TITLE[2] (80097A3A)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lhu(a0 - 0x6F5A);                              // Load from: gPaletteClutId_Title (800A90A6)
    t5 = -1;                                            // Result = FFFFFFFF
    sh(0, sp + 0x18);
    sh(0, sp + 0x28);
    sb(0, sp + 0x1C);
    sb(0, sp + 0x1D);
    sb(0, sp + 0x25);
    sb(0, sp + 0x2C);
    sh(v1, sp + 0x1A);
    sh(v1, sp + 0x22);
    v1 += 0xEF;
    sh(v1, sp + 0x2A);
    sh(v1, sp + 0x32);
    sh(v0, sp + 0x26);
    sh(a0, sp + 0x1E);
loc_800354E0:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t3 + a0;
        if (bJump) goto loc_80035548;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t4 + a0;
        if (bJump) goto loc_8003560C;
    }
    v0 = lw(a3);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(t8, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s0;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80035548:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t3 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_800355FC;
    if (v1 == a0) goto loc_800354E0;
loc_8003556C:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t7;
    if (v0 == 0) goto loc_800354E0;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t2;
    v0 |= t6;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800355D8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800355BC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800355BC;
loc_800355D8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800354E0;
    goto loc_8003556C;
loc_800355FC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t4;
loc_8003560C:
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
    t0--;                                               // Result = 00000008
    v0 = -1;                                            // Result = FFFFFFFF
    a3 += 4;
    if (t0 == v0) goto loc_8003566C;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80035654:
    v0 = lw(t1);
    t1 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_80035654;
loc_8003566C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80035724;
    t2 = 0x4000000;                                     // Result = 04000000
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003569C:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_80035724;
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
    if (a1 == t0) goto loc_80035708;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800356EC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800356EC;
loc_80035708:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003569C;
loc_80035724:
    a1 = *gpSkyTexture;
    v1 = lw(a1 + 0x1C);
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != v0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_800357C4;
    }
    v1 = lbu(a1 + 0x8);
    v0 = lhu(a1 + 0xA);
    v1 >>= 1;
    v0 &= 0xF;
    v0 <<= 6;
    v1 += v0;
    sh(v1, sp + 0x38);
    v1 = lhu(a1 + 0xA);
    a0 = lbu(a1 + 0x9);
    v0 = 0x20;                                          // Result = 00000020
    sh(v0, sp + 0x3C);
    v0 = 0x80;                                          // Result = 00000080
    sh(v0, sp + 0x3E);
    v1 &= 0x10;
    v1 <<= 4;
    a0 += v1;
    sh(a0, sp + 0x3A);
    v0 = lh(a1 + 0x10);
    v1 = *gpLumpCache;
    v0 <<= 2;
    v0 += v1;
    a1 = lw(v0);
    a0 = sp + 0x38;
    a1 += 8;
    _thunk_LIBGPU_LoadImage();
    v1 = *gpSkyTexture;
    v0 = *gNumFramesDrawn;
    sw(v0, v1 + 0x1C);
    v0 = 9;                                             // Result = 00000009
loc_800357C4:
    sb(v0, sp + 0x13);
    v0 = 0x2C;                                          // Result = 0000002C
    sb(v0, sp + 0x17);
    v0 = 0x74;                                          // Result = 00000074
    a0 = *gpSkyTexture;
    v1 = 0x3F;                                          // Result = 0000003F
    sh(v0, sp + 0x1A);
    sh(v0, sp + 0x22);
    v0 = 0xF3;                                          // Result = 000000F3
    sh(0, sp + 0x18);
    sh(v1, sp + 0x20);
    sh(0, sp + 0x28);
    sh(v0, sp + 0x2A);
    sh(v1, sp + 0x30);
    sh(v0, sp + 0x32);
    v0 = lbu(a0 + 0x8);
    t6 = 0;                                             // Result = 00000000
    sb(v0, sp + 0x1C);
    v0 = lbu(a0 + 0x9);
    t4 = 0xFF0000;                                      // Result = 00FF0000
    sb(v0, sp + 0x1D);
    v0 = lbu(a0 + 0x8);
    t4 |= 0xFFFF;                                       // Result = 00FFFFFF
    v0 += 0x3F;
    sb(v0, sp + 0x24);
    v0 = lbu(a0 + 0x9);
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    sb(v0, sp + 0x25);
    v0 = lbu(a0 + 0x8);
    t9 = s0 & t4;                                       // Result = 00086550
    sb(v0, sp + 0x2C);
    v0 = lbu(a0 + 0x9);
    t8 = 0x4000000;                                     // Result = 04000000
    v0 += 0x7F;
    sb(v0, sp + 0x2D);
    v0 = lbu(a0 + 0x8);
    t7 = 0x80000000;                                    // Result = 80000000
    v0 += 0x3F;
    sb(v0, sp + 0x34);
    v1 = lbu(a0 + 0x9);
    v0 = 0x80;                                          // Result = 00000080
    sb(v0, sp + 0x14);
    sb(v0, sp + 0x15);
    sb(v0, sp + 0x16);
    v1 += 0x7F;
    sb(v1, sp + 0x35);
    v0 = lhu(a0 + 0xA);
    v1 = *gPaletteClutId_CurMapSky;
    t5 = -1;                                            // Result = FFFFFFFF
    sh(v0, sp + 0x26);
    sh(v1, sp + 0x1E);
loc_8003589C:
    t3 = sp + 0x14;
    t0 = lbu(sp + 0x13);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_800358B4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8003591C;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_800359E0;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= t9;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003591C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_800359D0;
    if (v1 == a0) goto loc_800358B4;
loc_80035940:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t8;
    if (v0 == 0) goto loc_800358B4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t7;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_800359AC;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80035990:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_80035990;
loc_800359AC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800358B4;
    goto loc_80035940;
loc_800359D0:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_800359E0:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t4;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    a2 += 4;
    if (t0 == t5) goto loc_80035AA8;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80035A1C:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80035A1C;
    goto loc_80035AA8;
loc_80035A3C:
    v0 = lw(gp + 0x650);                                // Load from: GPU_REG_GP1 (80077C30)
    v0 = lw(v0);
    v0 &= t8;
    if (v0 == 0) goto loc_80035AC4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t4;
    v0 |= t7;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t5) goto loc_80035AA8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80035A8C:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x64C);                                // Load from: GPU_REG_GP0 (80077C2C)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80035A8C;
loc_80035AA8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80035A3C;
loc_80035AC4:
    t6++;
    v0 = lhu(sp + 0x18);
    v1 = lhu(sp + 0x20);
    v0 += 0x3F;
    sh(v0, sp + 0x18);
    v0 = lhu(sp + 0x28);
    v1 += 0x3F;
    sh(v1, sp + 0x20);
    v1 = lhu(sp + 0x30);
    v0 += 0x3F;
    v1 += 0x3F;
    sh(v0, sp + 0x28);
    v0 = (i32(t6) < 4);
    sh(v1, sp + 0x30);
    if (v0 != 0) goto loc_8003589C;
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x44);
    s0 = lw(sp + 0x40);
    sp += 0x48;
    return;
}
