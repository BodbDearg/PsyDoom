#include "PrintUtils.h"

#include <type_traits>

void PrintUtils::printHexDigit(const uint8_t nibble, std::ostream& out) noexcept {
    const uint8_t nibble4Bit = nibble & 0x0Fu;

    if (nibble4Bit < 10) {
        out.put('0' + (char) nibble4Bit);
    } else {
        out.put('A' + (char)(nibble4Bit - 10));
    }
}

template <class T>
static void printHexInt(const T val, const bool bZeroPad, std::ostream& out) noexcept {
    // Add in the negative sign if required
    if constexpr (std::is_signed_v<T>) {
        if (val < 0) {
            out << '-';
        }
    }

    // Get the absolute value to print
    typedef std::make_unsigned_t<T> UnsignedT;
    UnsignedT valAbs;

    if constexpr (std::is_signed_v<T>) {
        if (val < 0) {
            if (val == std::numeric_limits<T>::min()) {
                // Have to special case the lowest possible number because this is not representible
                // as positive in twos complement format when using the same signed data type.
                // Doing this bit cast will do the trick for us:
                valAbs = (UnsignedT) val;
            } else {
                valAbs = (UnsignedT) -val;
            }
        } else {
            valAbs = (UnsignedT) val;
        }
    } else {
        valAbs = val;
    }

    // First chop off any leading '0' if not zero padding
    uint32_t curShift = sizeof(T) * 8;

    if (!bZeroPad) {
        while (curShift > 4) {
            const uint8_t nibble = (uint8_t)((valAbs >> (curShift - 4)) & 0x0Fu);

            if (nibble == 0) {
                curShift -= 4;
            } else {
                break;
            }
        }
    }

    // Print each nibble
    while (curShift > 0) {
        curShift -= 4;
        const uint8_t nibble = (uint8_t)((valAbs >> curShift) & 0x0Fu);
        PrintUtils::printHexDigit(nibble, out);
    }
}

void PrintUtils::printHexU8(const uint8_t val, const bool bZeroPad, std::ostream& out) noexcept {
    printHexInt(val, bZeroPad, out);
}

void PrintUtils::printHexI8(const int8_t val, const bool bZeroPad, std::ostream& out) noexcept {
    printHexInt(val, bZeroPad, out);
}

void PrintUtils::printHexU16(const uint16_t val, const bool bZeroPad, std::ostream& out) noexcept {
    printHexInt(val, bZeroPad, out);
}

void PrintUtils::printHexI16(const int16_t val, const bool bZeroPad, std::ostream& out) noexcept {
    printHexInt(val, bZeroPad, out);
}

void PrintUtils::printHexU32(const uint32_t val, const bool bZeroPad, std::ostream& out) noexcept {
    printHexInt(val, bZeroPad, out);
}

void PrintUtils::printHexI32(const int32_t val, const bool bZeroPad, std::ostream& out) noexcept {
    printHexInt(val, bZeroPad, out);
}
