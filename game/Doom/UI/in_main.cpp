#include "in_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Game/p_password.h"
#include "PsxVm/PsxVm.h"
#include "Wess/psxcd.h"

void IN_Start() noexcept {
    sp -= 0x28;
    sw(ra, sp + 0x20);
    I_ResetTexCache();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7A10;                                       // Result = gTexInfo_BACK[0] (80097A10)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7CFC;                                       // Result = STR_LumpName_BACK_2[0] (80077CFC)
    a2 = 0;                                             // Result = 00000000
    I_CacheTexForLumpName();
    t3 = 0x64;                                          // Result = 00000064
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    a0 = 0;                                             // Result = 00000000
    t4 = 0x80080000;                                    // Result = 80080000
    t4 -= 0x7D60;                                       // Result = 800782A0
    a3 = t4;                                            // Result = 800782A0
    t2 = 0x80070000;                                    // Result = 80070000
    t2 = lw(t2 + 0x7F20);                               // Load from: gTotalKills (80077F20)
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7F2C);                               // Load from: gTotalItems (80077F2C)
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lw(t0 + 0x7FEC);                               // Load from: gTotalSecret (80077FEC)
loc_8003C7B4:
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7D98;                                       // Result = 80078268
    at += a2;
    sw(0, at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FDC;                                       // Result = 80077FDC
    at += a2;
    sw(0, at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7D54;                                       // Result = 800782AC
    at += a2;
    sw(0, at);
    sw(0, a3);
    if (t2 == 0) goto loc_8003C85C;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x774C;                                       // Result = gPlayer1[32] (800A88B4)
    at += a1;
    v0 = lw(at);
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v1 += v0;
    v1 <<= 2;
    div(v1, t2);
    if (t2 != 0) goto loc_8003C824;
    _break(0x1C00);
loc_8003C824:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (t2 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8003C83C;
    }
    if (v1 != at) goto loc_8003C83C;
    tge(zero, zero, 0x5D);
loc_8003C83C:
    v1 = lo;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C24;                                       // Result = 80097C24
    at += a0;
    sw(v1, at);
    goto loc_8003C86C;
loc_8003C85C:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C24;                                       // Result = 80097C24
    at += a0;
    sw(t3, at);
loc_8003C86C:
    if (t1 == 0) goto loc_8003C8E4;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7748;                                       // Result = gPlayer1[33] (800A88B8)
    at += a1;
    v0 = lw(at);
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v1 += v0;
    v1 <<= 2;
    div(v1, t1);
    if (t1 != 0) goto loc_8003C8AC;
    _break(0x1C00);
loc_8003C8AC:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (t1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8003C8C4;
    }
    if (v1 != at) goto loc_8003C8C4;
    tge(zero, zero, 0x5D);
loc_8003C8C4:
    v1 = lo;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C28;                                       // Result = 80097C28
    at += a0;
    sw(v1, at);
    goto loc_8003C8F4;
loc_8003C8E4:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C28;                                       // Result = 80097C28
    at += a0;
    sw(t3, at);
loc_8003C8F4:
    if (t0 == 0) goto loc_8003C96C;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7744;                                       // Result = gPlayer1[34] (800A88BC)
    at += a1;
    v0 = lw(at);
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v1 += v0;
    v1 <<= 2;
    div(v1, t0);
    if (t0 != 0) goto loc_8003C934;
    _break(0x1C00);
loc_8003C934:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (t0 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8003C94C;
    }
    if (v1 != at) goto loc_8003C94C;
    tge(zero, zero, 0x5D);
loc_8003C94C:
    v1 = lo;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C2C;                                       // Result = 80097C2C
    at += a0;
    sw(v1, at);
    goto loc_8003C97C;
loc_8003C96C:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C2C;                                       // Result = 80097C2C
    at += a0;
    sw(t3, at);
loc_8003C97C:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7FA4);                               // Load from: gNetGame (8007805C)
    v0 = 2;                                             // Result = 00000002
    a2 += 4;
    if (v1 != v0) goto loc_8003C9B4;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x77B0;                                       // Result = gPlayer1[19] (800A8850)
    at += a1;
    v0 = lw(at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C30;                                       // Result = 80097C30
    at += a0;
    sw(v0, at);
loc_8003C9B4:
    a1 += 0x12C;
    a3 += 4;
    v0 = t4 + 8;                                        // Result = gCloseDist (800782A8)
    v0 = (i32(a3) < i32(v0));
    a0 += 0x10;
    if (v0 != 0) goto loc_8003C7B4;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F68);                               // Load from: gNextMap (80078098)
    sw(0, gp + 0xCFC);                                  // Store to: 800782DC
    v0 = (i32(v0) < 0x3C);
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x7F0C);                                // Store to: gMenuTimeoutStartTicCon (80077F0C)
    if (v0 == 0) goto loc_8003CA10;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x6560;                                       // Result = gPasswordChars[0] (80096560)
    P_ComputePassword();
    v0 = 0xA;                                           // Result = 0000000A
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C40);                                // Store to: gNumPasswordCharsEntered (80077C40)
loc_8003CA10:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3E58;                                       // Result = CDTrackNum_Intermission (80073E58)
    a0 = lw(v0);                                        // Load from: CDTrackNum_Intermission (80073E58)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x75F8);                               // Load from: gCdMusicVol (800775F8)
    a2 = 0;                                             // Result = 00000000
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    v0 = lw(v0);                                        // Load from: CDTrackNum_Intermission (80073E58)
    a3 = 0;                                             // Result = 00000000
    sw(v0, sp + 0x10);
    sw(a1, sp + 0x14);
    psxcd_play_at_andloop();
