//------------------------------------------------------------------------------------------------------------------------------------------
// Dynamically loaded MAPINFO for the 'GEC Master Edition' (Beta 4 and later).
// These defaults are loaded from MAPINFO style text files, so could change if the disk image is altered.
// Much of this data is just copied to the regular MAPINFO structs that PsyDoom uses, but some of it is specific to this game type.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "GecMapInfo.h"

#include "Asserts.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Game/p_spec.h"
#include "Doom/Game/p_switch.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/UI/ti_main.h"
#include "MapInfo.h"
#include "MapInfo_Defaults_FinalDoom.h"
#include "MapInfo_Parse.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/PsxVm.h"
#include "PsyDoom/Utils.h"
#include "PsyQ/LIBSPU.h"

#include <algorithm>

BEGIN_NAMESPACE(GecMapInfo)

using namespace MapInfo;

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds a tokenized MAPINFO file and it's backing text storage
//------------------------------------------------------------------------------------------------------------------------------------------
struct MapInfoTokens {
    std::string         rawText;
    std::vector<Token>  tokens;     // Note: all text pointers reference the data in 'rawText'
};

//------------------------------------------------------------------------------------------------------------------------------------------
// All of the loaded MAPINFO
//------------------------------------------------------------------------------------------------------------------------------------------
static GameInfo                     gGameInfo;
static MenuSprite                   gTitleLogo;         // Definition for the "MASTER EDITION" logo that appears at the top of the screen
static std::vector<Episode>         gEpisodes;
static std::vector<Cluster>         gClusters;
static std::vector<Map>             gMaps;
static std::vector<Sky>             gSkies;
static std::vector<CreditsPage>     gCredits;
static std::vector<switchlist_t>    gBaseSwitchList;    // Built-in switch definitions, read from the MAPINFO text files
static std::vector<animdef_t>       gBaseAnimDefs;      // Built-in anim definitions, read from the MAPINFO text files

