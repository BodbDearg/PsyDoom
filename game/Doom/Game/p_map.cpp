#include "p_map.h"

#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_maputl.h"
#include "p_mobj.h"
#include "p_move.h"
#include "p_setup.h"
#include "p_shoot.h"
#include "p_sight.h"
#include "p_spec.h"
#include "p_switch.h"
#include "PsxVm/PsxVm.h"
#include <algorithm>

const VmPtr<VmPtr<mobj_t>>      gpShooter(0x800780B4);          // The map object currently taking a shot
const VmPtr<fixed_t>            gAttackRange(0x80077F98);       // Maximum attack range for an attacker
const VmPtr<angle_t>            gAttackAngle(0x80077F80);       // Angle of attack for an attacker
const VmPtr<fixed_t>            gAimTopSlope(0x80077FF8);       // Maximum Z slope for shooting (defines Z range that stuff can be hit within)
const VmPtr<fixed_t>            gAimBottomSlope(0x800782F8);    // Minimum Z slope for shooting (defines Z range that stuff can be hit within)
const VmPtr<VmPtr<mobj_t>>      gpTryMoveThing(0x8007808C);     // Try move: the thing being moved
const VmPtr<fixed_t>            gTryMoveX(0x80078150);          // Try move: position we're attempting to move to (X)
const VmPtr<fixed_t>            gTryMoveY(0x80078154);          // Try move: position we're attempting to move to (Y)
const VmPtr<bool32_t>           gbCheckPosOnly(0x800780E8);     // Try move: if 'true' then check if the position is valid to move to only, don't actually move there

static const VmPtr<VmPtr<mobj_t>>   gpLineTarget(0x80077EE8);       // The thing being shot at in 'P_AimLineAttack' and 'P_LineAttack'.
static const VmPtr<VmPtr<mobj_t>>   gpBombSource(0x80077EF0);       // Radius attacks: the thing responsible for the explosion (player, monster)
static const VmPtr<VmPtr<mobj_t>>   gpBombSpot(0x800781A0);         // Radius attacks: the object exploding and it's position (barrel, missile etc.)
static const VmPtr<int32_t>         gBombDamage(0x80077E94);        // Radius attacks: how much damage the explosion does before falloff

//------------------------------------------------------------------------------------------------------------------------------------------
// Test if the given x/y position can be moved to for the given map object and return 'true' if the move is allowed
//------------------------------------------------------------------------------------------------------------------------------------------
bool P_CheckPosition(mobj_t& mobj, const fixed_t x, const fixed_t y) noexcept {
    // Save inputs for P_TryMove2
    *gbCheckPosOnly = true;
    *gpTryMoveThing = &mobj;
    *gTryMoveX = x;
    *gTryMoveY = y;

    // Check if the move can be done and return the output result
    P_TryMove2();
    return *gbTryMove2;
}

void P_TryMove() noexcept {
loc_8001B67C:
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    sw(s0, gp + 0xAAC);                                 // Store to: gpTryMoveThing (8007808C)
    sw(a1, gp + 0xB70);                                 // Store to: gTryMoveX (80078150)
    sw(a2, gp + 0xB74);                                 // Store to: gTryMoveY (80078154)
    P_TryMove2();
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7D3C);                               // Load from: gpMoveThing (800782C4)
    v0 = 0x10000;                                       // Result = 00010000
    if (a0 == 0) goto loc_8001B788;
    v1 = lw(s0 + 0x64);
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = 0x1000000;                                 // Result = 01000000
        if (bJump) goto loc_8001B708;
    }
    _thunk_P_Random();
    v1 = lw(s0 + 0x58);
    v0 &= 7;
    v1 = lw(v1 + 0x4C);
    v0++;
    mult(v0, v1);
    a1 = s0;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7D3C);                               // Load from: gpMoveThing (800782C4)
    a2 = lw(a1 + 0x74);
    a3 = lo;
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    goto loc_8001B788;
loc_8001B708:
    v0 &= v1;
    if (v0 == 0) goto loc_8001B780;
    _thunk_P_Random();
    v1 = lw(s0 + 0x58);
    v0 &= 7;
    v1 = lw(v1 + 0x4C);
    v0++;
    mult(v0, v1);
    a1 = s0;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7D3C);                               // Load from: gpMoveThing (800782C4)
    a3 = lo;
    a2 = s0;
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    a0 = 0xFEFF0000;                                    // Result = FEFF0000
    v0 = lw(s0 + 0x64);
    v1 = lw(s0 + 0x58);
    a0 |= 0xFFFF;                                       // Result = FEFFFFFF
    sw(0, s0 + 0x50);
    sw(0, s0 + 0x4C);
    sw(0, s0 + 0x48);
    v0 &= a0;
    sw(v0, s0 + 0x64);
    a1 = lw(v1 + 0x4);
    a0 = s0;
    v0 = P_SetMObjState(*vmAddrToPtr<mobj_t>(a0), (statenum_t) a1);
    goto loc_8001B788;
