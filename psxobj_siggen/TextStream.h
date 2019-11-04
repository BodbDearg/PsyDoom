#pragma once

#include <cstdint>

//----------------------------------------------------------------------------------------------------------------------
// Helper class for text parsing.
// Maintains a position within a stream of text and provides utility functions for manipulating the stream.
//----------------------------------------------------------------------------------------------------------------------
struct TextStream {
    const char*     str;
    uint32_t        endOffset;
    uint32_t        curOffset;

    TextStream(const char* const str, const uint32_t size) noexcept;
    TextStream(const TextStream& other) noexcept = default;
    TextStream& operator = (const TextStream& other) noexcept = default;

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

    // Create a sub text stream consisting of the text up until the end of the current line.
    // Moves this stream along past this line.
    TextStream readNextLineAsStream();
};

//----------------------------------------------------------------------------------------------------------------------
// This gets thrown if certain stream operations fail
//----------------------------------------------------------------------------------------------------------------------
struct TextStreamException {
    const char* msg;
    TextStreamException(const char* msg) noexcept : msg(msg) {}
};
