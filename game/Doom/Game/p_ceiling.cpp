#include "p_ceiling.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "g_game.h"
#include "p_floor.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"

void T_MoveCeiling() noexcept {
    sp -= 0x28;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    v1 = lw(s0 + 0x24);
    if (v1 == 0) goto loc_80014C2C;
    v0 = 1;                                             // Result = 00000001
    if (i32(v1) > 0) goto loc_80014A70;
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 == v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80014B2C;
    }
    goto loc_80014C2C;
loc_80014A70:
    if (v1 != v0) goto loc_80014C2C;
    sw(v1, sp + 0x10);
    v0 = lw(s0 + 0x24);
    sw(v0, sp + 0x14);
    a0 = lw(s0 + 0x10);
    a1 = lw(s0 + 0x1C);
    a2 = lw(s0 + 0x18);
    a3 = 0;                                             // Result = 00000000
    T_MovePlane();
    v1 = *gGameTic;
    v1 &= 7;
    s1 = v0;
    if (v1 != 0) goto loc_80014AD0;
    v1 = lw(s0 + 0xC);
    v0 = 5;
    a1 = sfx_stnmov;
    if (v1 == v0) goto loc_80014AD0;
    a0 = lw(s0 + 0x10);
    a0 += 0x38;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
loc_80014AD0:
    v0 = 2;                                             // Result = 00000002
    if (s1 != v0) goto loc_80014C2C;
    v1 = lw(s0 + 0xC);
    v0 = (v1 < 5);
    {
        const bool bJump = (v0 == 0);
        v0 = (v1 < 3);
        if (bJump) goto loc_80014B08;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80014B20;
    }
    if (v1 == v0) goto loc_80014BE8;
    goto loc_80014C2C;
loc_80014B08:
    v0 = 5;                                             // Result = 00000005
    a1 = sfx_pstop;
    if (v1 != v0) goto loc_80014C2C;
    a0 = lw(s0 + 0x10);
    a0 += 0x38;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
loc_80014B20:
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, s0 + 0x24);
    goto loc_80014C2C;
loc_80014B2C:
    sw(v0, sp + 0x10);
    v0 = lw(s0 + 0x24);
    sw(v0, sp + 0x14);
    a0 = lw(s0 + 0x10);
    a1 = lw(s0 + 0x1C);
    a2 = lw(s0 + 0x14);
    a3 = lw(s0 + 0x20);
    T_MovePlane();
    v1 = *gGameTic;
    v1 &= 7;
    s1 = v0;
    if (v1 != 0) goto loc_80014B88;
    v1 = lw(s0 + 0xC);
    v0 = 5;
    a1 = sfx_stnmov;
    if (v1 == v0) goto loc_80014B88;
    a0 = lw(s0 + 0x10);
    a0 += 0x38;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
loc_80014B88:
    v0 = 2;                                             // Result = 00000002
    {
        const bool bJump = (s1 != v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80014BF8;
    }
    v1 = lw(s0 + 0xC);
    v0 = (v1 < 6);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80014C2C;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80014BE8: goto loc_80014BE8;
        case 0x80014C2C: goto loc_80014C2C;
        case 0x80014BD4: goto loc_80014BD4;
        case 0x80014BDC: goto loc_80014BDC;
        case 0x80014BC4: goto loc_80014BC4;
        default: jump_table_err(); break;
    }
loc_80014BC4:
    a0 = lw(s0 + 0x10);
    a1 = sfx_pstop;
    a0 += 0x38;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
loc_80014BD4:
    v0 = 0x20000;                                       // Result = 00020000
    sw(v0, s0 + 0x1C);
loc_80014BDC:
    v0 = 1;                                             // Result = 00000001
    sw(v0, s0 + 0x24);
    goto loc_80014C2C;
loc_80014BE8:
    a0 = s0;
    P_RemoveActiveCeiling();
    goto loc_80014C2C;
loc_80014BF8:
    if (s1 != v0) goto loc_80014C2C;
    v1 = lw(s0 + 0xC);
    v0 = (v1 < 2);
    {
        const bool bJump = (v0 != 0);
        v0 = (v1 < 4);
        if (bJump) goto loc_80014C2C;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0x4000;                                    // Result = 00004000
        if (bJump) goto loc_80014C28;
    }
    v0 = 5;                                             // Result = 00000005
    {
        const bool bJump = (v1 != v0);
        v0 = 0x4000;                                    // Result = 00004000
        if (bJump) goto loc_80014C2C;
    }
