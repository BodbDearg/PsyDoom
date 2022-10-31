#include "p_spec.h"

#include "Asserts.h"
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
#include "info.h"
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
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/ParserTokenizer.h"
#include "PsyDoom/ScriptingEngine.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

// Definition for a flat or texture animation
struct animdef_t {
    bool        istexture;      // False for flats
    char        startname[9];   // Name of the first lump in the animation
    char        endname[9];     // Name of the last lump in the animation
    uint32_t    ticmask;        // New field for PSX: controls which game tics the animation will advance on
};

// Mask applied to offsets for scrolling walls (wrap every 128 units)
static constexpr int32_t SCROLLMASK = 0xFF7F0000;

// Definitions for all flat and texture animations built into the game
static const animdef_t gBaseAnimDefs[BASE_NUM_ANIMS_FDOOM] = {
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
    { 1, "WARN01",   "WARN02",   3 },
    // Final Doom only animations: note that the 'GLOW01' texture is found in the original DOOM but it is not animated, even
    // though all the frames for it exist. Also note that these entries are NOT found at the end of the array in the Final Doom code.
    // I added them here for convenience so they can be ignored more easily in the case of the original Doom game.
    { 0, "GLOW01",   "GLOW04",   3 },
    { 1, "WFALL1",   "WFALL4",   3 },
};

// PsyDoom: anim definitions for the current game and user mod.
// The list is now built dynamically at runtime.
#if PSYDOOM_MODS
    static std::vector<animdef_t> gAnimDefs;
#else
    #define gAnimDefs gBaseAnimDefs
#endif

card_t      gMapBlueKeyType;        // What type of blue key the map uses (if map has a blue key)
card_t      gMapRedKeyType;         // What type of red key the map uses (if map has a red key)
card_t      gMapYellowKeyType;      // What type of yellow key the map uses (if map has a yellow key)
int32_t     gMapBossSpecialFlags;   // PSX addition: What types of boss specials (triggers) are active on the current map

// The list of animated textures.
// PsyDoom: this list is now allocated dynamically at runtime.
#if PSYDOOM_MODS
    static std::vector<anim_t> gAnims;
#else
    static anim_t gAnims[BASE_NUM_ANIMS_FDOOM];
#endif

// Points to the end of the list of animated textures
static anim_t* gpLastAnim;

// PsyDoom: can now have as many scrolling lines as we want
#if PSYDOOM_LIMIT_REMOVING
    static std::vector<line_t*> gpLineSpecialList;      // A list of scrolling lines for the level
#else
    static constexpr int32_t MAXLINEANIMS = 32;         // Maximum number of line animations allowed

    static line_t*  gpLineSpecialList[MAXLINEANIMS];    // A list of scrolling lines for the level
    static int32_t  gNumLinespecials;                   // The number of scrolling lines in the level
