#include "p_slide.h"

#include "Doom/Base/m_fixed.h"
#include "PsxVm/PsxVm.h"

void P_SlideMove() noexcept {
loc_8002502C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7ED8);                               // Load from: gpSlideThing (80077ED8)
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    s2 = lw(v0 + 0x48);
    s3 = lw(v0 + 0x4C);
    v1 = lw(v0);
    a0 = lw(v0 + 0x4);
    v0 = lw(v0 + 0x64);
    v0 &= 0x1000;
    sw(v1, gp + 0x9B0);                                 // Store to: gSlideX (80077F90)
    sw(a0, gp + 0x9B4);                                 // Store to: gSlideY (80077F94)
    s0 = 0x10000;                                       // Result = 00010000
    if (v0 == 0) goto loc_800250AC;
    a0 = s0;                                            // Result = 00010000
    goto loc_800250E0;
loc_80025084:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7ED8);                               // Load from: gpSlideThing (80077ED8)
    a0 = lw(v0);
    a1 = lw(v0 + 0x4);
    sw(s2, v0 + 0x48);
    sw(s3, v0 + 0x4C);
    SL_CheckSpecialLines();
    goto loc_80025198;
loc_800250AC:
    s4 = 0;                                             // Result = 00000000
    a0 = s2;
loc_800250B4:
    a1 = s3;
    P_CompletableFrac();
    s0 = v0;
    v0 = 0x10000;                                       // Result = 00010000
    if (s0 == v0) goto loc_800250D0;
    s0 -= 0x1000;
loc_800250D0:
    a0 = s0;
    if (i32(s0) >= 0) goto loc_800250E0;
    s0 = 0;                                             // Result = 00000000
    a0 = s0;                                            // Result = 00000000
loc_800250E0:
    a1 = s2;
    FixedMul();
    a0 = s0;
    a1 = s3;
    s1 = v0;
    FixedMul();
    v1 = lw(gp + 0x9B0);                                // Load from: gSlideX (80077F90)
    a0 = v0;
    a2 = s1 + v1;
    v1 = lw(gp + 0x9B4);                                // Load from: gSlideY (80077F94)
    v0 = 0x10000;                                       // Result = 00010000
    sw(a2, gp + 0x9B0);                                 // Store to: gSlideX (80077F90)
    a3 = a0 + v1;
    sw(a3, gp + 0x9B4);                                 // Store to: gSlideY (80077F94)
    if (s0 == v0) goto loc_80025084;
    s3 -= a0;
    a0 = s2 - s1;
    a1 = lw(gp + 0xBC8);                                // Load from: gBlockNvx (800781A8)
    s4++;
    FixedMul();
    s0 = v0;
    a1 = lw(gp + 0xBD0);                                // Load from: gBlockNvy (800781B0)
    a0 = s3;
    FixedMul();
    s0 += v0;
    a1 = lw(gp + 0xBC8);                                // Load from: gBlockNvx (800781A8)
    a0 = s0;
    FixedMul();
    s2 = v0;
    a1 = lw(gp + 0xBD0);                                // Load from: gBlockNvy (800781B0)
    a0 = s0;
    FixedMul();
    s3 = v0;
    v0 = (i32(s4) < 3);
    a0 = s2;
    if (v0 != 0) goto loc_800250B4;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7ED8);                               // Load from: gpSlideThing (80077ED8)
    v1 = lw(v0);
    a0 = lw(v0 + 0x4);
    sw(0, v0 + 0x4C);
    sw(0, v0 + 0x48);
    sw(v1, gp + 0x9B0);                                 // Store to: gSlideX (80077F90)
    sw(a0, gp + 0x9B4);                                 // Store to: gSlideY (80077F94)
