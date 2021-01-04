#pragma once

#include "Macros.h"

#include <algorithm>
#include <type_traits>

//------------------------------------------------------------------------------------------------------------------------------------------
// Various random helper functions used by this library
//------------------------------------------------------------------------------------------------------------------------------------------
BEGIN_NAMESPACE(vgl)
BEGIN_NAMESPACE(Utils)

//------------------------------------------------------------------------------------------------------------------------------------------
// Return the ceiling of the following unsigned integer division: dividend / divisor
// If there is a remainder or fractional part to the result, the result will be rounded up to the next whole integer.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline constexpr T udivCeil(const T dividend, const T divisor) noexcept {
    static_assert(std::is_integral_v<T>, "Should only be called for integer types!");
    static_assert(std::is_unsigned_v<T>, "Should only be called for unsigned integer types!");

    // According to Stack Overflow the compiler should be able to take advantage of the fact that the divide instruction often returns
    // the answer and remainder in one go, so it can optimize this code to avoid the extra modulus operation:
    //  https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
    return dividend / divisor + (dividend % divisor != 0);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Align the given unsigned number to be multiples of the given number.
// If the number is not in alignment then the alignment is performed upwards.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline constexpr T ualignUp(const T number, const T alignment) noexcept {
    return udivCeil(number, alignment) * alignment;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the highest set bit in an integer number: returns '0' if the 1st bit is the highest set bit, '1' if the 2nd bit is highest and so on.
// The result is undefined for a number with 0 bits set, since the question does not make sense in that context.
// 0 will be returned in that case.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline constexpr T highestSetBit(const T number) noexcept {
    // Get the value as an unsigned integer (to avoid undefined behavior) and figure out the highest bit by continually shifting to the right
    static_assert(std::is_integral_v<T>, "Should only be called for integer types!");
    typedef std::make_unsigned_t<T> TU;
    TU bitsLeft = (TU) number >> 1;
    T highestSetBit = 0;

    while (bitsLeft != 0) {
        ++highestSetBit;
        bitsLeft >>= 1;
    }

    return highestSetBit;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if the given number is a power of two
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline constexpr T isPowerOf2(const T number) noexcept {
    // Negative numbers cannot be powers of two
    if constexpr (std::is_signed_v<T>) {
        if (number <= 0)
            return false;
    }

    return ((T(1) << highestSetBit(number)) == number);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Perform a 'power' function recursively in such a way that it can be used as a constexpr function.
//
// Note that this is NOT an efficient way to do pow() at runtime (well, it's probably faster for ^2 and maybe ^3)
// and is more intended to be used at compile time.
//
// This pow() implementation also has the restriction that the power being raised to must be an unsigned integer only.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline constexpr T powRecursive(const T number, const size_t power) noexcept {
    return (power <= 0) ? 1 : number * powRecursive(number, power - 1);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if a container contains a given element
//------------------------------------------------------------------------------------------------------------------------------------------
template <class ContainerT, class ElemT>
inline constexpr bool containerContains(const ContainerT& container, const ElemT& elem) noexcept {
    return (std::find(container.begin(), container.end(), elem) != container.end());
}

END_NAMESPACE(Utils)
END_NAMESPACE(vgl)
