//------------------------------------------------------------------------------------------------------------------------------------------
// Handles the low-level details of parsing the MAPINFO lump and putting it into a format where it can be digested by higher level
// value block handlers. Also defines parsing related data structures and some utility functions.
// 
// For more on the new ZDoom MAPINFO format which PsyDoom uses, see:
//      https://zdoom.org/wiki/MAPINFO
//      https://doomwiki.org/wiki/MAPINFO#ZDoom_.28new.29
//------------------------------------------------------------------------------------------------------------------------------------------
#include "MapInfo_Parse.h"

#include "Asserts.h"

#include <cstdlib>
#include <vector>

BEGIN_NAMESPACE(MapInfo)

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a character is an ASCII letter
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isAsciiAlphaChar(const char c) noexcept {
    return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a character is digit
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isDigitChar(const char c) noexcept {
    return ((c >= '0') && (c <= '9'));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the specified character is a valid identifier start character.
// Note: only ASCII identifier names are allowed.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isValidIdentifierStartChar(const char c) noexcept {
    return (isAsciiAlphaChar(c) || (c == '_'));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the specified character is a valid identifier character past the start.
// Note: only ASCII identifier names are allowed.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isValidIdentifierNonStartChar(const char c) noexcept {
    return (isValidIdentifierStartChar(c) || isDigitChar(c));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a token type is valid for a block header
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isValidBlockHeaderTokenType(const TokenType type) noexcept {
    return (
        (type == TokenType::Identifier) ||
        (type == TokenType::String) ||
        (type == TokenType::Number) ||
        (type == TokenType::True) ||
        (type == TokenType::False)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a token type is valid for value data
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isValidDataTokenType(const TokenType type) noexcept {
    return (
        (type == TokenType::Identifier) ||
        (type == TokenType::String) ||
        (type == TokenType::Number) ||
        (type == TokenType::True) ||
        (type == TokenType::False)
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Skips past a single character at the specified text location.
// Note: more than one physical character may be consumed, if 2 character newlines are encountered for example.
//------------------------------------------------------------------------------------------------------------------------------------------
static TextLoc nextChar(TextLoc loc) noexcept {
    switch (loc.pStr[0]) {
        // Skip a horizontal tab (assume 4 character tab width)
        case '\t':
            loc.pStr++;
            loc.column = ((loc.column + 4) / 4) * 4;
            break;

        // Either a classic Mac newline (\r) or Windows newline (\r\n)
        case '\r':
            loc.pStr += (loc.pStr[1] == '\n') ? 2 : 1;
            loc.line++;
            loc.column = 0;
            break;

        // Unix style newline (\n), vertical tab or form feed
        case '\n':
        case '\v':
        case '\f':
            loc.pStr++;
            loc.line++;
            loc.column = 0;
            break;

        // Skip a single space, or some other character
        default:
            loc.pStr++;
            loc.column++;
            break;
    }

    return loc;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Skips the rest of the current text line
//------------------------------------------------------------------------------------------------------------------------------------------
static TextLoc skipCurrentLine(TextLoc loc) noexcept {
    while (const char c = loc.pStr[0]) {
        // Did we hit a newline?
        const bool bReachedLineEnd = (
            (c == '\r') ||
            (c == '\n') ||
            (c == '\v') ||
            (c == '\f')
        );

        // Move onto the next character and finish skipping if we reached a newline
        loc = nextChar(loc);

        if (bReachedLineEnd)
            break;
    }

    return loc;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Skips past all whitespace and comments starting at the specified text location
//------------------------------------------------------------------------------------------------------------------------------------------
static TextLoc skipWhitespaceAndComments(TextLoc loc) noexcept {
    while (true) {
        switch (loc.pStr[0]) {
            // Whitespace character? If so then skip:
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case '\v':
            case '\f':
                loc = nextChar(loc);
                break;

            // If there is a comment ahead skip past the entire line:
            case '/': {
                if (loc.pStr[1] == '/') {
                    loc = skipCurrentLine(loc);
                }
            }   break;

            // Not a whitespace character or the start of a comment?
            // If so then we are done skipping...
            default:
                return loc;
        }
    }

    return loc;     // Note: should never reach here
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a string token starting at the specified location, which is assumed to be a double quote character
//------------------------------------------------------------------------------------------------------------------------------------------
static Token parseStringToken(const TextLoc startLoc) noexcept {
    ASSERT(startLoc.pStr[0] == '\"');
    TextLoc endLoc = nextChar(startLoc);

    while (true) {
        const char peekChar = endLoc.pStr[0];

        if (peekChar != 0) {
            endLoc = nextChar(endLoc);

            if (peekChar == '\"') {
                break;
            }
        } else {
            error(endLoc, "Expected an end of string delimiter!");
        }
    }

    Token token = {};
    token.begin = startLoc;
    token.end = endLoc;
    token.type = TokenType::String;
    return token;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a single character token of the specified type
//------------------------------------------------------------------------------------------------------------------------------------------
static Token parseSingleCharToken(const TextLoc startLoc, const TokenType type) noexcept {
    Token token = {};
    token.begin = startLoc;
    token.end = nextChar(startLoc);
    token.type = type;
    return token;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a number token starting at the specified location
//------------------------------------------------------------------------------------------------------------------------------------------
static Token parseNumber(const TextLoc startLoc) noexcept {
    char* pNumEnd = const_cast<char*>(startLoc.pStr);
    const float numValue = std::strtof(pNumEnd, &pNumEnd);

    if (pNumEnd == startLoc.pStr) {
        error(startLoc, "Expected a valid number!");
    }

    Token token = {};
    token.begin = startLoc;
    token.end.line = token.begin.line;
    token.end.column = token.begin.column + (uint32_t)(pNumEnd - startLoc.pStr);
    token.end.pStr = pNumEnd;
    token.type = TokenType::Number;
    token.number = numValue;
    return token;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Peeks to see if boolean 'true' is ahead in the stream (case insensitive)
//------------------------------------------------------------------------------------------------------------------------------------------
static bool peekBooleanTrue(const TextLoc startLoc) noexcept {
    return (
        (std::toupper(startLoc.pStr[0]) == 'T') &&
        (std::toupper(startLoc.pStr[1]) == 'R') &&
        (std::toupper(startLoc.pStr[2]) == 'U') &&
        (std::toupper(startLoc.pStr[3]) == 'E')
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Peeks to see if boolean 'false' is ahead in the stream (case insensitive)
//------------------------------------------------------------------------------------------------------------------------------------------
static bool peekBooleanFalse(const TextLoc startLoc) noexcept {
    return (
        (std::toupper(startLoc.pStr[0]) == 'F') &&
        (std::toupper(startLoc.pStr[1]) == 'A') &&
        (std::toupper(startLoc.pStr[2]) == 'L') &&
        (std::toupper(startLoc.pStr[3]) == 'S') &&
        (std::toupper(startLoc.pStr[4]) == 'E')
    );
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses boolean 'true' starting at the specified location
//------------------------------------------------------------------------------------------------------------------------------------------
static Token parseBooleanTrue(const TextLoc startLoc) noexcept {
    ASSERT(peekBooleanTrue(startLoc));

    Token token = {};
    token.begin = startLoc;
    token.end = startLoc;
    token.end.column += 4;
    token.end.pStr += 4;
    token.type = TokenType::True;
    return token;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses boolean 'false' starting at the specified location
//------------------------------------------------------------------------------------------------------------------------------------------
static Token parseBooleanFalse(const TextLoc startLoc) noexcept {
    ASSERT(peekBooleanFalse(startLoc));

    Token token = {};
    token.begin = startLoc;
    token.end = startLoc;
    token.end.column += 5;
    token.end.pStr += 5;
    token.type = TokenType::False;
    return token;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses an identifier starting at the specified location
//------------------------------------------------------------------------------------------------------------------------------------------
static Token parseIdentifier(const TextLoc startLoc) noexcept {
    ASSERT(isValidIdentifierStartChar(startLoc.pStr[0]));
    TextLoc endLoc = nextChar(startLoc);

    while (true) {
        const char peekChar = endLoc.pStr[0];

        if (isValidIdentifierNonStartChar(peekChar)) {
            endLoc = nextChar(endLoc);
        } else {
            break;
        }
    }

    Token token = {};
    token.begin = startLoc;
    token.end = endLoc;
    token.type = TokenType::Identifier;
    return token;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Extracts the next MAPINFO token from a text stream, starting at the given location
//------------------------------------------------------------------------------------------------------------------------------------------
static Token nextToken(TextLoc startLoc) noexcept {
    // Skip whitespace and comments, then peek and parse what's ahead:
    startLoc = skipWhitespaceAndComments(startLoc);
    const char charAhead = startLoc.pStr[0];

    switch (charAhead) {
        // Reached the end of the text stream (null character ahead).
        case 0:
            break;

        case '\"':  return parseStringToken(startLoc);
        case '=':   return parseSingleCharToken(startLoc, TokenType::Equals);
        case '{':   return parseSingleCharToken(startLoc, TokenType::OpenBlock);
        case '}':   return parseSingleCharToken(startLoc, TokenType::CloseBlock);
        case ',':   return parseSingleCharToken(startLoc, TokenType::NextValue);

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '+':
        case '-':
        case '.':
            return parseNumber(startLoc);

        // Possibly an identifier, or 'true' or 'false' boolean literals
        default: {
            // Boolean literals 'true' or 'false' ahead?
            if (peekBooleanTrue(startLoc))
                return parseBooleanTrue(startLoc);

            if (peekBooleanFalse(startLoc))
                return parseBooleanFalse(startLoc);

            // Only valid possibility left is an identifier: we need to make sure it starts with a valid character
            if (isValidIdentifierStartChar(charAhead)) {
                return parseIdentifier(startLoc);
            } else {
                error(startLoc, "Not a valid token!");
            }
        }   break;
    }

    // Returning the null token for when the end of the text stream is reached.
    // Note that the default initializer for 'Token' creates a 'Null' type token.
    Token token = {};
    token.begin = startLoc;
    token.end = startLoc;
    return token;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: gets the next token after the given token
//------------------------------------------------------------------------------------------------------------------------------------------
static Token nextTokenAfter(const Token& afterToken) noexcept {
    return nextToken(afterToken.end);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tokenizes the specified MAPINFO string into a series of 'LinkedToken' structs.
// The returned list always has a null token at the end to delimit.
//------------------------------------------------------------------------------------------------------------------------------------------
static std::vector<LinkedToken> tokenizeMapInfoToLinkedTokens(const char* const mapInfoStr) noexcept {
    std::vector<LinkedToken> tokens;
    tokens.reserve(2048);

    Token curToken = nextToken(TextLoc{ 0, 0, mapInfoStr });

    while (curToken.type != TokenType::Null) {
        tokens.push_back(LinkedToken{ curToken });
        curToken = nextTokenAfter(curToken);
    }

    tokens.push_back(LinkedToken{ curToken });      // Add the null token at the end
    return tokens;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Potentially parses the data for a value and returns the next token to visit after parsing.
// Looks for a '=' (equals) symbol and consumes the value list if found.
//------------------------------------------------------------------------------------------------------------------------------------------
static LinkedToken* parseValueData(LinkedToken* pCurToken, LinkedToken& valueToken) noexcept {
    // If there is no assign then there is no data for the value (flag value)
    if (pCurToken->token.type != TokenType::Equals)
        return pCurToken;

    // Consume the '=' token
    ++pCurToken;

    // Start the value list
    if (!isValidDataTokenType(pCurToken->token.type)) {
        error(pCurToken->token.begin, "Expected a number, string or identifier for the value's data!");
    }

    valueToken.pNextData = pCurToken;
    LinkedToken* pPrevData = pCurToken;
    ++pCurToken;

    // Continue parsing values in the list while there are more
    while (pCurToken->token.type == TokenType::NextValue) {
        // Consume the ',' token
        ++pCurToken;

        // Ensure there is a valid piece of data following and link it into the list
        if (!isValidDataTokenType(pCurToken->token.type)) {
            error(pCurToken->token.begin, "Expected a number, string or identifier for the value's data!");
        }

        pPrevData->pNextData = pCurToken;
        pPrevData = pCurToken;
        ++pCurToken;
    }

    return pCurToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parses a block of MAPINFO values and returns the next token to visit after parsing
//------------------------------------------------------------------------------------------------------------------------------------------
static LinkedToken* parseBlock(LinkedToken* pCurToken, Block& block) noexcept {
    // Parse the block identifier token, which must be an identifier
    ASSERT(pCurToken);

    if (pCurToken->token.type != TokenType::Identifier) {
        error(pCurToken->token.begin, "Expected a identifier/type for a value block!");
    }

    block.pType = pCurToken;
    pCurToken++;

    // Parse all block header values
    if (isValidBlockHeaderTokenType(pCurToken->token.type)) {
        // Start the list of header tokens
        block.pHeader = pCurToken;
        LinkedToken* pPrevHeaderToken = pCurToken;
        ++pCurToken;

        // Link all of the header tokens together
        while (isValidBlockHeaderTokenType(pCurToken->token.type)) {
            pPrevHeaderToken->pNext = pCurToken;
            pPrevHeaderToken = pCurToken;
            ++pCurToken;
        }
    }

    // Ensure there is a block opener and skip past it
    if (pCurToken->token.type != TokenType::OpenBlock) {
        error(pCurToken->token.begin, "Expected a '{' to open the value block!");
    }

    ++pCurToken;

    // Parse all values in the block
    if (pCurToken->token.type == TokenType::Identifier) {
        // Start the list of values and parse the data for the first value
        block.pValues = pCurToken;
        LinkedToken* pPrevValueToken = pCurToken;
        ++pCurToken;

        pCurToken = parseValueData(pCurToken, *pPrevValueToken);

        // Link all of the values together and parse their data
        while (pCurToken->token.type == TokenType::Identifier) {
            pPrevValueToken->pNext = pCurToken;
            pPrevValueToken = pCurToken;
            ++pCurToken;

            pCurToken = parseValueData(pCurToken, *pPrevValueToken);
        }
    }

    // Ensure there is a block closer and skip past it
    if (pCurToken->token.type != TokenType::CloseBlock) {
        error(pCurToken->token.begin, "Expected a '}' to close the value block!");
    }

    ++pCurToken;
    return pCurToken;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Performs a simple tokenize of a MAPINFO style string, returning a list of tokens with no links between them.
// Useful for implementing alternative methods of MAPINFO parsing elsewhere.
//------------------------------------------------------------------------------------------------------------------------------------------
std::vector<Token> tokenizeMapInfo(const char* const mapInfoStr) noexcept {
    std::vector<Token> tokens;
    tokens.reserve(4096);

    Token curToken = nextToken(TextLoc{ 0, 0, mapInfoStr });

    while (curToken.type != TokenType::Null) {
        tokens.push_back(curToken);
        curToken = nextTokenAfter(curToken);
    }

    tokens.push_back(curToken);     // Add the null token at the end
    return tokens;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tokenizes and parses the specified MAPINFO into a series of value blocks
//------------------------------------------------------------------------------------------------------------------------------------------
MapInfo parseMapInfo(const char* const mapInfoStr) noexcept {
    // Tokenize the string first, this will always produce a list terminated by the null token
    MapInfo mapInfo;
    mapInfo.tokens = tokenizeMapInfoToLinkedTokens(mapInfoStr);
    ASSERT(mapInfo.tokens.size() > 0);
    
    // Parse each value block and return the result
    mapInfo.blocks.reserve(256);
    LinkedToken* pCurToken = mapInfo.tokens.data();

    while (pCurToken->token.type != TokenType::Null) {
        Block& block = mapInfo.blocks.emplace_back();
        pCurToken = parseBlock(pCurToken, block);
    }

    return mapInfo;
}

END_NAMESPACE(MapInfo)
