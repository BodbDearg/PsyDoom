#include "p_sight.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "p_setup.h"
#include "PsxVm/PsxVm.h"

void P_CheckSights() noexcept {
loc_80024908:
    sp -= 0x20;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    s0 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 -= 0x14;                                         // Result = gMObjHead[0] (800A8E90)
    s1 = 0x400000;                                      // Result = 00400000
    if (s0 == v0) goto loc_8002499C;
loc_80024930:
    a0 = lw(s0 + 0x64);
    v0 = a0 & s1;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80024988;
    }
    v1 = lw(s0 + 0x5C);
    {
        const bool bJump = (v1 != v0);
        v0 = 0xFBFF0000;                                // Result = FBFF0000
        if (bJump) goto loc_80024988;
    }
    v0 |= 0xFFFF;                                       // Result = FBFFFFFF
    a1 = lw(s0 + 0x74);
    v0 &= a0;
    sw(v0, s0 + 0x64);
    if (a1 == 0) goto loc_80024988;
    a0 = s0;
    P_CheckSight();
    v1 = 0x4000000;                                     // Result = 04000000
    if (v0 == 0) goto loc_80024988;
    v0 = lw(s0 + 0x64);
    v0 |= v1;
    sw(v0, s0 + 0x64);
loc_80024988:
    s0 = lw(s0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    if (s0 != v0) goto loc_80024930;
loc_8002499C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_CheckSight() noexcept {
loc_800249B4:
    sp -= 0x18;
    t0 = a0;
    t1 = a1;
    a1 = 0xE9BD0000;                                    // Result = E9BD0000
    sw(ra, sp + 0x10);
    v0 = lw(t0 + 0xC);
    a0 = *gpSectors;
    v0 = lw(v0);
    a1 |= 0x37A7;                                       // Result = E9BD37A7
    v0 -= a0;
    mult(v0, a1);
    v0 = lw(t1 + 0xC);
    v0 = lw(v0);
    v1 = lo;
    v0 -= a0;
    mult(v0, a1);
    v0 = lo;
    a0 = *gNumSectors;
    v1 = u32(i32(v1) >> 2);
    mult(v1, a0);
    v0 = u32(i32(v0) >> 2);
    v1 = 1;                                             // Result = 00000001
    a0 = lo;
    a0 += v0;
    a1 = u32(i32(a0) >> 3);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F1C);                               // Load from: gpRejectMatrix (800780E4)
    a0 &= 7;
    v0 += a1;
    v0 = lbu(v0);
    v1 = v1 << a0;
    v0 &= v1;
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80024B2C;
    }
    a3 = 0xFFFE0000;                                    // Result = FFFE0000
    a2 = 0x10000;                                       // Result = 00010000
    a1 = lw(t0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    a1 &= a3;
    a1 |= a2;
    at = 0x80090000;                                    // Result = 80090000
    sw(a1, at + 0x7C00);                                // Store to: gSTrace[0] (80097C00)
    a0 = lw(t0 + 0x4);
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    v0 = u32(i32(a1) >> 16);
    sw(v0, gp + 0xC18);                                 // Store to: gT1xs (800781F8)
    a0 &= a3;
    a0 |= a2;
    at = 0x80090000;                                    // Result = 80090000
    sw(a0, at + 0x7C04);                                // Store to: gSTrace[1] (80097C04)
    v0 = lw(t1);
    v1 = u32(i32(a0) >> 16);
    sw(v1, gp + 0xC28);                                 // Store to: gT1ys (80078208)
    v1 = lw(t1 + 0x4);
    v0 &= a3;
    v0 |= a2;
    v1 &= a3;
    v1 |= a2;
    a1 = v0 - a1;
    a0 = v1 - a0;
    at = 0x80090000;                                    // Result = 80090000
    sw(a1, at + 0x7C08);                                // Store to: gSTrace[2] (80097C08)
    at = 0x80090000;                                    // Result = 80090000
    sw(a0, at + 0x7C0C);                                // Store to: gSTrace[3] (80097C0C)
    a1 = lw(t0 + 0x8);
    a0 = lw(t0 + 0x44);
    sw(v0, gp + 0xB20);                                 // Store to: gT2x (80078100)
    v0 = u32(i32(v0) >> 16);
    sw(v0, gp + 0xC24);                                 // Store to: gT2xs (80078204)
    v0 = lw(t1 + 0x8);
    sw(v1, gp + 0xB28);                                 // Store to: gT2y (80078108)
    v1 = u32(i32(v1) >> 16);
    sw(v1, gp + 0xC30);                                 // Store to: gT2ys (80078210)
    v1 = lw(t1 + 0x44);
    a1 += a0;
    a0 = u32(i32(a0) >> 2);
    a1 -= a0;
    v0 += v1;
    v0 -= a1;
    sw(v0, gp + 0xC00);                                 // Store to: gTopSlope (800781E0)
    v0 = lw(t1 + 0x8);
    a0 = *gNumBspNodes;
    sw(a1, gp + 0xA40);                                 // Store to: gSightZStart (80078020)
    v0 -= a1;
    sw(v0, gp + 0xA28);                                 // Store to: gBottomSlope (80078008)
    a0--;
    PS_CrossBSPNode();
loc_80024B2C:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PS_SightCrossLine() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v1 = lw(a0 + 0x4);
    a1 = lw(gp + 0xC28);                                // Load from: gT1ys (80078208)
    t8 = lw(gp + 0xC30);                                // Load from: gT2ys (80078210)
    v0 = lw(gp + 0xC18);                                // Load from: gT1xs (800781F8)
    t7 = lh(v1 + 0x2);
    a2 = t8 - a1;
    t0 = t7 - v0;
    mult(a2, t0);
    t6 = lw(gp + 0xC24);                                // Load from: gT2xs (80078204)
    t5 = lh(v1 + 0x6);
    t4 = lo;
    t1 = t6 - v0;
    a3 = t5 - a1;
    mult(a3, t1);
    a0 = lw(a0);
    t3 = lh(a0 + 0x2);
    v1 = lo;
    t0 = t3 - v0;
    mult(a2, t0);
    t2 = lh(a0 + 0x6);
    a2 = lo;
    a3 = t2 - a1;
    mult(a3, t1);
    v1 = (i32(t4) < i32(v1));
    v0 = lo;
    v0 = (i32(a2) < i32(v0));
    t1 = t2 - t5;
    if (v1 == v0) goto loc_80024C00;
    mult(t1, t0);
    a0 = lo;
    a2 = t7 - t3;
    mult(a2, a3);
    v0 = lo;
    t0 = t6 - t3;
    mult(t1, t0);
    v1 = lo;
    a3 = t8 - t2;
    mult(a2, a3);
    t4 = a0 + v0;
    a0 = t4;
    v0 = lo;
    a2 = v1 + v0;
    a1 = a0 + a2;
    FixedDiv();
    goto loc_80024C04;
loc_80024C00:
    v0 = -1;                                            // Result = FFFFFFFF
loc_80024C04:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PS_CrossSubsector() noexcept {
loc_80024C14:
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v1 = lh(a0 + 0x6);
    s1 = lh(a0 + 0x4);
    v0 = v1 << 2;
    v0 += v1;
    v1 = *gpSegs;
    v0 <<= 3;
    s2 = v0 + v1;
    if (s1 == 0) goto loc_80024EA0;
loc_80024C4C:
    s0 = lw(s2 + 0x14);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    v0 = lw(s0 + 0x40);
    if (v0 == v1) goto loc_80024E94;
    v0 = lw(s0 + 0x4);
    a0 = lw(gp + 0xC28);                                // Load from: gT1ys (80078208)
    t8 = lw(gp + 0xC30);                                // Load from: gT2ys (80078210)
    sw(v1, s0 + 0x40);
    v1 = lw(gp + 0xC18);                                // Load from: gT1xs (800781F8)
    t7 = lh(v0 + 0x2);
    t1 = t8 - a0;
    a3 = t7 - v1;
    mult(t1, a3);
    t6 = lw(gp + 0xC24);                                // Load from: gT2xs (80078204)
    t5 = lh(v0 + 0x6);
    t2 = lo;
    t0 = t6 - v1;
    a2 = t5 - a0;
    mult(a2, t0);
    v0 = lw(s0);
    t4 = lh(v0 + 0x2);
    a1 = lo;
    a3 = t4 - v1;
    mult(t1, a3);
    t3 = lh(v0 + 0x6);
    v1 = lo;
    a2 = t3 - a0;
    mult(a2, t0);
    t2 = (i32(t2) < i32(a1));
    v0 = lo;
    v1 = (i32(v1) < i32(v0));
    t0 = t3 - t5;
    if (t2 != v1) goto loc_80024CE8;
    a1 = -1;                                            // Result = FFFFFFFF
    goto loc_80024D2C;
loc_80024CE8:
    mult(t0, a3);
    a0 = lo;
    t1 = t7 - t4;
    mult(t1, a2);
    v0 = lo;
    a3 = t6 - t4;
    mult(t0, a3);
    v1 = lo;
    a2 = t8 - t3;
    mult(t1, a2);
    t2 = a0 + v0;
    a0 = t2;
    v0 = lo;
    v1 += v0;
    a1 = a0 + v1;
    FixedDiv();
    a1 = v0;
loc_80024D2C:
    v1 = a1 - 4;
    v0 = 0xFFFC;                                        // Result = 0000FFFC
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80024E94;
    t0 = lw(s0 + 0x3C);
    if (t0 == 0) goto loc_80024E8C;
    t1 = lw(s0 + 0x38);
    a0 = lw(t0);
    a2 = lw(t1);
    if (a2 != a0) goto loc_80024D7C;
    v1 = lw(t1 + 0x4);
    v0 = lw(t0 + 0x4);
    if (v1 == v0) goto loc_80024E94;
loc_80024D7C:
    v0 = lw(t0 + 0x4);
    v1 = lw(t1 + 0x4);
    a3 = v0;
    v0 = (i32(v1) < i32(a3));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(a0) < i32(a2));
        if (bJump) goto loc_80024D98;
    }
    a3 = v1;
