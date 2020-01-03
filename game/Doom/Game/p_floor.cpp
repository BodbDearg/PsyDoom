#include "p_floor.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Renderer/r_data.h"
#include "g_game.h"
#include "p_change.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"

void T_MovePlane() noexcept {
loc_80018DF0:
    sp -= 0x28;
    sw(s4, sp + 0x20);
    s4 = lw(sp + 0x38);
    sw(s3, sp + 0x1C);
    s3 = lw(sp + 0x3C);
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s2, sp + 0x18);
    s2 = a3;
    sw(ra, sp + 0x24);
    sw(s1, sp + 0x14);
    if (s4 == 0) goto loc_80018E34;
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (s4 == v0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_80018F10;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_80018FEC;
loc_80018E34:
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (s3 == v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80018E50;
    }
    {
        const bool bJump = (s3 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80018EB0;
    }
    goto loc_80018FEC;
loc_80018E50:
    v1 = lw(s0);
    a1 = v1 - a1;
    v0 = (i32(a1) < i32(a2));
    s1 = v1;
    if (v0 == 0) goto loc_80018E8C;
    sw(a2, s0);
    a0 = s0;
    a1 = s2;
    P_ChangeSector();
    v1 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v0 != v1);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80018FEC;
    }
    sw(s1, s0);
    goto loc_80018FC8;
loc_80018E8C:
    sw(a1, s0);
    a0 = s0;
    a1 = s2;
    P_ChangeSector();
    v1 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v0 != v1);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80018FEC;
    }
    sw(s1, s0);
    goto loc_80018F84;
loc_80018EB0:
    v1 = lw(s0);
    a1 += v1;
    v0 = (i32(a2) < i32(a1));
    s1 = v1;
    if (v0 == 0) goto loc_80018EE8;
    sw(a2, s0);
    a0 = s0;
    a1 = s2;
    P_ChangeSector();
    {
        const bool bJump = (v0 != s3);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80018FEC;
    }
    sw(s1, s0);
    goto loc_80018F84;
