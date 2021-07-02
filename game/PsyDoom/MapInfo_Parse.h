#pragma once

#include "FatalErrors.h"
#include "Macros.h"

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <string_view>
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
// Issues a fatal MAPINFO error at the specified text location
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...FmtStrArgs>
[[noreturn]] inline void error(const TextLoc loc, const char* const errorFmtStr, FmtStrArgs... fmtStrArgs) noexcept {
    char locStr[128];
    char msgStr[256];
    std::snprintf(locStr, C_ARRAY_SIZE(locStr), "Error parsing MAPINFO at line %u column %u!", loc.line + 1u, loc.column + 1u);
    std::snprintf(msgStr, C_ARRAY_SIZE(msgStr), errorFmtStr, fmtStrArgs...);
    FatalErrors::raiseF("%s\n%s", locStr, msgStr);
}

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

    // Returns the begining and end of the textual data for the token. For most token types this just returns the entire range of
    // characters that makeup the token but for 'String' the pointers will be adjusted to ignore the double quotes.
    const char* textBegin() const noexcept {
        return (type == TokenType::String) ? begin.pStr + 1 : begin.pStr;
    }

    const char* textEnd() const noexcept {
        return (type == TokenType::String) ? end.pStr - 1 : end.pStr;
    }

    // Tells if the token text matches the specified string.
    // The comparison is case insensitive.
    bool textEqualsIgnoreCase(const char* pOther) const noexcept {
        const char* pThis = textBegin();
        const char* const pThisEnd = textEnd();

        while (pThis < pThisEnd) {
            const char c1 = (char) std::toupper(*pThis);
            const char c2 = (char) std::toupper(*pOther);
            ++pThis;
            ++pOther;

            if (c1 != c2) {
                return false;
            } else if (c1 == 0) {
                break;
            }
        }

        return true;
    }

    // Returns a string view for the token text
    std::string_view text() const noexcept {
        const char* const pBeg = textBegin();
        const char* const pEnd = textEnd();
        return (pBeg < pEnd) ? std::string_view(pBeg, (size_t)(pEnd - pBeg)) : std::string_view(pBeg, 0);
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

    // Figures out how many header tokens there are
    int32_t getHeaderTokenCount() const noexcept {
        return (pHeader) ? 1 + pHeader->numTokensAhead() : 0;
    }

    // Gets the header token with the specified index.
    // Returns 'nullptr' if the index is invalid.
    const LinkedToken* getHeaderTokenWithIndex(const uint32_t index) const noexcept {
        uint32_t i = 0;

        for (const LinkedToken* pCurToken = pHeader; pCurToken; pCurToken = pCurToken->pNext, ++i) {
            if (i == index)
                return pCurToken;
        }

        return nullptr;
    }

    // Ensure the list of header tokens has the exact amount of tokens specified.
    // Issues a fatal error if this is not the case.
    void ensureHeaderTokenCount(const int32_t count) const noexcept {
        if (getHeaderTokenCount() != count) {
            error(pType->token.end, "MAPINFO block has an invalid header! See PsyDoom's MAPINFO docs for the expected format.");
        }
    }

    // Gets a mandatory header token which must be of the specified type.
    // Issues a fatal error if not found or matching.
    const LinkedToken& getRequiredHeaderToken(const int32_t index, const TokenType type) const noexcept {
        const LinkedToken* pToken = getHeaderTokenWithIndex(index);

        if ((!pToken) || (pToken->token.type != type)) {
            error(pType->token.end, "MAPINFO block has an invalid header! See PsyDoom's MAPINFO docs for the expected format.");
        }

        return *pToken;
    }

    // Helper: gets a mandatory header number and issues a fatal error if not existing
    float getRequiredHeaderNumber(const int32_t index) const noexcept {
        return getRequiredHeaderToken(index, TokenType::Number).token.number;
    }

    // Helper: gets a mandatory header string and issues a fatal error if not existing
    std::string_view getRequiredHeaderString(const int32_t index) const noexcept {
        return getRequiredHeaderToken(index, TokenType::String).token.text();
    }

    // Gets a value (of any type) with the specified name; name comparison rules are case insensitive.
    // Returns 'nullptr' if not found.
    const LinkedToken* getValue(const char* const name) const noexcept {
        for (const LinkedToken* pCurToken = pValues; pCurToken; pCurToken = pCurToken->pNext) {
            if (pCurToken->token.textEqualsIgnoreCase(name)) {
                return pCurToken;
            }
        }

        return nullptr;
    }

    // Gets a single number value with the specified name; name comparison rules are case insensitive.
    // Returns a default value if not found or if the wrong type.
    // Note: if the value is a list then all entries except the 1st are ignored.
    float getSingleNumberValue(const char* const name, const float defaultValue) const noexcept {
        const LinkedToken* pToken = getValue(name);
        const LinkedToken* pDataToken = (pToken) ? pToken->pNextData : nullptr;
        return (pDataToken && (pDataToken->token.type == TokenType::Number)) ? pDataToken->token.number : defaultValue;
    }

    // Gets a single string value with the specified name; name comparison rules are case insensitive.
    // Returns a default value if not found or if the wrong type.
    // Note: if the value is a list then all entries except the 1st are ignored.
    std::string_view getSingleStringValue(const char* const name, const std::string_view defaultValue) const noexcept {
        const LinkedToken* pToken = getValue(name);
        const LinkedToken* pDataToken = (pToken) ? pToken->pNextData : nullptr;
        return (pDataToken) ? pDataToken->token.text() : defaultValue;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains the result of parsing MAPINFO, all of the tokens plus the blocks referencing them
//------------------------------------------------------------------------------------------------------------------------------------------
struct MapInfo {
    std::vector<LinkedToken>    tokens;
    std::vector<Block>          blocks;
};

MapInfo parseMapInfo(const char* const mapInfoStr) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// An overload of 'error' that issues an error at the start of the specified block
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ...FmtStrArgs>
[[noreturn]] inline void error(const Block& block, const char* const errorFmtStr, FmtStrArgs... fmtStrArgs) noexcept {
    error(block.pType->token.begin, errorFmtStr, fmtStrArgs...);
}

END_NAMESPACE(MapInfo)
