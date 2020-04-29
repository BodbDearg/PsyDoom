#include "p_base.h"

#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "doomdata.h"
#include "p_local.h"
#include "p_setup.h"
#include "PsxVm/PsxVm.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <algorithm>
END_THIRD_PARTY_INCLUDES

static const VmPtr<VmPtr<mobj_t>>           gpBaseThing(0x8007824C);        // The current thing that is doing collision testing against other stuff: used by various functions in the module
static const VmPtr<fixed_t>                 gTestX(0x80077EF4);             // The thing position to use for collision testing - X
static const VmPtr<fixed_t>                 gTestY(0x80077F00);             // The thing position to use for collision testing - Y
static const VmPtr<fixed_t[4]>              gTestBBox(0x800A9064);          // Bounding box for various collision tests
static const VmPtr<uint32_t>                gTestFlags(0x800782B4);         // Used in place of 'mobj_t' flags for various functions in this module
static const VmPtr<VmPtr<subsector_t>>      gpTestSubSec(0x80077F28);       // Current cached thing subsector: input and output for some functions in this module
static const VmPtr<VmPtr<mobj_t>>           gpHitThing(0x80078184);         // The thing that was collided against during collision testing
static const VmPtr<VmPtr<line_t>>           gpCeilingLine(0x80077F8C);      // Collision testing: the line for the lowest ceiling edge the collider is in contact with
static const VmPtr<fixed_t>                 gTestCeilingz(0x80078104);      // Collision testing: the Z value for the lowest ceiling the collider is in contact with
static const VmPtr<fixed_t>                 gTestFloorZ(0x80077F68);        // Collision testing: the Z value for the highest floor the collider is in contact with
static const VmPtr<fixed_t>                 gTestDropoffZ(0x80078120);      // Collision testing: the Z value for the lowest floor the collider is in contact with. Used by monsters so they don't walk off cliffs.

void P_RunMobjBase() noexcept {
loc_80013840:
    sp -= 0x18;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    v1 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 -= 0x14;                                         // Result = gMObjHead[0] (800A8E90)
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7FB8);                                 // Store to: gCurMObjIdx (80077FB8)
    sw(v1, gp + 0xC6C);                                 // Store to: gpCurMObj (8007824C)
    s0 = v0;                                            // Result = gMObjHead[0] (800A8E90)
    if (v1 == v0) goto loc_800138C4;
loc_80013870:
    a0 = lw(gp + 0xC6C);                                // Load from: gpCurMObj (8007824C)
    v0 = lw(a0 + 0x80);
    if (v0 != 0) goto loc_800138A8;
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7FB8);                               // Load from: gCurMObjIdx (80077FB8)
    sw(0, a0 + 0x18);
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7FB8);                                // Store to: gCurMObjIdx (80077FB8)
    P_MobjThinker();
loc_800138A8:
    v0 = lw(gp + 0xC6C);                                // Load from: gpCurMObj (8007824C)
    v0 = lw(v0 + 0x14);
    sw(v0, gp + 0xC6C);                                 // Store to: gpCurMObj (8007824C)
    if (v0 != s0) goto loc_80013870;
