#include "p_floor.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "g_game.h"
#include "p_change.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to move a floor or ceiling up or down, potentially crushing things contained within.
// Returns the high level result of the movement.
//------------------------------------------------------------------------------------------------------------------------------------------
result_e T_MovePlane(
    sector_t& sector,                   // The sector to move up/down
    const fixed_t speed,                // Speed of plane movement
    const fixed_t destHeight,           // Height that the floor or ceiling wants to get to
    const bool bCrush,                  // If true then damage things contained within when they are being crushed (not fitting vertically)
    const int32_t floorOrCeiling,       // 0 = floor, 1 = ceiling
    const int32_t direction             // -1 = down, 1 = up
) noexcept {
    // What are we moving?
    if (floorOrCeiling == 0) {
        // Moving a floor
        if (direction == -1) {
            // Moving a floor down
            const fixed_t prevFloorH = sector.floorheight;

            if (sector.floorheight - speed < destHeight) {
                // Reached destination: allow move if not crushing stuff, otherwise restore previous height
                sector.floorheight = destHeight;
                
                if (P_ChangeSector(sector, bCrush)) {
                    sector.floorheight = prevFloorH;
                    P_ChangeSector(sector, bCrush);     // Need to run again after restoring floor height (I won't repeat this comment for the other similar cases)
                }

                return pastdest;
            } 
            else {
                // Move ongoing: allow move if not crushing stuff, otherwise restore previous height
                sector.floorheight -= speed;
                
                if (P_ChangeSector(sector, bCrush)) {
                    sector.floorheight = prevFloorH;
                    P_ChangeSector(sector, bCrush);
                    return crushed;
                }
            }
        } 
        else if (direction == 1) {
            // Moving a floor up
            const fixed_t prevFloorH = sector.floorheight;

            if (sector.floorheight + speed > destHeight) {
                // Reached destination: allow move if not crushing stuff, otherwise restore previous height
                sector.floorheight = destHeight;
                
                if (P_ChangeSector(sector, bCrush)) {
                    sector.floorheight = prevFloorH;
                    P_ChangeSector(sector, bCrush);
                    return crushed;
                }

                return pastdest;
            }
            else {
                // Move ongoing: allow move if not crushing stuff, otherwise restore previous height
                sector.floorheight += speed;

                if (P_ChangeSector(sector, bCrush)) {
                    // For floors moving up allow the move if the crush flag is set, otherwise we need to restore
                    if (!bCrush) {
                        sector.floorheight = prevFloorH;
                        P_ChangeSector(sector, bCrush);
                    }

                    return crushed;
                }
            }
        }
    }
    else if (floorOrCeiling == 1) {
        // Moving a ceiling
        if (direction == -1) {
            // Moving a ceiling down
            const fixed_t prevCeilH = sector.ceilingheight;

            if (sector.ceilingheight - speed < destHeight) {
                // Reached destination: allow move if not crushing stuff, otherwise restore previous height
                sector.ceilingheight = destHeight;
                
                if (P_ChangeSector(sector, bCrush)) {
                    sector.ceilingheight = prevCeilH;
                    P_ChangeSector(sector, bCrush);
                }

                return pastdest;
            }
            else {
                // Move ongoing: allow move if not crushing stuff, otherwise restore previous height
                sector.ceilingheight -= speed;
                
                if (P_ChangeSector(sector, bCrush)) {
                    // For ceilings moving down allow the move if the crush flag is set, otherwise we need to restore
                    if (!bCrush) {
                        sector.ceilingheight = prevCeilH;
                        P_ChangeSector(sector, bCrush);
                    }

                    return crushed;
                }
            }
        } 
        else if (direction == 1) {
            // Moving a ceiling up
            const fixed_t prevCeilH = sector.ceilingheight;
            
            if (speed + sector.ceilingheight > destHeight) {
                // Reached destination: allow move if not crushing stuff, otherwise restore previous height
                sector.ceilingheight = destHeight;
                
                if (P_ChangeSector(sector, bCrush)) {
                    sector.ceilingheight = prevCeilH;
                    P_ChangeSector(sector, bCrush);
                }

                return pastdest;
            }
            else {
                // Move ongoing: allow move in all cases for a ceiling going up
                sector.ceilingheight += speed;
                P_ChangeSector(sector, bCrush);
            }
        }
    }

    // Movement was OK
    return ok;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker/update logic for a moving floor: moves the floor, does floor state transitions and sounds etc.
//------------------------------------------------------------------------------------------------------------------------------------------
void T_MoveFloor(floormove_t& floor) noexcept {
    // Move the floor!
    sector_t& floorSec = *floor.sector;
    const result_e moveResult = T_MovePlane(floorSec, floor.speed, floor.floordestheight, floor.crush, 0, floor.direction);

    // Every so often do a floor movement sound (every 4 tics)
    if ((*gGameTic & 3) == 0) {
        S_StartSound((mobj_t*) &floorSec.soundorg, sfx_stnmov);
    }

    // Has the floor reached it's intended position?
    if (moveResult == pastdest) {
        // Reached destination: change the floor texture and sector special if required
        if (floor.direction == 1) {
            if (floor.type == donutRaise) {
                floorSec.special = floor.newspecial;
                floorSec.floorpic = floor.texture;
            }
        }
        else if (floor.direction == -1) {
            if (floor.type == lowerAndChange) {
                floorSec.special = floor.newspecial;
                floorSec.floorpic = floor.texture;
            }
        }

        // Remove the floor thinker and it's link to the sector, this movement is now done
        floorSec.specialdata = nullptr;
        P_RemoveThinker(floor.thinker);
    }
}

// TODO: REMOVE eventually
void _thunk_T_MoveFloor() noexcept {
    T_MoveFloor(*vmAddrToPtr<floormove_t>(*PsxVm::gpReg_a0));
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
    v0 = P_FindSectorFromLineTag(*vmAddrToPtr<line_t>(a0), a1);
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
    _thunk_P_AddThinker();
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
    v0 = P_FindHighestFloorSurrounding(*vmAddrToPtr<sector_t>(a0));
    sw(v0, s0 + 0x24);
    goto loc_80019144;
loc_8001921C:
    a0 = s2;
    t0 = -1;                                            // Result = FFFFFFFF
    sw(t0, s0 + 0x18);
    sw(a0, s0 + 0x14);
    sw(s5, s0 + 0x28);
    v0 = P_FindLowestFloorSurrounding(*vmAddrToPtr<sector_t>(a0));
    sw(v0, s0 + 0x24);
    goto loc_80019144;
loc_8001923C:
    a0 = s2;
    t0 = -1;                                            // Result = FFFFFFFF
    v0 = 0xC0000;                                       // Result = 000C0000
    sw(t0, s0 + 0x18);
    sw(s2, s0 + 0x14);
    sw(v0, s0 + 0x28);
    v0 = P_FindHighestFloorSurrounding(*vmAddrToPtr<sector_t>(a0));
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
    v0 = P_FindLowestCeilingSurrounding(*vmAddrToPtr<sector_t>(a0));
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
    v0 = P_FindNextHighestFloor(*vmAddrToPtr<sector_t>(a0), a1);
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
    v0 = twoSided(a0, a1);
    a0 = s4;
    if (v0 == 0) goto loc_80019414;
    a1 = s1;
    a2 = 0;                                             // Result = 00000000
    v0 = ptrToVmAddr(getSide(a0, a1, a2));
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
    v0 = ptrToVmAddr(getSide(a0, a1, a2));
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
    v0 = P_FindLowestFloorSurrounding(*vmAddrToPtr<sector_t>(a0));
    sw(v0, s0 + 0x24);
    v0 = lhu(s2 + 0x8);
    sh(v0, s0 + 0x20);
    v0 = lw(s2 + 0x54);
    s1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80019144;
    a0 = s4;
loc_80019480:
    a1 = s1;
    v0 = twoSided(a0, a1);
    a0 = s4;
    if (v0 == 0) goto loc_800194F4;
    a1 = s1;
    a2 = 0;                                             // Result = 00000000
    v0 = ptrToVmAddr(getSide(a0, a1, a2));
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
    v0 = ptrToVmAddr(getSector(a0, a1, a2));
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
    v0 = P_FindSectorFromLineTag(*vmAddrToPtr<line_t>(a0), a1);
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
    _thunk_P_AddThinker();
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
    _thunk_P_AddThinker();
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
