#include "p_move.h"

#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "p_local.h"
#include "p_maputl.h"
#include "p_setup.h"
#include "p_spec.h"
#include "PsxVm/PsxVm.h"

void P_TryMove2() noexcept {
loc_8001E4F4:
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F74);                               // Load from: gpTryMoveThing (8007808C)
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    v1 = lw(v0);
    v0 = lw(v0 + 0x4);
    sw(0, gp + 0xB5C);                                  // Store to: gbTryMove2 (8007813C)
    sw(0, gp + 0xA9C);                                  // Store to: gbFloatOk (8007807C)
    sw(v1, gp + 0xC60);                                 // Store to: gOldX (80078240)
    sw(v0, gp + 0xC64);                                 // Store to: gOldY (80078244)
    PM_CheckPosition();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F18);                               // Load from: gbCheckPosOnly (800780E8)
    if (v0 == 0) goto loc_8001E554;
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7F18);                                 // Store to: gbCheckPosOnly (800780E8)
    goto loc_8001E704;
loc_8001E554:
    v0 = lw(gp + 0xB5C);                                // Load from: gbTryMove2 (8007813C)
    if (v0 == 0) goto loc_8001E704;
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7F74);                               // Load from: gpTryMoveThing (8007808C)
    v0 = lw(a0 + 0x64);
    v0 &= 0x1000;
    if (v0 != 0) goto loc_8001E608;
    a3 = lw(gp + 0x924);                                // Load from: gTmCeilingZ (80077F04)
    a2 = lw(gp + 0xC08);                                // Load from: gTmFloorZ (800781E8)
    v1 = lw(a0 + 0x44);
    sw(0, gp + 0xB5C);                                  // Store to: gbTryMove2 (8007813C)
    v0 = a3 - a2;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001E704;
    }
    t0 = lw(a0 + 0x64);
    sw(v0, gp + 0xA9C);                                 // Store to: gbFloatOk (8007807C)
    v0 = t0 & 0x8000;
    {
        const bool bJump = (v0 != 0);
        v0 = t0 & 0x4400;
        if (bJump) goto loc_8001E5E0;
    }
    a1 = lw(a0 + 0x8);
    v1 = lw(a0 + 0x44);
    v0 = a3 - a1;
    v0 = (i32(v0) < i32(v1));
    v1 = a2 - a1;
    if (v0 != 0) goto loc_8001E704;
    v0 = 0x180000;                                      // Result = 00180000
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = t0 & 0x4400;
        if (bJump) goto loc_8001E704;
    }
loc_8001E5E0:
    {
        const bool bJump = (v0 != 0);
        v0 = 0x180000;                                  // Result = 00180000
        if (bJump) goto loc_8001E600;
    }
    v1 = lw(gp + 0x95C);                                // Load from: gTmDropoffZ (80077F3C)
    v1 = a2 - v1;
    v0 = (i32(v0) < i32(v1));
    if (v0 != 0) goto loc_8001E704;
loc_8001E600:
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7F74);                               // Load from: gpTryMoveThing (8007808C)
loc_8001E608:
    P_UnsetThingPosition2(*vmAddrToPtr<mobj_t>(a0));

    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7F74);                               // Load from: gpTryMoveThing (8007808C)
    v0 = lw(gp + 0xC08);                                // Load from: gTmFloorZ (800781E8)
    v1 = lw(gp + 0x924);                                // Load from: gTmCeilingZ (80077F04)
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7EB0);                               // Load from: gTryMoveX (80078150)
    a2 = 0x80080000;                                    // Result = 80080000
    a2 = lw(a2 - 0x7EAC);                               // Load from: gTryMoveY (80078154)
    sw(v0, a0 + 0x38);
    sw(v1, a0 + 0x3C);
    sw(a1, a0);
    sw(a2, a0 + 0x4);
    P_SetThingPosition2();
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7F74);                               // Load from: gpTryMoveThing (8007808C)
    v0 = lw(v1 + 0x80);
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001E700;
    }
    v0 = lw(v1 + 0x64);
    v0 &= 0x9000;
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001E700;
    }
    v0 = lw(gp + 0xAE0);                                // Load from: gNumCrossCheckLines (800780C0)
    v1 = -1;                                            // Result = FFFFFFFF
    v0--;
    sw(v0, gp + 0xAE0);                                 // Store to: gNumCrossCheckLines (800780C0)
    {
        const bool bJump = (v0 == v1);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001E700;
    }
    s2 = 0x800B0000;                                    // Result = 800B0000
    s2 -= 0x70D8;                                       // Result = gpCrossCheckLines[0] (800A8F28)
