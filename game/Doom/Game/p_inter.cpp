#include "p_inter.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_main.h"
#include "g_game.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "PsxVm/PsxVm.h"

// The maximum amount of ammo for each ammo type and how much ammo each clip type gives
const VmPtr<int32_t[NUMAMMO]>   gMaxAmmo(0x800670D4);
const VmPtr<int32_t[NUMAMMO]>   gClipAmmo(0x800670E4);

void P_GiveAmmo() noexcept {
loc_800197A4:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s1, sp + 0x14);
    s1 = a2;
    v0 = 5;                                             // Result = 00000005
    sw(ra, sp + 0x1C);
    if (s2 == v0) goto loc_80019804;
    v0 = (s2 < 5);
    a0 = s2 << 2;
    if (v0 != 0) goto loc_800197EC;
    I_Error("P_GiveAmmo: bad type %i", (int32_t) s2);
    a0 = s2 << 2;
loc_800197EC:
    v0 = a0 + s0;
    v1 = lw(v0 + 0x98);
    v0 = lw(v0 + 0xA8);
    if (v1 != v0) goto loc_8001980C;
loc_80019804:
    v0 = 0;                                             // Result = 00000000
    goto loc_80019970;
loc_8001980C:
    if (s1 == 0) goto loc_80019838;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70E4;                                       // Result = gClipAmmo[0] (800670E4)
    at += a0;
    v0 = lw(at);
    mult(s1, v0);
    s1 = lo;
    goto loc_80019858;
loc_80019838:
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70E4;                                       // Result = gClipAmmo[0] (800670E4)
    at += a0;
    v0 = lw(at);
    v1 = v0 >> 31;
    v0 += v1;
    s1 = u32(i32(v0) >> 1);
loc_80019858:
    v0 = *gGameSkill;
    {
        const bool bJump = (v0 != 0);
        v0 = s2 << 2;
        if (bJump) goto loc_80019870;
    }
    s1 <<= 1;
loc_80019870:
    v1 = v0 + s0;
    a1 = lw(v1 + 0x98);
    a0 = lw(v1 + 0xA8);
    v0 = s1 + a1;
    sw(v0, v1 + 0x98);
    v0 = (i32(a0) < i32(v0));
    if (v0 == 0) goto loc_80019894;
    sw(a0, v1 + 0x98);
loc_80019894:
    v0 = 1;                                             // Result = 00000001
    if (a1 != 0) goto loc_80019970;
    v1 = 1;                                             // Result = 00000001
    if (s2 == v1) goto loc_800198F0;
    v0 = 2;                                             // Result = 00000002
    if (s2 == 0) goto loc_800198C8;
    {
        const bool bJump = (s2 == v0);
        v0 = 3;                                         // Result = 00000003
        if (bJump) goto loc_8001991C;
    }
    {
        const bool bJump = (s2 == v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80019948;
    }
    goto loc_80019970;
loc_800198C8:
    v0 = lw(s0 + 0x6C);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80019970;
    }
    v0 = lw(s0 + 0x84);
    {
        const bool bJump = (v0 != 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_80019968;
    }
    sw(v1, s0 + 0x70);
    goto loc_8001996C;
loc_800198F0:
    v0 = lw(s0 + 0x6C);
    v0 = (v0 < 2);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80019970;
    }
    v0 = lw(s0 + 0x7C);
    {
        const bool bJump = (v0 == 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8001996C;
    }
    sw(v0, s0 + 0x70);
    goto loc_8001996C;
loc_8001991C:
    v0 = lw(s0 + 0x6C);
    v0 = (v0 < 2);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80019970;
    }
    v0 = lw(s0 + 0x8C);
    {
        const bool bJump = (v0 == 0);
        v0 = 6;                                         // Result = 00000006
        if (bJump) goto loc_8001996C;
    }
    sw(v0, s0 + 0x70);
    goto loc_8001996C;
loc_80019948:
    v0 = lw(s0 + 0x6C);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80019970;
    }
    v0 = lw(s0 + 0x88);
    {
        const bool bJump = (v0 == 0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_8001996C;
    }
loc_80019968:
    sw(v0, s0 + 0x70);
loc_8001996C:
    v0 = 1;                                             // Result = 00000001
loc_80019970:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_GiveWeapon() noexcept {
loc_8001998C:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s0, sp + 0x10);
    s0 = a1;
    a1 = *gNetGame;
    v0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x18);
    if (a1 != v0) goto loc_80019A1C;
    v0 = s0 << 1;
    if (a2 != 0) goto loc_80019A20;
    v0 = s0 << 2;
    v1 = v0 + s1;
    v0 = lw(v1 + 0x74);
    a0 = s1;
    if (v0 == 0) goto loc_800199DC;
    v0 = 0;                                             // Result = 00000000
    goto loc_80019ADC;
loc_800199DC:
    v0 = s0 << 1;
    v0 += s0;
    v0 <<= 3;
    sw(a1, v1 + 0x74);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v0;
    a1 = lw(at);
    a2 = 2;                                             // Result = 00000002
    P_GiveAmmo();
    a0 = lw(s1);
    a1 = sfx_wpnup;
    sw(s0, s1 + 0x70);
    S_StartSound();
    v0 = 0;
    goto loc_80019ADC;
loc_80019A1C:
    v0 = s0 << 1;
loc_80019A20:
    v0 += s0;
    v0 <<= 3;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x70F4;                                       // Result = WeaponInfo_Fist[0] (800670F4)
    at += v0;
    a1 = lw(at);
    v0 = 5;                                             // Result = 00000005
    if (a1 == v0) goto loc_80019A68;
    a0 = s1;
    if (a2 == 0) goto loc_80019A54;
    a2 = 1;                                             // Result = 00000001
    goto loc_80019A58;
loc_80019A54:
    a2 = 2;                                             // Result = 00000002
loc_80019A58:
    P_GiveAmmo();
    a1 = v0;
    goto loc_80019A6C;
loc_80019A68:
    a1 = 0;                                             // Result = 00000000
