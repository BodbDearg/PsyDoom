#include "p_shoot.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_fixed.h"
#include "p_setup.h"
#include "PsxVm/PsxVm.h"

void P_Shoot2() noexcept {
loc_80023C34:
    t0 = 0x80080000;                                    // Result = 80080000
    t0 = lw(t0 - 0x7F4C);                               // Load from: gpShooter (800780B4)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7F80);                               // Load from: gAttackAngle (80077F80)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    t1 = 0x80070000;                                    // Result = 80070000
    t1 = lh(t1 + 0x7F9A);                               // Load from: gAttackRange + 2 (80077F9A) (80077F9A)
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6F8C;                                       // Result = gShootDiv[0] (800A9074)
    sw(ra, sp + 0x14);
    a1 >>= 19;
    a2 = lw(t0);
    a1 <<= 2;
    sw(a2, s0);                                         // Store to: gShootDiv[0] (800A9074)
    a3 = lw(t0 + 0x4);
    v0 += a1;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x6F88);                                // Store to: gShootDiv[1] (800A9078)
    v0 = lw(v0);
    mult(t1, v0);
    a0 = *gNumBspNodes;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D08);                               // Load from: gAimBottomSlope (800782F8)
    sw(0, gp + 0xCF0);                                  // Store to: gpShootLine (800782D0)
    sw(0, gp + 0xCF4);                                  // Store to: gpShootMObj (800782D4)
    sw(0, gp + 0xB4C);                                  // Store to: gOldFrac (8007812C)
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7FF8);                               // Load from: gAimTopSlope (80077FF8)
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += a1;
    a1 = lw(at);
    v0 += v1;
    v0 = u32(i32(v0) >> 1);
    v1 = lo;
    sw(v0, gp + 0x9CC);                                 // Store to: gAimMidSlope (80077FAC)
    v0 = u32(i32(a2) >> 16);
    mult(t1, a1);
    sw(v0, gp + 0xC10);                                 // Store to: gSsx1 (800781F0)
    a1 = lw(t0);
    v0 = u32(i32(a3) >> 16);
    sw(v0, gp + 0xC20);                                 // Store to: gSsy1 (80078200)
    v1 += a1;
    a1 = lw(t0 + 0x4);
    a2 = v1 - a2;
    sw(v1, gp + 0xA58);                                 // Store to: gShootX2 (80078038)
    v1 = u32(i32(v1) >> 16);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a2, at - 0x6F84);                                // Store to: gShootDiv[2] (800A907C)
    sw(v1, gp + 0xC1C);                                 // Store to: gSsx2 (800781FC)
    v0 = lo;
    v0 += a1;
    a3 = v0 - a3;
    a2 ^= a3;
    sw(v0, gp + 0xA64);                                 // Store to: gShootY2 (80078044)
    v0 = u32(i32(v0) >> 16);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x6F80);                                // Store to: gShootDiv[3] (800A9080)
    sw(v0, gp + 0xC2C);                                 // Store to: gSsy2 (8007820C)
    v0 = lw(t0 + 0x44);
    v1 = lw(t0 + 0x8);
    a2 = (i32(a2) > 0);
    sw(a2, gp + 0xA8C);                                 // Store to: gbShootDivPositive (8007806C)
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    v1 = 0x80000;                                       // Result = 00080000
    v0 += v1;
    sw(v0, gp + 0x9F4);                                 // Store to: gShootZ (80077FD4)
    a0--;
    PA_CrossBSPNode();
    v0 = lw(gp + 0xCF4);                                // Load from: gpShootMObj (800782D4)
    a0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80023E28;
    a1 = 0;                                             // Result = 00000000
    a2 = 0x10000;                                       // Result = 00010000
    PA_DoIntercept();
    v0 = lw(gp + 0xCF4);                                // Load from: gpShootMObj (800782D4)
    if (v0 != 0) goto loc_80023E28;
    v0 = lw(gp + 0xCF0);                                // Load from: gpShootLine (800782D0)
    if (v0 == 0) goto loc_80023E28;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7F98);                               // Load from: gAttackRange (80077F98)
    a0 = 0x40000;                                       // Result = 00040000
    _thunk_FixedDiv();
    a1 = lw(gp + 0xBF0);                                // Load from: gFirstLineFrac (800781D0)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x6F84);                               // Load from: gShootDiv[2] (800A907C)
    a1 -= v0;
    sw(a1, gp + 0xBF0);                                 // Store to: gFirstLineFrac (800781D0)
    _thunk_FixedMul();
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x6F80);                               // Load from: gShootDiv[3] (800A9080)
    v1 = lw(s0);                                        // Load from: gShootDiv[0] (800A9074)
    a1 = lw(gp + 0xBF0);                                // Load from: gFirstLineFrac (800781D0)
    v0 += v1;
    sw(v0, gp + 0x9E4);                                 // Store to: gShootX (80077FC4)
    _thunk_FixedMul();
    a0 = lw(gp + 0xBF0);                                // Load from: gFirstLineFrac (800781D0)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6F88);                               // Load from: gShootDiv[1] (800A9078)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7F98);                               // Load from: gAttackRange (80077F98)
    v0 += v1;
    sw(v0, gp + 0x9F0);                                 // Store to: gShootY (80077FD0)
    _thunk_FixedMul();
    a0 = lw(gp + 0x9CC);                                // Load from: gAimMidSlope (80077FAC)
    a1 = v0;
    _thunk_FixedMul();
    v1 = lw(gp + 0x9F4);                                // Load from: gShootZ (80077FD4)
    v0 += v1;
    sw(v0, gp + 0x9F4);                                 // Store to: gShootZ (80077FD4)
