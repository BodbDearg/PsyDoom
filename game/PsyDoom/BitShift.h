#pragma once

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: left and right shift numbers by fixed amounts according to the method PSX Doom relies upon for its behavior.
//
// Left and right shifts of negative numbers are undefined in the C++ standard and could behave differently depending on architecture.
// These helpers allow us to create well defined behavior in the case of negative numbers to avoid these problems.
// An optimizing compiler will likely re-convert these operations back to normal shifts with little overhead, architecture allowing...
// 
// Note: this module can probably go away if C++ 20 becomes the minimum language target for PsyDoom.
// In C++ 20 the language now defines the integer format as two's complement, thus the behavior of signed shifts become well defined.
//------------------------------------------------------------------------------------------------------------------------------------------
#include <cstdint>
#include <limits>
#include <type_traits>

template <uint32_t Shift, class T>
static inline constexpr T d_lshift(const T val) noexcept {
    static_assert(std::is_integral_v<T>);

    // Signed or unsigned type?
    if constexpr (std::is_unsigned_v<T>) {
        return val << Shift;
    } else {
        // Left shifts of signed integers are just done with a simple logical shift, even for negative numbers
        typedef std::make_unsigned_t<T> TUnsigned;
        return (T)((TUnsigned) val << Shift);
    }
}

template <> inline constexpr int32_t d_lshift<0, int32_t>(const int32_t val) noexcept { return val; }
template <> inline constexpr int64_t d_lshift<0, int64_t>(const int64_t val) noexcept { return val; }

template <uint32_t Shift, class T>
static inline constexpr T d_rshift(const T val) noexcept {
    static_assert(std::is_integral_v<T>);

    // Signed or unsigned type?
    if constexpr (std::is_unsigned_v<T>) {
        return val >> Shift;
    } else {
        // This replicates Intel/MIPS style behavior in the case of negative numbers by filling leading bits with '1'
        typedef std::make_unsigned_t<T> TUnsigned;

        // First logical shift the value as if it were unsigned
        const TUnsigned valu = (TUnsigned) val;
        const TUnsigned shifted = valu >> Shift;

        // Next sign extend by filling newly empty bits with '1' if negative
        constexpr TUnsigned SIGN_BIT = TUnsigned(1) << ((sizeof(T) * 8) - 1);
        const TUnsigned signExtend = -((valu & SIGN_BIT) >> Shift);
        return shifted | signExtend;
    }
}

template <> inline constexpr int32_t d_rshift<0, int32_t>(const int32_t val) noexcept { return val; }
template <> inline constexpr int64_t d_rshift<0, int64_t>(const int64_t val) noexcept { return val; }