loc_8003CA44:
    psxcd_elapsed_sectors();
    if (v0 == 0) goto loc_8003CA44;
    ra = lw(sp + 0x20);
    sp += 0x28;
    return;
}

void IN_Stop() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    IN_Drawer();
    psxcd_stop();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void IN_Ticker() noexcept {
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB4);                               // Load from: gTicCon (8007814C)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F0C);                               // Load from: gMenuTimeoutStartTicCon (80077F0C)
    sp -= 0x28;
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 -= v1;
    v0 = (i32(v0) < 0x3D);
    sw(s0, sp + 0x10);
    if (v0 != 0) goto loc_8003CE4C;
    s0 = 0;                                             // Result = 00000000
    s3 = 0x80080000;                                    // Result = 80080000
    s3 -= 0x7D60;                                       // Result = 800782A0
    s2 = 0x80080000;                                    // Result = 80080000
    s2 -= 0x7D54;                                       // Result = 800782AC
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x7FDC;                                       // Result = 80077FDC
    v0 = s0 << 2;                                       // Result = 00000000
loc_8003CAE0:
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gPlayerPadButtons[0] (80077F44)
    at += v0;
    v1 = lw(at);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7DEC;                                       // Result = gPlayerOldPadButtons[0] (80078214)
    at += v0;
    v0 = lw(at);
    {
        const bool bJump = (v1 == v0);
        v0 = v1 & 0xF0;
        if (bJump) goto loc_8003CBD4;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8003CBD4;
    }
    v1 = lw(gp + 0xCFC);                                // Load from: 800782DC
    v1++;
    sw(v1, gp + 0xCFC);                                 // Store to: 800782DC
    a2 = s1;                                            // Result = 80077FDC
    if (v1 != v0) goto loc_8003CBC0;
    s0 = 0;                                             // Result = 00000000
    a3 = 0x80080000;                                    // Result = 80080000
    a3 -= 0x7D98;                                       // Result = 80078268
    a1 = s2;                                            // Result = 800782AC
    a0 = s3;                                            // Result = 800782A0
    v1 = 0;                                             // Result = 00000000
loc_8003CB44:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C24;                                       // Result = 80097C24
    at += v1;
    v0 = lw(at);
    s0++;
    sw(v0, a0);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C28;                                       // Result = 80097C28
    at += v1;
    v0 = lw(at);
    a0 += 4;
    sw(v0, a1);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C2C;                                       // Result = 80097C2C
    at += v1;
    v0 = lw(at);
    a1 += 4;
    sw(v0, a2);
    a2 += 4;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C30;                                       // Result = 80097C30
    at += v1;
    v0 = lw(at);
    v1 += 0x10;
    sw(v0, a3);
    v0 = (i32(s0) < 2);
    a3 += 4;
    if (v0 != 0) goto loc_8003CB44;
    a0 = 0;                                             // Result = 00000000
    a1 = 5;                                             // Result = 00000005
    S_StartSound();
