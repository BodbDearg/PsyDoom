//------------------------------------------------------------------------------------------------------------------------------------------
// Definitions for sprite frames and sequences of sprite frames.
// 
// Note: this module has been rewritten for PsyDoom to dynamically generate the list of sprites based on the main WADs loaded.
// Previously this list was fixed at compile time and contained hardcoded lump indexes, which could hinder modding the game.
// For the original version of this, see the 'Old' code folder.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "sprinfo.h"

#if PSYDOOM_MODS

#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "SmallString.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

const spritedef_t*  gSprites;       // Externally visible list of sprites in the game: just points to 'gSpriteDefs'
int32_t             gNumSprites;    // Externally visible list of sprites in the game: gives the number of sprites in 'gSpriteDefs'

static std::vector<spriteframe_t>                   gSpriteFrames;
static std::vector<spritedef_t>                     gSpriteDefs;
static std::unordered_map<uint32_t, spritedef_t*>   gSprNameToDef;      // LUT for accelerated lookup, use raw int as the key to avoid hashing difficulties

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: iterates through all of the sprite lumps defined in the main WAD file(s).
// Calls the specified function and passes in the current sprite lump index and name.
// Iterates backwards (recursively) through sprite lists so that sprites from WADs at the front of the WAD list take precedence.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
static void forEachSpriteLump(const T& callback, const int32_t searchStartLumpIdx = 0) noexcept {
    // Get the start and end of this sprite range (if there is one)
    const int32_t startLumpIdx = W_CheckNumForName("S_START", searchStartLumpIdx);

    if (startLumpIdx < 0)
        return;

    const int32_t endLumpIdx = W_CheckNumForName("S_END", startLumpIdx + 1);

    if (endLumpIdx < 0) {
        I_Error("WAD error: no 'S_END' found after 'S_START'!");
    }

    // Process later ranges first, then process all sprites in this range
    forEachSpriteLump(callback, endLumpIdx + 1);

    for (int32_t lumpIdx = startLumpIdx + 1; lumpIdx < endLumpIdx; ++lumpIdx) {
        const WadLumpName& lumpName = W_GetLumpName(lumpIdx);
        callback(lumpIdx, lumpName);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: extracts the sprite name from a WAD lump name (first 4 characters)
//------------------------------------------------------------------------------------------------------------------------------------------
static sprname_t getSpriteName(const WadLumpName lumpName) noexcept {
    // Note: need to kill the high bit of the 1st char (compressed flag)
    return sprname_t(lumpName.chars[0] & 0x7F, lumpName.chars[1], lumpName.chars[2], lumpName.chars[3]);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: raise an error about a particular sprite name
//------------------------------------------------------------------------------------------------------------------------------------------
[[noreturn]]
static void raiseSpriteError(const sprname_t name, const char* const msg) noexcept {
    I_Error("Sprite %c%c%c%c: %s", name.chars[0], name.chars[1], name.chars[2], name.chars[3], msg);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Builds a map of sprite names and the number of frames for each sprite
//------------------------------------------------------------------------------------------------------------------------------------------
static std::unordered_map<uint32_t, uint32_t> determineSpriteFrameCounts() noexcept {
    // Make a list of what frames are defined for each sprite.
    // Use a bit mask of up to 31 bits, with each bit representing a single frame.
    std::unordered_map<uint32_t, uint32_t> spriteFrames;
    spriteFrames.reserve(4096);

    forEachSpriteLump([&]([[maybe_unused]] const int32_t lumpIdx, const WadLumpName lumpName) noexcept {
        // Get which frames are currently defined for this sprite
        const sprname_t sprName = getSpriteName(lumpName);
        uint32_t& definedFrames = spriteFrames[sprName.word];

        // Get the normal and flipped sprite number from the lump name and incorporate them into the bit set if they are valid.
        // Up to 31 sprite frames can be defined starting from ASCII 'A', ignore any others.
        const int32_t normSprNum = (int32_t) lumpName.chars[4] - int32_t('A');
        const int32_t flipSprNum = (int32_t) lumpName.chars[6] - int32_t('A');

        if ((normSprNum >= 0) && (normSprNum <= 30)) {
            definedFrames |= 1 << normSprNum;
        } else {
            raiseSpriteError(sprName, "Invalid frame num!");
        }

        if (lumpName.chars[6]) {
            if ((flipSprNum >= 0) && (flipSprNum <= 30)) {
                definedFrames |= 1 << flipSprNum;
            } else {
                raiseSpriteError(sprName, "Invalid frame num!");
            }
        }
    });

    // Verify that the list of frame numbers for all sprites is contiguous (no gaps) and convert from a bit set to a frame count
    {
        const auto endIter = spriteFrames.end();

        for (auto iter = spriteFrames.begin(); iter != endIter; ++iter) {
            const sprname_t sprName = iter->first;
            uint32_t definedFrames = iter->second;
            uint32_t numFrames = 0;

            while (definedFrames != 0) {
                if ((definedFrames & 1) == 0) {
                    raiseSpriteError(sprName, "Non-contiguous frame list!");
                }

                definedFrames >>= 1;
                numFrames++;
            }

            iter->second = numFrames;
        }
    }

    return spriteFrames;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Counts the total number of sprite frames among all sprites, given the frame count for each sprite type
//------------------------------------------------------------------------------------------------------------------------------------------
static uint32_t getTotalSpriteFrameCount(const std::unordered_map<uint32_t, uint32_t>& spriteFrameCounts) noexcept {
    uint32_t total = 0;

    for (const auto& kvp : spriteFrameCounts) {
        total += kvp.second;
    }

    return total;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Rebuilds the LUT from sprite name to definition
//------------------------------------------------------------------------------------------------------------------------------------------
static void buildSpriteNameToDefLUT() noexcept {
    gSprNameToDef.clear();
    gSprNameToDef.reserve(gSpriteDefs.size() * 8);

    for (spritedef_t& spriteDef : gSpriteDefs) {
        gSprNameToDef[spriteDef.name.word] = &spriteDef;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Rebuilds the list of sprite definitions given the frame count for each sprite type.
// Assumes that the list of sprite frames has already been allocated.
//------------------------------------------------------------------------------------------------------------------------------------------
static void buildSpriteDefList(const std::unordered_map<uint32_t, uint32_t>& spriteFrameCounts) noexcept {
    // Build the list of sprite definitions
    gSpriteDefs.clear();
    gSpriteDefs.reserve(spriteFrameCounts.size());

    spriteframe_t* pSpriteFrames = gSpriteFrames.data();

    for (const auto& kvp : spriteFrameCounts) {
        const sprname_t sprName = kvp.first;
        const uint32_t frameCount = kvp.second;

        spritedef_t& spriteDef = gSpriteDefs.emplace_back();
        spriteDef.name = sprName;
        spriteDef.numframes = (int32_t) frameCount;
        spriteDef.spriteframes = pSpriteFrames;

        pSpriteFrames += frameCount;
        ASSERT(pSpriteFrames <= gSpriteFrames.data() + gSpriteFrames.size());
    }

    // Populate the external view of this list
    gSprites = gSpriteDefs.data();
    gNumSprites = (int32_t) gSpriteDefs.size();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determines which lumps are used for each sprite and viewing angles.
// Expects the lists of sprites to have been built and the sprite name to definition LUT to have been populated.
//------------------------------------------------------------------------------------------------------------------------------------------
static void populateSpriteFrameList() noexcept {
    // Fill in lump numbers and 'flipped' info for each sprite frame
    forEachSpriteLump([&]([[maybe_unused]] const int32_t lumpIdx, const WadLumpName lumpName) noexcept {
        // Get the sprite associated with this lump (should exist at this point)
        const sprname_t sprName = getSpriteName(lumpName);
        ASSERT(gSprNameToDef[sprName.word]);
        spritedef_t& spriteDef = *gSprNameToDef[sprName.word];

        // Get the normal and flipped sprite number and direction from the lump name.
        // Note that the flipped sprite number might not be defined.
        const int32_t normSprNum = lumpName.chars[4] - 'A';
        const int32_t normSprDir = lumpName.chars[5] - '0';
        const int32_t flipSprNum = lumpName.chars[6] - 'A';
        const int32_t flipSprDir = lumpName.chars[7] - '0';

        // Set the lump number and flipped info for the regular sprite frame
        if ((normSprNum >= 0) && (normSprNum <= 30)) {
            spriteframe_t& frame = spriteDef.spriteframes[normSprNum];

            if ((normSprDir >= 0) && (normSprDir <= 8)) {
                // Is this lump for all directions?
                if (normSprDir == 0) {
                    for (int32_t i = 0; i < 8; ++i) {
                        frame.lump[i] = lumpIdx;
                        frame.flip[i] = false;
                    }
                } else {
                    frame.lump[normSprDir - 1] = lumpIdx;
                    frame.flip[normSprDir - 1] = false;
                }
            } else {
                raiseSpriteError(sprName, "Invalid sprite dir!");
            }
        } else {
            raiseSpriteError(sprName, "Invalid frame num!");
        }

        // Set the lump number and flipped info for the flipped sprite frame (if existing)
        if (lumpName.chars[6]) {
            if ((flipSprNum >= 0) && (flipSprNum <= 30)) {
                spriteframe_t& frame = spriteDef.spriteframes[flipSprNum];

                if ((flipSprDir >= 0) && (flipSprDir <= 8)) {
                    // Is this lump for all directions?
                    if (flipSprDir == 0) {
                        for (int32_t i = 0; i < 8; ++i) {
                            frame.lump[i] = lumpIdx;
                            frame.flip[i] = true;
                        }
                    } else {
                        frame.lump[flipSprDir - 1] = lumpIdx;
                        frame.flip[flipSprDir - 1] = true;
                    }
                } else {
                    raiseSpriteError(sprName, "Invalid sprite dir!");
                }
            } else {
                raiseSpriteError(sprName, "Invalid frame num!");
            }
        }
    });

    // Fill in the 'rotated' flag for each sprite frame definition
    for (spriteframe_t& frame : gSpriteFrames) {
        const bool bAllLumpsTheSame = (
            (frame.lump[0] == frame.lump[1]) &&
            (frame.lump[1] == frame.lump[2]) &&
            (frame.lump[2] == frame.lump[3]) &&
            (frame.lump[3] == frame.lump[4]) &&
            (frame.lump[4] == frame.lump[5]) &&
            (frame.lump[5] == frame.lump[6]) &&
            (frame.lump[6] == frame.lump[7])
        );

        frame.rotate = (!bAllLumpsTheSame);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Sorts the built-in sprite definitions first in the sprite list, and according to the order of the 'gBaseSprNames' list.
// Following that any new (user-mod) sprite definitions are sorted according to name.
//------------------------------------------------------------------------------------------------------------------------------------------
static void sortSpriteDefs() noexcept {
    // Get the order of the built-in sprite definitions
    std::unordered_map<uint32_t, uint32_t> spriteOrder;
    spriteOrder.reserve(BASE_NUM_SPRITES * 8);

    for (int32_t i = 0; i < BASE_NUM_SPRITES; ++i) {
        spriteOrder[gBaseSprNames[i].word] = i;
    }

    // Sort the sprite list by order first, then name
    std::sort(
        gSpriteDefs.begin(),
        gSpriteDefs.end(),
        [&](const spritedef_t & s1, const spritedef_t & s2) noexcept {
            // Compare by order
            const auto orderIter1 = spriteOrder.find(s1.name.word);
            const auto orderIter2 = spriteOrder.find(s2.name.word);
            const int32_t order1 = (orderIter1 != spriteOrder.end()) ? orderIter1->second : INT32_MAX;
            const int32_t order2 = (orderIter2 != spriteOrder.end()) ? orderIter2->second : INT32_MAX;

            if (order1 != order2)
                return (order1 < order2);

            // Failing that compare by name
            return (s1.name.word < s2.name.word);
        }
    );

    // Need to rebuild this LUT after sorting all of the sprites
    buildSpriteNameToDefLUT();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Builds the data for all sprites in the game from the main WAD file(s)
//------------------------------------------------------------------------------------------------------------------------------------------
void P_InitSprites() noexcept {
    // Get a list of sprites defined in the main WAD(s) and how many frames each sprite has.
    // Then allocate the list of sprite frames required.
    std::unordered_map<uint32_t, uint32_t> spriteFrameCounts = determineSpriteFrameCounts();
    gSpriteFrames.clear();
    gSpriteFrames.resize(getTotalSpriteFrameCount(spriteFrameCounts));

    // Allocate the list of sprite definitions and assign each one it's sprite frame list, then build an LUT for this list
    buildSpriteDefList(spriteFrameCounts);
    buildSpriteNameToDefLUT();

    // Fill in the lump and rotation info for all sprites then sort the list of sprite definitions
    populateSpriteFrameList();
    sortSpriteDefs();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the index of the sprite for the specified name, or '-1' if not found
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t P_SpriteCheckNumForName(const sprname_t name) noexcept {
    const auto iter = gSprNameToDef.find(name.word);
    return (iter != gSprNameToDef.end()) ? (int32_t)(iter->second - gSpriteDefs.data()) : -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Same as 'P_SpriteCheckNumForName' except an error is issued if the sprite name is not found
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t P_SpriteGetNumForName(const sprname_t name) noexcept {
    const int32_t spriteIdx = P_SpriteCheckNumForName(name);

    if (spriteIdx >= 0)
        return spriteIdx;

    I_Error("P_SpriteGetNumForName: not found '%c%c%c%c'!", name.chars[0], name.chars[1], name.chars[2], name.chars[3]);
    return -1;
}

#endif  // #if PSYDOOM_MODS
