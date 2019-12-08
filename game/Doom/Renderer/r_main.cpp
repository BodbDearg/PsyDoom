#include "r_main.h"

#include "Doom/Base/i_main.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBGPU.h"
#include "PsyQ/LIBGTE.h"
#include "r_bsp.h"
#include "r_data.h"
#include "r_draw.h"
#include "r_sky.h"
#include "r_things.h"

void R_Init() noexcept {
loc_800305B0:
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    R_InitData();
    s0 = 0x80080000;                                    // Result = 80080000
    s0 += 0x6544;                                       // Result = 80086544
    s1 = s0 - 0x14;                                     // Result = gViewSinDiv16 (80086530)
    sw(0, s0);                                          // Store to: 80086544
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x6548);                                 // Store to: 80086548
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at + 0x654C);                                 // Store to: 8008654C
    a0 = s1;                                            // Result = gViewSinDiv16 (80086530)
    LIBGTE_SetTransMatrix();
    a0 = s1;                                            // Result = gViewSinDiv16 (80086530)
    v0 = 0x1000;                                        // Result = 00001000
    sh(0, s0 - 0x14);                                   // Store to: gViewSinDiv16 (80086530)
    sh(0, s0 - 0x12);                                   // Store to: gViewSinDiv16 + 2 (80086532) (80086532)
    sh(0, s0 - 0x10);                                   // Store to: gMinusViewCosDiv16 (80086534)
    sh(0, s0 - 0xE);                                    // Store to: gMinusViewCosDiv16 + 2 (80086536) (80086536)
    sh(v0, s0 - 0xC);                                   // Store to: 80086538
    sh(0, s0 - 0xA);                                    // Store to: 8008653A
    sh(0, s0 - 0x8);                                    // Store to: gViewCosDiv16 (8008653C)
    sh(0, s0 - 0x6);                                    // Store to: gViewCosDiv16 + 2 (8008653E) (8008653E)
    sh(0, s0 - 0x4);                                    // Store to: gViewSinDiv16_2 (80086540)
    LIBGTE_SetRotMatrix();
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void R_RenderPlayerView() noexcept {
loc_80030634:
    v0 = lw(gp + 0xC84);                                // Load from: gbRenderViewFullbright (80078264)
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s0, sp + 0x18);
    if (v0 != 0) goto loc_80030660;
    v1 = lw(gp + 0xA88);                                // Load from: gpLightsLump (80078068)
    v0 = 0x80;                                          // Result = 00000080
    sw(v0, gp + 0x990);                                 // Store to: gCurLightValB (80077F70)
    sw(v0, gp + 0xA54);                                 // Store to: gCurLightValG (80078034)
    sw(v0, gp + 0x8AC);                                 // Store to: gCurLightValR (80077E8C)
    sw(v1, gp + 0xA74);                                 // Store to: gpCurLightsLumpEntry (80078054)
