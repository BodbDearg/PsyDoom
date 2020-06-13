//------------------------------------------------------------------------------------------------------------------------------------------
// Utilities for parsing and writing ini files. 
//
// Notes:
//  (1) The parser is NOT fully UTF-8 aware (does NOT handle special Unicode whitespace etc.).
//      This is perfectly okay however for what I am using it for.
//  (2) The parser will automatically trim whitespace at the beginning and end of keys/values
//      unlike a lot of other ini parsers. This allows for instance spaces in the keys etc.
//  (3) 'true' and 'false' in any case are accepted for boolean values, but the code prefers to
//      write '0' and '1' when outputting since numbers are much more locale neutral.
//  (4) Comments must be on their own dedicated lines and can start with either ';' or '#'.
//      Comments on the same lines as key/value pairs or section headers are NOT supported.
//  (5) Newlines in keys or values are NOT supported and cannot be escaped - single line mode only!
//      Newlines in output keys/values will be skipped.
//  (6) The following are valid escape sequences ONLY:
//          \;
//          \#
//          \[
//          \]
//          \=
//------------------------------------------------------------------------------------------------------------------------------------------

#include "IniUtils.h"

BEGIN_NAMESPACE(IniUtils)

struct ParserState {
    const char*     pCurChar;
    const char*     pEndChar;
    Entry           curEntry;
    EntryHandler    entryHandler;
};

static inline bool isSpace(const char c) noexcept {
    return (
        (c == ' ') ||
        (c == '\t')
    );
}

static inline bool isNewline(const char c) noexcept {
    return (
        (c == '\n') ||
        (c == '\r') ||
        (c == '\f') ||  // Form feed
        (c == '\v')     // Vertical tab
    );
}

static inline bool isSpaceOrNewline(const char c) noexcept {
    return (isSpace(c) || isNewline(c));
}

static inline bool isStartOfComment(const char c) noexcept {
    return (
        (c == ';') ||
        (c == '#')
    );
}

static bool isEscapedCharAhead(const char* const pStrStart, const char* const pStrEnd) noexcept {
    if (pStrStart + 1 < pStrEnd) {
        if (pStrStart[0] == '\\') {
            const char c = pStrStart[1];
            return (
                (c == ';') ||
                (c == '#') ||
                (c == '[') ||
                (c == ']') ||
                (c == '=')
            );
        }
    }

    return false;
}

static void skipSpacesOrNewlines(ParserState& state) noexcept {
    while (state.pCurChar < state.pEndChar) {
        if (isSpaceOrNewline(state.pCurChar[0])) {
            ++state.pCurChar;
        } else {
            break;
        }
    }
}

static void skipRestOfLine(ParserState& state) noexcept {
    while (state.pCurChar < state.pEndChar) {
        if (!isNewline(state.pCurChar[0])) {
            ++state.pCurChar;
        } else {
            break;
        }
    }
}

static void trimUnescapeAndAssignStr(const char* pStart, const char* pEnd, std::string& output) noexcept {
    // Left trim
    while (pStart < pEnd) {
        if (isSpaceOrNewline(pStart[0])) {
            ++pStart;
        } else {
            break;
        }
    }

    // Right trim
    while (pEnd > pStart) {
        if (isSpaceOrNewline(pEnd[-1])) {
            --pEnd;
        } else {
            break;
        }
    }

    // Run through the string and handle escaped chars
    output.clear();

    while (pStart < pEnd) {
        if (isEscapedCharAhead(pStart, pEnd)) {
            ++pStart;
        }

        output.push_back(pStart[0]);
        ++pStart;
    }
}

static void parseSectionHeader(ParserState& state) noexcept {
    // Consume the expected opening '[' or if that fails then just skip the line
    if (state.pCurChar >= state.pEndChar)
        return;
    
    if (state.pCurChar[0] != '[') {
        skipRestOfLine(state);
        return;
    }
    
    ++state.pCurChar;

    // Parse the section name
    const char* const pStartChar = state.pCurChar;

    while (state.pCurChar < state.pEndChar) {
        const char nextChar = state.pCurChar[0];

        if (nextChar == ']') {
            // Found the end of the section header; trim, unescape and save the name and then skip whatever else is on the line:
            const char* const pEndChar = state.pCurChar;
            trimUnescapeAndAssignStr(pStartChar, pEndChar, state.curEntry.section);
            skipRestOfLine(state);
        }
        else if (isEscapedCharAhead(state.pCurChar, state.pEndChar)) {
            // Escaped character such as '\;' ahead!!
            // Consume 2 characters:
            state.pCurChar += 2;
        }
        else if (isNewline(nextChar)) {
            // Reached the end of the line without a terminating ']'!
            break;
        }
        else {
            // Regular character that is part of the section name
            ++state.pCurChar;
        }
    }
}

static void parseKeyValuePair(ParserState& state) noexcept {
    // Find where the key name ends
    const char* const pKeyStartChar = state.pCurChar;

    while (state.pCurChar < state.pEndChar) {
        const char nextChar = state.pCurChar[0];

        if (nextChar == '=') {
            // Found the end of the key; trim, unescape and save the name:
            const char* const pKeyEndChar = state.pCurChar;
            trimUnescapeAndAssignStr(pKeyStartChar, pKeyEndChar, state.curEntry.key);

            // If the key is of zero length then skip the rest of the line and abort.
            // A key name CAN'T be empty:
            if (state.curEntry.key.empty()) {
                skipRestOfLine(state);
                return;
            }

            // Skip the '=' and break out of the loop, we are done
            ++state.pCurChar;
            break;
        }
        else if (isEscapedCharAhead(state.pCurChar, state.pEndChar)) {
            // Escaped character such as '\;' ahead!!
            // Consume 2 characters:
            state.pCurChar += 2;
        }
        else if (isNewline(nextChar)) {
            // Reached the end of the line before finding '='!
            // This is an invalid key/value pair:
            return;
        }
        else {
            // Regular character that is part of the key name
            ++state.pCurChar;
        }
    }

    // Find where the value ends (the next newline or content end)
    const char* const pValueStartChar = state.pCurChar;

    while (state.pCurChar < state.pEndChar) {
        const char nextChar = state.pCurChar[0];

        if (isNewline(nextChar)) {
            break;
        } else {
            ++state.pCurChar;
        }
    }

    // Now at the end of the value. Trim, unescape and save the value:
    const char* const pValueEndChar = state.pCurChar;
    trimUnescapeAndAssignStr(pValueStartChar, pValueEndChar, state.curEntry.value);

    // Send the value to the entry handler
    state.entryHandler(state.curEntry);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Parse an INI from a string in memory.
// Does its best to try and recover from errors on each line
//------------------------------------------------------------------------------------------------------------------------------------------
void parseIniFromString(const char* const pStr, const size_t len, const EntryHandler entryHandler) noexcept {
    ASSERT(entryHandler);

    ParserState state;
    state.pCurChar = pStr;
    state.pEndChar = pStr + len;
    state.entryHandler = entryHandler;

    state.curEntry.section.reserve(64);
    state.curEntry.key.reserve(64);
    state.curEntry.value.reserve(128);

    while (state.pCurChar < state.pEndChar) {
        const char nextChar = state.pCurChar[0];

        if (isSpaceOrNewline(nextChar)) {
            skipSpacesOrNewlines(state);
        }
        else if (nextChar == '[') {
            parseSectionHeader(state);
        }
        else if (isStartOfComment(nextChar)) {
            // Skip the entire line if it is a comment line
            skipRestOfLine(state);
        }
        else {
            parseKeyValuePair(state);
        }
    }
}

END_NAMESPACE(IniUtils)