loc_8001B780:
    a1 = s0;
    P_TouchSpecialThing(*vmAddrToPtr<mobj_t>(a0), *vmAddrToPtr<mobj_t>(a1));
loc_8001B788:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EC4);                               // Load from: gbTryMove2 (8007813C)
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_InterceptVector() noexcept {
    a3 = lh(a1 + 0xE);
    v0 = lh(a0 + 0xA);
    mult(a3, v0);
    t0 = lh(a1 + 0xA);
    v1 = lo;
    v0 = lh(a0 + 0xE);
    mult(t0, v0);
    v0 = lo;
    a2 = v1 - v0;
    v0 = -1;                                            // Result = FFFFFFFF
    if (a2 == 0) goto loc_8001B840;
    v0 = lw(a1);
    v1 = lw(a0);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a3);
    v1 = lw(a0 + 0x4);
    a0 = lw(a1 + 0x4);
    v0 = lo;
    v1 -= a0;
    v1 = u32(i32(v1) >> 16);
    mult(v1, t0);
    v1 = lo;
    v0 += v1;
    v0 <<= 16;
    div(v0, a2);
    if (a2 != 0) goto loc_8001B824;
    _break(0x1C00);
loc_8001B824:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a2 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001B83C;
    }
    if (v0 != at) goto loc_8001B83C;
    tge(zero, zero, 0x5D);
loc_8001B83C:
    v0 = lo;
loc_8001B840:
    return;
}

bool PIT_UseLines(line_t& line) noexcept {
    a0 = ptrToVmAddr(&line);

    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x7898);                               // Load from: gUseBBox[3] (800A8768)
    sp -= 0x30;
    sw(s1, sp + 0x24);
    s1 = a0;
    sw(ra, sp + 0x28);
    sw(s0, sp + 0x20);
    v0 = lw(s1 + 0x2C);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9DC;
    }
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x789C);                               // Load from: gUseBBox[2] (800A8764)
    v1 = lw(s1 + 0x30);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9DC;
    }
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78A4);                               // Load from: gUseBBox[0] (800A875C)
    v0 = lw(s1 + 0x28);
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9DC;
    }
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A0);                               // Load from: gUseBBox[1] (800A8760)
    v1 = lw(s1 + 0x24);
    v0 = (i32(v0) < i32(v1));
    a0 = s1;
    if (v0 == 0) goto loc_8001B9CC;
    a1 = sp + 0x10;
    P_MakeDivline(*vmAddrToPtr<line_t>(a0), *vmAddrToPtr<divline_t>(a1));
    a0 = lh(sp + 0x1E);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lh(v0 - 0x78AE);                               // Load from: gUseLine[2] (800A8752)
    mult(a0, v0);
    a2 = lh(sp + 0x1A);
    v1 = lo;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lh(v0 - 0x78AA);                               // Load from: gUseLine[3] (800A8756)
    mult(a2, v0);
    v0 = lo;
    a1 = v1 - v0;
    s0 = -1;                                            // Result = FFFFFFFF
    if (a1 == 0) goto loc_8001B980;
    v0 = lw(sp + 0x10);
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x78B8);                               // Load from: gUseLine[0] (800A8748)
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a0);
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78B4);                               // Load from: gUseLine[1] (800A874C)
    a0 = lw(sp + 0x14);
    v1 = lo;
    v0 -= a0;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a2);
    v0 = lo;
    v1 += v0;
    v1 <<= 16;
    div(v1, a1);
    if (a1 != 0) goto loc_8001B964;
    _break(0x1C00);