//------------------------------------------------------------------------------------------------------------------------------------------
// Context: which MAPINFO file is currently being parsed
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* gCurMapInfoFilePath = nullptr;

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: removes the 'F_' prefix for a sky lump name if it starts with 'F_SKY'.
// Useful to get the actual lump name to load from the WAD.
//------------------------------------------------------------------------------------------------------------------------------------------
static void removeSkyLumpNameF_Prefix(String8& skyLumpName) noexcept {
    const bool bIsFSkyLumpName = (
        (skyLumpName.chars[0] == 'F') &&
        (skyLumpName.chars[1] == '_') &&
        (skyLumpName.chars[2] == 'S') &&
        (skyLumpName.chars[3] == 'K') &&
        (skyLumpName.chars[4] == 'Y')
    );

    if (bIsFSkyLumpName) {
        // Remove the 'F_' prefix from the lump name
        for (int32_t i = 0; i < 6; ++i) {
            skyLumpName.chars[i] = skyLumpName.chars[i + 2];
        }

        skyLumpName.chars[6] = 0;
        skyLumpName.chars[7] = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: assigns text to the specified 'SmallString', filtering out non ASCII characters like UTF-8 byte order marks (0xEF, 0xBB, 0xBF).
// The filtering is used to fix UTF-8 BOM being present in some of the map names for Beta 4 of the Master Edition.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class SmallStringT>
static void assignAsciiTextToSmallString(SmallStringT& dst, std::string_view src) {
    const char* const pSrcChars = src.data();
    const size_t srcLen = src.length();
    
    uint32_t srcIdx = 0;
    uint32_t dstIdx = 0;

    while ((dstIdx < SmallStringT::MAX_LEN) && (srcIdx < srcLen)) {
        const uint8_t c = (uint8_t) pSrcChars[srcIdx];

        if (c <= 0x7F) {
            dst.chars[dstIdx] = (char) c;
            dstIdx++;
        }

        srcIdx++;
    }

    // Zero fill the remaining characters if we didn't reach the end.
    // This is required for WAD lumpname chunked comparisons (64-bit word compare) to work.
    while (dstIdx < SmallStringT::MAX_LEN) {
        dst.chars[dstIdx] = 0;
        dstIdx++;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Reads the specified text file on disc and performs MAPINFO tokenization.
// Aborts with a fatal error if the request fails.
//------------------------------------------------------------------------------------------------------------------------------------------
static MapInfoTokens tokenizeMapInfoFileOnDisk(const char* const filePath) noexcept {
    // Attempt to read the file and panic if that failed
    const Utils::DiscFileData fileData = Utils::getDiscFileData(PsxVm::gDiscInfo, PsxVm::gIsoFileSys, filePath);
    const bool bHaveFileData = (fileData.pBytes && (fileData.numBytes > 0));

    if (!bHaveFileData)
        FatalErrors::raiseF("Failed to read required MapInfo file '%s'!", filePath);

    // Save the raw data into a std::string (ensures null termination via 'c_str()') and tokenize it.
    // Note that this may fail with a fatal error.
    MapInfoTokens mapInfoTokens = {};
    mapInfoTokens.rawText.assign(reinterpret_cast<const char*>(fileData.pBytes.get()), fileData.numBytes);
    mapInfoTokens.tokens = tokenizeMapInfo(mapInfoTokens.rawText.c_str());
    return mapInfoTokens;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check that a token is of an expected type and issue a fatal error if not
//------------------------------------------------------------------------------------------------------------------------------------------
static void ensureTokenType(const Token* const pToken, const TokenType expectedType) noexcept {
    ASSERT(pToken);

    if (pToken->type == expectedType)
        return;

    const char* expectedTypeStr = "";

    switch (expectedType) {
        case TokenType::Null:           expectedTypeStr = "<NULL>";         break;
        case TokenType::Identifier:     expectedTypeStr = "identifier";     break;
        case TokenType::String:         expectedTypeStr = "string";         break;
        case TokenType::Number:         expectedTypeStr = "number";         break;
        case TokenType::True:           expectedTypeStr = "'true'";         break;
        case TokenType::False:          expectedTypeStr = "'false'";        break;
        case TokenType::Equals:         expectedTypeStr = "'='";            break;
        case TokenType::OpenBlock:      expectedTypeStr = "'{'";            break;
        case TokenType::CloseBlock:     expectedTypeStr = "'}'";            break;
        case TokenType::NextValue:      expectedTypeStr = "','";            break;
    }

    error(pToken->begin, "Expected token type: %s! File path: %s", expectedTypeStr, (gCurMapInfoFilePath) ? gCurMapInfoFilePath : "");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Check that a token is of an expected type and issue a fatal error if not.
// Once done, skips past the token and returns the next token after that.
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* ensureTokenTypeAndSkip(const Token* const pToken, const TokenType expectedType) noexcept {
    ensureTokenType(pToken, expectedType);
    return pToken + 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Skips past the opening of a block of values.
// Skips everything before the '{' and the '{' itself.
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* skipBlockOpening(const Token* const pStartToken) noexcept {
    const Token* pToken = pStartToken;

    while ((pToken->type != TokenType::Null) && (pToken->type != TokenType::OpenBlock)) {
        pToken++;
    }

    pToken = ensureTokenTypeAndSkip(pToken, TokenType::OpenBlock);
    return pToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: performs a parse action inside a value block until a closing '}' is encountered.
// Once parsing is complete the closing '}' is skipped and the next token after that returned.
// Note: the given parse function is expected to return the next token after it is done parsing.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ParseFuncT>
static const Token* parseUntilEndOfBlock(const Token* const pStartToken, const ParseFuncT& parseFunc) noexcept {
    const Token* pToken = pStartToken;

    while ((pToken->type != TokenType::Null) && (pToken->type != TokenType::CloseBlock)) {
        pToken = parseFunc(pToken);
    }

    pToken = ensureTokenTypeAndSkip(pToken, TokenType::CloseBlock);
    return pToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Skips past a block of tokens and returns the next token after the end of the block.
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* skipBlock(const Token* const pStartToken) noexcept {
    // Skip the opening of the block and skip all tokens within the block.
    // Note: also handle any nested blocks!
    return parseUntilEndOfBlock(
        skipBlockOpening(pStartToken),
        [](const Token* pToken) noexcept {
            if (pToken->type == TokenType::OpenBlock) {
                return skipBlock(pToken);
            } else {
                return pToken + 1;
            }
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Skips all tokens on the current line and returns the first token on the next line.
// Note: might potentially skip multiple lines if a block opens on the current line.
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* skipCurrentLineData(const Token* const pStartToken) noexcept {
    const Token* pToken = pStartToken;

    while ((pToken->type != TokenType::Null) && (pToken->begin.line == pStartToken->begin.line)) {
        if (pToken->type == TokenType::OpenBlock) {
            pToken = skipBlock(pToken);
        } else {
            pToken++;
        }
    }

    ASSERT((pToken->type == TokenType::Null) || (pToken->begin.line > pStartToken->begin.line));    // It should be greater!
    return pToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to parse assigning to a single number to a field and return the next token after parsing
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ValueT>
static const Token* parseSingleNumberAssign(const Token* const pStartToken, ValueT& outputValue) noexcept {
    // A value should always start with an identifier
    const Token* pToken = ensureTokenTypeAndSkip(pStartToken, TokenType::Identifier);

    // Consume any '=' sign
    if (pToken->type == TokenType::Equals) {
        pToken++;
    }

    // Is there a number following?
    if (pToken->type == TokenType::Number) {
        outputValue = static_cast<ValueT>(pToken->number);
        return pToken + 1;
    }

    // Is there a 'true' or 'false' value following? (1.0 or 0.0)
    if (pToken->type == TokenType::True) {
        outputValue = ValueT(1);
        return pToken + 1;
    }

    if (pToken->type == TokenType::False) {
        outputValue = ValueT(0);
        return pToken + 1;
    }

    // If there is no number or boolean following the identifier then interpret it as being a flag set to 'true' or '1.0'.
    // Example: 'EnableCast' or 'HideNextMapForFinale' tokens on their own without any value.
    outputValue = ValueT(1);
    return pToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to parse assigning to a single boolean value to a field and return the next token after parsing
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseSingleBoolAssign(const Token* const pStartToken, bool& outputValue) noexcept {
    // A value should always start with an identifier
    const Token* pToken = ensureTokenTypeAndSkip(pStartToken, TokenType::Identifier);

    // Consume any '=' sign
    if (pToken->type == TokenType::Equals) {
        pToken++;
    }

    // Is there a number following?
    if (pToken->type == TokenType::Number) {
        outputValue = (pToken->number > 0);
        return pToken + 1;
    }

    // Is there a 'true' or 'false' value following? (1.0 or 0.0)
    if (pToken->type == TokenType::True) {
        outputValue = true;
        return pToken + 1;
    }

    if (pToken->type == TokenType::False) {
        outputValue = false;
        return pToken + 1;
    }

    // If there is no number or boolean following the identifier then interpret it as being a flag set to 'true'.
    // Example: 'EnableCast' or 'HideNextMapForFinale' tokens on their own without any value.
    outputValue = true;
    return pToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to parse assigning to a single string to a field and return the next token after parsing
//------------------------------------------------------------------------------------------------------------------------------------------
template <class StringT>
static const Token* parseSingleStringAssign(const Token* const pStartToken, StringT& outputValue) noexcept {
    // A value should always start with an identifier
    const Token* pToken = ensureTokenTypeAndSkip(pStartToken, TokenType::Identifier);

    // Consume any '=' sign and save the string
    if (pToken->type == TokenType::Equals) {
        pToken++;
    }

    const std::string_view text = pToken->text();
    outputValue.assign(text.data(), (uint32_t) text.size());
    return pToken + 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to parse a lump name and optional palette for a graphic, followed by an optional x/y offset.
// Returns the next token after parsing.
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseGraphicLumpNamePalAndOffset(
    const Token* const pStartToken,
    String8& lumpName,
    uint8_t& pal,
    int16_t& offsetX,
    int16_t& offsetY
) noexcept {
    // A value should always start with an identifier
    const Token* pToken = ensureTokenTypeAndSkip(pStartToken, TokenType::Identifier);

    // Default these in case they are not present
    pal = MAINPAL;
    offsetX = 0;
    offsetY = 0;

    // Consume any '=' sign
    if (pToken->type == TokenType::Equals) {
        pToken++;
    }

    // Lump name
    ensureTokenType(pToken, TokenType::String);
    assignAsciiTextToSmallString(lumpName, pToken->text());
    pToken++;

    // Palette
    if (pToken->type != TokenType::Number)
        return pToken;

    pal = (uint8_t) pToken->number;
    pToken++;

    // Offset X
    if (pToken->type != TokenType::Number)
        return pToken;

    offsetX = (int16_t) pToken->number;
    pToken++;

    // Offset Y
    if (pToken->type != TokenType::Number)
        return pToken;

    offsetY = (int16_t) pToken->number;
    pToken++;
    return pToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Same as 'parseGraphicLumpNamePalAndOffset', except the offset is discarded
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseGraphicLumpNameAndPal(
    const Token* const pStartToken,
    String8& lumpName,
    uint8_t& pal
) noexcept {
    int16_t unusedOffsetX;
    int16_t unusedOffsetY;
    return parseGraphicLumpNamePalAndOffset(pStartToken, lumpName, pal, unusedOffsetX, unusedOffsetY);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a single episode definition and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseEpisode(const Token* const pStartToken, const bool bIsSecretEpisode) noexcept {
    const Token* pToken = pStartToken;

    // Episode number is auto-determined
    Episode& episode = gEpisodes.emplace_back();
    episode.episodeNum = (int32_t) gEpisodes.size();

    // If it's a secret episode then don't show it on the main menu
    episode.bIsHidden = bIsSecretEpisode;

    // Parse episode name
    assignAsciiTextToSmallString(episode.name, pToken->text());
    pToken++;

    // Episode start map (only for regular episodes)
    if (!bIsSecretEpisode) {
        ensureTokenType(pToken, TokenType::Number);
        episode.startMap = (int32_t) pToken->number;

        if ((episode.startMap < 1) || (episode.startMap > 255)) {
            error(pToken->begin, "Episode: 'StartMap' must be specified and be between 1 and 255!");
        }

        pToken++;
    }

    // Parse 'PicLogo'
    if (pToken->type == TokenType::String) {
        assignAsciiTextToSmallString(episode.logoPic, pToken->text());
        pToken++;
    } 
 
    // Parse 'PalLogo'
    if (pToken->type == TokenType::Number) {
        episode.logoPal = (int16_t) pToken->number;

        if (episode.logoPal > 31) {
            error(pToken->begin, "Episode: 'PalLogo' must from 0-31 for a valid palette, or < 0 to use the default 'DOOM' logo palette!");
        }

        pToken++;
    }

    // Parse 'LogoX'
    if (pToken->type == TokenType::Number) {
        episode.logoX = (int16_t) pToken->number;
        pToken++;
    }
    
    // Parse 'LogoY'
    if (pToken->type == TokenType::Number) {
        episode.logoYOffset = (int16_t)(pToken->number - 20);   // A y value of '20' is the natural position for these logos: interpret anything other than that as an offset...
        pToken++;
    }

    return pToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses an 'Episodes' block and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseEpisodesBlock(const Token* const pStartToken) noexcept {
    return parseUntilEndOfBlock(
        skipBlockOpening(pStartToken),
        [](const Token* pToken) noexcept {
            // Episode definition?
            if (pToken->type == TokenType::String)
                return parseEpisode(pToken, false);

            // Parse the secret episode definition if there:
            if ((pToken->type == TokenType::Identifier) && (pToken->text() == "PicSecret"))
                return parseEpisode(pToken, true);

            // Unhandled or unwanted line of data - skip it!
            return skipCurrentLineData(pToken);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a 'Title' block and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseTitleBlock(const Token* const pStartToken) noexcept {
    // Parse the title screen type
    const Token* pToken = ensureTokenTypeAndSkip(pStartToken, TokenType::Identifier);
    ensureTokenType(pToken, TokenType::Number);
    gGameInfo.titleScreenStyle = (TitleScreenStyle) pToken->number;

    if ((uint32_t) gGameInfo.titleScreenStyle >= NUM_TITLE_SCREEN_STYLES) {
        error(pToken->begin, "GameInfo: 'TitleScreenStyle' must be between 0 and %u!", NUM_TITLE_SCREEN_STYLES - 1u);
    }

    pToken++;

    // Parse the values inside the block
    return parseUntilEndOfBlock(
        skipBlockOpening(pToken),
        [](const Token* pToken) noexcept {
            // Lets see if we recognize this data:
            if (pToken->type == TokenType::Identifier) {
                const std::string_view fieldId = pToken->text();
                
                if (fieldId == "PicTitle")
                    return parseGraphicLumpNameAndPal(pToken, gGameInfo.texLumpName_TITLE, gGameInfo.texPalette_TITLE);

                if (fieldId == "PicTitle2")
                    return parseGraphicLumpNameAndPal(pToken, gGameInfo.texLumpName_TITLE2, gGameInfo.texPalette_TITLE2);

                if (fieldId == "CdMusic") {
                    int32_t cdTrackIndex = {};
                    const Token* const pNextToken = parseSingleNumberAssign(pToken, cdTrackIndex);

                    if ((cdTrackIndex < 0) || (cdTrackIndex >= NUM_CD_MUSIC_TRACKS))
                        error(pToken->begin, "CdMusic: track index '%d' is out of range! Allowed range: 0 to %d", cdTrackIndex, NUM_CD_MUSIC_TRACKS - 1);

                    gGameInfo.titleScreenCdTrackOverride = (int8_t) gCDTrackNum[cdTrackIndex];
                    return pNextToken;
                }

                if (fieldId == "PalFire")
                    return parseSingleNumberAssign(pToken, gGameInfo.texPalette_titleScreenFire);
            }

            // Unhandled or unwanted line of data - skip it!
            return skipCurrentLineData(pToken);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a 'Credits' block and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseCreditsBlock(const Token* const pStartToken) noexcept {
    return parseUntilEndOfBlock(
        skipBlockOpening(pStartToken),
        [](const Token* pToken) noexcept {
            // Init basic credits page properties (hardcoded)
            CreditsPage& page = gCredits.emplace_back();
            page.maxScroll = 256;

            // Parse 'bgPic'
            ensureTokenType(pToken, TokenType::String);
            assignAsciiTextToSmallString(page.bgPic, pToken->text());
            pToken++;

            // Parse 'bgPal'
            ensureTokenType(pToken, TokenType::Number);
            page.bgPal = (uint8_t) pToken->number;
            pToken++;

            // Parse 'fgPic'
            ensureTokenType(pToken, TokenType::String);
            assignAsciiTextToSmallString(page.fgPic, pToken->text());
            pToken++;

            // Parse 'fgPal'
            ensureTokenType(pToken, TokenType::Number);
            page.fgPal = (uint8_t) pToken->number;
            pToken++;

            // Optional: 'fgXPos'
            if (pToken->type != TokenType::Number)
                return pToken;

            page.fgXPos = (int16_t) pToken->number;
            pToken++;

            // Optional: 'bFgAdditive'
            if ((pToken->type != TokenType::Number) && (pToken->type != TokenType::True) && (pToken->type != TokenType::False))
                return pToken;

            if (pToken->type == TokenType::Number) {
                page.bFgAdditive = (pToken->number > 0);
            } else if (pToken->type == TokenType::True) {
                page.bFgAdditive = true;
            } else {
                page.bFgAdditive = false;
            }

            pToken++;
            return pToken;
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a 'GameInfo' block and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseGameInfo(const Token* const pStartToken) noexcept {
    return parseUntilEndOfBlock(
        skipBlockOpening(pStartToken),
        [](const Token* pToken) noexcept {
            // Lets see if we recognize this data:
            if (pToken->type == TokenType::Identifier) {
                const std::string_view fieldId = pToken->text();

                if (fieldId == "NumMaps")
                    return parseSingleNumberAssign(pToken, gGameInfo.numMaps);
                
                if (fieldId == "NumRegularMaps")
                    return parseSingleNumberAssign(pToken, gGameInfo.numRegularMaps);

                if (fieldId == "Episodes")
                    return parseEpisodesBlock(pToken);

                if (fieldId == "Title")
                    return parseTitleBlock(pToken);

                if (fieldId == "PicStatus")
                    return parseGraphicLumpNameAndPal(pToken, gGameInfo.texLumpName_STATUS, gGameInfo.texPalette_STATUS);

                if (fieldId == "PicBack")
                    return parseGraphicLumpNameAndPal(pToken, gGameInfo.texLumpName_BACK, gGameInfo.texPalette_BACK);

                if (fieldId == "PicInter")
                    return parseGraphicLumpNameAndPal(pToken, gGameInfo.texLumpName_Inter_BACK, gGameInfo.texPalette_Inter_BACK);

                if (fieldId == "PicTitleLogo")
                    return parseGraphicLumpNamePalAndOffset(pToken, gTitleLogo.lumpName, gTitleLogo.paletteIdx, gTitleLogo.xPos, gTitleLogo.yPos);

                if (fieldId == "PalUI") {
                    uint8_t uiPalette = 0;
                    const Token* const pNextToken = parseSingleNumberAssign(pToken, uiPalette);

                    gGameInfo.texPalette_LOADING = uiPalette;
                    gGameInfo.texPalette_PAUSE = uiPalette;
                    gGameInfo.texPalette_NETERR = uiPalette;
                    gGameInfo.texPalette_CONNECT = uiPalette;

                    return pNextToken;
                }

                if (fieldId == "PicTile")
                    return parseGraphicLumpNameAndPal(pToken, gGameInfo.texLumpName_OptionsBG, gGameInfo.texPalette_OptionsBG);

                if (fieldId == "NumDemos") {
                    int32_t numDemos = 0;
                    const Token* const pNextToken = parseSingleNumberAssign(pToken, numDemos);
                    Game::gConstants.SetNumDemos_GecMe_Beta4OrLater(numDemos);
                    return pNextToken;
                }

                if (fieldId == "Credits")
                    return parseCreditsBlock(pToken);
            }

            // Unhandled or unwanted line of data - skip it!
            return skipCurrentLineData(pToken);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a single map definition and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseMap(const Token* const pStartToken) noexcept {
    Map& map = gMaps.emplace_back();

    // Parse the map number
    const Token* pToken = ensureTokenTypeAndSkip(pStartToken, TokenType::Identifier);
    ensureTokenType(pToken, TokenType::Number);
    map.mapNum = (int32_t) pToken->number;

    if ((map.mapNum < 1) || (map.mapNum > 255)) {
        error(pToken->begin, "Map: map number must be between 1 and 255!");
    }

    pToken++;

    // Parse the map name
    ensureTokenType(pToken, TokenType::String);
    assignAsciiTextToSmallString(map.name, pToken->text());
    pToken++;

    // Parse the data inside the map block
    return parseUntilEndOfBlock(
        skipBlockOpening(pToken),
        [&map](const Token* pToken) noexcept {
            // Lets see if we recognize this data:
            if (pToken->type == TokenType::Identifier) {
                const std::string_view fieldId = pToken->text();

                // Map music
                if (fieldId == "Music") {
                    const Token* const pNextToken = parseSingleNumberAssign(pToken, map.music);

                    if ((map.music < 0) || (map.music > 1024)) {
                        error(pToken->begin, "Map: 'Music' must be specified and be between 0 and 1024!");
                    }

                    return pNextToken;
                }

                // Cluster
                if (fieldId == "Cluster")
                    return parseSingleNumberAssign(pToken, map.cluster);

                // Play CD music?
                if (fieldId == "PlayCdMusic")
                    return parseSingleBoolAssign(pToken, map.bPlayCdMusic);

                // No intermission?
                if (fieldId == "NoIntermission")
                    return parseSingleBoolAssign(pToken, map.bNoIntermission);
                
                // Map reverb mode
                if (fieldId == "ReverbMode") {
                    const Token* const pNextToken = parseSingleNumberAssign(pToken, map.reverbMode);

                    if ((map.reverbMode < SpuReverbMode::SPU_REV_MODE_OFF) || (map.reverbMode > SpuReverbMode::SPU_REV_MODE_PIPE)) {
                        error(pToken->begin, "Map: 'ReverbMode' must be between 0 and 9!");
                    }

                    return pNextToken;
                }

                // Reverb depth
                if (fieldId == "ReverbDepth") {
                    int32_t reverbDepth = 0;
                    const Token* const pNextToken = parseSingleNumberAssign(pToken, reverbDepth);
                    reverbDepth = std::clamp(reverbDepth, (int32_t) INT16_MIN, (int32_t) INT16_MAX);
                    map.reverbDepthL = (int16_t) reverbDepth;
                    map.reverbDepthR = (int16_t) reverbDepth;
                    return pNextToken;
                }
            }

            // Unhandled or unwanted line of data - skip it!
            return skipCurrentLineData(pToken);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a single cluster definition and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseCluster(const Token* const pStartToken) noexcept {
    // Create a new Cluster and initialize with the defaults for the GEC ME
    Cluster& cluster = gClusters.emplace_back();
    cluster.castLcdFile = "CAST.LCD";
    cluster.bSkipFinale = false;

    // Parse the cluster number
    const Token* pToken = ensureTokenTypeAndSkip(pStartToken, TokenType::Identifier);
    ensureTokenType(pToken, TokenType::Number);
    cluster.clusterNum = (int32_t) pToken->number;

    if ((cluster.clusterNum < 1) || (cluster.clusterNum > 255)) {
        error(pToken->begin, "Cluster: cluster number must be between 1 and 255!");
    }

    pToken++;

    // Parse the data inside the cluster block
    return parseUntilEndOfBlock(
        skipBlockOpening(pToken),
        [&cluster](const Token* pToken) noexcept {
            // Lets see if we recognize this data:
            if (pToken->type == TokenType::Identifier) {
                const std::string_view fieldId = pToken->text();

                if (fieldId == "CastLcdFile")
                    return parseSingleStringAssign(pToken, cluster.castLcdFile);

                if (fieldId == "Pic")
                    return parseSingleStringAssign(pToken, cluster.pic);

                if (fieldId == "PicPal")
                    return parseSingleNumberAssign(pToken, cluster.picPal);

                if (fieldId == "CdMusicA")
                    return parseSingleNumberAssign(pToken, cluster.cdMusicA);

                if (fieldId == "CdMusicB")
                    return parseSingleNumberAssign(pToken, cluster.cdMusicB);

                if (fieldId == "TextX")
                    return parseSingleNumberAssign(pToken, cluster.textX);

                if (fieldId == "TextY")
                    return parseSingleNumberAssign(pToken, cluster.textY);

                if (fieldId == "SkipFinale")
                    return parseSingleBoolAssign(pToken, cluster.bSkipFinale);

                if (fieldId == "HideNextMapForFinale")
                    return parseSingleBoolAssign(pToken, cluster.bHideNextMapForFinale);

                if (fieldId == "EnableCast")
                    return parseSingleBoolAssign(pToken, cluster.bEnableCast);

                if (fieldId == "NoCenterText")
                    return parseSingleBoolAssign(pToken, cluster.bNoCenterText);

                if (fieldId == "SmallFont")
                    return parseSingleBoolAssign(pToken, cluster.bSmallFont);

                if (fieldId == "Text") {
                    // Skip the 'Text' identifier and any '=' sign
                    pToken++;

                    if (pToken->type == TokenType::Equals) {
                        pToken++;
                    }

                    // Parse all the lines of text
                    constexpr int32_t MAX_TEXT_LINES = C_ARRAY_SIZE(cluster.text);
                    int32_t lineIdx = 0;

                    while (pToken->type == TokenType::String) {
                        if (lineIdx < MAX_TEXT_LINES) {
                            const std::string_view text = pToken->text();
                            cluster.text[lineIdx].assign(text.data(), (uint32_t) text.size());
                            lineIdx++;
                        }

                        pToken++;

                        // Consume any ',' separator between this and the next text line
                        if (pToken->type == TokenType::NextValue) {
                            pToken++;
                        }
                    }

                    return pToken;
                }
            }

            // Unhandled or unwanted line of data - skip it!
            return skipCurrentLineData(pToken);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a single sky definition and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseSky(const Token* const pStartToken) noexcept {
    // Create a new sky and parse the lump name
    Sky& sky = gSkies.emplace_back();
    
    const Token* pToken = ensureTokenTypeAndSkip(pStartToken, TokenType::Identifier);
    ensureTokenType(pToken, TokenType::String);
    assignAsciiTextToSmallString(sky.lumpName, pToken->text());
    pToken++;

    // Remove the 'F_' prefix from the sky lump name (so it contains the real lump name)
    removeSkyLumpNameF_Prefix(sky.lumpName);

    // Parse the data inside the sky block
    return parseUntilEndOfBlock(
        skipBlockOpening(pToken),
        [&sky](const Token* pToken) noexcept {
            // Lets see if we recognize this data:
            if (pToken->type == TokenType::Identifier) {
                const std::string_view fieldId = pToken->text();

                if (fieldId == "Pal")
                    return parseSingleNumberAssign(pToken, sky.paletteIdx);

                if (fieldId == "Fire")
                    return parseSingleBoolAssign(pToken, sky.bIsFireSky);
            }

            // Unhandled or unwanted line of data - skip it!
            return skipCurrentLineData(pToken);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a single switch definition and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseSwitch(const Token* const pStartToken) noexcept {
    // Skip the 'SwTexPic' token
    const Token* pToken = ensureTokenTypeAndSkip(pStartToken, TokenType::Identifier);

    // Get the lump names for the switch definition
    ensureTokenType(pToken, TokenType::String);
    const std::string_view name1 = pToken->text();
    pToken++;

    ensureTokenType(pToken, TokenType::String);
    const std::string_view name2 = pToken->text();
    pToken++;

    // Save the lump names to a new 'switchlist_t' definition
    const size_t name1Len = std::min(name1.length(), (size_t)(C_ARRAY_SIZE(switchlist_t::name1) - 1));
    const size_t name2Len = std::min(name2.length(), (size_t)(C_ARRAY_SIZE(switchlist_t::name2) - 1));

    switchlist_t& sw = gBaseSwitchList.emplace_back();
    std::memcpy(sw.name1, name1.data(), name1Len);
    std::memcpy(sw.name2, name2.data(), name2Len);
    sw.name1[name1Len] = 0;
    sw.name2[name2Len] = 0;

    return pToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a single animated texture or flat definition and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseAnimDef(const Token* const pStartToken, const bool bIsTexture) noexcept {
    // Skip the 'FAnimPic' or 'TAnimPic' token
    const Token* pToken = ensureTokenTypeAndSkip(pStartToken, TokenType::Identifier);

    // Get the 'start' and 'end' lump names for the anim definition
    ensureTokenType(pToken, TokenType::String);
    const std::string_view startName = pToken->text();
    pToken++;

    ensureTokenType(pToken, TokenType::String);
    const std::string_view endName = pToken->text();
    pToken++;

    // Get the update 'tick mask' controlling how fast the animation is
    ensureTokenType(pToken, TokenType::Number);
    const uint32_t updateTickMask = (uint32_t) pToken->number;
    pToken++;

    // Save basic info for the new animation
    animdef_t& anim = gBaseAnimDefs.emplace_back();
    anim.istexture = bIsTexture;
    anim.ticmask = updateTickMask;

    // Save the start and end lump names for the new animation
    const size_t startNameLen = std::min(startName.length(), (size_t)(C_ARRAY_SIZE(animdef_t::startname) - 1));
    const size_t endNameLen = std::min(endName.length(), (size_t)(C_ARRAY_SIZE(animdef_t::endname) - 1));

    std::memcpy(anim.startname, startName.data(), startNameLen);
    std::memcpy(anim.endname, endName.data(), endNameLen);
    anim.startname[startNameLen] = 0;
    anim.endname[endNameLen] = 0;

    return pToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a GEC format MAPINFO file on disk from the specified path
//------------------------------------------------------------------------------------------------------------------------------------------
static void parseMapInfoFileOnDisk(const char* const filePath) noexcept {
    // Tokenize first
    gCurMapInfoFilePath = filePath;
    const MapInfoTokens mapInfoTokens = tokenizeMapInfoFileOnDisk(filePath);

    // Continue parsing blocks of values until the end of the token list is reached
    ASSERT(mapInfoTokens.tokens.size() > 0);
    const Token* pToken = mapInfoTokens.tokens.data();

    while (pToken->type != TokenType::Null) {
        // Start of value block or value line?
        if (pToken->type == TokenType::Identifier) {
            // What type of block or value line is it?
            const std::string_view blockId = pToken->text();

            if (blockId == "GameInfo") {
                pToken = parseGameInfo(pToken);
            } else if (blockId == "Map") {
                pToken = parseMap(pToken);
            } else if (blockId == "Cluster") {
                pToken = parseCluster(pToken);
            } else if (blockId == "Sky") {
                pToken = parseSky(pToken);
            } else if (blockId == "SwTexPic") {
                pToken = parseSwitch(pToken);
            } else if (blockId == "FAnimPic") {
                pToken = parseAnimDef(pToken, false);
            } else if (blockId == "TAnimPic") {
                pToken = parseAnimDef(pToken, true);
            } else {
                // Unhandled or unwanted line of data - skip it!
                pToken = skipCurrentLineData(pToken);
            }
        } else {
            // Unhandled or unwanted line of data - skip it!
            pToken = skipCurrentLineData(pToken);
        }
    }

    // Finish up
    gCurMapInfoFilePath = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if GEC format MAPINFO should be loaded and used 
//------------------------------------------------------------------------------------------------------------------------------------------
bool shouldUseGecMapInfo() noexcept {
    return (Game::gGameType == GameType::GEC_ME_Beta4);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads the GEC format MAPINFO
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    // Note: default to Final Doom settings if something isn't otherwise specified (with a few tweaks)
    initGameInfo_FinalDoom(gGameInfo);
    gGameInfo.texPalette_DOOM = 21;

    // Parse the MAPINFO and auto-generate the list of clusters
    parseMapInfoFileOnDisk("PSXDOOM/ABIN/PSXGMINF.TXT");
    parseMapInfoFileOnDisk("PSXDOOM/ABIN/PSXMPINF.TXT");
    parseMapInfoFileOnDisk("PSXDOOM/ABIN/PSXTXINF.TXT");

    // If there is a 'secret' episode then assign its start map as the 1st secret map
    for (Episode& episode : gEpisodes) {
        if (episode.bIsHidden) {
            episode.startMap = gGameInfo.numRegularMaps + 1;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Releases all of the GEC format MAPINFO
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gCurMapInfoFilePath = nullptr;
    gBaseAnimDefs.clear();
    gBaseSwitchList.clear();
    gCredits.clear();
    gSkies.clear();
    gMaps.clear();
    gClusters.clear();
    gEpisodes.clear();
    gGameInfo = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Basic access to various types of structs (for the purposes of copying)
//------------------------------------------------------------------------------------------------------------------------------------------
const GameInfo& getGameInfo() noexcept {
    return gGameInfo;
}

const MenuSprite& getTitleLogo() noexcept {
    return gTitleLogo;
}

const std::vector<Episode>& allEpisodes() noexcept {
    return gEpisodes;
}

const std::vector<Cluster>& allClusters() noexcept {
    return gClusters;
}

const std::vector<Map>& allMaps() noexcept {
    return gMaps;
}

const std::vector<Sky>& allSkies() noexcept {
    return gSkies;
}

const std::vector<CreditsPage>& allCreditPages() noexcept {
    return gCredits;
}

const std::vector<switchlist_t>& getBaseSwitchList() noexcept {
    return gBaseSwitchList;
}

const std::vector<animdef_t>& getBaseAnimDefs() noexcept {
    return gBaseAnimDefs;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find a 'Sky' structure by lump name or return all skies
//------------------------------------------------------------------------------------------------------------------------------------------
const Sky* getSky(const String8& lumpName) noexcept {
    // For the input name remove the special 'compressed' flag from first character of the lump name, if present:
    String8 lumpNameNoCompressionFlag = lumpName;
    lumpNameNoCompressionFlag.chars[0] &= 0x7Fu;

    // Try to find the sky by lump name
    for (const Sky& sky : gSkies) {
        if (sky.lumpName == lumpNameNoCompressionFlag)
            return &sky;
    }

    return nullptr;
}

END_NAMESPACE(GecMapInfo)