loc_80018EE8:
    sw(a1, s0);
    a0 = s0;
    a1 = s2;
    P_ChangeSector();
    if (v0 != s3) goto loc_80018FE8;
    {
        const bool bJump = (s2 == v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80018FEC;
    }
    sw(s1, s0);
    goto loc_80018F84;
loc_80018F10:
    if (s3 == v0) goto loc_80018F28;
    v0 = 0;                                             // Result = 00000000
    if (s3 == s4) goto loc_80018F98;
    goto loc_80018FEC;
loc_80018F28:
    v1 = lw(s0 + 0x4);
    a1 = v1 - a1;
    v0 = (i32(a1) < i32(a2));
    s1 = v1;
    if (v0 == 0) goto loc_80018F60;
    sw(a2, s0 + 0x4);
    a0 = s0;
    a1 = s2;
    P_ChangeSector();
    {
        const bool bJump = (v0 != s4);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80018FEC;
    }
    sw(s1, s0 + 0x4);
    goto loc_80018FC8;
loc_80018F60:
    sw(a1, s0 + 0x4);
    a0 = s0;
    a1 = s2;
    P_ChangeSector();
    if (v0 != s4) goto loc_80018FE8;
    {
        const bool bJump = (s2 == v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80018FEC;
    }
    sw(s1, s0 + 0x4);
loc_80018F84:
    a0 = s0;
    a1 = s2;
    P_ChangeSector();
    v0 = 1;                                             // Result = 00000001
    goto loc_80018FEC;
loc_80018F98:
    s1 = lw(s0 + 0x4);
    a1 += s1;
    v0 = (i32(a2) < i32(a1));
    a0 = s0;
    if (v0 == 0) goto loc_80018FDC;
    sw(a2, s0 + 0x4);
    a1 = s2;
    P_ChangeSector();
    {
        const bool bJump = (v0 != s3);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80018FEC;
    }
    sw(s1, s0 + 0x4);
loc_80018FC8:
    a0 = s0;
    a1 = s2;
    P_ChangeSector();
    v0 = 2;                                             // Result = 00000002
    goto loc_80018FEC;
loc_80018FDC:
    sw(a1, s0 + 0x4);
    a1 = s2;
    P_ChangeSector();
loc_80018FE8:
    v0 = 0;                                             // Result = 00000000
loc_80018FEC:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void T_MoveFloor() noexcept {
    sp -= 0x28;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(0, sp + 0x10);
    v0 = lw(s0 + 0x18);
    sw(v0, sp + 0x14);
    a0 = lw(s0 + 0x14);
    a1 = lw(s0 + 0x28);
    a2 = lw(s0 + 0x24);
    a3 = lw(s0 + 0x10);
    T_MovePlane();
    v1 = *gGameTic;
    v1 &= 3;
    s1 = v0;
    if (v1 != 0) goto loc_80019074;
    a0 = lw(s0 + 0x14);
    a1 = 0x15;                                          // Result = 00000015
    a0 += 0x38;
    S_StartSound();
loc_80019074:
    v0 = 2;                                             // Result = 00000002
    if (s1 != v0) goto loc_800190E8;
    v0 = lw(s0 + 0x14);
    sw(0, v0 + 0x50);
    v1 = lw(s0 + 0x18);
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v0 = -1;                                        // Result = FFFFFFFF
        if (bJump) goto loc_800190A8;
    }
    v1 = lw(s0 + 0xC);
    v0 = 0xA;                                           // Result = 0000000A
    goto loc_800190B4;
loc_800190A8:
    {
        const bool bJump = (v1 != v0);
        v0 = 6;                                         // Result = 00000006
        if (bJump) goto loc_800190E0;
    }
    v1 = lw(s0 + 0xC);
loc_800190B4:
    if (v1 != v0) goto loc_800190E0;
    v1 = lw(s0 + 0x14);
    v0 = lw(s0 + 0x1C);
    sw(v0, v1 + 0x14);
    v1 = lw(s0 + 0x14);
    v0 = lh(s0 + 0x20);
    sw(v0, v1 + 0x8);
loc_800190E0:
    a0 = s0;
    P_RemoveThinker();
loc_800190E8:
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

void EV_DoFloor() noexcept {
loc_80019100:
    sp -= 0x50;
    sw(fp, sp + 0x48);
    fp = a0;
    sw(s6, sp + 0x40);
    s6 = a1;
    sw(s4, sp + 0x38);
    s4 = -1;                                            // Result = FFFFFFFF
    sw(s5, sp + 0x3C);
    s5 = 0x30000;                                       // Result = 00030000
    sw(s7, sp + 0x44);
    s7 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x4C);
    sw(s3, sp + 0x34);
    sw(s2, sp + 0x30);
    sw(s1, sp + 0x2C);
    sw(s0, sp + 0x28);
    sw(0, sp + 0x10);
loc_80019144:
    a0 = fp;
loc_80019148:
    a1 = s4;
    P_FindSectorFromLineTag();
    s4 = v0;
    v0 = s4 << 1;
    if (i32(s4) < 0) goto loc_80019510;
    v0 += s4;
    v0 <<= 3;
    v0 -= s4;
    v1 = *gpSectors;
    v0 <<= 2;
    s2 = v0 + v1;
    v0 = lw(s2 + 0x50);
    t0 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_80019144;
    a1 = 0x2C;                                          // Result = 0000002C
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    sw(t0, sp + 0x10);
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    P_AddThinker();
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x6FF0;                                       // Result = T_MoveFloor (80019010)
    sw(s0, s2 + 0x50);
    sw(v0, s0 + 0x8);
    v0 = (s6 < 0xA);
    sw(s6, s0 + 0xC);
    sw(0, s0 + 0x10);
    if (v0 == 0) goto loc_80019144;
    v0 = s6 << 2;
    at = 0x80010000;                                    // Result = 80010000
    at += 0x3A8;                                        // Result = JumpTable_EV_DoFloor[0] (800103A8)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x800191FC: goto loc_800191FC;
        case 0x8001921C: goto loc_8001921C;
        case 0x8001923C: goto loc_8001923C;
        case 0x8001927C: goto loc_8001927C;
        case 0x800192C0: goto loc_800192C0;
        case 0x8001934C: goto loc_8001934C;
        case 0x80019444: goto loc_80019444;
        case 0x800192E0: goto loc_800192E0;
        case 0x80019304: goto loc_80019304;
        case 0x80019278: goto loc_80019278;
        default: jump_table_err(); break;
    }
loc_800191F0:
    a1 = s1;
    a2 = 1;                                             // Result = 00000001
    goto loc_800194D0;
loc_800191FC:
    a0 = s2;
    t0 = -1;                                            // Result = FFFFFFFF
    sw(t0, s0 + 0x18);
    sw(a0, s0 + 0x14);
    sw(s5, s0 + 0x28);
    P_FindHighestFloorSurrounding();
    sw(v0, s0 + 0x24);
    goto loc_80019144;
loc_8001921C:
    a0 = s2;
    t0 = -1;                                            // Result = FFFFFFFF
    sw(t0, s0 + 0x18);
    sw(a0, s0 + 0x14);
    sw(s5, s0 + 0x28);
    P_FindLowestFloorSurrounding();
    sw(v0, s0 + 0x24);
    goto loc_80019144;
loc_8001923C:
    a0 = s2;
    t0 = -1;                                            // Result = FFFFFFFF
    v0 = 0xC0000;                                       // Result = 000C0000
    sw(t0, s0 + 0x18);
    sw(s2, s0 + 0x14);
    sw(v0, s0 + 0x28);
    P_FindHighestFloorSurrounding();
    v1 = v0;
    sw(v1, s0 + 0x24);
    v0 = lw(s2);
    a0 = fp;
    if (v1 == v0) goto loc_80019148;
    v0 = 0x80000;                                       // Result = 00080000
    goto loc_800192F8;
loc_80019278:
    sw(s7, s0 + 0x10);
loc_8001927C:
    a0 = s2;
    sw(s7, s0 + 0x18);
    sw(s2, s0 + 0x14);
    sw(s5, s0 + 0x28);
    P_FindLowestCeilingSurrounding();
    sw(v0, s0 + 0x24);
    a0 = lw(s2 + 0x4);
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_800192AC;
    }
    sw(a0, s0 + 0x24);