loc_80023E28:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PA_DoIntercept() noexcept {
loc_80023E3C:
    v1 = a1;
    a1 = a2;
    a2 = lw(gp + 0xB4C);                                // Load from: gOldFrac (8007812C)
    sp -= 0x18;
    v0 = (i32(a2) < i32(a1));
    sw(ra, sp + 0x10);
    if (v0 == 0) goto loc_80023E78;
    v0 = lw(gp + 0xC80);                                // Load from: gpOldValue (80078260)
    sw(a0, gp + 0xC80);                                 // Store to: gpOldValue (80078260)
    sw(a1, gp + 0xB4C);                                 // Store to: gOldFrac (8007812C)
    a0 = v0;
    v0 = lw(gp + 0x8E8);                                // Load from: gbOld_isLine (80077EC8)
    a1 = a2;
    sw(v1, gp + 0x8E8);                                 // Store to: gbOld_isLine (80077EC8)
    v1 = v0;
loc_80023E78:
    v0 = 0xFFFF;                                        // Result = 0000FFFF
    if (a1 == 0) goto loc_80023E8C;
    v0 = (i32(v0) < i32(a1));
    if (v0 == 0) goto loc_80023E94;
loc_80023E8C:
    v0 = 1;                                             // Result = 00000001
    goto loc_80023EB4;
loc_80023E94:
    if (v1 != 0) goto loc_80023EAC;
    PA_ShootThing();
    goto loc_80023EB4;
loc_80023EAC:
    PA_ShootLine();
loc_80023EB4:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PA_ShootLine() noexcept {
loc_80023EC4:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x10);
    v0 &= 4;
    s1 = a1;
    if (v0 != 0) goto loc_80023F14;
    v0 = lw(gp + 0xCF0);                                // Load from: gpShootLine (800782D0)
    if (v0 != 0) goto loc_80023F08;
    sw(s0, gp + 0xCF0);                                 // Store to: gpShootLine (800782D0)
    sw(s1, gp + 0xBF0);                                 // Store to: gFirstLineFrac (800781D0)
loc_80023F08:
    sw(0, gp + 0xB4C);                                  // Store to: gOldFrac (8007812C)
    v0 = 0;                                             // Result = 00000000
    goto loc_800240A0;
loc_80023F14:
    a1 = lw(s0 + 0x38);
    a2 = lw(s0 + 0x3C);
    a0 = lw(a1 + 0x4);
    v1 = lw(a2 + 0x4);
    v0 = (i32(a0) < i32(v1));
    if (v0 == 0) goto loc_80023F44;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0x7F44);                                // Store to: gOpenTop (800780BC)
    goto loc_80023F4C;
loc_80023F44:
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7F44);                                // Store to: gOpenTop (800780BC)
loc_80023F4C:
    a0 = lw(a1);
    v1 = lw(a2);
    v0 = (i32(v1) < i32(a0));
    if (v0 == 0) goto loc_80023F74;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x7F30);                                // Store to: gOpenBottom (80077F30)
    goto loc_80023F7C;