loc_8001B964:
    at = -1;                                            // Result = FFFFFFFF
    {
        const bool bJump = (a1 != at);
        at = 0x80000000;                                // Result = 80000000
        if (bJump) goto loc_8001B97C;
    }
    if (v1 != at) goto loc_8001B97C;
    tge(zero, zero, 0x5D);
loc_8001B97C:
    s0 = lo;
loc_8001B980:
    v0 = 1;                                             // Result = 00000001
    if (i32(s0) < 0) goto loc_8001B9DC;
    v0 = lw(gp + 0xCC8);                                // Load from: gCloseDist (800782A8)
    v0 = (i32(v0) < i32(s0));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9DC;
    }
    v0 = lw(s1 + 0x14);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9D4;
    }
    a0 = s1;
    P_LineOpening(*vmAddrToPtr<line_t>(a0));
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D84);                               // Load from: gOpenRange (8007827C)
    {
        const bool bJump = (i32(v0) <= 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001B9D4;
    }
loc_8001B9CC:
    v0 = 1;                                             // Result = 00000001
    goto loc_8001B9DC;
loc_8001B9D4:
    sw(s1, gp + 0xC94);                                 // Store to: gpCloseLine (80078274)
    sw(s0, gp + 0xCC8);                                 // Store to: gCloseDist (800782A8)
loc_8001B9DC:
    ra = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return (v0 != 0);
}

void P_UseLines() noexcept {
loc_8001B9F4:
    sp -= 0x38;
    sw(s5, sp + 0x2C);
    s5 = a0;
    sw(ra, sp + 0x30);
    sw(s4, sp + 0x28);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
    a0 = lw(s5);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BD0);                               // Load from: gpFineCosine (80077BD0)
    v0 = lw(a0 + 0x24);
    a2 = lw(a0);
    a3 = lw(a0 + 0x4);
    v0 >>= 19;
    v0 <<= 2;
    v1 += v0;
    v1 = lw(v1);
    at = 0x80060000;                                    // Result = 80060000
    at += 0x7958;                                       // Result = FineSine[0] (80067958)
    at += v0;
    a0 = lw(at);
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a2, at - 0x78B8);                                // Store to: gUseLine[0] (800A8748)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x78B4);                                // Store to: gUseLine[1] (800A874C)
    v0 = v1 << 3;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v0 <<= 1;
    a1 = a2 + v0;
    v1 = a1 - a2;
    v0 = a0 << 3;
    v0 += a0;
    v0 <<= 2;
    v0 -= a0;
    v0 <<= 1;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v1, at - 0x78B0);                                // Store to: gUseLine[2] (800A8750)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(v0, at - 0x78AC);                                // Store to: gUseLine[3] (800A8754)
    a0 = a3 + v0;
    if (i32(v1) <= 0) goto loc_8001BAC0;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a1, at - 0x7898);                                // Store to: gUseBBox[3] (800A8768)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a2, at - 0x789C);                                // Store to: gUseBBox[2] (800A8764)
    goto loc_8001BAD0;
loc_8001BAC0:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a2, at - 0x7898);                                // Store to: gUseBBox[3] (800A8768)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a1, at - 0x789C);                                // Store to: gUseBBox[2] (800A8764)
loc_8001BAD0:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78AC);                               // Load from: gUseLine[3] (800A8754)
    if (i32(v0) <= 0) goto loc_8001BAFC;
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a0, at - 0x78A4);                                // Store to: gUseBBox[0] (800A875C)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x78A0);                                // Store to: gUseBBox[1] (800A8760)
    goto loc_8001BB0C;