loc_8001E694:
    v1 = lw(gp + 0xAE0);                                // Load from: gNumCrossCheckLines (800780C0)
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7F74);                               // Load from: gpTryMoveThing (8007808C)
    v1 <<= 2;
    v1 += s2;
    a0 = lw(v0);
    s1 = lw(v1);
    a1 = lw(v0 + 0x4);
    a2 = s1;
    P_PointOnLineSide();
    a2 = s1;
    a0 = lw(gp + 0xC60);                                // Load from: gOldX (80078240)
    a1 = lw(gp + 0xC64);                                // Load from: gOldY (80078244)
    s0 = v0;
    P_PointOnLineSide();
    if (s0 == v0) goto loc_8001E6E8;
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7F74);                               // Load from: gpTryMoveThing (8007808C)
    a0 = s1;
    P_CrossSpecialLine();
loc_8001E6E8:
    v1 = lw(gp + 0xAE0);                                // Load from: gNumCrossCheckLines (800780C0)
    v0 = -1;                                            // Result = FFFFFFFF
    v1--;
    sw(v1, gp + 0xAE0);                                 // Store to: gNumCrossCheckLines (800780C0)
    {
        const bool bJump = (v1 != v0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001E694;
    }
loc_8001E700:
    sw(v0, gp + 0xB5C);                                 // Store to: gbTryMove2 (8007813C)
loc_8001E704:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void UNKNOWN_DoomFunc3() noexcept {
    v1 = lw(a2);
    v0 = lw(v1);
    a0 -= v0;
    v0 = lh(a2 + 0xE);
    a0 = u32(i32(a0) >> 16);
    mult(v0, a0);
    v0 = lw(v1 + 0x4);
    a1 -= v0;
    v1 = lo;
    v0 = lh(a2 + 0xA);
    a1 = u32(i32(a1) >> 16);
    mult(a1, v0);
    v0 = lo;
    v0 = (i32(v0) < i32(v1));
    v0 ^= 1;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unlinks the given thing from sector thing lists and the blockmap.
// Very similar to 'P_UnsetThingPosition' except the thing is always unlinked from sectors.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_UnsetThingPosition2(mobj_t& thing) noexcept {
    // Remove the thing from sector thing lists
    if (thing.snext) {
        thing.snext->sprev = thing.sprev;
    }

    if (thing.sprev) {
        thing.sprev->snext = thing.snext;
    } else {
        thing.subsector->sector->thinglist = thing.snext;
    }

    // Does this thing get added to the blockmap?
    // If so remove it from the blockmap.
    if ((thing.flags & MF_NOBLOCKMAP) == 0) {
        if (thing.bnext) {
            thing.bnext->bprev = thing.bprev;
        }
        
        if (thing.bprev) {
            thing.bprev->bnext = thing.bnext;
        } else {
            const int32_t blockx = (thing.x - *gBlockmapOriginX) >> MAPBLOCKSHIFT;
            const int32_t blocky = (thing.y - *gBlockmapOriginY) >> MAPBLOCKSHIFT;
            (*gppBlockLinks)[blocky * (*gBlockmapWidth) + blockx] = thing.bnext;
        }
    }
}

void P_SetThingPosition2() noexcept {
loc_8001E868:
    a1 = a0;
    v0 = lw(a1 + 0x64);
    v1 = lw(gp + 0xCDC);                                // Load from: gpNewSubsec (800782BC)
    v0 &= 8;
    sw(v1, a1 + 0xC);
    if (v0 != 0) goto loc_8001E8AC;
    v1 = lw(v1);
    sw(0, a1 + 0x20);
    v0 = lw(v1 + 0x4C);
    sw(v0, a1 + 0x1C);
    v0 = lw(v1 + 0x4C);
    if (v0 == 0) goto loc_8001E8A8;
    sw(a1, v0 + 0x20);
loc_8001E8A8:
    sw(a1, v1 + 0x4C);
loc_8001E8AC:
    v0 = lw(a1 + 0x64);
    v0 &= 0x10;
    if (v0 != 0) goto loc_8001E970;
    v1 = lw(a1);
    v0 = *gBlockmapOriginX;
    a0 = *gBlockmapOriginY;
    v1 -= v0;
    v0 = lw(a1 + 0x4);
    v1 = u32(i32(v1) >> 23);
    v0 -= a0;
    a0 = u32(i32(v0) >> 23);
    if (i32(v1) < 0) goto loc_8001E968;
    a2 = *gBlockmapWidth;
    v0 = (i32(v1) < i32(a2));
    if (v0 == 0) goto loc_8001E968;
    if (i32(a0) < 0) goto loc_8001E968;
    v0 = *gBlockmapHeight;
    v0 = (i32(a0) < i32(v0));
    mult(a0, a2);
    if (v0 == 0) goto loc_8001E968;
    sw(0, a1 + 0x34);
    v0 = lo;
    v0 += v1;
    v1 = *gppBlockLinks;
    v0 <<= 2;
    v1 += v0;
    v0 = lw(v1);
    sw(v0, a1 + 0x30);
    v0 = lw(v1);
    if (v0 == 0) goto loc_8001E960;
    sw(a1, v0 + 0x34);
loc_8001E960:
    sw(a1, v1);
    goto loc_8001E970;
loc_8001E968:
    sw(0, a1 + 0x34);
    sw(0, a1 + 0x30);
loc_8001E970:
    return;
}

void PM_CheckPosition() noexcept {
loc_8001E978:
    v1 = 0x80080000;                                    // Result = 80080000
    v1 = lw(v1 - 0x7F74);                               // Load from: gpTryMoveThing (8007808C)
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7EAC);                               // Load from: gTryMoveY (80078154)
    sp -= 0x28;
    sw(s0, sp + 0x10);
    s0 = 0x80090000;                                    // Result = 80090000
    s0 += 0x7C10;                                       // Result = gtTmbBox[0] (80097C10)
    sw(ra, sp + 0x24);
    sw(s4, sp + 0x20);
    sw(s3, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    v0 = lw(v1 + 0x40);
    a2 = lw(v1 + 0x64);
    v0 += a1;
    sw(v0, s0);                                         // Store to: gtTmbBox[0] (80097C10)
    v0 = lw(v1 + 0x40);
    a0 = 0x80080000;                                    // Result = 80080000
    a0 = lw(a0 - 0x7EB0);                               // Load from: gTryMoveX (80078150)
    v0 = a1 - v0;
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7C14);                                // Store to: gtTmbBox[1] (80097C14)
    v0 = lw(v1 + 0x40);
    v0 += a0;
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7C1C);                                // Store to: gtTmbBox[3] (80097C1C)
    v0 = lw(v1 + 0x40);
    sw(a2, gp + 0xA98);                                 // Store to: gTmFlags (80078078)
    v0 = a0 - v0;
    at = 0x80090000;                                    // Result = 80090000
    sw(v0, at + 0x7C18);                                // Store to: gtTmbBox[2] (80097C18)
    _thunk_R_PointInSubsector();
    v1 = lw(v0);
    sw(v0, gp + 0xCDC);                                 // Store to: gpNewSubsec (800782BC)
    v0 = lw(v0);
    sw(0, gp + 0xAE0);                                  // Store to: gNumCrossCheckLines (800780C0)
    sw(0, gp + 0xCE4);                                  // Store to: gpMoveThing (800782C4)
    sw(0, gp + 0xC68);                                  // Store to: gpBlockLine (80078248)
    a0 = lw(v1);
    a1 = lw(v0 + 0x4);
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    v1 = lw(gp + 0xA98);                                // Load from: gTmFlags (80078078)
    v0++;
    v1 &= 0x1000;
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    sw(a0, gp + 0x95C);                                 // Store to: gTmDropoffZ (80077F3C)
    sw(a0, gp + 0xC08);                                 // Store to: gTmFloorZ (800781E8)
    sw(a1, gp + 0x924);                                 // Store to: gTmCeilingZ (80077F04)
    {
        const bool bJump = (v1 == 0);
        v1 = 0xFFE00000;                                // Result = FFE00000
        if (bJump) goto loc_8001EA68;
    }
    v0 = 1;                                             // Result = 00000001
    goto loc_8001EC40;
