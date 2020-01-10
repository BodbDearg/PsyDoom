#include "p_base.h"

#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "p_local.h"
#include "p_setup.h"
#include "PsxVm/PsxVm.h"

// Used in place of 'mobj_t' flags in 'PB_UnsetThingPosition' and 'PB_SetThingPosition' etc.
static const VmPtr<int32_t> gTestFlags(0x800782B4);

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
    PB_CheckPosition();
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
    PB_SetThingPosition();
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

void PB_SetThingPosition() noexcept {
loc_800140DC:
    v0 = lw(gp + 0x948);                                // Load from: gpTestSubSec (80077F28)
    a1 = a0;
    sw(v0, a1 + 0xC);
    v1 = lw(v0);
    sw(0, a1 + 0x20);
    v0 = lw(v1 + 0x4C);
    sw(v0, a1 + 0x1C);
    v0 = lw(v1 + 0x4C);
    if (v0 == 0) goto loc_80014110;
    sw(a1, v0 + 0x20);
loc_80014110:
    v0 = *gTestFlags;
    v0 &= 0x10;
    sw(a1, v1 + 0x4C);
    if (v0 != 0) goto loc_800141D4;
    v1 = lw(a1);
    v0 = *gBlockmapOriginX;
    a0 = *gBlockmapOriginY;
    v1 -= v0;
    v0 = lw(a1 + 0x4);
    v1 = u32(i32(v1) >> 23);
    v0 -= a0;
    a0 = u32(i32(v0) >> 23);
    if (i32(v1) < 0) goto loc_800141CC;
    a2 = *gBlockmapWidth;
    v0 = (i32(v1) < i32(a2));
    if (v0 == 0) goto loc_800141CC;
    if (i32(a0) < 0) goto loc_800141CC;
    v0 = *gBlockmapHeight;
    v0 = (i32(a0) < i32(v0));
    mult(a0, a2);
    if (v0 == 0) goto loc_800141CC;
    sw(0, a1 + 0x34);
    v0 = lo;
    v0 += v1;
    v1 = *gppBlockLinks;
    v0 <<= 2;
    v1 += v0;
    v0 = lw(v1);
    sw(v0, a1 + 0x30);
    v0 = lw(v1);
    if (v0 == 0) goto loc_800141C4;
    sw(a1, v0 + 0x34);
loc_800141C4:
    sw(a1, v1);
    goto loc_800141D4;
loc_800141CC:
    sw(0, a1 + 0x34);
    sw(0, a1 + 0x30);
loc_800141D4:
    return;
}

void PB_CheckPosition() noexcept {
loc_800141DC:
    a0 = lw(gp + 0x914);                                // Load from: gTestX (80077EF4)
    v1 = lw(gp + 0xC6C);                                // Load from: gpCurMObj (8007824C)
    a1 = lw(gp + 0x920);                                // Load from: gTestY (80077F00)
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lw(v1 + 0x64);
    v1 = lw(v1 + 0x40);
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x6F9C;                                       // Result = gTestBBox[0] (800A9064)
    *gTestFlags = v0;
    v0 = v1 + a1;
    sw(v0, s0);                                         // Store to: gTestBBox[0] (800A9064)
    v0 = a1 - v1;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x6F98);                                // Store to: gTestBBox[1] (800A9068)
    v0 = v1 + a0;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x6F90);                                // Store to: gTestBBox[3] (800A9070)
    v0 = a0 - v1;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x6F94);                                // Store to: gTestBBox[2] (800A906C)
    _thunk_R_PointInSubsector();
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x6F94);                               // Load from: gTestBBox[2] (800A906C)
    a1 = *gBlockmapOriginX;
    a2 = *gBlockmapOriginY;
    v1 = 0xFFE00000;                                    // Result = FFE00000
    sw(v0, gp + 0x948);                                 // Store to: gpTestSubSec (80077F28)
    sw(0, gp + 0x9AC);                                  // Store to: gpCeilingLine (80077F8C)
    sw(0, gp + 0xBA4);                                  // Store to: gpHitThing (80078184)
    a0 -= a1;
    a0 += v1;
    a3 = u32(i32(a0) >> 23);
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6F90);                               // Load from: gTestBBox[3] (800A9070)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x6F98);                               // Load from: gTestBBox[1] (800A9068)
    v1 -= a1;
    a1 = 0x200000;                                      // Result = 00200000
    v1 += a1;
    s3 = u32(i32(v1) >> 23);
    a0 -= a2;
    a0 -= a1;
    s4 = u32(i32(a0) >> 23);
    v1 = lw(s0);                                        // Load from: gTestBBox[0] (800A9064)
    a0 = lw(v0);
    v1 -= a2;
    v1 += a1;
    s2 = u32(i32(v1) >> 23);
    a0 = lw(a0);
    v1 = lw(v0);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    v1 = lw(v1 + 0x4);
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    sw(a0, gp + 0xB40);                                 // Store to: gTestDropoffZ (80078120)
    sw(a0, gp + 0x988);                                 // Store to: gTestFloorZ (80077F68)
    sw(v1, gp + 0xB24);                                 // Store to: gTestCeilingz (80078104)
    if (i32(a3) >= 0) goto loc_800142F8;
    a3 = 0;                                             // Result = 00000000
