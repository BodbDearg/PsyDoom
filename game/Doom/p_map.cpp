#include "p_map.h"

#include "PsxVm/PsxVm.h"

void P_CheckPosition() noexcept {
loc_8001B640:
    sp -= 0x18;
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x10);
    sw(a0, gp + 0xAAC);                                 // Store to: gpTryMoveThing (8007808C)
    sw(a1, gp + 0xB70);                                 // Store to: gTryMoveX (80078150)
    sw(a2, gp + 0xB74);                                 // Store to: gTryMoveY (80078154)
    sw(v0, gp + 0xB08);                                 // Store to: gbCheckPosOnly (800780E8)
    P_TryMove2();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EC4);                               // Load from: gbTryMove2 (8007813C)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_TryMove() noexcept {
loc_8001B67C:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    sw(s0, gp + 0xAAC);                                 // Store to: gpTryMoveThing (8007808C)
    sw(a1, gp + 0xB70);                                 // Store to: gTryMoveX (80078150)
    sw(a2, gp + 0xB74);                                 // Store to: gTryMoveY (80078154)
    P_TryMove2();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7D3C);                               // Load from: gpMoveThing (800782C4)
    v0 = 0x10000;                                       // Result = 00010000
    if (a0 == 0) goto loc_8001B788;
    v1 = lw(s0 + 0x64);
    v0 &= v1;
    {
        const bool bJump = (v0 == 0)
        v0 = 0x1000000;                                 // Result = 01000000
        if (bJump) goto loc_8001B708;
    }
    P_Random();
    v1 = lw(s0 + 0x58);
    v0 &= 7;
    v1 = lw(v1 + 0x4C);
    v0++;
    mult(v0, v1);
    a1 = s0;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7D3C);                               // Load from: gpMoveThing (800782C4)
    a2 = lw(a1 + 0x74);
    a3 = lo;
    P_DamageMObj();
    goto loc_8001B788;
loc_8001B708:
    v0 &= v1;
    if (v0 == 0) goto loc_8001B780;
    P_Random();
    v1 = lw(s0 + 0x58);
    v0 &= 7;
    v1 = lw(v1 + 0x4C);
    v0++;
    mult(v0, v1);
    a1 = s0;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7D3C);                               // Load from: gpMoveThing (800782C4)
    a3 = lo;
    a2 = s0;
    P_DamageMObj();
    a0 = 0xFEFF0000;                                    // Result = FEFF0000
    v0 = lw(s0 + 0x64);
    v1 = lw(s0 + 0x58);
    a0 |= 0xFFFF;                                       // Result = FEFFFFFF
    sw(0, s0 + 0x50);
    sw(0, s0 + 0x4C);
    sw(0, s0 + 0x48);
    v0 &= a0;
    sw(v0, s0 + 0x64);
    a1 = lw(v1 + 0x4);
    a0 = s0;
    P_SetMObjState();
    goto loc_8001B788;
loc_8001B780:
    a1 = s0;
    P_TouchSpecialThing();
loc_8001B788:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EC4);                               // Load from: gbTryMove2 (8007813C)
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_InterceptVector() noexcept {
    a3 = lh(a1 + 0xE);
    v0 = lh(a0 + 0xA);
    mult(a3, v0);
    t0 = lh(a1 + 0xA);
    v1 = lo;
    v0 = lh(a0 + 0xE);
    mult(t0, v0);
    v0 = lo;
    a2 = v1 - v0;
    v0 = -1;                                            // Result = FFFFFFFF
    if (a2 == 0) goto loc_8001B840;
    v0 = lw(a1);
    v1 = lw(a0);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a3);
    v1 = lw(a0 + 0x4);
    a0 = lw(a1 + 0x4);
    v0 = lo;
    v1 -= a0;
    v1 = u32(i32(v1) >> 16);
    mult(v1, t0);
    v1 = lo;
    v0 += v1;
    v0 <<= 16;
    div(v0, a2);
    if (a2 != 0) goto loc_8001B824;
    _break(0x1C00);
loc_8001B824:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a2 != at)
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001B83C;
    }
    if (v0 != at) goto loc_8001B83C;
    tge(zero, zero, 0x5D);
loc_8001B83C:
    v0 = lo;
loc_8001B840:
    return;
}

