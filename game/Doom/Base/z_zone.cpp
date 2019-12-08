#include "z_zone.h"

#include "PsxVm/PsxVm.h"

void Z_Init() noexcept {
loc_80032144:
    sp -= 0x18;
    v1 = 0x1FFF0000;                                    // Result = 1FFF0000
    v1 |= 0xFFFF;                                       // Result = 1FFFFFFF
    a0 = 0x80070000;                                    // Result = 80070000
    a0 = lw(a0 + 0x4C00);                               // Load from: I_ZoneBase (80074C00)
    a1 = lw(gp + 0x618);                                // Load from: StackEndAddr (80077BF8)
    v0 = lw(gp + 0x61C);                                // Load from: StackSize (80077BFC)
    a2 = -4;                                            // Result = FFFFFFFC
    sw(ra, sp + 0x10);
    a0 += 3;
    a0 &= a2;
    a1 -= v0;
    v1 &= a0;
    a1 -= v1;
    a1 += 3;
    a1 &= a2;
    Z_InitZone();
    sw(v0, gp + 0xBB8);                                 // Store to: gpMainMemZone (80078198)
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Z_InitZone() noexcept {
loc_8003219C:
    v0 = a0;
    v1 = v0 + 8;
    sw(a1, v0);
    a1 -= 8;
    sw(v1, v0 + 0x4);
    v1 = 0x1D4A;                                        // Result = 00001D4A
    sw(a1, v0 + 0x8);
    sw(0, v0 + 0xC);
    sh(0, v0 + 0x10);
    sh(v1, v0 + 0x12);
    sw(0, v0 + 0x18);
    sw(0, v0 + 0x1C);
    return;
}

void Z_Malloc2() noexcept {
loc_800321D0:
    sp -= 0x30;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s6, sp + 0x28);
    s6 = a2;
    sw(s4, sp + 0x20);
    s4 = a3;
    a1 += 0x1B;
    v0 = -4;                                            // Result = FFFFFFFC
    sw(ra, sp + 0x2C);
    sw(s5, sp + 0x24);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    s1 = lw(s3 + 0x4);
    s2 = a1 & v0;
    v1 = lw(s1 + 0x4);
    s5 = s1;
    goto loc_800322DC;
loc_8003221C:
    s0 = s1;
    if (v1 != 0) goto loc_80032228;
    s0 = lw(s1 + 0x10);
loc_80032228:
    if (s0 == 0) goto loc_80032268;
    v0 = lw(s0 + 0x4);
    if (v0 == 0) goto loc_8003229C;
    v0 = lh(s0 + 0x8);
    v0 = (i32(v0) < 0x10);
    a0 = s3;
    if (v0 == 0) goto loc_80032294;
    s1 = lw(s0 + 0x10);
    if (s1 != 0) goto loc_8003226C;
loc_80032268:
    s1 = s3 + 8;
loc_8003226C:
    if (s1 != s5) goto loc_800322D8;
    Z_DumpHeap();
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x13DC;                                       // Result = STR_Z_Malloc_AllocFailed_Err[0] (800113DC)
    a1 = s2;
    I_Error();
    goto loc_800322D8;
loc_80032294:
    a1 = s0 + 0x18;
    Z_Free2();
loc_8003229C:
    if (s1 == s0) goto loc_800322D8;
    v0 = lw(s1);
    v1 = lw(s0);
    v0 += v1;
    sw(v0, s1);
    v0 = lw(s0 + 0x10);
    sw(v0, s1 + 0x10);
    v1 = lw(s0 + 0x10);
    if (v1 == 0) goto loc_800322D8;
    sw(s1, v1 + 0x14);
loc_800322D8:
    v1 = lw(s1 + 0x4);
loc_800322DC:
    s0 = s1;
    if (v1 != 0) goto loc_80032228;
    v0 = lw(s1);
    v0 = (i32(v0) < i32(s2));
    if (v0 != 0) goto loc_8003221C;
    v0 = lw(s1);
    a0 = v0 - s2;
    v0 = (i32(a0) < 0x41);
    {
        const bool bJump = (v0 != 0)
        v0 = s1 + s2;
        if (bJump) goto loc_80032340;
    }
    sw(s1, v0 + 0x14);
    v1 = lw(s1 + 0x10);
    sw(v1, v0 + 0x10);
    if (v1 == 0) goto loc_8003232C;
    sw(v0, v1 + 0x14);
loc_8003232C:
    sw(v0, s1 + 0x10);
    sw(s2, s1);
    sw(a0, v0);
    sw(0, v0 + 0x4);
    sh(0, v0 + 0x8);
loc_80032340:
    v0 = s1 + 0x18;
    if (s4 == 0) goto loc_80032354;
    sw(s4, s1 + 0x4);
    sw(v0, s4);
    goto loc_80032378;
loc_80032354:
    v0 = (i32(s6) < 0x10);
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80032374;
    }
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1400;                                       // Result = STR_Z_Malloc_NoBlockOwner_Err[0] (80011400)
    I_Error();
    v0 = 1;                                             // Result = 00000001