loc_80019A6C:
    v0 = s0 << 2;
    v1 = v0 + s1;
    v0 = lw(v1 + 0x74);
    a0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80019AC8;
    v0 = *gCurPlayerIndex;
    a0 = 1;                                             // Result = 00000001
    sw(a0, v1 + 0x74);
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    sw(s0, s1 + 0x70);
    if (s1 != v0) goto loc_80019AC8;
    v0 = 6;                                             // Result = 00000006
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78E8);                                // Store to: gStatusBar[0] (80098718)
loc_80019AC8:
    v0 = 0;                                             // Result = 00000000
    if (a0 != 0) goto loc_80019AD8;
    if (a1 == 0) goto loc_80019ADC;
loc_80019AD8:
    v0 = 1;                                             // Result = 00000001
loc_80019ADC:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_GiveBody() noexcept {
    v1 = a0;
    a0 = lw(v1 + 0x24);
    v0 = (i32(a0) < 0x64);
    {
        const bool bJump = (v0 != 0);
        v0 = a1 + a0;
        if (bJump) goto loc_80019B14;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_80019B38;
loc_80019B14:
    sw(v0, v1 + 0x24);
    v0 = (i32(v0) < 0x65);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x64;                                      // Result = 00000064
        if (bJump) goto loc_80019B28;
    }
    sw(v0, v1 + 0x24);
loc_80019B28:
    a0 = lw(v1);
    v1 = lw(v1 + 0x24);
    v0 = 1;                                             // Result = 00000001
    sw(v1, a0 + 0x68);
loc_80019B38:
    return;
}

void P_GiveArmor() noexcept {
    v0 = a1 << 1;
    v0 += a1;
    v0 <<= 3;
    v0 += a1;
    v1 = lw(a0 + 0x28);
    a2 = v0 << 2;
    v1 = (i32(v1) < i32(a2));
    v0 = 1;                                             // Result = 00000001
    if (v1 == 0) goto loc_80019B70;
    sw(a1, a0 + 0x2C);
    sw(a2, a0 + 0x28);
    goto loc_80019B74;
loc_80019B70:
    v0 = 0;                                             // Result = 00000000
loc_80019B74:
    return;
}

void P_GiveCard() noexcept {
    a1 <<= 2;
    a1 += a0;
    v0 = lw(a1 + 0x48);
    {
        const bool bJump = (v0 != 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_80019BA0;
    }
    sw(v0, a0 + 0xDC);
    v0 = 1;                                             // Result = 00000001
    sw(v0, a1 + 0x48);
loc_80019BA0:
    return;
}

void P_GivePower() noexcept {
    v0 = (a1 < 6);
    if (v0 == 0) goto loc_80019C84;
    v0 = a1 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += 0x7B8;                                        // Result = JumpTable_P_GivePower[0] (800107B8)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80019BD4: goto loc_80019BD4;
        case 0x80019C28: goto loc_80019C28;
        case 0x80019BE4: goto loc_80019BE4;
        case 0x80019C18: goto loc_80019C18;
        case 0x80019C6C: goto loc_80019C6C;
        case 0x80019C08: goto loc_80019C08;
        default: jump_table_err(); break;
    }
loc_80019BD4:
    v0 = 0x1C2;                                         // Result = 000001C2
    sw(v0, a0 + 0x30);
    v0 = 1;                                             // Result = 00000001
    goto loc_80019C84;
loc_80019BE4:
    v0 = 1;                                             // Result = 00000001
    a1 = lw(a0);
    v1 = 0x384;                                         // Result = 00000384
    sw(v1, a0 + 0x38);
    v1 = lw(a1 + 0x64);
    a0 = 0x70000000;                                    // Result = 70000000
    v1 |= a0;
    sw(v1, a1 + 0x64);
    goto loc_80019C84;
loc_80019C08:
    v0 = 0x708;                                         // Result = 00000708
    sw(v0, a0 + 0x44);
    v0 = 1;                                             // Result = 00000001
    goto loc_80019C84;
loc_80019C18:
    v0 = 0x384;                                         // Result = 00000384
    sw(v0, a0 + 0x3C);
    v0 = 1;                                             // Result = 00000001
    goto loc_80019C84;
loc_80019C28:
    v1 = lw(a0 + 0x24);
    v0 = (i32(v1) < 0x64);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 + 0x64;
        if (bJump) goto loc_80019C60;
    }
    sw(v0, a0 + 0x24);
    v0 = (i32(v0) < 0x65);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x64;                                      // Result = 00000064
        if (bJump) goto loc_80019C50;
    }
    sw(v0, a0 + 0x24);
loc_80019C50:
    v1 = lw(a0);
    v0 = lw(a0 + 0x24);
    sw(v0, v1 + 0x68);
loc_80019C60:
    v0 = 1;                                             // Result = 00000001
    sw(v0, a0 + 0x34);
    goto loc_80019C84;
loc_80019C6C:
    v0 = lw(a0 + 0x40);
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80019C84;
    }
    v0 = 1;                                             // Result = 00000001
    sw(v0, a0 + 0x40);
loc_80019C84:
    return;
}

