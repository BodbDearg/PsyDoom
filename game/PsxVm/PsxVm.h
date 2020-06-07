#pragma once

#include "PcPsx/Macros.h"

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface: shorthand type aliases
//------------------------------------------------------------------------------------------------------------------------------------------
typedef void (*VmFunc)();

//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface: function calls and utilities
//------------------------------------------------------------------------------------------------------------------------------------------

// Emulate sound until we have enough samples to handle an upcoming buffer request
void emulate_sound_if_required() noexcept;

// Fire timer (root counter) related events if appropriate.
// Note: this is implemented in LIBAPI, where timers are handled.
void generate_timer_events() noexcept;

// Returns the 32-bit address in PSX RAM (in the 0x80000000 space/segment) of the given real pointer.
// Used to convert real pointers/addresses back to VM ones.
// May exit the application with an error if an invalid pointer is given.
uint32_t ptrToVmAddr(const void* const ptr) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Avocado types
//------------------------------------------------------------------------------------------------------------------------------------------
struct System;

namespace mips              { struct CPU;   }
namespace gpu               { class GPU;    }
namespace spu               { struct SPU;   }
namespace device::cdrom     { class CDROM;  }

//------------------------------------------------------------------------------------------------------------------------------------------
// VM control: setup and control the VM
//------------------------------------------------------------------------------------------------------------------------------------------
namespace PsxVm {
    // Access to the emulated PSX SPU & GPU etc.
    extern System*                  gpSystem;
    extern gpu::GPU*                gpGpu;
    extern spu::SPU*                gpSpu;
    extern device::cdrom::CDROM*    gpCdrom;
    extern uint8_t*                 gpRam;    
    extern uint8_t*                 gpScratchpad;       // Cache used as fast RAM (1 KiB) (TODO: make this a native app buffer)

    // Initialize the VM with the original Playstation DOOM .EXE and CD file path (.cue format)
    bool init(const char* const doomExePath, const char* const doomCdCuePath) noexcept;
    void shutdown() noexcept;

    // Updates inputs to the emulator from real inputs on the host machine
    void updateInput() noexcept;

    // Lookup the function pointer to call for a given address
    VmFunc getVmFuncForAddr(const uint32_t addr) noexcept;

    // Give the PSX VM address for the given native C++ function.
    // Will return '0' if the native function is not mapped to a VM address.
    uint32_t getNativeFuncVmAddr(void* const pFunc) noexcept;

    // Get the button bits for the controller directly, bypassing emulation
    uint16_t getControllerButtonBits() noexcept;

    // Submit a drawing primitive to the GPU
    void submitGpuPrimitive(const void* const pPrim) noexcept;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a VM address to a pointer.
// Note: only RAM and scratchpad memory addresses are converted.
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline T* vmAddrToPtr(const uint32_t addr) noexcept {
    if (addr != 0) {
        if (addr >= 0x1F800000 && addr <= 0x1F800400) {
            // Scratchpad RAM address
            const uint32_t relativeAddr = addr - 0x1F800000;
            ASSERT_LOG(relativeAddr + sizeof(T) <= 0x400, "Address pointed to spills past the 1 KiB scratchpad!");
            return reinterpret_cast<T*>(PsxVm::gpScratchpad + relativeAddr);
        } else {
            // Regular RAM address
            // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
            const uint32_t wrappedAddr = (addr & 0x3FFFFF);
            // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
            ASSERT_LOG(wrappedAddr + sizeof(T) <= 0x400000, "Address pointed to spills past the 2 MiB of PSX RAM!");
            return reinterpret_cast<T*>(PsxVm::gpRam + wrappedAddr);
        }
    } else {
        return nullptr;
    }
}

template <>
inline void* vmAddrToPtr(const uint32_t addr) noexcept {
    if (addr != 0) {
        if (addr >= 0x1F800000 && addr <= 0x1F800400) {
            // Scratchpad RAM address
            const uint32_t relativeAddr = addr - 0x1F800000;
            return reinterpret_cast<void*>(PsxVm::gpScratchpad + relativeAddr);
        } else {
            // Regular RAM address
            // FIXME: temporarily expanding the PSX RAM to 4 MiB to accomodate larger structs due to 64-bit pointers
            const uint32_t wrappedAddr = (addr & 0x3FFFFF);
            return reinterpret_cast<void*>(PsxVm::gpRam + wrappedAddr);
        }
    } else {
        return nullptr;
    }
}
