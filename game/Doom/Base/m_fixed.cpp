#include "m_fixed.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Multiply two numbers in 16.16 fixed point format
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t FixedMul(const fixed_t a, const fixed_t b) noexcept {
    // Note: the real version of this relied on combining the seperate 32-bit 'hi' and 'lo' portions of the MIPS 'mulu' instruction result.
    // This version shortcuts that a bit and just uses 64-bit types provided by modern C++ rather than going through emulator instructions
    // to get a split hi/lo result. It's the exact same thing basically but this should hopefully be a little faster...
    const bool bNegativeResult = ((a ^ b) < 0);
    const uint64_t a_u64 = (a < 0) ? -a : a;
    const uint64_t b_u64 = (b < 0) ? -b : b;

    const uint64_t result_u64 = (a_u64 * b_u64) >> FRACBITS;
    const fixed_t result_abs = (fixed_t) result_u64;

    return (bNegativeResult) ? -result_abs : result_abs;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Divide 'a' by 'b'. Both numbers are in 16.16 fixed point format.
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t FixedDiv(const fixed_t a, const fixed_t b) noexcept {
    // First step: make the problem an unsigned one
    const bool bNegativeResult = ((a ^ b) < 0);
    uint32_t dividend = (a < 0) ? -a : a;
    uint32_t divisor = (b < 0) ? -b : b;

    // Start off asuming the max result is '1.0'.
    // Shift up the max result by 1 bit until we make the divisor bigger than the dividend.
    // This gives the maximum possible estimated result:
    uint32_t resultBit = FRACUNIT;

    while (divisor < dividend) {
        divisor <<= 1;
        resultBit <<= 1;
    }

    // Start doing the division, one bit at a time
    fixed_t result = 0;

    do {
        // See if the dividend will fit into the divisor at this bit.
        // If so then include this result bit.
        //
        // Note: the original code did this comparison as a SIGNED comparison so I'm doing
        // the same thing here for compatibility. It shouldn't matter in most cases but might
        // be important to get the same result in cases of overflow...
        //
        if ((int32_t) dividend >= (int32_t) divisor) {
            dividend -= divisor;
            result |= resultBit;
        }

        // Move along to the next bit.
        // Stop iteration once theres nothing left to divide or when we have processed all bits.
        dividend <<= 1;
        resultBit >>= 1;
    } while ((resultBit != 0) && (dividend != 0));

    // Last step, correct the sign of the result
    return (bNegativeResult) ? -result : result;
}
