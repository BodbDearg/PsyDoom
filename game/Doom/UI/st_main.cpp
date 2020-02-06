#include "st_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBC2.h"
#include "PsyQ/LIBGPU.h"

const VmPtr<VmPtr<const char>>  gpStatusBarMsgStr(0x80098740);
const VmPtr<int32_t>            gStatusBarMsgTicsLeft(0x80098744);

void ST_Init() noexcept {
loc_80038558:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FD8);                               // Load from: gTexCacheFillPage (80078028)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_8003857C;
    I_Error("ST_Init: initial texture cache foulup\n");
loc_8003857C:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x6B18;                                       // Result = gTex_STATUS[0] (800A94E8)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 += 0x7CE4;                                       // Result = STR_LumpName_STATUS[0] (80077CE4)
    a2 = 0;                                             // Result = 00000000
    _thunk_I_LoadAndCacheTexLump();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7FD8);                               // Load from: gTexCacheFillPage (80078028)
    if (v0 == 0) goto loc_800385B8;
    I_Error("ST_Init: final texture cache foulup\n");
loc_800385B8:
    v1 = *gLockedTexPagesMask;
    v0 = 1;                                             // Result = 00000001
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D1C);                                 // Store to: gTexCacheFillBlockX (800782E4)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D18);                                 // Store to: gTexCacheFillBlockY (800782E8)
    at = 0x80080000;                                    // Result = 80080000
    sw(v0, at - 0x7FD8);                                // Store to: gTexCacheFillPage (80078028)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7D88);                                 // Store to: gTexCacheRowBlockH (80078278)
    v1 |= 1;
    *gLockedTexPagesMask = v1;
    
    Z_FreeTags(**gpMainMemZone, PU_CACHE);

    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void ST_Start() noexcept {
loc_80038610:
    a1 = 0;                                             // Result = 00000000
    a0 = 0;                                             // Result = 00000000
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 -= 0x78CC;                                       // Result = gStatusBar[7] (80098734)
    v1 = v0 - 0x18;                                     // Result = gStatusBar[1] (8009871C)
    sw(0, v0);                                          // Store to: gStatusBar[7] (80098734)
    v0 = 1;                                             // Result = 00000001
    sw(v0, gp + 0xB50);                                 // Store to: gbDrawStatusBarFace (80078130)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3E68;                                       // Result = StatusBarFaceSpriteInfo[0] (80073E68)
    sw(0, gp + 0xA78);                                  // Store to: gbStatusBarPlayerGotGibbed (80078058)
    sw(0, gp + 0x8EC);                                  // Store to: gbStatusBarIsShowingSpecialFace (80077ECC)
    at = 0x800A0000;                                    // Result = 800A0000
    sw(0, at - 0x78E8);                                 // Store to: gStatusBar[0] (80098718)
    *gStatusBarMsgTicsLeft = 0;
    sw(0, gp + 0xB54);                                  // Store to: gFaceTics (80078134)
    sw(v0, gp + 0xC50);                                 // Store to: gpCurStatusBarFaceSpriteInfo (80078230)
loc_80038658:
    sw(0, v1);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4C;                                       // Result = gStatusBarKeyState[0] (800A94B4)
    at += a0;
    sh(0, at);
    a0 += 8;
    a1++;
    v0 = (i32(a1) < 6);
    v1 += 4;
    if (v0 != 0) goto loc_80038658;
    return;
}