loc_80025198:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_CompletableFrac() noexcept {
loc_800251BC:
    sp -= 0x40;
    a2 = a0;
    v0 = 0x10000;                                       // Result = 00010000
    v1 = lw(gp + 0x9B4);                                // Load from: gSlideY (80077F94)
    a0 = 0x170000;                                      // Result = 00170000
    sw(ra, sp + 0x3C);
    sw(s6, sp + 0x38);
    sw(s5, sp + 0x34);
    sw(s4, sp + 0x30);
    sw(s3, sp + 0x2C);
    sw(s2, sp + 0x28);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    sw(v0, gp + 0xC48);                                 // Store to: gBlockFrac (80078228)
    sw(a2, gp + 0xA90);                                 // Store to: gSlideDx (80078070)
    sw(a1, gp + 0xA94);                                 // Store to: gSlideDy (80078074)
    v0 = v1 + a0;
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7BF0);                                // Store to: gEndBox[0] (80097BF0)
    v0 = lw(gp + 0x9B0);                                // Load from: gSlideX (80077F90)
    v1 -= a0;
    at = 0x80090000;                                    // Result = 80090000
    sw(v1, at + 0x7BF4);                                // Store to: gEndBox[1] (80097BF4)
    v1 = v0 + a0;
    v0 -= a0;
    at = 0x80090000;                                    // Result = 80090000
    sw(v1, at + 0x7BFC);                                // Store to: gEndBox[3] (80097BFC)
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7BF8);                                // Store to: gEndBox[2] (80097BF8)
    v0 += a2;
    if (i32(a2) <= 0) goto loc_8002524C;
    v0 = a2 + v1;
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7BFC);                                // Store to: gEndBox[3] (80097BFC)
    goto loc_80025254;
loc_8002524C:
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7BF8);                                // Store to: gEndBox[2] (80097BF8)
loc_80025254:
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7BF4;                                       // Result = gEndBox[1] (80097BF4)
    if (i32(a1) <= 0) goto loc_8002526C;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7BF0;                                       // Result = gEndBox[0] (80097BF0)
loc_8002526C:
    v0 = lw(v1);
    v0 += a1;
    sw(v0, v1);
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7BF8);                               // Load from: gEndBox[2] (80097BF8)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E74);                               // Load from: gBlockmapOriginX (8007818C)
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7BFC);                               // Load from: gEndBox[3] (80097BFC)
    v0 -= a0;
    a1 = u32(i32(v0) >> 23);
    v1 -= a0;
    s5 = u32(i32(v1) >> 23);
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7BF4);                               // Load from: gEndBox[1] (80097BF4)
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E6C);                               // Load from: gBlockmapOriginY (80078194)
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7BF0);                               // Load from: gEndBox[0] (80097BF0)
    v0 -= a0;
    s6 = u32(i32(v0) >> 23);
    v1 -= a0;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    s3 = u32(i32(v1) >> 23);
    if (i32(a1) >= 0) goto loc_800252EC;
    a1 = 0;                                             // Result = 00000000
loc_800252EC:
    if (i32(s6) >= 0) goto loc_800252F8;
    s6 = 0;                                             // Result = 00000000
loc_800252F8:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D7C);                               // Load from: gBlockmapWidth (80078284)
    v0 = (i32(s5) < i32(v1));
    if (v0 != 0) goto loc_80025314;
    s5 = v1 - 1;
loc_80025314:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EB8);                               // Load from: gBlockmapHeight (80077EB8)
    v0 = (i32(s3) < i32(v1));
    s2 = a1;
    if (v0 != 0) goto loc_80025330;
    s3 = v1 - 1;
loc_80025330:
    v0 = (i32(s5) < i32(s2));
    if (v0 != 0) goto loc_80025410;
    v0 = (i32(s3) < i32(s6));
loc_80025340:
    s1 = s6;
    if (v0 != 0) goto loc_80025400;
loc_80025348:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D7C);                               // Load from: gBlockmapWidth (80078284)
    mult(s1, v0);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7EC0);                               // Load from: gpBlockmap (80078140)
    v0 = lo;
    v0 += s2;
    v0 <<= 1;
    v0 += v1;
    v0 = lh(v0);
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7F3C);                               // Load from: gpBlockmapLump (800780C4)
    v0 <<= 1;
    s0 = v0 + v1;
    v0 = -1;                                            // Result = FFFFFFFF
    v1 = lh(s0);
    a0 = lhu(s0);
    {
        const bool bJump = (v1 == v0);
        v1 = a0 << 16;
        if (bJump) goto loc_800253F0;
    }
    s4 = -1;                                            // Result = FFFFFFFF
loc_8002539C:
    v1 = u32(i32(v1) >> 16);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EB0);                               // Load from: gpLines (80077EB0)
    v0 <<= 2;
    a0 = v0 + v1;
    v0 = lw(a0 + 0x40);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    s0 += 2;
    if (v0 == v1) goto loc_800253E0;
    sw(v1, a0 + 0x40);
    SL_CheckLine();
loc_800253E0:
    v0 = lh(s0);
    a0 = lhu(s0);
    v1 = a0 << 16;
    if (v0 != s4) goto loc_8002539C;
