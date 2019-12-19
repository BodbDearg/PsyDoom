#include "p_plats.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/z_zone.h"
#include "g_game.h"
#include "p_floor.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"

void T_PlatRaise() noexcept {
    sp -= 0x28;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    v1 = lw(s0 + 0x24);
    a0 = 1;                                             // Result = 00000001
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 == a0) goto loc_8001F3B8;
    v0 = 2;                                             // Result = 00000002
    if (v1 == 0) goto loc_8001F2BC;
    if (v1 == v0) goto loc_8001F3FC;
    goto loc_8001F44C;
loc_8001F2BC:
    sw(0, sp + 0x10);
    sw(a0, sp + 0x14);
    a0 = lw(s0 + 0xC);
    a1 = lw(s0 + 0x10);
    a2 = lw(s0 + 0x18);
    a3 = lw(s0 + 0x2C);
    T_MovePlane();
    v1 = lw(s0 + 0x34);
    v1 -= 2;
    v1 = (v1 < 2);
    s1 = v0;
    if (v1 == 0) goto loc_8001F31C;
    v0 = *gGameTic;
    v0 &= 7;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001F320;
    }
    a0 = lw(s0 + 0xC);
    a1 = 0x15;                                          // Result = 00000015
    a0 += 0x38;
    S_StartSound();
loc_8001F31C:
    v0 = 1;                                             // Result = 00000001
loc_8001F320:
    {
        const bool bJump = (s1 != v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8001F354;
    }
    v0 = lw(s0 + 0x2C);
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8001F354;
    }
    a0 = lw(s0 + 0xC);
    v0 = lw(s0 + 0x1C);
    a1 = 0x11;                                          // Result = 00000011
    sw(s1, s0 + 0x24);
    a0 += 0x38;
    sw(v0, s0 + 0x20);
    goto loc_8001F444;
loc_8001F354:
    a1 = 0x12;                                          // Result = 00000012
    if (s1 != v0) goto loc_8001F44C;
    a0 = lw(s0 + 0xC);
    v0 = lw(s0 + 0x1C);
    sw(s1, s0 + 0x24);
    a0 += 0x38;
    sw(v0, s0 + 0x20);
    S_StartSound();
    v1 = lw(s0 + 0x34);
    v0 = (v1 < 3);
    if (v1 == s1) goto loc_8001F3A8;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001F39C;
    }
    if (v1 == v0) goto loc_8001F3A8;
    goto loc_8001F44C;
loc_8001F39C:
    v0 = 4;                                             // Result = 00000004
    if (v1 != v0) goto loc_8001F44C;
loc_8001F3A8:
    a0 = s0;
    P_RemoveActivePlat();
    goto loc_8001F44C;
loc_8001F3B8:
    sw(0, sp + 0x10);
    sw(v0, sp + 0x14);
    a0 = lw(s0 + 0xC);
    a1 = lw(s0 + 0x10);
    a2 = lw(s0 + 0x14);
    a3 = 0;                                             // Result = 00000000
    T_MovePlane();
    s1 = v0;
    v0 = 2;                                             // Result = 00000002
    a1 = 0x12;                                          // Result = 00000012
    if (s1 != v0) goto loc_8001F44C;
    a0 = lw(s0 + 0xC);
    v0 = lw(s0 + 0x1C);
    sw(s1, s0 + 0x24);
    a0 += 0x38;
    sw(v0, s0 + 0x20);
    goto loc_8001F444;
loc_8001F3FC:
    v0 = lw(s0 + 0x20);
    v0--;
    sw(v0, s0 + 0x20);
    if (v0 != 0) goto loc_8001F44C;
    v0 = lw(s0 + 0xC);
    v1 = lw(v0);
    v0 = lw(s0 + 0x14);
    if (v1 != v0) goto loc_8001F434;
    sw(0, s0 + 0x24);
    goto loc_8001F438;
loc_8001F434:
    sw(a0, s0 + 0x24);
loc_8001F438:
    a0 = lw(s0 + 0xC);
    a1 = 0x11;                                          // Result = 00000011
    a0 += 0x38;
loc_8001F444:
    S_StartSound();
loc_8001F44C:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void EV_DoPlat() noexcept {
loc_8001F464:
    sp -= 0x38;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s4, sp + 0x20);
    s4 = a1;
    sw(s7, sp + 0x2C);
    s7 = a2;
    sw(s2, sp + 0x18);
    s2 = -1;                                            // Result = FFFFFFFF
    sw(s5, sp + 0x24);
    s5 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x30);
    sw(s6, sp + 0x28);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    if (s4 != 0) goto loc_8001F4B0;
    a0 = lw(s3 + 0x18);
    P_ActivateInStasis();
loc_8001F4B0:
    s6 = 0x2D;                                          // Result = 0000002D
loc_8001F4B4:
    a0 = s3;