void ST_Ticker() noexcept {
loc_80038688:
    a0 = lw(gp + 0xB54);                                // Load from: gFaceTics (80078134)
    v1 = *gCurPlayerIndex;
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    a0--;
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    sw(a0, gp + 0xB54);                                 // Store to: gFaceTics (80078134)
    s4 = v1 + v0;
    if (i32(a0) > 0) goto loc_80038710;
    _thunk_M_Random();
    v0 &= 0xF;
    sw(v0, gp + 0xB54);                                 // Store to: gFaceTics (80078134)
    _thunk_M_Random();
    v0 &= 3;
    v1 = 3;                                             // Result = 00000003
    sw(v0, gp + 0xA44);                                 // Store to: gStatusBarFaceFrameNum (80078024)
    {
        const bool bJump = (v0 != v1);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8003870C;
    }
    sw(v0, gp + 0xA44);                                 // Store to: gStatusBarFaceFrameNum (80078024)
loc_8003870C:
    sw(0, gp + 0x8EC);                                  // Store to: gbStatusBarIsShowingSpecialFace (80077ECC)
loc_80038710:
    v1 = 0x800A0000;                                    // Result = 800A0000
    v1 -= 0x78E8;                                       // Result = gStatusBar[0] (80098718)
    v0 = lw(v1);                                        // Load from: gStatusBar[0] (80098718)
    if (v0 == 0) goto loc_80038740;
    sw(v0, gp + 0x928);                                 // Store to: gStatusBarCurSpecialFace (80077F08)
    v0 = 0xF;                                           // Result = 0000000F
    sw(v0, gp + 0xB54);                                 // Store to: gFaceTics (80078134)
    v0 = 1;                                             // Result = 00000001
    sw(0, v1);                                          // Store to: gStatusBar[0] (80098718)
    sw(v0, gp + 0x8EC);                                 // Store to: gbStatusBarIsShowingSpecialFace (80077ECC)
loc_80038740:
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 = lw(v0 - 0x78CC);                               // Load from: gStatusBar[7] (80098734)
    {
        const bool bJump = (v0 == 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80038774;
    }
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78C4);                                // Store to: gStatusBarGibAnimTicsLeft (8009873C)
    v0 = 1;                                             // Result = 00000001
    at = 0x800A0000;                                    // Result = 800A0000
    sw(0, at - 0x78C8);                                 // Store to: gStatusBarGibAnimFrame (80098738)
    at = 0x800A0000;                                    // Result = 800A0000
    sw(0, at - 0x78CC);                                 // Store to: gStatusBar[7] (80098734)
    sw(v0, gp + 0xA78);                                 // Store to: gbStatusBarPlayerGotGibbed (80078058)
loc_80038774:
    v0 = lw(gp + 0xA78);                                // Load from: gbStatusBarPlayerGotGibbed (80078058)
    if (v0 == 0) goto loc_800387D4;
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 = lw(v0 - 0x78C4);                               // Load from: gStatusBarGibAnimTicsLeft (8009873C)
    v0--;
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78C4);                                // Store to: gStatusBarGibAnimTicsLeft (8009873C)
    v1 = 2;                                             // Result = 00000002
    if (i32(v0) > 0) goto loc_800387D4;
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 = lw(v0 - 0x78C8);                               // Load from: gStatusBarGibAnimFrame (80098738)
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v1, at - 0x78C4);                                // Store to: gStatusBarGibAnimTicsLeft (8009873C)
    v0++;
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78C8);                                // Store to: gStatusBarGibAnimFrame (80098738)
    v0 = (i32(v0) < 5);
    if (v0 != 0) goto loc_800387D4;
    sw(0, gp + 0xA78);                                  // Store to: gbStatusBarPlayerGotGibbed (80078058)
    sw(0, gp + 0xB50);                                  // Store to: gbDrawStatusBarFace (80078130)
loc_800387D4:
    v0 = lw(s4 + 0xD4);
    if (v0 == 0) goto loc_8003882C;
    v0 = *gCurPlayerIndex;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7740;                                       // Result = gPlayer1[35] (800A88C0)
    at += v0;
    v1 = lw(at);
    v0 = 0x4B;
    *gStatusBarMsgTicsLeft = v0;
    *gpStatusBarMsgStr = v1;
    sw(0, s4 + 0xD4);
loc_8003882C:
    v0 = *gStatusBarMsgTicsLeft;
    s2 = 0;
    if (v0 == 0) goto loc_8003884C;
    v0--;
    *gStatusBarMsgTicsLeft = v0;
loc_8003884C:
    s3 = 4;                                             // Result = 00000004
    s1 = 0x800B0000;                                    // Result = 800B0000
    s1 -= 0x6B4C;                                       // Result = gStatusBarKeyState[0] (800A94B4)
    s0 = 0;                                             // Result = 00000000
