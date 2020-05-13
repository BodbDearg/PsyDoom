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

void P_CrossSpecialLine() noexcept {
loc_80026794:
    sp -= 0x28;
    sw(ra, sp + 0x24);
    sw(s0, sp + 0x20);
    v0 = lw(a1 + 0x80);
    s0 = a0;
    if (v0 != 0) goto loc_8002683C;
    v1 = lw(a1 + 0x54);
    v0 = (v1 < 0x1A);
    {
        const bool bJump = (v0 == 0);
        v0 = (v1 < 0x14);
        if (bJump) goto loc_800267CC;
    }
    if (v0 == 0) goto loc_80026D2C;
loc_800267CC:
    v1 = lw(s0 + 0x14);
    v0 = 0x58;                                          // Result = 00000058
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0x59);
        if (bJump) goto loc_8002683C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 0xA;                                       // Result = 0000000A
        if (bJump) goto loc_80026818;
    }
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0xB);
        if (bJump) goto loc_8002683C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = 4;                                         // Result = 00000004
        if (bJump) goto loc_80026804;
    }
    if (v1 == v0) goto loc_8002683C;
    goto loc_80026D2C;
loc_80026804:
    v0 = 0x27;                                          // Result = 00000027
    if (v1 == v0) goto loc_8002683C;
    goto loc_80026D2C;
loc_80026818:
    v0 = 0x61;                                          // Result = 00000061
    {
        const bool bJump = (v1 == v0);
        v0 = (i32(v1) < 0x61);
        if (bJump) goto loc_8002683C;
    }
    {
        const bool bJump = (v0 != 0);
        v0 = (i32(v1) < 0x7F);
        if (bJump) goto loc_80026D2C;
    }
    {
        const bool bJump = (v0 == 0);
        v0 = (i32(v1) < 0x7D);
        if (bJump) goto loc_80026D2C;
    }
    if (v0 != 0) goto loc_80026D2C;