loc_800253F0:
    s1++;
    v0 = (i32(s3) < i32(s1));
    if (v0 == 0) goto loc_80025348;
loc_80025400:
    s2++;
    v0 = (i32(s5) < i32(s2));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s3) < i32(s6));
        if (bJump) goto loc_80025340;
    }
loc_80025410:
    v1 = lw(gp + 0xC48);                                // Load from: gBlockFrac (80078228)
    v0 = (i32(v1) < 0x1000);
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8002542C;
    }
    v0 = v1;
    goto loc_80025434;
loc_8002542C:
    sw(0, gp + 0xC48);                                  // Store to: gBlockFrac (80078228)
    sw(0, gp + 0x9BC);                                  // Store to: gpSpecialLine (80077F9C)
loc_80025434:
    ra = lw(sp + 0x3C);
    s6 = lw(sp + 0x38);
    s5 = lw(sp + 0x34);
    s4 = lw(sp + 0x30);
    s3 = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x40;
    return;
}

void SL_PointOnSide() noexcept {
    v0 = lw(gp + 0xAF4);                                // Load from: gP1y (800780D4)
    sp -= 0x20;
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    s0 = a1 - v0;
    v0 = lw(gp + 0xAEC);                                // Load from: gP1x (800780CC)
    a1 = lw(gp + 0xB78);                                // Load from: gNvx (80078158)
    a0 -= v0;
    FixedMul();
    s1 = v0;
    a1 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    a0 = s0;
    FixedMul();
    s1 += v0;
    v0 = 0x10000;                                       // Result = 00010000
    v0 = (i32(v0) < i32(s1));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800254B8;
    }
    v0 = 0xFFFF0000;                                    // Result = FFFF0000
    v0 = (i32(s1) < i32(v0));
    v0 = -v0;
