#pragma once

#include <cstdint>
#include <string>

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper class for input text stream parsing.
// Maintains a position within a stream of text and provides utility functions for manipulating the stream.
//------------------------------------------------------------------------------------------------------------------------------------------
struct TextIStream {
    const char*     str;
    uint32_t        endOffset;
    uint32_t        curOffset;

    TextIStream(const char* const str, const uint32_t size) noexcept;
    TextIStream(const TextIStream& other) noexcept = default;
    TextIStream& operator = (const TextIStream& other) noexcept = default;

    char peekChar() const;
    char readChar();
    void skipChar();
    void skipChars(const uint32_t count);
    bool isAtEnd() const noexcept;
    void ensureAtEnd();
    bool skipAsciiWhiteSpace() noexcept;
    bool skipAsciiWhiteSpaceOnThisLine() noexcept;

    // Check if a certain string is ahead
    template <bool CaseSensitive = false>
    bool checkStringAhead(const char* const checkStr) const noexcept;

    // Consume a string ahead, throw an exception if not ahead
    template <bool CaseSensitive = false>
    void consumeStringAhead(const char* const consumeStr);

    // Similar to consume string ahead, except skips all white space preceeding the token and after it
    template <bool CaseSensitive = false>
    void consumeSpaceSeparatedTokenAhead(const char* const tokenStr);

    // Consume SOME, i.e 1 or more elements of whitespace ahead.
    // Throws an exception if there is no whitespace ahead.
    void consumeAsciiWhiteSpaceAhead();

    // Parse an unsigned decimal integer ahead.
    // Stops when a non digit character is reached.
    // Expects at least 1 valid digit character.
    uint32_t readDecimalUint();

    // Parse an unsigned hex integer ahead.
    // Stops when a non hex digit character is reached.
    // Expects at least 1 valid hex digit character.
    uint32_t readHexUint();

    // Read a single hex digit (nibble)
    uint32_t readHexDigit();

    // Create a sub text stream consisting of the text up until the end of the current line.
    // Moves this stream along past this line.
    TextIStream readNextLineAsStream();

    // Read a string delimited by the given delimiters.
    // Throws an exception if delimiters are missing.
    // Escaping is NOT supported.
    std::string readDelimitedString(const char begDelimiter, const char endDelimiter);
};

//------------------------------------------------------------------------------------------------------------------------------------------
// This gets thrown if certain stream operations fail
//------------------------------------------------------------------------------------------------------------------------------------------
struct TextStreamException {
    const char* msg;
    TextStreamException(const char* msg) noexcept : msg(msg) {}
};