loc_80024D98:
    v1 = a0;
    if (v0 == 0) goto loc_80024DA4;
    v1 = a2;
loc_80024DA4:
    v0 = (i32(v1) < i32(a3));
    if (v0 == 0) goto loc_80024E8C;
    a1 = u32(i32(a1) >> 2);
    if (a2 == a0) goto loc_80024E0C;
    v0 = lw(gp + 0xA40);                                // Load from: gSightZStart (80078020)
    v0 = v1 - v0;
    v0 <<= 6;
    div(v0, a1);
    if (a1 != 0) goto loc_80024DD8;
    _break(0x1C00);
loc_80024DD8:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80024DF0;
    }
    if (v0 != at) goto loc_80024DF0;
    tge(zero, zero, 0x5D);
loc_80024DF0:
    v0 = lo;
    v1 = lw(gp + 0xA28);                                // Load from: gBottomSlope (80078008)
    v0 <<= 8;
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80024E0C;
    sw(v0, gp + 0xA28);                                 // Store to: gBottomSlope (80078008)
loc_80024E0C:
    v1 = lw(t1 + 0x4);
    v0 = lw(t0 + 0x4);
    if (v1 == v0) goto loc_80024E74;
    v0 = lw(gp + 0xA40);                                // Load from: gSightZStart (80078020)
    v0 = a3 - v0;
    v0 <<= 6;
    div(v0, a1);
    if (a1 != 0) goto loc_80024E40;
    _break(0x1C00);
