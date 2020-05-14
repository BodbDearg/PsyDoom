#include "p_spec.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/st_main.h"
#include "doomdata.h"
#include "g_game.h"
#include "p_ceiling.h"
#include "p_doors.h"
#include "p_floor.h"
#include "p_inter.h"
#include "p_lights.h"
#include "p_plats.h"
#include "p_setup.h"
#include "p_switch.h"
#include "p_telept.h"
#include "p_tick.h"
#include "PsxVm/PsxVm.h"
#include "Wess/psxcd.h"
#include <algorithm>

// Definition for a flat or texture animation
struct animdef_t {
    bool        istexture;      // False for flats
    char        startname[9];   // Name of the first lump in the animation
    char        endname[9];     // Name of the last lump in the animation
    uint32_t    ticmask;        // New field for PSX: controls which game tics the animation will advance on
};

// New to PSX DOOM: definition for a thinker which performs an actionfunc after a delay
struct delayaction_t {
    thinker_t       thinker;
    int32_t         ticsleft;       // How many tics until we perform the actionfunc
    VmPtr<void(*)>  actionfunc;     // The action to perform after the delay
};

static_assert(sizeof(delayaction_t) == 20);

// Definitions for all flat and texture animations in the game
static const animdef_t gAnimDefs[MAXANIMS] = {
    { 0, "BLOOD1",   "BLOOD3",   3 },
    { 0, "BSLIME01", "BSLIME04", 3 },
    { 0, "CSLIME01", "CSLIME04", 3 },
    { 0, "ENERG01",  "ENERG04",  3 },
    { 0, "LAVA01",   "LAVA04",   3 },
    { 0, "WATER01",  "WATER04",  3 },
    { 0, "SLIME01",  "SLIME03",  3 },
    { 1, "BFALL1",   "BFALL4",   3 },
    { 1, "ENERGY01", "ENERGY04", 3 },
    { 1, "FIRE01",   "FIRE02",   3 },
    { 1, "FLAME01",  "FLAME03",  3 },
    { 1, "LAVWAL01", "LAVWAL03", 3 },
    { 1, "SFALL1",   "SFALL4",   3 },
    { 1, "SLIM01",   "SLIM04",   3 },
    { 1, "TVSNOW01", "TVSNOW03", 1 },
    { 1, "WARN01",   "WARN02",   3 }
};

const VmPtr<anim_t[MAXANIMS]>   gAnims(0x800863AC);
const VmPtr<VmPtr<anim_t>>      gpLastAnim(0x80078164);
const VmPtr<card_t>             gMapBlueKeyType(0x80077E9C);
const VmPtr<card_t>             gMapRedKeyType(0x8007821C);
const VmPtr<card_t>             gMapYellowKeyType(0x800780A0);

