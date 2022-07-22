#pragma once

#include "Macros.h"

#include <functional>
#include <string>

BEGIN_NAMESPACE(IniUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Stores the value for an entry in an INI file
//------------------------------------------------------------------------------------------------------------------------------------------
struct IniValue {
    std::string strValue;

    template <class T>
    inline void set(const T& newValue) noexcept {
        if constexpr (std::is_same_v<T, std::string>) {
            strValue = newValue;
        } else if constexpr (std::is_same_v<T, bool>) {
            if (newValue) {
                strValue = "1";
            } else {
                strValue = "0";
            }
        } else {
            strValue = std::to_string(newValue);
        }
    }

    inline const std::string& getAsString() const noexcept {
        return strValue;
    }

    inline bool getAsBool() const THROWS {
        const bool isLiteralTrue = (
            (strValue.length() == 4) &&
            (strValue[0] == 't' || strValue[0] == 'T') &&
            (strValue[1] == 'r' || strValue[1] == 'R') &&
            (strValue[2] == 'u' || strValue[2] == 'U') &&
            (strValue[3] == 'e' || strValue[3] == 'E')
        );

        if (isLiteralTrue)
            return true;

        const bool isLiteralFalse = (
            (strValue.length() == 5) &&
            (strValue[0] == 'f' || strValue[0] == 'F') &&
            (strValue[1] == 'a' || strValue[1] == 'A') &&
            (strValue[2] == 'l' || strValue[2] == 'L') &&
            (strValue[3] == 's' || strValue[3] == 'S') &&
            (strValue[4] == 'e' || strValue[4] == 'E')
        );

        if (isLiteralFalse)
            return false;

        const int32_t intValue = getAsInt();
        return (intValue > 0);
    }

    inline bool tryGetAsBool(const bool bDefaultValue) const noexcept {
        try {
            return getAsBool();
        } catch (...) {
            return bDefaultValue;
        }
    }

    inline int32_t getAsInt() const THROWS {
        return std::stoi(strValue);
    }

    inline int32_t tryGetAsInt(const int32_t defaultValue) const noexcept {
        try {
            return getAsInt();
        } catch (...) {
            return defaultValue;
        }
    }

    inline uint32_t getAsUint() const THROWS {
        return (uint32_t) std::stoul(strValue);
    }

    inline uint32_t tryGetAsUint(const uint32_t defaultValue) const noexcept {
        try {
            return getAsUint();
        } catch (...) {
            return defaultValue;
        }
    }

    inline float getAsFloat() const THROWS {
        return std::stof(strValue);
    }

    inline float tryGetAsFloat(const float defaultValue) const noexcept {
        try {
            return getAsFloat();
        } catch (...) {
            return defaultValue;
        }
    }
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents an entry in an INI file
//------------------------------------------------------------------------------------------------------------------------------------------
struct IniEntry {
    std::string     section;
    std::string     key;
    IniValue        value;
};

// Represents a callback/handler that receives parsed ini entries (SAX style).
// Note: I would make this function prototype 'noexcept' but std::function<> doesn't handle that so well currently...
typedef std::function<void (const IniEntry& entry)> IniEntryHandler;

void parseIniFromString(const char* const pStr, const size_t len, const IniEntryHandler entryHandler) noexcept;

END_NAMESPACE(IniUtils)