loc_8003CBC0:
    v0 = lw(gp + 0xCFC);                                // Load from: 800782DC
    v0 = (i32(v0) < 2);
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8003CC20;
loc_8003CBD4:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7FA4);                               // Load from: gNetGame (8007805C)
    if (a0 == 0) goto loc_8003CBF8;
    s0++;
    v0 = (i32(s0) < 2);
    {
        const bool bJump = (v0 != 0);
        v0 = s0 << 2;
        if (bJump) goto loc_8003CAE0;
    }
loc_8003CBF8:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7FB4);                               // Load from: gGameTic (8007804C)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7FA4);                               // Load from: gPrevGameTic (80077FA4)
    v0 = (i32(v0) < i32(v1));
    s0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_8003CC30;
    v0 = 0;                                             // Result = 00000000
    goto loc_8003CE50;
loc_8003CC20:
    a1 = 5;                                             // Result = 00000005
    S_StartSound();
    v0 = 1;                                             // Result = 00000001
    goto loc_8003CE50;
loc_8003CC30:
    t3 = a0;
    a1 = 0;                                             // Result = 00000000
    t1 = 0;                                             // Result = 00000000
    t2 = 0x80080000;                                    // Result = 80080000
    t2 -= 0x7D54;                                       // Result = 800782AC
    a3 = t2;                                            // Result = 800782AC
    t0 = 0x80080000;                                    // Result = 80080000
    t0 -= 0x7D60;                                       // Result = 800782A0
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7D98;                                       // Result = 80078268
loc_8003CC58:
    v0 = 2;                                             // Result = 00000002
    if (t3 != v0) goto loc_8003CCF0;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C30;                                       // Result = 80097C30
    at += a1;
    v0 = lw(at);
    if (i32(v0) >= 0) goto loc_8003CCC0;
    v1 = lw(a2);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 - 2;
        if (bJump) goto loc_8003CDE0;
    }
    sw(v0, a2);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C30;                                       // Result = 80097C30
    at += a1;
    v1 = lw(at);
    v0 = (i32(v0) < i32(v1));
loc_8003CCB0:
    s0 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8003CDE0;
    sw(v1, a2);
    goto loc_8003CDE0;
loc_8003CCC0:
    v1 = lw(a2);
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 + 2;
        if (bJump) goto loc_8003CDE0;
    }
    sw(v0, a2);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C30;                                       // Result = 80097C30
    at += a1;
    v1 = lw(at);
    v0 = (i32(v1) < i32(v0));
    goto loc_8003CCB0;
loc_8003CCF0:
    v1 = lw(t0);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C24;                                       // Result = 80097C24
    at += a1;
    v0 = lw(at);
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 + 2;
        if (bJump) goto loc_8003CD3C;
    }
    sw(v0, t0);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C24;                                       // Result = 80097C24
    at += a1;
    v1 = lw(at);
    v0 = (i32(v1) < i32(v0));
    s0 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8003CD3C;
    sw(v1, t0);
loc_8003CD3C:
    v1 = lw(a3);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C28;                                       // Result = 80097C28
    at += a1;
    v0 = lw(at);
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 + 2;
        if (bJump) goto loc_8003CD88;
    }
    sw(v0, a3);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C28;                                       // Result = 80097C28
    at += a1;
    v1 = lw(at);
    v0 = (i32(v1) < i32(v0));
    s0 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8003CD88;
    sw(v1, a3);
loc_8003CD88:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x7FDC;                                       // Result = 80077FDC
    a0 = t1 + v0;
    v1 = lw(a0);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C2C;                                       // Result = 80097C2C
    at += a1;
    v0 = lw(at);
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = v1 + 2;
        if (bJump) goto loc_8003CDE0;
    }
    sw(v0, a0);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x7C2C;                                       // Result = 80097C2C
    at += a1;
    v1 = lw(at);
    v0 = (i32(v1) < i32(v0));
    s0 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8003CDE0;
    sw(v1, a0);
loc_8003CDE0:
    a1 += 0x10;
    t1 += 4;
    a3 += 4;
    t0 += 4;
    v0 = t2 + 8;                                        // Result = gTestFlags (800782B4)
    v0 = (i32(a3) < i32(v0));
    a2 += 4;
    if (v0 != 0) goto loc_8003CC58;
    v1 = 1;                                             // Result = 00000001
    if (s0 != 0) goto loc_8003CE24;
    v0 = lw(gp + 0xCFC);                                // Load from: 800782DC
    a0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_8003CE24;
    sw(v1, gp + 0xCFC);                                 // Store to: 800782DC
    a1 = 5;                                             // Result = 00000005
    S_StartSound();