loc_8002683C:
    v0 = lw(s0 + 0x14);
    v1 = v0 - 2;
    v0 = (v1 < 0x8D);
    {
        const bool bJump = (v0 == 0);
        v0 = v1 << 2;
        if (bJump) goto loc_80026D2C;
    }
    at = 0x80010000;                                    // Result = 80010000
    at += 0xBAC;                                        // Result = JumpTable_P_CrossSpecialLine[0] (80010BAC)
    at += v0;
    v0 = lw(at);
    switch (v0) {
        case 0x80026870: goto loc_80026870;
        case 0x8002687C: goto loc_8002687C;
        case 0x80026888: goto loc_80026888;
        case 0x80026894: goto loc_80026894;
        case 0x800268A0: goto loc_800268A0;
        case 0x80026D2C: goto loc_80026D2C;
        case 0x800268AC: goto loc_800268AC;
        case 0x800268B8: goto loc_800268B8;
        case 0x800268C4: goto loc_800268C4;
        case 0x800268D0: goto loc_800268D0;
        case 0x800268DC: goto loc_800268DC;
        case 0x800268E8: goto loc_800268E8;
        case 0x800268F8: goto loc_800268F8;
        case 0x80026904: goto loc_80026904;
        case 0x80026910: goto loc_80026910;
        case 0x8002691C: goto loc_8002691C;
        case 0x80026928: goto loc_80026928;
        case 0x80026940: goto loc_80026940;
        case 0x8002694C: goto loc_8002694C;
        case 0x80026958: goto loc_80026958;
        case 0x80026A94: goto loc_80026A94;
        case 0x80026964: goto loc_80026964;
        case 0x8002697C: goto loc_8002697C;
        case 0x80026988: goto loc_80026988;
        case 0x80026998: goto loc_80026998;
        case 0x800269A4: goto loc_800269A4;
        case 0x800269B4: goto loc_800269B4;
        case 0x800269C0: goto loc_800269C0;
        case 0x800269D0: goto loc_800269D0;
        case 0x800269DC: goto loc_800269DC;
        case 0x80026B00: goto loc_80026B00;
        case 0x80026B14: goto loc_80026B14;
        case 0x80026B28: goto loc_80026B28;
        case 0x80026B38: goto loc_80026B38;
        case 0x80026B4C: goto loc_80026B4C;
        case 0x80026B60: goto loc_80026B60;
        case 0x80026B74: goto loc_80026B74;
        case 0x80026B88: goto loc_80026B88;
        case 0x80026B9C: goto loc_80026B9C;
        case 0x80026BB0: goto loc_80026BB0;
        case 0x80026BC4: goto loc_80026BC4;
        case 0x80026BD8: goto loc_80026BD8;
        case 0x80026BEC: goto loc_80026BEC;
        case 0x80026C00: goto loc_80026C00;
        case 0x80026C0C: goto loc_80026C0C;
        case 0x80026C18: goto loc_80026C18;
        case 0x80026C28: goto loc_80026C28;
        case 0x80026C3C: goto loc_80026C3C;
        case 0x80026C50: goto loc_80026C50;
        case 0x80026C64: goto loc_80026C64;
        case 0x80026C78: goto loc_80026C78;
        case 0x80026C8C: goto loc_80026C8C;
        case 0x80026C98: goto loc_80026C98;
        case 0x80026D24: goto loc_80026D24;
        case 0x80026CAC: goto loc_80026CAC;
        case 0x80026A10: goto loc_80026A10;
        case 0x800269E8: goto loc_800269E8;
        case 0x80026CC0: goto loc_80026CC0;
        case 0x80026CD4: goto loc_80026CD4;
        case 0x80026CE8: goto loc_80026CE8;
        case 0x800269F8: goto loc_800269F8;
        case 0x80026A04: goto loc_80026A04;
        case 0x80026A28: goto loc_80026A28;
        case 0x80026A40: goto loc_80026A40;
        case 0x80026CFC: goto loc_80026CFC;
        case 0x80026A58: goto loc_80026A58;
        case 0x80026A70: goto loc_80026A70;
        case 0x80026A84: goto loc_80026A84;
        case 0x80026D14: goto loc_80026D14;
        case 0x80026AA4: goto loc_80026AA4;
        case 0x80026ABC: goto loc_80026ABC;
        default: jump_table_err(); break;
    }
loc_80026870:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    goto loc_80026A30;
loc_8002687C:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    goto loc_80026A30;
loc_80026888:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    goto loc_80026A30;
loc_80026894:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    goto loc_80026A48;
loc_800268A0:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
    goto loc_80026AAC;
loc_800268AC:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    goto loc_80026A18;
loc_800268B8:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    goto loc_80026A60;
loc_800268C4:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    goto loc_80026930;
loc_800268D0:
    a0 = s0;
    a1 = 0xFF;                                          // Result = 000000FF
    goto loc_80026930;
loc_800268DC:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    goto loc_80026A30;
loc_800268E8:
    a0 = s0;
    EV_StartLightStrobing();
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_800268F8:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    goto loc_80026A48;
loc_80026904:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    goto loc_80026A60;
loc_80026910:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    goto loc_80026AAC;
loc_8002691C:
    a0 = s0;
    a1 = 5;                                             // Result = 00000005
    goto loc_80026A48;
loc_80026928:
    a0 = s0;
    a1 = 0x23;                                          // Result = 00000023
loc_80026930:
    EV_LightTurnOn();
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_80026940:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    goto loc_80026A48;
loc_8002694C:
    a0 = s0;
    a1 = 6;                                             // Result = 00000006
    goto loc_80026A48;