loc_8001EA5C:
    sw(0, gp + 0xB5C);                                  // Store to: gbTryMove2 (8007813C)
    goto loc_8001EC44;
loc_8001EA68:
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7C18);                               // Load from: gtTmbBox[2] (80097C18)
    a1 = *gBlockmapOriginX;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x7C1C);                               // Load from: gtTmbBox[3] (80097C1C)
    a2 = *gBlockmapOriginY;
    v0 -= a1;
    v0 += v1;
    a3 = u32(i32(v0) >> 23);
    a0 -= a1;
    a1 = 0x200000;                                      // Result = 00200000
    a0 += a1;
    s3 = u32(i32(a0) >> 23);
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7C14);                               // Load from: gtTmbBox[1] (80097C14)
    v0 = lw(s0);                                        // Load from: gtTmbBox[0] (80097C10)
    v1 -= a2;
    v1 -= a1;
    s4 = u32(i32(v1) >> 23);
    v0 -= a2;
    v0 += a1;
    s2 = u32(i32(v0) >> 23);
    if (i32(a3) >= 0) goto loc_8001EAD0;
    a3 = 0;                                             // Result = 00000000
loc_8001EAD0:
    if (i32(s4) >= 0) goto loc_8001EADC;
    s4 = 0;                                             // Result = 00000000