loc_8003CE24:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FB4);                               // Load from: gGameTic (8007804C)
    v0 &= 1;
    if (v0 != 0) goto loc_8003CE4C;
    a0 = 0;                                             // Result = 00000000
    if (s0 == 0) goto loc_8003CE4C;
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
loc_8003CE4C:
    v0 = 0;                                             // Result = 00000000
loc_8003CE50:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void IN_Drawer() noexcept {
loc_8003CE70:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    I_IncDrawnFrameCount();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7FA4);                               // Load from: gNetGame (8007805C)
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8003CEA4;
    }
    IN_CoopDrawer();
    goto loc_8003CEC4;
loc_8003CEA4:
    if (v1 != v0) goto loc_8003CEBC;
    IN_DeathmatchDrawer();
    goto loc_8003CEC4;
loc_8003CEBC:
    IN_SingleDrawer();
loc_8003CEC4:
    I_SubmitGpuCmds();
    I_DrawPresent();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void IN_SingleDrawer() noexcept {
loc_8003CEE4:
    sp -= 0x30;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7A10;                                       // Result = gTexInfo_BACK[0] (80097A10)
    a1 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F7C);                               // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    I_CacheAndDrawSprite();
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = 0x14;                                          // Result = 00000014
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0x7FB8);                               // Load from: gGameMap (80078048)
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x40BC;                                       // Result = StatusBarWeaponBoxesXPos[6] (800740BC)
    a2 <<= 5;
    a2 += s1;
    I_DrawString();
    a0 = -1;                                            // Result = FFFFFFFF
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1648;                                       // Result = STR_Finished[0] (80011648)
    a1 = 0x24;                                          // Result = 00000024
    I_DrawString();
    a0 = 0x39;                                          // Result = 00000039
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D04;                                       // Result = STR_Kills[0] (80077D04)
    a1 = 0x41;                                          // Result = 00000041
    I_DrawString();
    a0 = 0xB6;                                          // Result = 000000B6
    a1 = 0x41;                                          // Result = 00000041
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x7D0C;                                       // Result = STR_Percent[0] (80077D0C)
    a2 = s0;                                            // Result = STR_Percent[0] (80077D0C)
    I_DrawString();
    a0 = 0xAA;                                          // Result = 000000AA
    a2 = lw(gp + 0xCC0);                                // Load from: 800782A0
    a1 = 0x41;                                          // Result = 00000041
    I_DrawNumber();
    a0 = 0x35;                                          // Result = 00000035
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D10;                                       // Result = STR_Items[0] (80077D10)
    a1 = 0x5B;                                          // Result = 0000005B
    I_DrawString();
    a0 = 0xB6;                                          // Result = 000000B6
    a1 = 0x5B;                                          // Result = 0000005B
    a2 = s0;                                            // Result = STR_Percent[0] (80077D0C)
    I_DrawString();
    a0 = 0xAA;                                          // Result = 000000AA
    a2 = lw(gp + 0xCCC);                                // Load from: 800782AC
    a1 = 0x5B;                                          // Result = 0000005B
    I_DrawNumber();
    a0 = 0x1A;                                          // Result = 0000001A
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D18;                                       // Result = STR_Secrets[0] (80077D18)
    a1 = 0x75;                                          // Result = 00000075
    I_DrawString();
    a0 = 0xB6;                                          // Result = 000000B6
    a1 = 0x75;                                          // Result = 00000075
    a2 = s0;                                            // Result = STR_Percent[0] (80077D0C)
    I_DrawString();
    a0 = 0xAA;                                          // Result = 000000AA
    a2 = lw(gp + 0x9FC);                                // Load from: 80077FDC
    a1 = 0x75;                                          // Result = 00000075
    I_DrawNumber();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F68);                               // Load from: gNextMap (80078098)
    v0 = (i32(v0) < 0x3C);
    a0 = -1;                                            // Result = FFFFFFFF
    if (v0 == 0) goto loc_8003D09C;
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1654;                                       // Result = STR_Entering[0] (80011654)
    a1 = 0x91;                                          // Result = 00000091
    I_DrawString();
    a0 = -1;                                            // Result = FFFFFFFF
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0x7F68);                               // Load from: gNextMap (80078098)
    a1 = 0xA1;                                          // Result = 000000A1
    a2 <<= 5;
    a2 += s1;
    I_DrawString();
    a0 = -1;                                            // Result = FFFFFFFF
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1660;                                       // Result = STR_Password[0] (80011660)
    a1 = 0xBB;                                          // Result = 000000BB
    I_DrawString();
    v1 = 0;                                             // Result = 00000000
    a0 = sp + 0x10;
