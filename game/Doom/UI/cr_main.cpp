#include "cr_main.h"

#include "Doom/Base/i_main.h"
#include "PsxVm/PsxVm.h"
#include "Wess/psxcd.h"

void START_Credits() noexcept {
    sp -= 0x28;
    sw(s0, sp + 0x20);
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7B30;                                       // Result = gTexInfo_IDCRED1[0] (80097B30)
    a0 = s0;                                            // Result = gTexInfo_IDCRED1[0] (80097B30)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7CBC;                                       // Result = STR_LumpName_IDCRED1[0] (80077CBC)
    sw(ra, sp + 0x24);
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    a0 = s0 + 0x20;                                     // Result = gTexInfo_IDCRED2[0] (80097B50)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7CC4;                                       // Result = STR_LumpName_IDCRED2[0] (80077CC4)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    a0 = s0 + 0x40;                                     // Result = gTexInfo_WMSCRED1[0] (80097B70)
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1590;                                       // Result = STR_LumpName_WMSCRED1[0] (80011590)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    a0 = s0 + 0x60;                                     // Result = gTexInfo_WMSCRED2[0] (80097B90)
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x159C;                                       // Result = STR_LumpName_WMSCRED2[0] (8001159C)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x3E54;                                       // Result = CDTrackNum_Credits_Demo (80073E54)
    a2 = 0;                                             // Result = 00000000
    a0 = lw(v1);                                        // Load from: CDTrackNum_Credits_Demo (80073E54)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75F8);                               // Load from: gCdMusicVol (800775F8)
    v0 = 0xF0;                                          // Result = 000000F0
    sw(v0, gp + 0xC7C);                                 // Store to: gCreditsScrollYPos (8007825C)
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    v0 = lw(v1);                                        // Load from: CDTrackNum_Credits_Demo (80073E54)
    a3 = 0;                                             // Result = 00000000
    sw(0, gp + 0xBC4);                                  // Store to: gCreditsPage (800781A4)
    sw(v0, sp + 0x10);
    sw(a1, sp + 0x14);
    psxcd_play_at_andloop();
loc_80036C7C:
    psxcd_elapsed_sectors();
    if (v0 == 0) goto loc_80036C7C;
    ra = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x28;
    return;
}

void STOP_Credits() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    psxcd_stop();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void TIC_Credits() noexcept {
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F44);                               // Load from: gTicButtons[0] (80077F44)
    {
        const bool bJump = (v0 != 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_80036D50;
    }
    v0 = lw(gp + 0x6EC);                                // Load from: gVBlanksUntilCreditScreenUpdate (80077CCC)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FBC);                               // Load from: gPlayersElapsedVBlanks[0] (80077FBC)
    v0 -= v1;
    sw(v0, gp + 0x6EC);                                 // Store to: gVBlanksUntilCreditScreenUpdate (80077CCC)
    {
        const bool bJump = (i32(v0) > 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80036D50;
    }
    v0 = 2;                                             // Result = 00000002
    sw(v0, gp + 0x6EC);                                 // Store to: gVBlanksUntilCreditScreenUpdate (80077CCC)
    v0 = lw(gp + 0xC7C);                                // Load from: gCreditsScrollYPos (8007825C)
    a0 = lw(gp + 0xBC4);                                // Load from: gCreditsPage (800781A4)
    v1 = v0 - 1;
    sw(v1, gp + 0xC7C);                                 // Store to: gCreditsScrollYPos (8007825C)
    v0 = 1;                                             // Result = 00000001
    if (a0 == 0) goto loc_80036D24;
    {
        const bool bJump = (a0 == v0);
        v0 = (i32(v1) < -0xE4);
        if (bJump) goto loc_80036D44;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_80036D50;
loc_80036D24:
    v0 = (i32(v1) < -0xB6);
    {
        const bool bJump = (v0 == 0);
        v0 = 0xF0;                                      // Result = 000000F0
        if (bJump) goto loc_80036D4C;
    }
    sw(v0, gp + 0xC7C);                                 // Store to: gCreditsScrollYPos (8007825C)
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0xBC4);                                 // Store to: gCreditsPage (800781A4)
    v0 = 0;                                             // Result = 00000000
    goto loc_80036D50;
loc_80036D44:
    {
        const bool bJump = (v0 != 0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_80036D50;
    }
loc_80036D4C:
    v0 = 0;                                             // Result = 00000000
loc_80036D50:
    return;
}

void DRAW_Credits() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    I_IncDrawnFrameCount();
    v1 = lw(gp + 0xBC4);                                // Load from: gCreditsPage (800781A4)
    v0 = 1;                                             // Result = 00000001
    if (v1 == 0) goto loc_80036D88;
    a1 = 0;                                             // Result = 00000000
    if (v1 == v0) goto loc_80036DC0;
    goto loc_80036DF8;
loc_80036D88:
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7B30;                                       // Result = gTexInfo_IDCRED1[0] (80097B30)
    a0 = s0;                                            // Result = gTexInfo_IDCRED1[0] (80097B30)
    a1 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F58);                               // Load from: gPaletteClutId_IdCredits1 (800A90A8)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_CacheAndDrawSprite();
    a0 = s0 + 0x20;                                     // Result = gTexInfo_IDCRED2[0] (80097B50)
    a2 = lw(gp + 0xC7C);                                // Load from: gCreditsScrollYPos (8007825C)
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    a1 = 9;                                             // Result = 00000009
    goto loc_80036DF0;
loc_80036DC0:
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7B70;                                       // Result = gTexInfo_WMSCRED1[0] (80097B70)
    a0 = s0;                                            // Result = gTexInfo_WMSCRED1[0] (80097B70)
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F56);                               // Load from: gPaletteClutId_WilliamsCredits1 (800A90AA)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_CacheAndDrawSprite();
    a0 = s0 + 0x20;                                     // Result = gTexInfo_WMSCRED2[0] (80097B90)
    a2 = lw(gp + 0xC7C);                                // Load from: gCreditsScrollYPos (8007825C)
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    a1 = 7;                                             // Result = 00000007
loc_80036DF0:
    _thunk_I_CacheAndDrawSprite();
loc_80036DF8:
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}