loc_80014C28:
    sw(v0, s0 + 0x1C);
loc_80014C2C:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void EV_DoCeiling() noexcept {
loc_80014C44:
    sp -= 0x38;
    sw(s5, sp + 0x24);
    s5 = a0;
    sw(s3, sp + 0x1C);
    s3 = a1;
    sw(s2, sp + 0x18);
    s2 = -1;                                            // Result = FFFFFFFF
    sw(s6, sp + 0x28);
    s6 = 0;                                             // Result = 00000000
    v0 = (s3 < 6);
    sw(ra, sp + 0x30);
    sw(s7, sp + 0x2C);
    sw(s4, sp + 0x20);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    if (v0 == 0) goto loc_80014C98;
    v0 = (s3 < 3);
    if (v0 != 0) goto loc_80014C98;
    P_ActivateInStasisCeiling();
loc_80014C98:
    v1 = 0x80010000;                                    // Result = 80010000
    v1 += 0x18;                                         // Result = JumpTable_EV_DoCeiling[0] (80010018)
    v0 = s3 << 2;
    s7 = v0 + v1;
    s4 = 1;                                             // Result = 00000001
loc_80014CAC:
    a0 = s5;
loc_80014CB0:
    a1 = s2;
    v0 = P_FindSectorFromLineTag(*vmAddrToPtr<line_t>(a0), a1);
    s2 = v0;
    v0 = s2 << 1;
    if (i32(s2) < 0) goto loc_80014DE4;
    v0 += s2;
    v0 <<= 3;
    v0 -= s2;
    v1 = *gpSectors;
    v0 <<= 2;
    s1 = v0 + v1;
    v0 = lw(s1 + 0x50);
    a1 = 0x30;                                          // Result = 00000030
    if (v0 != 0) goto loc_80014CAC;
    s6 = 1;                                             // Result = 00000001
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x4A30;                                       // Result = T_MoveCeiling (80014A30)
    sw(s0, s1 + 0x50);
    sw(v0, s0 + 0x8);
    v0 = (s3 < 6);
    sw(s1, s0 + 0x10);
    sw(0, s0 + 0x20);
    if (v0 == 0) goto loc_80014DC8;
    v0 = lw(s7);
    switch (v0) {
        case 0x80014D88: goto loc_80014D88;
        case 0x80014DB0: goto loc_80014DB0;
        case 0x80014D78: goto loc_80014D78;
        case 0x80014D44: goto loc_80014D44;
        default: jump_table_err(); break;
    }
loc_80014D44:
    sw(s4, s0 + 0x20);
    v0 = lw(s1 + 0x4);
    sw(v0, s0 + 0x18);
    v1 = lw(s1);
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, s0 + 0x24);
    v0 = 0x40000;                                       // Result = 00040000
    sw(v0, s0 + 0x1C);
    v0 = 0x80000;                                       // Result = 00080000
    v1 += v0;
    sw(v1, s0 + 0x14);
    goto loc_80014DC8;
loc_80014D78:
    sw(s4, s0 + 0x20);
    v0 = lw(s1 + 0x4);
    sw(v0, s0 + 0x18);
loc_80014D88:
    v1 = lw(s1);
    sw(v1, s0 + 0x14);
    if (s3 == 0) goto loc_80014DA0;
    v0 = 0x80000;                                       // Result = 00080000
    v0 += v1;
    sw(v0, s0 + 0x14);
loc_80014DA0:
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, s0 + 0x24);
    v0 = 0x20000;                                       // Result = 00020000
    goto loc_80014DC4;
loc_80014DB0:
    a0 = s1;
    v0 = P_FindHighestCeilingSurrounding(*vmAddrToPtr<sector_t>(a0));
    sw(v0, s0 + 0x18);
    v0 = 0x20000;                                       // Result = 00020000
    sw(s4, s0 + 0x24);
loc_80014DC4:
    sw(v0, s0 + 0x1C);
