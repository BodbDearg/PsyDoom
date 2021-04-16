#pragma once

#include "Macros.h"

#include <algorithm>
#include <limits>
#include <rapidjson/document.h>
#include <type_traits>

BEGIN_NAMESPACE(JsonUtils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to get a single value in a json object.
// On failure a defaulted value will be returned.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline T getOrDefault(const rapidjson::Value& jsonObj, const char* const fieldName, const T& defaultVal) noexcept {
    if (jsonObj.IsObject()) {
        auto iter = jsonObj.FindMember(fieldName);

        if (iter != jsonObj.MemberEnd()) {
            const rapidjson::Value& fieldVal = iter->value;

            if (fieldVal.Is<T>()) {
                return fieldVal.Get<T>();
            }
        }
    }

    return defaultVal;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Same as 'getOrDefault' except the value retrieved is clamped to be in range for it's type.
// Useful for reading 32 or 64 bit integers and clamping to be in range for 8 or 16 bit integers - RapidJson doesn't support those natively.
// This function is only callable for 32-bit or smaller types, since all data must be read as 64-bit in order for clamping to work.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline T clampedGetOrDefault(const rapidjson::Value& jsonObj, const char* const fieldName, const T& defaultVal) noexcept {
    static_assert(sizeof(T) <= 4, "This function should only be used with 32-bit, 16-bit or 8-bit types!");
    static_assert(std::is_arithmetic_v<T>, "This function is only usable on numeric types!");

    if constexpr (std::is_integral_v<T>) {
        const int64_t value = getOrDefault<int64_t>(jsonObj, fieldName, (int64_t) defaultVal);
        return (T) std::clamp<int64_t>(value, (int64_t) std::numeric_limits<T>::min(), (int64_t) std::numeric_limits<T>::max());
    } else {
        const double value = getOrDefault<double>(jsonObj, fieldName, (double) defaultVal);
        return (T) std::clamp<double>(value, (double) std::numeric_limits<T>::min(), (double) std::numeric_limits<T>::max());
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Attempt to get an array child member from a json object and return null on failure
//------------------------------------------------------------------------------------------------------------------------------------------
inline const rapidjson::Value* tryGetArray(const rapidjson::Value& jsonObj, const char* const fieldName) noexcept {
    if (jsonObj.IsObject()) {
        if (auto iter = jsonObj.FindMember(fieldName); iter != jsonObj.MemberEnd()) {
            if (const rapidjson::Value& fieldVal = iter->value; fieldVal.IsArray()) {
                return &fieldVal;
            }
        }
    }

    return nullptr;
}

END_NAMESPACE(JsonUtils)