loc_8003D04C:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x6560;                                       // Result = gPasswordChars[0] (80096560)
    at += v1;
    v0 = lbu(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3D4C;                                       // Result = STR_PasswordChars[0] (80073D4C)
    at += v0;
    v0 = lbu(at);
    v1++;
    sb(v0, a0);
    v0 = (i32(v1) < 0xA);
    a0++;
    if (v0 != 0) goto loc_8003D04C;
    a2 = sp + 0x10;
    v0 = a2 + v1;
    sb(0, v0);
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = 0xCB;                                          // Result = 000000CB
    I_DrawString();
loc_8003D09C:
    ra = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void IN_CoopDrawer() noexcept {
loc_8003D0B4:
    sp -= 0x58;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7A10;                                       // Result = gTexInfo_BACK[0] (80097A10)
    a1 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F7C);                               // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x50);
    sw(s1, sp + 0x4C);
    sw(s0, sp + 0x48);
    I_CacheAndDrawSprite();
    a2 = 0x8B;                                          // Result = 0000008B
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6B0E;                                       // Result = gTexInfo_STATUS[2] (800A94F2)
    s1 = 0x80070000;                                    // Result = 80070000
    s1 += 0x3E6A;                                       // Result = StatusBarFaceSpriteInfo[2] (80073E6A)
    a0 = lhu(s0);                                       // Load from: gTexInfo_STATUS[2] (800A94F2)
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lh(a1 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    v0 = lbu(s1);                                       // Load from: StatusBarFaceSpriteInfo[2] (80073E6A)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x3E6B);                              // Load from: StatusBarFaceSpriteInfo[3] (80073E6B)
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lbu(t0 + 0x3E6C);                              // Load from: StatusBarFaceSpriteInfo[4] (80073E6C)
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lbu(t1 + 0x3E6D);                              // Load from: StatusBarFaceSpriteInfo[5] (80073E6D)
    a3 = 0x14;                                          // Result = 00000014
    sw(v0, sp + 0x10);
    sw(v1, sp + 0x14);
    sw(t0, sp + 0x18);
    sw(t1, sp + 0x1C);
    I_DrawSprite();
    a0 = 0x82;                                          // Result = 00000082
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D20;                                       // Result = STR_You[0] (80077D20)
    a1 = 0x34;                                          // Result = 00000034
    I_DrawString();
    a2 = 0xD5;                                          // Result = 000000D5
    a0 = lhu(s0);                                       // Load from: gTexInfo_STATUS[2] (800A94F2)
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lh(a1 - 0x6F5C);                               // Load from: gPaletteClutId_UI (800A90A4)
    v0 = lbu(s1);                                       // Load from: StatusBarFaceSpriteInfo[2] (80073E6A)
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lbu(v1 + 0x3E6B);                              // Load from: StatusBarFaceSpriteInfo[3] (80073E6B)
    t0 = 0x80070000;                                    // Result = 80070000
    t0 = lbu(t0 + 0x3E6C);                              // Load from: StatusBarFaceSpriteInfo[4] (80073E6C)
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lbu(t1 + 0x3E6D);                              // Load from: StatusBarFaceSpriteInfo[5] (80073E6D)
    a3 = 0x14;                                          // Result = 00000014
    sw(v0, sp + 0x10);
    sw(v1, sp + 0x14);
    sw(t0, sp + 0x18);
    sw(t1, sp + 0x1C);
    I_DrawSprite();
    a0 = 0xD0;                                          // Result = 000000D0
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D24;                                       // Result = STR_Him[0] (80077D24)
    a1 = 0x34;                                          // Result = 00000034
    I_DrawString();
    a0 = 0x39;                                          // Result = 00000039
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D04;                                       // Result = STR_Kills[0] (80077D04)
    a1 = 0x4F;                                          // Result = 0000004F
    I_DrawString();
    a0 = 0x9B;                                          // Result = 0000009B
    a1 = 0x4F;                                          // Result = 0000004F
    s0 = 0x80070000;                                    // Result = 80070000
    s0 += 0x7D0C;                                       // Result = STR_Percent[0] (80077D0C)
    a2 = s0;                                            // Result = STR_Percent[0] (80077D0C)
    I_DrawString();
    a0 = 0xE4;                                          // Result = 000000E4
    a1 = 0x4F;                                          // Result = 0000004F
    a2 = s0;                                            // Result = STR_Percent[0] (80077D0C)
    I_DrawString();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a0 = 0x8F;                                          // Result = 0000008F
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7D60;                                       // Result = 800782A0
    at += v0;
    a2 = lw(at);
    a1 = 0x4F;                                          // Result = 0000004F
    I_DrawNumber();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7D60;                                       // Result = 800782A0
    a0 = 0xD8;                                          // Result = 000000D8
    if (v0 != 0) goto loc_8003D224;
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7D5C;                                       // Result = 800782A4
loc_8003D224:
    a2 = lw(a2);
    a1 = 0x4F;                                          // Result = 0000004F
    I_DrawNumber();
    a0 = 0x35;                                          // Result = 00000035
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D10;                                       // Result = STR_Items[0] (80077D10)
    a1 = 0x65;                                          // Result = 00000065
    I_DrawString();
    a0 = 0x9B;                                          // Result = 0000009B
    a1 = 0x65;                                          // Result = 00000065
    a2 = s0;                                            // Result = STR_Percent[0] (80077D0C)
    I_DrawString();
    a0 = 0xE4;                                          // Result = 000000E4
    a1 = 0x65;                                          // Result = 00000065
    a2 = s0;                                            // Result = STR_Percent[0] (80077D0C)
    I_DrawString();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a0 = 0x8F;                                          // Result = 0000008F
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7D54;                                       // Result = 800782AC
    at += v0;
    a2 = lw(at);
    a1 = 0x65;                                          // Result = 00000065
    I_DrawNumber();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7D54;                                       // Result = 800782AC
    a0 = 0xD8;                                          // Result = 000000D8
    if (v0 != 0) goto loc_8003D2AC;
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7D50;                                       // Result = 800782B0
loc_8003D2AC:
    a2 = lw(a2);
    a1 = 0x65;                                          // Result = 00000065
    I_DrawNumber();
    a0 = 0x1A;                                          // Result = 0000001A
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D18;                                       // Result = STR_Secrets[0] (80077D18)
    a1 = 0x7B;                                          // Result = 0000007B
    I_DrawString();
    a0 = 0x9B;                                          // Result = 0000009B
    a1 = 0x7B;                                          // Result = 0000007B
    a2 = s0;                                            // Result = STR_Percent[0] (80077D0C)
    I_DrawString();
    a0 = 0xE4;                                          // Result = 000000E4
    a1 = 0x7B;                                          // Result = 0000007B
    a2 = s0;                                            // Result = STR_Percent[0] (80077D0C)
    I_DrawString();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a0 = 0x8F;                                          // Result = 0000008F
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FDC;                                       // Result = 80077FDC
    at += v0;
    a2 = lw(at);
    a1 = 0x7B;                                          // Result = 0000007B
    I_DrawNumber();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7FDC;                                       // Result = 80077FDC
    a0 = 0xD8;                                          // Result = 000000D8
    if (v0 != 0) goto loc_8003D334;
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7FE0;                                       // Result = 80077FE0
loc_8003D334:
    a2 = lw(a2);
    a1 = 0x7B;                                          // Result = 0000007B
    I_DrawNumber();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F68);                               // Load from: gNextMap (80078098)
    v0 = (i32(v0) < 0x3C);
    a0 = -1;                                            // Result = FFFFFFFF
    if (v0 == 0) goto loc_8003D430;
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1654;                                       // Result = STR_Entering[0] (80011654)
    a1 = 0x95;                                          // Result = 00000095
    I_DrawString();
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = 0xA5;                                          // Result = 000000A5
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0x7F68);                               // Load from: gNextMap (80078098)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x40BC;                                       // Result = StatusBarWeaponBoxesXPos[6] (800740BC)
    a2 <<= 5;
    a2 += v0;
    I_DrawString();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x77F0;                                       // Result = gPlayer1[9] (800A8810)
    at += v0;
    v0 = lw(at);
    a0 = -1;                                            // Result = FFFFFFFF
    if (i32(v0) <= 0) goto loc_8003D430;
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1660;                                       // Result = STR_Password[0] (80011660)
    a1 = 0xBF;                                          // Result = 000000BF
    I_DrawString();
    v1 = 0;                                             // Result = 00000000
    a0 = sp + 0x20;
