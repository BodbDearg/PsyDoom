#pragma once

#include <array>
#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains a fixed capacity string of up to 16 characters (2 64-bit words)
//------------------------------------------------------------------------------------------------------------------------------------------
union String16 {
    // Maximum length of this string (in characters)
    static constexpr uint32_t MAX_LEN = 16;

    // The string expressed as 2 64-bit words or 16 characters
    uint64_t words[2];
    char     chars[16];

    // Various constructors (constexpr, so conversions can also happen at compile time)
    inline constexpr String16() noexcept : words{} {}

    inline constexpr String16(const uint64_t word1, const uint64_t word2) noexcept : words{} {
        words[0] = word1;
        words[1] = word2;
    }

    inline constexpr String16(
        const char c0,      const char c1  = 0, const char c2  = 0, const char c3  = 0,
        const char c4  = 0, const char c5  = 0, const char c6  = 0, const char c7  = 0,
        const char c8  = 0, const char c9  = 0, const char c10 = 0, const char c11 = 0,
        const char c12 = 0, const char c13 = 0, const char c14 = 0, const char c15 = 0
    ) noexcept : chars{}
    {
        chars[0 ] = c0;   chars[1 ] = c1;   chars[2 ] = c2;   chars[3 ] = c3;
        chars[4 ] = c4;   chars[5 ] = c5;   chars[6 ] = c6;   chars[7 ] = c7;
        chars[8 ] = c8;   chars[9 ] = c9;   chars[10] = c10;  chars[11] = c11;
        chars[12] = c12;  chars[13] = c13;  chars[14] = c14;  chars[15] = c15;
    }

    inline constexpr String16(const char* const str) noexcept : chars{} {
        for (uint32_t i = 0; i < MAX_LEN; ++i) {
            if (str[i]) {
                chars[i] = str[i];
            } else {
                break;
            }
        }
    }

    // Comparison of strings
    inline constexpr int64_t compare(const String16& other) const noexcept {
        return (words[0] != other.words[0]) ? (words[0] - other.words[0]) : (words[1] - other.words[1]);
    }

    inline constexpr bool operator <  (const String16& other) const noexcept { return (compare(other) <  0); }
    inline constexpr bool operator <= (const String16& other) const noexcept { return (compare(other) <= 0); }
    inline constexpr bool operator >  (const String16& other) const noexcept { return (compare(other) >  0); }
    inline constexpr bool operator >= (const String16& other) const noexcept { return (compare(other) >= 0); }
    inline constexpr bool operator == (const String16& other) const noexcept { return (compare(other) == 0); }
    inline constexpr bool operator != (const String16& other) const noexcept { return (compare(other) != 0); }

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
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains a fixed capacity string of up to 8 characters (1 64-bit word)
//------------------------------------------------------------------------------------------------------------------------------------------
union String8 {
    // Maximum length of this string (in characters)
    static constexpr uint32_t MAX_LEN = 8;

    // The string expressed as 1 64-bit word or 8 characters
    uint64_t word;
    char     chars[8];

    // Various constructors (constexpr, so conversions can also happen at compile time)
    inline constexpr String8() noexcept : word{} {}
    inline constexpr String8(const uint64_t word) noexcept : word(word) {}

    inline constexpr String8(
        const char c0,     const char c1 = 0, const char c2 = 0, const char c3 = 0,
        const char c4 = 0, const char c5 = 0, const char c6 = 0, const char c7 = 0
    ) noexcept : chars{}
    {
        chars[0] = c0;  chars[1] = c1;  chars[2] = c2;  chars[3] = c3;
        chars[4] = c4;  chars[5] = c5;  chars[6] = c6;  chars[7] = c7;
    }

    inline constexpr String8(const char* const str) noexcept : chars{} {
        for (uint32_t i = 0; i < MAX_LEN; ++i) {
            if (str[i]) {
                chars[i] = str[i];
            } else {
                break;
            }
        }
    }

    // Comparison of strings
    inline constexpr int64_t compare(const String8& other) const noexcept {
        return word - other.word;
    }

    inline constexpr bool operator <  (const String8& other) const noexcept { return (compare(other) <  0); }
    inline constexpr bool operator <= (const String8& other) const noexcept { return (compare(other) <= 0); }
    inline constexpr bool operator >  (const String8& other) const noexcept { return (compare(other) >  0); }
    inline constexpr bool operator >= (const String8& other) const noexcept { return (compare(other) >= 0); }
    inline constexpr bool operator == (const String8& other) const noexcept { return (compare(other) == 0); }
    inline constexpr bool operator != (const String8& other) const noexcept { return (compare(other) != 0); }

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
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Contains a fixed capacity string of up to 4 characters (1 32-bit word)
//------------------------------------------------------------------------------------------------------------------------------------------
union String4 {
    // Maximum length of this string (in characters)
    static constexpr uint32_t MAX_LEN = 4;

    // The string expressed as 1 32-bit word or 4 characters
    uint32_t word;
    char     chars[4];

    // Various constructors (constexpr, so conversions can also happen at compile time)
    inline constexpr String4() noexcept : word{} {}
    inline constexpr String4(const uint32_t word) noexcept : word(word) {}

    inline constexpr String4(const char c0, const char c1 = 0, const char c2 = 0, const char c3 = 0) noexcept : chars{} {
        chars[0] = c0;
        chars[1] = c1;
        chars[2] = c2;
        chars[3] = c3;
    }

    inline constexpr String4(const char* const str) noexcept : chars{} {
        for (uint32_t i = 0; i < MAX_LEN; ++i) {
            if (str[i]) {
                chars[i] = str[i];
            } else {
                break;
            }
        }
    }

    // Comparison of strings
    inline constexpr int32_t compare(const String4& other) const noexcept {
        return word - other.word;
    }

    inline constexpr bool operator <  (const String4& other) const noexcept { return (compare(other) <  0); }
    inline constexpr bool operator <= (const String4& other) const noexcept { return (compare(other) <= 0); }
    inline constexpr bool operator >  (const String4& other) const noexcept { return (compare(other) >  0); }
    inline constexpr bool operator >= (const String4& other) const noexcept { return (compare(other) >= 0); }
    inline constexpr bool operator == (const String4& other) const noexcept { return (compare(other) == 0); }
    inline constexpr bool operator != (const String4& other) const noexcept { return (compare(other) != 0); }

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
};