void PIT_UseLines() noexcept {
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x7898);                               // Load from: gUseBBox[3] (800A8768)
    sp -= 0x30;
    sw(s1, sp + 0x24);
    s1 = a0;
    sw(ra, sp + 0x28);
    sw(s0, sp + 0x20);
    v0 = lw(s1 + 0x2C);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9DC;
    }
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x789C);                               // Load from: gUseBBox[2] (800A8764)
    v1 = lw(s1 + 0x30);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9DC;
    }
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A4);                               // Load from: gUseBBox[0] (800A875C)
    v0 = lw(s1 + 0x28);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9DC;
    }
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A0);                               // Load from: gUseBBox[1] (800A8760)
    v1 = lw(s1 + 0x24);
    v0 = (i32(v0) < i32(v1));
    a0 = s1;
    if (v0 == 0) goto loc_8001B9CC;
    a1 = sp + 0x10;
    P_MakeDivline();
    a0 = lh(sp + 0x1E);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lh(v0 - 0x78AE);                               // Load from: gUseLine[2] (800A8752)
    mult(a0, v0);
    a2 = lh(sp + 0x1A);
    v1 = lo;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lh(v0 - 0x78AA);                               // Load from: gUseLine[3] (800A8756)
    mult(a2, v0);
    v0 = lo;
    a1 = v1 - v0;
    s0 = -1;                                            // Result = FFFFFFFF
    if (a1 == 0) goto loc_8001B980;
    v0 = lw(sp + 0x10);
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78B8);                               // Load from: gUseLine[0] (800A8748)
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a0);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78B4);                               // Load from: gUseLine[1] (800A874C)
    a0 = lw(sp + 0x14);
    v1 = lo;
    v0 -= a0;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a2);
    v0 = lo;
    v1 += v0;
    v1 <<= 16;
    div(v1, a1);
    if (a1 != 0) goto loc_8001B964;
    _break(0x1C00);
loc_8001B964:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at)
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001B97C;
    }
    if (v1 != at) goto loc_8001B97C;
    tge(zero, zero, 0x5D);
loc_8001B97C:
    s0 = lo;
loc_8001B980:
    v0 = 1;                                             // Result = 00000001
    if (i32(s0) < 0) goto loc_8001B9DC;
    v0 = lw(gp + 0xCC8);                                // Load from: gCloseDist (800782A8)
    v0 = (i32(v0) < i32(s0));
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9DC;
    }
    v0 = lw(s1 + 0x14);
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9D4;
    }
    a0 = s1;
    P_LineOpening();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D84);                               // Load from: gOpenRange (8007827C)
    {
        const bool bJump = (i32(v0) <= 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9D4;
    }
loc_8001B9CC:
    v0 = 1;                                             // Result = 00000001
    goto loc_8001B9DC;
loc_8001B9D4:
    sw(s1, gp + 0xC94);                                 // Store to: gpCloseLine (80078274)
    sw(s0, gp + 0xCC8);                                 // Store to: gCloseDist (800782A8)
loc_8001B9DC:
    ra = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void P_UseLines() noexcept {
loc_8001B9F4:
    sp -= 0x38;
    sw(s5, sp + 0x2C);
    s5 = a0;
    sw(ra, sp + 0x30);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    a0 = lw(s5);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v0 = lw(a0 + 0x24);
    a2 = lw(a0);
    a3 = lw(a0 + 0x4);
    v0 >>= 19;
    v0 <<= 2;
    v1 += v0;
    v1 = lw(v1);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v0;
    a0 = lw(at);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a2, at - 0x78B8);                                // Store to: gUseLine[0] (800A8748)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x78B4);                                // Store to: gUseLine[1] (800A874C)
    v0 = v1 << 3;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 1;
    a1 = a2 + v0;
    v1 = a1 - a2;
    v0 = a0 << 3;
    v0 += a0;
    v0 <<= 2;
    v0 -= a0;
    v0 <<= 1;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v1, at - 0x78B0);                                // Store to: gUseLine[2] (800A8750)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x78AC);                                // Store to: gUseLine[3] (800A8754)
    a0 = a3 + v0;
    if (i32(v1) <= 0) goto loc_8001BAC0;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a1, at - 0x7898);                                // Store to: gUseBBox[3] (800A8768)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a2, at - 0x789C);                                // Store to: gUseBBox[2] (800A8764)
    goto loc_8001BAD0;