loc_80026958:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    goto loc_80026A48;
loc_80026964:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    v0 = EV_DoCeiling(*vmAddrToPtr<line_t>(a0), (ceiling_e) a1);
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    goto loc_80026A48;
loc_8002697C:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    goto loc_80026AAC;
loc_80026988:
    G_ExitLevel();
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_80026998:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    goto loc_80026A60;
loc_800269A4:
    a0 = s0;
    EV_StopPlat(*vmAddrToPtr<line_t>(a0));
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_800269B4:
    a0 = s0;
    a1 = 9;                                             // Result = 00000009
    goto loc_80026A48;
loc_800269C0:
    a0 = s0;
    v0 = EV_CeilingCrushStop(*vmAddrToPtr<line_t>(a0));
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_800269D0:
    a0 = s0;
    a1 = 7;                                             // Result = 00000007
    goto loc_80026A48;
loc_800269DC:
    a0 = s0;
    a1 = 8;                                             // Result = 00000008
    goto loc_80026A48;
loc_800269E8:
    a0 = s0;
    EV_TurnTagLightsOff();
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_800269F8:
    a0 = s0;
    a1 = 5;                                             // Result = 00000005
    goto loc_80026A30;
loc_80026A04:
    a0 = s0;
    a1 = 6;                                             // Result = 00000006
    goto loc_80026A30;
loc_80026A10:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
loc_80026A18:
    v0 = EV_BuildStairs(*vmAddrToPtr<line_t>(a0), (stair_e) a1);
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_80026A28:
    a0 = s0;
    a1 = 7;                                             // Result = 00000007
loc_80026A30:
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_80026A40:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
loc_80026A48:
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_80026A58:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
loc_80026A60:
    a2 = 0;                                             // Result = 00000000
    v0 = EV_DoPlat(*vmAddrToPtr<line_t>(a0), (plattype_e) a1, a2);
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_80026A70:
    a0 = lw(s0 + 0x18);
    G_SecretExitLevel();
    goto loc_80026D2C;
loc_80026A84:
    v0 = lw(a1 + 0x80);
    if (v0 != 0) goto loc_80026D2C;
loc_80026A94:
    a0 = s0;
    v0 = EV_Teleport(*vmAddrToPtr<line_t>(a0), *vmAddrToPtr<mobj_t>(a1));
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_80026AA4:
    a0 = s0;
    a1 = 5;                                             // Result = 00000005
loc_80026AAC:
    v0 = EV_DoCeiling(*vmAddrToPtr<line_t>(a0), (ceiling_e) a1);
    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_80026ABC:
    S_StopMusic();
    v0 = 0x80070000;                                    // Result = 80070000
    v0 += 0x3E5C;                                       // Result = CDTrackNum_ClubDoom (80073E5C)
    a0 = lw(v0);                                        // Load from: CDTrackNum_ClubDoom (80073E5C)
    a1 = *gCdMusicVol;
    a2 = 0;                                             // Result = 00000000
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    v0 = lw(v0);                                        // Load from: CDTrackNum_ClubDoom (80073E5C)
    a3 = 0;                                             // Result = 00000000
    sw(v0, sp + 0x10);
    sw(a1, sp + 0x14);
    psxcd_play_at_andloop(a0, a1, a2, a3, lw(sp + 0x10), lw(sp + 0x14), lw(sp + 0x18), lw(sp + 0x1C));

    sw(0, s0 + 0x14);
    goto loc_80026D2C;
loc_80026B00:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    v0 = EV_DoCeiling(*vmAddrToPtr<line_t>(a0), (ceiling_e) a1);
    goto loc_80026D2C;
loc_80026B14:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    v0 = EV_DoCeiling(*vmAddrToPtr<line_t>(a0), (ceiling_e) a1);
    goto loc_80026D2C;
loc_80026B28:
    a0 = s0;
    v0 = EV_CeilingCrushStop(*vmAddrToPtr<line_t>(a0));
    goto loc_80026D2C;
loc_80026B38:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    goto loc_80026D2C;
loc_80026B4C:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    goto loc_80026D2C;
loc_80026B60:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
    v0 = EV_DoCeiling(*vmAddrToPtr<line_t>(a0), (ceiling_e) a1);
    goto loc_80026D2C;