loc_800138C4:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_XYMovement() noexcept {
loc_800138D8:
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = a0;
    v1 = -8;                                            // Result = FFFFFFF8
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(s0 + 0x48);
    a0 = 0x100000;                                      // Result = 00100000
    s4 = v0 & v1;
    v0 = lw(s0 + 0x4C);
    s2 = s4;
    s3 = v0 & v1;
    v0 = s4 + a0;
    v1 = 0x200000;                                      // Result = 00200000
    v0 = (v1 < v0);
    s1 = s3;
    if (v0 != 0) goto loc_80013938;
    v0 = s3 + a0;
    v0 = (v1 < v0);
    if (v0 == 0) goto loc_80013A1C;
loc_80013938:
    s2 = u32(i32(s2) >> 1);
    a0 = 0x100000;                                      // Result = 00100000
loc_80013940:
    v0 = s2 + a0;
    v1 = 0x200000;                                      // Result = 00200000
    v0 = (v1 < v0);
    s1 = u32(i32(s1) >> 1);
    if (v0 != 0) goto loc_80013938;
    v0 = s1 + a0;
    v0 = (v1 < v0);
    if (v0 == 0) goto loc_80013A1C;
    s2 = u32(i32(s2) >> 1);
    goto loc_80013940;
loc_8001396C:
    s3 -= s1;
    a0 = lw(s0);
    a1 = lw(s0 + 0x4);
    a0 += s2;
    a1 += s1;
    PB_TryMove();
    v1 = 0x1000000;                                     // Result = 01000000
    if (v0 != 0) goto loc_80013A1C;
    v0 = lw(s0 + 0x64);
    v0 &= v1;
    if (v0 == 0) goto loc_800139B4;
    v1 = lw(gp + 0xBA4);                                // Load from: gpHitThing (80078184)
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x72AC;                                       // Result = L_SkullBash (80018D54)
    sw(v0, s0 + 0x18);
    sw(v1, s0 + 0x84);
loc_800139B4:
    v0 = lw(s0 + 0x64);
    v1 = 0x10000;                                       // Result = 00010000
    v0 &= v1;
    if (v0 == 0) goto loc_80013A10;
    v0 = lw(gp + 0x9AC);                                // Load from: gpCeilingLine (80077F8C)
    if (v0 == 0) goto loc_800139F8;
    v0 = lw(v0 + 0x3C);
    if (v0 == 0) goto loc_800139F8;
    v1 = lw(v0 + 0xC);
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 == v0) goto loc_80013AC4;
loc_800139F8:
    v1 = lw(gp + 0xBA4);                                // Load from: gpHitThing (80078184)
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x7320;                                       // Result = L_MissileHit (80018CE0)
    sw(v0, s0 + 0x18);
    sw(v1, s0 + 0x84);
    goto loc_80013B14;
loc_80013A10:
    sw(0, s0 + 0x4C);
    sw(0, s0 + 0x48);
    goto loc_80013B14;
loc_80013A1C:
    {
        const bool bJump = (s4 != 0);
        s4 -= s2;
        if (bJump) goto loc_8001396C;
    }
    s4 += s2;
    s4 -= s2;
    if (s3 != 0) goto loc_8001396C;
    v1 = lw(s0 + 0x64);
    v0 = 0x1010000;                                     // Result = 01010000
    v0 &= v1;
    s4 += s2;
    if (v0 != 0) goto loc_80013B14;
    v0 = lw(s0 + 0x8);
    a0 = lw(s0 + 0x38);
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = 0x100000;                                  // Result = 00100000
        if (bJump) goto loc_80013B14;
    }
    v0 &= v1;
    if (v0 == 0) goto loc_80013A88;
    v0 = lw(s0 + 0xC);
    v0 = lw(v0);
    v0 = lw(v0);
    if (a0 != v0) goto loc_80013B14;
loc_80013A88:
    v0 = lw(s0 + 0x48);
    v0 += 0xFFF;
    v0 = (v0 < 0x1FFF);
    if (v0 == 0) goto loc_80013AD4;
    v0 = lw(s0 + 0x4C);
    v0 += 0xFFF;
    v0 = (v0 < 0x1FFF);
    if (v0 == 0) goto loc_80013AD4;
    sw(0, s0 + 0x48);
    sw(0, s0 + 0x4C);
    goto loc_80013B14;
loc_80013AC4:
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x38DC;                                       // Result = P_RemoveMObj (8001C724)
    sw(v0, s0 + 0x18);
    goto loc_80013B14;
loc_80013AD4:
    v0 = lw(s0 + 0x48);
    a0 = lw(s0 + 0x4C);
    v0 = u32(i32(v0) >> 8);
    v1 = v0 << 3;
    v1 -= v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 1;
    a0 = u32(i32(a0) >> 8);
    v1 = a0 << 3;
    v1 -= a0;
    sw(v0, s0 + 0x48);
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 1;
    sw(v0, s0 + 0x4C);