loc_8003885C:
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 -= 0x78E4;                                       // Result = gStatusBar[1] (8009871C)
    v1 = s2 << 2;
    v1 += v0;
    v0 = lw(v1);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800388C0;
    }
    sw(0, v1);
    sh(v0, s1);
    v0 = 7;                                             // Result = 00000007
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B48;                                       // Result = gStatusBarKeyState[2] (800A94B8)
    at += s0;
    sh(s3, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B46;                                       // Result = gStatusBarKeyState[3] (800A94BA)
    at += s0;
    sh(v0, at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4A;                                       // Result = gStatusBarKeyState[1] (800A94B6)
    at += s0;
    sh(0, at);
    s1 += 8;
    goto loc_800389AC;
loc_800388C0:
    v0 = lh(s1);
    if (v0 == 0) goto loc_800389A8;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x6B4C;                                       // Result = gStatusBarKeyState[0] (800A94B4)
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B48;                                       // Result = gStatusBarKeyState[2] (800A94B8)
    at += s0;
    v0 = lhu(at);
    a0 = s0 + v1;
    v0--;
    sh(v0, a0 + 0x4);
    v0 <<= 16;
    if (v0 != 0) goto loc_800389A8;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4A;                                       // Result = gStatusBarKeyState[1] (800A94B6)
    at += s0;
    v0 = lhu(at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B46;                                       // Result = gStatusBarKeyState[3] (800A94BA)
    at += s0;
    v1 = lhu(at);
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B48;                                       // Result = gStatusBarKeyState[2] (800A94B8)
    at += s0;
    sh(s3, at);
    v0 ^= 1;
    v1--;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4A;                                       // Result = gStatusBarKeyState[1] (800A94B6)
    at += s0;
    sh(v0, at);
    sh(v1, a0 + 0x6);
    v1 <<= 16;
    if (v1 != 0) goto loc_80038968;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4C;                                       // Result = gStatusBarKeyState[0] (800A94B4)
    at += s0;
    sh(0, at);
loc_80038968:
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4A;                                       // Result = gStatusBarKeyState[1] (800A94B6)
    at += s0;
    v0 = lh(at);
    if (v0 == 0) goto loc_800389A8;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4C;                                       // Result = gStatusBarKeyState[0] (800A94B4)
    at += s0;
    v0 = lh(at);
    a0 = 0;
    if (v0 == 0) goto loc_800389A8;
    a1 = sfx_itemup;
    S_StartSound();
loc_800389A8:
    s1 += 8;
loc_800389AC:
    s2++;
    v0 = (i32(s2) < 6);
    s0 += 8;
    if (v0 != 0) goto loc_8003885C;
    v0 = lw(s4 + 0xC0);
    v0 &= 2;
    v1 = 0x28;                                          // Result = 00000028
    if (v0 != 0) goto loc_800389E0;
    v0 = lw(s4 + 0x30);
    if (v0 == 0) goto loc_800389F0;
loc_800389E0:
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v1, at - 0x78EC);                                // Store to: gStatusBarFaceAnimNum (80098714)
    goto loc_80038AB8;
loc_800389F0:
    v0 = lw(gp + 0xA78);                                // Load from: gbStatusBarPlayerGotGibbed (80078058)
    if (v0 == 0) goto loc_80038A10;
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 = lw(v0 - 0x78C8);                               // Load from: gStatusBarGibAnimFrame (80098738)
    v0 += 0x2A;
    goto loc_80038AB0;
loc_80038A10:
    v1 = lw(s4 + 0x24);
    v0 = 0x29;                                          // Result = 00000029
    if (v1 == 0) goto loc_80038AB0;
    v0 = lw(gp + 0x8EC);                                // Load from: gbStatusBarIsShowingSpecialFace (80077ECC)
    {
        const bool bJump = (v0 == 0);
        v0 = 0x66660000;                                // Result = 66660000
        if (bJump) goto loc_80038A70;
    }
    v0 |= 0x6667;                                       // Result = 66666667
    mult(v1, v0);
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 = u32(i32(v0) >> 3);
    v1 = v0 - v1;
    v0 = (i32(v1) < 4);
    {
        const bool bJump = (v0 != 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_80038A5C;
    }
    v1 = 0;                                             // Result = 00000000
    goto loc_80038A64;
loc_80038A5C:
    v0 -= v1;
    v1 = v0 << 3;
loc_80038A64:
    v0 = lw(gp + 0x928);                                // Load from: gStatusBarCurSpecialFace (80077F08)
    v0 += v1;
    goto loc_80038AB0;
loc_80038A70:
    v0 |= 0x6667;                                       // Result = 66666667
    mult(v1, v0);
    v1 = u32(i32(v1) >> 31);
    v0 = hi;
    v0 = u32(i32(v0) >> 3);
    v1 = v0 - v1;
    v0 = (i32(v1) < 4);
    {
        const bool bJump = (v0 != 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_80038A9C;
    }
    v1 = 0;                                             // Result = 00000000
    goto loc_80038AA4;
loc_80038A9C:
    v0 -= v1;
    v1 = v0 << 3;
loc_80038AA4:
    v0 = lw(gp + 0xA44);                                // Load from: gStatusBarFaceFrameNum (80078024)
    v0 += v1;
loc_80038AB0:
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78EC);                                // Store to: gStatusBarFaceAnimNum (80098714)
loc_80038AB8:
    v0 = 0x800A0000;                                    // Result = 800A0000
    v0 = lw(v0 - 0x78EC);                               // Load from: gStatusBarFaceAnimNum (80098714)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 1;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3E68;                                       // Result = StatusBarFaceSpriteInfo[0] (80073E68)
    v1 += v0;
    sw(v1, gp + 0xC50);                                 // Store to: gpCurStatusBarFaceSpriteInfo (80078230)
    I_UpdatePalette();
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void ST_Drawer() noexcept {
loc_80038B0C:
    sp -= 0x78;
    sw(s0, sp + 0x58);
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    a0 = s0;                                            // Result = 1F800200
    a1 = 0;                                             // Result = 00000000
    a3 = 0x800B0000;                                    // Result = 800B0000
    a3 = lhu(a3 - 0x6B0E);                              // Load from: gTex_STATUS[2] (800A94F2)
    v1 = *gCurPlayerIndex;
    a2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x70);
    sw(s5, sp + 0x6C);
    sw(s4, sp + 0x68);
    sw(s3, sp + 0x64);
    sw(s2, sp + 0x60);
    sw(s1, sp + 0x5C);
    sw(0, sp + 0x10);
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    s2 = v1 + v0;
    _thunk_LIBGPU_SetDrawMode();
    s0 += 4;                                            // Result = 1F800204
    t2 = 0xFF0000;                                      // Result = 00FF0000
    t2 |= 0xFFFF;                                       // Result = 00FFFFFF
    t6 = 0x80080000;                                    // Result = 80080000
    t6 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = t6 & t2;                                       // Result = 00086550
    t5 = 0x4000000;                                     // Result = 04000000
    t4 = 0x80000000;                                    // Result = 80000000
    t3 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t7 = t1 + 4;
loc_80038BB4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80038C18;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_80038E3C;
    v0 = lw(a2);
    at = 0x80070000;                                    // Result = 80070000
    sw(t6, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80038C18:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80038CCC;
    if (v1 == a0) goto loc_80038BB4;
loc_80038C3C:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t5;
    if (v0 == 0) goto loc_80038BB4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t2;
    v0 |= t4;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t3) goto loc_80038CA8;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80038C8C:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_80038C8C;
loc_80038CA8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80038BB4;
    goto loc_80038C3C;
loc_80038CCC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t7;
loc_80038CDC:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a1 = 0xFF0000;                                      // Result = 00FF0000
    a1 |= 0xFFFF;                                       // Result = 00FFFFFF
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= a1;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    v0 = -1;                                            // Result = FFFFFFFF
    a2 += 4;
    if (t0 == v0) goto loc_80038D3C;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80038D24:
    v0 = lw(s0);
    s0 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80038D24;
loc_80038D3C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80038DF4;
    t2 = 0x4000000;                                     // Result = 04000000
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_80038D6C:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_80038DF4;
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
    if (a1 == t0) goto loc_80038DD8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80038DBC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80038DBC;
loc_80038DD8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80038D6C;
loc_80038DF4:
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lhu(v1 - 0x6F5C);                              // Load from: gPaletteClutId_UI (800A90A4)
    a0 = *gStatusBarMsgTicsLeft;
    v0 = 4;                                             // Result = 00000004
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x203);                                 // Store to: 1F800203
    v0 = 0x65;                                          // Result = 00000065
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x207);                                 // Store to: 1F800207
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x20E);                                 // Store to: 1F80020E
    {
        const bool bJump = (i32(a0) <= 0);
        a0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_80038E54;
    }
    a2 = *gpStatusBarMsgStr;
    a1 = 0xC1;
    goto loc_80038E98;
