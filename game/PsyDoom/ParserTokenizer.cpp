#include "ParserTokenizer.h"

#include "Asserts.h"

BEGIN_NAMESPACE(ParserTokenizer)

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a character is a horizontal space character
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isHSpace(const char c) noexcept {
    return ((c == ' ') || (c == '\t'));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a character is a vertical space (newline) character
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isVSpace(const char c) noexcept {
    return ((c == '\n') || (c == '\r') || (c == '\f') || (c == '\v'));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the character is a horizontal or vertical space
//------------------------------------------------------------------------------------------------------------------------------------------
static bool isSpace(const char c) noexcept {
    return (isHSpace(c) || isVSpace(c));
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Skip characters matching a condition in a string
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* skipChars(const char* const pStr, const char* const pStrEnd, bool (* const Cond)(const char c) noexcept) noexcept {
    const char* pStrCur = pStr;

    while (pStrCur < pStrEnd) {
        const char c = *pStrCur;

        if (Cond(c)) {
            ++pStrCur;
        } else {
            break;
        }
    }

    return pStrCur;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Find characters matching a condition
//------------------------------------------------------------------------------------------------------------------------------------------
static const char* findChars(const char* const pStr, const char* const pStrEnd, bool (* const Cond)(const char c) noexcept) noexcept {
    const char* pStrCur = pStr;

    while (pStrCur < pStrEnd) {
        const char c = *pStrCur;

        if (Cond(c)) {
            break;
        } else {
            ++pStrCur;
        }
    }

    return pStrCur;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the beginning and end pointer for the next line in the string.
// Skips whitespace and newlines at the start of the given string and finds the next newline to mark the end, and trims the string end.
// Returns 'true' if there is another line, 'false' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
bool getNextLine(const char* const pStr, const char* const pStrEnd, const char*& pBeg, const char*& pEnd) noexcept {
    // Find the line boundaries, trimmed at the start and delimited by a new line at the end
    pBeg = skipChars(pStr, pStrEnd, isSpace);
    pEnd = findChars(pBeg, pStrEnd, isVSpace);

    // End whitespace trim
    while ((pEnd > pBeg) && isHSpace(*pEnd)) {
        --pEnd;
    }

    return (pBeg < pEnd);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the next token in a string.
// Skips whitespace at the start of the given string and finds the next whitespace to mark the end of the token.
// Returns 'true' if there is another token, 'false' otherwise.
//------------------------------------------------------------------------------------------------------------------------------------------
bool getNextToken(const char* const pStr, const char* const pStrEnd, const char*& pBeg, const char*& pEnd) noexcept {
    pBeg = skipChars(pStr, pStrEnd, isHSpace);
    pEnd = findChars(pBeg, pStrEnd, isHSpace);
    return (pBeg < pEnd);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper that visits all lines in a text block and all tokens on a line, using the newline and space delimiters defined by this module.
// This does a lot of the heavy lifting towards parsing a block of text into game specific data.
//------------------------------------------------------------------------------------------------------------------------------------------
void visitAllLineTokens(
    const char* const pStrBeg,
    const char* const pStrEnd,
    const VisitLineFunc& onNewLine,
    const VisitTokenFunc& onToken
) noexcept {
    // Preconditions
    ASSERT(onNewLine);
    ASSERT(onToken);

    // Parse each line of the string
    int32_t lineIdx = 0;
    const char* pLineBeg = {};
    const char* pLineEnd = pStrBeg;

    while (true) {
        // Move onto the line after the end of the previous one
        if (!ParserTokenizer::getNextLine(pLineEnd, pStrEnd, pLineBeg, pLineEnd))
            break;

        onNewLine(lineIdx);

        // Process all tokens in this line
        int32_t tokenIdx = 0;
        const char* pTokenBeg = {};
        const char* pTokenEnd = pLineBeg;

        while (true) {
            if (!ParserTokenizer::getNextToken(pTokenEnd, pLineEnd, pTokenBeg, pTokenEnd))
                break;

            onToken(tokenIdx, pTokenBeg, (size_t)(pTokenEnd - pTokenBeg));
            ++tokenIdx;
        }

        ++lineIdx;
    }
}

END_NAMESPACE(ParserTokenizer)