loc_800142F8:
    if (i32(s4) >= 0) goto loc_80014304;
    s4 = 0;                                             // Result = 00000000
loc_80014304:
    v1 = *gBlockmapWidth;
    v0 = (i32(s3) < i32(v1));
    if (v0 != 0) goto loc_80014320;
    s3 = v1 - 1;
loc_80014320:
    v1 = *gBlockmapHeight;
    v0 = (i32(s2) < i32(v1));
    s1 = a3;
    if (v0 != 0) goto loc_8001433C;
    s2 = v1 - 1;
loc_8001433C:
    v0 = (i32(s3) < i32(s1));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001439C;
    }
loc_80014348:
    v0 = (i32(s2) < i32(s4));
    s0 = s4;
    if (v0 != 0) goto loc_8001438C;
    a0 = s1;
loc_80014358:
    a1 = s0;
    PB_CheckLines();
    a0 = s1;
    if (v0 == 0) goto loc_80014378;
    a1 = s0;
    PB_CheckThings();
    s0++;
    if (v0 != 0) goto loc_80014380;
loc_80014378:
    v0 = 0;                                             // Result = 00000000
    goto loc_8001439C;
loc_80014380:
    v0 = (i32(s2) < i32(s0));
    a0 = s1;
    if (v0 == 0) goto loc_80014358;
loc_8001438C:
    s1++;
    v0 = (i32(s3) < i32(s1));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80014348;
    }
loc_8001439C:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void PB_BoxCrossLine() noexcept {
    a1 = a0;
    a2 = 0x800B0000;                                    // Result = 800B0000
    a2 = lw(a2 - 0x6F90);                               // Load from: gTestBBox[3] (800A9070)
    v0 = lw(a1 + 0x2C);
    v0 = (i32(v0) < i32(a2));
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800144D0;
    }
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x6F94);                               // Load from: gTestBBox[2] (800A906C)
    v0 = lw(a1 + 0x30);
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800144D0;
    }
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6F9C);                               // Load from: gTestBBox[0] (800A9064)
    v0 = lw(a1 + 0x28);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800144D0;
    }
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6F98);                               // Load from: gTestBBox[1] (800A9068)
    v1 = lw(a1 + 0x24);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_8001443C;
    }
    v0 = 0;                                             // Result = 00000000
    goto loc_800144D0;
loc_8001443C:
    v1 = lw(a1 + 0x34);
    {
        const bool bJump = (v1 != v0);
        v0 = a2;
        if (bJump) goto loc_80014458;
    }
    v0 = a0;
    t1 = a2;
    goto loc_8001445C;
loc_80014458:
    t1 = a0;