loc_80014DC8:
    v0 = lw(s1 + 0x18);
    a0 = s0;
    sw(s3, a0 + 0xC);
    sw(v0, a0 + 0x28);
    P_AddActiveCeiling();
    a0 = s5;
    goto loc_80014CB0;
loc_80014DE4:
    v0 = s6;
    ra = lw(sp + 0x30);
    s7 = lw(sp + 0x2C);
    s6 = lw(sp + 0x28);
    s5 = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x38;
    return;
}

void P_AddActiveCeiling() noexcept {
loc_80014E18:
    a1 = 0;                                             // Result = 00000000
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 -= 0x62E8;                                       // Result = gpActiveCeilings[0] (800A9D18)
loc_80014E24:
    v0 = lw(v1);
    if (v0 != 0) goto loc_80014E3C;
    sw(a0, v1);
    goto loc_80014E4C;
loc_80014E3C:
    a1++;
    v0 = (i32(a1) < 0x1E);
    v1 += 4;
    if (v0 != 0) goto loc_80014E24;
loc_80014E4C:
    return;
}

void P_RemoveActiveCeiling() noexcept {
loc_80014E54:
    sp -= 0x18;
    v1 = 0;                                             // Result = 00000000
    sw(s0, sp + 0x10);
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x62E8;                                       // Result = gpActiveCeilings[0] (800A9D18)
    sw(ra, sp + 0x14);
loc_80014E6C:
    v0 = lw(s0);
    v1++;
    if (v0 != a0) goto loc_80014E9C;
    v0 = lw(v0 + 0x10);
    sw(0, v0 + 0x50);
    a0 = lw(s0);
    _thunk_P_RemoveThinker();
    sw(0, s0);
    goto loc_80014EA8;
loc_80014E9C:
    v0 = (i32(v1) < 0x1E);
    s0 += 4;
    if (v0 != 0) goto loc_80014E6C;
loc_80014EA8:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_ActivateInStasisCeiling() noexcept {
loc_80014EBC:
    a3 = 0;                                             // Result = 00000000
    t0 = 0x80010000;                                    // Result = 80010000
    t0 += 0x4A30;                                       // Result = T_MoveCeiling (80014A30)
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 -= 0x62E8;                                       // Result = gpActiveCeilings[0] (800A9D18)
loc_80014ED0:
    a1 = lw(a2);
    a3++;
    if (a1 == 0) goto loc_80014F1C;
    v1 = lw(a1 + 0x28);
    v0 = lw(a0 + 0x18);
    {
        const bool bJump = (v1 != v0);
        v0 = (i32(a3) < 0x1E);
        if (bJump) goto loc_80014F20;
    }
    v0 = lw(a1 + 0x24);
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a3) < 0x1E);
        if (bJump) goto loc_80014F20;
    }
    v0 = lw(a1 + 0x2C);
    sw(v0, a1 + 0x24);
    v0 = lw(a2);
    sw(t0, v0 + 0x8);
loc_80014F1C:
    v0 = (i32(a3) < 0x1E);
loc_80014F20:
    a2 += 4;
    if (v0 != 0) goto loc_80014ED0;
    return;
}

void EV_CeilingCrushStop() noexcept {
loc_80014F30:
    t0 = 0;                                             // Result = 00000000
    a3 = 0;                                             // Result = 00000000
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 -= 0x62E8;                                       // Result = gpActiveCeilings[0] (800A9D18)
loc_80014F40:
    a1 = lw(a2);
    a3++;
    if (a1 == 0) goto loc_80014F90;
    v1 = lw(a1 + 0x28);
    v0 = lw(a0 + 0x18);
    {
        const bool bJump = (v1 != v0);
        v0 = (i32(a3) < 0x1E);
        if (bJump) goto loc_80014F94;
    }
    v0 = lw(a1 + 0x24);
    if (v0 == 0) goto loc_80014F90;
    sw(v0, a1 + 0x2C);
    v0 = lw(a2);
    sw(0, v0 + 0x8);
    v0 = lw(a2);
    t0 = 1;                                             // Result = 00000001
    sw(0, v0 + 0x24);
loc_80014F90:
    v0 = (i32(a3) < 0x1E);
loc_80014F94:
    a2 += 4;
    if (v0 != 0) goto loc_80014F40;
    v0 = t0;                                            // Result = 00000000
    return;
}