loc_800254B8:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void SL_CrossFrac() noexcept {
    a1 = lw(gp + 0xB78);                                // Load from: gNvx (80078158)
    a2 = lw(gp + 0xAFC);                                // Load from: gP3x (800780DC)
    v1 = lw(gp + 0xB14);                                // Load from: gP3y (800780F4)
    v0 = lw(gp + 0xAF4);                                // Load from: gP1y (800780D4)
    a0 = lw(gp + 0xAEC);                                // Load from: gP1x (800780CC)
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    s1 = v1 - v0;
    a0 = a2 - a0;
    FixedMul();
    s2 = v0;
    a1 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    a0 = s1;
    FixedMul();
    s2 += v0;
    a1 = lw(gp + 0xB78);                                // Load from: gNvx (80078158)
    a2 = lw(gp + 0xB10);                                // Load from: gP4x (800780F0)
    v1 = lw(gp + 0xB1C);                                // Load from: gP4y (800780FC)
    v0 = lw(gp + 0xAF4);                                // Load from: gP1y (800780D4)
    a0 = lw(gp + 0xAEC);                                // Load from: gP1x (800780CC)
    s1 = v1 - v0;
    a0 = a2 - a0;
    FixedMul();
    s0 = v0;
    a1 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    a0 = s1;
    FixedMul();
    s0 += v0;
    v0 = ~s2;
    v0 >>= 31;
    v1 = s0 >> 31;
    {
        const bool bJump = (v0 != v1);
        v0 = 0x10000;                                   // Result = 00010000
        if (bJump) goto loc_8002556C;
    }
    a0 = s2;
    a1 = a0 - s0;
    FixedDiv();
loc_8002556C:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void CheckLineEnds() noexcept {
    v1 = lw(gp + 0xB14);                                // Load from: gP3y (800780F4)
    v0 = lw(gp + 0xAF4);                                // Load from: gP1y (800780D4)
    sp -= 0x28;
    sw(s2, sp + 0x18);
    s2 = lw(gp + 0xAFC);                                // Load from: gP3x (800780DC)
    a0 = lw(gp + 0xAEC);                                // Load from: gP1x (800780CC)
    sw(s0, sp + 0x10);
    s0 = lw(gp + 0xB1C);                                // Load from: gP4y (800780FC)
    sw(ra, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s1, sp + 0x14);
    s3 = v0 - v1;
    a0 -= s2;
    s0 -= v1;
    v0 = lw(gp + 0xB10);                                // Load from: gP4x (800780F0)
    a1 = s0;
    s2 -= v0;
    FixedMul();
    s1 = v0;
    a0 = s3;
    a1 = s2;
    FixedMul();
    s1 += v0;
    a1 = s0;
    a2 = lw(gp + 0xAF0);                                // Load from: gP2x (800780D0)
    v1 = lw(gp + 0xB00);                                // Load from: gP2y (800780E0)
    v0 = lw(gp + 0xB14);                                // Load from: gP3y (800780F4)
    a0 = lw(gp + 0xAFC);                                // Load from: gP3x (800780DC)
    s3 = v1 - v0;
    a0 = a2 - a0;
    FixedMul();
    s0 = v0;
    a0 = s3;
    a1 = s2;
    FixedMul();
    s0 += v0;
    s1 = ~s1;
    s1 >>= 31;
    s0 >>= 31;
    v0 = s1 ^ s0;
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void ClipToLine() noexcept {
loc_80025648:
    a1 = lw(gp + 0xB78);                                // Load from: gNvx (80078158)
    v1 = lw(gp + 0x9B0);                                // Load from: gSlideX (80077F90)
    a0 = lw(gp + 0xAEC);                                // Load from: gP1x (800780CC)
    a2 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = lw(gp + 0x9B4);                                // Load from: gSlideY (80077F94)
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = a1 << 1;
    v0 += a1;
    v0 <<= 3;
    v0 -= a1;
    v1 -= v0;
    v0 = a2 << 1;
    v0 += a2;
    v0 <<= 3;
    v0 -= a2;
    s0 -= v0;
    a2 = lw(gp + 0xA90);                                // Load from: gSlideDx (80078070)
    v0 = lw(gp + 0xA94);                                // Load from: gSlideDy (80078074)
    a0 = v1 - a0;
    sw(v1, gp + 0xAFC);                                 // Store to: gP3x (800780DC)
    sw(s0, gp + 0xB14);                                 // Store to: gP3y (800780F4)
    v1 += a2;
    sw(v1, gp + 0xB10);                                 // Store to: gP4x (800780F0)
    v1 = lw(gp + 0xAF4);                                // Load from: gP1y (800780D4)
    v0 += s0;
    sw(v0, gp + 0xB1C);                                 // Store to: gP4y (800780FC)
    s0 -= v1;
    FixedMul();
    s1 = v0;
    a1 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    a0 = s0;
    FixedMul();
    s1 += v0;
    v0 = 0x10000;                                       // Result = 00010000
    v0 = (i32(v0) < i32(s1));
    s2 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_800256F8;
    v0 = 0xFFFF0000;                                    // Result = FFFF0000
    v0 = (i32(s1) < i32(v0));
    s2 = -v0;
loc_800256F8:
    v0 = -1;                                            // Result = FFFFFFFF
    if (s2 == v0) goto loc_80025824;
    a1 = lw(gp + 0xB78);                                // Load from: gNvx (80078158)
    v1 = lw(gp + 0xB10);                                // Load from: gP4x (800780F0)
    s0 = lw(gp + 0xB1C);                                // Load from: gP4y (800780FC)
    a0 = lw(gp + 0xAEC);                                // Load from: gP1x (800780CC)
    v0 = lw(gp + 0xAF4);                                // Load from: gP1y (800780D4)
    a0 = v1 - a0;
    s0 -= v0;
    FixedMul();
    s1 = v0;
    a1 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    a0 = s0;
    FixedMul();
    s1 += v0;
    v0 = 0x10000;                                       // Result = 00010000
    v0 = (i32(v0) < i32(s1));
    v1 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_80025754;
    v0 = 0xFFFF0000;                                    // Result = FFFF0000
    v0 = (i32(s1) < i32(v0));
    v1 = -v0;
loc_80025754:
    v0 = 1;                                             // Result = 00000001
    if (v1 == 0) goto loc_80025824;
    if (v1 == v0) goto loc_80025824;
    v1 = 0;                                             // Result = 00000000
    if (s2 == 0) goto loc_8002580C;
    a1 = lw(gp + 0xB78);                                // Load from: gNvx (80078158)
    a2 = lw(gp + 0xAFC);                                // Load from: gP3x (800780DC)
    v1 = lw(gp + 0xB14);                                // Load from: gP3y (800780F4)
    v0 = lw(gp + 0xAF4);                                // Load from: gP1y (800780D4)
    a0 = lw(gp + 0xAEC);                                // Load from: gP1x (800780CC)
    s1 = v1 - v0;
    a0 = a2 - a0;
    FixedMul();
    s2 = v0;
    a1 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    a0 = s1;
    FixedMul();
    s2 += v0;
    a1 = lw(gp + 0xB78);                                // Load from: gNvx (80078158)
    a2 = lw(gp + 0xB10);                                // Load from: gP4x (800780F0)
    v1 = lw(gp + 0xB1C);                                // Load from: gP4y (800780FC)
    v0 = lw(gp + 0xAF4);                                // Load from: gP1y (800780D4)
    a0 = lw(gp + 0xAEC);                                // Load from: gP1x (800780CC)
    s1 = v1 - v0;
    a0 = a2 - a0;
    FixedMul();
    s0 = v0;
    a1 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    a0 = s1;
    FixedMul();
    s0 += v0;
    v0 = ~s2;
    v0 >>= 31;
    v1 = s0 >> 31;
    {
        const bool bJump = (v0 != v1);
        v1 = 0x10000;                                   // Result = 00010000
        if (bJump) goto loc_800257F8;
    }
    a0 = s2;
    a1 = a0 - s0;
    FixedDiv();
    v1 = v0;
loc_800257F8:
    v0 = lw(gp + 0xC48);                                // Load from: gBlockFrac (80078228)
    v0 = (i32(v1) < i32(v0));
    if (v0 == 0) goto loc_80025824;
loc_8002580C:
    v0 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    sw(v1, gp + 0xC48);                                 // Store to: gBlockFrac (80078228)
    v1 = lw(gp + 0xB78);                                // Load from: gNvx (80078158)
    v0 = -v0;
    sw(v0, gp + 0xBC8);                                 // Store to: gBlockNvx (800781A8)
    sw(v1, gp + 0xBD0);                                 // Store to: gBlockNvy (800781B0)
loc_80025824:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void SL_CheckLine() noexcept {
loc_80025840:
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7BFC);                               // Load from: gEndBox[3] (80097BFC)
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(ra, sp + 0x1C);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v1 = lw(s2 + 0x2C);
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_80025A70;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7BF8);                               // Load from: gEndBox[2] (80097BF8)
    v0 = lw(s2 + 0x30);
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_80025A70;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7BF0);                               // Load from: gEndBox[0] (80097BF0)
    v1 = lw(s2 + 0x28);
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_80025A70;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7BF4);                               // Load from: gEndBox[1] (80097BF4)
    v0 = lw(s2 + 0x24);
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_80025A70;
    a0 = lw(s2 + 0x3C);
    if (a0 == 0) goto loc_80025960;
    v0 = lw(s2 + 0x10);
    v0 &= 1;
    if (v0 != 0) goto loc_80025960;
    a2 = lw(s2 + 0x38);
    v0 = lw(a0);
    v1 = lw(a2);
    a1 = v0;
    v0 = (i32(a1) < i32(v1));
    if (v0 == 0) goto loc_8002590C;
    a1 = v1;
