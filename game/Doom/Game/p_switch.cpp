#include "p_switch.h"

#include "PsxVm/PsxVm.h"

void P_InitSwitchList() noexcept {
loc_80027D84:
    sp -= 0x30;
    sw(s5, sp + 0x24);
    s5 = 0;                                             // Result = 00000000
    v0 = 0x80060000;                                    // Result = 80060000
    v0 += 0x7404;                                       // Result = AlphSwitchList_1_a[0] (80067404)
    sw(s4, sp + 0x20);
    s4 = v0 + 9;                                        // Result = AlphSwitchList_1_b[0] (8006740D)
    sw(s3, sp + 0x1C);
    s3 = v0;                                            // Result = AlphSwitchList_1_a[0] (80067404)
    sw(s1, sp + 0x14);
    s1 = 0x80090000;                                    // Result = 80090000
    s1 += 0x75FC;                                       // Result = gSwitchList[0] (800975FC)
    sw(ra, sp + 0x28);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
loc_80027DC0:
    a0 = s3;
    R_TextureNumForName();
    a0 = s4;
    s2 = v0;
    R_TextureNumForName();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7ED8);                               // Load from: gpTextures (80078128)
    v1 = s2 << 5;
    v1 += a0;
    v1 = lhu(v1 + 0xA);
    s0 = v0;
    if (v1 == 0) goto loc_80027E14;
    v0 = s0 << 5;
    a0 += v0;
    v0 = lhu(a0 + 0xA);
    if (v0 != 0) goto loc_80027E34;
    I_CacheTex();
loc_80027E14:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7ED8);                               // Load from: gpTextures (80078128)
    v1 = s0 << 5;
    v1 += v0;
    v0 = lhu(v1 + 0xA);
    if (v0 == 0) goto loc_80027E5C;
loc_80027E34:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7ED8);                               // Load from: gpTextures (80078128)
    v1 = s2 << 5;
    a0 = v1 + v0;
    v0 = lhu(a0 + 0xA);
    if (v0 != 0) goto loc_80027E5C;
    I_CacheTex();
loc_80027E5C:
    sw(s2, s1);
    s1 += 4;
    sw(s0, s1);
    s1 += 4;
    s4 += 0x12;
    s5++;
    v0 = (s5 < 0x31);
    s3 += 0x12;
    if (v0 != 0) goto loc_80027DC0;
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

void P_StartButton() noexcept {
    t1 = 0;                                             // Result = 00000000
    t0 = 0x80090000;                                    // Result = 80090000
    t0 += 0x77B8;                                       // Result = gButtonList_1[3] (800977B8)
    v1 = 0;                                             // Result = 00000000
loc_80027EB8:
    v0 = lw(t0);
    if (v0 != 0) goto loc_80027F20;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    at += v1;
    sw(a0, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B0;                                       // Result = gButtonList_1[1] (800977B0)
    at += v1;
    sw(a1, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B4;                                       // Result = gButtonList_1[2] (800977B4)
    at += v1;
    sw(a2, at);
    sw(a3, t0);
    v0 = lw(a0 + 0x38);
    v0 += 0x38;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77BC;                                       // Result = gButtonList_1[4] (800977BC)
    at += v1;
    sw(v0, at);
    goto loc_80027F34;
loc_80027F20:
    t0 += 0x14;
    t1++;
    v0 = (i32(t1) < 0x10);
    v1 += 0x14;
    if (v0 != 0) goto loc_80027EB8;
loc_80027F34:
    return;
}

void P_ChangeSwitchTexture() noexcept {
loc_80027F3C:
    sp -= 0x28;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s3, sp + 0x1C);
    s3 = a1;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s2, sp + 0x18);
    sw(s0, sp + 0x10);
    if (s3 != 0) goto loc_80027F68;
    sw(0, s1 + 0x14);
loc_80027F68:
    v1 = lw(s1 + 0x1C);
    a0 = 0xB;                                           // Result = 0000000B
    v0 = v1 << 1;
    v0 += v1;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7EA0);                               // Load from: gpSides (80077EA0)
    v0 <<= 3;
    v0 += v1;
    t0 = lw(v0 + 0x8);
    a3 = lw(v0 + 0x10);
    v1 = lw(s1 + 0x14);
    a2 = lw(v0 + 0xC);
    a1 = 0x16;                                          // Result = 00000016
    if (v1 != a0) goto loc_80027FA4;
    a1 = 0x17;                                          // Result = 00000017