loc_80023F74:
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x7F30);                                // Store to: gOpenBottom (80077F30)
loc_80023F7C:
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7F98);                               // Load from: gAttackRange (80077F98)
    a1 = s1;
    _thunk_FixedMul();
    v1 = lw(s0 + 0x38);
    a0 = lw(s0 + 0x3C);
    a1 = lw(v1);
    v1 = lw(a0);
    s2 = v0;
    if (a1 == v1) goto loc_8002400C;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F30);                               // Load from: gOpenBottom (80077F30)
    a0 = lw(gp + 0x9F4);                                // Load from: gShootZ (80077FD4)
    a1 = s2;
    a0 = v0 - a0;
    _thunk_FixedDiv();
    v1 = lw(gp + 0x9CC);                                // Load from: gAimMidSlope (80077FAC)
    a0 = v0;
    v1 = (i32(a0) < i32(v1));
    if (v1 != 0) goto loc_80023FEC;
    v0 = lw(gp + 0xCF0);                                // Load from: gpShootLine (800782D0)
    if (v0 != 0) goto loc_80023FEC;
    sw(s0, gp + 0xCF0);                                 // Store to: gpShootLine (800782D0)
    sw(s1, gp + 0xBF0);                                 // Store to: gFirstLineFrac (800781D0)
loc_80023FEC:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D08);                               // Load from: gAimBottomSlope (800782F8)
    v0 = (i32(v0) < i32(a0));
    if (v0 == 0) goto loc_8002400C;
    at = 0x80080000;                                    // Result = 80080000
    sw(a0, at - 0x7D08);                                // Store to: gAimBottomSlope (800782F8)
loc_8002400C:
    v0 = lw(s0 + 0x38);
    v1 = lw(s0 + 0x3C);
    a0 = lw(v0 + 0x4);
    v0 = lw(v1 + 0x4);
    a1 = s2;
    if (a0 == v0) goto loc_80024088;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F44);                               // Load from: gOpenTop (800780BC)
    a0 = lw(gp + 0x9F4);                                // Load from: gShootZ (80077FD4)
    a0 = v0 - a0;
    _thunk_FixedDiv();
    v1 = lw(gp + 0x9CC);                                // Load from: gAimMidSlope (80077FAC)
    a0 = v0;
    v1 = (i32(v1) < i32(a0));
    if (v1 != 0) goto loc_80024068;
    v0 = lw(gp + 0xCF0);                                // Load from: gpShootLine (800782D0)
    if (v0 != 0) goto loc_80024068;
    sw(s0, gp + 0xCF0);                                 // Store to: gpShootLine (800782D0)
    sw(s1, gp + 0xBF0);                                 // Store to: gFirstLineFrac (800781D0)
loc_80024068:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7FF8);                               // Load from: gAimTopSlope (80077FF8)
    v0 = (i32(a0) < i32(v0));
    if (v0 == 0) goto loc_80024088;
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x7FF8);                                // Store to: gAimTopSlope (80077FF8)
loc_80024088:
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FF8);                               // Load from: gAimTopSlope (80077FF8)
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D08);                               // Load from: gAimBottomSlope (800782F8)
    v0 = (i32(v0) < i32(v1));
loc_800240A0:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void PA_ShootThing() noexcept {
loc_800240BC:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F4C);                               // Load from: gpShooter (800780B4)
    sp -= 0x28;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s3, sp + 0x1C);
    s3 = a1;
    sw(ra, sp + 0x20);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
    if (s1 == v0) goto loc_80024170;
    v0 = lw(s1 + 0x64);
    v0 &= 4;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8002423C;
    }
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7F98);                               // Load from: gAttackRange (80077F98)
    a1 = s3;
    _thunk_FixedMul();
    s2 = v0;
    a1 = s2;
    a0 = lw(s1 + 0x8);
    v0 = lw(s1 + 0x44);
    v1 = lw(gp + 0x9F4);                                // Load from: gShootZ (80077FD4)
    a0 += v0;
    a0 -= v1;
    _thunk_FixedDiv();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D08);                               // Load from: gAimBottomSlope (800782F8)
    s0 = v0;
    v1 = (i32(s0) < i32(v1));
    v0 = 1;                                             // Result = 00000001
    if (v1 != 0) goto loc_8002423C;
    v0 = lw(s1 + 0x8);
    a0 = lw(gp + 0x9F4);                                // Load from: gShootZ (80077FD4)
    a1 = s2;
    a0 = v0 - a0;
    _thunk_FixedDiv();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7FF8);                               // Load from: gAimTopSlope (80077FF8)
    a2 = v0;
    v0 = (i32(v1) < i32(a2));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < i32(s0));
        if (bJump) goto loc_80024178;
    }