loc_8001445C:
    a0 = lw(a1);
    v1 = lw(a0);
    a2 = lh(a1 + 0xE);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(a2, v0);
    a1 = lh(a1 + 0xA);
    a3 = lw(a0 + 0x4);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6F9C);                               // Load from: gTestBBox[0] (800A9064)
    a0 = lo;
    v0 -= a3;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a1);
    t0 = lo;
    v1 = t1 - v1;
    v1 = u32(i32(v1) >> 16);
    mult(a2, v1);
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6F98);                               // Load from: gTestBBox[1] (800A9068)
    v0 = lo;
    v1 -= a3;
    v1 = u32(i32(v1) >> 16);
    mult(v1, a1);
    a0 = (i32(a0) < i32(t0));
    v1 = lo;
    v0 = (i32(v0) < i32(v1));
    v0 ^= a0;
loc_800144D0:
    return;
}

void PB_CheckLine() noexcept {
    v0 = lw(a0 + 0x3C);
    v1 = 0x10000;                                       // Result = 00010000
    if (v0 == 0) goto loc_80014524;
    v0 = *gTestFlags;
    v0 &= v1;
    if (v0 != 0) goto loc_80014510;
    v0 = lw(a0 + 0x10);
    v0 &= 3;
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_800145BC;
    }
loc_80014510:
    v0 = lw(a0 + 0x10);
    v0 &= 0x800;
    if (v0 == 0) goto loc_8001452C;
loc_80014524:
    v0 = 0;                                             // Result = 00000000
    goto loc_800145BC;
loc_8001452C:
    a3 = lw(a0 + 0x3C);
    a1 = lw(a0 + 0x38);
    v0 = lw(a3 + 0x4);
    v1 = lw(a1 + 0x4);
    a2 = v0;
    v0 = (i32(v1) < i32(a2));
    if (v0 == 0) goto loc_80014550;
    a2 = v1;
loc_80014550:
    a1 = lw(a1);
    v1 = lw(a3);
    v0 = (i32(v1) < i32(a1));
    a3 = a1;
    if (v0 != 0) goto loc_80014570;
    a3 = v1;
    v1 = a1;
loc_80014570:
    v0 = lw(gp + 0xB24);                                // Load from: gTestCeilingz (80078104)
    v0 = (i32(a2) < i32(v0));
    if (v0 == 0) goto loc_8001458C;
    sw(a2, gp + 0xB24);                                 // Store to: gTestCeilingz (80078104)
    sw(a0, gp + 0x9AC);                                 // Store to: gpCeilingLine (80077F8C)
loc_8001458C:
    v0 = lw(gp + 0x988);                                // Load from: gTestFloorZ (80077F68)
    v0 = (i32(v0) < i32(a3));
    if (v0 == 0) goto loc_800145A4;
    sw(a3, gp + 0x988);                                 // Store to: gTestFloorZ (80077F68)
loc_800145A4:
    v0 = lw(gp + 0xB40);                                // Load from: gTestDropoffZ (80078120)
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800145BC;
    }
    sw(v1, gp + 0xB40);                                 // Store to: gTestDropoffZ (80078120)
loc_800145BC:
    return;
}

void PB_CheckThing() noexcept {
loc_800145C4:
    a2 = a0;
    t0 = lw(a2 + 0x64);
    v0 = t0 & 2;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800146E8;
    }
    a3 = lw(gp + 0xC6C);                                // Load from: gpCurMObj (8007824C)
    a1 = lw(a2 + 0x40);
    a0 = lw(a2);
    v0 = lw(gp + 0x914);                                // Load from: gTestX (80077EF4)
    v1 = lw(a3 + 0x40);
    v0 = a0 - v0;
    a1 += v1;
    if (i32(v0) >= 0) goto loc_80014600;
    v0 = -v0;
loc_80014600:
    v0 = (i32(v0) < i32(a1));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800146E8;
    }
    v1 = lw(a2 + 0x4);
    v0 = lw(gp + 0x920);                                // Load from: gTestY (80077F00)
    v0 = v1 - v0;
    if (i32(v0) >= 0) goto loc_80014628;
    v0 = -v0;