loc_80032374:
    sw(v0, s1 + 0x4);
loc_80032378:
    v1 = lw(s1 + 0x10);
    v0 = 0x1D4A;                                        // Result = 00001D4A
    sh(s6, s1 + 0x8);
    sh(v0, s1 + 0xA);
    sw(v1, s3 + 0x4);
    if (v1 != 0) goto loc_80032398;
    v0 = s3 + 8;
    sw(v0, s3 + 0x4);
loc_80032398:
    v0 = s1 + 0x18;
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

void Z_Malloc2_b() noexcept {
loc_800323C8:
    sp -= 0x30;
    sw(s3, sp + 0x1C);
    s3 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s5, sp + 0x24);
    s5 = a2;
    sw(s4, sp + 0x20);
    s4 = a3;
    sw(ra, sp + 0x28);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lw(s3 + 0x18);
    s1 = s3 + 8;
    if (v0 == 0) goto loc_80032420;
loc_80032408:
    s1 = lw(s1 + 0x10);
    v0 = lw(s1 + 0x10);
    if (v0 != 0) goto loc_80032408;
loc_80032420:
    a0 = lw(s1 + 0x4);
    v1 = s2 + 0x1B;
    v0 = -4;                                            // Result = FFFFFFFC
    s2 = v1 & v0;
    goto loc_800324F8;
loc_80032434:
    if (a0 == 0) goto loc_80032444;
loc_8003243C:
    s0 = s1;
    goto loc_80032464;
loc_80032444:
    s0 = lw(s1 + 0x14);
    if (s0 != 0) goto loc_80032464;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x13DC;                                       // Result = STR_Z_Malloc_AllocFailed_Err[0] (800113DC)
    a1 = s2;
    I_Error();
loc_80032464:
    v0 = lw(s0 + 0x4);
    if (v0 == 0) goto loc_800324B8;
    v0 = lh(s0 + 0x8);
    v0 = (i32(v0) < 0x10);
    a0 = s3;
    if (v0 == 0) goto loc_800324B0;
    s1 = lw(s0 + 0x14);
    if (s1 != 0) goto loc_800324F4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x13DC;                                       // Result = STR_Z_Malloc_AllocFailed_Err[0] (800113DC)
    a1 = s2;
    I_Error();
    goto loc_800324F4;
loc_800324B0:
    a1 = s0 + 0x18;
    Z_Free2();
loc_800324B8:
    if (s1 == s0) goto loc_800324F4;
    v0 = lw(s0);
    v1 = lw(s1);
    v0 += v1;
    sw(v0, s0);
    v0 = lw(s1 + 0x10);
    sw(v0, s0 + 0x10);
    v0 = lw(s1 + 0x10);
    s1 = s0;
    if (v0 == 0) goto loc_800324F4;
    sw(s0, v0 + 0x14);
