#include "p_telept.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_local.h"
#include "p_inter.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_setup.h"
#include "PsxVm/PsxVm.h"

void P_Telefrag() noexcept {
    sp -= 0x28;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    sw(s3, sp + 0x1C);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    sw(ra, sp + 0x20);
    sw(s0, sp + 0x10);
    s0 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    s3 = a2;
    if (s0 == v0) goto loc_800288F8;
loc_8002885C:
    v0 = lw(s0 + 0x64);
    v0 &= 4;
    if (v0 == 0) goto loc_800288E4;
    v1 = lw(s0 + 0x40);
    v0 = lw(s0);
    a0 = lw(s1 + 0x40);
    a1 = v0 - s2;
    v1 += a0;
    v0 = 0x40000;                                       // Result = 00040000
    v1 += v0;
    a0 = -v1;
    v0 = (i32(a1) < i32(a0));
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(v1) < i32(a1));
        if (bJump) goto loc_800288E4;
    }
    if (v0 != 0) goto loc_800288E4;
    v0 = lw(s0 + 0x4);
    a1 = v0 - s3;
    v0 = (i32(a1) < i32(a0));
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(v1) < i32(a1));
        if (bJump) goto loc_800288E4;
    }
    a0 = s0;
    if (v0 != 0) goto loc_800288E4;
    a1 = s1;
    a2 = s1;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    v0 = lw(s0 + 0x64);
    v1 = -7;                                            // Result = FFFFFFF9
    v0 &= v1;
    sw(v0, s0 + 0x64);
loc_800288E4:
    s0 = lw(s0 + 0x14);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    if (s0 != v0) goto loc_8002885C;
loc_800288F8:
    ra = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void EV_Teleport() noexcept {
loc_80028918:
    sp -= 0x40;
    sw(s0, sp + 0x18);
    s0 = a0;
    sw(s1, sp + 0x1C);
    s1 = a1;
    sw(ra, sp + 0x3C);
    sw(fp, sp + 0x38);
    sw(s7, sp + 0x34);
    sw(s6, sp + 0x30);
    sw(s5, sp + 0x2C);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    a0 = lw(s1);
    a1 = lw(s1 + 0x4);
    a2 = s0;
    v0 = P_PointOnLineSide(a0, a1, *vmAddrToPtr<line_t>(a2));
    a0 = (v0 < 1);
    v0 = lw(s1 + 0x64);
    v1 = 0x10000;                                       // Result = 00010000
    v0 &= v1;
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80028C04;
    }
    v0 = 1;                                             // Result = 00000001
    {
        const bool bJump = (a0 == v0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80028C04;
    }
    v0 = *gNumSectors;
    a2 = lw(s0 + 0x18);
    a1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80028C00;
    t0 = 0x800B0000;                                    // Result = 800B0000
    t0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    s5 = t0 - 0x14;                                     // Result = gMObjHead[0] (800A8E90)
    a3 = 0;                                             // Result = 00000000
loc_800289A4:
    v0 = *gpSectors;
    v0 += a3;
    v0 = lw(v0 + 0x18);
    if (v0 != a2) goto loc_80028BE8;
    s2 = lw(t0);                                        // Load from: gMObjHead[5] (800A8EA4)
    if (s2 == s5) goto loc_80028BE8;
loc_800289D4:
    v1 = lw(s2 + 0x54);
    v0 = 0x1F;                                          // Result = 0000001F
    a0 = 0xE9BD0000;                                    // Result = E9BD0000
    if (v1 != v0) goto loc_80028BD8;
    v0 = lw(s2 + 0xC);
    v1 = *gpSectors;
    v0 = lw(v0);
    a0 |= 0x37A7;                                       // Result = E9BD37A7
    v0 -= v1;
    mult(v0, a0);
    v0 = lo;
    v0 = u32(i32(v0) >> 2);
    if (v0 != a1) goto loc_80028BD8;
    v0 = lw(s1 + 0x64);
    s7 = lw(s1);
    fp = lw(s1 + 0x4);
    s6 = lw(s1 + 0x8);
    v0 |= 0x8000;
    sw(v0, s1 + 0x64);
    v0 = lw(s1 + 0x80);
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7F40);                                 // Store to: gNumCrossCheckLines (800780C0)
    if (v0 == 0) goto loc_80028AEC;
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 = lw(s0 - 0x715C);                               // Load from: gMObjHead[5] (800A8EA4)
    s4 = lw(s2);
    s3 = lw(s2 + 0x4);
    if (s0 == s5) goto loc_80028AEC;