loc_80030660:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7618);                               // Load from: gCurPlayerIndex (80077618)
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v0 = v1 << 2;
    v0 += v1;
    v1 = v0 << 4;
    v1 -= v0;
    v1 <<= 2;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v1 += v0;
    v0 = lw(v1 + 0x14);
    a1 = lw(v1);
    a3 = 0xFFFF0000;                                    // Result = FFFF0000
    sw(v1, gp + 0x954);                                 // Store to: gpViewPlayer (80077F34)
    t0 = lw(a1 + 0x24);
    v0 &= a3;
    sw(v0, gp + 0x90C);                                 // Store to: gViewZ (80077EEC)
    v0 = lw(v1);
    a1 = t0 >> 19;
    a1 <<= 2;
    a2 += a1;
    v0 = lw(v0);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += a1;
    a1 = lw(at);
    v0 &= a3;
    sw(v0, gp + 0x900);                                 // Store to: gViewX (80077EE0)
    v0 = lw(v1);
    v1 = lw(a2);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 += 0x6530;                                       // Result = gViewSinDiv16 (80086530)
    sw(t0, gp + 0xCB4);                                 // Store to: gViewAngle (80078294)
    sw(a1, gp + 0xAD8);                                 // Store to: gViewSin (800780B8)
    v0 = lw(v0 + 0x4);
    a1 = u32(i32(a1) >> 4);
    sw(v1, gp + 0xABC);                                 // Store to: gViewCos (8007809C)
    sh(a1, a0);                                         // Store to: gViewSinDiv16 (80086530)
    at = 0x80080000;                                    // Result = 80080000
    sh(a1, at + 0x6540);                                // Store to: gViewSinDiv16_2 (80086540)
    v0 &= a3;
    sw(v0, gp + 0x904);                                 // Store to: gViewY (80077EE4)
    v0 = -v1;
    v0 = u32(i32(v0) >> 4);
    v1 = u32(i32(v1) >> 4);
    at = 0x80080000;                                    // Result = 80080000
    sh(v0, at + 0x6534);                                // Store to: gMinusViewCosDiv16 (80086534)
    at = 0x80080000;                                    // Result = 80080000
    sh(v1, at + 0x653C);                                // Store to: gViewCosDiv16 (8008653C)
    LIBGTE_SetRotMatrix();
    R_BSP();
    v0 = lw(gp + 0xA84);                                // Load from: gppEndDrawSubsector (80078064)
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6E4C;                                       // Result = gpDrawSubsectors[0] (800A91B4)
    v0 -= s0;
    v0 = u32(i32(v0) >> 2);
    sw(v0, gp + 0xB0C);                                 // Store to: gNumDrawSubsectors (800780EC)
    I_DrawPresent();
    v0 = lw(gp + 0xC14);                                // Load from: gbIsSkyVisible (800781F4)
    if (v0 == 0) goto loc_80030774;
    R_DrawSky();
loc_80030774:
    v1 = lw(gp + 0xA84);                                // Load from: gppEndDrawSubsector (80078064)
    v0 = s0 - 4;                                        // Result = gCheatSequenceBtns[6] (800A91B0)
    v1 -= 4;
    v0 = (v0 < v1);
    sw(v1, gp + 0xA84);                                 // Store to: gppEndDrawSubsector (80078064)
    s0 = 0xFF;                                          // Result = 000000FF
    if (v0 == 0) goto loc_8003089C;
loc_80030790:
    v0 = lw(gp + 0xA84);                                // Load from: gppEndDrawSubsector (80078064)
    t0 = lw(v0);
    v0 = lw(gp + 0xC84);                                // Load from: gbRenderViewFullbright (80078264)
    a1 = lw(t0);
    sw(a1, gp + 0xA2C);                                 // Store to: gpCurSector (8007800C)
    if (v0 == 0) goto loc_80030874;
    v1 = lh(a1 + 0x10);
    v0 = lw(gp + 0xA88);                                // Load from: gpLightsLump (80078068)
    a0 = lh(a1 + 0x12);
    v1 <<= 2;
    v1 += v0;
    sw(v1, gp + 0xA74);                                 // Store to: gpCurLightsLumpEntry (80078054)
    v0 = lbu(v1);
    mult(a0, v0);
    a0 = lh(a1 + 0x12);
    v0 = lo;
    a3 = u32(i32(v0) >> 8);
    sw(a3, gp + 0x8AC);                                 // Store to: gCurLightValR (80077E8C)
    v0 = lbu(v1 + 0x1);
    mult(a0, v0);
    a0 = lh(a1 + 0x12);
    v0 = lo;
    a2 = u32(i32(v0) >> 8);
    sw(a2, gp + 0xA54);                                 // Store to: gCurLightValG (80078034)
    v0 = lbu(v1 + 0x2);
    mult(a0, v0);
    a0 = lw(gp + 0x954);                                // Load from: gpViewPlayer (80077F34)
    v1 = lw(a0 + 0xE4);
    v0 = lo;
    a1 = u32(i32(v0) >> 8);
    sw(a1, gp + 0x990);                                 // Store to: gCurLightValB (80077F70)
    v0 = a3 + v1;
    if (v1 == 0) goto loc_80030874;
    sw(v0, gp + 0x8AC);                                 // Store to: gCurLightValR (80077E8C)
    v0 = (i32(v0) < 0x100);
    v1 = lw(a0 + 0xE4);
    a0 = lw(a0 + 0xE4);
    v1 += a2;
    a0 += a1;
    sw(v1, gp + 0xA54);                                 // Store to: gCurLightValG (80078034)
    sw(a0, gp + 0x990);                                 // Store to: gCurLightValB (80077F70)
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(v1) < 0x100);
        if (bJump) goto loc_8003085C;
    }
    sw(s0, gp + 0x8AC);                                 // Store to: gCurLightValR (80077E8C)
