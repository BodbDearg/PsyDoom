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

// Forward declare, so we don't have to include the header
uint32_t ptrToVmAddr(const void* const ptr) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Virtual Machine RAM pointer.
// A 32-bit pointer to a location inside of RAM or the scratchpad memory of the emulated PlayStation.
// Provides easy pointer like operations, and works the same in both 32-bit and 64-bit host environments.
//
// Notes:
//  (1) Pointer addresses wrap every 2MB and map to the same location in PSX RAM.
//  (2) This pointer cannot be used to refer to device mapped memory addresses, just plain PSX RAM and the scratchpad.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
class VmPtr {
public:
    // This removes the array bounds in the type, so that arrays decay to pointers like in regular C
    typedef std::remove_all_extents_t<T> ElemTy;

    // Construction and assign
    inline constexpr VmPtr() noexcept : mAddr(0) {}
    inline constexpr VmPtr(const VmPtr& other) noexcept : mAddr(other.mAddr) {}
    inline constexpr VmPtr(const uint32_t addr) noexcept : mAddr(addr) {}
    inline constexpr VmPtr(const std::nullptr_t) noexcept : mAddr(0) {}
    inline VmPtr(T* const pObj) noexcept : mAddr(ptrToVmAddr(pObj)) {}

    inline constexpr void operator = (const VmPtr& other) noexcept { mAddr = other.mAddr; }
    inline constexpr void operator = (const uint32_t addr) noexcept { mAddr = addr; }
    inline constexpr void operator = (const std::nullptr_t) noexcept { mAddr = 0; }
    inline void operator = (T* const pObj) noexcept { mAddr = ptrToVmAddr(pObj); }

    // Change or get the address pointed to
    inline constexpr void reset() noexcept { mAddr = 0; }
    inline constexpr void reset(const uint32_t addr) noexcept { mAddr = addr; }
    inline constexpr uint32_t addr() const noexcept { return mAddr; }

    // Implicit conversion back to uint32_t
    inline constexpr operator uint32_t() const noexcept { return mAddr; }
    
    // Pointer dereferencing
    inline ElemTy* get() const noexcept {
        if (mAddr != 0) {
            // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
            const uint32_t wrappedAddr = (mAddr & 0x3FFFFF);
            // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
            ASSERT_LOG(wrappedAddr + sizeof(ElemTy) <= 0x400000, "Address pointed to spills past the 4 MiB of PSX RAM!");
            return reinterpret_cast<ElemTy*>(PsxVm::gpRam + wrappedAddr);
        } else {
            return nullptr;
        }
    }

    inline ElemTy& operator * () const noexcept {
        ASSERT(mAddr != 0);
        // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
        const uint32_t wrappedAddr = (mAddr & 0x3FFFFF);
        // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
        ASSERT_LOG(wrappedAddr + sizeof(ElemTy) <= 0x400000, "Address pointed to spills past the 4 MiB of PSX RAM!");
        return *reinterpret_cast<ElemTy*>(PsxVm::gpRam + wrappedAddr);
    }

    inline ElemTy* operator -> () const noexcept {
        ASSERT(mAddr != 0);
        // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
        const uint32_t wrappedAddr = (mAddr & 0x3FFFFF);
        // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
        ASSERT_LOG(wrappedAddr + sizeof(ElemTy) <= 0x400000, "Address pointed to spills past the 4 MiB of PSX RAM!");
        return reinterpret_cast<ElemTy*>(PsxVm::gpRam + wrappedAddr);
    }

    inline ElemTy& operator [] (const uint32_t index) const noexcept {
        ASSERT(mAddr != 0);
        // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
        const uint32_t wrappedAddr = (mAddr & 0x3FFFFF);
        const uint32_t elemAddr = wrappedAddr + uint32_t(sizeof(ElemTy)) * index;
        // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
        ASSERT_LOG(elemAddr + sizeof(ElemTy) <= 0x400000, "Array element accessed spills past the 4 MiB of PSX RAM!");
        return *reinterpret_cast<ElemTy*>(PsxVm::gpRam + elemAddr);
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
    inline constexpr VmPtr operator + (const size_t count) const noexcept {
        const int32_t offset = int32_t(sizeof(ElemTy)) * (int32_t) count;
        return VmPtr(mAddr + (uint32_t) offset);
    }

    inline constexpr void operator += (const size_t count) noexcept {
        const int32_t offset = int32_t(sizeof(ElemTy)) * (int32_t) count;
        mAddr += (uint32_t) offset;
    }

    inline constexpr VmPtr operator - (const size_t count) const noexcept {
        const int32_t offset = int32_t(sizeof(ElemTy)) * (int32_t) count;
        return VmPtr(mAddr - (uint32_t) offset);
    }

    inline constexpr void operator -= (const size_t count) noexcept {
        const int32_t offset = int32_t(sizeof(ElemTy)) * (int32_t) count;
        mAddr -= (uint32_t) offset;
    }

    inline constexpr int32_t operator - (const VmPtr other) const noexcept {
        const int32_t addrDiff = (int32_t)(mAddr - other.mAddr);
        return addrDiff / (int32_t) sizeof(ElemTy);
    }

    inline constexpr void operator -- () noexcept     {  mAddr -= sizeof(ElemTy); }
    inline constexpr void operator -- (int) noexcept  {  mAddr -= sizeof(ElemTy); }
    inline constexpr void operator ++ () noexcept     {  mAddr += sizeof(ElemTy); }
    inline constexpr void operator ++ (int) noexcept  {  mAddr += sizeof(ElemTy); }

private:
    uint32_t mAddr;
};

//------------------------------------------------------------------------------------------------------------------------------------------
// Specialization for void pointers
//------------------------------------------------------------------------------------------------------------------------------------------
template <>
class VmPtr<void> {
public:
    inline constexpr VmPtr() noexcept : mAddr(0) {}
    inline constexpr VmPtr(const VmPtr& other) noexcept : mAddr(other.mAddr) {}
    inline constexpr VmPtr(const uint32_t addr) noexcept : mAddr(addr) {}
    inline constexpr VmPtr(const std::nullptr_t) noexcept : mAddr(0) {}
    inline VmPtr(void* const pMem) noexcept : mAddr(ptrToVmAddr(pMem)) {}

    inline constexpr void operator = (const VmPtr& other) noexcept { mAddr = other.mAddr; }
    inline constexpr void operator = (const uint32_t addr) noexcept { mAddr = addr; }
    inline constexpr void operator = (const std::nullptr_t) noexcept { mAddr = 0; }
    inline void operator = (void* const pMem) noexcept { mAddr = ptrToVmAddr(pMem); }

    inline constexpr void reset() noexcept { mAddr = 0; }
    inline constexpr void reset(const uint32_t addr) noexcept { mAddr = addr; }
    inline constexpr uint32_t addr() const noexcept { return mAddr; }

    inline constexpr operator uint32_t() const noexcept { return mAddr; }

    inline void* get() const noexcept {
        if (mAddr != 0) {
            // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
            const uint32_t wrappedAddr = (mAddr & 0x3FFFFF);
            return reinterpret_cast<void*>(PsxVm::gpRam + wrappedAddr);
        } else {
            return nullptr;
        }
    }

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

private:
    uint32_t mAddr;
};
