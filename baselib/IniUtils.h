#pragma once

#include "Macros.h"

#include <functional>
#include <string>

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

// Represents a callback/handler that receives parsed ini entries (SAX style).
// Note: I would make this function prototype 'noexcept' but std::function<> doesn't handle that so well currently...
typedef std::function<void (const Entry& entry)> EntryHandler;

void parseIniFromString(const char* const pStr, const size_t len, const EntryHandler entryHandler) noexcept;

END_NAMESPACE(IniUtils)
