#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// Endian related utilities.
// A lot (or all?) of this code can probably go away once we have C++ 20 and it's endian utilities.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(Endian)

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the host CPU is little endian
//------------------------------------------------------------------------------------------------------------------------------------------
inline constexpr bool isLittle() noexcept {
    // Note: there's no standardized way to tell this in C++ 17 but in C++ 20 there will be.
    // For now however just assume little endian, since that pretty much dominates computing these days.
    // This function can always be adjusted later if need be:
    return true;
}

inline constexpr bool isBig() noexcept {
    return (!isLittle());
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Swap bytes of integer types
//------------------------------------------------------------------------------------------------------------------------------------------
inline constexpr uint8_t byteSwap(const uint8_t num) noexcept { return num; }

inline constexpr uint16_t byteSwap(const uint16_t num) noexcept {
    return (
        (uint16_t)((num & 0x00FFu) << 8) |
        (uint16_t)((num & 0xFF00u) >> 8)
    );
}

inline constexpr uint32_t byteSwap(const uint32_t num) noexcept {
    return (
        ((num & 0x000000FFu) << 24) |
        ((num & 0x0000FF00u) << 8) |
        ((num & 0x00FF0000u) >> 8) |
        ((num & 0xFF000000u) >> 24)
    );
}

inline constexpr int8_t byteSwap(const int8_t num) noexcept { return num; }
inline constexpr int16_t byteSwap(const int16_t num) noexcept { return (int16_t) byteSwap((uint16_t) num); }
inline constexpr int32_t byteSwap(const int32_t num) noexcept { return (int32_t) byteSwap((uint32_t) num); }

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert from little endian to host endian and visa versa
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline constexpr T littleToHost(const T num) noexcept {
    if constexpr (isLittle()) {
        return num;
    } else {
        return byteSwap(num);
    }
}

template <class T>
inline constexpr T bigToHost(const T num) noexcept {
    if constexpr (isLittle()) {
        return byteSwap(num);
    } else {
        return num;
    }
}

template <class T>
inline constexpr T hostToLittle(const T num) noexcept {
    if constexpr (isLittle()) {
        return num;
    } else {
        return byteSwap(num);
    }
}

template <class T>
inline constexpr T hostToBig(const T num) noexcept {
    if constexpr (isLittle()) {
        return byteSwap(num);
    } else {
        return num;
    }
}

END_NAMESPACE(Endian)