loc_8003085C:
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a0) < 0x100);
        if (bJump) goto loc_80030868;
    }
    sw(s0, gp + 0xA54);                                 // Store to: gCurLightValG (80078034)
loc_80030868:
    if (v0 != 0) goto loc_80030874;
    sw(s0, gp + 0x990);                                 // Store to: gCurLightValB (80077F70)
loc_80030874:
    a0 = t0;
    R_DrawSubsector();
    v1 = lw(gp + 0xA84);                                // Load from: gppEndDrawSubsector (80078064)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x6E50;                                       // Result = gCheatSequenceBtns[6] (800A91B0)
    v1 -= 4;
    v0 = (v0 < v1);
    sw(v1, gp + 0xA84);                                 // Store to: gppEndDrawSubsector (80078064)
    if (v0 != 0) goto loc_80030790;
loc_8003089C:
    R_DrawWeapon();
    s0 = 0x1F800000;                                    // Result = 1F800000
    s0 += 0x200;                                        // Result = 1F800200
    a0 = s0;                                            // Result = 1F800200
    a1 = sp + 0x10;
    sh(0, sp + 0x10);
    sh(0, sp + 0x12);
    sh(0, sp + 0x14);
    sh(0, sp + 0x16);
    LIBGPU_SetTexWindow();
    s0 += 4;                                            // Result = 1F800204
    t3 = 0xFF0000;                                      // Result = 00FF0000
    t3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t7 = 0x80080000;                                    // Result = 80080000
    t7 += 0x6550;                                       // Result = gGpuCmdsBuffer[0] (80086550)
    t8 = t7 & t3;                                       // Result = 00086550
    t6 = 0x4000000;                                     // Result = 04000000
    t5 = 0x80000000;                                    // Result = 80000000
    t4 = -1;                                            // Result = FFFFFFFF
    t0 = 0x1F800000;                                    // Result = 1F800000
    t0 = lbu(t0 + 0x203);                               // Load from: 1F800203
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t1 = t0 << 2;
    t2 = t1 + 4;
loc_80030904:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = t1 + a0;
        if (bJump) goto loc_8003096C;
    }
    v0 += 4;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 != 0);
        v0 = t2 + a0;
        if (bJump) goto loc_80030A30;
    }
    v0 = lw(a2);
    v1 = 0xFF000000;                                    // Result = FF000000
    at = 0x80070000;                                    // Result = 80070000
    sw(t7, at + 0x7C18);                                // Store to: gpGpuPrimsEnd (80077C18)
    v0 &= v1;
    v0 |= t8;
    sw(v0, a2);
    sb(0, a2 + 0x3);
    a2 = 0x80070000;                                    // Result = 80070000
    a2 = lw(a2 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
loc_8003096C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = t1 + a0;
    v0 += 4;
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80030A20;
    if (v1 == a0) goto loc_80030904;
loc_80030990:
    v0 = lw(gp + 0x5E0);                                // Load from: GPU_REG_GP1 (80077BC0)
    v0 = lw(v0);
    v0 &= t6;
    if (v0 == 0) goto loc_80030904;
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
    if (a1 == t4) goto loc_800309FC;
    a3 = -1;                                            // Result = FFFFFFFF
loc_800309E0:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x5DC);                                // Load from: GPU_REG_GP0 (80077BBC)
    a1--;
    sw(v1, v0);
    if (a1 != a3) goto loc_800309E0;
loc_800309FC:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 == v0) goto loc_80030904;
    goto loc_80030990;
loc_80030A20:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    v0 += t2;
loc_80030A30:
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
    if (t0 == v0) goto loc_80030A90;
    v1 = -1;                                            // Result = FFFFFFFF
loc_80030A78:
    v0 = lw(s0);
    s0 += 4;
    t0--;
    sw(v0, a2);
    a2 += 4;
    if (t0 != v1) goto loc_80030A78;