loc_800324F4:
    a0 = lw(s1 + 0x4);
loc_800324F8:
    if (a0 != 0) goto loc_8003243C;
    v0 = lw(s1);
    v0 = (i32(v0) < i32(s2));
    if (v0 != 0) goto loc_80032434;
    v0 = lw(s1);
    a0 = v0 - s2;
    v0 = (i32(a0) < 0x41);
    v1 = s1;
    if (v0 != 0) goto loc_80032560;
    s1 += a0;
    sw(v1, s1 + 0x14);
    v0 = lw(v1 + 0x10);
    sw(v0, s1 + 0x10);
    if (v0 == 0) goto loc_8003254C;
    sw(s1, v0 + 0x14);
loc_8003254C:
    sw(s1, v1 + 0x10);
    sw(s2, s1);
    sw(a0, v1);
    sw(0, v1 + 0x4);
    sh(0, v1 + 0x8);
loc_80032560:
    v0 = s1 + 0x18;
    if (s4 == 0) goto loc_80032574;
    sw(s4, s1 + 0x4);
    sw(v0, s4);
    goto loc_80032598;
loc_80032574:
    v0 = (i32(s5) < 0x10);
    {
        const bool bJump = (v0 != 0)
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80032594;
    }
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1400;                                       // Result = STR_Z_Malloc_NoBlockOwner_Err[0] (80011400)
    I_Error();
    v0 = 1;                                             // Result = 00000001
loc_80032594:
    sw(v0, s1 + 0x4);
loc_80032598:
    v0 = 0x1D4A;                                        // Result = 00001D4A
    sh(v0, s1 + 0xA);
    v0 = s3 + 8;
    sh(s5, s1 + 0x8);
    sw(v0, s3 + 0x4);
    v0 = s1 + 0x18;
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