loc_80026B74:
    a0 = s0;
    a1 = 0x23;                                          // Result = 00000023
    EV_LightTurnOn();
    goto loc_80026D2C;
loc_80026B88:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    EV_LightTurnOn();
    goto loc_80026D2C;
loc_80026B9C:
    a0 = s0;
    a1 = 0xFF;                                          // Result = 000000FF
    EV_LightTurnOn();
    goto loc_80026D2C;
loc_80026BB0:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    goto loc_80026D2C;
loc_80026BC4:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    goto loc_80026D2C;
loc_80026BD8:
    a0 = s0;
    a1 = 6;                                             // Result = 00000006
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    goto loc_80026D2C;
loc_80026BEC:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    goto loc_80026D2C;
loc_80026C00:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    goto loc_80026D04;
loc_80026C0C:
    a0 = s0;
    a1 = 1;                                             // Result = 00000001
    goto loc_80026D04;
loc_80026C18:
    a0 = s0;
    EV_StopPlat(*vmAddrToPtr<line_t>(a0));
    goto loc_80026D2C;
loc_80026C28:
    a0 = s0;
    a1 = 0;                                             // Result = 00000000
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    goto loc_80026D2C;
loc_80026C3C:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    goto loc_80026D2C;
loc_80026C50:
    a0 = s0;
    a1 = 7;                                             // Result = 00000007
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    goto loc_80026D2C;
loc_80026C64:
    a0 = s0;
    a1 = 8;                                             // Result = 00000008
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    goto loc_80026D2C;
loc_80026C78:
    a0 = s0;
    a1 = 9;                                             // Result = 00000009
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    goto loc_80026D2C;
loc_80026C8C:
    a0 = s0;
    a1 = 3;                                             // Result = 00000003
    goto loc_80026D04;
loc_80026C98:
    a0 = s0;
    a1 = 5;                                             // Result = 00000005
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    goto loc_80026D2C;
loc_80026CAC:
    a0 = s0;
    a1 = 2;                                             // Result = 00000002
    v0 = EV_DoFloor(*vmAddrToPtr<line_t>(a0), (floor_e) a1);
    goto loc_80026D2C;
loc_80026CC0:
    a0 = s0;
    a1 = 5;                                             // Result = 00000005
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    goto loc_80026D2C;
loc_80026CD4:
    a0 = s0;
    a1 = 6;                                             // Result = 00000006
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    goto loc_80026D2C;
loc_80026CE8:
    a0 = s0;
    a1 = 7;                                             // Result = 00000007
    v0 = EV_DoDoor(*vmAddrToPtr<line_t>(a0), (vldoor_e) a1);
    goto loc_80026D2C;
loc_80026CFC:
    a0 = s0;
    a1 = 4;                                             // Result = 00000004
loc_80026D04:
    a2 = 0;                                             // Result = 00000000
    v0 = EV_DoPlat(*vmAddrToPtr<line_t>(a0), (plattype_e) a1, a2);
    goto loc_80026D2C;
loc_80026D14:
    v0 = lw(a1 + 0x80);
    if (v0 != 0) goto loc_80026D2C;
loc_80026D24:
    a0 = s0;
    v0 = EV_Teleport(*vmAddrToPtr<line_t>(a0), *vmAddrToPtr<mobj_t>(a1));
loc_80026D2C:
    ra = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x28;
    return;
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
    P_ChangeSwitchTexture();
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