loc_8001EADC:
    v1 = *gBlockmapWidth;
    v0 = (i32(s3) < i32(v1));
    if (v0 != 0) goto loc_8001EAF8;
    s3 = v1 - 1;
loc_8001EAF8:
    v1 = *gBlockmapHeight;
    v0 = (i32(s2) < i32(v1));
    s1 = a3;
    if (v0 != 0) goto loc_8001EB14;
    s2 = v1 - 1;
loc_8001EB14:
    v0 = (i32(s3) < i32(s1));
    if (v0 != 0) goto loc_8001EB5C;
    v0 = (i32(s2) < i32(s4));
loc_8001EB24:
    s0 = s4;
    if (v0 != 0) goto loc_8001EB4C;
    a0 = s1;
loc_8001EB30:
    a1 = s0;
    PM_CheckThings();
    s0++;
    if (v0 == 0) goto loc_8001EA5C;
    v0 = (i32(s2) < i32(s0));
    a0 = s1;
    if (v0 == 0) goto loc_8001EB30;
loc_8001EB4C:
    s1++;
    v0 = (i32(s3) < i32(s1));
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(s2) < i32(s4));
        if (bJump) goto loc_8001EB24;
    }
loc_8001EB5C:
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7C18);                               // Load from: gtTmbBox[2] (80097C18)
    a0 = *gBlockmapOriginX;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7C1C);                               // Load from: gtTmbBox[3] (80097C1C)
    a1 = *gBlockmapOriginY;
    v0 -= a0;
    a3 = u32(i32(v0) >> 23);
    v1 -= a0;
    s3 = u32(i32(v1) >> 23);
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x7C14);                               // Load from: gtTmbBox[1] (80097C14)
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x7C10);                               // Load from: gtTmbBox[0] (80097C10)
    a0 -= a1;
    s4 = u32(i32(a0) >> 23);
    v0 -= a1;
    s2 = u32(i32(v0) >> 23);
    if (i32(a3) >= 0) goto loc_8001EBB4;
    a3 = 0;                                             // Result = 00000000