loc_80028A54:
    v0 = lw(s0 + 0x64);
    v0 &= 4;
    if (v0 == 0) goto loc_80028ADC;
    a0 = lw(s0 + 0x40);
    v0 = lw(s0);
    v1 = lw(s1 + 0x40);
    a1 = v0 - s4;
    a0 += v1;
    v0 = 0x40000;                                       // Result = 00040000
    a0 += v0;
    v1 = -a0;
    v0 = (i32(a1) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a0) < i32(a1));
        if (bJump) goto loc_80028ADC;
    }
    if (v0 != 0) goto loc_80028ADC;
    v0 = lw(s0 + 0x4);
    a1 = v0 - s3;
    v0 = (i32(a1) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(a0) < i32(a1));
        if (bJump) goto loc_80028ADC;
    }
    a0 = s0;
    if (v0 != 0) goto loc_80028ADC;
    a1 = s1;
    a2 = s1;
    a3 = 0x2710;                                        // Result = 00002710
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    v1 = lw(s0 + 0x64);
    v0 = -7;                                            // Result = FFFFFFF9
    v1 &= v0;
    sw(v1, s0 + 0x64);
loc_80028ADC:
    s0 = lw(s0 + 0x14);
    if (s0 != s5) goto loc_80028A54;
loc_80028AEC:
    a1 = lw(s2);
    a2 = lw(s2 + 0x4);
    a0 = s1;
    v0 = P_TryMove(*vmAddrToPtr<mobj_t>(a0), a1, a2);
    a0 = 0xFFFF0000;                                    // Result = FFFF0000
    v1 = lw(s1 + 0x64);
    a0 |= 0x7FFF;                                       // Result = FFFF7FFF
    v1 &= a0;
    sw(v1, s1 + 0x64);
    if (v0 == 0) goto loc_80028C00;
    a0 = s7;
    a1 = fp;
    a2 = s6;
    v0 = lw(s1 + 0x38);
    a3 = 0x1D;                                          // Result = 0000001D
    sw(v0, s1 + 0x8);
    v0 = ptrToVmAddr(P_SpawnMObj(a0, a1, a2, (mobjtype_t) a3));
    a0 = v0;
    a1 = sfx_telept;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    a3 = 0x1D;
    v1 = lw(s2 + 0x24);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    a2 = lw(s1 + 0x8);
    v1 >>= 19;
    v1 <<= 2;
    v0 += v1;
    v0 = lw(v0);
    a1 = lw(s2);
    a0 = v0 << 2;
    a0 += v0;
    a0 <<= 2;
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v1;
    v0 = lw(at);
    a0 += a1;
    a1 = v0 << 2;
    a1 += v0;
    v0 = lw(s2 + 0x4);
    a1 <<= 2;
    a1 += v0;
    v0 = ptrToVmAddr(P_SpawnMObj(a0, a1, a2, (mobjtype_t) a3));
    a0 = v0;
    a1 = sfx_telept;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    v0 = lw(s1 + 0x80);
    {
        const bool bJump = (v0 == 0);
        v0 = 9;                                         // Result = 00000009
        if (bJump) goto loc_80028BBC;
    }
    sw(v0, s1 + 0x78);
loc_80028BBC:
    v1 = lw(s2 + 0x24);
    v0 = 1;                                             // Result = 00000001
    sw(0, s1 + 0x50);
    sw(0, s1 + 0x4C);
    sw(0, s1 + 0x48);
    sw(v1, s1 + 0x24);
    goto loc_80028C04;
loc_80028BD8:
    s2 = lw(s2 + 0x14);
    if (s2 != s5) goto loc_800289D4;
loc_80028BE8:
    v0 = *gNumSectors;
    a1++;
    v0 = (i32(a1) < i32(v0));
    a3 += 0x5C;
    if (v0 != 0) goto loc_800289A4;
loc_80028C00:
    v0 = 0;                                             // Result = 00000000
loc_80028C04:
    ra = lw(sp + 0x3C);
    fp = lw(sp + 0x38);
    s7 = lw(sp + 0x34);
    s6 = lw(sp + 0x30);
    s5 = lw(sp + 0x2C);
    s4 = lw(sp + 0x28);
    s3 = lw(sp + 0x24);
    s2 = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x40;
    return;
}