loc_8001BAFC:
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a3, at - 0x78A4);                                // Store to: gUseBBox[0] (800A875C)
    at = 0x800B0000;                                    // Result = 800B0000
    sw(a0, at - 0x78A0);                                // Store to: gUseBBox[1] (800A8760)
loc_8001BB0C:
    a1 = *gBlockmapOriginY;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x78A0);                               // Load from: gUseBBox[1] (800A8760)
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x78A4);                               // Load from: gUseBBox[0] (800A875C)
    v1 = 0x10000;                                       // Result = 00010000
    sw(v1, gp + 0xCC8);                                 // Store to: gCloseDist (800782A8)
    v1 = 0x800B0000;                                    // Result = 800B0000
    v1 = lw(v1 - 0x7898);                               // Load from: gUseBBox[3] (800A8768)
    sw(0, gp + 0xC94);                                  // Store to: gpCloseLine (80078274)
    v0 -= a1;
    s1 = u32(i32(v0) >> 23);
    a0 -= a1;
    s3 = u32(i32(a0) >> 23);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    a1 = *gBlockmapOriginX;
    v0++;
    v1 -= a1;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 = lw(v0 - 0x789C);                               // Load from: gUseBBox[2] (800A8764)
    v0 -= a1;
    s4 = u32(i32(v0) >> 23);
    v0 = (i32(s3) < i32(s1));
    s2 = u32(i32(v1) >> 23);
    if (v0 != 0) goto loc_8001BBC8;
    v0 = (i32(s2) < i32(s4));
loc_8001BB8C:
    s0 = s4;
    if (v0 != 0) goto loc_8001BBB8;
    a0 = s0;
loc_8001BB98:
    a2 = 0x80020000;                                    // Result = 80020000
    a2 -= 0x47B8;                                       // Result = PIT_UseLines (8001B848)
    a1 = s1;
    v0 = P_BlockLinesIterator(a0, a1, PIT_UseLines);
    s0++;
    v0 = (i32(s2) < i32(s0));
    a0 = s0;
    if (v0 == 0) goto loc_8001BB98;
loc_8001BBB8:
    s1++;
    v0 = (i32(s3) < i32(s1));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s2) < i32(s4));
        if (bJump) goto loc_8001BB8C;
    }
loc_8001BBC8:
    a1 = lw(gp + 0xC94);                                // Load from: gpCloseLine (80078274)
    if (a1 == 0) goto loc_8001BC08;
    v0 = lw(a1 + 0x14);
    if (v0 != 0) goto loc_8001BBFC;
    a0 = lw(s5);
    a1 = sfx_noway;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    goto loc_8001BC08;
loc_8001BBFC:
    a0 = lw(s5);
    P_UseSpecialLine();
