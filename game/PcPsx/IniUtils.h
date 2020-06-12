#include "Macros.h"

#include <string>

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
BEGIN_NAMESPACE(IniUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents an entry in an ini file
//------------------------------------------------------------------------------------------------------------------------------------------
struct Entry {
    std::string section;
    std::string key;
    std::string value;

    template <class T>
    inline void setValue(const T& newValue) noexcept {
        if constexpr (std::is_same_v<T, std::string>) {
            value = newValue;
        } else if constexpr (std::is_same_v<T, bool>) {
            if (newValue) {
                value = "1";
            } else {
                value = "0";
            }
        } else {
            value = std::to_string(newValue);
        }
    }

    inline bool getBoolValue(const bool bDefaultValue = false) const noexcept {
        const bool isLiteralTrue = (
            (value[0] == 't' || value[0] == 'T') &&
            (value[1] == 'r' || value[1] == 'R') &&
            (value[2] == 'u' || value[2] == 'U') &&
            (value[3] == 'e' || value[3] == 'E')
        );

        if (isLiteralTrue)
            return true;

        const bool isLiteralFalse = (
            (value[0] == 'f' || value[0] == 'F') &&
            (value[1] == 'a' || value[1] == 'A') &&
            (value[2] == 'l' || value[2] == 'L') &&
            (value[3] == 's' || value[3] == 'S') &&
            (value[4] == 'e' || value[4] == 'E')
        );

        if (isLiteralFalse)
            return false;

        int32_t intValue = getIntValue(bDefaultValue ? 1 : 0);
        return (intValue > 0);
    }

    inline int32_t getIntValue(const int32_t defaultValue = 0) const noexcept {
        try {
            return std::stoi(value);
        } catch (...) {
            return defaultValue;
        }
    }

    inline uint32_t getUintValue(const uint32_t defaultValue = 0) const noexcept {
        try {
            return (uint32_t) std::stoul(value);
        } catch (...) {
            return defaultValue;
        }
    }

    inline float getFloatValue(const float defaultValue = 0.0f) const noexcept {
        try {
            return std::stof(value);
        } catch (...) {
            return defaultValue;
        }
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents a callback/handler that receives parsed ini entries (SAX style)
//------------------------------------------------------------------------------------------------------------------------------------------
typedef void (*EntryHandler)(const Entry& entry) noexcept;

void parseIniFromString(const char* const pStr, const size_t len, const EntryHandler entryHandler) noexcept;

END_NAMESPACE(IniUtils)