// TODO: REMOVE eventually
void _thunk_T_DelayedAction() noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Caches into RAM the textures for all animated flats and textures.
// Also sets up the spot in VRAM where these animations will go - they occupy the same spot as the base animation frame.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_InitPicAnims() noexcept {
    *gpLastAnim = &gAnims[0];

    for (int32_t animIdx = 0; animIdx < MAXANIMS; ++animIdx) {
        const animdef_t& animdef = gAnimDefs[animIdx];
        anim_t& lastanim = **gpLastAnim;

        if (animdef.istexture) {
            // Determine the lump range for the animation
            lastanim.basepic = R_TextureNumForName(animdef.startname);
            lastanim.picnum = R_TextureNumForName(animdef.endname);

            // Ignore this animation if it's not used in the level
            texture_t& basetex = (*gpTextures)[lastanim.basepic];

            if (basetex.texPageId == 0)
                continue;

            // For all frames in the animation, cache the lump in RAM and make it occupy the same VRAM spot as the base texture
            for (int32_t picNum = lastanim.basepic; picNum <= lastanim.picnum; ++picNum) {
                // Ensure the lump is cached in RAM for runtime (will need to be backed in RAM in case it's evicted from VRAM)
                W_CacheLumpNum(*gFirstTexLumpNum + picNum, PU_ANIMATION, false);

                texture_t& dstTex = (*gpTextures)[picNum];
                dstTex.texPageCoordX = basetex.texPageCoordX;
                dstTex.texPageCoordY = basetex.texPageCoordY;
                dstTex.texPageId = basetex.texPageId;
                dstTex.ppTexCacheEntries = basetex.ppTexCacheEntries;
            }
        } else {
            // Determine the lump range for the animation
            lastanim.basepic = R_FlatNumForName(animdef.startname);
            lastanim.picnum = R_FlatNumForName(animdef.endname);

            // Ignore this animation if it's not used in the level
            texture_t& basetex = (*gpFlatTextures)[lastanim.basepic];

            if (basetex.texPageId == 0)
                continue;

            // For all frames in the animation, cache the lump in RAM and make it occupy the same VRAM spot as the base texture
            for (int32_t picNum = lastanim.basepic; picNum <= lastanim.picnum; ++picNum) {
                // Ensure the lump is cached in RAM for runtime (will need to be backed in RAM in case it's evicted from VRAM)
                W_CacheLumpNum(*gFirstFlatLumpNum + picNum, PU_ANIMATION, false);

                texture_t& dstTex = (*gpFlatTextures)[picNum];
                dstTex.texPageCoordX = basetex.texPageCoordX;
                dstTex.texPageCoordY = basetex.texPageCoordY;
                dstTex.texPageId = basetex.texPageId;
                dstTex.ppTexCacheEntries = basetex.ppTexCacheEntries;
            }
        }
        
        // Init the anim state and move onto the next
        lastanim.istexture = animdef.istexture;
        lastanim.numpics = lastanim.picnum - lastanim.basepic + 1;
        lastanim.current = lastanim.basepic;
        lastanim.ticmask = animdef.ticmask;

        *gpLastAnim += 1;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the specified side of a line in the given sector
//------------------------------------------------------------------------------------------------------------------------------------------
side_t* getSide(const int32_t sectorIdx, const int32_t lineIdx, const int32_t sideIdx) noexcept {
    return gpSides->get() + gpSectors->get()[sectorIdx].lines[lineIdx]->sidenum[sideIdx];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the sector on the specified side of a line in the given sector
//------------------------------------------------------------------------------------------------------------------------------------------
sector_t* getSector(const int32_t sectorIdx, const int32_t lineIdx, const int32_t sideIdx) noexcept {
    return gpSides->get()[gpSectors->get()[sectorIdx].lines[lineIdx]->sidenum[sideIdx]].sector.get();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: tell if a line in sector is two sided
//------------------------------------------------------------------------------------------------------------------------------------------
bool twoSided(const int32_t sectorIdx, const int32_t lineIdx) noexcept {
    return (gpSectors->get()[sectorIdx].lines[lineIdx]->flags & ML_TWOSIDED);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the sector on the opposite side of the line to the given sector.
// Returns 'nullptr' if the line is single sided.
//------------------------------------------------------------------------------------------------------------------------------------------
sector_t* getNextSector(line_t& line, sector_t& sector) noexcept {
    if ((line.flags & ML_TWOSIDED) == 0)
        return nullptr;

    sector_t* const pFrontSec = line.frontsector.get();
    return (&sector == pFrontSec) ? line.backsector.get() : pFrontSec;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the lowest floor height for sectors surrounding the given sector, and including the input sector itself
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_FindLowestFloorSurrounding(sector_t& sector) noexcept {
    fixed_t lowestFloor = sector.floorheight;

    for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
        line_t& line = *sector.lines[lineIdx].get();
        sector_t* const pNextSector = getNextSector(line, sector);
        
        if (pNextSector && (pNextSector->floorheight < lowestFloor)) {
            lowestFloor = pNextSector->floorheight;
        }
    }

    return lowestFloor;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the highest floor height for sectors surrounding the given sector, NOT including the sector itself.
// The minimum height returned is -500.0.
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_FindHighestFloorSurrounding(sector_t& sector) noexcept {
    fixed_t highestFloor = -500 * FRACUNIT;
    
    for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
        line_t& line = *sector.lines[lineIdx].get();
        sector_t* const pNextSector = getNextSector(line, sector);

        if (pNextSector && (pNextSector->floorheight > highestFloor)) {
            highestFloor = pNextSector->floorheight;
        }
    }

    return highestFloor;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find the lowest floor height in the sectors surrounding a given sector which is higher than the given input 'base' height.
//
// PC-PSX: if there is no such sector, the input base height will be returned rather than an undefined/garbage height value.
// It also no longer overflows buffers when checking > 20 adjoining sectors.
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_FindNextHighestFloor(sector_t& sector, const fixed_t baseHeight) noexcept {
    // PC-PSX: rewrite this to eliminate potential buffer overflow and also an undefined answer when there is no next higher floor
    #if PC_PSX_DOOM_MODS
        fixed_t nextHighestFloor = INT32_MAX;

        for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
            line_t& line = *sector.lines[lineIdx].get();
            sector_t* const pNextSector = getNextSector(line, sector);

            if (!pNextSector)
                continue;

            const fixed_t floorH = pNextSector->floorheight;

            if ((floorH > baseHeight) && (floorH < nextHighestFloor)) {
                nextHighestFloor = floorH;
            }
        }
        
        // If there is no next highest floor return the input height rather than something undefined
        return (nextHighestFloor != INT32_MAX) ? nextHighestFloor : baseHeight;
    #else
        // Build a list of sector heights that are higher than the given height (20 max)
        fixed_t higherHeights[20];
        int32_t numHigherHeights = 0;

        for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
            line_t& line = *sector.lines[lineIdx].get();
            sector_t* const pNextSector = getNextSector(line, sector);

            if (pNextSector && (pNextSector->floorheight > baseHeight)) {
                higherHeights[numHigherHeights] = pNextSector->floorheight;
                numHigherHeights++;
            }
        }

        // Return the minimum of those sector heights that are higher
        fixed_t minHigherHeight = higherHeights[0];

        for (int32_t i = 1; i < numHigherHeights; ++i) {
            minHigherHeight = std::min(minHigherHeight, higherHeights[i]);
        }

        return minHigherHeight;
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find the lowest ceiling height in sectors surrounding the given sector.
// If there are no sectors surrounding the given sector, the highest possible height is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_FindLowestCeilingSurrounding(sector_t& sector) noexcept {
    fixed_t lowestHeight = INT32_MAX;
    
    for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
        line_t& line = *sector.lines[lineIdx].get();
        sector_t* const pNextSector = getNextSector(line, sector);

        if (pNextSector && (pNextSector->ceilingheight < lowestHeight)) {
            lowestHeight = pNextSector->ceilingheight;
        }
    }

    return lowestHeight;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find the highest ceiling height in sectors surrounding the given sector.
// Note: the minimum value returned by this function is '0'; this is also returned when there are no surrounding sectors.
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_FindHighestCeilingSurrounding(sector_t& sector) noexcept {
    fixed_t highestHeight = 0;

    for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
        line_t& line = *sector.lines[lineIdx].get();
        sector_t* const pNextSector = getNextSector(line, sector);

        if (pNextSector && (pNextSector->ceilingheight > highestHeight)) {
            highestHeight = pNextSector->ceilingheight;
        }
    }

    return highestHeight;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find the next sector in the global sectors list with a tag matching the given line's tag.
// The search starts at the given index + 1.
// Returns the index of the next matching sector found, or '-1' if there was no next matching sector.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t P_FindSectorFromLineTag(line_t& line, const int32_t searchStart) noexcept {
    const int32_t lineTag = line.tag;
    sector_t* const pSectors = gpSectors->get();
    const int32_t numSectors = *gNumSectors;

    for (int32_t sectorIdx = searchStart + 1; sectorIdx < numSectors; ++sectorIdx) {
        sector_t& sector = pSectors[sectorIdx];

        if (sector.tag == lineTag)
            return sectorIdx;
    }

    return -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find the minimum light level in the sectors surrounding the given sector which is less than the given max light level.
// If there is no light level less than the given max, then the max value is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t P_FindMinSurroundingLight(sector_t& sector, const int32_t maxLightLevel) noexcept {
    int32_t minLightLevel = maxLightLevel;
    
    for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
        line_t& line = *sector.lines[lineIdx].get();
        sector_t* const pNextSector = getNextSector(line, sector);

        if (pNextSector && (pNextSector->lightlevel < minLightLevel)) {
            minLightLevel = pNextSector->lightlevel;
        }
    }

    return minLightLevel;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does line specials for the given thing crossing the given line.
// Assumes the special for the line is not '0' and that the line has already been crossed.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_CrossSpecialLine(line_t& line, mobj_t& mobj) noexcept {
    // If the object triggering the special is not a player then only certain specials can be triggered.
    // Some things like projectiles are also not allowed to trigger specials.
    if (!mobj.player) {
        // Only monsters and (oddly) weapons and barrels can trigger specials
        if ((mobj.type >= MT_TROOPSHOT) && (mobj.type <= MT_BFG))
            return;

        // Monsters can only activate certain types of triggers: see if this is one of them
        bool bCanTrigger = false;

        switch (line.special) {
            case 4:     // Once:  Raise door
            case 10:    // Once:  Platform - Down, wait, up, stay
            case 39:    // Once:  Teleport (regular, everyone)
            case 88:    // Multi: Platform - down, wait, up, stay
            case 97:    // Multi: Teleport (regular, everyone)
            case 125:   // Once:  Teleport (monsters ONLY!)
            case 126:   // Multi: Teleport (monsters ONLY!)
                bCanTrigger = true;
                break;

            default:
                break;
        }

        if (!bCanTrigger)
            return;
    }

    switch (line.special) {
        //----------------------------------------------------------------------------------------------------------------------------------
        // Once only triggers
        //----------------------------------------------------------------------------------------------------------------------------------
        
        // Open door
        case 2:
            EV_DoDoor(line, Open);
            line.special = 0;
            break;

        // Close door
        case 3:
            EV_DoDoor(line, Close);
            line.special = 0;
            break;

        // Raise door
        case 4:
            EV_DoDoor(line, Normal);
            line.special = 0;
            break;

        // Raise floor
        case 5:
            EV_DoFloor(line, raiseFloor);
            line.special = 0;
            break;

        // Fast ceiling crush & raise
        case 6:
            EV_DoCeiling(line, fastCrushAndRaise);
            line.special = 0;
            break;

        // Build stairs
        case 8:
            EV_BuildStairs(line, build8);
            line.special = 0;
            break;

        // Platform - Down, wait, up, stay
        case 10:
            EV_DoPlat(line, downWaitUpStay, 0);
            line.special = 0;
            break;

        // Light turn on - Use the brightest adjacent sector light level
        case 12:
            EV_LightTurnOn(line, 0);
            line.special = 0;
            break;

        // Light turn on to '255'
        case 13:
            EV_LightTurnOn(line, 255);
            line.special = 0;
            break;

        // Close door and open in 30 seconds
        case 16:
            EV_DoDoor(line, Close30ThenOpen);
            line.special = 0;
            break;

        // Start light strobing
        case 17:
            EV_StartLightStrobing(line);
            line.special = 0;
            break;

        // Raise floor to nearest height and change texture
        case 19:
            EV_DoFloor(line, lowerFloor);
            line.special = 0;
            break;

        // Raise floor to nearest height and change texture
        case 22:
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            line.special = 0;
            break;

        // Ceiling Crush and Raise
        case 25:
            EV_DoCeiling(line, crushAndRaise);
            line.special = 0;
            break;

        // Raise floor to shortest texture height on either side of lines
        case 30:
            EV_DoFloor(line, raiseToTexture);
            line.special = 0;
            break;

        // Lights very dark
        case 35:
            EV_LightTurnOn(line, 35);
            line.special = 0;
            break;

        // Lower Floor (turbo)
        case 36:
            EV_DoFloor(line, turboLower);
            line.special = 0;
            break;

        // Lower floor to lowest surrounding floor and change texture
        case 37:
            EV_DoFloor(line, lowerAndChange);
            line.special = 0;
            break;

        // Lower floor to the lowest surrounding floor height
        case 38:
            EV_DoFloor(line, lowerFloorToLowest);
            line.special = 0;
            break;

        // Teleport (regular, everyone)
        case 39:
            EV_Teleport(line, mobj);
            line.special = 0;
            break;

        // Raise ceiling to highest and lower floor to lowest
        case 40:
            EV_DoCeiling(line, raiseToHighest);
            EV_DoFloor(line, lowerFloorToLowest);
            line.special = 0;
            break;

        // Ceiling crush
        case 44:
            EV_DoCeiling(line, lowerAndCrush);
            line.special = 0;
            break;

        // Exit the level (regular)
        case 52:
            G_ExitLevel();
            line.special = 0;
            break;

        // Perpetual platform raise
        case 53:
            EV_DoPlat(line, perpetualRaise, 0);
            line.special = 0;
            break;

        // Platform Stop
        case 54:
            EV_StopPlat(line);
            line.special = 0;
            break;

        // Raise floor crush
        case 56:
            EV_DoFloor(line, raiseFloorCrush);
            line.special = 0;
            break;

        // Ceiling crush stop
        case 57:
            EV_CeilingCrushStop(line);
            line.special = 0;
            break;

        // Raise floor 24
        case 58:
            EV_DoFloor(line, raiseFloor24);
            line.special = 0;
            break;

        // Raise floor 24 and change
        case 59:
            EV_DoFloor(line, raiseFloor24AndChange);
            line.special = 0;
            break;

        // Build stairs turbo 16
        case 100:
            EV_BuildStairs(line, turbo16);
            line.special = 0;
            break;

        // Turn lights off in sector (tag)
        case 104:
            EV_TurnTagLightsOff(line);
            line.special = 0;
            break;

        // Blazing door raise (faster than turbo!)
        case 108:
            EV_DoDoor(line, BlazeRaise);
            line.special = 0;
            break;

        // Blazing door open (faster than turbo!)
        case 109:
            EV_DoDoor(line, BlazeOpen);
            line.special = 0;
            break;

        // Blazing door close (faster than turbo!)
        case 110:
            EV_DoDoor(line, BlazeClose);
            line.special = 0;
            break;

        // Raise floor to nearest surrounding floor
        case 119:
            EV_DoFloor(line, raiseFloorToNearest);
            line.special = 0;
            break;

        // Blazing platform - down, wait, up, stay
        case 121:
            EV_DoPlat(line, blazeDWUS, 0);
            line.special = 0;
            break;

        // Secret exit
        case 124:
            G_SecretExitLevel(line.tag);
            break;

        // Teleport (monsters ONLY!)
        case 125: {
            if (!mobj.player) {
                EV_Teleport(line, mobj);
                line.special = 0;
            }
        }   break;

        // Silent ceiling crush & raise
        case 141:
            EV_DoCeiling(line, silentCrushAndRaise);
            line.special = 0;
            break;

        // New for PSX: play the Club Doom CD audio!
        case 142:
            S_StopMusic();
            psxcd_play_at_andloop(gCDTrackNum[cdmusic_club_doom], *gCdMusicVol, 0, 0, gCDTrackNum[cdmusic_club_doom], *gCdMusicVol, 0, 0);
            line.special = 0;
            break;

        //----------------------------------------------------------------------------------------------------------------------------------
        // Repeatable triggers
        //----------------------------------------------------------------------------------------------------------------------------------

        // Ceiling crush
        case 72:
            EV_DoCeiling(line, lowerAndCrush);
            break;

        // Ceiling crush and raise
        case 73:
            EV_DoCeiling(line, crushAndRaise);
            break;

        // Ceiling crush stop
        case 74:
            EV_CeilingCrushStop(line);
            break;

        // Close door
        case 75:
            EV_DoDoor(line, Close);
            break;

        // Close door for 30 seconds then open
        case 76:
            EV_DoDoor(line, Close30ThenOpen);
            break;

        // Fast ceiling crush & raise
        case 77:
            EV_DoCeiling(line, fastCrushAndRaise);
            break;

        // Lights very dark
        case 79:
            EV_LightTurnOn(line, 35);
            break;

        // Light turn on - use light level of brightest adjacent sector
        case 80:
            EV_LightTurnOn(line, 0);
            break;

        // Light turn on to '255'
        case 81:
            EV_LightTurnOn(line, 255);
            break;

        // Lower floor to lowest
        case 82:
            EV_DoFloor(line, lowerFloorToLowest);
            break;

        // Lower floor
        case 83:
            EV_DoFloor(line, lowerFloor);
            break;

        // Lower floor to lowest surrounding floor and change texture
        case 84:
            EV_DoFloor(line, lowerAndChange);
            break;

        // Open door
        case 86:
            EV_DoDoor(line, Open);
            break;

        // Perpetual platform raise
        case 87:
            EV_DoPlat(line, perpetualRaise, 0);
            break;

        // Platform - down, wait, up, stay
        case 88:
            EV_DoPlat(line, downWaitUpStay, 0);
            break;

        // Platform stop
        case 89:
            EV_StopPlat(line);
            break;

        // Raise Door
        case 90:
            EV_DoDoor(line, Normal);
            break;

        // Raise floor
        case 91:
            EV_DoFloor(line, raiseFloor);
            break;

        // Raise floor 24
        case 92:
            EV_DoFloor(line, raiseFloor24);
            break;

        // Raise floor 24 and change texture
        case 93:
            EV_DoFloor(line, raiseFloor24AndChange);
            break;

        // Raise floor and crush
        case 94:
            EV_DoFloor(line, raiseFloorCrush);
            break;

        // Raise floor to nearest height and change texture
        case 95:
            EV_DoPlat(line, raiseToNearestAndChange, 0);
            break;

        // Raise floor to shortest texture height on either side of lines
        case 96:
            EV_DoFloor(line, raiseToTexture);
            break;

        // Teleport (regular, everyone)
        case 97:
            EV_Teleport(line, mobj);
            break;

        // Lower Floor (turbo)
        case 98:
            EV_DoFloor(line, turboLower);
            break;

        // Blazing door raise (faster than turbo!)
        case 105:
            EV_DoDoor(line, BlazeRaise);
            break;

        // Blazing door open (faster than turbo!)
        case 106:
            EV_DoDoor(line, BlazeOpen);
            break;

        // Blazing door close (faster than turbo!)
        case 107:
            EV_DoDoor(line, BlazeClose);
            break;

        // Blazing platform - down, wait, up, stay
        case 120:
            EV_DoPlat(line, blazeDWUS, 0);
            break;

        // Teleport (monsters ONLY!)
        case 126: {
            if (!mobj.player) {
                EV_Teleport(line, mobj);
            }
        }   break;

        default:
            break;
    }
}

void P_ShootSpecialLine() noexcept {
loc_80026D40:
    sp -= 0x18;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    v0 = lw(a0 + 0x80);
    s0 = a1;
    if (v0 != 0) goto loc_80026D6C;
    v1 = lw(s0 + 0x14);
    v0 = 0x2E;                                          // Result = 0000002E
    if (v1 != v0) goto loc_80026DF4;
loc_80026D6C:
    v1 = lw(s0 + 0x14);
    v0 = 0x2E;                                          // Result = 0000002E
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0x2F);
        if (bJump) goto loc_80026DB8;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0x18;                                      // Result = 00000018
        if (bJump) goto loc_80026D94;
    }
    a0 = s0;
    if (v1 == v0) goto loc_80026DA8;
    goto loc_80026DF4;