loc_80013B14:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void P_FloatChange() noexcept {
    a1 = lw(a0 + 0x74);
    v0 = lw(a0);
    v1 = lw(a1);
    a2 = v1 - v0;
    if (i32(a2) >= 0) goto loc_80013B58;
    a2 = -a2;
loc_80013B58:
    v1 = lw(a1 + 0x4);
    v0 = lw(a0 + 0x4);
    a1 = v1 - v0;
    v0 = (i32(a2) < i32(a1));
    if (i32(a1) >= 0) goto loc_80013B78;
    a1 = -a1;
    v0 = (i32(a2) < i32(a1));
loc_80013B78:
    v1 = a2 + a1;
    if (v0 == 0) goto loc_80013B88;
    v0 = u32(i32(a2) >> 1);
    goto loc_80013B8C;
loc_80013B88:
    v0 = u32(i32(a1) >> 1);
loc_80013B8C:
    a1 = v1 - v0;
    v0 = lw(a0 + 0x44);
    v1 = lw(a0 + 0x74);
    a2 = lw(a0 + 0x8);
    v1 = lw(v1 + 0x8);
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    v1 = v0 - a2;
    v0 = v1 << 1;
    if (i32(v1) >= 0) goto loc_80013BD0;
    v0 += v1;
    v0 = -v0;
    v0 = (i32(a1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFFF80000;                                // Result = FFF80000
        if (bJump) goto loc_80013BD0;
    }
    v0 += a2;
    goto loc_80013BF4;
loc_80013BD0:
    v0 = v1 << 1;
    if (i32(v1) <= 0) goto loc_80013BF8;
    v0 += v1;
    v0 = (i32(a1) < i32(v0));
    v1 = 0x80000;                                       // Result = 00080000
    if (v0 == 0) goto loc_80013BF8;
    v0 = lw(a0 + 0x8);
    v0 += v1;
loc_80013BF4:
    sw(v0, a0 + 0x8);
loc_80013BF8:
    return;
}

void P_ZMovement() noexcept {
loc_80013C00:
    a2 = a0;
    v0 = lw(a2 + 0x8);
    a0 = lw(a2 + 0x50);
    v1 = lw(a2 + 0x64);
    v0 += a0;
    v1 &= 0x4000;
    sw(v0, a2 + 0x8);
    if (v1 == 0) goto loc_80013CEC;
    a0 = lw(a2 + 0x74);
    if (a0 == 0) goto loc_80013CEC;
    v1 = lw(a0);
    v0 = lw(a2);
    a1 = v1 - v0;
    if (i32(a1) >= 0) goto loc_80013C4C;
    a1 = -a1;
loc_80013C4C:
    v1 = lw(a0 + 0x4);
    v0 = lw(a2 + 0x4);
    a0 = v1 - v0;
    v0 = (i32(a1) < i32(a0));
    if (i32(a0) >= 0) goto loc_80013C6C;
    a0 = -a0;
    v0 = (i32(a1) < i32(a0));
loc_80013C6C:
    v1 = a1 + a0;
    if (v0 == 0) goto loc_80013C7C;
    v0 = u32(i32(a1) >> 1);
    goto loc_80013C80;
loc_80013C7C:
    v0 = u32(i32(a0) >> 1);
loc_80013C80:
    a0 = v1 - v0;
    v0 = lw(a2 + 0x44);
    v1 = lw(a2 + 0x74);
    a1 = lw(a2 + 0x8);
    v1 = lw(v1 + 0x8);
    v0 = u32(i32(v0) >> 1);
    v0 += v1;
    v1 = v0 - a1;
    v0 = v1 << 1;
    if (i32(v1) >= 0) goto loc_80013CC4;
    v0 += v1;
    v0 = -v0;
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = 0xFFF80000;                                // Result = FFF80000
        if (bJump) goto loc_80013CC4;
    }
    v0 += a1;
    goto loc_80013CE8;
loc_80013CC4:
    v0 = v1 << 1;
    if (i32(v1) <= 0) goto loc_80013CEC;
    v0 += v1;
    v0 = (i32(a0) < i32(v0));
    v1 = 0x80000;                                       // Result = 00080000
    if (v0 == 0) goto loc_80013CEC;
    v0 = lw(a2 + 0x8);
    v0 += v1;
loc_80013CE8:
    sw(v0, a2 + 0x8);
loc_80013CEC:
    v1 = lw(a2 + 0x8);
    v0 = lw(a2 + 0x38);
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_80013D40;
    v0 = lw(a2 + 0x50);
    v1 = 0x10000;                                       // Result = 00010000
    if (i32(v0) >= 0) goto loc_80013D18;
    sw(0, a2 + 0x50);
loc_80013D18:
    v0 = lw(a2 + 0x38);
    sw(v0, a2 + 0x8);
    v0 = lw(a2 + 0x64);
    v0 &= v1;
    if (v0 == 0) goto loc_80013D74;
    goto loc_80013DCC;
loc_80013D40:
    v0 = lw(a2 + 0x64);
    v0 &= 0x200;
    if (v0 != 0) goto loc_80013D74;
    v1 = lw(a2 + 0x50);
    v0 = 0xFFFE0000;                                    // Result = FFFE0000
    if (v1 != 0) goto loc_80013D6C;
    v0 = 0xFFFC0000;                                    // Result = FFFC0000
    goto loc_80013D70;
loc_80013D6C:
    v0 += v1;
loc_80013D70:
    sw(v0, a2 + 0x50);
loc_80013D74:
    v0 = lw(a2 + 0x8);
    a0 = lw(a2 + 0x44);
    v1 = lw(a2 + 0x3C);
    v0 += a0;
    v1 = (i32(v1) < i32(v0));
    if (v1 == 0) goto loc_80013DD8;
    v0 = lw(a2 + 0x50);
    if (i32(v0) <= 0) goto loc_80013DA4;
    sw(0, a2 + 0x50);
loc_80013DA4:
    v0 = lw(a2 + 0x3C);
    v1 = lw(a2 + 0x44);
    v0 -= v1;
    sw(v0, a2 + 0x8);
    v0 = lw(a2 + 0x64);
    v1 = 0x10000;                                       // Result = 00010000
    v0 &= v1;
    if (v0 == 0) goto loc_80013DD8;
loc_80013DCC:
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x3464;                                       // Result = P_ExplodeMissile (8001CB9C)
    sw(v0, a2 + 0x18);
loc_80013DD8:
    return;
}

