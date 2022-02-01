#pragma once

#include "Asserts.h"

#include <cstdint>
#include <type_traits>

//------------------------------------------------------------------------------------------------------------------------------------------
// A simple unsigned integer container for a series of bit flags.
// Provides helper functionality for getting and setting the flags.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class BitsT>
struct Flags {
    // The underlying bit container must be an unsigned integer type!
    static_assert(std::is_unsigned_v<BitsT>);

    // The maximum number of bit flags that can be stored
    static constexpr uint32_t CAPACITY = sizeof(BitsT) * 8;

    // The raw bits for the flags.
    // Normally you wouldn't be accessing these directly.
    BitsT bits;

    // Get a particular bit flag.
    // Assumes the bit index is in range, the results are undefined otherwise!
    inline constexpr bool get(const uint32_t bitIdx) const noexcept {
        ASSERT(bitIdx < CAPACITY);
        return ((bits >> bitIdx) & 1);
    }

    // Set a particular bit flag only.
    // Assumes the bit index is in range, the results are undefined otherwise!
    inline constexpr void set(const uint32_t bitIdx) noexcept {
        ASSERT(bitIdx < CAPACITY);
        const BitsT bitMask = BitsT(1) << bitIdx;
        bits |= bitMask;
    }

    // Clear a particular bit flag only.
    // Assumes the bit index is in range, the results are undefined otherwise!
    inline constexpr void clear(const uint32_t bitIdx) noexcept {
        ASSERT(bitIdx < CAPACITY);
        const BitsT bitMask = BitsT(1) << bitIdx;
        bits &= ~bitMask;
    }

    // Set or clear a particular bit flag.
    // Assumes the bit index is in range, the results are undefined otherwise!
    inline constexpr void set(const uint32_t bitIdx, const bool bSet) noexcept {
        ASSERT(bitIdx < CAPACITY);
        const BitsT bitMask = BitsT(1) << bitIdx;
        
        if (bSet) {
            bits |= bitMask;
        } else {
            bits &= ~bitMask;
        }
    }

    // This is a helper bound to a particular flag in the flags set.
    // It's useful for abstracting the details of which bit correpsonds to a particular flag.
    template <uint32_t BitIdx>
    struct Field {
        inline constexpr Field(Flags<BitsT>& flags) noexcept : flags(flags) {}

        inline constexpr bool get() const noexcept { return flags.get(BitIdx); }
        inline constexpr void set() noexcept { flags.set(BitIdx); }
        inline constexpr void clear() noexcept { flags.clear(BitIdx); }
        inline constexpr void set(const bool bSet) noexcept { flags.set(BitIdx, bSet); }
        inline constexpr operator bool() const noexcept { return flags.get(BitIdx); }

        inline constexpr bool operator = (const bool bSet) noexcept {
            flags.set(BitIdx, bSet);
            return bSet;
        }

        Flags<BitsT>& flags;
    };

    // Binding to a read-only version of a field.
    template <uint32_t BitIdx>
    struct CField {
        inline constexpr CField(const Flags<BitsT>& flags) noexcept : flags(flags) {}

        inline constexpr bool get() const noexcept { return flags.get(BitIdx); }
        inline constexpr operator bool() const noexcept { return flags.get(BitIdx); }

        const Flags<BitsT>& flags;
    };

    // Get a particular field in the flag set
    template <uint32_t BitIdx>
    inline constexpr Field<BitIdx> getField() noexcept { return Field<BitIdx>(*this); }

    template <uint32_t BitIdx>
    inline constexpr CField<BitIdx> getField() const noexcept { return CField<BitIdx>(*this); }
};

// Convenience typedefs
typedef Flags<uint8_t>      Flags8;
typedef Flags<uint16_t>     Flags16;
typedef Flags<uint32_t>     Flags32;
typedef Flags<uint64_t>     Flags64;

// Helper macro that defines member functions returning a flag field, in const and non-const variants.
// This takes some of the boilerplate out of defining bit flags.
#define DEFINE_FLAGS_FIELD_MEMBER(FlagsHolder, BitIdx, FieldName)\
    inline constexpr auto FieldName() noexcept { return FlagsHolder.getField<BitIdx>(); }\
    inline constexpr auto FieldName() const noexcept { return FlagsHolder.getField<BitIdx>(); }