loc_8002590C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7ED8);                               // Load from: gpSlideThing (80077ED8)
    v1 = lw(v0 + 0x8);
    v0 = 0x180000;                                      // Result = 00180000
    v1 = a1 - v1;
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_80025960;
    v0 = lw(a0 + 0x4);
    a0 = lw(a2 + 0x4);
    v1 = v0;
    v0 = (i32(a0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0x370000;                                  // Result = 00370000
        if (bJump) goto loc_8002594C;
    }
    v1 = a0;
loc_8002594C:
    v0 |= 0xFFFF;                                       // Result = 0037FFFF
    v1 -= a1;
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_80025A70;
loc_80025960:
    v0 = lw(s2);
    a0 = lw(gp + 0x9B0);                                // Load from: gSlideX (80077F90)
    v1 = lw(s2);
    a1 = lw(s2 + 0x4);
    s0 = lw(gp + 0x9B4);                                // Load from: gSlideY (80077F94)
    a3 = lw(v0);
    v0 = lw(s2 + 0x48);
    t0 = lw(v1 + 0x4);
    a2 = lw(a1);
    v1 = lw(s2 + 0x4);
    v0 <<= 2;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v0;
    a1 = lw(at);
    v0 = lw(v1 + 0x4);
    a0 -= a3;
    sw(v0, gp + 0xB00);                                 // Store to: gP2y (800780E0)
    v0 = lw(s2 + 0x48);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v0 <<= 2;
    v0 += v1;
    v0 = lw(v0);
    sw(a3, gp + 0xAEC);                                 // Store to: gP1x (800780CC)
    sw(t0, gp + 0xAF4);                                 // Store to: gP1y (800780D4)
    sw(a2, gp + 0xAF0);                                 // Store to: gP2x (800780D0)
    sw(a1, gp + 0xB78);                                 // Store to: gNvx (80078158)
    v0 = -v0;
    sw(v0, gp + 0xB7C);                                 // Store to: gNvy (8007815C)
    s0 -= t0;
    FixedMul();
    s1 = v0;
    a1 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    a0 = s0;
    FixedMul();
    s1 += v0;
    v0 = 0x10000;                                       // Result = 00010000
    v0 = (i32(v0) < i32(s1));
    v1 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_80025A10;
    v0 = 0xFFFF0000;                                    // Result = FFFF0000
    v0 = (i32(s1) < i32(v0));
    v1 = -v0;
loc_80025A10:
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 == 0) goto loc_80025A70;
    if (v1 != v0) goto loc_80025A68;
    v0 = lw(s2 + 0x3C);
    if (v0 == 0) goto loc_80025A70;
    a0 = lw(gp + 0xAEC);                                // Load from: gP1x (800780CC)
    v1 = lw(gp + 0xAF0);                                // Load from: gP2x (800780D0)
    v0 = lw(gp + 0xB00);                                // Load from: gP2y (800780E0)
    sw(a0, gp + 0xAF0);                                 // Store to: gP2x (800780D0)
    a0 = lw(gp + 0xAF4);                                // Load from: gP1y (800780D4)
    sw(v0, gp + 0xAF4);                                 // Store to: gP1y (800780D4)
    v0 = lw(gp + 0xB78);                                // Load from: gNvx (80078158)
    sw(v1, gp + 0xAEC);                                 // Store to: gP1x (800780CC)
    v1 = lw(gp + 0xB7C);                                // Load from: gNvy (8007815C)
    v0 = -v0;
    v1 = -v1;
    sw(v0, gp + 0xB78);                                 // Store to: gNvx (80078158)
    sw(v1, gp + 0xB7C);                                 // Store to: gNvy (8007815C)
    sw(a0, gp + 0xB00);                                 // Store to: gP2y (800780E0)
