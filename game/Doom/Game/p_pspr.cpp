#include "p_pspr.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/w_wad.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_main.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_mobj.h"
#include "PsxVm/PsxVm.h"

void P_RecursiveSound() noexcept {
loc_8001F918:
    sp -= 0x28;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    v0 = lw(s0 + 0x48);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    s2 = a1;
    if (v0 != a0) goto loc_8001F95C;
    v1 = lw(s0 + 0x1C);
    v0 = s2 + 1;
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001FA18;
loc_8001F95C:
    v0 = lw(gp + 0xA1C);                                // Load from: gpSoundTarget (80077FFC)
    s1 = 0;                                             // Result = 00000000
    sw(a0, s0 + 0x48);
    a0 = lw(s0 + 0x54);
    v1 = s2 + 1;
    sw(v1, s0 + 0x1C);
    sw(v0, s0 + 0x20);
    if (i32(a0) <= 0) goto loc_8001FA18;
loc_8001F97C:
    v1 = lw(s0 + 0x58);
    v0 = s1 << 2;
    v0 += v1;
    a2 = lw(v0);
    a1 = lw(a2 + 0x3C);
    if (a1 == 0) goto loc_8001FA04;
    a0 = lw(a2 + 0x38);
    v1 = lw(a1 + 0x4);
    v0 = lw(a0);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001FA04;
    v1 = lw(a0 + 0x4);
    v0 = lw(a1);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001FA04;
    if (a0 != s0) goto loc_8001F9E0;
    a0 = a1;
loc_8001F9E0:
    v0 = lw(a2 + 0x10);
    v0 &= 0x40;
    a1 = s2;
    if (v0 == 0) goto loc_8001F9FC;
    a1 = 1;                                             // Result = 00000001
    if (s2 != 0) goto loc_8001FA04;
loc_8001F9FC:
    P_RecursiveSound();
loc_8001FA04:
    v0 = lw(s0 + 0x54);
    s1++;
    v0 = (i32(s1) < i32(v0));
    if (v0 != 0) goto loc_8001F97C;
loc_8001FA18:
    ra = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void P_NoiseAlert() noexcept {
    sp -= 0x28;
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    v0 = lw(a0);
    v0 = lw(v0 + 0xC);
    s0 = lw(v0);
    v0 = lw(a0 + 0x114);
    if (v0 == s0) goto loc_8001FB58;
    a1 = lw(a0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    sw(s0, a0 + 0x114);
    v1 = lw(s0 + 0x48);
    a0 = v0 + 1;
    sw(a1, gp + 0xA1C);                                 // Store to: gpSoundTarget (80077FFC)
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    s1 = 0;                                             // Result = 00000000
    if (v1 != a0) goto loc_8001FAA8;
    v1 = lw(s0 + 0x1C);
    v0 = 1;                                             // Result = 00000001
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001FB58;
loc_8001FAA8:
    v1 = lw(s0 + 0x54);
    v0 = 1;                                             // Result = 00000001
    sw(a0, s0 + 0x48);
    sw(v0, s0 + 0x1C);
    sw(a1, s0 + 0x20);
    if (i32(v1) <= 0) goto loc_8001FB58;
loc_8001FAC0:
    v1 = lw(s0 + 0x58);
    v0 = s1 << 2;
    v0 += v1;
    a2 = lw(v0);
    a1 = lw(a2 + 0x3C);
    if (a1 == 0) goto loc_8001FB44;
    a0 = lw(a2 + 0x38);
    v1 = lw(a1 + 0x4);
    v0 = lw(a0);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001FB44;
    v1 = lw(a0 + 0x4);
    v0 = lw(a1);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001FB44;
    if (a0 != s0) goto loc_8001FB24;
    a0 = a1;
loc_8001FB24:
    v0 = lw(a2 + 0x10);
    v0 &= 0x40;
    a1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8001FB3C;
    a1 = 1;                                             // Result = 00000001
loc_8001FB3C:
    P_RecursiveSound();
loc_8001FB44:
    v0 = lw(s0 + 0x54);
    s1++;
    v0 = (i32(s1) < i32(v0));
    if (v0 != 0) goto loc_8001FAC0;
loc_8001FB58:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void P_SetPsprite() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    a1 <<= 4;
    a1 += 0xF0;
    sw(s0, sp + 0x10);
    s0 = s1 + a1;
    sw(ra, sp + 0x18);
    goto loc_8001FBF4;
loc_8001FB94:
    v0 -= a2;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_8001FBE0;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_8001FC00;
loc_8001FBE0:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a2 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_8001FC00;
loc_8001FBF4:
    v0 = a2 << 3;
    if (a2 != 0) goto loc_8001FB94;
    sw(0, s0);
loc_8001FC00:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_BringUpWeapon() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v1 = lw(s1 + 0x70);
    v0 = 0xA;                                           // Result = 0000000A
    {
        const bool bJump = (v1 != v0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8001FC50;
    }
    v0 = lw(s1 + 0x6C);
    sw(v0, s1 + 0x70);
    v1 = lw(s1 + 0x70);
    v0 = 8;                                             // Result = 00000008
loc_8001FC50:
    s0 = s1 + 0xF0;
    if (v1 != v0) goto loc_8001FC78;
    v0 = *gbIsLevelDataCached;
    if (v0 == 0) goto loc_8001FC78;
    a0 = lw(s1);
    a1 = 0xB;                                           // Result = 0000000B
    S_StartSound();
loc_8001FC78:
    v1 = lw(s1 + 0x70);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F8;                                       // Result = WeaponInfo_Fist[1] (800670F8)
    at += v0;
    v1 = lw(at);
    v0 = 0xA;                                           // Result = 0000000A
    sw(v0, s1 + 0x70);
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s1 + 0xF8);
    v0 = 0x600000;                                      // Result = 00600000
    a0 = v1;
    sw(v0, s1 + 0xFC);
    if (a0 != 0) goto loc_8001FCC4;
    sw(0, s1 + 0xF0);
    goto loc_8001FD34;
loc_8001FCC4:
    v0 = a0 << 3;
loc_8001FCC8:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_8001FD14;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_8001FD34;
loc_8001FD14:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_8001FD34;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_8001FCC8;
    sw(0, s0);
loc_8001FD34:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_CheckAmmo() noexcept {
loc_8001FD4C:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    a0 = lw(s1 + 0x6C);
    v1 = 7;                                             // Result = 00000007
    v0 = a0 << 1;
    v0 += a0;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v0;
    a1 = lw(at);
    v0 = 3;                                             // Result = 00000003
    if (a0 != v1) goto loc_8001FD94;
    v1 = 0x28;                                          // Result = 00000028
    goto loc_8001FDA0;
loc_8001FD94:
    v1 = 1;                                             // Result = 00000001
    if (a0 != v0) goto loc_8001FDA0;
    v1 = 2;                                             // Result = 00000002
loc_8001FDA0:
    v0 = 5;                                             // Result = 00000005
    {
        const bool bJump = (a1 == v0);
        v0 = a1 << 2;
        if (bJump) goto loc_8001FDC4;
    }
    v0 += s1;
    v0 = lw(v0 + 0x98);
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_8001FDCC;
loc_8001FDC4:
    v0 = 1;                                             // Result = 00000001
    goto loc_8001FFA4;
loc_8001FDCC:
    v0 = lw(s1 + 0x8C);
    if (v0 == 0) goto loc_8001FDF4;
    v0 = lw(s1 + 0xA0);
    {
        const bool bJump = (v0 == 0);
        v0 = 6;                                         // Result = 00000006
        if (bJump) goto loc_8001FDF4;
    }
    sw(v0, s1 + 0x70);
    goto loc_8001FEF8;
loc_8001FDF4:
    v0 = lw(s1 + 0x80);
    if (v0 == 0) goto loc_8001FE20;
    v0 = lw(s1 + 0x9C);
    v0 = (i32(v0) < 3);
    {
        const bool bJump = (v0 != 0);
        v0 = 3;                                         // Result = 00000003
        if (bJump) goto loc_8001FE20;
    }
    sw(v0, s1 + 0x70);
    goto loc_8001FEF8;
loc_8001FE20:
    v0 = lw(s1 + 0x84);
    if (v0 == 0) goto loc_8001FE48;
    v0 = lw(s1 + 0x98);
    {
        const bool bJump = (v0 == 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_8001FE48;
    }
    sw(v0, s1 + 0x70);
    goto loc_8001FEF8;
loc_8001FE48:
    v0 = lw(s1 + 0x7C);
    if (v0 == 0) goto loc_8001FE70;
    v0 = lw(s1 + 0x9C);
    {
        const bool bJump = (v0 == 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8001FE70;
    }
    sw(v0, s1 + 0x70);
    goto loc_8001FEF8;
loc_8001FE70:
    v0 = lw(s1 + 0x98);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001FE88;
    }
    sw(v0, s1 + 0x70);
    goto loc_8001FEF8;
loc_8001FE88:
    v0 = lw(s1 + 0x94);
    {
        const bool bJump = (v0 == 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_8001FEA0;
    }
    sw(v0, s1 + 0x70);
    goto loc_8001FEF8;
loc_8001FEA0:
    v0 = lw(s1 + 0x88);
    if (v0 == 0) goto loc_8001FEC8;
    v0 = lw(s1 + 0xA4);
    {
        const bool bJump = (v0 == 0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_8001FEC8;
    }
    sw(v0, s1 + 0x70);
    goto loc_8001FEF8;
loc_8001FEC8:
    v0 = lw(s1 + 0x90);
    if (v0 == 0) goto loc_8001FEF4;
    v0 = lw(s1 + 0xA0);
    v0 = (i32(v0) < 0x29);
    {
        const bool bJump = (v0 != 0);
        v0 = 7;                                         // Result = 00000007
        if (bJump) goto loc_8001FEF4;
    }
    sw(v0, s1 + 0x70);
    goto loc_8001FEF8;
loc_8001FEF4:
    sw(0, s1 + 0x70);
loc_8001FEF8:
    v1 = lw(s1 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70FC;                                       // Result = WeaponInfo_Fist[2] (800670FC)
    at += v0;
    a0 = lw(at);
    s0 = s1 + 0xF0;
    if (a0 != 0) goto loc_8001FF30;
    sw(0, s1 + 0xF0);
    goto loc_8001FFA0;
loc_8001FF30:
    v0 = a0 << 3;
loc_8001FF34:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_8001FF80;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001FFA4;
    }
loc_8001FF80:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_8001FFA0;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_8001FF34;
    sw(0, s0);
loc_8001FFA0:
    v0 = 0;                                             // Result = 00000000
loc_8001FFA4:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_FireWeapon() noexcept {
loc_8001FFBC:
    sp -= 0x28;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(ra, sp + 0x20);
    sw(s0, sp + 0x18);
    P_CheckAmmo();
    if (v0 == 0) goto loc_800201AC;
    a0 = lw(s1);
    a1 = 0x9F;                                          // Result = 0000009F
    P_SetMObjState();
    v1 = lw(s1 + 0x6C);
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s1 + 0xF8);
    sw(0, s1 + 0xFC);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7104;                                       // Result = WeaponInfo_Fist[4] (80067104)
    at += v0;
    a0 = lw(at);
    s0 = s1 + 0xF0;
    if (a0 != 0) goto loc_80020028;
    sw(0, s1 + 0xF0);
    goto loc_80020098;
loc_80020028:
    v0 = a0 << 3;
loc_8002002C:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_80020078;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_80020098;
loc_80020078:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_80020098;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_8002002C;
    sw(0, s0);
loc_80020098:
    v0 = lw(s1);
    v0 = lw(v0 + 0xC);
    s0 = lw(v0);
    v0 = lw(s1 + 0x114);
    if (v0 == s0) goto loc_800201AC;
    a1 = lw(s1);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    sw(s0, s1 + 0x114);
    v1 = lw(s0 + 0x48);
    a0 = v0 + 1;
    sw(a1, gp + 0xA1C);                                 // Store to: gpSoundTarget (80077FFC)
    at = 0x80070000;                                    // Result = 80070000
    sw(a0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    s1 = 0;                                             // Result = 00000000
    if (v1 != a0) goto loc_800200FC;
    v1 = lw(s0 + 0x1C);
    v0 = 1;                                             // Result = 00000001
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_800201AC;
loc_800200FC:
    v1 = lw(s0 + 0x54);
    v0 = 1;                                             // Result = 00000001
    sw(a0, s0 + 0x48);
    sw(v0, s0 + 0x1C);
    sw(a1, s0 + 0x20);
    if (i32(v1) <= 0) goto loc_800201AC;
loc_80020114:
    v1 = lw(s0 + 0x58);
    v0 = s1 << 2;
    v0 += v1;
    a2 = lw(v0);
    a1 = lw(a2 + 0x3C);
    if (a1 == 0) goto loc_80020198;
    a0 = lw(a2 + 0x38);
    v1 = lw(a1 + 0x4);
    v0 = lw(a0);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_80020198;
    v1 = lw(a0 + 0x4);
    v0 = lw(a1);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_80020198;
    if (a0 != s0) goto loc_80020178;
    a0 = a1;
loc_80020178:
    v0 = lw(a2 + 0x10);
    v0 &= 0x40;
    a1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80020190;
    a1 = 1;                                             // Result = 00000001
loc_80020190:
    P_RecursiveSound();
loc_80020198:
    v0 = lw(s0 + 0x54);
    s1++;
    v0 = (i32(s1) < i32(v0));
    if (v0 != 0) goto loc_80020114;
loc_800201AC:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void P_DropWeapon() noexcept {
loc_800201C4:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v1 = lw(s1 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70FC;                                       // Result = WeaponInfo_Fist[2] (800670FC)
    at += v0;
    a0 = lw(at);
    s0 = s1 + 0xF0;
    if (a0 != 0) goto loc_80020210;
    sw(0, s1 + 0xF0);
    goto loc_80020280;
loc_80020210:
    v0 = a0 << 3;
loc_80020214:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_80020260;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_80020280;
loc_80020260:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_80020280;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_80020214;
    sw(0, s0);
loc_80020280:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_WeaponReady() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s0, sp + 0x10);
    sw(ra, sp + 0x18);
    v1 = lw(s1 + 0x6C);
    v0 = 8;                                             // Result = 00000008
    s0 = a1;
    if (v1 != v0) goto loc_800202DC;
    v1 = lw(s0);
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x6B20;                                       // Result = State_S_SAW[0] (800594E0)
    if (v1 != v0) goto loc_800202DC;
    a0 = lw(s1);
    a1 = 0xC;                                           // Result = 0000000C
    S_StartSound();
loc_800202DC:
    v1 = lw(s1 + 0x70);
    v0 = 0xA;                                           // Result = 0000000A
    if (v1 != v0) goto loc_800202FC;
    v0 = lw(s1 + 0x24);
    if (v0 != 0) goto loc_800203A8;
loc_800202FC:
    v1 = lw(s1 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70FC;                                       // Result = WeaponInfo_Fist[2] (800670FC)
    at += v0;
    a0 = lw(at);
    s0 = s1 + 0xF0;
    if (a0 != 0) goto loc_80020334;
    sw(0, s1 + 0xF0);
    goto loc_80020468;
loc_80020334:
    v0 = a0 << 3;
loc_80020338:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_80020384;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_80020468;
loc_80020384:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_80020468;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_80020338;
    sw(0, s0);
    goto loc_80020468;
loc_800203A8:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D14);                               // Load from: gPlayerNum (800782EC)
    v0 <<= 2;
    at = ptrToVmAddr(&gpPlayerBtnBindings[0]);
    at += v0;
    v1 = lw(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gPlayerPadButtons[0] (80077F44)
    at += v0;
    v0 = lw(at);
    v1 = lw(v1);
    v0 &= v1;
    if (v0 == 0) goto loc_800203FC;
    a0 = s1;
    P_FireWeapon();
    goto loc_80020468;
loc_800203FC:
    v1 = *gTicCon;
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v1 <<= 6;
    a1 = v1 & 0x1FFF;
    v0 = a1 << 2;
    v0 += a0;
    a0 = lh(s1 + 0x22);
    v0 = lw(v0);
    mult(a0, v0);
    a1 = v1 & 0xFFF;
    v1 = 0x10000;                                       // Result = 00010000
    v0 = lo;
    v0 += v1;
    sw(v0, s0 + 0x8);
    v0 = a1 << 2;
    v1 = lh(s1 + 0x22);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v0;
    v0 = lw(at);
    mult(v1, v0);
    v0 = lo;
    sw(v0, s0 + 0xC);
loc_80020468:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_ReFire() noexcept {
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D14);                               // Load from: gPlayerNum (800782EC)
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v0 <<= 2;
    at = ptrToVmAddr(&gpPlayerBtnBindings[0]);
    at += v0;
    v1 = lw(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gPlayerPadButtons[0] (80077F44)
    at += v0;
    v0 = lw(at);
    v1 = lw(v1);
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0xA;                                       // Result = 0000000A
        if (bJump) goto loc_80020504;
    }
    v1 = lw(a0 + 0x70);
    if (v1 != v0) goto loc_80020504;
    v0 = lw(a0 + 0x24);
    if (v0 == 0) goto loc_80020504;
    v0 = lw(a0 + 0xC4);
    v0++;
    sw(v0, a0 + 0xC4);
    P_FireWeapon();
    goto loc_8002050C;
loc_80020504:
    sw(0, a0 + 0xC4);
    P_CheckAmmo();
loc_8002050C:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_CheckReload() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    P_CheckAmmo();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_Lower() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    v0 = 0x5F0000;                                      // Result = 005F0000
    v0 |= 0xFFFF;                                       // Result = 005FFFFF
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v1 = lw(a1 + 0xC);
    a0 = 0xC0000;                                       // Result = 000C0000
    v1 += a0;
    v0 = (i32(v0) < i32(v1));
    sw(v1, a1 + 0xC);
    if (v0 == 0) goto loc_8002069C;
    v1 = lw(s1 + 0x4);
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v0 = 0x600000;                                  // Result = 00600000
        if (bJump) goto loc_80020588;
    }
    sw(v0, a1 + 0xC);
    goto loc_8002069C;
loc_80020588:
    v0 = lw(s1 + 0x24);
    {
        const bool bJump = (v0 == 0);
        v0 = 0xA;                                       // Result = 0000000A
        if (bJump) goto loc_80020624;
    }
    a0 = lw(s1 + 0x70);
    v1 = lw(s1 + 0x70);
    sw(a0, s1 + 0x6C);
    if (v1 != v0) goto loc_800205B0;
    sw(a0, s1 + 0x70);
loc_800205B0:
    v1 = lw(s1 + 0x70);
    v0 = 8;                                             // Result = 00000008
    s0 = s1 + 0xF0;
    if (v1 != v0) goto loc_800205E4;
    v0 = *gbIsLevelDataCached;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 1;
        if (bJump) goto loc_800205EC;
    }
    a0 = lw(s1);
    a1 = 0xB;                                           // Result = 0000000B
    S_StartSound();
    v1 = lw(s1 + 0x70);
loc_800205E4:
    v0 = v1 << 1;
loc_800205EC:
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F8;                                       // Result = WeaponInfo_Fist[1] (800670F8)
    at += v0;
    v1 = lw(at);
    v0 = 0xA;                                           // Result = 0000000A
    sw(v0, s1 + 0x70);
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s1 + 0xF8);
    v0 = 0x600000;                                      // Result = 00600000
    a0 = v1;
    sw(v0, s1 + 0xFC);
    if (a0 != 0) goto loc_8002062C;
loc_80020624:
    sw(0, s1 + 0xF0);
    goto loc_8002069C;
loc_8002062C:
    v0 = a0 << 3;
loc_80020630:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_8002067C;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_8002069C;
loc_8002067C:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_8002069C;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_80020630;
    sw(0, s0);
loc_8002069C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_Raise() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(a1 + 0xC);
    v1 = 0xFFF40000;                                    // Result = FFF40000
    v0 += v1;
    sw(v0, a1 + 0xC);
    if (i32(v0) > 0) goto loc_80020788;
    sw(0, a1 + 0xC);
    v1 = lw(s1 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7100;                                       // Result = WeaponInfo_Fist[3] (80067100)
    at += v0;
    a0 = lw(at);
    s0 = s1 + 0xF0;
    if (a0 != 0) goto loc_80020718;
    sw(0, s1 + 0xF0);
    goto loc_80020788;
loc_80020718:
    v0 = a0 << 3;
loc_8002071C:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_80020768;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_80020788;
loc_80020768:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_80020788;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_8002071C;
    sw(0, s0);
loc_80020788:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_GunFlash() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v1 = lw(s1 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7108;                                       // Result = WeaponInfo_Fist[5] (80067108)
    at += v0;
    a0 = lw(at);
    s0 = s1 + 0x100;
    if (a0 != 0) goto loc_800207EC;
    sw(0, s1 + 0x100);
    goto loc_8002085C;
loc_800207EC:
    v0 = a0 << 3;
loc_800207F0:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_8002083C;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_8002085C;
loc_8002083C:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_8002085C;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_800207F0;
    sw(0, s0);
loc_8002085C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void A_Punch() noexcept {
    sp -= 0x30;
    sw(s3, sp + 0x24);
    s3 = a0;
    sw(ra, sp + 0x28);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    P_Random();
    v0 &= 7;
    v0++;
    v1 = v0 << 1;
    a0 = lw(s3 + 0x34);
    s2 = v1 + v0;
    if (a0 == 0) goto loc_800208BC;
    v0 = s2 << 2;
    v0 += s2;
    s2 = v0 << 1;
loc_800208BC:
    v0 = lw(s3);
    s1 = lw(v0 + 0x24);
    P_Random();
    s0 = v0;
    P_Random();
    a2 = 0x460000;                                      // Result = 00460000
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    s0 -= v0;
    s0 <<= 18;
    sw(s2, sp + 0x10);
    a0 = lw(s3);
    a1 = s1 + s0;
    P_LineAttack();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    if (v0 == 0) goto loc_8002094C;
    a0 = lw(s3);
    a1 = 2;                                             // Result = 00000002
    S_StartSound();
    v0 = lw(s3);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    a0 = lw(v0);
    a1 = lw(v0 + 0x4);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    v1 = lw(s3);
    sw(v0, v1 + 0x24);
loc_8002094C:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_Saw() noexcept {
    sp -= 0x30;
    sw(s3, sp + 0x24);
    s3 = a0;
    sw(ra, sp + 0x28);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    P_Random();
    v0 &= 7;
    v0++;
    s1 = v0 << 1;
    v1 = lw(s3);
    s2 = lw(v1 + 0x24);
    s1 += v0;
    P_Random();
    s0 = v0;
    P_Random();
    a2 = 0x460000;                                      // Result = 00460000
    a2 |= 1;                                            // Result = 00460001
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    s0 -= v0;
    s0 <<= 18;
    sw(s1, sp + 0x10);
    a0 = lw(s3);
    a1 = s2 + s0;
    P_LineAttack();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    if (v0 != 0) goto loc_80020A04;
    a0 = lw(s3);
    a1 = 0xD;                                           // Result = 0000000D
    S_StartSound();
    goto loc_80020AC4;
loc_80020A04:
    a0 = lw(s3);
    a1 = 0xE;                                           // Result = 0000000E
    S_StartSound();
    v0 = lw(s3);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    a0 = lw(v0);
    a1 = lw(v0 + 0x4);
    a2 = lw(v1);
    a3 = lw(v1 + 0x4);
    R_PointToAngle2();
    a2 = lw(s3);
    s2 = v0;
    a1 = lw(a2 + 0x24);
    v0 = 0x80000000;                                    // Result = 80000000
    v1 = s2 - a1;
    v0 = (v0 < v1);
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFCCC0000;                                // Result = FCCC0000
        if (bJump) goto loc_80020A84;
    }
    v0 |= 0xCCCC;                                       // Result = FCCCCCCC
    v0 = (v0 < v1);
    if (v0 != 0) goto loc_80020A74;
    v0 = 0x30C0000;                                     // Result = 030C0000
    v0 |= 0x30C3;                                       // Result = 030C30C3
    v0 += s2;
    goto loc_80020AA8;
loc_80020A74:
    v0 = 0xFCCC0000;                                    // Result = FCCC0000
    v0 |= 0xCCCD;                                       // Result = FCCCCCCD
    v0 += a1;
    goto loc_80020AA8;
loc_80020A84:
    a0 = 0x3330000;                                     // Result = 03330000
    a0 |= 0x3333;                                       // Result = 03333333
    v0 = (a0 < v1);
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFCF30000;                                // Result = FCF30000
        if (bJump) goto loc_80020AA4;
    }
    v0 |= 0xCF3D;                                       // Result = FCF3CF3D
    v0 += s2;
    goto loc_80020AA8;
loc_80020AA4:
    v0 = a1 + a0;
loc_80020AA8:
    sw(v0, a2 + 0x24);
    v1 = lw(s3);
    v0 = lw(v1 + 0x64);
    v0 |= 0x80;
    sw(v0, v1 + 0x64);
loc_80020AC4:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_FireMissile() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v1 = lw(a0 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v0;
    v1 = lw(at);
    v1 <<= 2;
    v1 += a0;
    v0 = lw(v1 + 0x98);
    v0--;
    sw(v0, v1 + 0x98);
    a0 = lw(a0);
    a1 = 0x17;                                          // Result = 00000017
    P_SpawnPlayerMissile();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_FireBFG() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    v1 = lw(a0 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v0;
    v1 = lw(at);
    v1 <<= 2;
    v1 += a0;
    v0 = lw(v1 + 0x98);
    v0 -= 0x28;
    sw(v0, v1 + 0x98);
    a0 = lw(a0);
    a1 = 0x19;                                          // Result = 00000019
    P_SpawnPlayerMissile();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_FirePlasma() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v1 = lw(s1 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v0;
    v1 = lw(at);
    v1 <<= 2;
    v1 += s1;
    v0 = lw(v1 + 0x98);
    v0--;
    sw(v0, v1 + 0x98);
    P_Random();
    a0 = lw(s1 + 0x6C);
    v0 &= 1;
    v1 = a0 << 1;
    v1 += a0;
    v1 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7108;                                       // Result = WeaponInfo_Fist[5] (80067108)
    at += v1;
    v1 = lw(at);
    a0 = v0 + v1;
    s0 = s1 + 0x100;
    if (a0 != 0) goto loc_80020C40;
    sw(0, s1 + 0x100);
    goto loc_80020CB0;
loc_80020C40:
    v0 = a0 << 3;
loc_80020C44:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_80020C90;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_80020CB0;
loc_80020C90:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_80020CB0;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_80020C44;
    sw(0, s0);
loc_80020CB0:
    a0 = lw(s1);
    a1 = 0x18;                                          // Result = 00000018
    P_SpawnPlayerMissile();
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_BulletSlope() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    s0 = lw(s1 + 0x24);
    a2 = 0x4000000;                                     // Result = 04000000
    a1 = s0;
    P_AimLineAttack();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    sw(v0, gp + 0xA10);                                 // Store to: gSlope (80077FF0)
    v0 = 0x4000000;                                     // Result = 04000000
    if (v1 != 0) goto loc_80020D48;
    s0 += v0;
    a0 = s1;
    a1 = s0;
    a2 = 0x4000000;                                     // Result = 04000000
    P_AimLineAttack();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    sw(v0, gp + 0xA10);                                 // Store to: gSlope (80077FF0)
    a0 = s1;
    if (v1 != 0) goto loc_80020D48;
    a1 = 0xF8000000;                                    // Result = F8000000
    a1 += s0;
    a2 = 0x4000000;                                     // Result = 04000000
    P_AimLineAttack();
    sw(v0, gp + 0xA10);                                 // Store to: gSlope (80077FF0)
loc_80020D48:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_GunShot() noexcept {
    sp -= 0x30;
    sw(s3, sp + 0x24);
    s3 = a0;
    sw(s0, sp + 0x18);
    s0 = a1;
    sw(ra, sp + 0x28);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    P_Random();
    v0 &= 3;
    v0++;
    s1 = lw(s3 + 0x24);
    s2 = v0 << 2;
    if (s0 != 0) goto loc_80020DB4;
    P_Random();
    s0 = v0;
    P_Random();
    s0 -= v0;
    s0 <<= 18;
    s1 += s0;
loc_80020DB4:
    sw(s2, sp + 0x10);
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    a0 = s3;
    a1 = s1;
    a2 = 0x8000000;                                     // Result = 08000000
    P_LineAttack();
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_FirePistol() noexcept {
    sp -= 0x30;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s0, sp + 0x18);
    a0 = lw(s1);
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v1 = lw(s1 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v0;
    v1 = lw(at);
    v1 <<= 2;
    v1 += s1;
    v0 = lw(v1 + 0x98);
    v0--;
    sw(v0, v1 + 0x98);
    v1 = lw(s1 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7108;                                       // Result = WeaponInfo_Fist[5] (80067108)
    at += v0;
    a0 = lw(at);
    s0 = s1 + 0x100;
    if (a0 != 0) goto loc_80020E90;
    sw(0, s1 + 0x100);
    goto loc_80020F00;
loc_80020E90:
    v0 = a0 << 3;
loc_80020E94:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_80020EE0;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_80020F00;
loc_80020EE0:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_80020F00;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_80020E94;
    sw(0, s0);
loc_80020F00:
    s0 = lw(s1 + 0xC4);
    s3 = lw(s1);
    s0 = (s0 < 1);
    P_Random();
    v0 &= 3;
    v0++;
    s1 = lw(s3 + 0x24);
    s2 = v0 << 2;
    if (s0 != 0) goto loc_80020F40;
    P_Random();
    s0 = v0;
    P_Random();
    s0 -= v0;
    s0 <<= 18;
    s1 += s0;
loc_80020F40:
    sw(s2, sp + 0x10);
    a0 = s3;
    a1 = s1;
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    a2 = 0x8000000;                                     // Result = 08000000
    P_LineAttack();
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_FireShotgun() noexcept {
    sp -= 0x38;
    sw(s3, sp + 0x24);
    s3 = a0;
    sw(ra, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    a0 = lw(s3);
    a1 = 8;                                             // Result = 00000008
    S_StartSound();
    v1 = lw(s3 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v0;
    v1 = lw(at);
    v1 <<= 2;
    v1 += s3;
    v0 = lw(v1 + 0x98);
    v0--;
    sw(v0, v1 + 0x98);
    v1 = lw(s3 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7108;                                       // Result = WeaponInfo_Fist[5] (80067108)
    at += v0;
    a0 = lw(at);
    s0 = s3 + 0x100;
    if (a0 != 0) goto loc_80021024;
    sw(0, s3 + 0x100);
    goto loc_80021094;
loc_80021024:
    v0 = a0 << 3;
loc_80021028:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s3;
    if (v0 == 0) goto loc_80021074;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_80021094;
loc_80021074:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_80021094;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_80021028;
    sw(0, s0);
loc_80021094:
    a0 = lw(s3);
    a2 = 0x8000000;                                     // Result = 08000000
    a1 = lw(a0 + 0x24);
    s4 = 0;                                             // Result = 00000000
    P_AimLineAttack();
    s5 = v0;
loc_800210AC:
    s4++;
    P_Random();
    s0 = v0 & 3;
    s0++;
    v1 = lw(s3);
    s2 = lw(v1 + 0x24);
    s0 <<= 2;
    P_Random();
    s1 = v0;
    P_Random();
    a2 = 0x8000000;                                     // Result = 08000000
    s1 -= v0;
    s1 <<= 18;
    a3 = s5;
    sw(s0, sp + 0x10);
    a0 = lw(s3);
    a1 = s2 + s1;
    P_LineAttack();
    v0 = (i32(s4) < 7);
    if (v0 != 0) goto loc_800210AC;
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

void A_FireShotgun2() noexcept {
    sp -= 0x30;
    sw(s3, sp + 0x24);
    s3 = a0;
    sw(ra, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    a0 = lw(s3);
    a1 = 0x1D;                                          // Result = 0000001D
    S_StartSound();
    a0 = lw(s3);
    a1 = 0xA0;                                          // Result = 000000A0
    P_SetMObjState();
    v1 = lw(s3 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v0;
    v1 = lw(at);
    v1 <<= 2;
    v1 += s3;
    v0 = lw(v1 + 0x98);
    v0 -= 2;
    sw(v0, v1 + 0x98);
    v1 = lw(s3 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7108;                                       // Result = WeaponInfo_Fist[5] (80067108)
    at += v0;
    a0 = lw(at);
    s0 = s3 + 0x100;
    if (a0 != 0) goto loc_800211DC;
    sw(0, s3 + 0x100);
    goto loc_8002124C;
loc_800211DC:
    v0 = a0 << 3;
loc_800211E0:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s3;
    if (v0 == 0) goto loc_8002122C;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_8002124C;
loc_8002122C:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_8002124C;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_800211E0;
    sw(0, s0);
loc_8002124C:
    s1 = lw(s3);
    a2 = 0x4000000;                                     // Result = 04000000
    s0 = lw(s1 + 0x24);
    a0 = s1;
    a1 = s0;
    P_AimLineAttack();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    sw(v0, gp + 0xA10);                                 // Store to: gSlope (80077FF0)
    s4 = 0;                                             // Result = 00000000
    if (v1 != 0) goto loc_800212B8;
    v0 = 0x4000000;                                     // Result = 04000000
    s0 += v0;
    a0 = s1;
    a1 = s0;
    a2 = 0x4000000;                                     // Result = 04000000
    P_AimLineAttack();
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    sw(v0, gp + 0xA10);                                 // Store to: gSlope (80077FF0)
    a0 = s1;
    if (v1 != 0) goto loc_800212B8;
    a1 = 0xF8000000;                                    // Result = F8000000
    a1 += s0;
    a2 = 0x4000000;                                     // Result = 04000000
    P_AimLineAttack();
    sw(v0, gp + 0xA10);                                 // Store to: gSlope (80077FF0)
loc_800212B8:
    s4++;
    P_Random();
    v1 = 0x55550000;                                    // Result = 55550000
    v1 |= 0x5556;                                       // Result = 55555556
    mult(v0, v1);
    v1 = lw(s3);
    s2 = lw(v1 + 0x24);
    v1 = u32(i32(v0) >> 31);
    a0 = hi;
    a0 -= v1;
    v1 = a0 << 1;
    v1 += a0;
    v0 -= v1;
    v0++;
    s1 = v0 << 2;
    s1 += v0;
    P_Random();
    s0 = v0;
    P_Random();
    s0 -= v0;
    s0 <<= 19;
    s2 += s0;
    P_Random();
    s0 = v0;
    P_Random();
    a1 = s2;
    a2 = 0x8000000;                                     // Result = 08000000
    s0 -= v0;
    a3 = lw(gp + 0xA10);                                // Load from: gSlope (80077FF0)
    s0 <<= 5;
    sw(s1, sp + 0x10);
    a0 = lw(s3);
    a3 += s0;
    P_LineAttack();
    v0 = (i32(s4) < 0x14);
    if (v0 != 0) goto loc_800212B8;
    ra = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_FireCGun() noexcept {
    sp -= 0x30;
    sw(s1, sp + 0x1C);
    s1 = a0;
    sw(s0, sp + 0x18);
    s0 = a1;
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    a0 = lw(s1);
    a1 = 7;                                             // Result = 00000007
    S_StartSound();
    v0 = lw(s1 + 0x6C);
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v1;
    v0 = lw(at);
    v0 <<= 2;
    v1 = v0 + s1;
    v0 = lw(v1 + 0x98);
    {
        const bool bJump = (v0 == 0);
        v0--;
        if (bJump) goto loc_8002153C;
    }
    sw(v0, v1 + 0x98);
    v1 = lw(s1 + 0x6C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7108;                                       // Result = WeaponInfo_Fist[5] (80067108)
    at += v0;
    v0 = lw(at);
    v1 = v0 << 3;
    v1 -= v0;
    v1 <<= 2;
    v0 = lw(s0);
    v1 += v0;
    v0 = 0x80060000;                                    // Result = 80060000
    v0 -= 0x6CC4;                                       // Result = State_S_CHAIN1[0] (8005933C)
    v1 -= v0;
    v0 = v1 << 3;
    v0 += v1;
    a0 = v0 << 6;
    v0 += a0;
    v0 <<= 3;
    v0 += v1;
    a0 = v0 << 15;
    v0 += a0;
    v0 <<= 3;
    v0 += v1;
    v0 = -v0;
    a0 = u32(i32(v0) >> 2);
    s0 = s1 + 0x100;
    if (a0 != 0) goto loc_80021470;
    sw(0, s1 + 0x100);
    goto loc_800214E0;
loc_80021470:
    v0 = a0 << 3;
loc_80021474:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_800214C0;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_800214E0;
loc_800214C0:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_800214E0;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_80021474;
    sw(0, s0);
loc_800214E0:
    s0 = lw(s1 + 0xC4);
    s3 = lw(s1);
    s0 = (s0 < 1);
    P_Random();
    v0 &= 3;
    v0++;
    s1 = lw(s3 + 0x24);
    s2 = v0 << 2;
    if (s0 != 0) goto loc_80021520;
    P_Random();
    s0 = v0;
    P_Random();
    s0 -= v0;
    s0 <<= 18;
    s1 += s0;
loc_80021520:
    sw(s2, sp + 0x10);
    a0 = s3;
    a1 = s1;
    a3 = 0x7FFF0000;                                    // Result = 7FFF0000
    a3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    a2 = 0x8000000;                                     // Result = 08000000
    P_LineAttack();
loc_8002153C:
    ra = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x30;
    return;
}

void A_Light0() noexcept {
    sw(0, a0 + 0xE4);
    return;
}

void A_Light1() noexcept {
    v0 = 8;                                             // Result = 00000008
    sw(v0, a0 + 0xE4);
    return;
}

void A_Light2() noexcept {
    v0 = 0x10;                                          // Result = 00000010
    sw(v0, a0 + 0xE4);
    return;
}

void A_BFGSpray() noexcept {
    sp -= 0x28;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s4, sp + 0x20);
    s4 = 0;                                             // Result = 00000000
    sw(s2, sp + 0x18);
    s2 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x24);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    a2 = 0x4000000;                                     // Result = 04000000
loc_800215A8:
    a1 = 0xE0000000;                                    // Result = E0000000
    a1 += s2;
    v0 = lw(s3 + 0x24);
    a0 = lw(s3 + 0x74);
    a1 += v0;
    P_AimLineAttack();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    a3 = 0x20;                                          // Result = 00000020
    if (v0 == 0) goto loc_8002162C;
    s1 = 0;                                             // Result = 00000000
    s0 = 0;                                             // Result = 00000000
    a0 = lw(v0);
    a1 = lw(v0 + 0x4);
    a2 = lw(v0 + 0x44);
    v0 = lw(v0 + 0x8);
    a2 = u32(i32(a2) >> 2);
    a2 += v0;
    P_SpawnMObj();
loc_800215F8:
    s0++;
    P_Random();
    v1 = s1 + 1;
    v0 &= 7;
    s1 = v1 + v0;
    v0 = (i32(s0) < 0xF);
    a3 = s1;
    if (v0 != 0) goto loc_800215F8;
    a1 = lw(s3 + 0x74);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7EE8);                               // Load from: gpLineTarget (80077EE8)
    a2 = a1;
    P_DamageMObj();
loc_8002162C:
    v0 = 0x1990000;                                     // Result = 01990000
    v0 |= 0x9999;                                       // Result = 01999999
    s2 += v0;
    s4++;
    v0 = (i32(s4) < 0x28);
    a2 = 0x4000000;                                     // Result = 04000000
    if (v0 != 0) goto loc_800215A8;
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void A_BFGsound() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = lw(a0);
    a1 = 0xA;                                           // Result = 0000000A
    S_StartSound();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_OpenShotgun2() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = lw(a0);
    a1 = 0x1E;                                          // Result = 0000001E
    S_StartSound();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_LoadShotgun2() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a0 = lw(a0);
    a1 = 0x1F;                                          // Result = 0000001F
    S_StartSound();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void A_CloseShotgun2() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    a0 = lw(s0);
    a1 = 0x20;                                          // Result = 00000020
    S_StartSound();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D14);                               // Load from: gPlayerNum (800782EC)
    v0 <<= 2;
    at = ptrToVmAddr(&gpPlayerBtnBindings[0]);
    at += v0;
    v1 = lw(at);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7F44;                                       // Result = gPlayerPadButtons[0] (80077F44)
    at += v0;
    v0 = lw(at);
    v1 = lw(v1);
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0xA;                                       // Result = 0000000A
        if (bJump) goto loc_80021774;
    }
    v1 = lw(s0 + 0x70);
    if (v1 != v0) goto loc_80021774;
    v0 = lw(s0 + 0x24);
    a0 = s0;
    if (v0 == 0) goto loc_80021774;
    v0 = lw(s0 + 0xC4);
    v0++;
    sw(v0, a0 + 0xC4);
    P_FireWeapon();
    goto loc_80021780;
loc_80021774:
    sw(0, s0 + 0xC4);
    a0 = s0;
    P_CheckAmmo();
loc_80021780:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_SetupPsprites() noexcept {
loc_80021794:
    sp -= 0x20;
    v1 = a0 << 2;
    a1 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7F70;                                       // Result = gTicRemainder[0] (80078090)
    at += v1;
    sw(0, at);
    v1 += a0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    s1 = v0 + v1;
    v0 = s1 + 0x10;
loc_800217DC:
    sw(0, v0 + 0xF0);
    a1--;
    v0 -= 0x10;
    if (i32(a1) >= 0) goto loc_800217DC;
    v1 = lw(s1 + 0x6C);
    v0 = 0xA;                                           // Result = 0000000A
    sw(v1, s1 + 0x70);
    if (v1 != v0) goto loc_80021808;
    v0 = lw(s1 + 0x6C);
    sw(v0, s1 + 0x70);
loc_80021808:
    v1 = lw(s1 + 0x70);
    v0 = 8;                                             // Result = 00000008
    s0 = s1 + 0xF0;
    if (v1 != v0) goto loc_8002183C;
    v0 = *gbIsLevelDataCached;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 1;
        if (bJump) goto loc_80021844;
    }
    a0 = lw(s1);
    a1 = 0xB;                                           // Result = 0000000B
    S_StartSound();
    v1 = lw(s1 + 0x70);
loc_8002183C:
    v0 = v1 << 1;
loc_80021844:
    v0 += v1;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F8;                                       // Result = WeaponInfo_Fist[1] (800670F8)
    at += v0;
    v1 = lw(at);
    v0 = 0xA;                                           // Result = 0000000A
    sw(v0, s1 + 0x70);
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s1 + 0xF8);
    v0 = 0x600000;                                      // Result = 00600000
    a0 = v1;
    sw(v0, s1 + 0xFC);
    if (a0 != 0) goto loc_80021884;
    sw(0, s1 + 0xF0);
    goto loc_800218F4;
loc_80021884:
    v0 = a0 << 3;
loc_80021888:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_800218D4;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_800218F4;
loc_800218D4:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_800218F4;
    v0 = a0 << 3;
    if (a0 != 0) goto loc_80021888;
    sw(0, s0);
loc_800218F4:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_MovePsprites() noexcept {
loc_8002190C:
    sp -= 0x38;
    sw(s1, sp + 0x1C);
    s1 = a0;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D14);                               // Load from: gPlayerNum (800782EC)
    a1 = 0x80080000;                                    // Result = 80080000
    a1 -= 0x7F70;                                       // Result = gTicRemainder[0] (80078090)
    sw(ra, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s0, sp + 0x18);
    v1 <<= 2;
    a0 = v1 + a1;
    v0 = lw(a0);
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v1;
    v1 = lw(at);
    v0 += v1;
    sw(v0, a0);
    v0 = (i32(v0) < 4);
    if (v0 != 0) goto loc_80021A94;
loc_80021974:
    s3 = s1 + 0xF0;
    s5 = 0;                                             // Result = 00000000
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7D14);                               // Load from: gPlayerNum (800782EC)
    s2 = s1 + 0xF4;
    v1 <<= 2;
    v1 += a1;
    v0 = lw(v1);
    s4 = 0xF0;                                          // Result = 000000F0
    v0 -= 4;
    sw(v0, v1);
loc_800219A0:
    v0 = lw(s3);
    {
        const bool bJump = (v0 == 0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_80021A48;
    }
    v1 = lw(s2);
    {
        const bool bJump = (v1 == v0);
        v0 = v1 - 1;
        if (bJump) goto loc_80021A48;
    }
    sw(v0, s2);
    if (v0 != 0) goto loc_80021A48;
    v0 = lw(s3);
    a0 = lw(v0 + 0x10);
    s0 = s1 + s4;
    goto loc_80021A3C;
loc_800219DC:
    v0 -= a0;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x4);
    v0 = lw(v0 + 0xC);
    a0 = s1;
    if (v0 == 0) goto loc_80021A28;
    a1 = s0;
    ptr_call(v0);
    v0 = lw(s0);
    if (v0 == 0) goto loc_80021A48;
loc_80021A28:
    v0 = lw(s0);
    v1 = lw(s0 + 0x4);
    a0 = lw(v0 + 0x10);
    if (v1 != 0) goto loc_80021A48;
loc_80021A3C:
    v0 = a0 << 3;
    if (a0 != 0) goto loc_800219DC;
    sw(0, s0);
loc_80021A48:
    s4 += 0x10;
    s5++;
    s2 += 0x10;
    v0 = (i32(s5) < 2);
    s3 += 0x10;
    if (v0 != 0) goto loc_800219A0;
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D14);                               // Load from: gPlayerNum (800782EC)
    v0 <<= 2;
    at = 0x80080000;                                    // Result = 80080000
    at -= 0x7F70;                                       // Result = gTicRemainder[0] (80078090)
    at += v0;
    v0 = lw(at);
    a1 = 0x80080000;                                    // Result = 80080000
    a1 -= 0x7F70;                                       // Result = gTicRemainder[0] (80078090)
    v0 = (i32(v0) < 4);
    if (v0 == 0) goto loc_80021974;
loc_80021A94:
    v0 = lw(s1 + 0xF8);
    v1 = lw(s1 + 0xFC);
    sw(v0, s1 + 0x108);
    sw(v1, s1 + 0x10C);
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