loc_8003D3E0:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x6560;                                       // Result = gPasswordChars[0] (80096560)
    at += v1;
    v0 = lbu(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x3D4C;                                       // Result = STR_PasswordChars[0] (80073D4C)
    at += v0;
    v0 = lbu(at);
    v1++;
    sb(v0, a0);
    v0 = (i32(v1) < 0xA);
    a0++;
    if (v0 != 0) goto loc_8003D3E0;
    a2 = sp + 0x20;
    v0 = a2 + v1;
    sb(0, v0);
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = 0xCF;                                          // Result = 000000CF
    I_DrawString();
loc_8003D430:
    ra = lw(sp + 0x50);
    s1 = lw(sp + 0x4C);
    s0 = lw(sp + 0x48);
    sp += 0x58;
    return;
}

void IN_DeathmatchDrawer() noexcept {
loc_8003D448:
    sp -= 0x50;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7A10;                                       // Result = gTexInfo_BACK[0] (80097A10)
    a1 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lh(a3 - 0x6F7C);                               // Load from: gPaletteClutId_Main (800A9084)
    a2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x48);
    sw(s3, sp + 0x44);
    sw(s2, sp + 0x40);
    sw(s1, sp + 0x3C);
    sw(s0, sp + 0x38);
    I_CacheAndDrawSprite();
    a0 = -1;                                            // Result = FFFFFFFF
    a1 = 0x14;                                          // Result = 00000014
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0x7FB8);                               // Load from: gGameMap (80078048)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x40BC;                                       // Result = StatusBarWeaponBoxesXPos[6] (800740BC)
    a2 <<= 5;
    a2 += v0;
    I_DrawString();
    a0 = -1;                                            // Result = FFFFFFFF
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1648;                                       // Result = STR_Finished[0] (80011648)
    a1 = 0x24;                                          // Result = 00000024
    I_DrawString();
    a0 = lw(gp + 0xC88);                                // Load from: 80078268
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D94);                               // Load from: 8007826C
    v0 = (i32(v1) < i32(a0));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a0) < i32(v1));
        if (bJump) goto loc_8003D504;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    if (v0 != 0) goto loc_8003D4F4;
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x3E8C;                                       // Result = StatusBarFaceSpriteInfo[24] (80073E8C)
    s3 = s2 + 0xD2;                                     // Result = StatusBarFaceSpriteInfo[F6] (80073F5E)
    goto loc_8003D554;