loc_80038E3C:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_80038CDC;
loc_80038E48:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_80039044;
loc_80038E54:
    v0 = lw(s2 + 0x124);
    v0 &= 1;
    a0 = sp + 0x18;
    if (v0 == 0) goto loc_80038EA0;
    a2 = *gGameMap;
    a1 = 0x80010000;                                    // Result = 80010000
    a1 += 0x1628;                                       // Result = STR_LevelNumAndName[0] (80011628)
    a3 = 0x80070000;                                    // Result = 80070000
    a3 += 0x40BC;                                       // Result = StatusBarWeaponBoxesXPos[6] (800740BC)
    v0 = a2 << 5;
    a3 += v0;
    LIBC2_sprintf();
    a0 = 7;                                             // Result = 00000007
    a1 = 0xC1;                                          // Result = 000000C1
    a2 = sp + 0x18;
loc_80038E98:
    I_DrawStringSmall();
loc_80038EA0:
    t3 = 0x1F800000;                                    // Result = 1F800000
    t3 += 0x204;                                        // Result = 1F800204
    t2 = 0xFF0000;                                      // Result = 00FF0000
    t2 |= 0xFFFF;                                       // Result = 00FFFFFF
    t7 = 0x80080000;                                    // Result = 80080000
    t7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = t7 & t2;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0xC8;                                          // Result = 000000C8
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20A);                                 // Store to: 1F80020A
    v0 = 0x100;                                         // Result = 00000100
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    v0 = 0x28;                                          // Result = 00000028
    at = 0x1F800000;                                    // Result = 1F800000
    sh(0, at + 0x208);                                  // Store to: 1F800208
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x20C);                                  // Store to: 1F80020C
    at = 0x1F800000;                                    // Result = 1F800000
    sb(0, at + 0x20D);                                  // Store to: 1F80020D
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    t1 = t0 << 2;
    s0 = t1 + 4;
