//------------------------------------------------------------------------------------------------------------------------------------------
// A module which handles precaching all the sprites needed for a map.
// 
// Required because PsyDoom no longer uses 'MAPSPR__.IMG' and 'MAPTEX__.IMG' files (to make modding easier). These files were essentially
// caches of all the textures and sprites needed for a map, arranged in nice flat files for fast CD-ROM access.
// 
// Instead for PsyDoom, we load all resources from the main IWAD and do it once during map load so there are no hitches during gameplay.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MobjSpritePrecacher.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Game/sprinfo.h"
#include "Doom/Renderer/r_data.h"

#include <cstring>
#include <vector>

BEGIN_NAMESPACE(MobjSpritePrecacher)

static std::vector<bool>    gbCacheSprite;                      // Whether to precache each sprite in the game
static bool                 gbCachedMobjType[NUMMOBJTYPES];     // Whether sprites were precached for each 'mobjtype_t'

//------------------------------------------------------------------------------------------------------------------------------------------
// Clears the set of sprites to be precached and the set of map objects marked as precached
//------------------------------------------------------------------------------------------------------------------------------------------
static void clearPrecacheInfo() noexcept {
    gbCacheSprite.clear();
    gbCacheSprite.resize(gNumSprites);

    std::memset(gbCachedMobjType, 0, sizeof(gbCachedMobjType));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flag a sprite as needing precaching
//------------------------------------------------------------------------------------------------------------------------------------------
static void flagSpritesForPrecache() noexcept {}

template <class ...Types> 
static void flagSpritesForPrecache(const spritenum_t sprNum, Types... types) noexcept {
    gbCacheSprite[sprNum] = true;
    flagSpritesForPrecache(types...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Precache the sprite for a particular 'statenum_t'
//------------------------------------------------------------------------------------------------------------------------------------------
static void flagSpriteForPrecache(const statenum_t& stateType) noexcept {
    if (stateType != S_NULL) {
        const state_t& state = gStates[stateType];
        flagSpritesForPrecache(state.sprite);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flag all sprites referenced by a 'mobjinfo_t' for precaching.
// Searches through the basic states of the info block for sprites and precaches them all.
//------------------------------------------------------------------------------------------------------------------------------------------
static void flagSpritesForPrecache(const mobjinfo_t& info) noexcept {
    flagSpriteForPrecache(info.spawnstate);
    flagSpriteForPrecache(info.seestate);
    flagSpriteForPrecache(info.painstate);
    flagSpriteForPrecache(info.meleestate);
    flagSpriteForPrecache(info.missilestate);
    flagSpriteForPrecache(info.deathstate);
    flagSpriteForPrecache(info.xdeathstate);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flag sprites referenced by all of the specified 'mobjtype_t' as needing precaching
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...Types> 
static void flagSpritesForPrecache(const mobjtype_t& type, Types... types) noexcept {
    flagSpritesForPrecache(gMObjInfo[type]);
    flagSpritesForPrecache(types...);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flag extra 'mobjtype_t' types to precache for a specific 'mobjtype_t'.
// This is used to manually specify additional required sprites which can't be determined automatically from 'mobjinfo_t'.
// It covers relationships that are only defined in code, like monsters that fire certain projectiles, weapon drops and so on.
//------------------------------------------------------------------------------------------------------------------------------------------
static void flagIndirectDependenciesForPrecache(const mobjtype_t& type) noexcept {
    switch (type) {
        case MT_BABY:       flagSpritesForPrecache(MT_ARACHPLAZ);           break;
        case MT_BRUISER:    flagSpritesForPrecache(MT_BRUISERSHOT);         break;
        case MT_CHAINGUY:   flagSpritesForPrecache(MT_CHAINGUN);            break;
        case MT_FATSO:      flagSpritesForPrecache(MT_FATSHOT);             break;
        case MT_HEAD:       flagSpritesForPrecache(MT_HEADSHOT);            break;
        case MT_KNIGHT:     flagSpritesForPrecache(MT_BRUISERSHOT);         break;
        case MT_PAIN:       flagSpritesForPrecache(MT_SKULL);               break;
        case MT_POSSESSED:  flagSpritesForPrecache(MT_CLIP);                break;
        case MT_SHOTGUY:    flagSpritesForPrecache(MT_SHOTGUN);             break;
        case MT_TROOP:      flagSpritesForPrecache(MT_TROOPSHOT);           break;
        case MT_UNDEAD:     flagSpritesForPrecache(MT_TRACER, MT_SMOKE);    break;

        default: break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flag sprites for the specified player weapon that need precaching
//------------------------------------------------------------------------------------------------------------------------------------------
static void flagSpritesForPrecache(const weaponinfo_t& weaponInfo) noexcept {
    flagSpriteForPrecache(weaponInfo.upstate);
    flagSpriteForPrecache(weaponInfo.downstate);
    flagSpriteForPrecache(weaponInfo.readystate);
    flagSpriteForPrecache(weaponInfo.atkstate);
    flagSpriteForPrecache(weaponInfo.flashstate);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flag sprites in general that need precaching, regardless of things in the map
//------------------------------------------------------------------------------------------------------------------------------------------
static void flagGeneralSpritesToPrecache() noexcept {
    flagSpritesForPrecache(MT_PUFF, MT_BLOOD, MT_TFOG);     // General particle fx 
    flagSpritesForPrecache(MT_TELEPORTMAN);                 // Just in case it's modified to show something
    flagSpritesForPrecache(SPR_POL5);                       // The gib sprite

    if (gNetGame != gt_single) {
        flagSpritesForPrecache(MT_PLAYER, MT_IFOG);         // Multiplayer: player sprites and item respawn sprites
    }

    // Precache all player weapon sprites and projectiles
    for (const weaponinfo_t& weaponInfo : gWeaponInfo) {
        flagSpritesForPrecache(weaponInfo);
    }

    flagSpritesForPrecache(MT_ROCKET, MT_PLASMA, MT_BFG, MT_EXTRABFG);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flags all sprites used by things in the level for precaching
//------------------------------------------------------------------------------------------------------------------------------------------
static void flagAllThingSpritesToPrecache() noexcept {
    // Run through all things in the map and precache their sprites
    for (const mobj_t* pMobj = gMObjHead.next; pMobj != &gMObjHead; pMobj = pMobj->next) {
        // Ignore this type if we already cached it
        const mobjtype_t type = pMobj->type;

        if (gbCachedMobjType[type])
            continue;

        // Ignore player objects, only want to precache those sprites manually in multiplayer
        if (type == MT_PLAYER)
            continue;

        // Cache all the sprites for this type, including indirect dependencies like fireballs
        gbCachedMobjType[type] = true;
        flagSpritesForPrecache(type);
        flagIndirectDependenciesForPrecache(type);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Flag the sprites to precache for the level
//------------------------------------------------------------------------------------------------------------------------------------------
static void flagSpritesToPrecache() noexcept {
    flagGeneralSpritesToPrecache();
    flagAllThingSpritesToPrecache();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Precaches all sprites that are flagged for precaching
//------------------------------------------------------------------------------------------------------------------------------------------
static void precacheSprites() noexcept {
    // How many sprite lumps are there?
    const int32_t firstSpriteLumpNum = gFirstSpriteLumpNum;
    const int32_t lastSpriteLumpNum = gLastSpriteLumpNum;
    const int32_t numSprites = gNumSprites;

    for (int32_t sprIdx = 0; sprIdx < numSprites; ++sprIdx) {
        // Ignore if this sprite was not flagged to be precached
        if (!gbCacheSprite[sprIdx])
            continue;

        // Otherwise cache the lumps for all sprite frames
        const spritedef_t& spriteDef = gSprites[sprIdx];
        const spriteframe_t* const pBegFrame = spriteDef.spriteframes;
        const spriteframe_t* const pEndFrame = pBegFrame + spriteDef.numframes;

        for (const spriteframe_t* pFrame = pBegFrame; pFrame < pEndFrame; ++pFrame) {
            for (int32_t sprLumpNum : pFrame->lump) {
                // Die with an error if the lump number is invalid, otherwise cache the sprite
                if ((sprLumpNum < firstSpriteLumpNum) || (sprLumpNum > lastSpriteLumpNum)) {
                    I_Error("SprCache: bad lump num %d!", sprLumpNum);
                }

                W_CacheLumpNum(sprLumpNum, PU_CACHE, false);
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called near the end of level setup to precache all the sprites needed for the map
//------------------------------------------------------------------------------------------------------------------------------------------
void doPrecaching() noexcept {
    clearPrecacheInfo();
    flagSpritesToPrecache();
    precacheSprites();
}

END_NAMESPACE(MobjSpritePrecacher)