void P_TouchSpecialThing() noexcept {
loc_80019C8C:
    sp -= 0x28;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(ra, sp + 0x20);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    a0 = lw(s3 + 0x8);
    v1 = lw(a1 + 0x8);
    v0 = lw(a1 + 0x44);
    a0 -= v1;
    v0 = (i32(v0) < i32(a0));
    {
        const bool bJump = (v0 != 0);
        v0 = 0xFFF80000;                                // Result = FFF80000
        if (bJump) goto loc_8001A55C;
    }
    v0 = (i32(a0) < i32(v0));
    if (v0 != 0) goto loc_8001A55C;
    v0 = lw(a1 + 0x68);
    s1 = lw(a1 + 0x80);
    s2 = 0x18;                                          // Result = 00000018
    if (i32(v0) <= 0) goto loc_8001A55C;
    v0 = lw(s3 + 0x28);
    v1 = v0 - 0x31;
    v0 = (v1 < 0x27);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_8001A4EC;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0x7D0;                                        // Result = JumpTable_P_TouchSpecialThing[0] (800107D0)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x8001A118: goto loc_8001A118;
        case 0x8001A15C: goto loc_8001A15C;
        case 0x8001A4EC: goto loc_8001A4EC;
        case 0x80019D14: goto loc_80019D14;
        case 0x80019D54: goto loc_80019D54;
        case 0x8001A1A0: goto loc_8001A1A0;
        case 0x8001A210: goto loc_8001A210;
        case 0x8001A1D8: goto loc_8001A1D8;
        case 0x8001A248: goto loc_8001A248;
        case 0x8001A2B8: goto loc_8001A2B8;
        case 0x8001A280: goto loc_8001A280;
        case 0x8001A308: goto loc_8001A308;
        case 0x8001A364: goto loc_8001A364;
        case 0x80019D98: goto loc_80019D98;
        case 0x8001A3E4: goto loc_8001A3E4;
        case 0x8001A3FC: goto loc_8001A3FC;
        case 0x8001A45C: goto loc_8001A45C;
        case 0x80019DDC: goto loc_80019DDC;
        case 0x8001A488: goto loc_8001A488;
        case 0x8001A4A0: goto loc_8001A4A0;
        case 0x8001A4D4: goto loc_8001A4D4;
        case 0x80019E18: goto loc_80019E18;
        case 0x80019E60: goto loc_80019E60;
        case 0x80019E88: goto loc_80019E88;
        case 0x80019EB0: goto loc_80019EB0;
        case 0x80019ED8: goto loc_80019ED8;
        case 0x80019F00: goto loc_80019F00;
        case 0x80019F28: goto loc_80019F28;
        case 0x80019F50: goto loc_80019F50;
        case 0x80019F78: goto loc_80019F78;
        case 0x80019FE4: goto loc_80019FE4;
        case 0x8001A00C: goto loc_8001A00C;
        case 0x8001A03C: goto loc_8001A03C;
        case 0x8001A064: goto loc_8001A064;
        case 0x8001A08C: goto loc_8001A08C;
        case 0x8001A0B4: goto loc_8001A0B4;
        case 0x8001A0E4: goto loc_8001A0E4;
        default: jump_table_err(); break;
    }
loc_80019D14:
    v0 = lw(s1 + 0x24);
    v0 += 2;
    sw(v0, s1 + 0x24);
    v0 = (i32(v0) < 0xC9);
    {
        const bool bJump = (v0 != 0);
        v0 = 0xC8;                                      // Result = 000000C8
        if (bJump) goto loc_80019D34;
    }
    sw(v0, s1 + 0x24);
loc_80019D34:
    v0 = lw(s1);
    v1 = lw(s1 + 0x24);
    sw(v1, v0 + 0x68);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x3E8;                                        // Result = STR_HealthBonusPickedUpMsg[0] (800103E8)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019D54:
    v0 = lw(s1 + 0x28);
    v0 += 2;
    sw(v0, s1 + 0x28);
    v0 = (i32(v0) < 0xC9);
    {
        const bool bJump = (v0 != 0);
        v0 = 0xC8;                                      // Result = 000000C8
        if (bJump) goto loc_80019D74;
    }
    sw(v0, s1 + 0x28);
loc_80019D74:
    v0 = lw(s1 + 0x2C);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80019D88;
    }
    sw(v0, s1 + 0x2C);
loc_80019D88:
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x404;                                        // Result = STR_ArmorBonusPickedUpMsg[0] (80010404)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019D98:
    v0 = lw(s1 + 0x24);
    v0 += 0x64;
    sw(v0, s1 + 0x24);
    v0 = (i32(v0) < 0xC9);
    s2 = 0x59;                                          // Result = 00000059
    if (v0 != 0) goto loc_80019DBC;
    v0 = 0xC8;                                          // Result = 000000C8
    sw(v0, s1 + 0x24);
loc_80019DBC:
    v1 = lw(s1);
    v0 = lw(s1 + 0x24);
    sw(v0, v1 + 0x68);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x420;                                        // Result = STR_SuperChargePickedUpMsg[0] (80010420)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019DDC:
    v0 = lw(s1);
    v1 = 0xC8;                                          // Result = 000000C8
    sw(v1, s1 + 0x24);
    sw(v1, v0 + 0x68);
    v0 = lw(s1 + 0x28);
    v0 = (i32(v0) < 0xC8);
    a0 = 2;                                             // Result = 00000002
    if (v0 == 0) goto loc_80019E08;
    sw(a0, s1 + 0x2C);
    sw(v1, s1 + 0x28);
loc_80019E08:
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x430;                                        // Result = STR_MegaSpherePickedUpMsg[0] (80010430)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4CC;
loc_80019E18:
    v0 = lw(s3 + 0x64);
    v1 = 0x20000;                                       // Result = 00020000
    v0 &= v1;
    a0 = s1;
    if (v0 == 0) goto loc_80019E38;
    a1 = 0;                                             // Result = 00000000
    a2 = 0;                                             // Result = 00000000
    goto loc_80019E40;
loc_80019E38:
    a1 = 0;                                             // Result = 00000000
    a2 = 1;                                             // Result = 00000001
loc_80019E40:
    P_GiveAmmo();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x440;                                        // Result = STR_ClipPickedUpMsg[0] (80010440)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019E60:
    a0 = s1;
    a1 = 0;                                             // Result = 00000000
    a2 = 5;                                             // Result = 00000005
    P_GiveAmmo();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x454;                                        // Result = STR_BoxOfBulletsPickedUpMsg[0] (80010454)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019E88:
    a0 = s1;
    a1 = 3;                                             // Result = 00000003
    a2 = 1;                                             // Result = 00000001
    P_GiveAmmo();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x470;                                        // Result = STR_RocketPickedUpMsg[0] (80010470)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019EB0:
    a0 = s1;
    a1 = 3;                                             // Result = 00000003
    a2 = 5;                                             // Result = 00000005
    P_GiveAmmo();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x484;                                        // Result = STR_BoxOfRocketsPickedUpMsg[0] (80010484)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019ED8:
    a0 = s1;
    a1 = 2;                                             // Result = 00000002
    a2 = 1;                                             // Result = 00000001
    P_GiveAmmo();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x4A0;                                        // Result = STR_EnergyCellPickedUpMsg[0] (800104A0)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019F00:
    a0 = s1;
    a1 = 2;                                             // Result = 00000002
    a2 = 5;                                             // Result = 00000005
    P_GiveAmmo();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x4BC;                                        // Result = STR_EnergyCellPackPickedUpMsg[0] (800104BC)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019F28:
    a0 = s1;
    a1 = 1;                                             // Result = 00000001
    a2 = 1;                                             // Result = 00000001
    P_GiveAmmo();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x4DC;                                        // Result = STR_FourShotgunShellsPickedUpMsg[0] (800104DC)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019F50:
    a0 = s1;
    a1 = 1;                                             // Result = 00000001
    a2 = 5;                                             // Result = 00000005
    P_GiveAmmo();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x4F8;                                        // Result = STR_BoxOfShotgunShellsPickedUpMsg[0] (800104F8)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019F78:
    v0 = lw(s1 + 0x60);
    s0 = 0;                                             // Result = 00000000
    if (v0 != 0) goto loc_80019FB4;
    v1 = s1;