loc_80038F1C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80038F80;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_80038E48;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(t7, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80038F80:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80039034;
    if (v1 == a0) goto loc_80038F1C;
loc_80038FA4:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_80038F1C;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t2;
    v0 |= t5;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_80039010;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80038FF4:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80038FF4;
loc_80039010:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80038F1C;
    goto loc_80038FA4;
loc_80039034:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += s0;
loc_80039044:
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
    if (t0 == v0) goto loc_800390A4;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003908C:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_8003908C;
loc_800390A4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003915C;
    t2 = 0x4000000;                                     // Result = 04000000
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_800390D4:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8003915C;
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
    if (a1 == t0) goto loc_80039140;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80039124:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80039124;
loc_80039140:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_800390D4;
loc_8003915C:
    s4 = lw(s2 + 0x70);
    v0 = 0xA;                                           // Result = 0000000A
    {
        const bool bJump = (s4 != v0);
        v0 = s4 << 1;
        if (bJump) goto loc_80039178;
    }
    s4 = lw(s2 + 0x6C);
    v0 = s4 << 1;
loc_80039178:
    v0 += s4;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v0;
    a2 = lw(at);
    v0 = 5;                                             // Result = 00000005
    {
        const bool bJump = (a2 != v0);
        v0 = a2 << 2;
        if (bJump) goto loc_800391D4;
    }
    a2 = 0;                                             // Result = 00000000
    goto loc_800391DC;
loc_800391A4:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_800396DC;
loc_800391B0:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_80039CBC;
loc_800391BC:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_80039F84;
loc_800391C8:
    v0 = t1 + 4;
    v0 += a0;
    goto loc_8003A26C;
loc_800391D4:
    v0 += s2;
    a2 = lw(v0 + 0x98);
loc_800391DC:
    a0 = 0x1C;                                          // Result = 0000001C
    a1 = 0xCC;                                          // Result = 000000CC
    I_DrawNumber();
    a0 = 0x47;                                          // Result = 00000047
    a2 = lw(s2 + 0x24);
    a1 = 0xCC;                                          // Result = 000000CC
    I_DrawNumber();
    a0 = 0xA8;                                          // Result = 000000A8
    a2 = lw(s2 + 0x28);
    a1 = 0xCC;                                          // Result = 000000CC
    I_DrawNumber();
    t4 = 0;                                             // Result = 00000000
    s5 = 0x1F800000;                                    // Result = 1F800000
    s5 += 0x200;                                        // Result = 1F800200
    t5 = 0xFF0000;                                      // Result = 00FF0000
    t5 |= 0xFFFF;                                       // Result = 00FFFFFF
    s3 = 0x80080000;                                    // Result = 80080000
    s3 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = s3 & t5;                                       // Result = 00086550
    t9 = 0x4000000;                                     // Result = 04000000
    t8 = 0x80000000;                                    // Result = 80000000
    t7 = -1;                                            // Result = FFFFFFFF
    s0 = 0x72;                                          // Result = 00000072
    t6 = 0;                                             // Result = 00000000
    v0 = 0x64;                                          // Result = 00000064
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x208);                                 // Store to: 1F800208
    v0 = 0xB8;                                          // Result = 000000B8
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20D);                                 // Store to: 1F80020D
    v0 = 0xB;                                           // Result = 0000000B
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    v0 = 8;                                             // Result = 00000008
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
loc_8003926C:
    v0 = t4 << 2;
    v0 += s2;
    v0 = lw(v0 + 0x48);
    t3 = s5 + 4;                                        // Result = 1F800204
    if (v0 != 0) goto loc_800392BC;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4C;                                       // Result = gStatusBarKeyState[0] (800A94B4)
    at += t6;
    v0 = lh(at);
    if (v0 == 0) goto loc_80039508;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x6B4A;                                       // Result = gStatusBarKeyState[1] (800A94B6)
    at += t6;
    v0 = lh(at);
    if (v0 == 0) goto loc_80039508;
loc_800392BC:
    v0 = t4 << 1;
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    at = 0x80070000;                                    // Result = 80070000
    at += 0x40D0;                                       // Result = StatusBarKeyYPos[0] (800740D0)
    at += v0;
    v0 = lhu(at);
    at = 0x1F800000;                                    // Result = 1F800000
    sb(s0, at + 0x20C);                                 // Store to: 1F80020C
    t1 = t0 << 2;
    t2 = t1 + 4;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20A);                                 // Store to: 1F80020A
