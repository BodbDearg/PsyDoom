#include "p_info.h"

#if PSYDOOM_MODS

#include "Doom/Base/i_main.h"
#include "Doom/Base/w_wad.h"
#include "info.h"
#include "PsyDoom/ParserTokenizer.h"
#include "sprinfo.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

static std::vector<state_t>     gStateVec;          // All of the states defined by the game
static std::vector<mobjinfo_t>  gMobjInfoVec;       // All of the map objects defined by the game

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks to see if the specified DoomEd num is in use already
//------------------------------------------------------------------------------------------------------------------------------------------
static bool P_IsDoomEdNumUsed(const int32_t doomEdNum) noexcept {
    for (const mobjinfo_t& mobjInfo : gMobjInfoVec) {
        if (mobjInfo.doomednum == doomEdNum)
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: parse the frame list for a decor map object from the specified token string.
// Generates the finite state machine states for the decor object and links them together.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_ParseDecorFrameList(
    const char* const lumpName,
    const int32_t curLineIdx,
    const std::string& tokenStr,
    mobjinfo_t& mobjInfo,
    const int32_t spriteIndex
) noexcept {
    // Indexes for the beginning and end of the parsed state list
    int32_t begStateIdx = (int32_t) gStateVec.size();
    int32_t endStateIdx = begStateIdx;

    // Continue parsing states (durations + frame indexes) for the map object
    const char* pStr = tokenStr.c_str();

    while (*pStr) {
        // Parse and validate the tic count
        const int32_t tics = std::atoi(pStr);

        if (tics <= 0) {
            I_Error("Invalid decor tic count for lump '%s' line %d", lumpName, curLineIdx + 1);
        }

        // Skip past the tick count (digits)
        while (std::isdigit(*pStr)) {
            ++pStr;
        }

        // Get the sprite frame index (a single character)
        const int32_t frameIdx = pStr[0] - 'A';
        ++pStr;

        if ((frameIdx < 0) || (frameIdx > 30)) {
            I_Error("Invalid decor frame number for lump '%s' line %d", lumpName, curLineIdx + 1);
        }

        // Save this state
        state_t& state = gStateVec.emplace_back();
        state.sprite = (spritenum_t) spriteIndex;
        state.frame = frameIdx;
        state.tics = tics;

        // Link with the previous state
        if (endStateIdx > begStateIdx) {
            gStateVec[endStateIdx - 1].nextstate = (statenum_t) endStateIdx;
        }

        ++endStateIdx;
    }

    // There must be at least 1 frame defined
    if (begStateIdx >= endStateIdx) {
        I_Error("Invalid decor for lump '%s' line %d, no frames defined!", lumpName, curLineIdx + 1);
    }

    // Make the last frame loop back around to the first and set the first state as the map object spawn state
    gStateVec[endStateIdx - 1].nextstate = (statenum_t) begStateIdx;
    mobjInfo.spawnstate = (statenum_t) begStateIdx;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: parse one of the optional attributes & flags for a decor map object:
//      [RADIUS:X]
//      [HEIGHT:X]
//      [CEILING]
//      [BLOCKING]
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_ParseDecorAttribute(
    const char* const lumpName,
    const int32_t curLineIdx,
    const std::string& tokenStr,
    mobjinfo_t& mobjInfo
) noexcept {
    if (tokenStr == "CEILING") {
        mobjInfo.flags |= MF_SPAWNCEILING;
    }
    else if (tokenStr == "BLOCKING") {
        mobjInfo.flags |= MF_SOLID;
        mobjInfo.flags &= (~MF_NOBLOCKMAP);
    }
    else if (tokenStr.find("RADIUS:") == 0) {
        mobjInfo.radius = (fixed_t)(std::atof(tokenStr.c_str() + 7) * 65536.0);
    }
    else if (tokenStr.find("HEIGHT:") == 0) {
        mobjInfo.height = (fixed_t)(std::atof(tokenStr.c_str() + 7) * 65536.0);
    }
    else {
        I_Error("Invalid decor attribute '%s' for lump '%s' line %d", tokenStr.c_str(), lumpName, curLineIdx + 1);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: reads the PsyDoom 'decorate' lump which is used to define new basic props/decor. 
// 
// Grammar for these definitions (one per line):
//      DOOMEDNUM SPRITENAME FRAMES [RADIUS:X] [HEIGHT:X] [CEILING] [BLOCKING]
// 
// Where:
//  DOOMEDNUM       Doom editor number for the thing
//  SPRITENAME      4 character name for the sprite, e.g 'CEYE'
//  FRAMES          A list of tic duration and frame letter pairs, in one unbroken string (no spaces).
//                  E.G: 2A3B4C will play frame 'A' for 2 tics, 'B' for 3 tics and 'C' for 4 tics.
//  [RADIUS:X]      Optionally specify the thing's radius as 'X'. Note: there must be no spaces between the colon and number.
//  [HEIGHT:X]      Optionally specify the thing's height as 'X'. Note: there must be no spaces between the colon and number.
//  [CEILING]       Optional flag. If specified then the decor is attached to the ceiling.
//  [BLOCKING]      Optional flag. If specified then the thing is treated as solid/blocking.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_ReadDecorateLump(const char* const lumpName) noexcept {
    // Does the lump exist?
    const int32_t lumpIdx = W_CheckNumForName(lumpName);

    if (lumpIdx < 0)
        return;

    // Read the lump entirely and null terminate the text data
    const int32_t lumpSize = W_LumpLength(lumpIdx);
    std::unique_ptr<char[]> lumpChars(new char[lumpSize + 1]);
    W_ReadLump(lumpIdx, lumpChars.get(), true);
    lumpChars[lumpSize] = 0;

    // Re-use this string during parsing
    std::string tmpStr;
    tmpStr.reserve(64);

    // The name of the sprite for the current decoration
    sprname_t sprname = {};
    int32_t curLineIdx = 0;

    // Parse the decoration definitions
    ParserTokenizer::visitAllLineTokens(
        lumpChars.get(),
        lumpChars.get() + lumpSize,
        [&](const int32_t lineIdx) noexcept {
            // A new decor definition lies ahead: initialize its basic properties
            curLineIdx = lineIdx;

            mobjinfo_t& mobjInfo = gMobjInfoVec.emplace_back();
            mobjInfo.spawnhealth = 1000;
            mobjInfo.radius = 16 * FRACUNIT;
            mobjInfo.height = 64 * FRACUNIT;
            mobjInfo.mass = 100;
            mobjInfo.flags = MF_NOGRAVITY | MF_NOBLOCKMAP;
        },
        [&](const int32_t tokenIdx, const char* const token, const size_t tokenLen) noexcept {
            // Make the token ASCII uppercase for case insensitive comparison
            tmpStr.assign(token, tokenLen);
            std::transform(tmpStr.begin(), tmpStr.end(), tmpStr.begin(), [](const char c) noexcept { return (char) std::toupper(c); });

            // Parse each token
            mobjinfo_t& mobjInfo = gMobjInfoVec.back();

            if (tokenIdx == 0) {
                // DoomEd number
                const int32_t doomEdNum = std::atoi(tmpStr.c_str());
                
                if (doomEdNum == 0) {
                    I_Error("Invalid DoomEdNum '%s' for lump '%s' line %d!", tmpStr.c_str(), lumpName, curLineIdx + 1);
                }

                if (P_IsDoomEdNumUsed(doomEdNum)) {
                    I_Error("DoomEdNum '%d' for lump '%s' line %d is invalid - already in use!", mobjInfo.doomednum, lumpName, curLineIdx + 1);
                }

                mobjInfo.doomednum = doomEdNum;
            }
            else if (tokenIdx == 1) {
                // Sprite name
                if (tmpStr.length() > sprname_t::MAX_LEN) {
                    I_Error("Sprite name too long for lump '%s' line %d, must be 4 chars max!", lumpName, curLineIdx + 1);
                }

                sprname = tmpStr.c_str();
            }
            else if (tokenIdx == 2) {
                // Frame list
                P_ParseDecorFrameList(lumpName, curLineIdx, tmpStr, mobjInfo, P_SpriteGetNumForName(sprname));
            }
            else {
                // All other optional attributes
                P_ParseDecorAttribute(lumpName, curLineIdx, tmpStr, mobjInfo);
            }
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// New for PsyDoom: initializes the lists of 'state_t' and 'mobjinfo_t' for the game.
// Most of this actor and state machine data is built into the game, but PsyDoom allows very basic decoration sprites to be defined also.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_InitMobjInfo() noexcept {
    // Add base states to the states vector
    gStateVec.clear();
    gStateVec.reserve(2048);
    gStateVec.insert(gStateVec.begin(), gBaseStates, gBaseStates + BASE_NUM_STATES);

    // Add base map objects to the vector
    gMobjInfoVec.clear();
    gMobjInfoVec.reserve(512);
    gMobjInfoVec.insert(gMobjInfoVec.begin(), gBaseMobjInfo, gBaseMobjInfo + BASE_NUM_MOBJ_TYPES);

    // Read new states and map objects defined by the PsyDoom format 'decorate' lump
    P_ReadDecorateLump("PSYDECOR");

    // Setup the global lists of states and map objects used by the game
    gStates = gStateVec.data();
    gNumStates = (int32_t) gStateVec.size();
    gMobjInfo = gMobjInfoVec.data();
    gNumMobjInfo = (int32_t) gMobjInfoVec.size();
}

#endif  // #if PSYDOOM_MODS