loc_80019F8C:
    v0 = lw(v1 + 0xA8);
    s0++;
    v0 <<= 1;
    sw(v0, v1 + 0xA8);
    v0 = (i32(s0) < 4);
    v1 += 4;
    if (v0 != 0) goto loc_80019F8C;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s1 + 0x60);
    s0 = 0;                                             // Result = 00000000
loc_80019FB4:
    a0 = s1;
loc_80019FB8:
    a1 = s0;
    a2 = 1;                                             // Result = 00000001
    P_GiveAmmo();
    s0++;
    v0 = (i32(s0) < 4);
    a0 = s1;
    if (v0 != 0) goto loc_80019FB8;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x514;                                        // Result = STR_BackpackPickedUpMsg[0] (80010514)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_80019FE4:
    a0 = s1;
    a1 = 7;                                             // Result = 00000007
    a2 = 0;                                             // Result = 00000000
    P_GiveWeapon();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x52C;                                        // Result = STR_BfgPickedUpMsg[0] (8001052C)
    sw(v0, s1 + 0xD4);
    goto loc_8001A110;
loc_8001A00C:
    a0 = s1;
    a1 = 4;                                             // Result = 00000004
    v0 = lw(s3 + 0x64);
    a2 = 0x20000;                                       // Result = 00020000
    a2 &= v0;
    P_GiveWeapon();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x54C;                                        // Result = STR_ChaingunPickedUpMsg[0] (8001054C)
    sw(v0, s1 + 0xD4);
    goto loc_8001A110;
loc_8001A03C:
    a0 = s1;
    a1 = 8;                                             // Result = 00000008
    a2 = 0;                                             // Result = 00000000
    P_GiveWeapon();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x564;                                        // Result = STR_ChainsawPickedUpMsg[0] (80010564)
    sw(v0, s1 + 0xD4);
    goto loc_8001A110;
loc_8001A064:
    a0 = s1;
    a1 = 5;                                             // Result = 00000005
    a2 = 0;                                             // Result = 00000000
    P_GiveWeapon();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x584;                                        // Result = STR_RocketLauncherPickedUpMsg[0] (80010584)
    sw(v0, s1 + 0xD4);
    goto loc_8001A110;
loc_8001A08C:
    a0 = s1;
    a1 = 6;                                             // Result = 00000006
    a2 = 0;                                             // Result = 00000000
    P_GiveWeapon();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x5A4;                                        // Result = STR_PlasmaGunPickedUpMsg[0] (800105A4)
    sw(v0, s1 + 0xD4);
    goto loc_8001A110;
loc_8001A0B4:
    a0 = s1;
    a1 = 2;                                             // Result = 00000002
    v0 = lw(s3 + 0x64);
    a2 = 0x20000;                                       // Result = 00020000
    a2 &= v0;
    P_GiveWeapon();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x5BC;                                        // Result = STR_ShotgunPickedUpMsg[0] (800105BC)
    sw(v0, s1 + 0xD4);
    goto loc_8001A110;
loc_8001A0E4:
    a0 = s1;
    a1 = 3;                                             // Result = 00000003
    v0 = lw(s3 + 0x64);
    a2 = 0x20000;                                       // Result = 00020000
    a2 &= v0;
    P_GiveWeapon();
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x5D4;                                        // Result = STR_SuperShotgunPickedUpMsg[0] (800105D4)
    sw(v0, s1 + 0xD4);
loc_8001A110:
    s2 = 0x19;                                          // Result = 00000019
    goto loc_8001A4EC;
loc_8001A118:
    a0 = 1;                                             // Result = 00000001
    v0 = lw(s1 + 0x28);
    v0 = (i32(v0) < 0x64);
    v1 = 0x64;                                          // Result = 00000064
    if (v0 != 0) goto loc_8001A138;
    v0 = 0;                                             // Result = 00000000
    goto loc_8001A144;
loc_8001A138:
    v0 = 1;                                             // Result = 00000001
    sw(a0, s1 + 0x2C);
    sw(v1, s1 + 0x28);
loc_8001A144:
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x5F0;                                        // Result = STR_ArmorPickedUpMsg[0] (800105F0)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_8001A15C:
    a0 = 2;                                             // Result = 00000002
    v0 = lw(s1 + 0x28);
    v0 = (i32(v0) < 0xC8);
    v1 = 0xC8;                                          // Result = 000000C8
    if (v0 != 0) goto loc_8001A17C;
    v0 = 0;                                             // Result = 00000000
    goto loc_8001A188;
loc_8001A17C:
    v0 = 1;                                             // Result = 00000001
    sw(a0, s1 + 0x2C);
    sw(v1, s1 + 0x28);
loc_8001A188:
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x608;                                        // Result = STR_MegaArmorPickedUpMsg[0] (80010608)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_8001A1A0:
    v0 = lw(s1 + 0x4C);
    if (v0 != 0) goto loc_8001A2EC;
    v1 = lw(s1 + 0x4C);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x620;                                        // Result = STR_BlueKeycardPickedUpMsg[0] (80010620)
    sw(v0, s1 + 0xD4);
    if (v1 != 0) goto loc_8001A2EC;
    v0 = 4;                                             // Result = 00000004
    sw(v0, s1 + 0xDC);
    v0 = 1;                                             // Result = 00000001
    sw(v0, s1 + 0x4C);
    goto loc_8001A2EC;
loc_8001A1D8:
    v0 = lw(s1 + 0x50);
    if (v0 != 0) goto loc_8001A2EC;
    v1 = lw(s1 + 0x50);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x63C;                                        // Result = STR_YellowKeycardPickedUpMsg[0] (8001063C)
    sw(v0, s1 + 0xD4);
    if (v1 != 0) goto loc_8001A2EC;
    v0 = 4;                                             // Result = 00000004
    sw(v0, s1 + 0xDC);
    v0 = 1;                                             // Result = 00000001
    sw(v0, s1 + 0x50);
    goto loc_8001A2EC;