loc_800392F8:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80039360;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_80039424;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(s3, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80039360:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80039414;
    if (v1 == a0) goto loc_800392F8;
loc_80039384:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_800392F8;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t5;
    v0 |= t8;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t7) goto loc_800393F0;
    a3 = -1;                                            // Result = FFFFFFFF
loc_800393D4:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_800393D4;
loc_800393F0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800392F8;
    goto loc_80039384;
loc_80039414:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_80039424:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t5;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    a2 += 4;
    if (t0 == t7) goto loc_800394EC;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80039460:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80039460;
    goto loc_800394EC;
loc_80039480:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t9;
    if (v0 == 0) goto loc_80039508;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t5;
    v0 |= t8;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t7) goto loc_800394EC;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800394D0:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800394D0;
loc_800394EC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80039480;
loc_80039508:
    s0 += 0xB;
    t4++;
    v0 = (i32(t4) < 6);
    t6 += 8;
    if (v0 != 0) goto loc_8003926C;
    v1 = *gNetGame;
    v0 = 2;
    t2 = 0xFF0000;
    if (v1 == v0) goto loc_80039DD8;
    t3 = 0x1F800000;                                    // Result = 1F800000
    t3 += 0x204;                                        // Result = 1F800204
    t2 |= 0xFFFF;                                       // Result = 00FFFFFF
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = s0 & t2;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0xC8;                                          // Result = 000000C8
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x208);                                 // Store to: 1F800208
    v0 = 0xCD;                                          // Result = 000000CD
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20A);                                 // Store to: 1F80020A
    v0 = 0xB4;                                          // Result = 000000B4
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
    v0 = 0xB8;                                          // Result = 000000B8
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20D);                                 // Store to: 1F80020D
    v0 = 0x33;                                          // Result = 00000033
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    v0 = 0x17;                                          // Result = 00000017
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    t1 = t0 << 2;
    t7 = t1 + 4;
loc_800395B4:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80039618;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_800391A4;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80039618:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_800396CC;
    if (v1 == a0) goto loc_800395B4;
loc_8003963C:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_800395B4;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t2;
    v0 |= t5;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_800396A8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003968C:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003968C;
loc_800396A8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800395B4;
    goto loc_8003963C;
loc_800396CC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t7;
loc_800396DC:
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
    if (t0 == v0) goto loc_8003973C;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80039724:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_80039724;
loc_8003973C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t4 = 2;                                             // Result = 00000002
    if (v1 == v0) goto loc_800397F4;
    t2 = 0x4000000;                                     // Result = 04000000
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003976C:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t2;
    t4 = 2;                                             // Result = 00000002
    if (v0 == 0) goto loc_800397F4;
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
    if (a1 == t0) goto loc_800397D8;
    a2 = -1;                                            // Result = FFFFFFFF
loc_800397BC:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_800397BC;
loc_800397D8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t4 = 2;                                             // Result = 00000002
    if (v1 != v0) goto loc_8003976C;
loc_800397F4:
    s3 = 0x1F800000;                                    // Result = 1F800000
    s3 += 0x200;                                        // Result = 1F800200
    t5 = 0xFF0000;                                      // Result = 00FF0000
    t5 |= 0xFFFF;                                       // Result = 00FFFFFF
    s1 = 0x80080000;                                    // Result = 80080000
    s1 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    t9 = s1 & t5;                                       // Result = 00086550
    t8 = 0x4000000;                                     // Result = 04000000
    s0 = 0x80000000;                                    // Result = 80000000
    t6 = -1;                                            // Result = FFFFFFFF
    t7 = -0x18;                                         // Result = FFFFFFE8
    v0 = 0xB8;                                          // Result = 000000B8
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20D);                                 // Store to: 1F80020D
    v0 = 4;                                             // Result = 00000004
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    v0 = 6;                                             // Result = 00000006
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
loc_80039844:
    v0 = t4 << 2;
    v0 += s2;
    v0 = lw(v0 + 0x74);
    t3 = s3 + 4;                                        // Result = 1F800204
    if (v0 == 0) goto loc_80039AC8;
    v0 = t4 << 1;
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    at = 0x80070000;                                    // Result = 80070000
    at += 0x40B0;                                       // Result = StatusBarWeaponBoxesXPos[0] (800740B0)
    at += v0;
    v1 = lhu(at);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 += 5;
    t1 = t0 << 2;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x208);                                 // Store to: 1F800208
    at = 0x80070000;                                    // Result = 80070000
    at += 0x40C0;                                       // Result = StatusBarWeaponBoxesYPos[0] (800740C0)
    at += v0;
    v0 = lhu(at);
    t2 = t1 + 4;
    at = 0x1F800000;                                    // Result = 1F800000
    sb(t7, at + 0x20C);                                 // Store to: 1F80020C
    v0 += 3;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20A);                                 // Store to: 1F80020A
