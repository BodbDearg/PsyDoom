#include "PrintUtils.h"

void PrintUtils::printHexDigit(const uint8_t nibble, std::stringstream& out) noexcept {
    const uint8_t nibble4Bit = nibble & 0x0Fu;

    if (nibble4Bit < 10) {
        out.put('0' + (char) nibble4Bit);
    } else {
        out.put('A' + (char)(nibble4Bit - 10));
    }
}

void PrintUtils::printHexU8(const uint8_t valU16, const bool bZeroPad, std::stringstream& out) noexcept {
    // First chop off any leading '0' if not zero padding
    int16_t curShift = 8;

    if (!bZeroPad) {
        while (curShift > 4) {
            const uint8_t nibble = (uint8_t)((valU16 >> (curShift - 4)) & 0x0Fu);

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
        const uint8_t nibble = (uint8_t)((valU16 >> curShift) & 0x0Fu);
        printHexDigit(nibble, out);
    }
}

void PrintUtils::printHexI16(const int16_t valI16, const bool bZeroPad, std::stringstream& out) noexcept {
    // Add in the negative sign and get the absolute value to print
    const int32_t valI32 = valI16;
    const uint32_t valAbs = std::abs(valI32);

    if (valI32 < 0) {
        out << '-';
    }

    // First chop off any leading '0' if not zero padding
    int16_t curShift = 16;

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
        printHexDigit(nibble, out);
    }
}

void PrintUtils::printHexU16(const uint16_t valU16, const bool bZeroPad, std::stringstream& out) noexcept {
    // First chop off any leading '0' if not zero padding
    int16_t curShift = 16;

    if (!bZeroPad) {
        while (curShift > 4) {
            const uint8_t nibble = (uint8_t)((valU16 >> (curShift - 4)) & 0x0Fu);

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
        const uint8_t nibble = (uint8_t)((valU16 >> curShift) & 0x0Fu);
        printHexDigit(nibble, out);
    }
}

void PrintUtils::printHexU32(const uint32_t valU32, const bool bZeroPad, std::stringstream& out) noexcept {
    // First chop off any leading '0' if not zero padding
    int16_t curShift = 32;

    if (!bZeroPad) {
        while (curShift > 4) {
            const uint8_t nibble = (uint8_t)((valU32 >> (curShift - 4)) & 0x0Fu);

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
        const uint8_t nibble = (uint8_t)((valU32 >> curShift) & 0x0Fu);
        printHexDigit(nibble, out);
    }
}