loc_8001A210:
    v0 = lw(s1 + 0x48);
    if (v0 != 0) goto loc_8001A2EC;
    v1 = lw(s1 + 0x48);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x65C;                                        // Result = STR_RedKeycardPickedUpMsg[0] (8001065C)
    sw(v0, s1 + 0xD4);
    if (v1 != 0) goto loc_8001A2EC;
    v0 = 4;                                             // Result = 00000004
    sw(v0, s1 + 0xDC);
    v0 = 1;                                             // Result = 00000001
    sw(v0, s1 + 0x48);
    goto loc_8001A2EC;
loc_8001A248:
    v0 = lw(s1 + 0x58);
    if (v0 != 0) goto loc_8001A2EC;
    v1 = lw(s1 + 0x58);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x678;                                        // Result = STR_BlueSkullKeyPickedUpMsg[0] (80010678)
    sw(v0, s1 + 0xD4);
    if (v1 != 0) goto loc_8001A2EC;
    v0 = 4;                                             // Result = 00000004
    sw(v0, s1 + 0xDC);
    v0 = 1;                                             // Result = 00000001
    sw(v0, s1 + 0x58);
    goto loc_8001A2EC;
loc_8001A280:
    v0 = lw(s1 + 0x5C);
    if (v0 != 0) goto loc_8001A2EC;
    v1 = lw(s1 + 0x5C);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x698;                                        // Result = STR_YellowSkullKeyPickedUpMsg[0] (80010698)
    sw(v0, s1 + 0xD4);
    if (v1 != 0) goto loc_8001A2EC;
    v0 = 4;                                             // Result = 00000004
    sw(v0, s1 + 0xDC);
    v0 = 1;                                             // Result = 00000001
    sw(v0, s1 + 0x5C);
    goto loc_8001A2EC;
loc_8001A2B8:
    v0 = lw(s1 + 0x54);
    if (v0 != 0) goto loc_8001A2EC;
    v1 = lw(s1 + 0x54);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x6B8;                                        // Result = STR_RedSkullKeyPickedUpMsg[0] (800106B8)
    sw(v0, s1 + 0xD4);
    if (v1 != 0) goto loc_8001A2EC;
    v0 = 4;                                             // Result = 00000004
    sw(v0, s1 + 0xDC);
    v0 = 1;                                             // Result = 00000001
    sw(v0, s1 + 0x54);
loc_8001A2EC:
    v0 = *gNetGame;
    if (v0 == gt_single) goto loc_8001A4EC;
    goto loc_8001A55C;
loc_8001A308:
    v1 = lw(s1 + 0x24);
    v0 = (i32(v1) < 0x64);
    {
        const bool bJump = (v0 != 0);
        v0 = v1 + 0xA;
        if (bJump) goto loc_8001A324;
    }
    a0 = 0;                                             // Result = 00000000
    goto loc_8001A34C;
loc_8001A324:
    sw(v0, s1 + 0x24);
    v0 = (i32(v0) < 0x65);
    a0 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_8001A33C;
    v0 = 0x64;                                          // Result = 00000064
    sw(v0, s1 + 0x24);
loc_8001A33C:
    v1 = lw(s1);
    v0 = lw(s1 + 0x24);
    sw(v0, v1 + 0x68);
loc_8001A34C:
    if (a0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x6D8;                                        // Result = STR_StimpackPickedUpMsg[0] (800106D8)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_8001A364:
    v1 = lw(s1 + 0x24);
    v0 = (i32(v1) < 0x64);
    {
        const bool bJump = (v0 != 0);
        v0 = v1 + 0x19;
        if (bJump) goto loc_8001A380;
    }
    a0 = 0;                                             // Result = 00000000
    goto loc_8001A3A8;
loc_8001A380:
    sw(v0, s1 + 0x24);
    v0 = (i32(v0) < 0x65);
    a0 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_8001A398;
    v0 = 0x64;                                          // Result = 00000064
    sw(v0, s1 + 0x24);
loc_8001A398:
    v1 = lw(s1);
    v0 = lw(s1 + 0x24);
    sw(v0, v1 + 0x68);
loc_8001A3A8:
    if (a0 == 0) goto loc_8001A55C;
    v0 = lw(s1 + 0x24);
    v0 = (i32(v0) < 0x19);
    if (v0 == 0) goto loc_8001A3D4;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x6F0;                                        // Result = STR_NeededMedKitPickedUpMsg[0] (800106F0)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_8001A3D4:
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x71C;                                        // Result = STR_MedKitPickedUpMsg[0] (8001071C)
    sw(v0, s1 + 0xD4);
    goto loc_8001A4EC;
loc_8001A3E4:
    v0 = 0x1C2;                                         // Result = 000001C2
    sw(v0, s1 + 0x30);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x734;                                        // Result = STR_InvunerabilityPickedUpMsg[0] (80010734)
    s2 = 0x59;                                          // Result = 00000059
    goto loc_8001A4E8;
loc_8001A3FC:
    v1 = lw(s1 + 0x24);
    v0 = (i32(v1) < 0x64);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 + 0x64;
        if (bJump) goto loc_8001A434;
    }
    sw(v0, s1 + 0x24);
    v0 = (i32(v0) < 0x65);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x64;                                      // Result = 00000064
        if (bJump) goto loc_8001A424;
    }
    sw(v0, s1 + 0x24);
loc_8001A424:
    v1 = lw(s1);
    v0 = lw(s1 + 0x24);
    sw(v0, v1 + 0x68);
loc_8001A434:
    s2 = 0x59;                                          // Result = 00000059
    v1 = lw(s1 + 0x6C);
    v0 = 1;                                             // Result = 00000001
    sw(v0, s1 + 0x34);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x748;                                        // Result = STR_BerserkPickedUpMsg[0] (80010748)
    sw(v0, s1 + 0xD4);
    if (v1 == 0) goto loc_8001A4EC;
    sw(0, s1 + 0x70);
    goto loc_8001A4EC;