loc_8001F4B8:
    a1 = s2;
    P_FindSectorFromLineTag();
    s2 = v0;
    v0 = s2 << 1;
    if (i32(s2) < 0) goto loc_8001F72C;
    v0 += s2;
    v0 <<= 3;
    v0 -= s2;
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7F58);                               // Load from: gpSectors (800780A8)
    v0 <<= 2;
    s1 = v0 + v1;
    v0 = lw(s1 + 0x50);
    a1 = 0x38;                                          // Result = 00000038
    if (v0 != 0) goto loc_8001F4B4;
    a2 = 4;                                             // Result = 00000004
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7E68);                               // Load from: gpMainMemZone (80078198)
    a3 = 0;                                             // Result = 00000000
    Z_Malloc2();
    s0 = v0;
    a0 = s0;
    P_AddThinker();
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0xD80;                                        // Result = T_PlatRaise (8001F280)
    sw(s4, s0 + 0x34);
    sw(s1, s0 + 0xC);
    sw(s0, s1 + 0x50);
    sw(v0, s0 + 0x8);
    sw(0, s0 + 0x2C);
    v0 = lw(s3 + 0x18);
    sw(v0, s0 + 0x30);
    v0 = (s4 < 5);
    s5 = 1;                                             // Result = 00000001
    if (v0 == 0) goto loc_8001F71C;
    v0 = s4 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += 0x8E8;                                        // Result = JumpTable_EV_DoPlat[0] (800108E8)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x8001F6AC: goto loc_8001F6AC;
        case 0x8001F61C: goto loc_8001F61C;
        case 0x8001F5C8: goto loc_8001F5C8;
        case 0x8001F56C: goto loc_8001F56C;
        case 0x8001F664: goto loc_8001F664;
        default: jump_table_err(); break;
    }
loc_8001F56C:
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s0 + 0x10);
    v1 = lw(s3 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EA0);                               // Load from: gpSides (80077EA0)
    v0 <<= 3;
    v0 += v1;
    v0 = lw(v0 + 0x14);
    a1 = lw(s1);
    v0 = lw(v0 + 0x8);
    a0 = s1;
    sw(v0, s1 + 0x8);
    P_FindNextHighestFloor();
    a0 = s1 + 0x38;
    a1 = 0x15;                                          // Result = 00000015
    sw(v0, s0 + 0x18);
    sw(0, s0 + 0x1C);
    sw(0, s0 + 0x24);
    sw(0, s1 + 0x14);
    goto loc_8001F714;
loc_8001F5C8:
    v0 = 0x10000;                                       // Result = 00010000
    sw(v0, s0 + 0x10);
    v1 = lw(s3 + 0x1C);
    a0 = s1 + 0x38;
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EA0);                               // Load from: gpSides (80077EA0)
    v0 <<= 3;
    v0 += v1;
    v0 = lw(v0 + 0x14);
    v1 = lw(s1);
    v0 = lw(v0 + 0x8);
    a1 = 0x15;                                          // Result = 00000015
    sw(v0, s1 + 0x8);
    v0 = s7 << 16;
    v0 += v1;
    sw(v0, s0 + 0x18);
    sw(0, s0 + 0x1C);
    sw(0, s0 + 0x24);
    goto loc_8001F714;
loc_8001F61C:
    a0 = s1;
    v0 = 0x80000;                                       // Result = 00080000
    sw(v0, s0 + 0x10);
    P_FindLowestFloorSurrounding();
    sw(v0, s0 + 0x14);
    v1 = lw(s1);
    v0 = (i32(v1) < i32(v0));
    a0 = s1 + 0x38;
    if (v0 == 0) goto loc_8001F648;
    sw(v1, s0 + 0x14);
loc_8001F648:
    a1 = 0x11;                                          // Result = 00000011
    v1 = lw(s1);
    v0 = 1;                                             // Result = 00000001
    sw(s6, s0 + 0x1C);
    sw(v0, s0 + 0x24);
    sw(v1, s0 + 0x18);
    goto loc_8001F714;
loc_8001F664:
    a0 = s1;
    v0 = 0x100000;                                      // Result = 00100000
    sw(v0, s0 + 0x10);
    P_FindLowestFloorSurrounding();
    sw(v0, s0 + 0x14);
    v1 = lw(s1);
    v0 = (i32(v1) < i32(v0));
    a0 = s1 + 0x38;
    if (v0 == 0) goto loc_8001F690;
    sw(v1, s0 + 0x14);
loc_8001F690:
    a1 = 0x11;                                          // Result = 00000011
    v1 = lw(s1);
    v0 = 1;                                             // Result = 00000001
    sw(s6, s0 + 0x1C);
    sw(v0, s0 + 0x24);
    sw(v1, s0 + 0x18);
    goto loc_8001F714;
loc_8001F6AC:
    a0 = s1;
    v0 = 0x20000;                                       // Result = 00020000
    sw(v0, s0 + 0x10);
    P_FindLowestFloorSurrounding();
    sw(v0, s0 + 0x14);
    v1 = lw(s1);
    v0 = (i32(v1) < i32(v0));
    if (v0 == 0) goto loc_8001F6D8;
    sw(v1, s0 + 0x14);