loc_80027FA4:
    s0 = 0;                                             // Result = 00000000
    s2 = 0x80090000;                                    // Result = 80090000
    s2 += 0x75FC;                                       // Result = gSwitchList[0] (800975FC)
    s4 = s2;                                            // Result = gSwitchList[0] (800975FC)
loc_80027FB4:
    v0 = lw(s2);
    if (v0 != t0) goto loc_80028080;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x77BC);                               // Load from: gButtonList_1[4] (800977BC)
    S_StartSound();
    v0 = lw(s1 + 0x1C);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7EA0);                               // Load from: gpSides (80077EA0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = s0 ^ 1;
    v0 <<= 2;
    v0 += s4;
    v0 = lw(v0);
    v1 += a0;
    sw(v0, v1 + 0x8);
    if (s3 == 0) goto loc_80028208;
    a2 = lw(s2);
    a3 = 0xF;                                           // Result = 0000000F
    a0 = 0;                                             // Result = 00000000
    a1 = 0x80090000;                                    // Result = 80090000
    a1 += 0x77B8;                                       // Result = gButtonList_1[3] (800977B8)
    v1 = 0;                                             // Result = 00000000
loc_80028020:
    v0 = lw(a1);
    a0++;
    if (v0 != 0) goto loc_80028068;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    at += v1;
    sw(s1, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B0;                                       // Result = gButtonList_1[1] (800977B0)
    at += v1;
    sw(0, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B4;                                       // Result = gButtonList_1[2] (800977B4)
    at += v1;
    sw(a2, at);
    sw(a3, a1);
    goto loc_800281BC;
loc_80028068:
    a1 += 0x14;
    v0 = (i32(a0) < 0x10);
    v1 += 0x14;
    if (v0 != 0) goto loc_80028020;
    goto loc_80028208;
loc_80028080:
    if (v0 != a3) goto loc_80028110;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x77BC);                               // Load from: gButtonList_1[4] (800977BC)
    S_StartSound();
    v0 = lw(s1 + 0x1C);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7EA0);                               // Load from: gpSides (80077EA0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = s0 ^ 1;
    v0 <<= 2;
    v0 += s4;
    v0 = lw(v0);
    v1 += a0;
    sw(v0, v1 + 0x10);
    if (s3 == 0) goto loc_80028208;
    a3 = 1;                                             // Result = 00000001
    a2 = lw(s2);
    t0 = 0xF;                                           // Result = 0000000F
    a0 = 0;                                             // Result = 00000000
    a1 = 0x80090000;                                    // Result = 80090000
    a1 += 0x77B8;                                       // Result = gButtonList_1[3] (800977B8)
    v1 = 0;                                             // Result = 00000000
loc_800280E8:
    v0 = lw(a1);
    a0++;
    if (v0 == 0) goto loc_80028188;
    a1 += 0x14;
    v0 = (i32(a0) < 0x10);
    v1 += 0x14;
    if (v0 != 0) goto loc_800280E8;
    goto loc_80028208;
loc_80028110:
    if (v0 != a2) goto loc_800281F8;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x77BC);                               // Load from: gButtonList_1[4] (800977BC)
    S_StartSound();
    v0 = lw(s1 + 0x1C);
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x7EA0);                               // Load from: gpSides (80077EA0)
    v1 = v0 << 1;
    v1 += v0;
    v1 <<= 3;
    v0 = s0 ^ 1;
    v0 <<= 2;
    v0 += s4;
    v0 = lw(v0);
    v1 += a0;
    sw(v0, v1 + 0xC);
    if (s3 == 0) goto loc_80028208;
    a3 = 2;                                             // Result = 00000002
    a2 = lw(s2);
    t0 = 0xF;                                           // Result = 0000000F
    a0 = 0;                                             // Result = 00000000
    a1 = 0x80090000;                                    // Result = 80090000
    a1 += 0x77B8;                                       // Result = gButtonList_1[3] (800977B8)
    v1 = 0;                                             // Result = 00000000