void P_MobjThinker() noexcept {
loc_80013DE0:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v0 = lw(s0 + 0x48);
    if (v0 != 0) goto loc_80013E10;
    v0 = lw(s0 + 0x4C);
    if (v0 == 0) goto loc_80013E28;
loc_80013E10:
    a0 = s0;
    P_XYMovement();
    v0 = lw(s0 + 0x18);
    if (v0 != 0) goto loc_80013EEC;
loc_80013E28:
    v1 = lw(s0 + 0x8);
    v0 = lw(s0 + 0x38);
    if (v1 != v0) goto loc_80013E4C;
    v0 = lw(s0 + 0x50);
    if (v0 == 0) goto loc_80013E64;
loc_80013E4C:
    a0 = s0;
    P_ZMovement();
    v0 = lw(s0 + 0x18);
    if (v0 != 0) goto loc_80013EEC;
loc_80013E64:
    v1 = lw(s0 + 0x5C);
    v0 = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (v1 == v0);
        v0 = v1 - 1;
        if (bJump) goto loc_80013EEC;
    }
    sw(v0, s0 + 0x5C);
    if (i32(v0) > 0) goto loc_80013EEC;
    v0 = lw(s0 + 0x60);
    v1 = lw(v0 + 0x10);
    v0 = v1 << 3;
    if (v1 != 0) goto loc_80013EA4;
    v0 = 0x80020000;                                    // Result = 80020000
    v0 -= 0x38DC;                                       // Result = P_RemoveMObj (8001C724)
    sw(v0, s0 + 0x18);
    goto loc_80013EEC;
loc_80013EA4:
    v0 -= v1;
    v0 <<= 2;
    v1 = 0x80060000;                                    // Result = 80060000
    v1 -= 0x7274;                                       // Result = State_S_NULL[0] (80058D8C)
    v0 += v1;
    sw(v0, s0 + 0x60);
    v1 = lw(v0 + 0x8);
    sw(v1, s0 + 0x5C);
    v1 = lw(v0);
    sw(v1, s0 + 0x28);
    v1 = lw(v0 + 0x4);
    sw(v1, s0 + 0x2C);
    v0 = lw(v0 + 0xC);
    sw(v0, s0 + 0x18);