loc_80026D94:
    v0 = 0x2F;                                          // Result = 0000002F
    a0 = s0;
    if (v1 == v0) goto loc_80026DD4;
    goto loc_80026DF4;
loc_80026DA8:
    a1 = 3;                                             // Result = 00000003
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    goto loc_80026DE0;
loc_80026DB8:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    a0 = s0;
    if (v0 == 0) goto loc_80026DF4;
    a1 = 1;                                             // Result = 00000001
    goto loc_80026DEC;
loc_80026DD4:
    a1 = 3;                                             // Result = 00000003
    a2 = 0;                                             // Result = 00000000
    v0 = EV_DoPlat(*vmAddrToPtr<line_t>(a0), (plattype_e) a1, a2);
loc_80026DE0:
    a0 = s0;
    if (v0 == 0) goto loc_80026DF4;
    a1 = 0;                                             // Result = 00000000
loc_80026DEC:
    P_ChangeSwitchTexture(*vmAddrToPtr<line_t>(a0), a1);
loc_80026DF4:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called every game tic that the player position is in a sector which has a special.
// Credits secrets found by entering sectors and damage taken from floors (lava, slime etc.).
//------------------------------------------------------------------------------------------------------------------------------------------
void P_PlayerInSpecialSector(player_t& player) noexcept {
    sector_t& sector = *player.mo->subsector->sector;

    // Logic only runs when the player is on the floor
    if (player.mo->z != sector.floorheight)
        return;
    
    switch (sector.special) {
        // Hellslime damage
        case 5: {
            if (!player.powers[pw_ironfeet]) {
                gStatusBar->specialFace = f_mowdown;

                if ((*gGameTic > *gPrevGameTic) && (*gGameTic % 16 == 0)) {     // Apply roughly every 1 second
                    P_DamageMObj(*player.mo, nullptr, nullptr, 10);
                }
            }
        }   break;

        // Nukage damage
        case 7: {
            if (!player.powers[pw_ironfeet]) {
                gStatusBar->specialFace = f_mowdown;

                if ((*gGameTic > *gPrevGameTic) && (*gGameTic % 16 == 0)) {     // Apply roughly every 1 second
                    P_DamageMObj(*player.mo, nullptr, nullptr, 5);
                }
            }
        }   break;

        // Strobe hurt
        case 4:
        // Super hellslime damage
        case 16: {
            if ((!player.powers[pw_ironfeet]) || (P_Random() < 5)) {    // Occasionally damages even if a radiation suit is worn
                gStatusBar->specialFace = f_mowdown;

                if ((*gGameTic > *gPrevGameTic) && (*gGameTic % 16 == 0)) {     // Apply roughly every 1 second
                    P_DamageMObj(*player.mo, nullptr, nullptr, 20);
                }
            }
        }   break;

        // Secret sector
        case 9: {
            player.secretcount += 1;
            sector.special = 0;         // Consume the special so it's only counted once!
        }   break;

        default:
            I_Error("P_PlayerInSpecialSector: unknown special %i", sector.special);
            return;
    }
}