loc_80025A68:
    ClipToLine();
loc_80025A70:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void SL_PointOnSide2() noexcept {
    sp -= 0x20;
    sw(s2, sp + 0x18);
    s2 = a2;
    sw(s1, sp + 0x14);
    s1 = a1 - a3;
    a1 = lw(sp + 0x34);
    v0 = lw(sp + 0x30);
    a0 -= s2;
    sw(ra, sp + 0x1C);
    sw(s0, sp + 0x10);
    a1 -= a3;
    s2 -= v0;
    FixedMul();
    s0 = v0;
    a0 = s1;
    a1 = s2;
    FixedMul();
    s0 += v0;
    v0 = -1;                                            // Result = FFFFFFFF
    if (i32(s0) < 0) goto loc_80025AE0;
    v0 = 1;                                             // Result = 00000001
loc_80025AE0:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void SL_CheckSpecialLines() noexcept {
loc_80025AFC:
    sp -= 0x98;
    sw(a0, sp + 0x10);
    sw(a2, sp + 0x20);
    v0 = (i32(a0) < i32(a2));
    sw(ra, sp + 0x94);
    sw(fp, sp + 0x90);
    sw(s7, sp + 0x8C);
    sw(s6, sp + 0x88);
    sw(s5, sp + 0x84);
    sw(s4, sp + 0x80);
    sw(s3, sp + 0x7C);
    sw(s2, sp + 0x78);
    sw(s1, sp + 0x74);
    sw(s0, sp + 0x70);
    sw(a1, sp + 0x18);
    sw(a3, sp + 0x28);
    if (v0 == 0) goto loc_80025B50;
    t1 = lw(sp + 0x10);
    t0 = lw(sp + 0x20);
    sw(t1, sp + 0x38);
    goto loc_80025B5C;
loc_80025B50:
    t1 = lw(sp + 0x20);
    t0 = lw(sp + 0x10);
    sw(t1, sp + 0x38);
loc_80025B5C:
    sw(t0, sp + 0x40);
    t1 = lw(sp + 0x18);
    t0 = lw(sp + 0x28);
    v0 = (i32(t1) < i32(t0));
    if (v0 == 0) goto loc_80025B94;
    sw(t1, sp + 0x48);
    goto loc_80025BA0;
loc_80025B80:
    v0 = lw(gp + 0xA3C);                                // Load from: gpLine (8007801C)
    sw(v0, gp + 0x9BC);                                 // Store to: gpSpecialLine (80077F9C)
    goto loc_80025F10;
loc_80025B94:
    t1 = lw(sp + 0x28);
    t0 = lw(sp + 0x18);
    sw(t1, sp + 0x48);
loc_80025BA0:
    sw(t0, sp + 0x50);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7E74);                               // Load from: gBlockmapOriginX (8007818C)
    t1 = lw(sp + 0x38);
    t0 = lw(sp + 0x40);
    v1 = t1 - v0;
    a0 = u32(i32(v1) >> 23);
    v0 = t0 - v0;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7E6C);                               // Load from: gBlockmapOriginY (80078194)
    t1 = lw(sp + 0x48);
    t0 = lw(sp + 0x50);
    v0 = u32(i32(v0) >> 23);
    sw(v0, sp + 0x58);
    v0 = t1 - v1;
    v0 = u32(i32(v0) >> 23);
    v1 = t0 - v1;
    v1 = u32(i32(v1) >> 23);
    sw(v0, sp + 0x60);
    sw(v1, sp + 0x68);
    if (i32(a0) >= 0) goto loc_80025BF8;
    a0 = 0;                                             // Result = 00000000