loc_80024170:
    v0 = 1;                                             // Result = 00000001
    goto loc_8002423C;
loc_80024178:
    if (v0 == 0) goto loc_80024184;
    s0 = v1;
loc_80024184:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D08);                               // Load from: gAimBottomSlope (800782F8)
    v0 = (i32(a2) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = s0 + a2;
        if (bJump) goto loc_800241A4;
    }
    a2 = v1;
    v0 = s0 + a2;
loc_800241A4:
    v1 = v0 >> 31;
    v0 += v1;
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7F98);                               // Load from: gAttackRange (80077F98)
    v0 = u32(i32(v0) >> 1);
    sw(v0, gp + 0x96C);                                 // Store to: gShootSlope (80077F4C)
    sw(s1, gp + 0xCF4);                                 // Store to: gpShootMObj (800782D4)
    a0 = 0xA0000;                                       // Result = 000A0000
    _thunk_FixedDiv();
    s0 = s3 - v0;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x6F84);                               // Load from: gShootDiv[2] (800A907C)
    a1 = s0;
    _thunk_FixedMul();
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6F8C);                               // Load from: gShootDiv[0] (800A9074)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x6F80);                               // Load from: gShootDiv[3] (800A9080)
    v0 += v1;
    sw(v0, gp + 0x9E4);                                 // Store to: gShootX (80077FC4)
    a1 = s0;
    _thunk_FixedMul();
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6F88);                               // Load from: gShootDiv[1] (800A9078)
    a1 = 0x80070000;                                    // Result = 80070000
    a1 = lw(a1 + 0x7F98);                               // Load from: gAttackRange (80077F98)
    v0 += v1;
    sw(v0, gp + 0x9F0);                                 // Store to: gShootY (80077FD0)
    a0 = s0;
    _thunk_FixedMul();
    a0 = lw(gp + 0x96C);                                // Load from: gShootSlope (80077F4C)
    a1 = v0;
    _thunk_FixedMul();
    v1 = lw(gp + 0x9F4);                                // Load from: gShootZ (80077FD4)
    v1 += v0;
    v0 = 0;                                             // Result = 00000000
    sw(v1, gp + 0x9F4);                                 // Store to: gShootZ (80077FD4)
loc_8002423C:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void PA_SightCrossLine() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v1 = lw(a0 + 0x4);
    a1 = lw(gp + 0xC20);                                // Load from: gSsy1 (80078200)
    t8 = lw(gp + 0xC2C);                                // Load from: gSsy2 (8007820C)
    v0 = lw(gp + 0xC10);                                // Load from: gSsx1 (800781F0)
    t7 = lh(v1 + 0x2);
    a2 = t8 - a1;
    t0 = t7 - v0;
    mult(a2, t0);
    t6 = lw(gp + 0xC1C);                                // Load from: gSsx2 (800781FC)
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
    if (v1 == v0) goto loc_80024320;
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
    _thunk_FixedDiv();
    goto loc_80024324;
loc_80024320:
    v0 = -1;                                            // Result = FFFFFFFF
