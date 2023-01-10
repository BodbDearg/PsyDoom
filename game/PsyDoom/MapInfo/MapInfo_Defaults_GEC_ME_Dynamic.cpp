//------------------------------------------------------------------------------------------------------------------------------------------
// Dynamically loaded MAPINFO defaults for 'GEC Master Edition' (Beta 4 and later).
// These defaults are loaded from MAPINFO style text files, so could change depending if the disk image is altered.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapInfo_Defaults_GEC_ME_Dynamic.h"

#include "Asserts.h"
#include "MapInfo.h"
#include "MapInfo_Defaults_FinalDoom.h"
#include "MapInfo_Parse.h"
#include "PsyDoom/PsxVm.h"
#include "PsyDoom/Utils.h"
#include "PsyQ/LIBSPU.h"

#include <algorithm>
#include <memory>

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds a tokenized MAPINFO file and it's backing text storage
//------------------------------------------------------------------------------------------------------------------------------------------
struct MapInfoTokens {
    std::string         rawText;
    std::vector<Token>  tokens;     // Note: all text pointers reference the data in 'rawText'
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Holds the defaults loaded from GEC format MAPINFO files
//------------------------------------------------------------------------------------------------------------------------------------------
struct DynamicMapInfoDefaults {
    GameInfo                gameInfo;
    std::vector<Episode>    episodes;
    std::vector<Cluster>    clusters;
    std::vector<Map>        maps;
};

static std::unique_ptr<DynamicMapInfoDefaults> gpDefaultMapInfo;

//------------------------------------------------------------------------------------------------------------------------------------------
// Context: which MAPINFO file is currently being parsed
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* gCurMapInfoFilePath = nullptr;

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
// Assign text to the specified 'SmallString', filtering out non ASCII characters like UTF-8 byte order marks (0xEF, 0xBB, 0xBF).
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