loc_8001BAC0:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a2, at - 0x7898);                                // Store to: gUseBBox[3] (800A8768)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a1, at - 0x789C);                                // Store to: gUseBBox[2] (800A8764)
loc_8001BAD0:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78AC);                               // Load from: gUseLine[3] (800A8754)
    if (i32(v0) <= 0) goto loc_8001BAFC;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a0, at - 0x78A4);                                // Store to: gUseBBox[0] (800A875C)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x78A0);                                // Store to: gUseBBox[1] (800A8760)
    goto loc_8001BB0C;
loc_8001BAFC:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x78A4);                                // Store to: gUseBBox[0] (800A875C)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a0, at - 0x78A0);                                // Store to: gUseBBox[1] (800A8760)
loc_8001BB0C:
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7E6C);                               // Load from: gBlockmapOriginY (80078194)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A0);                               // Load from: gUseBBox[1] (800A8760)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A4);                               // Load from: gUseBBox[0] (800A875C)
    v1 = 0x10000;                                       // Result = 00010000
    sw(v1, gp + 0xCC8);                                 // Store to: gCloseDist (800782A8)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x7898);                               // Load from: gUseBBox[3] (800A8768)
    sw(0, gp + 0xC94);                                  // Store to: gpCloseLine (80078274)
    v0 -= a1;
    s1 = u32(i32(v0) >> 23);
    a0 -= a1;
    s3 = u32(i32(a0) >> 23);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7E74);                               // Load from: gBlockmapOriginX (8007818C)
    v0++;
    v1 -= a1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x789C);                               // Load from: gUseBBox[2] (800A8764)
    v0 -= a1;
    s4 = u32(i32(v0) >> 23);
    v0 = (i32(s3) < i32(s1));
    s2 = u32(i32(v1) >> 23);
    if (v0 != 0) goto loc_8001BBC8;
    v0 = (i32(s2) < i32(s4));
loc_8001BB8C:
    s0 = s4;
    if (v0 != 0) goto loc_8001BBB8;
    a0 = s0;
loc_8001BB98:
    a2 = 0x80020000;                                    // Result = 80020000
    a2 -= 0x47B8;                                       // Result = PIT_UseLines (8001B848)
    a1 = s1;
    P_BlockLinesIterator();
    s0++;
    v0 = (i32(s2) < i32(s0));
    a0 = s0;
    if (v0 == 0) goto loc_8001BB98;
loc_8001BBB8:
    s1++;
    v0 = (i32(s3) < i32(s1));
    {
        const bool bJump = (v0 == 0)
        v0 = (i32(s2) < i32(s4));
        if (bJump) goto loc_8001BB8C;
    }
loc_8001BBC8:
    a1 = lw(gp + 0xC94);                                // Load from: gpCloseLine (80078274)
    if (a1 == 0) goto loc_8001BC08;
    v0 = lw(a1 + 0x14);
    if (v0 != 0) goto loc_8001BBFC;
    a0 = lw(s5);
    a1 = 0x1C;                                          // Result = 0000001C
    S_StartSound();
    goto loc_8001BC08;
loc_8001BBFC:
    a0 = lw(s5);
    P_UseSpecialLine();
loc_8001BC08:
    ra = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x38;
    return;
}

