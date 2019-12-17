#pragma once

#include "PcPsx/Macros.h"
#include <cstdint>
#include <type_traits>

namespace PsxVm {
    extern uint8_t* gpRam;
}

// Sanity check: there could be undefined behavior if std::uint8_t (Avovado's PSX ram unit) is not a 'char' type
// and and therefore exempt from strict aliasing assumptions...
static_assert(
    std::is_same_v<char, std::uint8_t> ||
    std::is_same_v<unsigned char, std::uint8_t>,
    "For this class to work without strict aliasing violations, std::uint8_t should be a 'char' type!"
);

//------------------------------------------------------------------------------------------------------------------------------------------
// Virtual Machine RAM pointer.
// A 32-bit pointer to a location inside of RAM of the emulated PlayStation.
// Provides easy pointer like operations, and works the same in both 32-bit and 64-bit host environments.
//
// Notes:
//  (1) Pointer addresses wrap every 2MB and map to the same location in PSX RAM.
//  (2) This pointer cannot be used to refer to device mapped memory addresses, just plain PSX RAM.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
class VmPtr {
public:
    // Construction and assign
    inline constexpr VmPtr() : mAddr(0) noexcept {}
    inline constexpr VmPtr(const VmPtr& other) noexcept : mAddr(other.mAddr) {}
    inline constexpr VmPtr(const uint32_t addr) noexcept : mAddr(addr) {}
    inline constexpr VmPtr(const std::nullptr_t) noexcept : mAddr(0) {}

    inline constexpr void operator = (const VmPtr& other) noexcept { mAddr = other.addr; }
    inline constexpr void operator = (const uint32_t addr) noexcept { mAddr = addr; }
    inline constexpr void operator = (const std::nullptr_t) noexcept { mAddr = 0; }

    // Change or get the address pointed to
    inline constexpr void reset() noexcept { mAddr = 0; }
    inline constexpr void reset(const uint32_t addr) noexcept { mAddr = addr; }
    inline constexpr uint32_t addr() const noexcept { return mAddr; }

    // Check if not null
    inline constexpr operator bool () const noexcept { return (mAddr != 0); }

    // Pointer dereferencing
    inline T* get() const noexcept {
        const uint32_t wrappedAddr = (mAddr & 0x1FFFFF);
        ASSERT_LOG(wrappedAddr + sizeof(T) <= 0x200000, "Address pointed to spills past the 2MB of PSX RAM!");
        return reinterpret_cast<T*>(PsxVm::gpRam + wrappedAddr);
    }

    inline T& operator * () const noexcept {    
        const uint32_t wrappedAddr = (mAddr & 0x1FFFFF);
        ASSERT_LOG(wrappedAddr + sizeof(T) <= 0x200000, "Address pointed to spills past the 2MB of PSX RAM!");
        return *reinterpret_cast<T*>(PsxVm::gpRam + wrappedAddr);
    }

    inline T* operator -> () const noexcept {
        const uint32_t wrappedAddr = (mAddr & 0x1FFFFF);
        ASSERT_LOG(wrappedAddr + sizeof(T) <= 0x200000, "Address pointed to spills past the 2MB of PSX RAM!");
        return reinterpret_cast<T*>(PsxVm::gpRam + wrappedAddr);
    }

    inline T& operator [] (const size_t index) const noexcept {
        const uint32_t wrappedAddr = (mAddr & 0x1FFFFF);
        const uint32_t elemAddr = wrappedAddr + uint32_t(sizeof(T)) * index;
        ASSERT_LOG(elemAddr + sizeof(T) <= 0x200000, "Array element accessed spills past the 2MB of PSX RAM!");
        return reinterpret_cast<T*>(PsxVm::gpRam + elemAddr);
    }

    // Pointer comparison
    inline constexpr bool operator == (const VmPtr other) const noexcept { return (mAddr == other.mAddr); }
    inline constexpr bool operator != (const VmPtr other) const noexcept { return (mAddr != other.mAddr); }
    inline constexpr bool operator < (const VmPtr other) const noexcept { return (mAddr < other.mAddr); }
    inline constexpr bool operator <= (const VmPtr other) const noexcept { return (mAddr <= other.mAddr); }
    inline constexpr bool operator > (const VmPtr other) const noexcept { return (mAddr > other.mAddr); }
    inline constexpr bool operator >= (const VmPtr other) const noexcept { return (mAddr >= other.mAddr); }

    inline constexpr bool operator == (const std::nullptr_t) const noexcept { return (mAddr == 0); }
    inline constexpr bool operator != (const std::nullptr_t) const noexcept { return (mAddr != 0); }
    inline constexpr bool operator < (const std::nullptr_t) const noexcept { return false; }
    inline constexpr bool operator <= (const std::nullptr_t) const noexcept { return (mAddr == 0); }
    inline constexpr bool operator > (const std::nullptr_t) const noexcept { return (mAddr > 0); }
    inline constexpr bool operator >= (const std::nullptr_t) const noexcept { return (mAddr >= 0); }

    // Pointer arithmetic
    inline constexpr VmPtr operator + (const int32_t count) const noexcept {
        const int32_t offset = int32_t(sizeof(T)) * count;
        return VmPtr(mAddr + (uint32_t) offset);
    }

    inline constexpr void operator += (const int32_t count) const noexcept {
        const int32_t offset = int32_t(sizeof(T)) * count;
        mAddr += (uint32_t) offset;
    }

    inline constexpr VmPtr operator - (const int32_t count) const noexcept {
        const int32_t offset = int32_t(sizeof(T)) * count;
        return VmPtr(mAddr - (uint32_t) offset);
    }

    inline constexpr void operator -= (const int32_t count) const noexcept {
        const int32_t offset = int32_t(sizeof(T)) * count;
        mAddr -= (uint32_t) offset;
    }

    inline constexpr int32_t operator - (const VmPtr other) const noexcept {
        const int32_t addrDiff = (int32_t)(mAddr - other.mAddr);
        return addrDiff / (int32_t) sizeof(T);
    }

private:
    uint32_t mAddr;
};