loc_80013EEC:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void PB_TryMove() noexcept {
loc_80013F00:
    sp -= 0x18;
    sw(ra, sp + 0x10);
    sw(a0, gp + 0x914);                                 // Store to: gTestX (80077EF4)
    sw(a1, gp + 0x920);                                 // Store to: gTestY (80077F00)
    v0 = PB_CheckPosition();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80013FD0;
    }
    a2 = lw(gp + 0xB24);                                // Load from: gTestCeilingz (80078104)
    v1 = lw(gp + 0xC6C);                                // Load from: gpCurMObj (8007824C)
    a0 = lw(gp + 0x988);                                // Load from: gTestFloorZ (80077F68)
    a1 = lw(v1 + 0x44);
    v0 = a2 - a0;
    v0 = (i32(v0) < i32(a1));
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80013FD0;
    }
    v1 = lw(v1 + 0x8);
    v0 = a2 - v1;
    v0 = (i32(v0) < i32(a1));
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80013FD0;
    }
    v0 = a0 - v1;
    v1 = 0x180000;                                      // Result = 00180000
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80013FD0;
    }
    v0 = *gTestFlags;
    v0 &= 0x4400;
    if (v0 != 0) goto loc_80013F98;
    v0 = lw(gp + 0xB40);                                // Load from: gTestDropoffZ (80078120)
    v0 = a0 - v0;
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80013FD0;
    }
loc_80013F98:
    a0 = lw(gp + 0xC6C);                                // Load from: gpCurMObj (8007824C)
    PB_UnsetThingPosition(*vmAddrToPtr<mobj_t>(a0));

    a0 = lw(gp + 0xC6C);                                // Load from: gpCurMObj (8007824C)
    v0 = lw(gp + 0x988);                                // Load from: gTestFloorZ (80077F68)
    v1 = lw(gp + 0xB24);                                // Load from: gTestCeilingz (80078104)
    a1 = lw(gp + 0x914);                                // Load from: gTestX (80077EF4)
    a2 = lw(gp + 0x920);                                // Load from: gTestY (80077F00)
    sw(v0, a0 + 0x38);
    sw(v1, a0 + 0x3C);
    sw(a1, a0);
    sw(a2, a0 + 0x4);
    PB_SetThingPosition(*vmAddrToPtr<mobj_t>(a0));
    v0 = 1;                                             // Result = 00000001
