#include "p_maputl.h"

#include "Doom/Base/m_fixed.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "p_local.h"
#include "p_setup.h"
#include "PsxVm/PsxVm.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <algorithm>
    #include <cmath>
END_THIRD_PARTY_INCLUDES

//------------------------------------------------------------------------------------------------------------------------------------------
// Gives a cheap approximate/estimated length for the given vector
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_AproxDistance(const fixed_t dx, const fixed_t dy) noexcept {
    const fixed_t udx = std::abs(dx);
    const fixed_t udy = std::abs(dy);
    return udx + udy - std::min(udx, udy) / 2;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell what side of a line a point is on.
// Returns '0' if on the front side, '1' if on the back side.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t P_PointOnLineSide(const fixed_t x, const fixed_t y, const line_t& line) noexcept {
    // Easy case: north/south wall.
    // Check what side of the line we are on, then return the answer based on what way the wall is winding.
    if (line.dx == 0) {
        if (x > line.vertex1->x) {
            return (line.dy < 0);
        } else {
            return (line.dy > 0);
        }
    }

    // Easy case: east/west wall.
    // Check what side of the line we are on, then return the answer based on what way the wall is winding.
    if (line.dy == 0) {
        if (y > line.vertex1->y) {
            return (line.dx > 0);
        } else {
            return (line.dx < 0);
        }
    }

    // Harder case: use the same cross product trick found in places like 'R_PointOnSide' to tell what side of the line we are on
    const int32_t vdx = (x - line.vertex1->x) >> FRACBITS;
    const int32_t vdy = (y - line.vertex1->y) >> FRACBITS;
    const int32_t ldx = line.dx >> FRACBITS;
    const int32_t ldy = line.dy >> FRACBITS;

    return (vdx * ldy <= vdy * ldx);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tell what side of a dividing line a point is on.
// Returns '0' if on the front side, '1' if on the back side.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t P_PointOnDivlineSide(const fixed_t x, const fixed_t y, const divline_t& divline) noexcept {
    // Easy case: north/south wall.
    // Check what side of the line we are on, then return the answer based on what way the wall is winding.
    if (divline.dx == 0) {
        if (x > divline.x) {
            return (divline.dy < 0);
        } else {
            return (divline.dy > 0);
        }
    }

    // Easy case: east/west wall.
    // Check what side of the line we are on, then return the answer based on what way the wall is winding.
    if (divline.dy == 0) {
        if (y > divline.y) {
            return (divline.dx > 0);
        } else {
            return (divline.dx < 0);
        }
    }

    // Try to decide based on the sign bits if there is a sign difference between the left and right products.
    // The comparsion below is ultimately 'lprod <= rprod' and if we can tell just by examining signs, do that now:
    const fixed_t vdx = x - divline.x;
    const fixed_t vdy = y - divline.y;

    if ((divline.dy ^ divline.dx ^ vdx ^ vdy) < 0) {
        // Sign difference: use the left product sign to decide - it must be the opposite of the right product sign (see below)
        return ((vdx ^ divline.dy) < 0);
    }

    // Finally fallback to using the 2d vector cross product to decide what side we are on
    const fixed_t lprod = FixedMul(vdx >> 8, divline.dy >> 8);
    const fixed_t rprod = FixedMul(vdy >> 8, divline.dx >> 8);
    return (lprod <= rprod);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: populate a divline struct from the given line
//------------------------------------------------------------------------------------------------------------------------------------------
void P_MakeDivline(const line_t& line, divline_t& divline) noexcept {
    divline.x = line.vertex1->x;
    divline.y = line.vertex1->y;
    divline.dx = line.dx;
    divline.dy = line.dy;
}

void P_LineOpening() noexcept {
loc_8001C25C:
    v1 = lw(a0 + 0x20);
    v0 = -1;                                            // Result = FFFFFFFF
    if (v1 != v0) goto loc_8001C278;
    sw(0, gp + 0xC9C);                                  // Store to: gOpenRange (8007827C)
    goto loc_8001C2F0;
loc_8001C278:
    a2 = lw(a0 + 0x38);
    a0 = lw(a0 + 0x3C);
    a1 = lw(a2 + 0x4);
    v1 = lw(a0 + 0x4);
    v0 = (i32(a1) < i32(v1));
    if (v0 == 0) goto loc_8001C2A4;
    sw(a1, gp + 0xADC);                                 // Store to: gOpenTop (800780BC)
    goto loc_8001C2A8;
loc_8001C2A4:
    sw(v1, gp + 0xADC);                                 // Store to: gOpenTop (800780BC)
loc_8001C2A8:
    a1 = lw(a2);
    v1 = lw(a0);
    v0 = (i32(v1) < i32(a1));
    if (v0 == 0) goto loc_8001C2D0;
    v0 = lw(a0);
    sw(a1, gp + 0x950);                                 // Store to: gOpenBottom (80077F30)
    goto loc_8001C2D8;
loc_8001C2D0:
    v0 = lw(a2);
    sw(v1, gp + 0x950);                                 // Store to: gOpenBottom (80077F30)
loc_8001C2D8:
    sw(v0, gp + 0xBFC);                                 // Store to: gLowFloor (800781DC)
    v0 = lw(gp + 0xADC);                                // Load from: gOpenTop (800780BC)
    v1 = lw(gp + 0x950);                                // Load from: gOpenBottom (80077F30)
    v0 -= v1;
    sw(v0, gp + 0xC9C);                                 // Store to: gOpenRange (8007827C)
loc_8001C2F0:
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Unlinks the given thing from sector thing lists and the blockmap
//------------------------------------------------------------------------------------------------------------------------------------------
void P_UnsetThingPosition(mobj_t& thing) noexcept {
    // Does this thing get assigned to sector thing lists?
    // If so remove it from the sector thing list.
    if ((thing.flags & MF_NOSECTOR) == 0) {
        if (thing.snext) {
            thing.snext->sprev = thing.sprev;
        }
        
        if (thing.sprev) {
            thing.sprev->snext = thing.snext;
        } else {
            thing.subsector->sector->thinglist = thing.snext;
        }
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
// Update the subsector for the thing.
// Also add the thing to sector and blockmap thing lists if applicable.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SetThingPosition(mobj_t& thing) noexcept {
    // First update what subsector the thing is in
    subsector_t* const pSubsec = R_PointInSubsector(thing.x, thing.y);
    thing.subsector = pSubsec;

    // Add the thing to sector thing lists, if the thing flags allow it
    if ((thing.flags & MF_NOSECTOR) == 0) {
        sector_t& sec = *pSubsec->sector;
        thing.sprev = nullptr;
        thing.snext = sec.thinglist;
    
        if (sec.thinglist) {
            sec.thinglist->sprev = &thing;
        }

        sec.thinglist = &thing;
    }

    // Add the thing to blockmap thing lists, if the thing flags allow it
    if ((thing.flags & MF_NOBLOCKMAP) == 0) {
        const int32_t blockx = (thing.x - *gBlockmapOriginX) >> MAPBLOCKSHIFT;
        const int32_t blocky = (thing.y - *gBlockmapOriginY) >> MAPBLOCKSHIFT;

        if (blockx >= 0 && blockx < *gBlockmapWidth && blocky >= 0 && blocky < *gBlockmapHeight) {
            VmPtr<mobj_t>& blockmapEntry = (*gppBlockLinks)[blocky * (*gBlockmapWidth) + blockx];
            thing.bprev = nullptr;
            thing.bnext = blockmapEntry;

            if (blockmapEntry) {
                blockmapEntry->bprev = &thing;
            }

            blockmapEntry = &thing;
        } else {
            thing.bprev = nullptr;
            thing.bnext = nullptr;
        }
    }
}

void P_BlockLinesIterator() noexcept {
loc_8001C540:
    sp -= 0x30;
    sw(s2, sp + 0x28);
    s2 = a2;
    sw(ra, sp + 0x2C);
    sw(s1, sp + 0x24);
    sw(s0, sp + 0x20);
    if (i32(a0) < 0) goto loc_8001C640;
    v0 = 1;                                             // Result = 00000001
    if (i32(a1) < 0) goto loc_8001C644;
    v1 = *gBlockmapWidth;
    v0 = (i32(a0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001C644;
    }
    v0 = *gBlockmapHeight;
    v0 = (i32(a1) < i32(v0));
    mult(a1, v1);
    if (v0 != 0) goto loc_8001C59C;
    v0 = 1;                                             // Result = 00000001
    goto loc_8001C644;
loc_8001C59C:
    v1 = *gpBlockmap;
    v0 = lo;
    v0 += a0;
    v0 <<= 1;
    v0 += v1;
    v0 = lh(v0);
    v1 = *gpBlockmapLump;
    v0 <<= 1;
    s0 = v0 + v1;
    v0 = -1;                                            // Result = FFFFFFFF
    v1 = lh(s0);
    a0 = lhu(s0);
    {
        const bool bJump = (v1 == v0);
        v0 = 1;
        if (bJump) goto loc_8001C644;
    }
    s1 = -1;                                            // Result = FFFFFFFF
    v1 = a0 << 16;
loc_8001C5E4:
    v1 = u32(i32(v1) >> 16);
    v0 = v1 << 2;
    v0 += v1;
    v0 <<= 2;
    v0 -= v1;
    v1 = *gpLines;
    v0 <<= 2;
    a0 = v0 + v1;
    v0 = lw(a0 + 0x40);
    v1 = 0x80070000;                                    // Result = 80070000
    v1 = lw(v1 + 0x7BC4);                               // Load from: gValidCount (80077BC4)
    s0 += 2;
    if (v0 == v1) goto loc_8001C630;
    sw(v1, a0 + 0x40);
    ptr_call(s2);
    {
        const bool bJump = (v0 == 0);
        v0 = 0;                                         // Result = 00000000
        if (bJump) goto loc_8001C644;
    }
loc_8001C630:
    v0 = lh(s0);
    a0 = lhu(s0);
    v1 = a0 << 16;
    if (v0 != s1) goto loc_8001C5E4;
loc_8001C640:
    v0 = 1;                                             // Result = 00000001
loc_8001C644:
    ra = lw(sp + 0x2C);
    s2 = lw(sp + 0x28);
    s1 = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x30;
    return;
}

void P_BlockThingsIterator() noexcept {
loc_8001C660:
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a2;
    sw(ra, sp + 0x18);
    sw(s0, sp + 0x10);
    if (i32(a0) < 0) goto loc_8001C708;
    v0 = 1;                                             // Result = 00000001
    if (i32(a1) < 0) goto loc_8001C70C;
    v1 = *gBlockmapWidth;
    v0 = (i32(a0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_8001C70C;
    }
    v0 = *gBlockmapHeight;
    v0 = (i32(a1) < i32(v0));
    mult(a1, v1);
    if (v0 != 0) goto loc_8001C6C0;
    v0 = 1;                                             // Result = 00000001
    goto loc_8001C70C;
loc_8001C6B8:
    v0 = 0;                                             // Result = 00000000
    goto loc_8001C70C;
loc_8001C6C0:
    v1 = *gppBlockLinks;
    v0 = lo;
    v0 += a0;
    v0 <<= 2;
    v0 += v1;
    s0 = lw(v0);
    v0 = 1;                                             // Result = 00000001
    if (s0 == 0) goto loc_8001C70C;
loc_8001C6E8:
    a0 = s0;
    ptr_call(s1);
    if (v0 == 0) goto loc_8001C6B8;
    s0 = lw(s0 + 0x30);
    if (s0 != 0) goto loc_8001C6E8;
loc_8001C708:
    v0 = 1;                                             // Result = 00000001
loc_8001C70C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
