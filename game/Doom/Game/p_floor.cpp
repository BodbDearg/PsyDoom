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
#include <algorithm>

static constexpr fixed_t FLOORSPEED = FRACUNIT * 3;     // Standard speed for floors moving up and down

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
static void T_MoveFloor(floormove_t& floor) noexcept {
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger the given floor mover for sectors with the same tag as the given line
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoFloor(line_t& line, const floor_e floorType) noexcept {
    bool bActivatedAFloor = false;

    for (int32_t sectorIdx = P_FindSectorFromLineTag(line, -1); sectorIdx >= 0; sectorIdx = P_FindSectorFromLineTag(line, sectorIdx)) {
        // Ignore if the sector already has a special or moving floor
        sector_t& sector = gpSectors->get()[sectorIdx];

        if (sector.specialdata)
            continue;

        // Found a sector which will be affected by this floor special: create a thinker and link to the sector
        bActivatedAFloor = true;

        floormove_t& floor = *(floormove_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(floormove_t), PU_LEVSPEC, nullptr);
        P_AddThinker(floor.thinker);
        sector.specialdata = &floor;

        // Common floor mover setup
        floor.thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_MoveFloor);
        floor.type = floorType;
        floor.crush = false;

        // Setup for specific floor types
        switch (floorType) {
            case lowerFloor: {
                floor.direction = -1;
                floor.sector = &sector;
                floor.speed = FLOORSPEED;
                floor.floordestheight = P_FindHighestFloorSurrounding(sector);
            }   break;

            case lowerFloorToLowest: {
                floor.direction = -1;
                floor.sector = &sector;
                floor.speed = FLOORSPEED;
                floor.floordestheight = P_FindLowestFloorSurrounding(sector);
            }   break;

            case turboLower: {
                floor.direction = -1;
                floor.sector = &sector;
                floor.speed = FLOORSPEED * 4;
                floor.floordestheight = P_FindHighestFloorSurrounding(sector);

                if (floor.floordestheight != sector.floorheight) {      // Create a small lip if lowering
                    floor.floordestheight += 8 * FRACUNIT;
                }
            }   break;

            case raiseFloorCrush:
                floor.crush = true;     // Note: intentional fallthrough
            case raiseFloor: {
                floor.direction = 1;
                floor.sector = &sector;
                floor.speed = FLOORSPEED;
                floor.floordestheight = P_FindLowestCeilingSurrounding(sector);
                floor.floordestheight = std::min(floor.floordestheight, sector.ceilingheight);      // Don't go above the current ceiling
                
                if (floorType == raiseFloorCrush) {
                    floor.floordestheight -= 8 * FRACUNIT;      // Leave a small gap for crushing floors (in case player gets caught in it)
                }
            }   break;

            case raiseFloorToNearest: {
                floor.direction = 1;
                floor.sector = &sector;
                floor.speed = FLOORSPEED;
                floor.floordestheight = P_FindNextHighestFloor(sector, sector.floorheight);
            }   break;

            case raiseFloor24: {
                floor.direction = 1;
                floor.sector = &sector;
                floor.speed = FLOORSPEED;
                floor.floordestheight = sector.floorheight + 24 * FRACUNIT;
            }   break;

            case raiseFloor24AndChange: {
                floor.direction = 1;
                floor.sector = &sector;
                floor.speed = FLOORSPEED;
                floor.floordestheight = sector.floorheight + 24 * FRACUNIT;
                sector.floorpic = line.frontsector->floorpic;
                sector.special = line.frontsector->special;
            }   break;

            case raiseToTexture: {
                floor.direction = 1;
                floor.sector = &sector;
                floor.speed = FLOORSPEED;
                
                // Raise the floor by the height of the lowest surrounding lower texture
                fixed_t minLowerTexH = INT32_MAX;

                for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
                    if (!twoSided(sectorIdx, lineIdx))
                        continue;

                    side_t& side1 = *getSide(sectorIdx, lineIdx, 0);

                    if (side1.bottomtexture >= 0) {
                        texture_t& tex = gpTextures->get()[side1.bottomtexture];
                        minLowerTexH = std::min(minLowerTexH, (fixed_t) tex.height << FRACBITS);
                    }

                    side_t& side2 = *getSide(sectorIdx, lineIdx, 1);

                    if (side2.bottomtexture >= 0) {
                        texture_t& tex = gpTextures->get()[side2.bottomtexture];
                        minLowerTexH = std::min(minLowerTexH, (fixed_t) tex.height << FRACBITS);
                    }
                }

                floor.floordestheight = sector.floorheight + minLowerTexH;
            }   break;

            case lowerAndChange: {
                floor.direction = -1;
                floor.sector = &sector;
                floor.speed = FLOORSPEED;
                floor.floordestheight = P_FindLowestFloorSurrounding(sector);
                floor.texture = (int16_t) sector.floorpic;

                // Change the texture and special to be the same as the sector on the opposite side of the target sector's first 2 sided line
                for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
                    if (!twoSided(sectorIdx, lineIdx))
                        continue;
                    
                    side_t& side1 = *getSide(sectorIdx, lineIdx, 0);
                    const int32_t side1SectorIdx = (int32_t)(side1.sector.get() - gpSectors->get());
                    sector_t& oppositeSector = *getSector(sectorIdx, lineIdx, (side1SectorIdx == sectorIdx) ? 1 : 0);

                    floor.texture = (int16_t) oppositeSector.floorpic;
                    floor.newspecial = oppositeSector.special;
                    break;
                }
            }   break;

            default:
                break;
        }
    }

    return bActivatedAFloor;
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