loc_80025BF8:
    t1 = lw(sp + 0x60);
    if (i32(t1) >= 0) goto loc_80025C0C;
    sw(0, sp + 0x60);
loc_80025C0C:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D7C);                               // Load from: gBlockmapWidth (80078284)
    t0 = lw(sp + 0x58);
    v0 = (i32(t0) < i32(v1));
    v1--;
    if (v0 != 0) goto loc_80025C2C;
    sw(v1, sp + 0x58);
loc_80025C2C:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EB8);                               // Load from: gBlockmapHeight (80077EB8)
    t1 = lw(sp + 0x68);
    v0 = (i32(t1) < i32(v1));
    if (v0 != 0) goto loc_80025C50;
    v1--;
    sw(v1, sp + 0x68);
loc_80025C50:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    t0 = lw(sp + 0x58);
    sw(a0, sp + 0x30);
    sw(0, gp + 0x9BC);                                  // Store to: gpSpecialLine (80077F9C)
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    v0 = (i32(t0) < i32(a0));
    if (v0 != 0) goto loc_80025F10;
loc_80025C7C:
    fp = lw(sp + 0x60);
    t1 = lw(sp + 0x68);
    v0 = (i32(t1) < i32(fp));
    if (v0 != 0) goto loc_80025EF8;
loc_80025C94:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D7C);                               // Load from: gBlockmapWidth (80078284)
    mult(fp, v0);
    t0 = lw(sp + 0x30);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7EC0);                               // Load from: gpBlockmap (80078140)
    v1 = lo;
    v1 += t0;
    v0 = v1 << 1;
    v0 += a0;
    v0 = lh(v0);
    sw(v1, gp + 0x9A4);                                 // Store to: gSL_CheckSpecialLines_UNKNOWN_var (80077F84)
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7F3C);                               // Load from: gpBlockmapLump (800780C4)
    sw(v0, gp + 0x9A4);                                 // Store to: gSL_CheckSpecialLines_UNKNOWN_var (80077F84)
    v0 <<= 1;
    v0 += v1;
    sw(v0, gp + 0xC70);                                 // Store to: gpBlockmapLineNum (80078250)
    v1 = lh(v0);
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 == v0) goto loc_80025EE4;
loc_80025CF0:
    v0 = lw(gp + 0xC70);                                // Load from: gpBlockmapLineNum (80078250)
    v1 = lh(v0);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EB0);                               // Load from: gpLines (80077EB0)
    v0 <<= 2;
    v1 += v0;
    v0 = lw(v1 + 0x14);
    sw(v1, gp + 0xA3C);                                 // Store to: gpLine (8007801C)
    if (v0 == 0) goto loc_80025EC4;
    v0 = lw(v1 + 0x40);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    if (v0 == a0) goto loc_80025EC4;
    v0 = lw(v1 + 0x2C);
    sw(a0, v1 + 0x40);
    t1 = lw(sp + 0x40);
    v0 = (i32(t1) < i32(v0));
    if (v0 != 0) goto loc_80025EC4;
    v0 = lw(v1 + 0x30);
    t0 = lw(sp + 0x38);
    v0 = (i32(v0) < i32(t0));
    if (v0 != 0) goto loc_80025EC4;
    v0 = lw(v1 + 0x28);
    t1 = lw(sp + 0x50);
    v0 = (i32(t1) < i32(v0));
    if (v0 != 0) goto loc_80025EC4;
    v0 = lw(v1 + 0x24);
    t0 = lw(sp + 0x48);
    v0 = (i32(v0) < i32(t0));
    s5 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_80025EC4;
    v0 = lw(v1);
    v1 = lw(v1 + 0x4);
    t1 = lw(sp + 0x10);
    s2 = lw(v0);
    s4 = lw(v0 + 0x4);
    s7 = lw(v1 + 0x4);
    s6 = lw(v1);
    a0 = t1 - s2;
    s3 = s7 - s4;
    a1 = s3;
    FixedMul();
    s0 = v0;
    s1 = s2 - s6;
    t0 = lw(sp + 0x18);
    a1 = s1;
    a0 = t0 - s4;
    FixedMul();
    s0 += v0;
    a1 = s3;
    if (i32(s0) >= 0) goto loc_80025E00;
    s5 = -1;                                            // Result = FFFFFFFF