loc_80014628:
    v0 = (i32(v0) < i32(a1));
    if (v0 == 0) goto loc_800146B0;
    v0 = 0x1000000;                                     // Result = 01000000
    if (a2 == a3) goto loc_800146B0;
    v1 = *gTestFlags;
    v0 &= v1;
    {
        const bool bJump = (v0 != 0);
        v0 = 0x10000;                                   // Result = 00010000
        if (bJump) goto loc_800146D4;
    }
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = t0 >> 1;
        if (bJump) goto loc_800146E0;
    }
    a0 = lw(a2 + 0x8);
    v0 = lw(a2 + 0x44);
    v1 = lw(a3 + 0x8);
    v0 += a0;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800146E8;
    }
    v0 = lw(a3 + 0x44);
    v0 += v1;
    v0 = (i32(v0) < i32(a0));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800146E8;
    }
    v1 = lw(a3 + 0x74);
    v0 = lw(a2 + 0x54);
    a0 = lw(v1 + 0x54);
    if (a0 != v0) goto loc_800146C0;
    if (a2 != v1) goto loc_800146B8;
loc_800146B0:
    v0 = 1;                                             // Result = 00000001
    goto loc_800146E8;
loc_800146B8:
    v0 = 0;                                             // Result = 00000000
    if (a0 != 0) goto loc_800146E8;
loc_800146C0:
    v1 = lw(a2 + 0x64);
    v0 = v1 & 4;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 >> 1;
        if (bJump) goto loc_800146E0;
    }
loc_800146D4:
    sw(a2, gp + 0xBA4);                                 // Store to: gpHitThing (80078184)
    v0 = 0;                                             // Result = 00000000
    goto loc_800146E8;
loc_800146E0:
    v0 ^= 1;
    v0 &= 1;
loc_800146E8:
    return;
}

void PB_CheckLines() noexcept {
loc_800146F0:
    v0 = *gBlockmapWidth;
    mult(a1, v0);
    v1 = *gpBlockmap;
    v0 = lo;
    v0 += a0;
    v0 <<= 1;
    v0 += v1;
    v0 = lh(v0);
    v1 = *gpBlockmapLump;
    v0 <<= 1;
    t3 = v0 + v1;
    v0 = -1;                                            // Result = FFFFFFFF
    v1 = lh(t3);
    a0 = lhu(t3);
    sp -= 0x10;
    if (v1 == v0) goto loc_800149A8;
    v1 = *gTestFlags;
    v0 = 0x10000;
    t4 = v1 & v0;
    v1 = a0 << 16;
loc_80014750:
    v1 = u32(i32(v1) >> 16);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v1 = *gpLines;
    v0 <<= 2;
    t1 = v0 + v1;
    v0 = lw(t1 + 0x40);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    if (v0 == v1) goto loc_80014990;
    sw(v1, t1 + 0x40);
    a1 = 0x800B0000;                                    // Result = 800B0000
    a1 = lw(a1 - 0x6F90);                               // Load from: gTestBBox[3] (800A9070)
    v0 = lw(t1 + 0x2C);
    v0 = (i32(v0) < i32(a1));
    a0 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_800148A0;
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x6F94);                               // Load from: gTestBBox[2] (800A906C)
    v0 = lw(t1 + 0x30);
    v0 = (i32(a0) < i32(v0));
    if (v0 == 0) goto loc_80014800;
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x6F9C);                               // Load from: gTestBBox[0] (800A9064)
    v0 = lw(t1 + 0x28);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_80014800;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6F98);                               // Load from: gTestBBox[1] (800A9068)
    v1 = lw(t1 + 0x24);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80014808;
    }
loc_80014800:
    a0 = 0;                                             // Result = 00000000
    goto loc_800148A0;
loc_80014808:
    v1 = lw(t1 + 0x34);
    {
        const bool bJump = (v1 != v0);
        v0 = a1;
        if (bJump) goto loc_80014824;
    }
    v0 = a0;
    t2 = a1;
    goto loc_80014828;
loc_80014824:
    t2 = a0;
