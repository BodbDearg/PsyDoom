#pragma once

#include <array>
#include <cstdint>
#include <type_traits>

//------------------------------------------------------------------------------------------------------------------------------------------
// Base generic type for small string classes.
// Used to help eliminate redundancy between each small string type.
//------------------------------------------------------------------------------------------------------------------------------------------
template <typename WordT, uint32_t NumWords> 
struct TSmallString {
    // Maximum length of this string (in characters)
    static constexpr uint32_t MAX_LEN = sizeof(WordT) * NumWords;

    // The string expressed as words or characters
    union {
        WordT   words[NumWords];
        char    chars[MAX_LEN];
    };

    // Default empty string constructors.
    // One default intializes via the 'words' union member, the other via the 'chars' union member.
    inline constexpr TSmallString() noexcept : words{} {}
    inline constexpr TSmallString(std::nullptr_t) noexcept : chars{} {}

    // Default copy and assignment
    constexpr TSmallString(const TSmallString& other) = default;
    constexpr TSmallString& operator = (const TSmallString& other) = default;

    // Assigning a C-String
    inline constexpr TSmallString(const char* const str) noexcept : chars{} {
        assign(str, MAX_LEN);
    }

    inline constexpr TSmallString(const char* const str, const uint32_t maxCount) noexcept : chars{} {
        assign(str, maxCount);
    }

    inline constexpr void assign(const char* const str, const uint32_t maxCount = MAX_LEN) noexcept {
        // Copy the entire string until the maximum allowed size is reached or until the NUL character is found
        const uint32_t safeMaxCount = (maxCount > MAX_LEN) ? MAX_LEN : maxCount;

        for (uint32_t i = 0; i < safeMaxCount; ++i) {
            const char otherChar = str[i];
            chars[i] = otherChar;

            if (!otherChar)
                break;
        }

        // If we didn't terminate on a a NUL or didn't reach the capacity limit then this is required to cap off the string
        if (maxCount < MAX_LEN) {
            chars[maxCount] = 0;
        }
    }

    inline constexpr TSmallString& operator = (const char* const str) noexcept {
        assign(str);
        return *this;
    }

    // Comparison of strings
    inline constexpr WordT compare(const TSmallString& other) const noexcept {
        for (uint32_t i = 0; i < NumWords; ++i) {
            const WordT diff = words[i] - other.words[i];

            if (diff != 0)
                return diff;
        }

        return WordT(0);
    }

    inline constexpr bool operator <  (const TSmallString& other) const noexcept { return (compare(other) <  0); }
    inline constexpr bool operator <= (const TSmallString& other) const noexcept { return (compare(other) <= 0); }
    inline constexpr bool operator >  (const TSmallString& other) const noexcept { return (compare(other) >  0); }
    inline constexpr bool operator >= (const TSmallString& other) const noexcept { return (compare(other) >= 0); }
    inline constexpr bool operator == (const TSmallString& other) const noexcept { return (compare(other) == 0); }
    inline constexpr bool operator != (const TSmallString& other) const noexcept { return (compare(other) != 0); }

    // Get the length of this string
    inline constexpr uint32_t length() const noexcept {
        for (uint32_t i = 0; i < MAX_LEN; ++i) {
            if (!chars[i])
                return i;
        }

        return MAX_LEN;
    }

    // Get a null terminated copy of this string (c-string)
    inline constexpr std::array<char, MAX_LEN + 1> c_str() const noexcept {
        std::array<char, MAX_LEN + 1> strArray = {};

        for (uint32_t i = 0; i < MAX_LEN; ++i) {
            strArray[i] = chars[i];
        }

        return strArray;
    }