void PIT_RadiusAttack() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x64);
    v0 &= 4;
    {
        const bool bJump = (v0 == 0)
        v0 = 0x11;                                      // Result = 00000011
        if (bJump) goto loc_8001BD08;
    }
    v1 = lw(s1 + 0x54);
    {
        const bool bJump = (v1 == v0)
        v0 = 0xF;                                       // Result = 0000000F
        if (bJump) goto loc_8001BD08;
    }
    {
        const bool bJump = (v1 == v0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001BD0C;
    }
    a1 = lw(gp + 0xBC0);                                // Load from: gpBombSpot (800781A0)
    v1 = lw(s1);
    v0 = lw(a1);
    a0 = lw(a1 + 0x4);
    v1 -= v0;
    v0 = lw(s1 + 0x4);
    s0 = v1;
    if (i32(v1) >= 0) goto loc_8001BC94;
    s0 = -s0;
loc_8001BC94:
    v0 -= a0;
    v1 = v0;
    if (i32(v0) >= 0) goto loc_8001BCA4;
    v1 = -v1;
loc_8001BCA4:
    v0 = (i32(s0) < i32(v1));
    if (v0 == 0) goto loc_8001BCB4;
    s0 = v1;
loc_8001BCB4:
    v0 = lw(s1 + 0x40);
    v0 = s0 - v0;
    s0 = u32(i32(v0) >> 16);
    if (i32(s0) >= 0) goto loc_8001BCD0;
    s0 = 0;                                             // Result = 00000000
loc_8001BCD0:
    v0 = lw(gp + 0x8B4);                                // Load from: gBombDamage (80077E94)
    v0 = (i32(s0) < i32(v0));
    {
        const bool bJump = (v0 == 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001BD0C;
    }
    a0 = s1;
    P_CheckSight();
    a0 = s1;
    if (v0 == 0) goto loc_8001BD08;
    a1 = lw(gp + 0xBC0);                                // Load from: gpBombSpot (800781A0)
    a3 = lw(gp + 0x8B4);                                // Load from: gBombDamage (80077E94)
    a2 = lw(gp + 0x910);                                // Load from: gpBombSource (80077EF0)
    a3 -= s0;
    P_DamageMObj();
loc_8001BD08:
    v0 = 1;                                             // Result = 00000001
loc_8001BD0C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_RadiusAttack() noexcept {
loc_8001BD24:
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v1 = lw(a0 + 0x4);
    sw(a1, gp + 0x910);                                 // Store to: gpBombSource (80077EF0)
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7E6C);                               // Load from: gBlockmapOriginY (80078194)
    sw(a2, gp + 0x8B4);                                 // Store to: gBombDamage (80077E94)
    a2 <<= 16;
    sw(a0, gp + 0xBC0);                                 // Store to: gpBombSpot (800781A0)
    v0 = v1 - a2;
    v0 -= a1;
    s1 = u32(i32(v0) >> 23);
    v1 += a2;
    v1 -= a1;
    s3 = u32(i32(v1) >> 23);
    v1 = lw(a0);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E74);                               // Load from: gBlockmapOriginX (8007818C)
    v0 = a2 + v1;
    v0 -= a0;
    s2 = u32(i32(v0) >> 23);
    v1 -= a2;
    v1 -= a0;
    v0 = (i32(s3) < i32(s1));
    s4 = u32(i32(v1) >> 23);
    if (v0 != 0) goto loc_8001BDE0;
    s0 = s4;
loc_8001BDA4:
    v0 = (i32(s2) < i32(s0));
    a0 = s0;
    if (v0 != 0) goto loc_8001BDD0;
loc_8001BDB0:
    a2 = 0x80020000;                                    // Result = 80020000
    a2 -= 0x43D0;                                       // Result = PIT_RadiusAttack (8001BC30)
    a1 = s1;
    P_BlockThingsIterator();
    s0++;
    v0 = (i32(s2) < i32(s0));
    a0 = s0;
    if (v0 == 0) goto loc_8001BDB0;
loc_8001BDD0:
    s1++;
    v0 = (i32(s3) < i32(s1));
    s0 = s4;
    if (v0 == 0) goto loc_8001BDA4;
loc_8001BDE0:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_AimLineAttack() noexcept {
loc_8001BE04:
    sp -= 0x18;
    a3 = 0xFFFF0000;                                    // Result = FFFF0000
    a3 |= 0x6000;                                       // Result = FFFF6000
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    v0 = 0xA000;                                        // Result = 0000A000
    sw(ra, sp + 0x10);
    sw(a0, gp + 0xAD4);                                 // Store to: gpShooter (800780B4)
    sw(a2, gp + 0x9B8);                                 // Store to: gAttackRange (80077F98)
    sw(a1, gp + 0x9A0);                                 // Store to: gAttackAngle (80077F80)
    sw(v0, gp + 0xA18);                                 // Store to: gAimTopSlope (80077FF8)
    sw(a3, gp + 0xD18);                                 // Store to: gAimBottomSlope (800782F8)
    v1++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    P_Shoot2();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D2C);                               // Load from: gpShootMObj (800782D4)
    sw(v0, gp + 0x908);                                 // Store to: gpLineTarget (80077EE8)
    {
        const bool bJump = (v0 == 0)
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001BE68;
    }
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F4C);                               // Load from: gShootSlope (80077F4C)
loc_8001BE68:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_LineAttack() noexcept {
loc_8001BE78:
    sp -= 0x30;
    sw(s4, sp + 0x20);
    s4 = a0;
    v0 = 0x7FFF0000;                                    // Result = 7FFF0000
    sw(s5, sp + 0x24);
    s5 = lw(sp + 0x40);
    v0 |= 0xFFFF;                                       // Result = 7FFFFFFF
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    sw(s4, gp + 0xAD4);                                 // Store to: gpShooter (800780B4)
    sw(a2, gp + 0x9B8);                                 // Store to: gAttackRange (80077F98)
    sw(a1, gp + 0x9A0);                                 // Store to: gAttackAngle (80077F80)
    {
        const bool bJump = (a3 != v0)
        v0 = a3 + 1;
        if (bJump) goto loc_8001BED8;
    }
    v1 = 0xFFFF0000;                                    // Result = FFFF0000
    v1 |= 0x6000;                                       // Result = FFFF6000
    v0 = 0xA000;                                        // Result = 0000A000
    sw(v0, gp + 0xA18);                                 // Store to: gAimTopSlope (80077FF8)
    sw(v1, gp + 0xD18);                                 // Store to: gAimBottomSlope (800782F8)
    goto loc_8001BEE4;
loc_8001BED8:
    sw(v0, gp + 0xA18);                                 // Store to: gAimTopSlope (80077FF8)
    v0 = a3 - 1;
    sw(v0, gp + 0xD18);                                 // Store to: gAimBottomSlope (800782F8)
loc_8001BEE4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    P_Shoot2();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D2C);                               // Load from: gpShootMObj (800782D4)
    s0 = 0x80080000;                                    // Result = 80080000
    s0 = lw(s0 - 0x7D30);                               // Load from: gpShootLine (800782D0)
    s2 = 0x80070000;                                    // Result = 80070000
    s2 = lw(s2 + 0x7FC4);                               // Load from: gShootX (80077FC4)
    s3 = 0x80070000;                                    // Result = 80070000
    s3 = lw(s3 + 0x7FD0);                               // Load from: gShootY (80077FD0)
    s1 = 0x80070000;                                    // Result = 80070000
    s1 = lw(s1 + 0x7FD4);                               // Load from: gShootZ (80077FD4)
    sw(v0, gp + 0x908);                                 // Store to: gpLineTarget (80077EE8)
    v1 = 0x80000;                                       // Result = 00080000
    if (v0 == 0) goto loc_8001BF8C;
    v0 = lw(v0 + 0x64);
    v0 &= v1;
    a0 = s2;
    if (v0 == 0) goto loc_8001BF60;
    a1 = s3;
    a2 = s1;
    P_SpawnPuff();
    goto loc_8001BF70;