loc_800398B8:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80039920;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_800399E4;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(s1, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= t9;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80039920:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_800399D4;
    if (v1 == a0) goto loc_800398B8;
loc_80039944:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t8;
    if (v0 == 0) goto loc_800398B8;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t5;
    v0 |= s0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t6) goto loc_800399B0;
    a3 = -1;                                            // Result = FFFFFFFF
loc_80039994:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_80039994;
loc_800399B0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_800398B8;
    goto loc_80039944;
loc_800399D4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_800399E4:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    a0 = 0xFF000000;                                    // Result = FF000000
    v1 = lw(a2);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v1 &= a0;
    v0 &= t5;
    v1 |= v0;
    sw(v1, a2);
    sb(t0, a2 + 0x3);
    t0--;
    a2 += 4;
    if (t0 == t6) goto loc_80039AAC;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80039A20:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80039A20;
    goto loc_80039AAC;
loc_80039A40:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t8;
    if (v0 == 0) goto loc_80039AC8;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t5;
    v0 |= s0;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t6) goto loc_80039AAC;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80039A90:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80039A90;
loc_80039AAC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80039A40;
loc_80039AC8:
    t4++;
    v0 = (i32(t4) < 8);
    t7 += 4;
    if (v0 != 0) goto loc_80039844;
    t2 = 0x1F800000;                                    // Result = 1F800000
    t2 += 0x204;                                        // Result = 1F800204
    t3 = 0xFF0000;                                      // Result = 00FF0000
    t3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t7 = 0x80080000;                                    // Result = 80080000
    t7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = t7 & t3;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x408C;                                       // Result = WeaponNumbers[0] (8007408C)
    v1 = s4 << 2;
    v1 += v0;
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    v0 = lw(v1);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 <<= 1;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x40B0;                                       // Result = StatusBarWeaponBoxesXPos[0] (800740B0)
    at += v0;
    v0 = lhu(at);
    t1 = t0 << 2;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x208);                                 // Store to: 1F800208
    v0 = lw(v1);
    s0 = t1 + 4;
    v0 <<= 1;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x40C0;                                       // Result = StatusBarWeaponBoxesYPos[0] (800740C0)
    at += v0;
    v1 = lhu(at);
    v0 = 0xA4;                                          // Result = 000000A4
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
    v0 = 0xC0;                                          // Result = 000000C0
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20D);                                 // Store to: 1F80020D
    v0 = 0xC;                                           // Result = 0000000C
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v1, at + 0x20A);                                 // Store to: 1F80020A
loc_80039B94:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80039BF8;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_800391B0;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(t7, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80039BF8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80039CAC;
    if (v1 == a0) goto loc_80039B94;
loc_80039C1C:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_80039B94;
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
    if (a1 == t4) goto loc_80039C88;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80039C6C:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80039C6C;
loc_80039C88:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80039B94;
    goto loc_80039C1C;
loc_80039CAC:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += s0;
loc_80039CBC:
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
    if (t0 == v0) goto loc_80039D1C;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80039D04:
    v0 = lw(t2);
    t2 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_80039D04;
loc_80039D1C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_8003A0A8;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_80039D48:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8003A0A8;
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
    if (a1 == t0) goto loc_80039DB4;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80039D98:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80039D98;
loc_80039DB4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003A0A8;
    goto loc_80039D48;
loc_80039DD8:
    t3 = 0x1F800000;                                    // Result = 1F800000
    t3 += 0x204;                                        // Result = 1F800204
    t2 |= 0xFFFF;                                       // Result = 00FFFFFF
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    s1 = s0 & t2;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0xD1;                                          // Result = 000000D1
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x208);                                 // Store to: 1F800208
    v0 = 0xDD;                                          // Result = 000000DD
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20A);                                 // Store to: 1F80020A
    v0 = 0xD0;                                          // Result = 000000D0
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
    v0 = 0xF3;                                          // Result = 000000F3
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20D);                                 // Store to: 1F80020D
    v0 = 0x21;                                          // Result = 00000021
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    v0 = 8;                                             // Result = 00000008
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
    t1 = t0 << 2;
    t7 = t1 + 4;
loc_80039E5C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_80039EC0;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_800391BC;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_80039EC0:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80039F74;
    if (v1 == a0) goto loc_80039E5C;
loc_80039EE4:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_80039E5C;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t2;
    v0 |= t5;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_80039F50;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80039F34:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80039F34;