loc_8001A45C:
    a0 = lw(s1);
    v0 = 0x384;                                         // Result = 00000384
    sw(v0, s1 + 0x38);
    v0 = lw(a0 + 0x64);
    v1 = 0x70000000;                                    // Result = 70000000
    v0 |= v1;
    sw(v0, a0 + 0x64);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x754;                                        // Result = STR_PartialInvisibilityPickedUpMsg[0] (80010754)
    s2 = 0x59;                                          // Result = 00000059
    goto loc_8001A4E8;
loc_8001A488:
    v0 = 0x384;                                         // Result = 00000384
    sw(v0, s1 + 0x3C);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x76C;                                        // Result = STR_RadSuitPickedUpMsg[0] (8001076C)
    s2 = 0x59;                                          // Result = 00000059
    goto loc_8001A4E8;
loc_8001A4A0:
    v0 = lw(s1 + 0x40);
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001A4B8;
    }
    v0 = 1;                                             // Result = 00000001
    sw(v0, s1 + 0x40);
loc_8001A4B8:
    if (v0 == 0) goto loc_8001A55C;
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x788;                                        // Result = STR_ComputerAreaMapPickedUpMsg[0] (80010788)
    sw(v0, s1 + 0xD4);
loc_8001A4CC:
    s2 = 0x59;                                          // Result = 00000059
    goto loc_8001A4EC;
loc_8001A4D4:
    s2 = 0x59;                                          // Result = 00000059
    v0 = 0x708;                                         // Result = 00000708
    sw(v0, s1 + 0x44);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x79C;                                        // Result = STR_LightAmplificationGogglesUpMsg[0] (8001079C)
loc_8001A4E8:
    sw(v0, s1 + 0xD4);
loc_8001A4EC:
    v0 = lw(s3 + 0x64);
    v1 = 0x800000;                                      // Result = 00800000
    v0 &= v1;
    if (v0 == 0) goto loc_8001A510;
    v0 = lw(s1 + 0xCC);
    v0++;
    sw(v0, s1 + 0xCC);
loc_8001A510:
    a0 = s3;
    P_RemoveMObj();
    v0 = lw(s1 + 0xDC);
    a0 = *gCurPlayerIndex;
    v0 += 4;
    v1 = a0 << 2;
    v1 += a0;
    sw(v0, s1 + 0xDC);
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    a0 = 0;                                             // Result = 00000000
    if (s1 != v0) goto loc_8001A55C;
    a1 = s2;
    S_StartSound();
loc_8001A55C:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_KillMObj() noexcept {
loc_8001A57C:
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a1;
    v1 = 0xFEFF0000;                                    // Result = FEFF0000
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x64);
    v1 |= 0xBFFB;                                       // Result = FEFFBFFB
    a1 = v0 & v1;
    v1 = lw(s0 + 0x54);
    v0 = 0xE;                                           // Result = 0000000E
    sw(a1, s0 + 0x64);
    if (v1 == v0) goto loc_8001A5BC;
    v0 = -0x201;                                        // Result = FFFFFDFF
    v0 &= a1;
    sw(v0, s0 + 0x64);
loc_8001A5BC:
    v0 = 0x100000;                                      // Result = 00100000
    v1 = lw(s0 + 0x64);
    v0 |= 0x400;                                        // Result = 00100400
    v1 |= v0;
    v0 = lw(s0 + 0x44);
    s1 = 0;                                             // Result = 00000000
    sw(v1, s0 + 0x64);
    v1 = lw(s0 + 0x80);
    v0 = u32(i32(v0) >> 2);
    sw(v0, s0 + 0x44);
    if (v1 == 0) goto loc_8001A730;
    if (a0 == 0) goto loc_8001A608;
    a0 = lw(a0 + 0x80);
    if (a0 == 0) goto loc_8001A608;
    if (a0 != v1) goto loc_8001A624;
loc_8001A608:
    v1 = lw(s0 + 0x80);
    v0 = lw(v1 + 0x64);
    v0--;
    sw(v0, v1 + 0x64);
    goto loc_8001A634;
loc_8001A624:
    v0 = lw(a0 + 0x64);
    v0++;
    sw(v0, a0 + 0x64);
loc_8001A634:
    v0 = lw(s0 + 0x64);
    v1 = -3;                                            // Result = FFFFFFFD
    v0 &= v1;
    v1 = lw(s0 + 0x80);
    sw(v0, s0 + 0x64);
    v0 = 1;                                             // Result = 00000001
    sw(v0, v1 + 0x4);
    a0 = lw(s0 + 0x80);
    P_DropWeapon();
    v0 = lw(s0 + 0x68);
    v0 = (i32(v0) < -0x32);
    a0 = s0;
    if (v0 == 0) goto loc_8001A6B8;
    v0 = *gCurPlayerIndex;
    a0 = lw(s0 + 0x80);
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    s1 = 1;                                             // Result = 00000001
    if (a0 != v0) goto loc_8001A6AC;
    at = 0x800A0000;                                    // Result = 800A0000
    sw(s1, at - 0x78CC);                                // Store to: gStatusBar[7] (80098734)
loc_8001A6AC:
    a0 = s0;
    a1 = 0x23;                                          // Result = 00000023
    goto loc_8001A6BC;
loc_8001A6B8:
    a1 = sfx_pldeth;
loc_8001A6BC:
    S_StartSound();
    v1 = *gNumMObjKilled;
    v0 = (i32(v1) < 0x20);
    {
        const bool bJump = (v0 != 0);
        v0 = v1 & 0x1F;
        if (bJump) goto loc_8001A708;
    }
    v0 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7894;                                       // Result = gMObjPendingRemovalQueue[0] (800A876C)
    at += v0;
    a0 = lw(at);
    P_RemoveMObj();
    v1 = *gNumMObjKilled;
    v0 = v1 & 0x1F;
loc_8001A708:
    v0 <<= 2;
    v1++;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x7894;                                       // Result = gMObjPendingRemovalQueue[0] (800A876C)
    at += v0;
    sw(s0, at);
    *gNumMObjKilled = v1;
    goto loc_8001A7B0;
loc_8001A730:
    if (a0 == 0) goto loc_8001A770;
    a0 = lw(a0 + 0x80);
    v1 = 0x400000;                                      // Result = 00400000
    if (a0 == 0) goto loc_8001A770;
    v0 = lw(s0 + 0x64);
    v0 &= v1;
    if (v0 == 0) goto loc_8001A770;
    v0 = lw(a0 + 0xC8);
    v0++;
    sw(v0, a0 + 0xC8);
    goto loc_8001A7B0;