loc_800192AC:
    a0 = fp;
    if (s6 != v0) goto loc_80019148;
    v0 = lw(s0 + 0x24);
    v1 = 0xFFF80000;                                    // Result = FFF80000
    goto loc_800192F8;
loc_800192C0:
    sw(s7, s0 + 0x18);
    sw(s2, s0 + 0x14);
    sw(s5, s0 + 0x28);
    a1 = lw(s2);
    a0 = s2;
    P_FindNextHighestFloor();
    sw(v0, s0 + 0x24);
    goto loc_80019144;
loc_800192E0:
    sw(s2, s0 + 0x14);
    v0 = lw(s0 + 0x14);
    sw(s7, s0 + 0x18);
    sw(s5, s0 + 0x28);
    v0 = lw(v0);
    v1 = 0x180000;                                      // Result = 00180000
loc_800192F8:
    v0 += v1;
    sw(v0, s0 + 0x24);
    goto loc_80019144;
loc_80019304:
    sw(s2, s0 + 0x14);
    v0 = lw(s0 + 0x14);
    sw(s7, s0 + 0x18);
    sw(s5, s0 + 0x28);
    v0 = lw(v0);
    v1 = 0x180000;                                      // Result = 00180000
    v0 += v1;
    sw(v0, s0 + 0x24);
    v0 = lw(fp + 0x38);
    v0 = lw(v0 + 0x8);
    sw(v0, s2 + 0x8);
    v0 = lw(fp + 0x38);
    v0 = lw(v0 + 0x14);
    sw(v0, s2 + 0x14);
    goto loc_80019144;
loc_8001934C:
    s3 = 0x7FFF0000;                                    // Result = 7FFF0000
    s3 |= 0xFFFF;                                       // Result = 7FFFFFFF
    sw(s7, s0 + 0x18);
    sw(s2, s0 + 0x14);
    sw(s5, s0 + 0x28);
    v0 = lw(s2 + 0x54);
    s1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80019428;
    a0 = s4;
loc_80019374:
    a1 = s1;
    twoSided();
    a0 = s4;
    if (v0 == 0) goto loc_80019414;
    a1 = s1;
    a2 = 0;                                             // Result = 00000000
    getSide();
    v0 = lw(v0 + 0xC);
    a0 = s4;
    if (i32(v0) < 0) goto loc_800193CC;
    v1 = *gpTextures;
    v0 <<= 5;
    v0 += v1;
    v0 = lh(v0 + 0x6);
    v1 = v0 << 16;
    v0 = (i32(v1) < i32(s3));
    a1 = s1;
    if (v0 == 0) goto loc_800193D0;
    s3 = v1;