loc_8001BF60:
    a1 = s3;
    a2 = s1;
    a3 = s5;
    P_SpawnBlood();
loc_8001BF70:
    a0 = lw(gp + 0x908);                                // Load from: gpLineTarget (80077EE8)
    a1 = s4;
    a2 = a1;
    a3 = s5;
    P_DamageMObj();
    goto loc_8001C008;
loc_8001BF8C:
    if (s0 == 0) goto loc_8001C008;
    v0 = lw(s0 + 0x14);
    a0 = s4;
    if (v0 == 0) goto loc_8001BFAC;
    a1 = s0;
    P_ShootSpecialLine();
loc_8001BFAC:
    v1 = lw(s0 + 0x38);
    a0 = lw(v1 + 0xC);
    v0 = -1;                                            // Result = FFFFFFFF
    if (a0 != v0) goto loc_8001BFF8;
    v0 = lw(v1 + 0x4);
    v0 = (i32(v0) < i32(s1));
    if (v0 != 0) goto loc_8001C008;
    a1 = lw(s0 + 0x3C);
    if (a1 == 0) goto loc_8001BFF8;
    v0 = lw(a1 + 0xC);
    if (v0 == a0) goto loc_8001C008;
loc_8001BFF8:
    a0 = s2;
    a1 = s3;
    a2 = s1;
    P_SpawnPuff();
loc_8001C008:
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
