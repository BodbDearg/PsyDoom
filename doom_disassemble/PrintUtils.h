#pragma once

#include <cstdint>
#include <ostream>

//----------------------------------------------------------------------------------------------------------------------
// Various functions for formatting output
//----------------------------------------------------------------------------------------------------------------------
namespace PrintUtils {
    void printHexDigit(const uint8_t nibble, std::ostream& out) noexcept;
    void printHexU8(const uint8_t val, const bool bZeroPad, std::ostream& out) noexcept;
    void printHexI8(const int8_t val, const bool bZeroPad, std::ostream& out) noexcept;
    void printHexU16(const uint16_t val, const bool bZeroPad, std::ostream& out) noexcept;
    void printHexI16(const int16_t val, const bool bZeroPad, std::ostream& out) noexcept;
    void printHexU32(const uint32_t val, const bool bZeroPad, std::ostream& out) noexcept;
    void printHexI32(const int32_t val, const bool bZeroPad, std::ostream& out) noexcept;
}