loc_800193CC:
    a1 = s1;
loc_800193D0:
    a2 = 1;                                             // Result = 00000001
    getSide();
    v0 = lw(v0 + 0xC);
    {
        const bool bJump = (i32(v0) < 0);
        v0 <<= 5;
        if (bJump) goto loc_80019414;
    }
    v1 = *gpTextures;
    v0 += v1;
    v0 = lh(v0 + 0x6);
    v1 = v0 << 16;
    v0 = (i32(v1) < i32(s3));
    if (v0 == 0) goto loc_80019414;
    s3 = v1;
loc_80019414:
    v0 = lw(s2 + 0x54);
    s1++;
    v0 = (i32(s1) < i32(v0));
    a0 = s4;
    if (v0 != 0) goto loc_80019374;
loc_80019428:
    v0 = lw(s0 + 0x14);
    v0 = lw(v0);
    v0 += s3;
    sw(v0, s0 + 0x24);
    goto loc_80019144;
loc_80019444:
    a0 = s2;
    t0 = -1;                                            // Result = FFFFFFFF
    sw(t0, s0 + 0x18);
    sw(s2, s0 + 0x14);
    sw(s5, s0 + 0x28);
    P_FindLowestFloorSurrounding();
    sw(v0, s0 + 0x24);
    v0 = lhu(s2 + 0x8);
    sh(v0, s0 + 0x20);
    v0 = lw(s2 + 0x54);
    s1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80019144;
    a0 = s4;
loc_80019480:
    a1 = s1;
    twoSided();
    a0 = s4;
    if (v0 == 0) goto loc_800194F4;
    a1 = s1;
    a2 = 0;                                             // Result = 00000000
    getSide();
    a0 = 0xE9BD0000;                                    // Result = E9BD0000
    v0 = lw(v0 + 0x14);
    v1 = *gpSectors;
    a0 |= 0x37A7;                                       // Result = E9BD37A7
    v0 -= v1;
    mult(v0, a0);
    v0 = lo;
    v0 = u32(i32(v0) >> 2);
    a0 = s4;
    if (v0 == s4) goto loc_800191F0;
    a1 = s1;
    a2 = 0;                                             // Result = 00000000
loc_800194D0:
    getSector();
    s2 = v0;
    v0 = lhu(s2 + 0x8);
    sh(v0, s0 + 0x20);
    v0 = lw(s2 + 0x14);
    sw(v0, s0 + 0x1C);
    goto loc_80019144;
loc_800194F4:
    v0 = lw(s2 + 0x54);
    s1++;
    v0 = (i32(s1) < i32(v0));
    if (v0 != 0) goto loc_80019480;
    a0 = fp;
    goto loc_80019148;
loc_80019510:
    v0 = lw(sp + 0x10);
    ra = lw(sp + 0x4C);
    fp = lw(sp + 0x48);
    s7 = lw(sp + 0x44);
    s6 = lw(sp + 0x40);
    s5 = lw(sp + 0x3C);
    s4 = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x50;
    return;
}