    // Null terminate if we didn't reach the end
    if (dstIdx < SmallStringT::MAX_LEN) {
        dst.chars[dstIdx] = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a single episode definition and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseEpisode(const Token* const pStartToken) noexcept {
    ASSERT(gpDefaultMapInfo);
    const Token* pToken = pStartToken;

    // Episode number is auto-determined
    Episode& episode = gpDefaultMapInfo->episodes.emplace_back();
    episode.episodeNum = (int32_t) gpDefaultMapInfo->episodes.size();

    // Parse episode name
    assignAsciiTextToSmallString(episode.name, pToken->text());
    pToken++;

    // Episode start map
    ensureTokenType(pToken, TokenType::Number);
    episode.startMap = (int32_t) pToken->number;

    if ((episode.startMap < 1) || (episode.startMap > 255)) {
        error(pToken->begin, "Episode: 'StartMap' must be specified and be between 1 and 255!");
    }

    pToken++;

    // TODO: GEC ME BETA 4: parse 'PicLogo'
    if (pToken->type == TokenType::String) {
        pToken++;
    } 
 
    // TODO: GEC ME BETA 4: parse 'PalLogo'
    if (pToken->type == TokenType::Number) {
        pToken++;
    }

    // TODO: GEC ME BETA 4: parse 'LogoX'
    if (pToken->type == TokenType::Number) {
        pToken++;
    }
    
    // TODO: GEC ME BETA 4: parse 'LogoY'
    if (pToken->type == TokenType::Number) {
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
                return parseEpisode(pToken);

            // TODO: GEC ME BETA 4: parse 'PicSecret'?

            // Unhandled or unwanted line of data - skip it!
            return skipCurrentLineData(pToken);
        }
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a 'GameInfo' block and returns the next token after that
//------------------------------------------------------------------------------------------------------------------------------------------
static const Token* parseGameInfo(const Token* const pStartToken) noexcept {
    ASSERT(gpDefaultMapInfo);

    return parseUntilEndOfBlock(
        skipBlockOpening(pStartToken),
        [](const Token* pToken) noexcept {
            GameInfo& gameInfo = gpDefaultMapInfo->gameInfo;

            // Lets see if we recognize this data:
            if (pToken->type == TokenType::Identifier) {
                const std::string_view fieldId = pToken->text();

                if (fieldId == "NumMaps")
                    return parseSingleNumberAssign(pToken, gameInfo.numMaps);
                
                if (fieldId == "NumRegularMaps")
                    return parseSingleNumberAssign(pToken, gameInfo.numRegularMaps);

                if (fieldId == "Episodes")
                    return parseEpisodesBlock(pToken);

                // TODO: GEC ME BETA 4: parse 'Title'
                // TODO: GEC ME BETA 4: parse 'Credits'
                // TODO: GEC ME BETA 4: parse 'NumDemos'
                // TODO: GEC ME BETA 4: parse 'PicTile'
                // TODO: GEC ME BETA 4: parse 'PicStatus'
                // TODO: GEC ME BETA 4: parse 'PicBack'
                // TODO: GEC ME BETA 4: parse 'PicInter'
                // TODO: GEC ME BETA 4: parse 'PicTitleLogo'
                // TODO: GEC ME BETA 4: parse 'PalUI'
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
    ASSERT(gpDefaultMapInfo);
    Map& map = gpDefaultMapInfo->maps.emplace_back();

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

                // TODO: GEC ME BETA 4: parse 'NoIntermission'
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
    ASSERT(gpDefaultMapInfo);

    // Create a new Cluster and initialize with the defaults for the GEC ME
    Cluster& cluster = gpDefaultMapInfo->clusters.emplace_back();
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
        // Expect start of value block
        ensureTokenType(pToken, TokenType::Identifier);

        // What type of block is it?
        const std::string_view blockId = pToken->text();

        if (blockId == "GameInfo") {
            pToken = parseGameInfo(pToken);
        } else if (blockId == "Map") {
            pToken = parseMap(pToken);
        } else if (blockId == "Cluster") {
            pToken = parseCluster(pToken);
        } else {
            // Unhandled or unwanted block of data - skip it!
            pToken = skipBlock(pToken);
        }
    }

    // Finish up
    gCurMapInfoFilePath = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Loads all dynamic MapInfo defaults for the GEC Master Edition
//------------------------------------------------------------------------------------------------------------------------------------------
void loadDefaults_GEC_ME_Dynamic() noexcept {
    // Note: default to Final Doom settings if something isn't otherwise specified
    gpDefaultMapInfo = std::make_unique<DynamicMapInfoDefaults>();
    initGameInfo_FinalDoom(gpDefaultMapInfo->gameInfo);

    // Parse the MAPINFO and auto-generate the list of clusters
    parseMapInfoFileOnDisk("PSXDOOM/ABIN/PSXGMINF.TXT");
    parseMapInfoFileOnDisk("PSXDOOM/ABIN/PSXMPINF.TXT");
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Releases all of the dynamic MapInfo defaults dynamically loaded by this module
//------------------------------------------------------------------------------------------------------------------------------------------
void freeDefaults_GEC_ME_Dynamic() noexcept {
    gpDefaultMapInfo.reset();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the defaults for various types of structs.
// Note: the caller is expected to have called 'loadDefaults_GEC_ME_Dynamic()' first!
//------------------------------------------------------------------------------------------------------------------------------------------
void initGameInfo_GEC_ME_Dynamic(GameInfo& gameInfo) noexcept {
    ASSERT(gpDefaultMapInfo);
    gameInfo = gpDefaultMapInfo->gameInfo;
}

void addEpisodes_GEC_ME_Dynamic(std::vector<Episode>& episodes) noexcept {
    ASSERT(gpDefaultMapInfo);
    episodes = gpDefaultMapInfo->episodes;
}

void addClusters_GEC_ME_Dynamic(std::vector<Cluster>& clusters) noexcept {
    ASSERT(gpDefaultMapInfo);
    clusters = gpDefaultMapInfo->clusters;
}

void addMaps_GEC_ME_Dynamic(std::vector<Map>& maps) noexcept {
    ASSERT(gpDefaultMapInfo);
    maps = gpDefaultMapInfo->maps;
}

END_NAMESPACE(MapInfo)