loc_8001EBB4:
    if (i32(s4) >= 0) goto loc_8001EBC0;
    s4 = 0;                                             // Result = 00000000
loc_8001EBC0:
    v1 = *gBlockmapWidth;
    v0 = (i32(s3) < i32(v1));
    if (v0 != 0) goto loc_8001EBDC;
    s3 = v1 - 1;
loc_8001EBDC:
    v1 = *gBlockmapHeight;
    v0 = (i32(s2) < i32(v1));
    s1 = a3;
    if (v0 != 0) goto loc_8001EBF8;
    s2 = v1 - 1;
loc_8001EBF8:
    v0 = (i32(s3) < i32(s1));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001EC40;
    }
loc_8001EC04:
    v0 = (i32(s2) < i32(s4));
    s0 = s4;
    if (v0 != 0) goto loc_8001EC30;
    a0 = s1;
loc_8001EC14:
    a1 = s0;
    PM_CheckLines();
    s0++;
    if (v0 == 0) goto loc_8001EA5C;
    v0 = (i32(s2) < i32(s0));
    a0 = s1;
    if (v0 == 0) goto loc_8001EC14;
loc_8001EC30:
    s1++;
    v0 = (i32(s3) < i32(s1));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001EC04;
    }
loc_8001EC40:
    sw(v0, gp + 0xB5C);                                 // Store to: gbTryMove2 (8007813C)
loc_8001EC44:
    ra = lw(sp + 0x24);
    s4 = lw(sp + 0x20);
    s3 = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x28;
    return;
}

void PM_BoxCrossLine() noexcept {
    a2 = a0;
    a1 = 0x80090000;                                    // Result = 80090000
    a1 = lw(a1 + 0x7C1C);                               // Load from: gtTmbBox[3] (80097C1C)
    v0 = lw(a2 + 0x2C);
    v0 = (i32(v0) < i32(a1));
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001ED6C;
    }
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x7C18);                               // Load from: gtTmbBox[2] (80097C18)
    v0 = lw(a2 + 0x30);
    v0 = (i32(a0) < i32(v0));
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001ED6C;
    }
    a3 = 0x80090000;                                    // Result = 80090000
    a3 = lw(a3 + 0x7C10);                               // Load from: gtTmbBox[0] (80097C10)
    v0 = lw(a2 + 0x28);
    v0 = (i32(v0) < i32(a3));
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001ED6C;
    }
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7C14);                               // Load from: gtTmbBox[1] (80097C14)
    v0 = lw(a2 + 0x24);
    v0 = (i32(v1) < i32(v0));
    t2 = v1;
    if (v0 != 0) goto loc_8001ECE4;
    v0 = 0;                                             // Result = 00000000
    goto loc_8001ED6C;
loc_8001ECE4:
    v1 = lw(a2 + 0x34);
    v0 = 2;                                             // Result = 00000002
    t0 = a3;
    if (v1 != v0) goto loc_8001ED00;
    v0 = a0;
    t1 = a1;
    goto loc_8001ED08;
loc_8001ED00:
    v0 = a1;
    t1 = a0;
loc_8001ED08:
    v1 = lw(a2);
    a0 = lw(v1);
    a3 = lh(a2 + 0xE);
    v0 -= a0;
    v0 = u32(i32(v0) >> 16);
    mult(a3, v0);
    a2 = lh(a2 + 0xA);
    v1 = lw(v1 + 0x4);
    a1 = lo;
    v0 = t0 - v1;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a2);
    t0 = lo;
    a0 = t1 - a0;
    a0 = u32(i32(a0) >> 16);
    mult(a3, a0);
    v0 = lo;
    v1 = t2 - v1;
    v1 = u32(i32(v1) >> 16);
    mult(v1, a2);
    a1 = (i32(a1) < i32(t0));
    v1 = lo;
    v0 = (i32(v0) < i32(v1));
    v0 ^= a1;