loc_8001A770:
    v0 = *gNetGame;
    v1 = 0x400000;
    if (v0 != gt_single) goto loc_8001A7B0;
    v0 = lw(s0 + 0x64);
    v0 &= v1;
    if (v0 == 0) goto loc_8001A7B0;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x774C;                                       // Result = gPlayer1[32] (800A88B4)
    v0 = lw(v1);                                        // Load from: gPlayer1[32] (800A88B4)
    v0++;
    sw(v0, v1);                                         // Store to: gPlayer1[32] (800A88B4)
loc_8001A7B0:
    if (s1 != 0) goto loc_8001A7D8;
    v0 = lw(s0 + 0x58);
    v0 = lw(v0 + 0x8);
    v1 = lw(s0 + 0x68);
    v0 = -v0;
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_8001A7F0;
loc_8001A7D8:
    v0 = lw(s0 + 0x58);
    a1 = lw(v0 + 0x34);
    if (a1 != 0) goto loc_8001A7FC;
loc_8001A7F0:
    v0 = lw(s0 + 0x58);
    a1 = lw(v0 + 0x30);
loc_8001A7FC:
    a0 = s0;
    P_SetMObjState();
    _thunk_P_Random();
    v1 = lw(s0 + 0x5C);
    v0 &= 1;
    v1 -= v0;
    sw(v1, s0 + 0x5C);
    if (i32(v1) > 0) goto loc_8001A828;
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0x5C);
loc_8001A828:
    v1 = lw(s0 + 0x54);
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (v1 == v0);
        v0 = (v1 < 3);
        if (bJump) goto loc_8001A864;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001A850;
    }
    a3 = 0x35;                                          // Result = 00000035
    if (v1 == v0) goto loc_8001A868;
    goto loc_8001A888;
loc_8001A850:
    v0 = 8;                                             // Result = 00000008
    a3 = 0x3F;                                          // Result = 0000003F
    if (v1 == v0) goto loc_8001A868;
    goto loc_8001A888;
loc_8001A864:
    a3 = 0x43;                                          // Result = 00000043
loc_8001A868:
    a0 = lw(s0);
    a1 = lw(s0 + 0x4);
    a2 = 0x80000000;                                    // Result = 80000000
    P_SpawnMObj();
    v1 = lw(v0 + 0x64);
    a0 = 0x20000;                                       // Result = 00020000
    v1 |= a0;
    sw(v1, v0 + 0x64);
loc_8001A888:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_DamageMObj() noexcept {
loc_8001A8A0:
    sp -= 0x30;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s5, sp + 0x24);
    s5 = a1;
    sw(s6, sp + 0x28);
    s6 = a2;
    sw(ra, sp + 0x2C);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v1 = lw(s0 + 0x64);
    v0 = v1 & 4;
    s3 = a3;
    if (v0 == 0) goto loc_8001AD48;
    v0 = lw(s0 + 0x68);
    {
        const bool bJump = (i32(v0) <= 0);
        v0 = 0x1000000;                                 // Result = 01000000
        if (bJump) goto loc_8001AD48;
    }
    v0 &= v1;
    if (v0 == 0) goto loc_8001A90C;
    sw(0, s0 + 0x50);
    sw(0, s0 + 0x4C);
    sw(0, s0 + 0x48);
loc_8001A90C:
    s2 = lw(s0 + 0x80);
    if (s2 == 0) goto loc_8001A97C;
    v0 = *gGameSkill;
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(s3) < 0x1F);
        if (bJump) goto loc_8001A938;
    }
    s3 = u32(i32(s3) >> 1);
    v0 = (i32(s3) < 0x1F);
loc_8001A938:
    if (v0 != 0) goto loc_8001A97C;
    v0 = *gCurPlayerIndex;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    {
        const bool bJump = (s2 != v0);
        v0 = 5;                                         // Result = 00000005
        if (bJump) goto loc_8001A97C;
    }
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78E8);                                // Store to: gStatusBar[0] (80098718)
loc_8001A97C:
    if (s5 == 0) goto loc_8001AB20;
    if (s6 == 0) goto loc_8001A9AC;
    v0 = lw(s6 + 0x80);
    if (v0 == 0) goto loc_8001A9AC;
    v1 = lw(v0 + 0x6C);
    v0 = 8;                                             // Result = 00000008
    if (v1 == v0) goto loc_8001AB20;
loc_8001A9AC:
    a0 = lw(s5);
    a1 = lw(s5 + 0x4);
    a2 = lw(s0);
    a3 = lw(s0 + 0x4);
    _thunk_R_PointToAngle2();
    s4 = v0;
    v0 = s3 << 1;
    v0 += s3;
    v0 <<= 3;
    v1 = lw(s0 + 0x58);
    v0 += s3;
    v1 = lw(v1 + 0x48);
    v0 <<= 16;
    div(v0, v1);
    if (v1 != 0) goto loc_8001A9F4;
    _break(0x1C00);
loc_8001A9F4:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001AA0C;
    }
    if (v0 != at) goto loc_8001AA0C;
    tge(zero, zero, 0x5D);
loc_8001AA0C:
    s1 = lo;
    v0 = (i32(s3) < 0x28);
    a0 = s4 >> 19;
    if (v0 == 0) goto loc_8001AA70;
    v0 = lw(s0 + 0x68);
    v0 = (i32(v0) < i32(s3));
    if (v0 == 0) goto loc_8001AA70;
    v0 = lw(s0 + 0x8);
    v1 = lw(s5 + 0x8);
    v0 -= v1;
    v1 = 0x400000;                                      // Result = 00400000
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_8001AA70;
    _thunk_P_Random();
    v0 &= 1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001AA6C;
    }
    s4 -= v0;
    s1 <<= 2;
loc_8001AA6C:
    a0 = s4 >> 19;