loc_8003D4F4:
    s3 = 0x80070000;                                    // Result = 80070000
    s3 += 0x3E8C;                                       // Result = StatusBarFaceSpriteInfo[24] (80073E8C)
    s2 = s3 + 0xD2;                                     // Result = StatusBarFaceSpriteInfo[F6] (80073F5E)
    goto loc_8003D554;
loc_8003D504:
    if (v0 == 0) goto loc_8003D540;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    if (v0 != 0) goto loc_8003D530;
    s2 = 0x80070000;                                    // Result = 80070000
    s2 += 0x3F5E;                                       // Result = StatusBarFaceSpriteInfo[F6] (80073F5E)
    s3 = s2 - 0xD2;                                     // Result = StatusBarFaceSpriteInfo[24] (80073E8C)
    goto loc_8003D554;
loc_8003D530:
    s3 = 0x80070000;                                    // Result = 80070000
    s3 += 0x3F5E;                                       // Result = StatusBarFaceSpriteInfo[F6] (80073F5E)
    s2 = s3 - 0xD2;                                     // Result = StatusBarFaceSpriteInfo[24] (80073E8C)
    goto loc_8003D554;
loc_8003D540:
    if (v1 != a0) goto loc_8003D554;
    s3 = 0x80070000;                                    // Result = 80070000
    s3 += 0x3E68;                                       // Result = StatusBarFaceSpriteInfo[0] (80073E68)
    s2 = s3;                                            // Result = StatusBarFaceSpriteInfo[0] (80073E68)