void P_UpdateSpecials() noexcept {
loc_80026FC8:
    sp -= 0x28;
    v0 = *gpLastAnim;
    a1 = 0x80080000;                                    // Result = 80080000
    a1 += 0x63AC;                                       // Result = gAnims_1[0] (800863AC)
    sw(ra, sp + 0x20);
    sw(s1, sp + 0x1C);
    v0 = (a1 < v0);
    sw(s0, sp + 0x18);
    if (v0 == 0) goto loc_800270EC;
    a2 = -1;                                            // Result = FFFFFFFF
    a0 = a1 + 0x10;                                     // Result = gAnims_1[4] (800863BC)
    t3 = *gGameTic;
    t2 = *gpTextureTranslation;
    t1 = *gpTextures;
    t0 = *gpFlatTranslation;
    a3 = *gpFlatTextures;
loc_8002701C:
    v0 = lw(a0 + 0x4);
    v0 &= t3;
    if (v0 != 0) goto loc_800270D8;
    v0 = lw(a1);
    if (v0 == 0) goto loc_8002708C;
    v0 = lw(a0);
    v1 = lw(a0 - 0xC);
    v0++;
    v1 = (i32(v1) < i32(v0));
    sw(v0, a0);
    if (v1 == 0) goto loc_80027064;
    v0 = lw(a0 - 0x8);
    sw(v0, a0);
loc_80027064:
    v0 = lw(a0 - 0x8);
    v1 = lw(a0);
    v0 <<= 2;
    v0 += t2;
    sw(v1, v0);
    v0 = lw(a0);
    v0 <<= 5;
    v0 += t1;
    goto loc_800270D4;
loc_8002708C:
    v0 = lw(a0);
    v1 = lw(a0 - 0xC);
    v0++;
    v1 = (i32(v1) < i32(v0));
    sw(v0, a0);
    if (v1 == 0) goto loc_800270B0;
    v0 = lw(a0 - 0x8);
    sw(v0, a0);
loc_800270B0:
    v0 = lw(a0 - 0x8);
    v1 = lw(a0);
    v0 <<= 2;
    v0 += t0;
    sw(v1, v0);
    v0 = lw(a0);
    v0 <<= 5;
    v0 += a3;
loc_800270D4:
    sw(a2, v0 + 0x1C);
loc_800270D8:
    v0 = *gpLastAnim;
    a1 += 0x18;
    v0 = (a1 < v0);
    a0 += 0x18;
    if (v0 != 0) goto loc_8002701C;
loc_800270EC:
    v0 = lw(gp + 0x970);                                // Load from: gNumLinespecials (80077F50)
    s1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80027264;
    a1 = *gpSides;
    t1 = 0x10000;                                       // Result = 00010000
    a3 = 0xFF7F0000;                                    // Result = FF7F0000
    t0 = 0xFFFF0000;                                    // Result = FFFF0000
    a2 = 0x80090000;                                    // Result = 80090000
    a2 += 0x757C;                                       // Result = gpLineSpecialList[0] (8009757C)
loc_80027118:
    a0 = lw(a2);
    v1 = lw(a0 + 0x14);
    v0 = 0xC9;                                          // Result = 000000C9
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0xCA);
        if (bJump) goto loc_80027188;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0xC8;                                      // Result = 000000C8
        if (bJump) goto loc_80027148;
    }
    if (v1 == v0) goto loc_80027164;
    goto loc_80027250;