loc_8001ED6C:
    return;
}

void PIT_CheckLine() noexcept {
loc_8001ED74:
    v0 = lw(a0 + 0x3C);
    v1 = 0x10000;                                       // Result = 00010000
    if (v0 == 0) goto loc_8001EE04;
    a1 = 0x80080000;                                    // Result = 80080000
    a1 = lw(a1 - 0x7F74);                               // Load from: gpTryMoveThing (8007808C)
    v0 = lw(a1 + 0x64);
    v0 &= v1;
    if (v0 != 0) goto loc_8001EDD0;
    v1 = lw(a0 + 0x10);
    v0 = v1 & 1;
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001EEBC;
    }
    v0 = lw(a1 + 0x80);
    {
        const bool bJump = (v0 != 0);
        v0 = v1 & 2;
        if (bJump) goto loc_8001EDD0;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001EEBC;
    }
loc_8001EDD0:
    v0 = lw(a0 + 0x38);
    a1 = lw(v0 + 0x4);
    t0 = lw(v0);
    v0 = lw(a0 + 0x3C);
    if (a1 == t0) goto loc_8001EE00;
    v1 = lw(v0 + 0x4);
    a2 = lw(v0);
    a3 = v1;
    if (v1 != a2) goto loc_8001EE0C;
loc_8001EE00:
    sw(a0, gp + 0xC68);                                 // Store to: gpBlockLine (80078248)
loc_8001EE04:
    v0 = 0;                                             // Result = 00000000
    goto loc_8001EEBC;
loc_8001EE0C:
    v0 = (i32(a1) < i32(a3));
    v1 = t0;
    if (v0 == 0) goto loc_8001EE1C;
    a3 = a1;
loc_8001EE1C:
    a1 = a2;
    v0 = (i32(a1) < i32(v1));
    if (v0 != 0) goto loc_8001EE34;
    v1 = a2;
    a1 = t0;
loc_8001EE34:
    v0 = lw(gp + 0x924);                                // Load from: gTmCeilingZ (80077F04)
    v0 = (i32(a3) < i32(v0));
    if (v0 == 0) goto loc_8001EE4C;
    sw(a3, gp + 0x924);                                 // Store to: gTmCeilingZ (80077F04)
loc_8001EE4C:
    v0 = lw(gp + 0xC08);                                // Load from: gTmFloorZ (800781E8)
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001EE64;
    sw(v1, gp + 0xC08);                                 // Store to: gTmFloorZ (800781E8)
loc_8001EE64:
    v0 = lw(gp + 0x95C);                                // Load from: gTmDropoffZ (80077F3C)
    v0 = (i32(a1) < i32(v0));
    if (v0 == 0) goto loc_8001EE7C;
    sw(a1, gp + 0x95C);                                 // Store to: gTmDropoffZ (80077F3C)
loc_8001EE7C:
    v0 = lw(a0 + 0x14);
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001EEBC;
    }
    v1 = lw(gp + 0xAE0);                                // Load from: gNumCrossCheckLines (800780C0)
    v0 = (i32(v1) < 8);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_8001EEB8;
    }
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x70D8;                                       // Result = gpCrossCheckLines[0] (800A8F28)
    at += v0;
    sw(a0, at);
    v0 = v1 + 1;
    sw(v0, gp + 0xAE0);                                 // Store to: gNumCrossCheckLines (800780C0)
loc_8001EEB8:
    v0 = 1;                                             // Result = 00000001
loc_8001EEBC:
    return;
}

