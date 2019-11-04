#include "TextStream.h"

#include <cstring>

static constexpr bool uppercaseAsciiChar(const char c) noexcept {
    if (c >= 'a' && c <= 'z') {
        return (char)(c - 32);
    } else {
        return c;
    }
}

static constexpr bool isDecimalDigit(const char c) noexcept {
    return (c >= '0' && c <= '9');
}

static constexpr bool isHexDigit(const char c) noexcept {
    const char cUpper = uppercaseAsciiChar(c);

    return (
        (cUpper >= '0' && c <= '9') ||
        (cUpper >= 'A' && cUpper <= 'F')
    );
}

static constexpr bool isWhiteSpaceAsciiChar(const char c) noexcept {
    return (
        (c == ' ') ||
        (c == '\n') ||
        (c == '\r') ||
        (c == '\t') ||
        (c == '\v') ||
        (c == '\f')
    );
}

static constexpr bool isNewLineAsciiChar(const char c) noexcept {
    return ((c == '\n') || (c == '\r'));
}

static constexpr bool asciiCharsMatchCaseInsensitive(const char c1, const char c2) noexcept {
    return (uppercaseAsciiChar(c1) == uppercaseAsciiChar(c2));
}

static constexpr uint32_t getDecimalDigitValue(const char c) {
    if (c >= '0' && c <= '9') {
        return (uint32_t)(c - '0');
    } else {
        throw TextStreamException("Invalid decimal digit!");
    }
}

static constexpr uint32_t getHexDigitValue(const char c) {
    const char cUpper = uppercaseAsciiChar(c);

    if (cUpper >= '0' && cUpper <= '9') {
        return (uint32_t)(cUpper - '0');
    } else if (cUpper >= 'A' && cUpper <= 'F') {
        return (uint32_t)(cUpper - 'A') + 10u;
    } else {
        throw TextStreamException("Invalid hex digit!");
    }
}

TextStream::TextStream(const char* const str, const uint32_t size) noexcept
    : str(str)
    , endOffset(size)
    , curOffset(0)
{        
}

char TextStream::peekChar() const {
    if (curOffset < endOffset) {
        return str[curOffset];
    } else {
        throw TextStreamException("Unexpected end of data!");
    }
}

char TextStream::readChar() {
    if (curOffset < endOffset) {
        return str[curOffset++];            
    } else {
        throw TextStreamException("Unexpected end of data!");
    }
}

void TextStream::skipChar() {
    if (curOffset < endOffset) {
        ++curOffset;
    } else {
        throw TextStreamException("Unexpected end of data!");
    }
}

void TextStream::skipChars(const uint32_t count) {
    if (curOffset + count <= endOffset) {
        curOffset += count;
    } else {
        throw TextStreamException("Unexpected end of data!");
    }
}

bool TextStream::isAtEnd() const noexcept {
    return (curOffset >= endOffset);
}

void TextStream::ensureAtEnd() {
    if (!isAtEnd()) {
        throw TextStreamException("Expected to be at the end of the text stream!");
    }
}

bool TextStream::skipAsciiWhiteSpace() noexcept {
    bool bDidSkipChars = false;

    while (!isAtEnd()) {
        if (isWhiteSpaceAsciiChar(peekChar())) {
            skipChar();
            bDidSkipChars = true;
        } else {
            break;
        }
    }

    return bDidSkipChars;
}

bool TextStream::skipAsciiWhiteSpaceOnThisLine() noexcept {
    bool bDidSkipChars = false;

    while (!isAtEnd()) {
        const char c = peekChar();

        if (isWhiteSpaceAsciiChar(c) && (!isNewLineAsciiChar(c))) {
            skipChar();
            bDidSkipChars = true;
        } else {
            break;
        }
    }

    return bDidSkipChars;
}

template <bool CaseSensitive>
bool TextStream::checkStringAhead(const char* const checkStr) const noexcept {
    const char* pCurCheckStrChar = checkStr;
    uint32_t peekOffset = curOffset;

    while (true) {
        const char c1 = *pCurCheckStrChar;

        if (!c1) {  // If we reached the end of the check string without failing then there is a match!
            break;
        }

        if (curOffset >= endOffset) {   // If we reached the end of the text stream before the end of the check string then no match!
            return false;
        }

        const char c2 = str[peekOffset];

        if constexpr (CaseSensitive) {
            if (c1 != c2) {
                return false;
            }
        } else {
            if (!asciiCharsMatchCaseInsensitive(c1, c2)) {
                return false;
            }
        }

        ++pCurCheckStrChar;
        ++peekOffset;
    }

    return true;
}

template bool TextStream::checkStringAhead<false>(const char* const checkStr) const noexcept;
template bool TextStream::checkStringAhead<true>(const char* const checkStr) const noexcept;

template <bool CaseSensitive>
void TextStream::consumeStringAhead(const char* const consumeStr) {
    if (!checkStringAhead(consumeStr)) {
        throw TextStreamException("Unexpected string ahead!");
    }
    
    curOffset += (uint32_t) std::strlen(consumeStr);
}

template void TextStream::consumeStringAhead<false>(const char* const consumeStr);
template void TextStream::consumeStringAhead<true>(const char* const consumeStr);

template <bool CaseSensitive>
void TextStream::consumeSpaceSeparatedTokenAhead(const char* const tokenStr) {
    skipAsciiWhiteSpace();
    consumeStringAhead(tokenStr);
    skipAsciiWhiteSpace();
}

template void TextStream::consumeSpaceSeparatedTokenAhead<false>(const char* const tokenStr);
template void TextStream::consumeSpaceSeparatedTokenAhead<true>(const char* const tokenStr);

void TextStream::consumeAsciiWhiteSpaceAhead() {
    if (!isWhiteSpaceAsciiChar(readChar())) {
        throw TextStreamException("Expected some white space ahead!");
    }

    while (!isAtEnd()) {
        if (!isWhiteSpaceAsciiChar(peekChar())) {
            break;
        }
        
        readChar();
    }
}

uint32_t TextStream::readDecimalUint() {
    uint32_t result = getDecimalDigitValue(readChar());

    while (!isAtEnd()) {
        if (!isDecimalDigit(peekChar())) {
            break;
        }
        
        result *= 10;
        result += getDecimalDigitValue(readChar());
    }

    return result;
}

uint32_t TextStream::readHexUint() {
    uint32_t result = getHexDigitValue(readChar());

    while (!isAtEnd()) {
        if (!isHexDigit(peekChar())) {
            break;
        }
        
        result *= 10;
        result += readHexDigit();
    }

    return result;
}

uint32_t TextStream::readHexDigit() {
    return getHexDigitValue(readChar());
}

TextStream TextStream::readNextLineAsStream() {
    if (isAtEnd()) {
        throw TextStreamException("Unexpected end of data!");
    }
    
    const uint32_t startOffset = curOffset;

    while (!isNewLineAsciiChar(str[curOffset])) {
        ++curOffset;
    }

    ++curOffset;
    return TextStream(str + startOffset, curOffset - startOffset);
}

std::string TextStream::readDelimitedString(const char begDelimiter, const char endDelimiter) {
    if (peekChar() != begDelimiter) {
        throw TextStreamException("Expected a start of string delimiter!");
    }

    skipChar();
    std::string output;

    while (peekChar() != endDelimiter) {
        output.push_back(readChar());
    }

    skipChar();
    return output;
}