loc_80024324:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PA_CrossSubsector() noexcept {
loc_80024334:
    sp -= 0x28;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(ra, sp + 0x20);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lw(s3);
    s0 = lw(v0 + 0x4C);
    if (s0 == 0) goto loc_80024594;
    s2 = 0x800B0000;                                    // Result = 800B0000
    s2 -= 0x75BC;                                       // Result = gThingLine_tv1[0] (800A8A44)
    s1 = 0x800B0000;                                    // Result = 800B0000
    s1 -= 0x75A0;                                       // Result = gThingLine_tv2[0] (800A8A60)
loc_80024378:
    v0 = lw(s0 + 0xC);
    if (v0 != s3) goto loc_80024584;
    v0 = lw(gp + 0xA8C);                                // Load from: gbShootDivPositive (8007806C)
    if (v0 == 0) goto loc_800243E4;
    v0 = lw(s0);
    v1 = lw(s0 + 0x40);
    v0 -= v1;
    sw(v0, s2);                                         // Store to: gThingLine_tv1[0] (800A8A44)
    v0 = lw(s0 + 0x4);
    v1 = lw(s0 + 0x40);
    v0 += v1;
    sw(v0, s2 + 0x4);                                   // Store to: gThingLine_tv1[1] (800A8A48)
    v0 = lw(s0);
    v1 = lw(s0 + 0x40);
    v0 += v1;
    sw(v0, s1);                                         // Store to: gThingLine_tv2[0] (800A8A60)
    v0 = lw(s0 + 0x4);
    v1 = lw(s0 + 0x40);
    v0 -= v1;
    goto loc_80024430;
loc_800243E4:
    v0 = lw(s0);
    v1 = lw(s0 + 0x40);
    v0 -= v1;
    sw(v0, s2);                                         // Store to: gThingLine_tv1[0] (800A8A44)
    v0 = lw(s0 + 0x4);
    v1 = lw(s0 + 0x40);
    v0 -= v1;
    sw(v0, s2 + 0x4);                                   // Store to: gThingLine_tv1[1] (800A8A48)
    v0 = lw(s0);
    v1 = lw(s0 + 0x40);
    v0 += v1;
    sw(v0, s1);                                         // Store to: gThingLine_tv2[0] (800A8A60)
    v0 = lw(s0 + 0x4);
    v1 = lw(s0 + 0x40);
    v0 += v1;
loc_80024430:
    sw(v0, s1 + 0x4);                                   // Store to: gThingLine_tv2[1] (800A8A64)
    v0 = lw(gp + 0x538);                                // Load from: gpThingLine_tv2 (80077B18)
    a0 = lw(gp + 0xC20);                                // Load from: gSsy1 (80078200)
    t8 = lw(gp + 0xC2C);                                // Load from: gSsy2 (8007820C)
    v1 = lw(gp + 0xC10);                                // Load from: gSsx1 (800781F0)
    t7 = lh(v0 + 0x2);
    t2 = t8 - a0;
    a3 = t7 - v1;
    mult(t2, a3);
    t6 = lw(gp + 0xC1C);                                // Load from: gSsx2 (800781FC)
    t5 = lh(v0 + 0x6);
    t1 = lo;
    t0 = t6 - v1;
    a2 = t5 - a0;
    mult(a2, t0);
    v0 = lw(gp + 0x534);                                // Load from: gpThingLine_tv1 (80077B14)
    t4 = lh(v0 + 0x2);
    a1 = lo;
    a3 = t4 - v1;
    mult(t2, a3);
    t3 = lh(v0 + 0x6);
    v1 = lo;
    a2 = t3 - a0;
    mult(a2, t0);
    t1 = (i32(t1) < i32(a1));
    v0 = lo;
    v1 = (i32(v1) < i32(v0));
    t0 = t3 - t5;
    if (t1 != v1) goto loc_800244B0;
    a1 = -1;                                            // Result = FFFFFFFF
    goto loc_800244F4;
loc_800244B0:
    mult(t0, a3);
    a0 = lo;
    t2 = t7 - t4;
    mult(t2, a2);
    v0 = lo;
    a3 = t6 - t4;
    mult(t0, a3);
    v1 = lo;
    a2 = t8 - t3;
    mult(t2, a2);
    t1 = a0 + v0;
    a0 = t1;
    v0 = lo;
    v1 += v0;
    a1 = a0 + v1;
    _thunk_FixedDiv();
    a1 = v0;
loc_800244F4:
    v0 = 0x10000;                                       // Result = 00010000
    v0 = (v0 < a1);
    a0 = s0;
    if (v0 != 0) goto loc_80024584;
    a3 = 0;                                             // Result = 00000000
    a2 = lw(gp + 0xB4C);                                // Load from: gOldFrac (8007812C)
    v0 = (i32(a2) < i32(a1));
    v1 = a1;
    if (v0 == 0) goto loc_80024540;
    v0 = lw(gp + 0xC80);                                // Load from: gpOldValue (80078260)
    a0 = v0;
    v0 = lw(gp + 0x8E8);                                // Load from: gbOld_isLine (80077EC8)
    v1 = a2;
    sw(s0, gp + 0xC80);                                 // Store to: gpOldValue (80078260)
    sw(a1, gp + 0xB4C);                                 // Store to: gOldFrac (8007812C)
    sw(0, gp + 0x8E8);                                  // Store to: gbOld_isLine (80077EC8)
    a3 = v0;
loc_80024540:
    v0 = 0xFFFF;                                        // Result = 0000FFFF
    if (v1 == 0) goto loc_80024554;
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002455C;
loc_80024554:
    v0 = 1;                                             // Result = 00000001
    goto loc_8002457C;
loc_8002455C:
    if (a3 == 0) goto loc_80024574;
    a1 = v1;
    PA_ShootLine();
    goto loc_8002457C;
loc_80024574:
    a1 = v1;
    PA_ShootThing();
loc_8002457C:
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80024738;
    }