loc_80013FD0:
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unlinks the given thing from sector thing lists and the blockmap.
// Very similar to 'P_UnsetThingPosition' except the thing is always unlinked from sectors and thing flags are read from a global.
//------------------------------------------------------------------------------------------------------------------------------------------
void PB_UnsetThingPosition(mobj_t& thing) noexcept {
    // Remove the thing from sector thing lists
    if (thing.snext) {
        thing.snext->sprev = thing.sprev;
    }
    
    if (thing.sprev) {
        thing.sprev->snext = thing.snext;
    } else {
        thing.subsector->sector->thinglist = thing.snext;
    }

    // Remove the thing from the blockmap, if it is added to the blockmap
    if ((*gTestFlags & MF_NOBLOCKMAP) == 0) {
        if (thing.bnext) {
            thing.bnext->bprev = thing.bprev;
        }

        if (thing.bprev) {
            thing.bprev->bnext = thing.bnext;
        } else {
            const int32_t blockx = (thing.x - *gBlockmapOriginX) >> MAPBLOCKSHIFT;
            const int32_t blocky = (thing.y - *gBlockmapOriginY) >> MAPBLOCKSHIFT;

            // PC-PSX: prevent buffer overflow if the map object is out of bounds.
            // This is part of the fix for the famous 'linedef deletion' bug.
            #if PC_PSX_DOOM_MODS
                if (blockx >= 0 && blockx < *gBlockmapWidth) {
                    if (blocky >= 0 && blocky < *gBlockmapHeight) {
                        (*gppBlockLinks)[blocky * (*gBlockmapWidth) + blockx] = thing.bnext;
                    }
                }
            #else
                (*gppBlockLinks)[blocky * (*gBlockmapWidth) + blockx] = thing.bnext;
            #endif
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Links the given thing to sector thing lists and the blockmap. Very similar to 'P_SetThingPosition' except the thing is always linked
// to sectors and thing flags and the current subsector are taken from globals set elsewhere.
//------------------------------------------------------------------------------------------------------------------------------------------
void PB_SetThingPosition(mobj_t& mobj) noexcept {
    // Add the thing into the sector thing linked list
    subsector_t& subsec = *gpTestSubSec->get();
    sector_t& sec = *subsec.sector;

    mobj.subsector = &subsec;    
    mobj.sprev = nullptr;
    mobj.snext = sec.thinglist;
    
    if (sec.thinglist) {
        sec.thinglist->sprev = &mobj;
    }
    
    sec.thinglist = &mobj;

    // Add the thing into the blockmap unless the thing flags specify otherwise (inert things)
    if ((*gTestFlags & MF_NOBLOCKMAP) == 0) {
        // Compute the blockmap cell and see if it's in range for the blockmap
        const int32_t bmapX = (mobj.x - *gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t bmapY = (mobj.y - *gBlockmapOriginY) >> MAPBLOCKSHIFT;
        
        if ((bmapX >= 0) && (bmapY >= 0) && (bmapX < *gBlockmapWidth) && (bmapY < *gBlockmapHeight)) {
            // In range: link the thing into the blockmap list for this blockmap cell
            VmPtr<mobj_t>& blockmapList = gppBlockLinks->get()[bmapX + bmapY * (*gBlockmapWidth)];
            mobj_t* const pPrevListHead = blockmapList.get();
            
            mobj.bprev = nullptr;
            mobj.bnext = pPrevListHead;

            if (pPrevListHead) {
                pPrevListHead->bprev = &mobj;
            }

            blockmapList = &mobj;
        } else {
            // Thing is outside the blockmap
            mobj.bprev = nullptr;
            mobj.bnext = nullptr;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do a collision test for a thing against lines and other things, ignoring height differences.
// Returns 'false' if there was a collision, 'true' if there was no collision when height differences are ignored.
// Note: height difference blocking logic is handled externally to this function.
//
// Inputs:
//  gpBaseThing     : The thing doing the collision test
//  gTestX, gTestY  : Position to use for the thing for the collision test (can be set different to actual pos to test a move)
//
// Outputs:
//  gTestBBox       : The bounding box for the thing
//  gpTestSubSec    : The subsector the thing is in
//  gpHitThing      : The thing collided with
//  gpCeilingLine   : The upper wall line for the lowest ceiling touched
//  gTestCeilingz   : The Z value for the lowest ceiling touched
//  gTestFloorZ     : The Z value for the highest floor touched
//  gTestDropoffZ   : The Z value for the lowest floor touched
//------------------------------------------------------------------------------------------------------------------------------------------
bool PB_CheckPosition() noexcept {
    // Save the bounding box, flags and subsector for the thing having collision testing done
    mobj_t& baseThing = *gpBaseThing->get();
    *gTestFlags = baseThing.flags;

    {
        const fixed_t radius = baseThing.radius;
        gTestBBox[0] = *gTestY + radius;
        gTestBBox[1] = *gTestY - radius;
        gTestBBox[3] = *gTestX + radius;
        gTestBBox[2] = *gTestX - radius;
    }

    subsector_t& testSubsec = *R_PointInSubsector(*gTestX, *gTestY);
    sector_t& testSec = *testSubsec.sector;
    *gpTestSubSec = &testSubsec;

    // Initially have collided with nothing
    *gpCeilingLine = nullptr;
    *gpHitThing = nullptr;

    // Initialize the lowest ceiling, and highest/lowest floor values to that of the initial subsector
    *gTestFloorZ = testSec.floorheight;
    *gTestDropoffZ = testSec.floorheight;
    *gTestCeilingz = testSec.ceilingheight;

    // Determine the blockmap extents (left/right, top/bottom) to be tested against for collision and clamp to a valid range
    int32_t bmapLx = (gTestBBox[2] - *gBlockmapOriginX - MAXRADIUS) >> MAPBLOCKSHIFT;
    int32_t bmapRx = (gTestBBox[3] - *gBlockmapOriginX + MAXRADIUS) >> MAPBLOCKSHIFT;
    int32_t bmapTy = (gTestBBox[0] - *gBlockmapOriginY + MAXRADIUS) >> MAPBLOCKSHIFT;
    int32_t bmapBy = (gTestBBox[1] - *gBlockmapOriginY - MAXRADIUS) >> MAPBLOCKSHIFT;

    bmapLx = std::max(bmapLx, 0);
    bmapBy = std::max(bmapBy, 0);
    bmapRx = std::min(bmapRx, *gBlockmapWidth - 1);
    bmapTy = std::min(bmapTy, *gBlockmapHeight - 1);

    // This is a new collision test so increment this stamp
    *gValidCount += 1;

    // Test against everything in this blockmap range.
    // Stop and return 'false' for a definite collision if that happens.
    for (int32_t x = bmapLx; x <= bmapRx; ++x) {
        for (int32_t y = bmapBy; y <= bmapTy; ++y) {
            // Test against lines first then things
            if (!PB_BlockLinesIterator(x, y))
                return false;
            
            if (!PB_BlockThingsIterator(x, y))
                return false;
        }
    }

    // No definite collision, there may still be collisions due to height differences however
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Test if 'gTestBBox' intersects the given line: returns 'true' if there is an intersection
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PB_BoxCrossLine(line_t& line) noexcept {
    // Check if the test bounding box is outside the bounding box of the line: if it is then early out
    const bool bTestBBOutsideLineBB = (
        (gTestBBox[BOXTOP] <= line.bbox[BOXBOTTOM]) ||
        (gTestBBox[BOXBOTTOM] >= line.bbox[BOXTOP]) ||
        (gTestBBox[BOXLEFT] >= line.bbox[BOXRIGHT]) ||
        (gTestBBox[BOXRIGHT] <= line.bbox[BOXLEFT])
    );

    if (bTestBBOutsideLineBB)
        return false;

    // Choose what line diagonal in the test box to test for crossing the line.
    // This code is trying to get a box diagonal that is as perpendicular to the line as possible.
    // Some lines for instance might run at 45 degrees and be parallel to the opposite box diagonal...
    fixed_t x1;
    fixed_t x2;
    
    if (line.slopetype == ST_POSITIVE) {
        x1 = gTestBBox[BOXLEFT];
        x2 = gTestBBox[BOXRIGHT];
    } else {
        x1 = gTestBBox[BOXRIGHT];
        x2 = gTestBBox[BOXLEFT];
    }

    // Use the cross product trick found in many functions such as 'R_PointOnSide' to determine what side of the line
    // both points of the test bounding box diagonal lie on.
    const fixed_t lx = line.vertex1->x;
    const fixed_t ly = line.vertex1->y;
    const int32_t ldx = line.dx >> FRACBITS;
    const int32_t ldy = line.dy >> FRACBITS;

    const int32_t dx1 = (x1 - lx) >> FRACBITS;
    const int32_t dy1 = (gTestBBox[BOXTOP] - ly) >> FRACBITS;
    const int32_t dx2 = (x2 - lx) >> FRACBITS;
    const int32_t dy2 = (gTestBBox[BOXBOTTOM] - ly) >> FRACBITS;

    const uint32_t side1 = (ldy * dx1 < dy1 * ldx);
    const uint32_t side2 = (ldy * dx2 < dy2 * ldx);

    // If the bounding box diagonal line points are on opposite sides of the line, then the box crosses the line
    return (side1 != side2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Assuming a collider intersects the given line, tells if the given line will potentially block - ignoring height differences.
// Returns 'false' if the line is considered blocking ignoring height differences.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PB_CheckLine(line_t& line) noexcept {
    // A 1 sided line cannot be crossed: register a collision against this
    if (!line.backsector)
        return false;

    // If not a projectile and the line is marked as explicitly blocking then block
    if ((*gTestFlags & MF_MISSILE) == 0) {
        if (line.flags & (ML_BLOCKING | ML_BLOCKMONSTERS)) {
            return false;
        }
    }

    // PSX Doom addition: block always if the line blocks projectiles
    if (line.flags & ML_BLOCKPRJECTILE) 
        return false;

    // Get the top and bottom height of the opening/gap and the lowest floor
    sector_t& fsec = *line.frontsector;
    sector_t& bsec = *line.backsector;

    const fixed_t openTop = std::min(fsec.ceilingheight, bsec.ceilingheight);
    const fixed_t openBottom = std::max(fsec.floorheight, bsec.floorheight);
    const fixed_t lowFloor = std::min(fsec.floorheight, bsec.floorheight);

    // Adjust the global low ceiling, high floor and lowest floor values
    if (openTop < *gTestCeilingz) {
        *gTestCeilingz = openTop;
        *gpCeilingLine = &line;
    }

    if (openBottom > *gTestFloorZ) {
        *gTestFloorZ = openBottom;
    }

    if (lowFloor < *gTestDropoffZ) {
        *gTestDropoffZ = lowFloor;
    }

    // This line does not block, ignoring height differences.
    // Tthis function does NOT check whether the thing can pass the line due to height differences!
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does collision testing for the current thing doing collision testing against the given thing.
// Returns 'false' if there is a collision and hence a blockage.
// If a collision occurs the hit thing is saved in most cases, except where damage is not desired for missiles.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool PB_CheckThing(mobj_t& mobj) noexcept {    
    // If the thing is not solid you can't collide against it
    if ((mobj.flags & MF_SOLID) == 0)
        return true;
    
    // Get the thing which is doing the collision test and see if it is close enough to this thing.
    // If it isn't then we can early out here and return 'true' for no collision:
    mobj_t& baseThing = *gpBaseThing->get();
    const fixed_t totalRadius = mobj.radius + baseThing.radius;
    
    const fixed_t dx = std::abs(mobj.x - *gTestX);
    const fixed_t dy = std::abs(mobj.y - *gTestY);

    if ((dx >= totalRadius) || (dy >= totalRadius))
        return true;
    
    // The thing can't collide with itself
    if (&mobj == &baseThing)
        return true;

    // Check for a lost soul slamming into things
    const int32_t testFlags = *gTestFlags;

    if (testFlags & MF_SKULLFLY) {
        *gpHitThing = &mobj;
        return false;
    }

    // Missiles are special and are allowed to fly over and under things.
    // Most things have 'infinite' height in DOOM with regard to collision detection.
    if (testFlags & MF_MISSILE) {
        // Is the missile flying over this thing?
        if (baseThing.z > mobj.z + mobj.height)
            return true;
        
        // Is the missile flying under this thing?
        if (baseThing.z + baseThing.height < mobj.z)
            return true;

        // Is the missile hitting the same species that it came from?
        const mobjtype_t sourceObjType = baseThing.target->type;

        if (sourceObjType == mobj.type) {
            // Colliding with the same species type: don't explode the missile if it's hitting the shooter of the missile
            if (&mobj == baseThing.target.get())
                return true;
            
            // If it's hitting anything other than the player, explode the missile but do no damage (set no 'hit' thing).
            // Players can still damage each other with missiles however, hence the exception.
            if (sourceObjType != MT_PLAYER)
                return false;
        }
        
        // So long as the thing is shootable then the missile can hit it
        if (mobj.flags & MF_SHOOTABLE) {
            *gpHitThing = &mobj;
            return false;
        }
    }

    // Non missile: the collider is colliding against this thing if it is solid.
    // Set no hit thing here however because this is not a missile that can potentially do damage.
    return ((mobj.flags & MF_SOLID) == 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check for potential collisions against all lines in the given blockmap cell, ignoring height differences.
// Returns 'false' if there is a definite collision, 'true' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
bool PB_BlockLinesIterator(const int32_t x, const int32_t y) noexcept {
    // Get the line list for this blockmap cell
    const int16_t* pLineNum = (int16_t*)(gpBlockmapLump->get() + gpBlockmap->get()[y * (*gBlockmapWidth) + x]);

    // Visit all lines in the cell, checking for intersection and potential collision.
    // Stop when there is a definite collision.
    for (; *pLineNum != -1; ++pLineNum) {
        line_t& line = gpLines->get()[*pLineNum];

        // Only check the line if not already checked this test
        if (line.validcount != *gValidCount) {
            line.validcount = *gValidCount;

            // If it's collided with and definitely blocking then stop
            if (PB_BoxCrossLine(line) && (!PB_CheckLine(line)))
                return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does collision detection against all things in the specified blockmap cell's linked list of things.
// Stops when a collision is detected and returns 'false', otherwise returns 'true' for no collision.
//------------------------------------------------------------------------------------------------------------------------------------------
bool PB_BlockThingsIterator(const int32_t x, const int32_t y) noexcept {
    mobj_t* pmobj = gppBlockLinks->get()[x + y * (*gBlockmapWidth)].get();

    while (pmobj) {
        if (!PB_CheckThing(*pmobj))
            return false;

        pmobj = pmobj->bnext.get();
    }

    return true;
}