loc_80027148:
    v0 = 0xCA;                                          // Result = 000000CA
    {
        const bool bJump = (v1 == v0);
        v0 = 0xCB;                                      // Result = 000000CB
        if (bJump) goto loc_800271DC;
    }
    if (v1 == v0) goto loc_80027200;
    goto loc_80027250;
loc_80027164:
    v1 = lw(a0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += a1;
    v1 = lw(v0);
    v1 += t1;
    goto loc_800271AC;
loc_80027188:
    v1 = lw(a0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += a1;
    v1 = lw(v0);
    v1 += t0;
loc_800271AC:
    sw(v1, v0);
    v1 = lw(a0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += a1;
    v1 = lw(v0);
    v1 &= a3;
    sw(v1, v0);
    goto loc_80027250;
loc_800271DC:
    v1 = lw(a0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += a1;
    v1 = lw(v0 + 0x4);
    v1 += t1;
    goto loc_80027224;
loc_80027200:
    v1 = lw(a0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += a1;
    v1 = lw(v0 + 0x4);
    v1 += t0;
loc_80027224:
    sw(v1, v0 + 0x4);
    v1 = lw(a0 + 0x1C);
    v0 = v1 << 1;
    v0 += v1;
    v0 <<= 3;
    v0 += a1;
    v1 = lw(v0 + 0x4);
    v1 &= a3;
    sw(v1, v0 + 0x4);
loc_80027250:
    v0 = lw(gp + 0x970);                                // Load from: gNumLinespecials (80077F50)
    s1++;
    v0 = (i32(s1) < i32(v0));
    a2 += 4;
    if (v0 != 0) goto loc_80027118;
loc_80027264:
    s1 = 0;                                             // Result = 00000000
    s0 = 0;                                             // Result = 00000000
loc_8002726C:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B8;                                       // Result = gButtonList_1[3] (800977B8)
    at += s0;
    v1 = lw(at);
    if (i32(v1) <= 0) goto loc_80027400;
    v0 = *gCurPlayerIndex;
    v0 <<= 2;
    at = 0x80070000;                                    // Result = 80070000
    at += 0x7FBC;                                       // Result = gPlayersElapsedVBlanks[0] (80077FBC)
    at += v0;
    v0 = lw(at);
    v0 = v1 - v0;
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B8;                                       // Result = gButtonList_1[3] (800977B8)
    at += s0;
    sw(v0, at);
    {
        const bool bJump = (i32(v0) > 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_80027400;
    }
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B0;                                       // Result = gButtonList_1[1] (800977B0)
    at += s0;
    v1 = lw(at);
    if (v1 == v0) goto loc_80027344;
    v0 = 2;                                             // Result = 00000002
    if (v1 == 0) goto loc_800272FC;
    if (v1 == v0) goto loc_8002738C;
    goto loc_800273D0;
loc_800272FC:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    at += s0;
    v0 = lw(at);
    v1 = lw(v0 + 0x1C);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B4;                                       // Result = gButtonList_1[2] (800977B4)
    at += s0;
    a0 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v1 = *gpSides;
    v0 <<= 3;
    v0 += v1;
    sw(a0, v0 + 0x8);
    goto loc_800273D0;
loc_80027344:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    at += s0;
    v0 = lw(at);
    v1 = lw(v0 + 0x1C);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B4;                                       // Result = gButtonList_1[2] (800977B4)
    at += s0;
    a0 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v1 = *gpSides;
    v0 <<= 3;
    v0 += v1;
    sw(a0, v0 + 0x10);
    goto loc_800273D0;
loc_8002738C:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    at += s0;
    v0 = lw(at);
    v1 = lw(v0 + 0x1C);
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77B4;                                       // Result = gButtonList_1[2] (800977B4)
    at += s0;
    a0 = lw(at);
    v0 = v1 << 1;
    v0 += v1;
    v1 = *gpSides;
    v0 <<= 3;
    v0 += v1;
    sw(a0, v0 + 0xC);
loc_800273D0:
    at = 0x80090000;                                    // Result = 80090000
    at += 0x77BC;                                       // Result = gButtonList_1[4] (800977BC)
    at += s0;
    a0 = lw(at);
    a1 = sfx_swtchn;
    S_StartSound(vmAddrToPtr<mobj_t>(a0), (sfxenum_t) a1);
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    a0 += s0;
    a1 = 0;
    a2 = 0x14;
    _thunk_D_memset();
loc_80027400:
    s1++;
    v0 = (i32(s1) < 0x10);
    s0 += 0x14;
    if (v0 != 0) goto loc_8002726C;
    
    if (*gbIsSkyVisible && gUpdateFireSkyFunc) {
        gUpdateFireSkyFunc(**gpSkyTexture);
    }
    
    ra = lw(sp + 0x20);
    s1 = lw(sp + 0x1C);
    s0 = lw(sp + 0x18);
    sp += 0x28;
    return;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Activates a 'donut' mover effect on the inner/hole sectors matching the given line's tag.
// The donut effect raises the adjacent sector of the donut hole (outer donut) to the height of a sector adjacent to  that.
// In addition to this the outer donut part has all floor damage removed and it's floor texture changed to the outermost sector's floor.
// The inner part of the donut is also lowered to match the height of the outermost sector.
//------------------------------------------------------------------------------------------------------------------------------------------
bool EV_DoDonut(line_t& line) noexcept {
    bool bActivatedMovers = false;

    for (int32_t sectorIdx = P_FindSectorFromLineTag(line, -1); sectorIdx >= 0; sectorIdx = P_FindSectorFromLineTag(line, sectorIdx)) {
        // Only spawn a mover if there isn't already a special operating on this sector.
        // This sector will be the inner part of the 'donut', the bit that is lowered:
        sector_t& sector = gpSectors->get()[sectorIdx];

        if (sector.specialdata)
            continue;

        bActivatedMovers = true;

        // Get the next sector beyond the first line in this sector, that will be the outer ring of the 'donut', or the part that is raised:
        sector_t* const pNextSector = getNextSector(sector.lines->get()[0], sector);

        #if PC_PSX_DOOM_MODS
            // PC-PSX: safety in case the level is setup wrong - if this line is not two sided then just ignore the command
            if (!pNextSector)
                continue;
        #endif

        // Run through all the lines of the next sector and try to find the first that has a back sector that isn't the inner part of the donut.
        // This back sector gives us the floor height to raise to and also the floor pic to change to.
        for (int32_t lineIdx = 0; lineIdx < pNextSector->linecount; ++lineIdx) {
            line_t& nextSecLine = pNextSector->lines->get()[lineIdx];

            #if PC_PSX_DOOM_MODS
                // PC-PSX: safety - allow non two sided lines here, just skip over them
                if (!nextSecLine.backsector)
                    continue;
            #endif

            // Ignore this line if the back sector is the same as the inner part of the donut
            sector_t& backSector = *nextSecLine.backsector;

            if (&backSector == &sector)
                continue;

            // Create the mover for the outer part of the donut.
            // This raises the floor to the height of the back sector we just found and changes the texture to that.
            // This is normally used to raise slime and change the slime texture.
            {
                floormove_t& floorMove = *(floormove_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(floormove_t), PU_LEVSPEC, nullptr);
                P_AddThinker(floorMove.thinker);
                pNextSector->specialdata = &floorMove;

                floorMove.thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_MoveFloor);
                floorMove.type = donutRaise;
                floorMove.sector = pNextSector;
                floorMove.direction = 1;
                floorMove.crush = false;
                floorMove.speed = FLOORSPEED / 2;
                floorMove.newspecial = 0;                               // Remove slime etc. damage
                floorMove.texture = (int16_t) backSector.floorpic;      // Change floor to this picture
                floorMove.floordestheight = backSector.floorheight;
            }

            // Create the mover for the inner part or the 'hole' of the donut.
            // This sector just lowers down to the height of the back sector we just found.
            {
                floormove_t& floorMove = *(floormove_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(floormove_t), PU_LEVSPEC, nullptr);
                P_AddThinker(floorMove.thinker);
                sector.specialdata = &floorMove;

                floorMove.thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_MoveFloor);
                floorMove.type = lowerFloor;
                floorMove.sector = &sector;
                floorMove.direction = -1;
                floorMove.crush = false;
                floorMove.speed = FLOORSPEED / 2;
                floorMove.floordestheight = backSector.floorheight;
            }

            // We're done doing the donut effect now
            break;
        }
    }

    return bActivatedMovers;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedule an action to be invoked after the specified number of tics
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_ScheduleDelayedAction(const int32_t delayTics, void (* const actionFunc)()) noexcept {
    delayaction_t& delayed = *(delayaction_t*) Z_Malloc(*gpMainMemZone->get(), sizeof(delayaction_t), PU_LEVSPEC, nullptr);
    P_AddThinker(delayed.thinker);

    delayed.thinker.function = PsxVm::getNativeFuncVmAddr(_thunk_T_DelayedAction);
    delayed.ticsleft = delayTics;
    delayed.actionfunc = PsxVm::getNativeFuncVmAddr(actionFunc);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker function for performing a delayed action: performs the action after the delay time has passed and removes the thinker when done
//------------------------------------------------------------------------------------------------------------------------------------------
static void T_DelayedAction(delayaction_t& action) noexcept {
    if (--action.ticsleft <= 0) {
        // FIXME: use a native function call eventually
        const VmFunc actionFunc = PsxVm::getVmFuncForAddr(action.actionfunc);
        actionFunc();
        P_RemoveThinker(action.thinker);
    }
}

// TODO: REMOVE eventually
void _thunk_T_DelayedAction() noexcept {
    T_DelayedAction(*vmAddrToPtr<delayaction_t>(*PsxVm::gpReg_a0));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedules the level to end in 4 tics and go to the next map; this is the usual method of exiting a level
//------------------------------------------------------------------------------------------------------------------------------------------
void G_ExitLevel() noexcept {
    *gNextMap += *gGameMap + 1;
    P_ScheduleDelayedAction(4, G_CompleteLevel);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedules the level to end in 4 tics and go to the specified map; used for entering/exiting secret maps
//------------------------------------------------------------------------------------------------------------------------------------------
void G_SecretExitLevel(const int32_t nextMap) noexcept {
    *gNextMap = nextMap;
    P_ScheduleDelayedAction(4, G_CompleteLevel);
}

void P_SpawnSpecials() noexcept {
loc_8002784C:
    v0 = *gNumSectors;
    sp -= 0x38;
    sw(s0, sp + 0x28);
    s0 = *gpSectors;
    sw(s1, sp + 0x2C);
    s1 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x30);
    if (i32(v0) <= 0) goto loc_80027A54;
loc_80027874:
    v1 = lw(s0 + 0x14);
    v0 = 0xC;                                           // Result = 0000000C
    if (v1 == 0) goto loc_80027A38;
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0xD);
        if (bJump) goto loc_800279B8;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 3;                                         // Result = 00000003
        if (bJump) goto loc_800278F4;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 4);
        if (bJump) goto loc_8002796C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 1;                                         // Result = 00000001
        if (bJump) goto loc_800278BC;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = 2;                                         // Result = 00000002
        if (bJump) goto loc_80027954;
    }
    a0 = s0;
    if (v1 == v0) goto loc_80027964;
    s1++;
    goto loc_80027A3C;
loc_800278BC:
    v0 = 9;                                             // Result = 00000009
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0xA);
        if (bJump) goto loc_80027988;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 8;                                         // Result = 00000008
        if (bJump) goto loc_800278E0;
    }
    a0 = s0;
    if (v1 == v0) goto loc_80027978;
    s1++;
    goto loc_80027A3C;
loc_800278E0:
    v0 = 0xA;                                           // Result = 0000000A
    if (v1 == v0) goto loc_800279A8;
    s1++;
    goto loc_80027A3C;
loc_800278F4:
    v0 = 0xC8;                                          // Result = 000000C8
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0xC9);
        if (bJump) goto loc_800279F8;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0xE;                                       // Result = 0000000E
        if (bJump) goto loc_8002792C;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0xE);
        if (bJump) goto loc_800279D4;
    }
    a0 = s0;
    if (v0 != 0) goto loc_800279C8;
    v0 = 0x11;                                          // Result = 00000011
    if (v1 == v0) goto loc_800279E8;
    s1++;
    goto loc_80027A3C;