loc_80014828:
    a0 = lw(t1);
    v1 = lw(a0);
    a2 = lh(t1 + 0xE);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(a2, v0);
    a1 = lh(t1 + 0xA);
    a3 = lw(a0 + 0x4);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6F9C);                               // Load from: gTestBBox[0] (800A9064)
    a0 = lo;
    v0 -= a3;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a1);
    t0 = lo;
    v1 = t2 - v1;
    v1 = u32(i32(v1) >> 16);
    mult(a2, v1);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x6F98);                               // Load from: gTestBBox[1] (800A9068)
    v1 = lo;
    v0 -= a3;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a1);
    a0 = (i32(a0) < i32(t0));
    v0 = lo;
    v1 = (i32(v1) < i32(v0));
    a0 ^= v1;
    a0 = (a0 > 0);
loc_800148A0:
    if (a0 == 0) goto loc_80014990;
    v0 = lw(t1 + 0x3C);
    if (v0 == 0) goto loc_800148E8;
    if (t4 != 0) goto loc_800148D4;
    v0 = lw(t1 + 0x10);
    v0 &= 3;
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80014980;
    }
loc_800148D4:
    v0 = lw(t1 + 0x10);
    v0 &= 0x800;
    if (v0 == 0) goto loc_800148F0;
loc_800148E8:
    v0 = 0;                                             // Result = 00000000
    goto loc_80014980;
loc_800148F0:
    a2 = lw(t1 + 0x3C);
    a0 = lw(t1 + 0x38);
    v0 = lw(a2 + 0x4);
    v1 = lw(a0 + 0x4);
    a1 = v0;
    v0 = (i32(v1) < i32(a1));
    if (v0 == 0) goto loc_80014914;
    a1 = v1;
loc_80014914:
    a0 = lw(a0);
    v1 = lw(a2);
    v0 = (i32(v1) < i32(a0));
    a2 = a0;
    if (v0 != 0) goto loc_80014934;
    a2 = v1;
    v1 = a0;
loc_80014934:
    v0 = lw(gp + 0xB24);                                // Load from: gTestCeilingz (80078104)
    v0 = (i32(a1) < i32(v0));
    if (v0 == 0) goto loc_80014950;
    sw(a1, gp + 0xB24);                                 // Store to: gTestCeilingz (80078104)
    sw(t1, gp + 0x9AC);                                 // Store to: gpCeilingLine (80077F8C)
loc_80014950:
    v0 = lw(gp + 0x988);                                // Load from: gTestFloorZ (80077F68)
    v0 = (i32(v0) < i32(a2));
    if (v0 == 0) goto loc_80014968;
    sw(a2, gp + 0x988);                                 // Store to: gTestFloorZ (80077F68)
loc_80014968:
    v0 = lw(gp + 0xB40);                                // Load from: gTestDropoffZ (80078120)
    v0 = (i32(v1) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80014980;
    }
    sw(v1, gp + 0xB40);                                 // Store to: gTestDropoffZ (80078120)
loc_80014980:
    t3 += 2;
    if (v0 != 0) goto loc_80014994;
    v0 = 0;                                             // Result = 00000000
    goto loc_800149AC;
loc_80014990:
    t3 += 2;
loc_80014994:
    v0 = -1;                                            // Result = FFFFFFFF
    v1 = lh(t3);
    a0 = lhu(t3);
    {
        const bool bJump = (v1 != v0);
        v1 = a0 << 16;
        if (bJump) goto loc_80014750;
    }
loc_800149A8:
    v0 = 1;                                             // Result = 00000001
loc_800149AC:
    sp += 0x10;
    return;
}

void PB_CheckThings() noexcept {
loc_800149B8:
    v0 = *gBlockmapWidth;
    mult(a1, v0);
    v1 = *gppBlockLinks;
    sp -= 0x18;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lo;
    v0 += a0;
    v0 <<= 2;
    v0 += v1;
    s0 = lw(v0);
    v0 = 1;                                             // Result = 00000001
    if (s0 == 0) goto loc_80014A1C;
loc_800149FC:
    a0 = s0;
    PB_CheckThing();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_80014A1C;
    }
    s0 = lw(s0 + 0x30);
    v0 = 1;                                             // Result = 00000001
    if (s0 != 0) goto loc_800149FC;
loc_80014A1C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}