loc_80028178:
    v0 = lw(a1);
    a0++;
    if (v0 != 0) goto loc_800281E0;
loc_80028188:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    at += v1;
    sw(s1, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B0;                                       // Result = gButtonList_1[1] (800977B0)
    at += v1;
    sw(a3, at);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B4;                                       // Result = gButtonList_1[2] (800977B4)
    at += v1;
    sw(a2, at);
    sw(t0, a1);
loc_800281BC:
    v0 = lw(s1 + 0x38);
    v0 += 0x38;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77BC;                                       // Result = gButtonList_1[4] (800977BC)
    at += v1;
    sw(v0, at);
    goto loc_80028208;
loc_800281E0:
    a1 += 0x14;
    v0 = (i32(a0) < 0x10);
    v1 += 0x14;
    if (v0 != 0) goto loc_80028178;
    goto loc_80028208;
loc_800281F8:
    s0++;
    v0 = (s0 < 0x62);
    s2 += 4;
    if (v0 != 0) goto loc_80027FB4;
loc_80028208:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_UseSpecialLine() noexcept {
loc_8002822C:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    v0 = lw(s1 + 0x80);
    s0 = a1;
    if (v0 != 0) goto loc_80028274;
    v0 = lw(s0 + 0x10);
    v0 &= 0x20;
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80028808;
    }
    v1 = lw(s0 + 0x14);
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (v1 != v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80028808;
    }
loc_80028274:
    v0 = lw(s0 + 0x14);
    v1 = v0 - 1;
    v0 = (v1 < 0x8B);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80028804;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0xE34;                                        // Result = JumpTable_P_UseSpecialLine[0] (80010E34)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x800282BC: goto loc_800282BC;
        case 0x80028804: goto loc_80028804;
        case 0x800285C4: goto loc_800285C4;
        case 0x800285E0: goto loc_800285E0;
        case 0x800285F8: goto loc_800285F8;
        case 0x8002860C: goto loc_8002860C;
        case 0x8002862C: goto loc_8002862C;
        case 0x8002864C: goto loc_8002864C;
        case 0x80028668: goto loc_80028668;
        case 0x80028688: goto loc_80028688;
        case 0x800286A8: goto loc_800286A8;
        case 0x800282A8: goto loc_800282A8;
        case 0x800286C4: goto loc_800286C4;
        case 0x800286E0: goto loc_800286E0;
        case 0x8002842C: goto loc_8002842C;
        case 0x80028448: goto loc_80028448;
        case 0x80028464: goto loc_80028464;
        case 0x80028718: goto loc_80028718;
        case 0x80028734: goto loc_80028734;
        case 0x80028750: goto loc_80028750;
        case 0x80028768: goto loc_80028768;
        case 0x80028480: goto loc_80028480;
        case 0x8002849C: goto loc_8002849C;
        case 0x800284B8: goto loc_800284B8;
        case 0x800284D8: goto loc_800284D8;
        case 0x800284F4: goto loc_800284F4;
        case 0x80028550: goto loc_80028550;
        case 0x80028510: goto loc_80028510;
        case 0x80028530: goto loc_80028530;
        case 0x8002856C: goto loc_8002856C;
        case 0x8002858C: goto loc_8002858C;
        case 0x800285A8: goto loc_800285A8;
        case 0x800286FC: goto loc_800286FC;
        case 0x800282D0: goto loc_800282D0;
        case 0x80028784: goto loc_80028784;
        case 0x800287A0: goto loc_800287A0;
        case 0x800287BC: goto loc_800287BC;
        case 0x80028328: goto loc_80028328;
        case 0x80028344: goto loc_80028344;
        case 0x80028360: goto loc_80028360;
        case 0x8002837C: goto loc_8002837C;
        case 0x80028398: goto loc_80028398;
        case 0x800283B4: goto loc_800283B4;
        case 0x800283D0: goto loc_800283D0;
        case 0x800283F0: goto loc_800283F0;
        case 0x80028410: goto loc_80028410;
        case 0x800282FC: goto loc_800282FC;
        case 0x800287D8: goto loc_800287D8;
        case 0x800287E4: goto loc_800287E4;
        default: jump_table_err(); break;
    }