loc_8002792C:
    v0 = 0xCA;                                          // Result = 000000CA
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0xCA);
        if (bJump) goto loc_80027A1C;
    }
    a0 = s0;
    if (v0 != 0) goto loc_80027A0C;
    v0 = 0xCC;                                          // Result = 000000CC
    a1 = 4;                                             // Result = 00000004
    if (v1 == v0) goto loc_80027A2C;
    s1++;
    goto loc_80027A3C;
loc_80027954:
    a0 = s0;
    P_SpawnLightFlash(*vmAddrToPtr<sector_t>(a0));
    s1++;
    goto loc_80027A3C;
loc_80027964:
    a1 = 8;                                             // Result = 00000008
    goto loc_80027A2C;
loc_8002796C:
    a0 = s0;
    a1 = 0xF;                                           // Result = 0000000F
    goto loc_80027A2C;
loc_80027978:
    a1 = 0;                                             // Result = 00000000
    P_SpawnGlowingLight(*vmAddrToPtr<sector_t>(a0), (glowtype_e) a1);
    s1++;
    goto loc_80027A3C;
loc_80027988:
    v0 = *gTotalSecret;
    v0++;
    at = 0x80070000;                                    // Result = 80070000
    *gTotalSecret = v0;
    s1++;
    goto loc_80027A3C;
