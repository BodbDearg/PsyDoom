#pragma once

#include <cstdint>
#include <sstream>

//----------------------------------------------------------------------------------------------------------------------
// Various functions for formatting output
//----------------------------------------------------------------------------------------------------------------------
namespace PrintUtils {
    void printHexDigit(const uint8_t nibble, std::stringstream& out) noexcept;
    void printHexI16(const int16_t valI16, const bool bZeroPad, std::stringstream& out) noexcept;
    void printHexU16(const uint16_t valU16, const bool bZeroPad, std::stringstream& out) noexcept;
    void printHexU32(const uint32_t valU32, const bool bZeroPad, std::stringstream& out) noexcept;
}