loc_80024584:
    s0 = lw(s0 + 0x1C);
    if (s0 != 0) goto loc_80024378;
loc_80024594:
    v0 = lh(s3 + 0x6);
    s1 = lh(s3 + 0x4);
    v1 = v0 << 2;
    v1 += v0;
    v0 = *gpSegs;
    v1 <<= 3;
    s2 = v1 + v0;
    if (s1 == 0) goto loc_80024734;
loc_800245B8:
    s0 = lw(s2 + 0x14);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    v0 = lw(s0 + 0x40);
    if (v0 == v1) goto loc_80024728;
    v0 = lw(s0 + 0x4);
    a0 = lw(gp + 0xC20);                                // Load from: gSsy1 (80078200)
    t8 = lw(gp + 0xC2C);                                // Load from: gSsy2 (8007820C)
    sw(v1, s0 + 0x40);
    v1 = lw(gp + 0xC10);                                // Load from: gSsx1 (800781F0)
    t7 = lh(v0 + 0x2);
    t2 = t8 - a0;
    a3 = t7 - v1;
    mult(t2, a3);
    t6 = lw(gp + 0xC1C);                                // Load from: gSsx2 (800781FC)
    t5 = lh(v0 + 0x6);
    t1 = lo;
    t0 = t6 - v1;
    a2 = t5 - a0;
    mult(a2, t0);
    v0 = lw(s0);
    t4 = lh(v0 + 0x2);
    a1 = lo;
    a3 = t4 - v1;
    mult(t2, a3);
    t3 = lh(v0 + 0x6);
    v1 = lo;
    a2 = t3 - a0;
    mult(a2, t0);
    t1 = (i32(t1) < i32(a1));
    v0 = lo;
    v1 = (i32(v1) < i32(v0));
    t0 = t3 - t5;
    if (t1 != v1) goto loc_80024654;
    a1 = -1;                                            // Result = FFFFFFFF
    goto loc_80024698;
loc_80024654:
    mult(t0, a3);
    a0 = lo;
    t2 = t7 - t4;
    mult(t2, a2);
    v0 = lo;
    a3 = t6 - t4;
    mult(t0, a3);
    v1 = lo;
    a2 = t8 - t3;
    mult(t2, a2);
    t1 = a0 + v0;
    a0 = t1;
    v0 = lo;
    v1 += v0;
    a1 = a0 + v1;
    _thunk_FixedDiv();
    a1 = v0;
loc_80024698:
    v0 = 0x10000;                                       // Result = 00010000
    v0 = (v0 < a1);
    a0 = s0;
    if (v0 != 0) goto loc_80024728;
    a2 = lw(gp + 0xB4C);                                // Load from: gOldFrac (8007812C)
    v0 = (i32(a2) < i32(a1));
    v1 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_800246DC;
    v0 = lw(gp + 0xC80);                                // Load from: gpOldValue (80078260)
    sw(a0, gp + 0xC80);                                 // Store to: gpOldValue (80078260)
    sw(a1, gp + 0xB4C);                                 // Store to: gOldFrac (8007812C)
    a0 = v0;
    v0 = lw(gp + 0x8E8);                                // Load from: gbOld_isLine (80077EC8)
    a1 = a2;
    sw(v1, gp + 0x8E8);                                 // Store to: gbOld_isLine (80077EC8)
    v1 = v0;