loc_80024E40:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_80024E58;
    }
    if (v0 != at) goto loc_80024E58;
    tge(zero, zero, 0x5D);
loc_80024E58:
    v0 = lo;
    v1 = lw(gp + 0xC00);                                // Load from: gTopSlope (800781E0)
    v0 <<= 8;
    v1 = (i32(v0) < i32(v1));
    if (v1 == 0) goto loc_80024E74;
    sw(v0, gp + 0xC00);                                 // Store to: gTopSlope (800781E0)
loc_80024E74:
    v1 = lw(gp + 0xC00);                                // Load from: gTopSlope (800781E0)
    v0 = lw(gp + 0xA28);                                // Load from: gBottomSlope (80078008)
    v0 = (i32(v0) < i32(v1));
    s1--;
    if (v0 != 0) goto loc_80024E98;
loc_80024E8C:
    v0 = 0;                                             // Result = 00000000
    goto loc_80024EA4;
loc_80024E94:
    s1--;
loc_80024E98:
    s2 += 0x28;
    if (s1 != 0) goto loc_80024C4C;
loc_80024EA0:
    v0 = 1;                                             // Result = 00000001
loc_80024EA4:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PS_CrossBSPNode() noexcept {
loc_80024EC0:
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = a0 & 0x8000;
loc_80024ED8:
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFFFF0000;                                // Result = FFFF0000
        if (bJump) goto loc_80024F28;
    }
    v0 |= 0x7FFF;                                       // Result = FFFF7FFF
    a2 = *gNumSubsectors;
    s0 = a0 & v0;
    v0 = (i32(s0) < i32(a2));
    if (v0 != 0) goto loc_80024F0C;
    I_Error("PS_CrossSubsector: ss %i with numss = %i", (int32_t) s0, (int32_t) a2);