    // Give back the single word for the string.
    // Only allowed for single word strings, query is ambiguous otherwise.
    inline constexpr WordT word() const noexcept {
        static_assert(NumWords == 1, "Should only be called for single word string types!");
        return words[0];
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains a fixed capacity string of up to 32 characters (4 64-bit words)
//------------------------------------------------------------------------------------------------------------------------------------------
struct String32 : public TSmallString<uint64_t, 4> {
    // Inherit all constructors
    using TSmallString::TSmallString;

    // Various constructors (constexpr, so conversions can also happen at compile time)
    inline constexpr String32(
        const uint64_t word1,
        const uint64_t word2,
        const uint64_t word3,
        const uint64_t word4
    ) noexcept : TSmallString() {
        words[0] = word1;
        words[1] = word2;
        words[2] = word3;
        words[3] = word4;
    }

    inline constexpr String32(
        const char c0,      const char c1  = 0, const char c2  = 0, const char c3  = 0,
        const char c4  = 0, const char c5  = 0, const char c6  = 0, const char c7  = 0,
        const char c8  = 0, const char c9  = 0, const char c10 = 0, const char c11 = 0,
        const char c12 = 0, const char c13 = 0, const char c14 = 0, const char c15 = 0,
        const char c16 = 0, const char c17 = 0, const char c18 = 0, const char c19 = 0,
        const char c20 = 0, const char c21 = 0, const char c22 = 0, const char c23 = 0,
        const char c24 = 0, const char c25 = 0, const char c26 = 0, const char c27 = 0,
        const char c28 = 0, const char c29 = 0, const char c30 = 0, const char c31 = 0
    ) noexcept : TSmallString(nullptr) {
        chars[0 ] = c0;   chars[1 ] = c1;   chars[2 ] = c2;   chars[3 ] = c3;
        chars[4 ] = c4;   chars[5 ] = c5;   chars[6 ] = c6;   chars[7 ] = c7;
        chars[8 ] = c8;   chars[9 ] = c9;   chars[10] = c10;  chars[11] = c11;
        chars[12] = c12;  chars[13] = c13;  chars[14] = c14;  chars[15] = c15;
        chars[16] = c16;  chars[17] = c17;  chars[18] = c18;  chars[19] = c19;
        chars[20] = c20;  chars[21] = c21;  chars[22] = c22;  chars[23] = c23;
        chars[24] = c24;  chars[25] = c25;  chars[26] = c26;  chars[27] = c27;
        chars[28] = c28;  chars[29] = c29;  chars[30] = c30;  chars[31] = c31;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains a fixed capacity string of up to 16 characters (2 64-bit words)
//------------------------------------------------------------------------------------------------------------------------------------------
struct String16 : public TSmallString<uint64_t, 2> {
    // Inherit all constructors
    using TSmallString::TSmallString;

    // Various constructors (constexpr, so conversions can also happen at compile time)
    inline constexpr String16(const uint64_t word1, const uint64_t word2) noexcept : TSmallString() {
        words[0] = word1;
        words[1] = word2;
    }

    inline constexpr String16(
        const char c0,      const char c1  = 0, const char c2  = 0, const char c3  = 0,
        const char c4  = 0, const char c5  = 0, const char c6  = 0, const char c7  = 0,
        const char c8  = 0, const char c9  = 0, const char c10 = 0, const char c11 = 0,
        const char c12 = 0, const char c13 = 0, const char c14 = 0, const char c15 = 0
    ) noexcept : TSmallString(nullptr) {
        chars[0 ] = c0;   chars[1 ] = c1;   chars[2 ] = c2;   chars[3 ] = c3;
        chars[4 ] = c4;   chars[5 ] = c5;   chars[6 ] = c6;   chars[7 ] = c7;
        chars[8 ] = c8;   chars[9 ] = c9;   chars[10] = c10;  chars[11] = c11;
        chars[12] = c12;  chars[13] = c13;  chars[14] = c14;  chars[15] = c15;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains a fixed capacity string of up to 8 characters (1 64-bit word)
//------------------------------------------------------------------------------------------------------------------------------------------
struct String8 : public TSmallString<uint64_t, 1> {
    // Inherit all constructors
    using TSmallString::TSmallString;

    // Various constructors (constexpr, so conversions can also happen at compile time)
    inline constexpr String8(const uint64_t word) noexcept : TSmallString() {
         words[0] = word;
    }

    inline constexpr String8(
        const char c0,     const char c1 = 0, const char c2 = 0, const char c3 = 0,
        const char c4 = 0, const char c5 = 0, const char c6 = 0, const char c7 = 0
    ) noexcept : TSmallString(nullptr) {
        chars[0] = c0;  chars[1] = c1;  chars[2] = c2;  chars[3] = c3;
        chars[4] = c4;  chars[5] = c5;  chars[6] = c6;  chars[7] = c7;
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains a fixed capacity string of up to 4 characters (1 32-bit word)
//------------------------------------------------------------------------------------------------------------------------------------------
struct String4 : public TSmallString<uint32_t, 1> {
    // Inherit all constructors
    using TSmallString::TSmallString;

    // Various constructors (constexpr, so conversions can also happen at compile time)
    inline constexpr String4(const uint32_t word) noexcept : TSmallString() {
        words[0] = word;
    }

    inline constexpr String4(const char c0, const char c1 = 0, const char c2 = 0, const char c3 = 0) noexcept : TSmallString(nullptr) {
        chars[0] = c0;
        chars[1] = c1;
        chars[2] = c2;
        chars[3] = c3;
    }
};