loc_80030A90:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    t2 = 0x4000000;                                     // Result = 04000000
    if (v1 == v0) goto loc_80030B44;
    a3 = 0xFF0000;                                      // Result = 00FF0000
    a3 |= 0xFFFF;                                       // Result = 00FFFFFF
    t1 = 0x80000000;                                    // Result = 80000000
    t0 = -1;                                            // Result = FFFFFFFF
loc_80030ABC:
    v0 = lw(gp + 0x5E0);                                // Load from: GPU_REG_GP1 (80077BC0)
    v0 = lw(v0);
    v0 &= t2;
    if (v0 == 0) goto loc_80030B44;
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
    if (a1 == t0) goto loc_80030B28;
    a2 = -1;                                            // Result = FFFFFFFF
loc_80030B0C:
    v1 = lw(a0);
    a0 += 4;
    v0 = lw(gp + 0x5DC);                                // Load from: GPU_REG_GP0 (80077BBC)
    a1--;
    sw(v1, v0);
    if (a1 != a2) goto loc_80030B0C;
loc_80030B28:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7C14);                               // Load from: gpGpuPrimsBeg (80077C14)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7C18);                               // Load from: gpGpuPrimsEnd (80077C18)
    if (v1 != v0) goto loc_80030ABC;
loc_80030B44:
    ra = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x20;
    return;
}

void R_SlopeDiv() noexcept {
    v0 = (a1 < 0x200);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030B98;
    }
    v1 = a0 << 3;
    v0 = a1 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030B7C;
    _break(0x1C00);
loc_80030B7C:
    v1 = lo;
    v0 = (v1 < 0x801);
    {
        const bool bJump = (v0 != 0);
        v0 = v1;
        if (bJump) goto loc_80030B98;
    }
    v1 = 0x800;                                         // Result = 00000800
    v0 = v1;                                            // Result = 00000800
loc_80030B98:
    return;
}

void R_PointToAngle2() noexcept {
loc_80030BA0:
    a2 -= a0;
    a3 -= a1;
    if (a2 != 0) goto loc_80030BB4;
    v0 = 0;                                             // Result = 00000000
    if (a3 == 0) goto loc_80030EAC;
loc_80030BB4:
    if (i32(a2) < 0) goto loc_80030D30;
    v0 = (i32(a3) < i32(a2));
    if (i32(a3) < 0) goto loc_80030C68;
    {
        const bool bJump = (v0 == 0);
        v0 = (a2 < 0x200);
        if (bJump) goto loc_80030C24;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030C08;
    }
    v1 = a3 << 3;
    v0 = a2 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030BEC;
    _break(0x1C00);
loc_80030BEC:
    v1 = lo;
    v0 = (v1 < 0x801);
    {
        const bool bJump = (v0 != 0);
        v0 = v1;
        if (bJump) goto loc_80030C08;
    }
    v1 = 0x800;                                         // Result = 00000800
    v0 = v1;                                            // Result = 00000800
loc_80030C08:
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v0;
    v0 = lw(at);
    goto loc_80030EAC;
loc_80030C24:
    v0 = (a3 < 0x200);
    v1 = 0x800;                                         // Result = 00000800
    if (v0 != 0) goto loc_80030C60;
    v1 = a2 << 3;
    v0 = a3 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030C48;
    _break(0x1C00);
loc_80030C48:
    v1 = lo;
    v0 = (v1 < 0x801);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x3FFF0000;                                // Result = 3FFF0000
        if (bJump) goto loc_80030E90;
    }
    v1 = 0x800;                                         // Result = 00000800
loc_80030C60:
    v0 = 0x3FFF0000;                                    // Result = 3FFF0000
    goto loc_80030E90;
loc_80030C68:
    a3 = -a3;
    v0 = (i32(a3) < i32(a2));
    {
        const bool bJump = (v0 == 0);
        v0 = (a2 < 0x200);
        if (bJump) goto loc_80030CD0;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030CB4;
    }
    v1 = a3 << 3;
    v0 = a2 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030C98;
    _break(0x1C00);
loc_80030C98:
    v1 = lo;
    v0 = (v1 < 0x801);
    if (v0 != 0) goto loc_80030CB0;
    v1 = 0x800;                                         // Result = 00000800