void EV_DoDonut() noexcept {
loc_8002745C:
    sp -= 0x40;
    sw(fp, sp + 0x38);
    fp = a0;
    sw(s4, sp + 0x28);
    s4 = -1;                                            // Result = FFFFFFFF
    sw(s6, sp + 0x30);
    s6 = 0;                                             // Result = 00000000
    sw(s7, sp + 0x34);
    s7 = 0x80020000;                                    // Result = 80020000
    s7 -= 0x6FF0;                                       // Result = T_MoveFloor (80019010)
    sw(s5, sp + 0x2C);
    s5 = 0x10000;                                       // Result = 00010000
    s5 |= 0x8000;                                       // Result = 00018000
    sw(ra, sp + 0x3C);
    sw(s3, sp + 0x24);
    sw(s2, sp + 0x20);
    sw(s1, sp + 0x1C);
    sw(s0, sp + 0x18);
loc_800274A4:
    v1 = *gNumSectors;
    a0 = s4 + 1;                                        // Result = 00000000
    v0 = (i32(a0) < i32(v1));
    {
        const bool bJump = (v0 == 0);
        v0 = a0 << 1;                                   // Result = 00000000
        if (bJump) goto loc_80027500;
    }
    a2 = v1;
    v0 += a0;                                           // Result = 00000000
    v0 <<= 3;                                           // Result = 00000000
    v0 -= a0;                                           // Result = 00000000
    v0 <<= 2;                                           // Result = 00000000
    v1 = *gpSectors;
    a1 = lw(fp + 0x18);
    v1 += v0;
loc_800274E0:
    v0 = lw(v1 + 0x18);
    s4 = a0;
    if (v0 == a1) goto loc_80027504;
    a0++;
    v0 = (i32(a0) < i32(a2));
    v1 += 0x5C;
    if (v0 != 0) goto loc_800274E0;
loc_80027500:
    s4 = -1;                                            // Result = FFFFFFFF
loc_80027504:
    v0 = s4 << 1;                                       // Result = FFFFFFFE
    if (i32(s4) < 0) goto loc_80027670;
    v0 += s4;                                           // Result = FFFFFFFD
    v0 <<= 3;                                           // Result = FFFFFFE8
    v0 -= s4;                                           // Result = FFFFFFE9
    v1 = *gpSectors;
    v0 <<= 2;                                           // Result = FFFFFFA4
    s3 = v0 + v1;
    v0 = lw(s3 + 0x50);
    if (v0 != 0) goto loc_800274A4;
    v0 = lw(s3 + 0x58);
    v1 = lw(v0);
    v0 = lw(v1 + 0x10);
    v0 &= 4;
    s6 = 1;                                             // Result = 00000001
    if (v0 != 0) goto loc_80027610;
    s1 = 0;                                             // Result = 00000000
    goto loc_80027624;
loc_80027564:
    a1 = 0x2C;
    a2 = 4;
    a0 = *gpMainMemZone;
    a3 = 0;
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    a1 = 0x2C;
    a0 = *gpMainMemZone;
    v0 = 0xA;
    sw(s0, s1 + 0x50);
    sw(v0, s0 + 0xC);
    v0 = 1;                                             // Result = 00000001
    sw(s7, s0 + 0x8);
    sw(0, s0 + 0x10);
    sw(v0, s0 + 0x18);
    sw(s1, s0 + 0x14);
    sw(s5, s0 + 0x28);
    v0 = lhu(s2 + 0x8);
    a2 = 4;                                             // Result = 00000004
    sw(0, s0 + 0x1C);
    sh(v0, s0 + 0x20);
    v0 = lw(s2);
    a3 = 0;                                             // Result = 00000000
    sw(v0, s0 + 0x24);
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = -1;                                            // Result = FFFFFFFF
    sw(s0, s3 + 0x50);
    sw(s7, s0 + 0x8);
    sw(0, s0 + 0xC);
    sw(0, s0 + 0x10);
    sw(v0, s0 + 0x18);
    sw(s3, s0 + 0x14);
    sw(s5, s0 + 0x28);
    v0 = lw(s2);
    sw(v0, s0 + 0x24);
    goto loc_800274A4;
loc_80027610:
    s1 = lw(v1 + 0x38);
    if (s1 != s3) goto loc_80027624;
    s1 = lw(v1 + 0x3C);
loc_80027624:
    v0 = lw(s1 + 0x54);
    a0 = 0;                                             // Result = 00000000
    if (i32(v0) <= 0) goto loc_800274A4;
    a1 = v0;
    v1 = lw(s1 + 0x58);
loc_80027640:
    v0 = lw(v1);
    s2 = lw(v0 + 0x3C);
    a0++;
    if (s2 != s3) goto loc_80027564;
    v0 = (i32(a0) < i32(a1));
    v1 += 4;
    if (v0 != 0) goto loc_80027640;
    goto loc_800274A4;
loc_80027670:
    v0 = s6;                                            // Result = 00000000
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

void G_ScheduleExitLevel() noexcept {
    sp -= 0x20;
    sw(s1, sp + 0x14);
    s1 = a0;
    sw(s2, sp + 0x18);
    s2 = a1;
    a1 = 0x14;
    a2 = 4;
    a0 = *gpMainMemZone;
    a3 = 0;
    sw(ra, sp + 0x1C);
    sw(s0, sp + 0x10);
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80020000;                                    // Result = 80020000
    v0 += 0x7718;                                       // Result = G_BeginExitLevel (80027718)
    sw(v0, s0 + 0x8);
    sw(s1, s0 + 0xC);
    sw(s2, s0 + 0x10);
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void G_BeginExitLevel() noexcept {
    sp -= 0x18;
    sw(s0, sp + 0x10);
    s0 = a0;
    sw(ra, sp + 0x14);
    v0 = lw(s0 + 0xC);
    v0--;
    sw(v0, s0 + 0xC);
    if (i32(v0) > 0) goto loc_80027754;
    v0 = lw(s0 + 0x10);
    ptr_call(v0);
    a0 = s0;
    _thunk_P_RemoveThinker();
loc_80027754:
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void G_ExitLevel() noexcept {
loc_80027768:
    sp -= 0x18;
    a1 = 0x14;
    a2 = 4;
    v0 = *gGameMap;
    a0 = *gpMainMemZone;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    v0++;
    *gNextMap = v0;
    a3 = 0;                                             // Result = 00000000
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80020000;                                    // Result = 80020000
    v0 += 0x7718;                                       // Result = G_BeginExitLevel (80027718)
    sw(v0, s0 + 0x8);
    v0 = 4;                                             // Result = 00000004
    sw(v0, s0 + 0xC);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x3384;                                       // Result = G_SetGameComplete (80013384)
    sw(v0, s0 + 0x10);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void G_SecretExitLevel() noexcept {
loc_800277E0:
    sp -= 0x18;
    a1 = 0x14;
    a2 = 4;
    *gNextMap = a0;
    a0 = *gpMainMemZone;
    a3 = 0;
    sw(ra, sp + 0x14);
    sw(s0, sp + 0x10);
    _thunk_Z_Malloc();
    s0 = v0;
    a0 = s0;
    _thunk_P_AddThinker();
    v0 = 0x80020000;                                    // Result = 80020000
    v0 += 0x7718;                                       // Result = G_BeginExitLevel (80027718)
    sw(v0, s0 + 0x8);
    v0 = 4;                                             // Result = 00000004
    sw(v0, s0 + 0xC);
    v0 = 0x80010000;                                    // Result = 80010000
    v0 += 0x3384;                                       // Result = G_SetGameComplete (80013384)
    sw(v0, s0 + 0x10);
    ra = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x18;
    return;
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
    P_SpawnGlowingLight();
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
    P_SpawnGlowingLight();
    s1++;
    goto loc_80027A3C;
loc_80027A0C:
    a1 = 2;                                             // Result = 00000002
    P_SpawnGlowingLight();
    s1++;
    goto loc_80027A3C;
loc_80027A1C:
    a0 = s0;
    P_SpawnRapidStrobeFlash();
    s1++;
    goto loc_80027A3C;
loc_80027A2C:
    a2 = 0;                                             // Result = 00000000
loc_80027A30:
    P_SpawnStrobeFlash();
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