void PIT_CheckThing() noexcept {
loc_8001EEC4:
    a2 = a0;
    t0 = lw(a2 + 0x64);
    v0 = t0 & 7;
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001F020;
    }
    a3 = 0x80080000;                                    // Result = 80080000
    a3 = lw(a3 - 0x7F74);                               // Load from: gpTryMoveThing (8007808C)
    a1 = lw(a2 + 0x40);
    a0 = lw(a2);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EB0);                               // Load from: gTryMoveX (80078150)
    v1 = lw(a3 + 0x40);
    v0 = a0 - v0;
    a1 += v1;
    if (i32(v0) >= 0) goto loc_8001EF08;
    v0 = -v0;
loc_8001EF08:
    v0 = (i32(v0) < i32(a1));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001F020;
    }
    v1 = lw(a2 + 0x4);
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = lw(v0 - 0x7EAC);                               // Load from: gTryMoveY (80078154)
    v0 = v1 - v0;
    if (i32(v0) >= 0) goto loc_8001EF34;
    v0 = -v0;
loc_8001EF34:
    v0 = (i32(v0) < i32(a1));
    if (v0 == 0) goto loc_8001F004;
    v0 = 0x1000000;                                     // Result = 01000000
    if (a2 == a3) goto loc_8001F004;
    v1 = lw(a3 + 0x64);
    v0 &= v1;
    {
        const bool bJump = (v0 != 0);
        v0 = 0x10000;                                   // Result = 00010000
        if (bJump) goto loc_8001EFD8;
    }
    v0 &= v1;
    {
        const bool bJump = (v0 == 0);
        v0 = t0 & 1;
        if (bJump) goto loc_8001EFE4;
    }
    a0 = lw(a2 + 0x8);
    v0 = lw(a2 + 0x44);
    v1 = lw(a3 + 0x8);
    v0 += a0;
    v0 = (i32(v0) < i32(v1));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001F020;
    }
    v0 = lw(a3 + 0x44);
    v0 += v1;
    v0 = (i32(v0) < i32(a0));
    {
        const bool bJump = (v0 != 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001F020;
    }
    v1 = lw(a3 + 0x74);
    v0 = lw(a2 + 0x54);
    a0 = lw(v1 + 0x54);
    if (a0 != v0) goto loc_8001EFC4;
    if (a2 == v1) goto loc_8001F004;
    v0 = 0;                                             // Result = 00000000
    if (a0 != 0) goto loc_8001F020;
loc_8001EFC4:
    v1 = lw(a2 + 0x64);
    v0 = v1 & 4;
    {
        const bool bJump = (v0 == 0);
        v0 = v1 >> 1;
        if (bJump) goto loc_8001F018;
    }
loc_8001EFD8:
    sw(a2, gp + 0xCE4);                                 // Store to: gpMoveThing (800782C4)
    v0 = 0;                                             // Result = 00000000
    goto loc_8001F020;
loc_8001EFE4:
    if (v0 == 0) goto loc_8001F00C;
    v0 = lw(gp + 0xA98);                                // Load from: gTmFlags (80078078)
    v0 &= 0x800;
    if (v0 == 0) goto loc_8001F00C;
    sw(a2, gp + 0xCE4);                                 // Store to: gpMoveThing (800782C4)
loc_8001F004:
    v0 = 1;                                             // Result = 00000001
    goto loc_8001F020;
loc_8001F00C:
    v0 = lw(a2 + 0x64);
    v0 >>= 1;
loc_8001F018:
    v0 ^= 1;
    v0 &= 1;
loc_8001F020:
    return;
}

void PM_CheckLines() noexcept {
loc_8001F028:
    v0 = *gBlockmapWidth;
    mult(a1, v0);
    v1 = *gpBlockmap;
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s0, sp + 0x20);
    v0 = lo;
    v0 += a0;
    v0 <<= 1;
    v0 += v1;
    v0 = lh(v0);
    v1 = *gpBlockmapLump;
    v0 <<= 1;
    s0 = v0 + v1;
    goto loc_8001F1DC;