loc_800246DC:
    v0 = 0xFFFF;                                        // Result = 0000FFFF
    if (a1 == 0) goto loc_800246F0;
    v0 = (i32(v0) < i32(a1));
    if (v0 == 0) goto loc_800246F8;
loc_800246F0:
    v0 = 1;                                             // Result = 00000001
    goto loc_80024718;
loc_800246F8:
    if (v1 == 0) goto loc_80024710;
    PA_ShootLine();
    goto loc_80024718;
loc_80024710:
    PA_ShootThing();
loc_80024718:
    s1--;
    if (v0 != 0) goto loc_8002472C;
    v0 = 0;                                             // Result = 00000000
    goto loc_80024738;
loc_80024728:
    s1--;
loc_8002472C:
    s2 += 0x28;
    if (s1 != 0) goto loc_800245B8;
loc_80024734:
    v0 = 1;                                             // Result = 00000001
loc_80024738:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void PointOnVectorSide() noexcept {
    v0 = lw(a2);
    a0 -= v0;
    v0 = lh(a2 + 0xE);
    a0 = u32(i32(a0) >> 16);
    mult(v0, a0);
    v0 = lw(a2 + 0x4);
    a1 -= v0;
    v1 = lo;
    v0 = lh(a2 + 0xA);
    a1 = u32(i32(a1) >> 16);
    mult(a1, v0);
    v0 = lo;
    v0 = (i32(v0) < i32(v1));
    v0 ^= 1;
    return;
}

void PA_CrossBSPNode() noexcept {
loc_8002479C:
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = a0 & 0x8000;
loc_800247B4:
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFFFF0000;                                // Result = FFFF0000
        if (bJump) goto loc_80024804;
    }
    v0 |= 0x7FFF;                                       // Result = FFFF7FFF
    a2 = *gNumSubsectors;
    s0 = a0 & v0;
    v0 = (i32(s0) < i32(a2));
    if (v0 != 0) goto loc_800247E8;
    I_Error("PA_CrossSubsector: ss %i with numss = %i", (int32_t) s0, (int32_t) a2);
loc_800247E8:
    v0 = *gpSubsectors;
    a0 = s0 << 4;
    a0 += v0;
    PA_CrossSubsector();
    goto loc_800248EC;
loc_80024804:
    v0 = a0 << 3;
    v0 -= a0;
    v1 = *gpBspNodes;
    v0 <<= 3;
    s0 = v0 + v1;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6F8C);                               // Load from: gShootDiv[0] (800A9074)
    v1 = lw(s0);
    v0 -= v1;
    v1 = lh(s0 + 0xE);
    v0 = u32(i32(v0) >> 16);
    mult(v1, v0);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6F88);                               // Load from: gShootDiv[1] (800A9078)
    v1 = lw(s0 + 0x4);
    v0 -= v1;
    a0 = lo;
    v1 = lh(s0 + 0xA);
    v0 = u32(i32(v0) >> 16);
    mult(v0, v1);
    v0 = lo;
    s2 = (i32(v0) < i32(a0));
    s1 = s2 ^ 1;
    v0 = s1 << 2;
    v0 += s0;
    a0 = lw(v0 + 0x30);
    PA_CrossBSPNode();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800248EC;
    }
    v0 = lw(gp + 0xA58);                                // Load from: gShootX2 (80078038)
    v1 = lw(s0);
    v0 -= v1;
    v1 = lh(s0 + 0xE);
    v0 = u32(i32(v0) >> 16);
    mult(v1, v0);
    v0 = lw(gp + 0xA64);                                // Load from: gShootY2 (80078044)
    v1 = lw(s0 + 0x4);
    v0 -= v1;
    a0 = lo;
    v1 = lh(s0 + 0xA);
    v0 = u32(i32(v0) >> 16);
    mult(v0, v1);
    v0 = lo;
    v0 = (i32(v0) < i32(a0));
    v0 ^= 1;
    {
        const bool bJump = (s1 == v0);
        v0 = s2 << 2;
        if (bJump) goto loc_800248E8;
    }
    v0 += s0;
    a0 = lw(v0 + 0x30);
    v0 = a0 & 0x8000;
    goto loc_800247B4;
loc_800248E8:
    v0 = 1;                                             // Result = 00000001
loc_800248EC:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