loc_80030CB0:
    v0 = v1;
loc_80030CB4:
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v0;
    v0 = lw(at);
    v0 = -v0;
    goto loc_80030EAC;
loc_80030CD0:
    v0 = (a3 < 0x200);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030D10;
    }
    v1 = a2 << 3;
    v0 = a3 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030CF4;
    _break(0x1C00);
loc_80030CF4:
    v1 = lo;
    v0 = (v1 < 0x801);
    if (v0 != 0) goto loc_80030D0C;
    v1 = 0x800;                                         // Result = 00000800
loc_80030D0C:
    v0 = v1;
loc_80030D10:
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v0;
    v1 = lw(at);
    v0 = 0xC0000000;                                    // Result = C0000000
    v0 += v1;
    goto loc_80030EAC;
loc_80030D30:
    a2 = -a2;
    if (i32(a3) < 0) goto loc_80030DE4;
    v0 = (i32(a3) < i32(a2));
    {
        const bool bJump = (v0 == 0);
        v0 = (a2 < 0x200);
        if (bJump) goto loc_80030D84;
    }
    v1 = 0x800;                                         // Result = 00000800
    if (v0 != 0) goto loc_80030D7C;
    v1 = a3 << 3;
    v0 = a2 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030D64;
    _break(0x1C00);
loc_80030D64:
    v1 = lo;
    v0 = (v1 < 0x801);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x7FFF0000;                                // Result = 7FFF0000
        if (bJump) goto loc_80030E90;
    }
    v1 = 0x800;                                         // Result = 00000800
loc_80030D7C:
    v0 = 0x7FFF0000;                                    // Result = 7FFF0000
    goto loc_80030E90;
loc_80030D84:
    v0 = (a3 < 0x200);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030DC4;
    }
    v1 = a2 << 3;
    v0 = a3 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030DA8;
    _break(0x1C00);
loc_80030DA8:
    v1 = lo;
    v0 = (v1 < 0x801);
    if (v0 != 0) goto loc_80030DC0;
    v1 = 0x800;                                         // Result = 00000800
loc_80030DC0:
    v0 = v1;
loc_80030DC4:
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v0;
    v1 = lw(at);
    v0 = 0x40000000;                                    // Result = 40000000
    v0 += v1;
    goto loc_80030EAC;
loc_80030DE4:
    a3 = -a3;
    v0 = (i32(a3) < i32(a2));
    {
        const bool bJump = (v0 == 0);
        v0 = (a2 < 0x200);
        if (bJump) goto loc_80030E50;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x800;                                     // Result = 00000800
        if (bJump) goto loc_80030E30;
    }
    v1 = a3 << 3;
    v0 = a2 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030E14;
    _break(0x1C00);
loc_80030E14:
    v1 = lo;
    v0 = (v1 < 0x801);
    if (v0 != 0) goto loc_80030E2C;
    v1 = 0x800;                                         // Result = 00000800
loc_80030E2C:
    v0 = v1;
loc_80030E30:
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v0;
    v1 = lw(at);
    v0 = 0x80000000;                                    // Result = 80000000
    v0 = v1 - v0;
    goto loc_80030EAC;
loc_80030E50:
    v0 = (a3 < 0x200);
    v1 = 0x800;                                         // Result = 00000800
    if (v0 != 0) goto loc_80030E8C;
    v1 = a2 << 3;
    v0 = a3 >> 8;
    divu(v1, v0);
    if (v0 != 0) goto loc_80030E74;
    _break(0x1C00);
loc_80030E74:
    v1 = lo;
    v0 = (v1 < 0x801);
    {
        const bool bJump = (v0 != 0);
        v0 = 0xBFFF0000;                                // Result = BFFF0000
        if (bJump) goto loc_80030E90;
    }
    v1 = 0x800;                                         // Result = 00000800
loc_80030E8C:
    v0 = 0xBFFF0000;                                    // Result = BFFF0000
loc_80030E90:
    v1 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x1958;                                       // Result = TanToAngle[0] (80071958)
    at += v1;
    v1 = lw(at);
    v0 |= 0xFFFF;
    v0 -= v1;
loc_80030EAC:
    return;
}