loc_8003D554:
    v0 = lbu(s2 + 0x2);
    a2 = 0x7F;                                          // Result = 0000007F
    sw(v0, sp + 0x10);
    v0 = lbu(s2 + 0x3);
    s1 = 0x800B0000;                                    // Result = 800B0000
    s1 -= 0x6B0E;                                       // Result = gTexInfo_STATUS[2] (800A94F2)
    sw(v0, sp + 0x14);
    a0 = lhu(s1);                                       // Load from: gTexInfo_STATUS[2] (800A94F2)
    v0 = lbu(s2 + 0x4);
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6F5C;                                       // Result = gPaletteClutId_UI (800A90A4)
    sw(v0, sp + 0x18);
    a1 = lh(s0);                                        // Load from: gPaletteClutId_UI (800A90A4)
    v0 = lbu(s2 + 0x5);
    a3 = 0x46;                                          // Result = 00000046
    sw(v0, sp + 0x1C);
    I_DrawSprite();
    a0 = 0x76;                                          // Result = 00000076
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D20;                                       // Result = STR_You[0] (80077D20)
    a1 = 0x66;                                          // Result = 00000066
    I_DrawString();
    v0 = lbu(s3 + 0x2);
    sw(v0, sp + 0x10);
    v0 = lbu(s3 + 0x3);
    sw(v0, sp + 0x14);
    a0 = lhu(s1);                                       // Load from: gTexInfo_STATUS[2] (800A94F2)
    v0 = lbu(s3 + 0x4);
    a2 = 0xC8;                                          // Result = 000000C8
    sw(v0, sp + 0x18);
    a1 = lh(s0);                                        // Load from: gPaletteClutId_UI (800A90A4)
    v0 = lbu(s3 + 0x5);
    a3 = 0x46;                                          // Result = 00000046
    sw(v0, sp + 0x1C);
    I_DrawSprite();
    a0 = 0xC3;                                          // Result = 000000C3
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D24;                                       // Result = STR_Him[0] (80077D24)
    a1 = 0x66;                                          // Result = 00000066
    I_DrawString();
    a0 = 0x23;                                          // Result = 00000023
    a2 = 0x80070000;                                    // Result = 80070000
    a2 += 0x7D28;                                       // Result = STR_Frags[0] (80077D28)
    a1 = 0x8A;                                          // Result = 0000008A
    I_DrawString();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a0 = 0x85;                                          // Result = 00000085
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7D98;                                       // Result = 80078268
    at += v0;
    a2 = lw(at);
    a1 = 0x8A;                                          // Result = 0000008A
    I_DrawNumber();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7D98;                                       // Result = 80078268
    a0 = 0xCE;                                          // Result = 000000CE
    if (v0 != 0) goto loc_8003D658;
    a2 = 0x80080000;                                    // Result = 80080000
    a2 -= 0x7D94;                                       // Result = 8007826C
loc_8003D658:
    a2 = lw(a2);
    a1 = 0x8A;                                          // Result = 0000008A
    I_DrawNumber();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F68);                               // Load from: gNextMap (80078098)
    v0 = (i32(v0) < 0x3C);
    a0 = -1;                                            // Result = FFFFFFFF
    if (v0 == 0) goto loc_8003D6B0;
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0x1654;                                       // Result = STR_Entering[0] (80011654)
    a1 = 0xBE;                                          // Result = 000000BE
    I_DrawString();
    a0 = -1;                                            // Result = FFFFFFFF
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0x7F68);                               // Load from: gNextMap (80078098)
    a1 = 0xCE;                                          // Result = 000000CE
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x40BC;                                       // Result = StatusBarWeaponBoxesXPos[6] (800740BC)
    a2 <<= 5;
    a2 += v0;
    I_DrawString();
loc_8003D6B0:
    ra = lw(sp + 0x48);
    s3 = lw(sp + 0x44);
    s2 = lw(sp + 0x40);
    s1 = lw(sp + 0x3C);
    s0 = lw(sp + 0x38);
    sp += 0x50;
    return;
}
