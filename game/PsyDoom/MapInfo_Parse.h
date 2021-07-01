#pragma once

#include "FatalErrors.h"
#include "Macros.h"

#include <cstdint>
#include <cstdio>
#include <vector>

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Describes a location in the MAPINFO text
//------------------------------------------------------------------------------------------------------------------------------------------
struct TextLoc {
    uint32_t        line;       // ZERO based line number
    uint32_t        column;     // ZERO based column number
    const char*     pStr;       // Pointer to the actual text data at this location
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells what type a token is
//------------------------------------------------------------------------------------------------------------------------------------------
enum class TokenType : uint32_t {
    Null,           // Null token type (returned when there are no more tokens in the text stream)
    Identifier,     // An unquoted identifier like 'Map' or 'NoIntermission'
    String,         // A quoted string like "Hello"
    Number,         // A number of some sort, specified as an integer, hex value or float
    Equals,         // A '=' character
    OpenBlock,      // A '{' character
    CloseBlock,     // A '}' character
    NextValue,      // A ',' character (used for assinging multiple values to an identifier)
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a single token extracted by the MAPINFO parser
//------------------------------------------------------------------------------------------------------------------------------------------
struct Token {
    TextLoc     begin;      // Begining of the token
    TextLoc     end;        // End of the token (one character past the end)
    TokenType   type;       // What type of token this is
    float       number;     // The token's value as a number (for convenience, '0' if it's not a number)

    // Returns the number of characters in the token
    int32_t size() const noexcept {
        return (int32_t)(end.pStr - begin.pStr);
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a token linked together with other tokens to make for easier parsing and traversal of the MAPINFO
//------------------------------------------------------------------------------------------------------------------------------------------
struct LinkedToken {
    // The token being linked with other tokens
    Token token;

    // Has one of two meanings:
    //  (1) If the token is a block header token, points to the next block header token (if any).
    //      Block header tokens are all tokens past the initial block identifier (e.g 'map') and before the '{'.
    //  (2) If the token is a value within the block, points to the value identifier/name token.
    //      The data tokens for the value can be retrieved via 'pNextData'.
    LinkedToken* pNext;

    // For a value within a block this will point to the token containing the value data.
    // If the value is an array of multiple values then each array entry will link to the next via this field.
    LinkedToken* pNextData;

    // Returns how many tokens are ahead by following 'pNext'
    int32_t numTokensAhead() const noexcept {
        int32_t count = 0;
        
        for (LinkedToken* pToken = pNext; pToken; pToken = pToken->pNext) {
            count++;
        }

        return count;
    }

    // Returns how many tokens are ahead by following 'pNextData'
    int32_t numDataTokensAhead() const noexcept {
        int32_t count = 0;
        
        for (LinkedToken* pToken = pNextData; pToken; pToken = pToken->pNextData) {
            count++;
        }

        return count;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a block of values in the MAPINFO
//------------------------------------------------------------------------------------------------------------------------------------------
struct Block {
    LinkedToken*    pType;      // A single token containing the type/identifier for the block
    LinkedToken*    pHeader;    // A linked list of header tokens (these come after the block name) or 'nullptr' if none
    LinkedToken*    pValues;    // A linked list of values within the block or 'nullptr' if none
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains the result of parsing MAPINFO, all of the tokens plus the blocks referencing them
//------------------------------------------------------------------------------------------------------------------------------------------
struct MapInfo {
    std::vector<LinkedToken>    tokens;
    std::vector<Block>          blocks;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Issues a fatal MAPINFO error at the specified text location
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...FmtStrArgs>
static void error(const TextLoc loc, const char* const errorFmtStr, FmtStrArgs... fmtStrArgs) noexcept {
    char locStr[128];
    char msgStr[256];
    std::snprintf(locStr, C_ARRAY_SIZE(locStr), "Error parsing MAPINFO at line %u column %u!", loc.line + 1u, loc.column + 1u);
    std::snprintf(msgStr, C_ARRAY_SIZE(msgStr), errorFmtStr, fmtStrArgs...);
    FatalErrors::raiseF("%s\n%s", locStr, msgStr);
}

MapInfo parseMapInfo(const char* const mapInfoStr) noexcept;

END_NAMESPACE(MapInfo)