loc_8001BC08:
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Apply splash/bomb damage to the given thing from the current explosion, if applicable and in range etc.
// Note: the 'bomb source' is the thing that caused the explosion, not the projectile itself ('bomb spot').
//------------------------------------------------------------------------------------------------------------------------------------------
bool PIT_RadiusAttack(mobj_t& mobj) noexcept {    
    // Non shootable things get no splash damage
    if ((mobj.flags & MF_SHOOTABLE) == 0)
        return true;

    // Cyberdemons and Masterminds don't get splash damage
    if ((mobj.type == MT_CYBORG) || (mobj.type == MT_SPIDER))
        return true;

    // Get a distance estimate to the source of the blast
    mobj_t& bombSpot = *gpBombSpot->get();
    
    const fixed_t dx = std::abs(mobj.x - bombSpot.x);
    const fixed_t dy = std::abs(mobj.y - bombSpot.y);
    const int32_t approxDist = std::max(dx, dy);

    // Compute how much to fade out damage based on the approx distance
    const int32_t damageFade = std::max((approxDist - mobj.radius) >> FRACBITS, 0);

    // Apply the actual damage if > 0 and if the thing has a line of sight to the explosion
    const int32_t bombBaseDamage = *gBombDamage;
    mobj_t* pBombSource = gpBombSource->get();

    if ((bombBaseDamage > damageFade) && P_CheckSight(mobj, bombSpot)) {
        P_DamageMObj(mobj, &bombSpot, pBombSource, bombBaseDamage - damageFade);
    }

    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Do an explosion at the given bomb spot, doing the specified amount of damage. The damage falls off linearly over distance.
// The given 'source' object (optional) is the thing responsible for causing the explosion.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_RadiusAttack(mobj_t& bombSpot, mobj_t* const pSource, const int32_t damage) noexcept {    
    // Compute the range of the blockmap to search based on the damage amount.
    // Splash damage falls off linearly, so the damage amount is also pretty much the distance range:
    const fixed_t blastDist = damage << FRACBITS;

    #if PC_PSX_DOOM_MODS
        // PC-PSX: clamp these coords to the valid range of the blockmap to avoid potential undefined behavior near map edges
        const int32_t bmapLx = std::max((bombSpot.x - blastDist - *gBlockmapOriginX) >> MAPBLOCKSHIFT, 0);
        const int32_t bmapRx = std::min((bombSpot.x + blastDist - *gBlockmapOriginX) >> MAPBLOCKSHIFT, *gBlockmapWidth - 1);
        const int32_t bmapBy = std::max((bombSpot.y - blastDist - *gBlockmapOriginY) >> MAPBLOCKSHIFT, 0);
        const int32_t bmapTy = std::min((bombSpot.y + blastDist - *gBlockmapOriginY) >> MAPBLOCKSHIFT, *gBlockmapHeight - 1);
    #else
        const int32_t bmapLx = (bombSpot.x - blastDist - *gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t bmapRx = (bombSpot.x + blastDist - *gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t bmapBy = (bombSpot.y - blastDist - *gBlockmapOriginY) >> MAPBLOCKSHIFT;
        const int32_t bmapTy = (bombSpot.y + blastDist - *gBlockmapOriginY) >> MAPBLOCKSHIFT;
    #endif

    // Save bomb properties globally and apply the blast damage (where possible) to things within the blockmap search range
    *gpBombSpot = &bombSpot;
    *gpBombSource = pSource;
    *gBombDamage = damage;

    for (int32_t y = bmapBy; y <= bmapTy; ++y) {
        for (int32_t x = bmapLx; x <= bmapRx; ++x) {
            P_BlockThingsIterator(x, y, PIT_RadiusAttack);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Figures out the vertical slope (up/down auto-aiming) for a shot to be taken by the given shooter.
// Returns the slope to use for the shot.
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_AimLineAttack(mobj_t& shooter, const angle_t angle, const fixed_t maxDist) noexcept {
    // Can't shoot outside view angles: set the allowed Z slope range for the shot
    *gAimTopSlope = (100 * FRACUNIT) / 160;
    *gAimBottomSlope = (-100 * FRACUNIT) / 160;

    // Setup for shot simulation and see what this shot would hit
    *gValidCount += 1;
    *gAttackAngle = angle;
    *gAttackRange = maxDist;
    *gpShooter = &shooter;
    P_Shoot2();

    // Save what thing is being targeted and return the computed slope.
    // If we hit a thing then use the slope to the thing, otherwise shoot level ahead (slope 0).
    *gpLineTarget = gpShootMObj->get();
    return (gpShootMObj->get()) ? *gShootSlope : 0;
}

void P_LineAttack() noexcept {
loc_8001BE78:
    sp -= 0x30;
    sw(s4, sp + 0x20);
    s4 = a0;
    v0 = 0x7FFF0000;                                    // Result = 7FFF0000
    sw(s5, sp + 0x24);
    s5 = lw(sp + 0x40);
    v0 |= 0xFFFF;                                       // Result = 7FFFFFFF
    sw(ra, sp + 0x28);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    sw(s4, gp + 0xAD4);                                 // Store to: gpShooter (800780B4)
    sw(a2, gp + 0x9B8);                                 // Store to: gAttackRange (80077F98)
    sw(a1, gp + 0x9A0);                                 // Store to: gAttackAngle (80077F80)
    {
        const bool bJump = (a3 != v0);
        v0 = a3 + 1;
        if (bJump) goto loc_8001BED8;
    }
    v1 = 0xFFFF0000;                                    // Result = FFFF0000
    v1 |= 0x6000;                                       // Result = FFFF6000
    v0 = 0xA000;                                        // Result = 0000A000
    sw(v0, gp + 0xA18);                                 // Store to: gAimTopSlope (80077FF8)
    sw(v1, gp + 0xD18);                                 // Store to: gAimBottomSlope (800782F8)
    goto loc_8001BEE4;
loc_8001BED8:
    sw(v0, gp + 0xA18);                                 // Store to: gAimTopSlope (80077FF8)
    v0 = a3 - 1;
    sw(v0, gp + 0xD18);                                 // Store to: gAimBottomSlope (800782F8)
loc_8001BEE4:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    P_Shoot2();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7D2C);                               // Load from: gpShootMObj (800782D4)
    s0 = 0x80080000;                                    // Result = 80080000
    s0 = lw(s0 - 0x7D30);                               // Load from: gpShootLine (800782D0)
    s2 = 0x80070000;                                    // Result = 80070000
    s2 = lw(s2 + 0x7FC4);                               // Load from: gShootX (80077FC4)
    s3 = 0x80070000;                                    // Result = 80070000
    s3 = lw(s3 + 0x7FD0);                               // Load from: gShootY (80077FD0)
    s1 = 0x80070000;                                    // Result = 80070000
    s1 = lw(s1 + 0x7FD4);                               // Load from: gShootZ (80077FD4)
    sw(v0, gp + 0x908);                                 // Store to: gpLineTarget (80077EE8)
    v1 = 0x80000;                                       // Result = 00080000
    if (v0 == 0) goto loc_8001BF8C;
    v0 = lw(v0 + 0x64);
    v0 &= v1;
    a0 = s2;
    if (v0 == 0) goto loc_8001BF60;
    a1 = s3;
    a2 = s1;
    P_SpawnPuff(a0, a1, a2);
    goto loc_8001BF70;
loc_8001BF60:
    a1 = s3;
    a2 = s1;
    a3 = s5;
    P_SpawnBlood(a0, a1, a2, a3);
loc_8001BF70:
    a0 = lw(gp + 0x908);                                // Load from: gpLineTarget (80077EE8)
    a1 = s4;
    a2 = a1;
    a3 = s5;
    P_DamageMObj(*vmAddrToPtr<mobj_t>(a0), vmAddrToPtr<mobj_t>(a1), vmAddrToPtr<mobj_t>(a2), a3);
    goto loc_8001C008;
loc_8001BF8C:
    if (s0 == 0) goto loc_8001C008;
    v0 = lw(s0 + 0x14);
    a0 = s4;
    if (v0 == 0) goto loc_8001BFAC;
    a1 = s0;
    P_ShootSpecialLine();
loc_8001BFAC:
    v1 = lw(s0 + 0x38);
    a0 = lw(v1 + 0xC);
    v0 = -1;                                            // Result = FFFFFFFF
    if (a0 != v0) goto loc_8001BFF8;
    v0 = lw(v1 + 0x4);
    v0 = (i32(v0) < i32(s1));
    if (v0 != 0) goto loc_8001C008;
    a1 = lw(s0 + 0x3C);
    if (a1 == 0) goto loc_8001BFF8;
    v0 = lw(a1 + 0xC);
    if (v0 == a0) goto loc_8001C008;
loc_8001BFF8:
    a0 = s2;
    a1 = s3;
    a2 = s1;
    P_SpawnPuff(a0, a1, a2);
loc_8001C008:
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