loc_800279A8:
    a0 = s0;
    P_SpawnDoorCloseIn30(*vmAddrToPtr<sector_t>(a0));
    s1++;
    goto loc_80027A3C;
loc_800279B8:
    a0 = s0;
    a1 = 0xF;                                           // Result = 0000000F
    a2 = 1;                                             // Result = 00000001
    goto loc_80027A30;
loc_800279C8:
    a1 = 8;                                             // Result = 00000008
    a2 = 1;                                             // Result = 00000001
    goto loc_80027A30;
loc_800279D4:
    a0 = s0;
    a1 = s1;
    P_SpawnDoorRaiseIn5Mins(*vmAddrToPtr<sector_t>(a0), a1);
    s1++;
    goto loc_80027A3C;
loc_800279E8:
    a0 = s0;
    P_SpawnFireFlicker(*vmAddrToPtr<sector_t>(a0));
    s1++;
    goto loc_80027A3C;
loc_800279F8:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    P_SpawnGlowingLight(*vmAddrToPtr<sector_t>(a0), (glowtype_e) a1);
    s1++;
    goto loc_80027A3C;
loc_80027A0C:
    a1 = 2;                                             // Result = 00000002
    P_SpawnGlowingLight(*vmAddrToPtr<sector_t>(a0), (glowtype_e) a1);
    s1++;
    goto loc_80027A3C;
loc_80027A1C:
    a0 = s0;
    P_SpawnRapidStrobeFlash(*vmAddrToPtr<sector_t>(a0));
    s1++;
    goto loc_80027A3C;
loc_80027A2C:
    a2 = 0;                                             // Result = 00000000
loc_80027A30:
    P_SpawnStrobeFlash(*vmAddrToPtr<sector_t>(a0), a1, a2);
loc_80027A38:
    s1++;
loc_80027A3C:
    v0 = *gNumSectors;
    v0 = (i32(s1) < i32(v0));
    s0 += 0x5C;
    if (v0 != 0) goto loc_80027874;