loc_80024F0C:
    v0 = *gpSubsectors;
    a0 = s0 << 4;
    a0 += v0;
    PS_CrossSubsector();
    goto loc_80025010;
loc_80024F28:
    v0 = a0 << 3;
    v0 -= a0;
    v1 = *gpBspNodes;
    v0 <<= 3;
    s0 = v0 + v1;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7C04);                               // Load from: gSTrace[1] (80097C04)
    v1 = lw(s0 + 0x4);
    v0 -= v1;
    v1 = lh(s0 + 0xA);
    v0 = u32(i32(v0) >> 16);
    mult(v0, v1);
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7C00);                               // Load from: gSTrace[0] (80097C00)
    v1 = lw(s0);
    v0 -= v1;
    a0 = lo;
    v1 = lh(s0 + 0xE);
    v0 = u32(i32(v0) >> 16);
    mult(v1, v0);
    v0 = lo;
    s2 = (i32(a0) < i32(v0));
    s1 = s2 ^ 1;
    v0 = s1 << 2;
    v0 += s0;
    a0 = lw(v0 + 0x30);
    PS_CrossBSPNode();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80025010;
    }
    v0 = lw(gp + 0xB28);                                // Load from: gT2y (80078108)
    v1 = lw(s0 + 0x4);
    v0 -= v1;
    v1 = lh(s0 + 0xA);
    v0 = u32(i32(v0) >> 16);
    mult(v0, v1);
    v0 = lw(gp + 0xB20);                                // Load from: gT2x (80078100)
    v1 = lw(s0);
    v0 -= v1;
    v1 = lo;
    a0 = lh(s0 + 0xE);
    v0 = u32(i32(v0) >> 16);
    mult(a0, v0);
    v0 = lo;
    v1 = (i32(v1) < i32(v0));
    v1 ^= 1;
    v0 = s2 << 2;
    if (s1 == v1) goto loc_8002500C;
    v0 += s0;
    a0 = lw(v0 + 0x30);
    v0 = a0 & 0x8000;
    goto loc_80024ED8;
loc_8002500C:
    v0 = 1;                                             // Result = 00000001
loc_80025010:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