loc_800282A8:
    a0 = s0;
    a1 = s1;
    EV_DoLockedDoor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
loc_800282BC:
    a0 = s0;
    a1 = s1;
    EV_VerticalDoor();
    v0 = 1;                                             // Result = 00000001
    goto loc_80028808;
loc_800282D0:
    a0 = s0;
    a1 = s1;
    EV_DoLockedDoor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 6;                                             // Result = 00000006
    EV_DoDoor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800282FC:
    a0 = s0;
    a1 = s1;
    EV_DoLockedDoor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 6;                                             // Result = 00000006
    EV_DoDoor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028328:
    a0 = s0;
    a1 = 5;                                             // Result = 00000005
    EV_DoDoor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028344:
    a0 = s0;
    a1 = 6;                                             // Result = 00000006
    EV_DoDoor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028360:
    a0 = s0;
    a1 = 7;                                             // Result = 00000007
    EV_DoDoor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_8002837C:
    a0 = s0;
    a1 = 5;                                             // Result = 00000005
    EV_DoDoor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028398:
    a0 = s0;
    a1 = 6;                                             // Result = 00000006
    EV_DoDoor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800283B4:
    a0 = s0;
    a1 = 7;                                             // Result = 00000007
    EV_DoDoor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800283D0:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
    a2 = 0;                                             // Result = 00000000
    EV_DoPlat();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800283F0:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
    a2 = 0;                                             // Result = 00000000
    EV_DoPlat();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028410:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    EV_BuildStairs();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_8002842C:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    EV_DoDoor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028448:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_DoCeiling();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028464:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028480:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_8002849C:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    EV_DoDoor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800284B8:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    a2 = 1;                                             // Result = 00000001
    EV_DoPlat();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800284D8:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_DoDoor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800284F4:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028510:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    a2 = 0x18;                                          // Result = 00000018
    EV_DoPlat();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028530:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    a2 = 0x20;                                          // Result = 00000020
    EV_DoPlat();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_80028550:
    a0 = s0;
    a1 = 9;                                             // Result = 00000009
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_8002856C:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    a2 = 0;                                             // Result = 00000000
    EV_DoPlat();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_8002858C:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800285A8:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    EV_DoFloor();
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80028808;
    }
    a0 = s0;
    goto loc_800287F8;
loc_800285C4:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_BuildStairs();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800285E0:
    a0 = s0;
    EV_DoDonut();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800285F8:
    G_ExitLevel();
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_8002860C:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    a2 = 0x20;                                          // Result = 00000020
    EV_DoPlat();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_8002862C:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    a2 = 0x18;                                          // Result = 00000018
    EV_DoPlat();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_8002864C:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028668:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    a2 = 0;                                             // Result = 00000000
    EV_DoPlat();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028688:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    a2 = 0;                                             // Result = 00000000
    EV_DoPlat();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800286A8:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800286C4:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_DoDoor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800286E0:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_DoCeiling();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800286FC:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028718:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    EV_DoCeiling();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028734:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    EV_DoDoor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028750:
    a0 = lw(s0 + 0x18);
    G_SecretExitLevel();
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028768:
    a0 = s0;
    a1 = 9;                                             // Result = 00000009
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_80028784:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800287A0:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_DoFloor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800287BC:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    EV_DoDoor();
    a0 = s0;
    if (v0 == 0) goto loc_80028804;
    a1 = 0;                                             // Result = 00000000
    goto loc_800287FC;
loc_800287D8:
    a0 = s0;
    a1 = 0xFF;                                          // Result = 000000FF
    goto loc_800287EC;
loc_800287E4:
    a0 = s0;
    a1 = 0x23;                                          // Result = 00000023
loc_800287EC:
    EV_LightTurnOn();
    a0 = s0;
loc_800287F8:
    a1 = 1;                                             // Result = 00000001
loc_800287FC:
    P_ChangeSwitchTexture();
loc_80028804:
    v0 = 1;                                             // Result = 00000001
loc_80028808:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
