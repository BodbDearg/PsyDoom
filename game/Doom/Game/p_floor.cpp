#include "p_floor.h"

#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "doomdata.h"
#include "g_game.h"
#include "p_change.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/ScriptingEngine.h"

#include <algorithm>

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: default custom floor settings
//------------------------------------------------------------------------------------------------------------------------------------------
#if PSYDOOM_MODS
CustomFloorDef::CustomFloorDef() noexcept
    : bCrush(false)
    , bDoFinishScript(false)
    , destHeight(0)
    , speed(FLOORSPEED)
    , startSound(sfx_None)
    , moveSound(sfx_stnmov)
    , moveSoundFreq(4)
    , stopSound(sfx_None)
    , finishScriptActionNum(0)
    , finishScriptUserdata(0)
{
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempts to move a floor or ceiling up or down, potentially crushing things contained within.
// Returns the high level result of the movement.
//------------------------------------------------------------------------------------------------------------------------------------------
result_e T_MovePlane(
    sector_t& sector,                   // The sector to move up/down
    fixed_t speed,                      // Speed of plane movement
    const fixed_t destHeight,           // Height that the floor or ceiling wants to get to
    const bool bCrush,                  // If true then damage things contained within when they are being crushed (not fitting vertically)
    const int32_t floorOrCeiling,       // 0 = floor, 1 = ceiling
    const int32_t direction             // -1 = down, 1 = up
) noexcept {
    // PsyDoom: if the 'turbo' cheat is enabled allow planes to move 2x as fast
    #if PSYDOOM_MODS
        if (Game::gSettings.bTurboMode) {
            speed *= 2;
        }
    #endif

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

    // Every so often do a floor movement sound (every 4 tics).
    // PsyDoom: custom floors might now have a different sound frequency and id.
    const floor_e floorType = floor.type;

    #if PSYDOOM_MODS
        if (floorType != customFloor) {
            // Regular floor type, do the original floor mover sounds
            if ((gGameTic & 3) == 0) {
                S_StartSound((mobj_t*) &floorSec.soundorg, sfx_stnmov);
            }
        } else {
            // Custom floor type: do the mover sounds according to the settings for the floor
            if (floor.moveSound != sfx_None) {
                const uint32_t moveSoundFreq = floor.moveSoundFreq;

                if ((moveSoundFreq <= 1) || ((gGameTic % moveSoundFreq) == 0)) {
                    S_StartSound((mobj_t*) &floorSec.soundorg, floor.moveSound);
                }
            }
        }
    #else
        if ((gGameTic & 3) == 0) {
            S_StartSound((mobj_t*) &floorSec.soundorg, sfx_stnmov);
        }
    #endif

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

        // PsyDoom: custom floor sounds and actions on finish
        #if PSYDOOM_MODS
            if (floorType == customFloor) {
                if (floor.stopSound != sfx_None) {
                    S_StartSound((mobj_t*) &floorSec.soundorg, floor.stopSound);
                }

                if (floor.bDoFinishScript) {
                    ScriptingEngine::doAction(floor.finishScriptActionNum, nullptr, &floorSec, nullptr, 0, floor.finishScriptUserdata);
                }
            }
        #endif
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Trigger the given floor mover for sectors with the same tag as the given line
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoFloor(line_t& line, const floor_e floorType) noexcept {
    #if PSYDOOM_MODS
        ASSERT_LOG(floorType != customFloor, "Custom floors must be created via 'EV_DoCustomFloor'!");
    #endif

    bool bActivatedAMover = false;

    for (int32_t sectorIdx = P_FindSectorFromLineTag(line, -1); sectorIdx >= 0; sectorIdx = P_FindSectorFromLineTag(line, sectorIdx)) {
        // Ignore if the sector already has a special or moving floor
        sector_t& sector = gpSectors[sectorIdx];

        if (sector.specialdata)
            continue;

        // Found a sector which will be affected by this floor special: create a thinker and link to the sector
        bActivatedAMover = true;
        floormove_t& floor = *(floormove_t*) Z_Malloc(*gpMainMemZone, sizeof(floormove_t), PU_LEVSPEC, nullptr);

        #if PSYDOOM_MODS
            floor = {};   // PsyDoom: zero-init all fields, including ones unused by this function
        #endif

        P_AddThinker(floor.thinker);
        sector.specialdata = &floor;

        // Common floor mover setup
        floor.thinker.function = (think_t) &T_MoveFloor;
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
                [[fallthrough]];
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

            // PsyDoom: adding support for missing line specials from PC
            #if PSYDOOM_MODS
                case raiseFloorTurbo: {
                    floor.direction = 1;
                    floor.sector = &sector;
                    floor.speed = FLOORSPEED * 4;
                    floor.floordestheight = P_FindNextHighestFloor(sector, sector.floorheight);
                }   break;
            #endif

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

            // PsyDoom: adding support for the 'Raise Floor 512' special
            #if PSYDOOM_MODS
                case raiseFloor512: {
                    floor.direction = 1;
                    floor.sector = &sector;
                    floor.speed = FLOORSPEED;
                    floor.floordestheight = sector.floorheight + 512 * FRACUNIT;
                }   break;
            #endif

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
                        texture_t& tex = gpTextures[side1.bottomtexture];
                        minLowerTexH = std::min(minLowerTexH, d_int_to_fixed(tex.height));
                    }

                    side_t& side2 = *getSide(sectorIdx, lineIdx, 1);

                    if (side2.bottomtexture >= 0) {
                        texture_t& tex = gpTextures[side2.bottomtexture];
                        minLowerTexH = std::min(minLowerTexH, d_int_to_fixed(tex.height));
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
                    const int32_t side1SectorIdx = (int32_t)(side1.sector - gpSectors);
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

    return bActivatedAMover;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tries to build a staircase of the specified type, with the starting step for each stairs being sectors matching the given line's tag
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_BuildStairs(line_t& line, const stair_e stairType) noexcept {
    bool bActivatedAMover = false;

    // Search for first step sectors to build a stairs from using the given line tag
    for (int32_t sectorIdx = P_FindSectorFromLineTag(line, -1); sectorIdx >= 0; sectorIdx = P_FindSectorFromLineTag(line, sectorIdx)) {
        // Ignore if the sector already has a special or moving floor
        sector_t& firstSector = gpSectors[sectorIdx];

        if (firstSector.specialdata)
            continue;

        // Found a stairs sector which will be affected by this floor special: create a thinker for the first step and link to the sector
        bActivatedAMover = true;
        floormove_t& firstFloor = *(floormove_t*) Z_Malloc(*gpMainMemZone, sizeof(floormove_t), PU_LEVSPEC, nullptr);

        #if PSYDOOM_MODS
            firstFloor = {};   // PsyDoom: zero-init all fields, including ones unused by this function
        #endif

        P_AddThinker(firstFloor.thinker);
        firstSector.specialdata = &firstFloor;

        // Setup the mover for the first step
        firstFloor.thinker.function = (think_t) &T_MoveFloor;
        firstFloor.direction = 1;
        firstFloor.sector = &firstSector;

        fixed_t moveSpeed = FLOORSPEED;     // Note: these were previously un-initialized in the PSX code if the stair type was not known
        fixed_t stepHeight = 8 * FRACUNIT;

        if (stairType == build8) {
            moveSpeed = FLOORSPEED / 2;
            stepHeight = 8 * FRACUNIT;
        }
        else if (stairType == turbo16) {
            moveSpeed = FLOORSPEED * 2;
            stepHeight = 16 * FRACUNIT;
        }

        fixed_t height = firstSector.floorheight + stepHeight;   // First step height

        firstFloor.speed = moveSpeed;
        firstFloor.floordestheight = height;

        // Keep building stair steps while we find two sided lines where:
        //  (1) The front sector of the line is the same sector as the previous step sector.
        //  (2) The back sector of the line has the same floor texture as the stairs.
        const int32_t stairFloorPic = firstSector.floorpic;

        while (true) {
            bool bDidAStep = false;
            sector_t& sector = gpSectors[sectorIdx];

            for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
                // Ignore the line if not two sided
                line_t& stepLine = *sector.lines[lineIdx];

                if ((stepLine.flags & ML_TWOSIDED) == 0)
                    continue;

                // Ignore the line if the front sector is not the same as the previous step
                sector_t& fsec = *stepLine.frontsector;
                const int32_t fsecIdx = (int32_t)(&fsec - gpSectors);

                if (fsecIdx != sectorIdx)
                    continue;

                // Don't build a step if the back sector doesn't have the same texture
                sector_t& bsec = *stepLine.backsector;

                if (bsec.floorpic != stairFloorPic)
                    continue;

                // Inc height for this step and as a precaution don't do a step if there is already a sector special
                height += stepHeight;

                if (bsec.specialdata)
                    continue;

                // Create a thinker for this step's floor mover, link to the sector and populate it's settings
                floormove_t& floor = *(floormove_t*) Z_Malloc(*gpMainMemZone, sizeof(floormove_t), PU_LEVSPEC, nullptr);

                #if PSYDOOM_MODS
                    floor = {};   // PsyDoom: zero-init all fields, including ones unused by this function
                #endif

                P_AddThinker(floor.thinker);
                bsec.specialdata = &floor;

                floor.thinker.function = (think_t) &T_MoveFloor;
                floor.direction = 1;
                floor.sector = &bsec;
                floor.speed = moveSpeed;
                floor.floordestheight = height;

                // Search for the next step to make from the one we just did
                sectorIdx = (int32_t)(&bsec - gpSectors);

                // Need to start line checks anew because we are on a new step sector
                bDidAStep = true;
                break;
            }

            // Done making the staircase yet?
            if (!bDidAStep)
                break;
        }
    }

    return bActivatedAMover;
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: does a custom type of floor mover for the specified sector.
// Optionally, a script action can be set to execute upon the mover completing.
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoCustomFloor(sector_t& sector, const CustomFloorDef& floorDef) noexcept {
    // Can't do a floor mover if the sector already has some other special happening
    if (sector.specialdata)
        return false;

    // Allocate the floor mover, zero initialize and set as the sector thinker
    floormove_t& floor = *(floormove_t*) Z_Malloc(*gpMainMemZone, sizeof(floormove_t), PU_LEVSPEC, nullptr);
    floor = {};
    sector.specialdata = &floor;

    // Setup the floor thinker and return 'true' for success
    P_AddThinker(floor.thinker);
    floor.thinker.function = (think_t) &T_MoveFloor;
    floor.type = customFloor;
    floor.crush = floorDef.bCrush;
    floor.bDoFinishScript = floorDef.bDoFinishScript;
    floor.sector = &sector;
    floor.direction = (floorDef.destHeight >= sector.floorheight) ? +1 : -1;
    floor.floordestheight = floorDef.destHeight;
    floor.speed = std::abs(floorDef.speed);
    floor.moveSound = floorDef.moveSound;
    floor.moveSoundFreq = floorDef.moveSoundFreq;
    floor.stopSound = floorDef.stopSound;
    floor.finishScriptActionNum = floorDef.finishScriptActionNum;
    floor.finishScriptUserdata = floorDef.finishScriptUserdata;

    // Play the start sound, if any
    const sfxenum_t startSound = floorDef.startSound;

    if (startSound != sfx_None) {
        S_StartSound((mobj_t*) &sector.soundorg, startSound);
    }

    // This operation was successful
    return true;
}
#endif  // #if PSYDOOM_MODS