void EV_BuildStairs() noexcept {
loc_80019548:
    sp -= 0x50;
    sw(s3, sp + 0x34);
    s3 = -1;                                            // Result = FFFFFFFF
    sw(s7, sp + 0x44);
    s7 = 0;                                             // Result = 00000000
    sw(s6, sp + 0x40);
    s6 = 0xE9BD0000;                                    // Result = E9BD0000
    s6 |= 0x37A7;                                       // Result = E9BD37A7
    sw(ra, sp + 0x4C);
    sw(fp, sp + 0x48);
    sw(s5, sp + 0x3C);
    sw(s4, sp + 0x38);
    sw(s2, sp + 0x30);
    sw(s1, sp + 0x2C);
    sw(s0, sp + 0x28);
    sw(a0, sp + 0x10);
    sw(a1, sp + 0x18);
loc_8001958C:
    a0 = lw(sp + 0x10);
    a1 = s3;
    P_FindSectorFromLineTag();
    s3 = v0;
    v0 = s3 << 1;
    if (i32(s3) < 0) goto loc_8001976C;
    v0 += s3;
    v0 <<= 3;
    v0 -= s3;
    v1 = *gpSectors;
    v0 <<= 2;
    s1 = v0 + v1;
    v0 = lw(s1 + 0x50);
    a1 = 0x2C;                                          // Result = 0000002C
    if (v0 != 0) goto loc_8001958C;
    s7 = 1;                                             // Result = 00000001
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    P_AddThinker();
    sw(s0, s1 + 0x50);
    t0 = 0x80020000;                                    // Result = 80020000
    t0 -= 0x6FF0;                                       // Result = T_MoveFloor (80019010)
    sw(t0, s0 + 0x8);
    sw(s7, s0 + 0x18);
    sw(s1, s0 + 0x14);
    t0 = lw(sp + 0x18);
    if (t0 == 0) goto loc_8001962C;
    if (t0 == s7) goto loc_8001963C;
    sw(s5, s0 + 0x28);
    goto loc_80019648;
loc_8001962C:
    s5 = 0x10000;                                       // Result = 00010000
    s5 |= 0x8000;                                       // Result = 00018000
    s4 = 0x80000;                                       // Result = 00080000
    goto loc_80019644;
loc_8001963C:
    s5 = 0x60000;                                       // Result = 00060000
    s4 = 0x100000;                                      // Result = 00100000
loc_80019644:
    sw(s5, s0 + 0x28);
loc_80019648:
    v0 = lw(s1);
    s2 = s4 + v0;
    sw(s2, s0 + 0x24);
    fp = lw(s1 + 0x8);
    a2 = 0;                                             // Result = 00000000
loc_80019660:
    v0 = lw(s1 + 0x54);
    a3 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_8001975C;
loc_80019670:
    v0 = lw(s1 + 0x58);
    v1 = a2 << 2;
    v1 += v0;
    v1 = lw(v1);
    v0 = lw(v1 + 0x10);
    v0 &= 4;
    if (v0 == 0) goto loc_80019748;
    a0 = lw(v1 + 0x38);
    a1 = *gpSectors;
    v0 = a0 - a1;
    mult(v0, s6);
    v0 = lo;
    v0 = u32(i32(v0) >> 2);
    if (s3 != v0) goto loc_80019748;
    a0 = lw(v1 + 0x3C);
    v0 = a0 - a1;
    mult(v0, s6);
    v1 = lw(a0 + 0x8);
    v0 = lo;
    a1 = u32(i32(v0) >> 2);
    if (v1 != fp) goto loc_80019748;
    v0 = lw(a0 + 0x50);
    s2 += s4;
    if (v0 != 0) goto loc_80019748;
    s1 = a0;
    s3 = a1;
    a1 = 0x2C;                                          // Result = 0000002C
    a2 = 4;                                             // Result = 00000004
    a0 = *gpMainMemZone;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    P_AddThinker();
    a3 = 1;                                             // Result = 00000001
    sw(s0, s1 + 0x50);
    t0 = 0x80020000;                                    // Result = 80020000
    t0 -= 0x6FF0;                                       // Result = T_MoveFloor (80019010)
    v0 = 1;                                             // Result = 00000001
    sw(t0, s0 + 0x8);
    sw(v0, s0 + 0x18);
    sw(s1, s0 + 0x14);
    sw(s5, s0 + 0x28);
    sw(s2, s0 + 0x24);
    goto loc_8001975C;
loc_80019748:
    v0 = lw(s1 + 0x54);
    a2++;
    v0 = (i32(a2) < i32(v0));
    if (v0 != 0) goto loc_80019670;
loc_8001975C:
    a2 = 0;                                             // Result = 00000000
    if (a3 != 0) goto loc_80019660;
    goto loc_8001958C;
loc_8001976C:
    v0 = s7;                                            // Result = 00000000
    ra = lw(sp + 0x4C);
    fp = lw(sp + 0x48);
    s7 = lw(sp + 0x44);
    s6 = lw(sp + 0x40);
    s5 = lw(sp + 0x3C);
    s4 = lw(sp + 0x38);
    s3 = lw(sp + 0x34);
    s2 = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x50;
    return;
}