void R_PointOnSide() noexcept {
    a3 = lw(a2 + 0x8);
    if (a3 != 0) goto loc_80030EF0;
    v0 = lw(a2);
    v0 = (i32(v0) < i32(a0));
    if (v0 != 0) goto loc_80030EE4;
    v0 = lw(a2 + 0xC);
    v0 = (i32(v0) > 0);
    goto loc_80030F54;
loc_80030EE4:
    v0 = lw(a2 + 0xC);
    v0 >>= 31;
    goto loc_80030F54;
loc_80030EF0:
    v1 = lw(a2 + 0xC);
    if (v1 != 0) goto loc_80030F1C;
    v0 = lw(a2 + 0x4);
    v0 = (i32(v0) < i32(a1));
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a3) > 0);
        if (bJump) goto loc_80030F54;
    }
    v0 = a3 >> 31;
    goto loc_80030F54;
loc_80030F1C:
    v0 = lw(a2);
    v1 = u32(i32(v1) >> 16);
    v0 = a0 - v0;
    v0 = u32(i32(v0) >> 16);
    mult(v1, v0);
    v1 = u32(i32(a3) >> 16);
    v0 = lw(a2 + 0x4);
    a0 = lo;
    v0 = a1 - v0;
    v0 = u32(i32(v0) >> 16);
    mult(v0, v1);
    v0 = lo;
    v0 = (i32(v0) < i32(a0));
    v0 ^= 1;
loc_80030F54:
    return;
}

void R_PointInSubsector() noexcept {
loc_80030F5C:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7E48);                               // Load from: gNumBspNodes (800781B8)
    t0 = a0;
    if (v0 != 0) goto loc_80030F80;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F40);                               // Load from: gpSubsectors (80077F40)
    goto loc_80031080;
loc_80030F80:
    v1 = v0 - 1;
    v0 = v1 & 0x8000;
    {
        const bool bJump = (v0 != 0);
        v0 = 0xFFFF0000;                                // Result = FFFF0000
        if (bJump) goto loc_80031068;
    }
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lw(t1 + 0x7EA4);                               // Load from: gpBspNodes (80077EA4)
    v0 = v1 << 3;
loc_80030F9C:
    v0 -= v1;
    v0 <<= 3;
    a2 = v0 + t1;
    a3 = lw(a2 + 0x8);
    if (a3 != 0) goto loc_80030FE4;
    v0 = lw(a2);
    v0 = (i32(v0) < i32(t0));
    if (v0 != 0) goto loc_80030FD8;
    v0 = lw(a2 + 0xC);
    v0 = (i32(v0) > 0);
    goto loc_80031048;
loc_80030FD8:
    v0 = lw(a2 + 0xC);
    v0 >>= 31;
    goto loc_80031048;
loc_80030FE4:
    v1 = lw(a2 + 0xC);
    if (v1 != 0) goto loc_80031010;
    v0 = lw(a2 + 0x4);
    v0 = (i32(v0) < i32(a1));
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a3) > 0);
        if (bJump) goto loc_80031048;
    }
    v0 = a3 >> 31;
    goto loc_80031048;
loc_80031010:
    v0 = lw(a2);
    v1 = u32(i32(v1) >> 16);
    v0 = t0 - v0;
    v0 = u32(i32(v0) >> 16);
    mult(v1, v0);
    v0 = u32(i32(a3) >> 16);
    v1 = lw(a2 + 0x4);
    a0 = lo;
    v1 = a1 - v1;
    v1 = u32(i32(v1) >> 16);
    mult(v1, v0);
    v0 = lo;
    v0 = (i32(v0) < i32(a0));
    v0 ^= 1;
loc_80031048:
    v0 <<= 2;
    v0 += a2;
    v1 = lw(v0 + 0x30);
    v0 = v1 & 0x8000;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 3;
        if (bJump) goto loc_80030F9C;
    }
    v0 = 0xFFFF0000;                                    // Result = FFFF0000
loc_80031068:
    v0 |= 0x7FFF;                                       // Result = FFFF7FFF
    v0 &= v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7F40);                               // Load from: gpSubsectors (80077F40)
    v0 <<= 4;
    v0 += v1;
loc_80031080:
    return;
}