void Z_Free2() noexcept {
loc_800325D8:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a1;
    sw(ra, sp + 0x14);
    v1 = lh(s0 - 0xE);
    v0 = 0x1D4A;                                        // Result = 00001D4A
    if (v1 == v0) goto loc_80032608;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1434;                                       // Result = STR_Z_Free_PtrNoZoneId_Err[0] (80011434)
    I_Error();
loc_80032608:
    v1 = lw(s0 - 0x14);
    v0 = (v1 < 0x101);
    if (v0 != 0) goto loc_80032620;
    sw(0, v1);
loc_80032620:
    sw(0, s0 - 0x14);
    sh(0, s0 - 0x10);
    sh(0, s0 - 0xE);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void Z_FreeTags() noexcept {
loc_80032640:
    sp -= 0x28;
    sw(s2, sp + 0x18);
    s2 = a0;
    sw(s3, sp + 0x1C);
    s3 = a1;
    sw(s0, sp + 0x10);
    s0 = s2 + 8;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s1, sp + 0x14);
    if (s0 == 0) goto loc_800326E8;
    s4 = 0x1D4A;                                        // Result = 00001D4A
loc_80032670:
    v0 = lw(s0 + 0x4);
    s1 = lw(s0 + 0x10);
    if (v0 == 0) goto loc_800326D8;
    v0 = lh(s0 + 0x8);
    v0 &= s3;
    if (v0 == 0) goto loc_800326D8;
    v0 = lh(s0 + 0xA);
    if (v0 == s4) goto loc_800326B4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1434;                                       // Result = STR_Z_Free_PtrNoZoneId_Err[0] (80011434)
    I_Error();
loc_800326B4:
    v1 = lw(s0 + 0x4);
    v0 = (v1 < 0x101);
    if (v0 != 0) goto loc_800326CC;
    sw(0, v1);
loc_800326CC:
    sw(0, s0 + 0x4);
    sh(0, s0 + 0x8);
    sh(0, s0 + 0xA);
loc_800326D8:
    s0 = s1;
    if (s0 != 0) goto loc_80032670;
    s0 = s2 + 8;
loc_800326E8:
    v0 = s2 + 8;
    if (s0 == 0) goto loc_80032748;
loc_800326F0:
    v0 = lw(s0 + 0x4);
    s1 = lw(s0 + 0x10);
    if (v0 != 0) goto loc_8003273C;
    if (s1 == 0) goto loc_8003273C;
    v0 = lw(s1 + 0x4);
    if (v0 != 0) goto loc_8003273C;
    v0 = lw(s0);
    v1 = lw(s1);
    v0 += v1;
    sw(v0, s0);
    v0 = lw(s1 + 0x10);
    s1 = s0;
    sw(v0, s0 + 0x10);
    sw(s1, v0 + 0x14);
loc_8003273C:
    s0 = s1;
    v0 = s2 + 8;
    if (s0 != 0) goto loc_800326F0;
loc_80032748:
    sw(v0, s2 + 0x4);
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void Z_CheckHeap() noexcept {
loc_80032770:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s0, sp + 0x10);
    s0 = s1 + 8;
    sw(ra, sp + 0x18);
    if (s0 == 0) goto loc_80032820;
loc_8003278C:
    v1 = lw(s0 + 0x10);
    if (v1 != 0) goto loc_800327C4;
    v0 = lw(s0);
    v1 = lw(s1);
    v0 += s0;
    v0 -= s1;
    if (v0 == v1) goto loc_80032810;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x145C;                                       // Result = STR_Z_CheckHeap_ZoneSizeChanged_Err[0] (8001145C)
    goto loc_80032808;
loc_800327C4:
    v0 = lw(s0);
    v0 += s0;
    if (v0 == v1) goto loc_800327E8;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x147C;                                       // Result = STR_Z_CheckHeap_BlockNotTouching_Err[0] (8001147C)
    I_Error();
loc_800327E8:
    v0 = lw(s0 + 0x10);
    v0 = lw(v0 + 0x14);
    if (v0 == s0) goto loc_80032810;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x14B4;                                       // Result = STR_Z_CheckHeap_BadBlockBackLink_Err[0] (800114B4)
loc_80032808:
    I_Error();
loc_80032810:
    s0 = lw(s0 + 0x10);
    if (s0 != 0) goto loc_8003278C;
loc_80032820:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void Z_ChangeTag() noexcept {
    sp -= 0x20;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(s1, sp + 0x14);
    s1 = a1;
    sw(s2, sp + 0x18);
    sw(ra, sp + 0x1C);
    v1 = lh(s0 - 0xE);
    v0 = 0x1D4A;                                        // Result = 00001D4A
    s2 = s0 - 0x18;
    if (v1 == v0) goto loc_80032874;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x14EC;                                       // Result = STR_Z_ChangeTag_PtrNoZoneId_Err[0] (800114EC)
    I_Error();
loc_80032874:
    v0 = (i32(s1) < 0x10);
    if (v0 != 0) goto loc_800328A4;
    v0 = lw(s0 - 0x14);
    v0 = (v0 < 0x100);
    if (v0 == 0) goto loc_800328A4;
    a0 = 0x80010000;                                    // Result = 80010000
    a0 += 0x1518;                                       // Result = STR_Z_ChangeTag_NoBlockOwner_Err[0] (80011518)
    I_Error();
loc_800328A4:
    sh(s1, s2 + 0x8);
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void Z_FreeMemory() noexcept {
loc_800328C4:
    a0 += 8;
    v1 = 0;                                             // Result = 00000000
    if (a0 == 0) goto loc_800328FC;
loc_800328D0:
    v0 = lw(a0 + 0x4);
    if (v0 != 0) goto loc_800328EC;
    v0 = lw(a0);
    v1 += v0;
loc_800328EC:
    a0 = lw(a0 + 0x10);
    if (a0 != 0) goto loc_800328D0;
loc_800328FC:
    v0 = v1;                                            // Result = 00000000
    return;
}

void Z_DumpHeap() noexcept {
loc_80032904:
    return;
}