loc_8001F074:
    v1 = u32(i32(v1) >> 16);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v1 = *gpLines;
    v0 <<= 2;
    t0 = v0 + v1;
    v0 = lw(t0 + 0x40);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    if (v0 == v1) goto loc_8001F1D8;
    sw(v1, t0 + 0x40);
    a2 = 0x80090000;                                    // Result = 80090000
    a2 = lw(a2 + 0x7C1C);                               // Load from: gtTmbBox[3] (80097C1C)
    v0 = lw(t0 + 0x2C);
    v0 = (i32(v0) < i32(a2));
    a1 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_8001F1B8;
    a1 = 0x80090000;                                    // Result = 80090000
    a1 = lw(a1 + 0x7C18);                               // Load from: gtTmbBox[2] (80097C18)
    v0 = lw(t0 + 0x30);
    v0 = (i32(a1) < i32(v0));
    if (v0 == 0) goto loc_8001F124;
    v1 = 0x80090000;                                    // Result = 80090000
    v1 = lw(v1 + 0x7C10);                               // Load from: gtTmbBox[0] (80097C10)
    v0 = lw(t0 + 0x28);
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8001F124;
    a0 = 0x80090000;                                    // Result = 80090000
    a0 = lw(a0 + 0x7C14);                               // Load from: gtTmbBox[1] (80097C14)
    v0 = lw(t0 + 0x24);
    v0 = (i32(a0) < i32(v0));
    t2 = v1;
    if (v0 != 0) goto loc_8001F12C;
loc_8001F124:
    a1 = 0;                                             // Result = 00000000
    goto loc_8001F1B8;
loc_8001F12C:
    v1 = lw(t0 + 0x34);
    v0 = 2;                                             // Result = 00000002
    t3 = a0;
    if (v1 != v0) goto loc_8001F148;
    v0 = a1;
    t1 = a2;
    goto loc_8001F150;
loc_8001F148:
    v0 = a2;
    t1 = a1;
loc_8001F150:
    a0 = lw(t0);
    v1 = lw(a0);
    a3 = lh(t0 + 0xE);
    v0 -= v1;
    v0 = u32(i32(v0) >> 16);
    mult(a3, v0);
    a2 = lh(t0 + 0xA);
    a0 = lw(a0 + 0x4);
    a1 = lo;
    v0 = t2 - a0;
    v0 = u32(i32(v0) >> 16);
    mult(v0, a2);
    v0 = lo;
    v1 = t1 - v1;
    v1 = u32(i32(v1) >> 16);
    mult(a3, v1);
    v1 = lo;
    a0 = t3 - a0;
    a0 = u32(i32(a0) >> 16);
    mult(a0, a2);
    a1 = (i32(a1) < i32(v0));
    v0 = lo;
    v1 = (i32(v1) < i32(v0));
    a1 ^= v1;
    a1 = (a1 > 0);
loc_8001F1B8:
    if (a1 == 0) goto loc_8001F1D8;
    a0 = t0;
    PIT_CheckLine();
    s0 += 2;
    if (v0 != 0) goto loc_8001F1DC;
    v0 = 0;                                             // Result = 00000000
    goto loc_8001F1F4;
loc_8001F1D8:
    s0 += 2;
loc_8001F1DC:
    v0 = -1;                                            // Result = FFFFFFFF
    v1 = lh(s0);
    a0 = lhu(s0);
    {
        const bool bJump = (v1 != v0);
        v1 = a0 << 16;
        if (bJump) goto loc_8001F074;
    }
    v0 = 1;                                             // Result = 00000001
loc_8001F1F4:
    ra = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x28;
    return;
}

void PM_CheckThings() noexcept {
loc_8001F208:
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
    if (s0 == 0) goto loc_8001F26C;
loc_8001F24C:
    a0 = s0;
    PIT_CheckThing();
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001F26C;
    }
    s0 = lw(s0 + 0x30);
    v0 = 1;                                             // Result = 00000001
    if (s0 != 0) goto loc_8001F24C;
loc_8001F26C:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}