loc_80027A54:
    v0 = *gNumLines;
    sw(0, gp + 0x970);                                  // Store to: gNumLinespecials (80077F50)
    s1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80027AD0;
    a2 = 0x80090000;                                    // Result = 80090000
    a2 += 0x757C;                                       // Result = gpLineSpecialList[0] (8009757C)
    a1 = v0;
    a0 = *gpLines;
loc_80027A7C:
    v1 = lw(a0 + 0x14);
    v0 = (i32(v1) < 0xCC);
    s1++;
    if (v0 == 0) goto loc_80027AC4;
    v0 = (i32(v1) < 0xC8);
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(s1) < i32(a1));
        if (bJump) goto loc_80027AC8;
    }
    v1 = lw(gp + 0x970);                                // Load from: gNumLinespecials (80077F50)
    v0 = (i32(v1) < 0x20);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80027AC4;
    }
    v0 += a2;
    sw(a0, v0);
    v0 = v1 + 1;
    sw(v0, gp + 0x970);                                 // Store to: gNumLinespecials (80077F50)
loc_80027AC4:
    v0 = (i32(s1) < i32(a1));
loc_80027AC8:
    a0 += 0x4C;
    if (v0 != 0) goto loc_80027A7C;
loc_80027AD0:
    v0 = *gNumSectors;
    at = 0x80070000;                                    // Result = 80070000
    sw(0, at + 0x7F88);                                 // Store to: gMapBossSpecialFlags (80077F88)
    s1 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_80027BA4;
    a2 = 0x80010000;                                    // Result = 80010000
    a2 += 0xE1C;                                        // Result = JumpTable_P_SpawnSpecials[0] (80010E1C)
    a1 = v0;
    a0 = *gpSectors;
loc_80027AFC:
    v0 = lw(a0 + 0x18);
    v1 = v0 - 0x29A;
    v0 = (v1 < 6);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80027B94;
    }
    v0 += a2;
    v0 = lw(v0);
    switch (v0) {
        case 0x80027B2C: goto loc_80027B2C;
        case 0x80027B3C: goto loc_80027B3C;
        case 0x80027B4C: goto loc_80027B4C;
        case 0x80027B5C: goto loc_80027B5C;
        case 0x80027B6C: goto loc_80027B6C;
        case 0x80027B7C: goto loc_80027B7C;
        default: jump_table_err(); break;
    }
loc_80027B2C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v0 |= 1;
    goto loc_80027B8C;
loc_80027B3C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v0 |= 2;
    goto loc_80027B8C;
loc_80027B4C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v0 |= 4;
    goto loc_80027B8C;
loc_80027B5C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v0 |= 8;
    goto loc_80027B8C;
loc_80027B6C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v0 |= 0x10;
    goto loc_80027B8C;
loc_80027B7C:
    v0 = 0x80070000;                                    // Result = 80070000
    v0 = lw(v0 + 0x7F88);                               // Load from: gMapBossSpecialFlags (80077F88)
    v0 |= 0x20;
loc_80027B8C:
    at = 0x80070000;                                    // Result = 80070000
    sw(v0, at + 0x7F88);                                // Store to: gMapBossSpecialFlags (80077F88)
loc_80027B94:
    s1++;
    v0 = (i32(s1) < i32(a1));
    a0 += 0x5C;
    if (v0 != 0) goto loc_80027AFC;
loc_80027BA4:
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    v1 = 1;                                             // Result = 00000001
    at = 0x80070000;                                    // Result = 80070000
    sw(v1, at + 0x7E9C);                                // Store to: gMapBlueKeyType (80077E9C)
    v1 = 2;                                             // Result = 00000002
    a0 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 -= 0x14;                                         // Result = gMObjHead[0] (800A8E90)
    at = 0x80080000;                                    // Result = 80080000
    sw(v1, at - 0x7F60);                                // Store to: gMapYellowKeyType (800780A0)
    at = 0x80080000;                                    // Result = 80080000
    sw(0, at - 0x7DE4);                                 // Store to: gMapRedKeyType (8007821C)
    t3 = 0x29;                                          // Result = 00000029
    if (a0 == v0) goto loc_80027C5C;
    t2 = 0x28;                                          // Result = 00000028
    t1 = 0x2A;                                          // Result = 0000002A
    t0 = 4;                                             // Result = 00000004
    a3 = 5;                                             // Result = 00000005
    a2 = 3;                                             // Result = 00000003
    a1 = v0;                                            // Result = gMObjHead[0] (800A8E90)
loc_80027BF4:
    v1 = lw(a0 + 0x54);
    v0 = (v1 < 0x2A);
    if (v1 == t3) goto loc_80027C44;
    if (v0 == 0) goto loc_80027C1C;
    if (v1 == t2) goto loc_80027C34;
    goto loc_80027C4C;
loc_80027C1C:
    if (v1 != t1) goto loc_80027C4C;
    at = 0x80070000;                                    // Result = 80070000
    sw(t0, at + 0x7E9C);                                // Store to: gMapBlueKeyType (80077E9C)
    goto loc_80027C4C;
loc_80027C34:
    at = 0x80080000;                                    // Result = 80080000
    sw(a3, at - 0x7F60);                                // Store to: gMapYellowKeyType (800780A0)
    goto loc_80027C4C;
loc_80027C44:
    at = 0x80080000;                                    // Result = 80080000
    sw(a2, at - 0x7DE4);                                // Store to: gMapRedKeyType (8007821C)
loc_80027C4C:
    a0 = lw(a0 + 0x14);
    if (a0 != a1) goto loc_80027BF4;
loc_80027C5C:
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 -= 0x62E8;                                       // Result = gpActiveCeilings[0] (800A9D18)
    a1 = 0;                                             // Result = 00000000
    a2 = 0x78;                                          // Result = 00000078
    _thunk_D_memset();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x7C44;                                       // Result = gpActivePlats[0] (80097C44)
    a1 = 0;                                             // Result = 00000000
    a2 = 0x78;                                          // Result = 00000078
    _thunk_D_memset();
    a0 = 0x80090000;                                    // Result = 80090000
    a0 += 0x77AC;                                       // Result = gButtonList_1[0] (800977AC)
    a1 = 0;                                             // Result = 00000000
    a2 = 0x140;                                         // Result = 00000140
    _thunk_D_memset();
    ra = lw(sp + 0x30);
    s1 = lw(sp + 0x2C);
    s0 = lw(sp + 0x28);
    sp += 0x38;
    return;
}