loc_80025E00:
    t1 = lw(sp + 0x20);
    a0 = t1 - s2;
    FixedMul();
    s0 = v0;
    t0 = lw(sp + 0x28);
    a1 = s1;
    a0 = t0 - s4;
    FixedMul();
    s0 += v0;
    v0 = 1;                                             // Result = 00000001
    if (i32(s0) >= 0) goto loc_80025E30;
    v0 = -1;                                            // Result = FFFFFFFF
loc_80025E30:
    if (s5 == v0) goto loc_80025EC4;
    t1 = lw(sp + 0x10);
    t0 = lw(sp + 0x28);
    a0 = s2 - t1;
    t1 = lw(sp + 0x18);
    s2 = t0 - t1;
    a1 = s2;
    FixedMul();
    t0 = lw(sp + 0x18);
    t1 = lw(sp + 0x10);
    a0 = s4 - t0;
    t0 = lw(sp + 0x20);
    s0 = v0;
    s1 = t1 - t0;
    a1 = s1;
    FixedMul();
    s0 += v0;
    s5 = 1;                                             // Result = 00000001
    if (i32(s0) >= 0) goto loc_80025E88;
    s5 = -1;                                            // Result = FFFFFFFF
loc_80025E88:
    t1 = lw(sp + 0x10);
    a1 = s2;
    a0 = s6 - t1;
    FixedMul();
    s0 = v0;
    t0 = lw(sp + 0x18);
    a1 = s1;
    a0 = s7 - t0;
    FixedMul();
    s0 += v0;
    v0 = 1;                                             // Result = 00000001
    if (i32(s0) >= 0) goto loc_80025EBC;
    v0 = -1;                                            // Result = FFFFFFFF
loc_80025EBC:
    if (s5 != v0) goto loc_80025B80;
loc_80025EC4:
    v1 = lw(gp + 0xC70);                                // Load from: gpBlockmapLineNum (80078250)
    v0 = v1 + 2;
    sw(v0, gp + 0xC70);                                 // Store to: gpBlockmapLineNum (80078250)
    v1 = lh(v1 + 0x2);
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 != v0) goto loc_80025CF0;
loc_80025EE4:
    t1 = lw(sp + 0x68);
    fp++;
    v0 = (i32(t1) < i32(fp));
    if (v0 == 0) goto loc_80025C94;
loc_80025EF8:
    t0 = lw(sp + 0x30);
    t1 = lw(sp + 0x58);
    t0++;
    v0 = (i32(t1) < i32(t0));
    sw(t0, sp + 0x30);
    if (v0 == 0) goto loc_80025C7C;
loc_80025F10:
    ra = lw(sp + 0x94);
    fp = lw(sp + 0x90);
    s7 = lw(sp + 0x8C);
    s6 = lw(sp + 0x88);
    s5 = lw(sp + 0x84);
    s4 = lw(sp + 0x80);
    s3 = lw(sp + 0x7C);
    s2 = lw(sp + 0x78);
    s1 = lw(sp + 0x74);
    s0 = lw(sp + 0x70);
    sp += 0x98;
    return;
}
