#pragma once

#include "PcPsx/Macros.h"
#include <cstdint>
#include <type_traits>

namespace PsxVm {
    extern uint32_t*    gpReg_sp;
    extern uint8_t*     gpRam;
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
// Virtual Machine stack value.
// This is a value that is allocated on the stack inside the RAM of the emulated PlayStation and accessed in the same way as a VmPtr.
// The PSX stack pointer is automatically decreased and increased when the stack value is created and destroyed.
// Useful for replacing the use of stack allocated objects in the translated MIPS code.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
class VmSVal {
public:
    template <class ...CtorArgs>
    inline VmSVal(CtorArgs&&... ctorArgs) noexcept
        : mVal(*getObjPtrFromVmAddr(*PsxVm::gpReg_sp - getStackSize()))
    {
        new(&mVal) T(ctorArgs...);
        *PsxVm::gpReg_sp -= getStackSize();
    }

    inline ~VmSVal() noexcept {
        mVal.~T();
        *PsxVm::gpReg_sp += getStackSize();
    }

    // Get the RAM address of the object on the stack
    inline uint32_t addr() const noexcept { return ptrToVmAddr(&mVal); }

    // Pointer dereferencing
    inline T* get() const noexcept { return &mVal; }
    inline T& operator * () const noexcept { return mVal; }
    inline T* operator -> () const noexcept { return &mVal; }

private:
    // Get the size of this object when it is placed on the stack.
    // The size is aligned up to 4 byte boundaries.
    static inline constexpr uint32_t getStackSize() noexcept {
        return ((uint32_t) sizeof(T) + 3) & 0xFFFFFFFC;
    }

    // Convert the given VM address to a pointer of this object type
    inline T* getObjPtrFromVmAddr(const uint32_t addr) noexcept {
        if (addr != 0) {
            const uint32_t wrappedAddr = (addr & 0x1FFFFF);
            ASSERT_LOG(wrappedAddr + sizeof(T) <= 0x200000, "Stack overflow!!");
            return reinterpret_cast<T*>(PsxVm::gpRam + wrappedAddr);
        } else {
            return nullptr;
        }
    }

    T& mVal;
};
