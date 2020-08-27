#pragma once

#include <cstdint>
#include <ostream>

//------------------------------------------------------------------------------------------------------------------------------------------
// Various functions for formatting output
//------------------------------------------------------------------------------------------------------------------------------------------
namespace PrintUtils {
    void printHexDigit(const uint8_t nibble, std::ostream& out);
    void printHexU8(const uint8_t val, const bool bZeroPad, std::ostream& out);
    void printHexI8(const int8_t val, const bool bZeroPad, std::ostream& out);
    void printHexU16(const uint16_t val, const bool bZeroPad, std::ostream& out);
    void printHexI16(const int16_t val, const bool bZeroPad, std::ostream& out);
    void printHexU32(const uint32_t val, const bool bZeroPad, std::ostream& out);
    void printHexI32(const int32_t val, const bool bZeroPad, std::ostream& out);
    void printBool(const bool bVal, std::ostream& out);

    // Print a char with characters like newline '\n' or above ascii characters escaped
    void printEscapedChar(const char val, std::ostream& out);
}