loc_8001F6D8:
    a0 = s1;
    P_FindHighestFloorSurrounding();
    sw(v0, s0 + 0x18);
    v1 = lw(s1);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001F6FC;
    sw(v1, s0 + 0x18);
loc_8001F6FC:
    sw(s6, s0 + 0x1C);
    P_Random();
    a0 = s1 + 0x38;
    a1 = 0x11;                                          // Result = 00000011
    v0 &= 1;
    sw(v0, s0 + 0x24);
loc_8001F714:
    S_StartSound();
loc_8001F71C:
    a0 = s0;
    P_AddActivePlat();
    a0 = s3;
    goto loc_8001F4B8;
loc_8001F72C:
    v0 = s5;
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

void P_ActivateInStasis() noexcept {
loc_8001F760:
    a2 = 0;                                             // Result = 00000000
    t0 = 3;                                             // Result = 00000003
    a3 = 0x80020000;                                    // Result = 80020000
    a3 -= 0xD80;                                        // Result = T_PlatRaise (8001F280)
    a1 = 0x80090000;                                    // Result = 80090000
    a1 += 0x7C44;                                       // Result = gpActivePlats[0] (80097C44)
loc_8001F778:
    v1 = lw(a1);
    a2++;
    if (v1 == 0) goto loc_8001F7C0;
    v0 = lw(v1 + 0x30);
    {
        const bool bJump = (v0 != a0);
        v0 = (i32(a2) < 0x1E);
        if (bJump) goto loc_8001F7C4;
    }
    v0 = lw(v1 + 0x24);
    {
        const bool bJump = (v0 != t0);
        v0 = (i32(a2) < 0x1E);
        if (bJump) goto loc_8001F7C4;
    }
    v0 = lw(v1 + 0x28);
    sw(v0, v1 + 0x24);
    v0 = lw(a1);
    sw(a3, v0 + 0x8);
loc_8001F7C0:
    v0 = (i32(a2) < 0x1E);
loc_8001F7C4:
    a1 += 4;
    if (v0 != 0) goto loc_8001F778;
    return;
}

void EV_StopPlat() noexcept {
loc_8001F7D4:
    t0 = 0;                                             // Result = 00000000
    t1 = 3;                                             // Result = 00000003
    a3 = 0x80090000;                                    // Result = 80090000
    a3 += 0x7C44;                                       // Result = gpActivePlats[0] (80097C44)
loc_8001F7E4:
    a1 = lw(a3);
    t0++;
    if (a1 == 0) goto loc_8001F834;
    a2 = lw(a1 + 0x24);
    v0 = (i32(t0) < 0x1E);
    if (a2 == t1) goto loc_8001F838;
    v1 = lw(a1 + 0x30);
    v0 = lw(a0 + 0x18);
    {
        const bool bJump = (v1 != v0);
        v0 = (i32(t0) < 0x1E);
        if (bJump) goto loc_8001F838;
    }
    sw(a2, a1 + 0x28);
    v0 = lw(a3);
    sw(t1, v0 + 0x24);
    v0 = lw(a3);
    sw(0, v0 + 0x8);
loc_8001F834:
    v0 = (i32(t0) < 0x1E);
loc_8001F838:
    a3 += 4;
    if (v0 != 0) goto loc_8001F7E4;
    return;
}

void P_AddActivePlat() noexcept {
loc_8001F848:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    a1 = 0;                                             // Result = 00000000
    v1 = 0x80090000;                                    // Result = 80090000
    v1 += 0x7C44;                                       // Result = gpActivePlats[0] (80097C44)
loc_8001F85C:
    v0 = lw(v1);
    a1++;
    if (v0 != 0) goto loc_8001F874;
    sw(a0, v1);
    goto loc_8001F890;
loc_8001F874:
    v0 = (i32(a1) < 0x1E);
    v1 += 4;
    if (v0 != 0) goto loc_8001F85C;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x8A0;                                        // Result = STR_P_AddActivePlat_NoMorePlats_Err[0] (800108A0)
    I_Error();
loc_8001F890:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_RemoveActivePlat() noexcept {
loc_8001F8A0:
    sp -= 0x18;
    v1 = 0;                                             // Result = 00000000
    sw(s0, sp + 0x10);
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7C44;                                       // Result = gpActivePlats[0] (80097C44)
    sw(ra, sp + 0x14);
loc_8001F8B8:
    v0 = lw(s0);
    v1++;
    if (a0 != v0) goto loc_8001F8E8;
    v0 = lw(a0 + 0xC);
    sw(0, v0 + 0x50);
    a0 = lw(s0);
    P_RemoveThinker();
    sw(0, s0);
    goto loc_8001F904;
loc_8001F8E8:
    v0 = (i32(v1) < 0x1E);
    s0 += 4;
    if (v0 != 0) goto loc_8001F8B8;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x8C0;                                        // Result = STR_P_RemoveActivePlat_BadPlat_Err[0] (800108C0)
    I_Error();
loc_8001F904:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}