loc_80039F50:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80039E5C;
    goto loc_80039EE4;
loc_80039F74:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t7;
loc_80039F84:
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
    if (t0 == v0) goto loc_80039FE4;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80039FCC:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_80039FCC;
loc_80039FE4:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_8003A098;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003A010:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8003A098;
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
    if (a1 == t0) goto loc_8003A07C;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003A060:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003A060;
loc_8003A07C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003A010;
loc_8003A098:
    a0 = 0xE1;                                          // Result = 000000E1
    a2 = lw(s2 + 0x64);
    a1 = 0xCC;                                          // Result = 000000CC
    I_DrawNumber();
loc_8003A0A8:
    v0 = lw(gp + 0xB50);                                // Load from: gbDrawStatusBarFace (80078130)
    t6 = 0x4000000;                                     // Result = 04000000
    if (v0 == 0) goto loc_8003A384;
    t3 = 0x1F800000;                                    // Result = 1F800000
    t3 += 0x204;                                        // Result = 1F800204
    t2 = 0xFF0000;                                      // Result = 00FF0000
    t2 |= 0xFFFF;                                       // Result = 00FFFFFF
    t7 = 0x80080000;                                    // Result = 80080000
    t7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    v1 = lw(gp + 0xC50);                                // Load from: gpCurStatusBarFaceSpriteInfo (80078230)
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = lbu(v1);
    s1 = t7 & t2;                                       // Result = 00086550
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x208);                                 // Store to: 1F800208
    v0 = lbu(v1 + 0x1);
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x20A);                                 // Store to: 1F80020A
    v0 = lbu(v1 + 0x2);
    t5 = 0x80000000;                                    // Result = 80000000
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20C);                                 // Store to: 1F80020C
    v0 = lbu(v1 + 0x3);
    t4 = -1;                                            // Result = FFFFFFFF
    at = 0x1F800000;                                    // Result = 1F800000
    sb(v0, at + 0x20D);                                 // Store to: 1F80020D
    v0 = lbu(v1 + 0x4);
    t1 = t0 << 2;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x210);                                 // Store to: 1F800210
    v0 = lbu(v1 + 0x5);
    s0 = t1 + 4;
    at = 0x1F800000;                                    // Result = 1F800000
    sh(v0, at + 0x212);                                 // Store to: 1F800212
loc_8003A144:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8003A1A8;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    v1 = 0xFF000000;                                    // Result = FF000000
    if (v0 != 0) goto loc_800391C8;
    v0 = lw(a3);
    at = 0x80070000;                                    // Result = 80070000
    sw(t7, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= s1;
    sw(v0, a3);
    sb(0, a3 + 0x3);
    a3 = 0x80070000;                                    // Result = 80070000
    a3 = lw(a3 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003A1A8:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_8003A25C;
    if (v1 == a0) goto loc_8003A144;
loc_8003A1CC:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_8003A144;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    a1 = lbu(a0 + 0x3);
    v0 = lw(a0);
    a1--;
    v0 &= t2;
    v0 |= t5;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7C14);                                // Store to: gpGpuPrimsBeg (80077C14)
    a0 += 4;
    if (a1 == t4) goto loc_8003A238;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003A21C:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003A21C;
loc_8003A238:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003A144;
    goto loc_8003A1CC;
loc_8003A25C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += s0;
loc_8003A26C:
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
    if (t0 == v0) goto loc_8003A2CC;
    v1 = -1;                                            // Result = FFFFFFFF
loc_8003A2B4:
    v0 = lw(t3);
    t3 += 4;
    t0--;
    sw(v0, a3);
    a3 += 4;
    if (t0 != v1) goto loc_8003A2B4;
loc_8003A2CC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_8003A384;
    t2 = 0x4000000;                                     // Result = 04000000
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_8003A2FC:
    v0 = lw(gp + 0x700);                                // Load from: GPU_REG_GP1 (80077CE0)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_8003A384;
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
    if (a1 == t0) goto loc_8003A368;
    a2 = -1;                                            // Result = FFFFFFFF
loc_8003A34C:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x6FC);                                // Load from: GPU_REG_GP0 (80077CDC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_8003A34C;
loc_8003A368:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_8003A2FC;
loc_8003A384:
    v0 = *gbGamePaused;
    if (v0 == 0) goto loc_8003A3A0;
    I_DrawPausedOverlay();
loc_8003A3A0:
    ra = lw(sp + 0x70);
    s5 = lw(sp + 0x6C);
    s4 = lw(sp + 0x68);
    s3 = lw(sp + 0x64);
    s2 = lw(sp + 0x60);
    s1 = lw(sp + 0x5C);
    s0 = lw(sp + 0x58);
    sp += 0x78;
    return;
}