#endif

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: try to read a list of (wall or floor) animation definitions from the named text lump.
// 
// Grammar for these definitions (one per line):
//      BEGIN_TEX END_TEX TICMASK
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_ReadUserAnimDefs(const char* const lumpName, const bool bWallAnims) noexcept {
    // Does the user anim def lump exist?
    const int32_t lumpIdx = W_CheckNumForName(lumpName);

    if (lumpIdx < 0)
        return;

    // Read the lump entirely and null terminate the text data
    const int32_t lumpSize = W_LumpLength(lumpIdx);
    std::unique_ptr<char[]> lumpChars(new char[lumpSize + 1]);
    W_ReadLump(lumpIdx, lumpChars.get(), true);
    lumpChars[lumpSize] = 0;

    // Parse the anim defs
    int32_t curLineIdx = 0;

    ParserTokenizer::visitAllLineTokens(
        lumpChars.get(),
        lumpChars.get() + lumpSize,
        [&](const int32_t lineIdx) noexcept {
            // New definition lies ahead
            curLineIdx = lineIdx;
            animdef_t& animDef = gAnimDefs.emplace_back();
            animDef.istexture = bWallAnims;
        },
        [&](const int32_t tokenIdx, const char* const token, const size_t tokenLen) noexcept {
            // Too many tokens?
            if ((tokenIdx < 0) || (tokenIdx >= 3)) {
                I_Error("P_ReadUserAnimDefs: LUMP '%s' line %d has invalid syntax!\nExpected: BEGIN_TEX END_TEX TICMASK", lumpName, curLineIdx);
            }

            // Parse each token
            animdef_t& animDef = gAnimDefs.back();
            const bool bIsTexNameToken = ((tokenIdx == 0) || (tokenIdx == 1));

            if (bIsTexNameToken && (tokenLen > MAX_WAD_LUMPNAME)) {
                I_Error("P_ReadUserAnimDefs: LUMP '%s' line %d - texture name is too long!", lumpName, curLineIdx);
            }

            if (tokenIdx == 0) {
                // Start name
                std::memcpy(animDef.startname, token, tokenLen);
                animDef.startname[tokenLen] = 0;
            } else if (tokenIdx == 1) {
                // End time
                std::memcpy(animDef.endname, token, tokenLen);
                animDef.endname[tokenLen] = 0;
            } else if (tokenIdx == 2) {
               // Tic mask (integer)
               String16 numberStr = token;
               numberStr.chars[std::min<size_t>(String16::MAX_LEN - 1, tokenLen)] = 0;
               animDef.ticmask = std::atoi(numberStr.chars);
            }
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: initializes the dynamically populated list of anim definitions, and also allocates the list of animations.
// Uses the list of base animations appropriate to the game being played, plus any anims defined in user mods.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_InitAnimDefs() noexcept {
    // Add base animation definitions
    gAnimDefs.clear();
    gAnimDefs.reserve(128);
    const int32_t baseNumAnims = Game::gConstants.baseNumAnims;

    for (int32_t i = 0; i < baseNumAnims; ++i) {
        gAnimDefs.push_back(gBaseAnimDefs[i]);
    }

    // Read user animation definitions
    P_ReadUserAnimDefs("PSYTANIM", true);
    P_ReadUserAnimDefs("PSYFANIM", false);

    // Init the animation list - 1 slot per anim definition
    gAnims.clear();
    gAnims.resize(gAnimDefs.size());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: sets animated textures and flats to use the first frame number.
// Fixes cases where WAD authors might use the 2nd frame of an animation (e.g SLIME02) instead of the 1st.
// The engine requires an animation to be referred to by it's first frame in order to work...
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SetAnimsToBasePic() noexcept {
    // This trawls through the sector or side list for every animation.
    // Not the most efficient method but it's only done once on startup for not too many animations, and this way avoids allocations...
    const int32_t maxAnims = (int32_t) gAnimDefs.size();

    for (int32_t animIdx = 0; animIdx < maxAnims; ++animIdx) {
        const animdef_t& animdef = gAnimDefs[animIdx];

        if (animdef.istexture) {
            // Note: these textures MUST exist or otherwise a fatal error will be issued
            const int32_t startPic = R_TextureNumForName(animdef.startname, true);
            const int32_t endPic = R_TextureNumForName(animdef.endname, true);

            for (int32_t sideIdx = 0; sideIdx < gNumSides; ++sideIdx) {
                side_t& side = gpSides[sideIdx];

                if ((side.bottomtexture > startPic) && (side.bottomtexture <= endPic)) {
                    side.bottomtexture = startPic;
                }

                if ((side.midtexture > startPic) && (side.midtexture <= endPic)) {
                    side.midtexture = startPic;
                }

                if ((side.toptexture > startPic) && (side.toptexture <= endPic)) {
                    side.toptexture = startPic;
                }
            }
        } else {
            // Note: these textures MUST exist or otherwise a fatal error will be issued
            const int32_t startPic = R_FlatNumForName(animdef.startname, true);
            const int32_t endPic = R_FlatNumForName(animdef.endname, true);

            for (int32_t secIdx = 0; secIdx < gNumSectors; ++secIdx) {
                sector_t& sector = gpSectors[secIdx];

                if ((sector.floorpic > startPic) && (sector.floorpic <= endPic)) {
                    sector.floorpic = startPic;
                }

                if ((sector.ceilingpic > startPic) && (sector.ceilingpic <= endPic)) {
                    sector.ceilingpic = startPic;
                }
            }
        }
    }
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Caches into RAM the textures for all animated flats and textures.
// Also sets up the spot in VRAM where these animations will go - they occupy the same spot as the base animation frame.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_InitPicAnims() noexcept {
    // PsyDoom: the list of anims and anim defs is now dynamic
    #if PSYDOOM_MODS
        const int32_t maxAnims = (int32_t) gAnims.size();
    #else
        const int32_t maxAnims = (Game::isFinalDoom()) ? BASE_NUM_ANIMS_FDOOM : BASE_NUM_ANIMS_DOOM;
    #endif

    gpLastAnim = &gAnims[0];

    for (int32_t animIdx = 0; animIdx < maxAnims; ++animIdx) {
        const animdef_t& animdef = gAnimDefs[animIdx];
        anim_t& lastanim = *gpLastAnim;

        // PsyDoom: all anim textures MUST exist, otherwise issue a fatal error
        #if PSYDOOM_MODS
            constexpr bool bAnimTexturesMustExist = true;
        #else
            constexpr bool bAnimTexturesMustExist = false;
        #endif

        if (animdef.istexture) {
            // Determine the lump range for the animation
            lastanim.basepic = R_TextureNumForName(animdef.startname, bAnimTexturesMustExist);
            lastanim.picnum = R_TextureNumForName(animdef.endname, bAnimTexturesMustExist);

            // Ignore this animation if it's not used in the level
            texture_t& basetex = gpTextures[lastanim.basepic];

            // PsyDoom: the meaning of 'texPageId' has changed slightly, '0' is now the 1st page and 'bIsCached' is used check cache residency
            #if PSYDOOM_MODS
                if (!basetex.bIsCached)
                    continue;
            #else
                if (basetex.texPageId == 0)
                    continue;
            #endif

            // For all frames in the animation, cache the lump in RAM and make it occupy the same VRAM spot as the base texture
            for (int32_t picNum = lastanim.basepic; picNum <= lastanim.picnum; ++picNum) {
                // Ensure the lump is cached in RAM for runtime (will need to be backed in RAM in case it's evicted from VRAM).
                // PsyDoom: tweak the code slightly because the texture list might no longer be a contiguous set of lumps.
                texture_t& dstTex = gpTextures[picNum];

                #if PSYDOOM_MODS
                    W_CacheLumpNum(dstTex.lumpNum, PU_ANIMATION, false);
                #else
                    W_CacheLumpNum(gFirstTexLumpNum + picNum, PU_ANIMATION, false);
                #endif

                dstTex.texPageCoordX = basetex.texPageCoordX;
                dstTex.texPageCoordY = basetex.texPageCoordY;
                dstTex.texPageId = basetex.texPageId;
                dstTex.ppTexCacheEntries = basetex.ppTexCacheEntries;
            }
        } else {
            // Determine the lump range for the animation
            lastanim.basepic = R_FlatNumForName(animdef.startname, bAnimTexturesMustExist);
            lastanim.picnum = R_FlatNumForName(animdef.endname, bAnimTexturesMustExist);

            // Ignore this animation if it's not used in the level
            texture_t& basetex = gpFlatTextures[lastanim.basepic];

            // PsyDoom: the meaning of 'texPageId' has changed slightly, '0' is now the 1st page and 'bIsCached' is used check cache residency
            #if PSYDOOM_MODS
                if (!basetex.bIsCached)
                    continue;
            #else
                if (basetex.texPageId == 0)
                    continue;
            #endif

            // For all frames in the animation, cache the lump in RAM and make it occupy the same VRAM spot as the base texture
            for (int32_t picNum = lastanim.basepic; picNum <= lastanim.picnum; ++picNum) {
                // Ensure the lump is cached in RAM for runtime (will need to be backed in RAM in case it's evicted from VRAM).
                // PsyDoom: tweak the code slightly because the flat list might no longer be a contiguous set of lumps.
                texture_t& dstTex = gpFlatTextures[picNum];

                #if PSYDOOM_MODS
                    W_CacheLumpNum(dstTex.lumpNum, PU_ANIMATION, false);
                #else
                    W_CacheLumpNum(gFirstFlatLumpNum + picNum, PU_ANIMATION, false);
                #endif

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

        gpLastAnim++;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the specified side of a line in the given sector
//------------------------------------------------------------------------------------------------------------------------------------------
side_t* getSide(const int32_t sectorIdx, const int32_t lineIdx, const int32_t sideIdx) noexcept {
    return gpSides + gpSectors[sectorIdx].lines[lineIdx]->sidenum[sideIdx];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the sector on the specified side of a line in the given sector
//------------------------------------------------------------------------------------------------------------------------------------------
sector_t* getSector(const int32_t sectorIdx, const int32_t lineIdx, const int32_t sideIdx) noexcept {
    return gpSides[gpSectors[sectorIdx].lines[lineIdx]->sidenum[sideIdx]].sector;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Utility: tell if a line in sector is two sided
//------------------------------------------------------------------------------------------------------------------------------------------
bool twoSided(const int32_t sectorIdx, const int32_t lineIdx) noexcept {
    return (gpSectors[sectorIdx].lines[lineIdx]->flags & ML_TWOSIDED);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the sector on the opposite side of the line to the given sector.
// Returns 'nullptr' if the line is single sided.
//------------------------------------------------------------------------------------------------------------------------------------------
sector_t* getNextSector(line_t& line, sector_t& sector) noexcept {
    if ((line.flags & ML_TWOSIDED) == 0)
        return nullptr;

    sector_t* const pFrontSec = line.frontsector;
    return (&sector == pFrontSec) ? line.backsector : pFrontSec;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the lowest floor height for sectors surrounding the given sector, and including the input sector itself
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_FindLowestFloorSurrounding(sector_t& sector) noexcept {
    fixed_t lowestFloor = sector.floorheight;

    for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
        line_t& line = *sector.lines[lineIdx];
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
        line_t& line = *sector.lines[lineIdx];
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
// PsyDoom: if there is no such sector, the input base height will be returned rather than an undefined/garbage height value.
// It also no longer overflows buffers when checking > 20 adjoining sectors.
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_FindNextHighestFloor(sector_t& sector, const fixed_t baseHeight) noexcept {
    // PsyDoom: rewrite this to eliminate potential buffer overflow and also an undefined answer when there is no next higher floor.
    // Note that the PC version does NOT have the undefined height/answer bug - it returns the input height when there is no next highest floor.
    // The PC version DOES have a limit of 20 adjacent sectors however, so it can miss stuff in more complex maps.
    //
    // For an example of where this bug produced undefined behavior in PSX Doom see MAP24 in Final Doom, 'Heck'.
    // In the area leading down to the yellow skull key, you cross a line to trigger a platform to rise to the floor level.
    // The same trigger also causes the floor in a small alcove to rise and meet the floor, releasing Imps upon the player.
    // If you go into the alcove previously containing the Imps then the same 'rise to next highest floor' action is triggered again
    // on both sectors. For the alcove sector it rises up to meet a ledge, but for the platform there is no next highest sector so
    // undefined behavior will result. During playtesting and demo recording in the Avocado emulator I found this caused the platform
    // to rise slightly again, creating a small lip with the floor surrounding it. Since the behavior relies on on an uninitialized stack
    // value however, recreating this behavior exactly in PsyDoom would be extremely difficult or impossible. Because of this the demo
    // I recorded did de-synchronize when played back in PsyDoom and this is unfortunately unavoidable as undefined behavior is by
    // definition not reproducible under anything but the exact same hardware, execution conditions and compiler etc.
    //
    #if PSYDOOM_MODS && PSYDOOM_FIX_UB
        fixed_t nextHighestFloor = INT32_MAX;

        for (int32_t lineIdx = 0; lineIdx < sector.linecount; ++lineIdx) {
            line_t& line = *sector.lines[lineIdx];
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
            line_t& line = *sector.lines[lineIdx];
            sector_t* const pNextSector = getNextSector(line, sector);

            if (pNextSector && (pNextSector->floorheight > baseHeight)) {
                ASSERT(numHigherHeights < 20);
                higherHeights[numHigherHeights] = pNextSector->floorheight;
                numHigherHeights++;
            }
        }

        // Return the minimum of those sector heights that are higher
        ASSERT(numHigherHeights > 0);
        fixed_t minHigherHeight = higherHeights[0];

        for (int32_t i = 1; i < numHigherHeights; ++i) {
            minHigherHeight = (minHigherHeight < higherHeights[i]) ? minHigherHeight : higherHeights[i];
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
        line_t& line = *sector.lines[lineIdx];
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
        line_t& line = *sector.lines[lineIdx];
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
    sector_t* const pSectors = gpSectors;
    const int32_t numSectors = gNumSectors;

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
        line_t& line = *sector.lines[lineIdx];
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
    // PsyDoom: disable exits for deathmatch, if set.
    #if PSYDOOM_MODS
        const bool bDmExitDisabled = (gNetGame == gt_deathmatch && (Game::gSettings.bDmExitDisabled) && Game::gSettings.dmFragLimit > 0);
    #endif

    // If the object triggering the special is not a player then only certain specials can be triggered.
    // Some things like projectiles are also not allowed to trigger specials.
    if (!mobj.player) {
        // Only monsters and (oddly) weapons and barrels can trigger specials.
        //
        // BUG: should the end of the range here be 'MT_ARACHPLAZ' instead? Seems like it...
        // In any case it shouldnt't matter since line crossing checks are not done for non-AI objects in Jaguar and PSX Doom.
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
        #if PSYDOOM_MODS
            // PsyDoom: new line specials for triggering scripted actions
            case 301:   // Once: Do Script Action (Monsters only)
            case 302:   // Once: Do Script Action (Player + Monsters)
            case 311:   // Multi: Do Script Action (Monsters only)
            case 312:   // Multi: Do Script Action (Player + Monsters)
        #endif
            {
                bCanTrigger = true;
            }   break;

            default:
                break;
        }

        if (!bCanTrigger)
            return;
    } else {
        // PsyDoom: skip new PsyDoom monster-only line specials that are forbidden to the player
        #if PSYDOOM_MODS
            switch (line.special) {
                case 301:   // Once: Do Script Action (Monsters only)
                case 311:   // Multi: Do Script Action (Monsters only)
                    return;
            }
        #endif
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

        // Lower floor to nearest height and change texture
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
            #if PSYDOOM_MODS
                if (bDmExitDisabled) {
                    mobj.player->message = "Exits are disabled.";
                    S_StartSound(&mobj, sfx_getpow);
                    break;
                }
            #endif // #if PSYDOOM_MODS
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
            #if PSYDOOM_MODS
                if (bDmExitDisabled) {
                    mobj.player->message = "Exits are disabled.";
                    S_StartSound(&mobj, sfx_getpow);
                    break;
                }
            #endif // #if PSYDOOM_MODS
            G_SecretExitLevel(line.tag);
            break;

        // Teleport (monsters ONLY!)
        case 125: {
            if (!mobj.player) {
                EV_Teleport(line, mobj);
                line.special = 0;
            }
        }   break;

        // PsyDoom: adding support for missing line specials from PC
        #if PSYDOOM_MODS
            // Raise Floor Turbo
            case 130:
                EV_DoFloor(line, raiseFloorTurbo);
                line.special = 0;
                break;
        #endif

        // Silent ceiling crush & raise
        case 141:
            EV_DoCeiling(line, silentCrushAndRaise);
            line.special = 0;
            break;

        // New for PSX: play the Club Doom CD audio!
        case 142: {
            S_StopMusic();

            // PsyDoom: if this line has a tag then play that track number instead of the default 'Club Doom' track.
            // This allows the correct track to play for the 'Icon of Sin' level in the GEC Master Edition.
            #if PSYDOOM_MODS
                const int32_t cdTrackNum = (line.tag <= 0) ? gCDTrackNum[cdmusic_club_doom] : line.tag;
            #else
                const int32_t cdTrackNum = gCDTrackNum[cdmusic_club_doom];
            #endif

            psxcd_play_at_andloop(cdTrackNum, gCdMusicVol, 0, 0, cdTrackNum, gCdMusicVol, 0, 0);
            line.special = 0;
        }   break;

        // PsyDoom: new line specials for triggering scripted actions
        #if PSYDOOM_MODS
            case 300:   // Once: Do Script Action (Player only)
            case 301:   // Once: Do Script Action (Monsters only)
            case 302:   // Once: Do Script Action (Player + Monsters)
                line.special = 0;
                ScriptingEngine::doAction(line.tag, &line, line.frontsector, &mobj, 0, 0);
                break;
        #endif

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

        // PsyDoom: adding support for missing line specials from PC
        #if PSYDOOM_MODS
            // Raise To Nearest Floor
            case 128:
                EV_DoFloor(line, raiseFloorToNearest);
                break;

            // Raise Floor Turbo
            case 129:
                EV_DoFloor(line, raiseFloorTurbo);
                break;
        #endif

        // PsyDoom: new line specials for triggering scripted actions
        #if PSYDOOM_MODS
            case 310:   // Multi: Do Script Action (Player only)
            case 311:   // Multi: Do Script Action (Monsters only)
            case 312:   // Multi: Do Script Action (Player + Monsters)
                ScriptingEngine::doAction(line.tag, &line, line.frontsector, &mobj, 0, 0);
                break;
        #endif

        default:
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called when a map object shoots a line with a special; tries to activate whatever special the line has
//------------------------------------------------------------------------------------------------------------------------------------------
void P_ShootSpecialLine(mobj_t& mobj, line_t& line) noexcept {
    // Monsters can only shoot certain special lines
    if (!mobj.player) {
        switch (line.special) {
            case 46:    // Open door
                break;

            // PsyDoom: new line specials for triggering scripted actions
            #if PSYDOOM_MODS
                case 341:   // Once: Do Script Action (Monsters only)
                case 342:   // Once: Do Script Action (Player + Monsters)
                case 351:   // Multi: Do Script Action (Monsters only)
                case 352:   // Multi: Do Script Action (Player + Monsters)
                    break;
            #endif

            default:    // NOT allowed!
                return;
        }
    } else {
        // PsyDoom: skip new PsyDoom monster-only line specials that are forbidden to the player
        #if PSYDOOM_MODS
            switch (line.special) {
                case 341:   // Once: Do Script Action (Monsters only)
                case 351:   // Multi: Do Script Action (Monsters only)
                    return;
            }
        #endif
    }

    // Activate the line special
    switch (line.special) {
        // Raise floor
        case 24: {
            if (EV_DoFloor(line, raiseFloor)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // Open door
        case 46: {
            if (EV_DoDoor(line, Open)) {
                P_ChangeSwitchTexture(line, true);
            }
        }   break;

        // Raise floor to nearest and change
        case 47: {
            if (EV_DoPlat(line, raiseToNearestAndChange, 0)) {
                P_ChangeSwitchTexture(line, false);
            }
        }   break;

        // PsyDoom: new line specials for triggering scripted actions
        #if PSYDOOM_MODS
            case 340:   // Once: Do Script Action (Player only)
            case 341:   // Once: Do Script Action (Monsters only)
            case 342:   // Once: Do Script Action (Player + Monsters)
                line.special = 0;
                [[fallthrough]];
            case 350:   // Multi: Do Script Action (Player only)
            case 351:   // Multi: Do Script Action (Monsters only)
            case 352:   // Multi: Do Script Action (Player + Monsters)
                ScriptingEngine::doAction(line.tag, &line, line.frontsector, &mobj, 0, 0);
                break;
        #endif
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called every game tic that the player position is in a sector which has a special.
// Credits secrets found by entering sectors and damage taken from floors (lava, slime etc.).
//------------------------------------------------------------------------------------------------------------------------------------------
void P_PlayerInSpecialSector(player_t& player) noexcept {
    sector_t& sector = *player.mo->subsector->sector;

    // Helper: triggers a face on the status bar
    const auto doStatusBarFace = [&](const spclface_e face) noexcept {
        // PsyDoom: fix a bug in co-op/dm where sector specials that cause pain also change the status bar face for the other player.
        // Need to check if this user's player is being affected.
        #if PSYDOOM_MODS
            if (&player == &gPlayers[gCurPlayerIndex]) {
                gStatusBar.specialFace = face;
            }
        #else
            gStatusBar.specialFace = face;
        #endif
    };

    // Logic only runs when the player is on the floor
    if (player.mo->z != sector.floorheight)
        return;

    switch (sector.special) {
        // Hellslime damage
        case 5: {
            if (!player.powers[pw_ironfeet]) {
                doStatusBarFace(f_mowdown);

                if ((gGameTic > gPrevGameTic) && ((gGameTic & 0xF) == 0)) {     // Apply roughly every 1 second
                    P_DamageMobj(*player.mo, nullptr, nullptr, 10);
                }
            }
        }   break;

        // Nukage damage
        case 7: {
            if (!player.powers[pw_ironfeet]) {
                doStatusBarFace(f_mowdown);

                if ((gGameTic > gPrevGameTic) && ((gGameTic & 0xF) == 0)) {     // Apply roughly every 1 second
                    P_DamageMobj(*player.mo, nullptr, nullptr, 5);
                }
            }
        }   break;

        // Strobe hurt (note: on PSX Doom the strobe was actually removed, it's just 'hurt' now)
        case 4:
        // Super hellslime damage
        case 16:
    #if PSYDOOM_MODS
        // PsyDoom addition: an actual 'strobe hurt' (strobe flash + hurt) for the PSX engine.
        // This behaves the same as special '4' did originally on PC.
        case 32:
    #endif
        {
            if ((!player.powers[pw_ironfeet]) || (P_Random() < 5)) {    // Occasionally damages even if a radiation suit is worn
                doStatusBarFace(f_mowdown);

                if ((gGameTic > gPrevGameTic) && ((gGameTic & 0xF) == 0)) {     // Apply roughly every 1 second
                    P_DamageMobj(*player.mo, nullptr, nullptr, 20);
                }
            }
        }   break;

        // Secret sector
        case 9: {
            player.secretcount += 1;
            sector.special = 0;         // Consume the special so it's only counted once!
        }   break;

    // PsyDoom: add support for the PC E1M8 style exit.
    // This special kills the player and causes an exit, and even disables god mode.
    #if PSYDOOM_MODS
        case 11: {
            player.cheats &= ~CF_GODMODE;

            // Apply damage roughly every 1 second
            if ((gGameTic > gPrevGameTic) && ((gGameTic & 0xF) == 0)) {     // Apply roughly every 1 second
                P_DamageMobj(*player.mo, nullptr, nullptr, 20);
            }

            // If player health is low enough exit the level.
            // Note: exit immediately for this special (unlike normal exit switches) - we don't need to delay to hear the switch sound play.
            if (player.health <= 10) {
                S_StopSound(player.mo);     // PsyDoom: don't do the player death sound (silent death, similar to PC)
                G_ExitLevelImmediately();
            }
        }   break;
    #endif

    // PsyDoom: scripted player in special sector action
    #if PSYDOOM_MODS
        case 301: {
            ScriptingEngine::doAction(sector.tag, nullptr, &sector, player.mo, 0, 0);
        }   break;
    #endif

        default: {
            // PsyDoom: issue a warning rather than a fatal error if encountering a bad sector special
            #if PSYDOOM_MODS
                std::snprintf(gLevelStartupWarning, C_ARRAY_SIZE(gLevelStartupWarning), "W:bad sector special %d!", sector.special);
                gStatusBar.message = gLevelStartupWarning;
                gStatusBar.messageTicsLeft = 60;
            #else
                I_Error("P_PlayerInSpecialSector: unknown special %i", sector.special);
            #endif
        }   break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Updates all special things in the level, such as animated textures, scrolling walls and switching buttons back to their original state.
// PSX: also animates the fire sky if the level has that.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_UpdateSpecials() noexcept {
    // Animate flats and wall textures
    for (anim_t* pAnim = &gAnims[0]; pAnim < gpLastAnim; ++pAnim) {
        // Skip over this entry if it isn't time to advance the animation
        if (gGameTic & pAnim->ticmask)
            continue;

        // Advance the animation and rewind to the first frame if we past the last frame
        pAnim->current += 1;

        if (pAnim->current > pAnim->picnum) {
            pAnim->current = pAnim->basepic;
        }

        // Update the texture translation table and mark the texture as needing uploading to the cache.
        // For animated flats and walls only the current frame is kept in VRAM, to save on precious VRAM space.
        if (pAnim->istexture) {
            gpTextureTranslation[pAnim->basepic] = pAnim->current;
            gpTextures[pAnim->current].uploadFrameNum = TEX_INVALID_UPLOAD_FRAME_NUM;
        } else {
            gpFlatTranslation[pAnim->basepic] = pAnim->current;
            gpFlatTextures[pAnim->current].uploadFrameNum = TEX_INVALID_UPLOAD_FRAME_NUM;
        }
    }

    // Animate line specials (scrolling walls)
    #if PSYDOOM_LIMIT_REMOVING
        const int32_t numLineSpecials = (int32_t) gpLineSpecialList.size();
    #else
        const int32_t numLineSpecials = gNumLinespecials;
    #endif

    for (int32_t specialIdx = 0; specialIdx < numLineSpecials; ++specialIdx) {
        line_t& line = *gpLineSpecialList[specialIdx];
        side_t& side = gpSides[line.sidenum[0]];

        // PsyDoom: need to handle wrapping of uv offsets specially now that they are interpolated.
        // Adjust the 'old' value we are interpolating from so that the wrap adjustment is not part of the interpolation.
        // If interpolation is disabled also, just snap the value immediately:
        #if PSYDOOM_MODS
            const auto offsetUVAndWrap = [](InterpFixedT& uv, const fixed_t offset) noexcept {
                uv += offset;
                const fixed_t postWrapUV = uv & SCROLLMASK;
                const fixed_t wrapAdjustment = postWrapUV - uv;
                uv = postWrapUV;

                if (Config::gbInterpolateSectors) {
                    uv.oldValue += wrapAdjustment;      // Don't interpolate the quantity we just adjusted by to wrap
                } else {
                    uv.snap();                          // Interpolation disabled, just snap!
                }
            };
        #else
            const auto offsetUVAndWrap = [](fixed_t& uv, const fixed_t offset) noexcept {
                uv += offset;
                uv &= SCROLLMASK;
            };
        #endif

        switch (line.special) {
            // Effect: scroll left
            case 200: {
                offsetUVAndWrap(side.textureoffset, +FRACUNIT);
            }   break;

            // Effect: scroll right
            case 201: {
                offsetUVAndWrap(side.textureoffset, -FRACUNIT);
            }   break;

            // Effect: scroll up
            case 202: {
                offsetUVAndWrap(side.rowoffset, +FRACUNIT);
            }   break;

            // Effect: scroll down
            case 203: {
                offsetUVAndWrap(side.rowoffset, -FRACUNIT);
            }   break;
        }
    }

    // Update active switches/buttons and switch their wall textures back if it is time
    #if PSYDOOM_LIMIT_REMOVING
        const int32_t numButtons = (int32_t) gButtonList.size();
    #else
        const int32_t numButtons = MAXBUTTONS;
    #endif

    for (int32_t buttonIdx = 0; buttonIdx < numButtons; ++buttonIdx) {
        // Ignore this button if it is not active
        button_t& button = gButtonList[buttonIdx];

        if (button.btimer <= 0)
            continue;

        // Advance the countdown until when the button switches back and skip over it if its not time to do that yet
        button.btimer -= gPlayersElapsedVBlanks[gCurPlayerIndex];

        if (button.btimer > 0)
            continue;

        // Switch the wall texture for the button back to what it was
        line_t& line = *button.line;
        side_t& side = gpSides[line.sidenum[0]];

        switch (button.where) {
            case top:       side.toptexture     = button.btexture;  break;
            case middle:    side.midtexture     = button.btexture;  break;
            case bottom:    side.bottomtexture  = button.btexture;  break;
        }

        // Play a sound for the button switching back and reset the state of this active button slot (now free for use again)
        S_StartSound(button.soundorg, sfx_swtchn);
        D_memset(&button, std::byte(0), sizeof(button_t));

        // PsyDoom limit removing: if it's the last button in the list then compact the list
        #if PSYDOOM_LIMIT_REMOVING
            if (buttonIdx + 1 >= numButtons) {
                gButtonList.pop_back();
            }
        #endif
    }

    // New for PSX: update the fire sky if the level has a fire sky and the sky is visible
    if (gbIsSkyVisible && gUpdateFireSkyFunc) {
        gUpdateFireSkyFunc(*gpSkyTexture);
    }
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
        sector_t& sector = gpSectors[sectorIdx];

        if (sector.specialdata)
            continue;

        bActivatedMovers = true;

        // Get the next sector beyond the first line in this sector, that will be the outer ring of the 'donut', or the part that is raised:
        sector_t* const pNextSector = getNextSector(*sector.lines[0], sector);

        #if PSYDOOM_MODS && PSYDOOM_FIX_UB
            // PsyDoom: safety in case the level is setup wrong - if this line is not two sided then just ignore the command
            if (!pNextSector)
                continue;
        #else
            ASSERT(pNextSector);
        #endif

        // Run through all the lines of the next sector and try to find the first that has a back sector that isn't the inner part of the donut.
        // This back sector gives us the floor height to raise to and also the floor pic to change to.
        for (int32_t lineIdx = 0; lineIdx < pNextSector->linecount; ++lineIdx) {
            line_t& nextSecLine = *pNextSector->lines[lineIdx];

            #if PSYDOOM_MODS && PSYDOOM_FIX_UB
                // PsyDoom: safety - allow non two sided lines here, just skip over them
                if (!nextSecLine.backsector)
                    continue;
            #else
                ASSERT(nextSecLine.backsector);
            #endif

            // Ignore this line if the back sector is the same as the inner part of the donut
            sector_t& backSector = *nextSecLine.backsector;

            if (&backSector == &sector)
                continue;

            // Create the mover for the outer part of the donut.
            // This raises the floor to the height of the back sector we just found and changes the texture to that.
            // This is normally used to raise slime and change the slime texture.
            {
                floormove_t& floorMove = *(floormove_t*) Z_Malloc(*gpMainMemZone, sizeof(floormove_t), PU_LEVSPEC, nullptr);

                #if PSYDOOM_MODS
                    floorMove = {};     // PsyDoom: zero-init all fields to be safe
                #endif

                P_AddThinker(floorMove.thinker);
                pNextSector->specialdata = &floorMove;

                floorMove.thinker.function = (think_t) &T_MoveFloor;
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
                floormove_t& floorMove = *(floormove_t*) Z_Malloc(*gpMainMemZone, sizeof(floormove_t), PU_LEVSPEC, nullptr);

                #if PSYDOOM_MODS
                    floorMove = {};     // PsyDoom: zero-init all fields to be safe
                #endif

                P_AddThinker(floorMove.thinker);
                sector.specialdata = &floorMove;

                floorMove.thinker.function = (think_t) &T_MoveFloor;
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
static void P_ScheduleDelayedAction(const int32_t delayTics, const delayed_actionfn_t actionFunc) noexcept {
    delayaction_t& delayed = *(delayaction_t*) Z_Malloc(*gpMainMemZone, sizeof(delayaction_t), PU_LEVSPEC, nullptr);
    P_AddThinker(delayed.thinker);

    delayed.thinker.function = (think_t) &T_DelayedAction;
    delayed.ticsleft = delayTics;
    delayed.actionfunc = actionFunc;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Thinker function for performing a delayed action: performs the action after the delay time has passed and removes the thinker when done
//------------------------------------------------------------------------------------------------------------------------------------------
void T_DelayedAction(delayaction_t& action) noexcept {
    if (--action.ticsleft <= 0) {
        action.actionfunc();
        P_RemoveThinker(action.thinker);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedules the level to end in 4 tics and go to the next map; this is the usual method of exiting a level
//------------------------------------------------------------------------------------------------------------------------------------------
void G_ExitLevel() noexcept {
    gNextMap = gGameMap + 1;
    P_ScheduleDelayedAction(4, G_CompleteLevel);
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: same as 'G_ExitLevel()' except it happens immediately
//------------------------------------------------------------------------------------------------------------------------------------------
void G_ExitLevelImmediately() noexcept {
    gNextMap = gGameMap + 1;
    G_CompleteLevel();
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Schedules the level to end in 4 tics and go to the specified map; used for entering/exiting secret maps
//------------------------------------------------------------------------------------------------------------------------------------------
void G_SecretExitLevel(const int32_t nextMap) noexcept {
    gNextMap = nextMap;
    P_ScheduleDelayedAction(4, G_CompleteLevel);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Spawns special thinkers (for lights) in a level and sets up other special related stuff
//------------------------------------------------------------------------------------------------------------------------------------------
void P_SpawnSpecials() noexcept {
    // Spawn thinkers for sector specials
    for (int32_t sectorIdx = 0; sectorIdx < gNumSectors; ++sectorIdx) {
        sector_t& sector = gpSectors[sectorIdx];

        if (!sector.special)
            continue;

        switch (sector.special) {
            case 1:     P_SpawnLightFlash(sector);                      break;  // Flickering lights
            case 2:     P_SpawnStrobeFlash(sector, FASTDARK, false);    break;  // Strobe fast
            case 3:     P_SpawnStrobeFlash(sector, SLOWDARK, false);    break;  // Strobe slow
            case 8:     P_SpawnGlowingLight(sector, glowtolower);       break;  // Glowing light
            case 9:     gTotalSecret++;                                 break;  // Secret sector
            case 10:    P_SpawnDoorCloseIn30(sector);                   break;  // Door close in 30 seconds
            case 12:    P_SpawnStrobeFlash(sector, SLOWDARK, true);     break;  // Sync strobe slow
            case 13:    P_SpawnStrobeFlash(sector, FASTDARK, true);     break;  // Sync strobe fast
            case 14:    P_SpawnDoorRaiseIn5Mins(sector, sectorIdx);     break;  // Door raise in 5 minutes
            case 17:    P_SpawnFireFlicker(sector);                     break;  // Fire flicker

        #if PSYDOOM_MODS
            // PsyDoom addition: an actual 'strobe hurt' (strobe flash + hurt) for the PSX engine.
            // This behaves the same as special '4' did originally on PC.
            case 32:
                P_SpawnStrobeFlash(sector, FASTDARK, false);
                sector.special = 32;
                break;
        #endif

            case 200:   P_SpawnGlowingLight(sector, glowto10);          break;  // Glow to dark
            case 201:   P_SpawnGlowingLight(sector, glowto255);         break;  // Glow to bright
            case 202:   P_SpawnRapidStrobeFlash(sector);                break;  // Rapid strobe flash (PSX addition)
            case 204:   P_SpawnStrobeFlash(sector, TURBODARK, false);   break;  // Strobe flash

            // PsyDoom: scripted sector special spawn
            #if PSYDOOM_MODS
                case 300: {
                    sector.special = 0;
                    ScriptingEngine::doAction(sector.tag, nullptr, &sector, nullptr, 0, 0);
                }   break;
            #endif

            default:    break;
        }
    }

    // Save scrolling line specials to their own list (for quick updating)
    #if PSYDOOM_LIMIT_REMOVING
        gpLineSpecialList.clear();
        gpLineSpecialList.reserve(256);
    #else
        gNumLinespecials = 0;
    #endif

    for (int32_t lineIdx = 0; lineIdx < gNumLines; ++lineIdx) {
        line_t& line = gpLines[lineIdx];

        switch (line.special) {
            case 200:   // Effect: scroll left
            case 201:   // Effect: scroll right
            case 202:   // Effect: scroll up
            case 203:   // Effect: scroll down
            {
                #if PSYDOOM_LIMIT_REMOVING
                    gpLineSpecialList.push_back(&line);
                #else
                    if (gNumLinespecials < MAXLINEANIMS) {
                        gpLineSpecialList[gNumLinespecials] = &line;
                        gNumLinespecials++;
                    }
                #endif
            }   break;

            // PsyDoom: scripted spawn line special
            #if PSYDOOM_MODS
                case 380: {
                    line.special = 0;
                    ScriptingEngine::doAction(line.tag, &line, line.frontsector, nullptr, 0, 0);
                } break;
            #endif

            default:
                break;
        }
    }

    // PSX addition: if special sectors for bosses are found, set the appropriate boss special flag bits.
    // These are sectors that activate when all enemies of the corresponding boss type are killed.
    gMapBossSpecialFlags = 0;

    for (int32_t sectorIdx = 0; sectorIdx < gNumSectors; ++sectorIdx) {
        sector_t& sector = gpSectors[sectorIdx];

        switch (sector.tag) {
            case 666:   gMapBossSpecialFlags |= 0x01;  break;   // Kill all 'MT_FATSO' to activate this 'lowerFloorToLowest' floor
            case 667:   gMapBossSpecialFlags |= 0x02;  break;   // Kill all 'MT_BABY' to activate this 'raiseFloor24' floor
            case 668:   gMapBossSpecialFlags |= 0x04;  break;   // Kill all 'MT_SPIDER' to activate this 'lowerFloorToLowest' floor
            case 669:   gMapBossSpecialFlags |= 0x08;  break;   // Kill all 'MT_KNIGHT' to activate this 'lowerFloorToLowest' floor
            case 670:   gMapBossSpecialFlags |= 0x10;  break;   // Kill all 'MT_CYBORG' to activate this 'Open' door
            case 671:   gMapBossSpecialFlags |= 0x20;  break;   // Kill all 'MT_BRUISER' to activate this 'lowerFloorToLowest' floor

            // PsyDoom: adding a special action for when all Commander Keens die (re-implemented actor type from PC)
            #if PSYDOOM_MODS
                case 672: gMapBossSpecialFlags |= 0x40; break;  // Kill all 'MT_KEEN' to activate this 'Open' door
            #endif

            default:
                break;
        }
    }

    // Run through all map objects to see if any skull keys are being used instead of keycards.
    // This is required for the correct sprite to be used for HUD key flashes (skull vs card).
    gMapRedKeyType = it_redcard;
    gMapBlueKeyType = it_bluecard;
    gMapYellowKeyType = it_yellowcard;

    for (mobj_t* pmobj = gMobjHead.next; pmobj != &gMobjHead; pmobj = pmobj->next) {
        switch (pmobj->type) {
            case MT_MISC7:  gMapYellowKeyType   = it_yellowskull;   break;
            case MT_MISC8:  gMapRedKeyType      = it_redskull;      break;
            case MT_MISC9:  gMapBlueKeyType     = it_blueskull;     break;

            default: break;
        }
    }

    // Clear these lists
    #if PSYDOOM_LIMIT_REMOVING
        gpActiveCeilings.clear();
        gpActivePlats.clear();
        gpActiveCeilings.reserve(128);
        gpActivePlats.reserve(128);
        gButtonList.clear();
        gButtonList.reserve(16);
    #else
        D_memset(gpActiveCeilings, std::byte(0), MAXCEILINGS * sizeof(gpActiveCeilings[0]));
        D_memset(gpActivePlats, std::byte(0), MAXPLATS * sizeof(gpActivePlats[0]));
        D_memset(gButtonList, std::byte(0), MAXBUTTONS * sizeof(gButtonList[0]));
    #endif
}