loc_8001AA70:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a0 <<= 2;
    v0 += a0;
    v0 = lw(v0);
    s1 = u32(i32(s1) >> 16);
    mult(s1, v0);
    v1 = lw(s0 + 0x48);
    v0 = lo;
    v0 += v1;
    sw(v0, s0 + 0x48);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += a0;
    v0 = lw(at);
    mult(s1, v0);
    v1 = lw(s0 + 0x4C);
    a0 = lw(s0 + 0x80);
    v0 = lo;
    v0 += v1;
    sw(v0, s0 + 0x4C);
    if (a0 == 0) goto loc_8001AB24;
    v1 = lw(s0 + 0x48);
    a0 = 0x100000;                                      // Result = 00100000
    v0 = (i32(a0) < i32(v1));
    if (v0 != 0) goto loc_8001AAF0;
    a0 = 0xFFF00000;                                    // Result = FFF00000
    v0 = (i32(v1) < i32(a0));
    if (v0 == 0) goto loc_8001AAF4;
loc_8001AAF0:
    sw(a0, s0 + 0x48);
loc_8001AAF4:
    v1 = lw(s0 + 0x4C);
    a0 = 0x100000;                                      // Result = 00100000
    v0 = (i32(a0) < i32(v1));
    if (v0 != 0) goto loc_8001AB18;
    a0 = 0xFFF00000;                                    // Result = FFF00000
    v0 = (i32(v1) < i32(a0));
    if (v0 == 0) goto loc_8001AB24;
loc_8001AB18:
    sw(a0, s0 + 0x4C);
    goto loc_8001AB24;
loc_8001AB20:
    s4 = lw(s0 + 0x24);
loc_8001AB24:
    if (s2 == 0) goto loc_8001AC70;
    v0 = lw(s2 + 0xC0);
    v0 &= 2;
    if (v0 != 0) goto loc_8001AD48;
    v0 = lw(s2 + 0x30);
    if (i32(v0) > 0) goto loc_8001AD48;
    v0 = *gCurPlayerIndex;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    v0 += v1;
    {
        const bool bJump = (s2 != v0);
        v0 = 0xCFFF0000;                                // Result = CFFF0000
        if (bJump) goto loc_8001ABCC;
    }
    v0 |= 0xFFFF;                                       // Result = CFFFFFFF
    a0 = 0x4FFF0000;                                    // Result = 4FFF0000
    v1 = lw(s0 + 0x24);
    a0 |= 0xFFFE;                                       // Result = 4FFFFFFE
    s4 -= v1;
    v0 += s4;
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = 0x7FFF0000;                                // Result = 7FFF0000
        if (bJump) goto loc_8001ABB0;
    }
    v0 = 4;                                             // Result = 00000004
    goto loc_8001ABC4;
loc_8001ABB0:
    v0 |= 0xFFFF;                                       // Result = 7FFFFFFF
    v0 += s4;
    v0 = (a0 < v0);
    {
        const bool bJump = (v0 != 0);
        v0 = 3;                                         // Result = 00000003
        if (bJump) goto loc_8001ABCC;
    }
loc_8001ABC4:
    at = 0x800A0000;                                    // Result = 800A0000
    sw(v0, at - 0x78E8);                                // Store to: gStatusBar[0] (80098718)
loc_8001ABCC:
    v1 = lw(s2 + 0x2C);
    v0 = 1;                                             // Result = 00000001
    if (v1 == 0) goto loc_8001AC34;
    {
        const bool bJump = (v1 != v0);
        v0 = s3 >> 31;
        if (bJump) goto loc_8001AC00;
    }
    v0 = 0x55550000;                                    // Result = 55550000
    v0 |= 0x5556;                                       // Result = 55555556
    mult(s3, v0);
    v1 = u32(i32(s3) >> 31);
    v0 = hi;
    v1 = v0 - v1;
    goto loc_8001AC08;
loc_8001AC00:
    v0 += s3;
    v1 = u32(i32(v0) >> 1);
loc_8001AC08:
    a0 = lw(s2 + 0x28);
    v0 = (i32(v1) < i32(a0));
    if (v0 != 0) goto loc_8001AC24;
    v1 = a0;
    sw(0, s2 + 0x2C);
loc_8001AC24:
    v0 = lw(s2 + 0x28);
    s3 -= v1;
    v0 -= v1;
    sw(v0, s2 + 0x28);
loc_8001AC34:
    a0 = s0;
    a1 = sfx_plpain;
    S_StartSound();
    v0 = lw(s2 + 0x24);
    v0 -= s3;
    sw(v0, s2 + 0x24);
    if (i32(v0) >= 0) goto loc_8001AC58;
    sw(0, s2 + 0x24);
loc_8001AC58:
    v0 = lw(s2 + 0xD8);
    v1 = u32(i32(s3) >> 1);
    sw(s6, s2 + 0xE0);
    v0++;
    v0 += v1;
    sw(v0, s2 + 0xD8);
loc_8001AC70:
    v0 = lw(s0 + 0x68);
    v0 -= s3;
    sw(v0, s0 + 0x68);
    if (i32(v0) > 0) goto loc_8001AC98;
    a0 = s6;
    a1 = s0;
    P_KillMObj();
    goto loc_8001AD48;
loc_8001AC98:
    _thunk_P_Random();
    v1 = lw(s0 + 0x58);
    v1 = lw(v1 + 0x20);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0x1000000;                                 // Result = 01000000
        if (bJump) goto loc_8001ACE4;
    }
    a0 = lw(s0 + 0x64);
    v0 &= a0;
    {
        const bool bJump = (v0 != 0);
        v0 = a0 | 0x40;
        if (bJump) goto loc_8001ACE4;
    }
    v1 = lw(s0 + 0x58);
    sw(v0, s0 + 0x64);
    a1 = lw(v1 + 0x1C);
    a0 = s0;
    P_SetMObjState();
loc_8001ACE4:
    v0 = lw(s0 + 0x7C);
    sw(0, s0 + 0x78);
    if (v0 != 0) goto loc_8001AD48;
    v0 = 0x64;                                          // Result = 00000064
    if (s6 == 0) goto loc_8001AD48;
    a1 = lw(s0 + 0x58);
    sw(s6, s0 + 0x74);
    sw(v0, s0 + 0x7C);
    v1 = lw(a1 + 0x4);
    a0 = lw(s0 + 0x60);
    v0 = v1 << 3;
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    if (a0 != v0) goto loc_8001AD48;
    a1 = lw(a1 + 0xC);
    if (a1 == 0) goto loc_8001AD48;
    a0 = s0;
    P_SetMObjState();
loc_8001AD48:
    ra = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x30;
    return;
}
