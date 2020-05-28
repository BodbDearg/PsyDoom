#pragma once

#include "PcPsx/Macros.h"
#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface: shorthand type aliases
//------------------------------------------------------------------------------------------------------------------------------------------
typedef uint32_t    u32;
typedef uint16_t    u16;
typedef uint8_t     u8;
typedef int32_t     i32;
typedef int16_t     i16;
typedef int8_t      i8;

typedef void (*VmFunc)();

//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface: Mips R3000 registers and macros aliasing them.
// Note: not allowing modification of the 'zero' register.
//------------------------------------------------------------------------------------------------------------------------------------------
namespace PsxVm {
    extern const uint32_t* gpReg_zero;
    extern uint32_t* gpReg_at;
    extern uint32_t* gpReg_v0;
    extern uint32_t* gpReg_v1;
    extern uint32_t* gpReg_a0;
    extern uint32_t* gpReg_a1;
    extern uint32_t* gpReg_a2;
    extern uint32_t* gpReg_a3;
    extern uint32_t* gpReg_t0;
    extern uint32_t* gpReg_t1;
    extern uint32_t* gpReg_t2;
    extern uint32_t* gpReg_t3;
    extern uint32_t* gpReg_t4;
    extern uint32_t* gpReg_t5;
    extern uint32_t* gpReg_t6;
    extern uint32_t* gpReg_t7;
    extern uint32_t* gpReg_s0;
    extern uint32_t* gpReg_s1;
    extern uint32_t* gpReg_s2;
    extern uint32_t* gpReg_s3;
    extern uint32_t* gpReg_s4;
    extern uint32_t* gpReg_s5;
    extern uint32_t* gpReg_s6;
    extern uint32_t* gpReg_s7;
    extern uint32_t* gpReg_t8;
    extern uint32_t* gpReg_t9;
    extern uint32_t* gpReg_k0;
    extern uint32_t* gpReg_k1;
    extern uint32_t* gpReg_gp;
    extern uint32_t* gpReg_sp;
    extern uint32_t* gpReg_fp;
    extern uint32_t* gpReg_ra;
    extern uint32_t* gpReg_hi;
    extern uint32_t* gpReg_lo;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface: mips instructions.
// These instructions were not so easy to convert directly to C++ so they are handled via functions.
//------------------------------------------------------------------------------------------------------------------------------------------

// Arithmetic
uint32_t add(const uint32_t r1, const uint32_t r2) noexcept;
uint32_t addi(const uint32_t r1, const int16_t i) noexcept;
uint32_t sub(const uint32_t r1, const uint32_t r2) noexcept;

// RAM to CPU loads
uint32_t lb(const uint32_t addr) noexcept;
uint32_t lbu(const uint32_t addr) noexcept;
uint32_t lh(const uint32_t addr) noexcept;
uint32_t lhu(const uint32_t addr) noexcept;
uint32_t lw(const uint32_t addr) noexcept;

// CPU to RAM stores
void sb(const uint32_t r1, const uint32_t addr) noexcept;
void sh(const uint32_t r1, const uint32_t addr) noexcept;
void sw(const uint32_t r1, const uint32_t addr) noexcept;

// Write directly to GPU registers GP0 and GP1 and read the GPU status/control register GP1
void writeGP0(const uint32_t data) noexcept;
void writeGP1(const uint32_t data) noexcept;
uint32_t getGpuStat() noexcept;

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
class Interrupt;
struct System;

namespace mips              { struct CPU;   }
namespace gpu               { class GPU;    }
namespace spu               { struct SPU;   }
namespace device::cdrom     { class CDROM;  }

//------------------------------------------------------------------------------------------------------------------------------------------
// VM control: setup and control the VM
//------------------------------------------------------------------------------------------------------------------------------------------
namespace PsxVm {
    // Access to the emulated PSX, CPU, GPU etc.
    extern System*                  gpSystem;
    extern mips::CPU*               gpCpu;
    extern gpu::GPU*                gpGpu;
    extern spu::SPU*                gpSpu;
    extern device::cdrom::CDROM*    gpCdrom;
    extern Interrupt*               gpInterrupt;
    extern uint8_t*                 gpRam;
    extern uint8_t*                 gpScratchpad;       // Cache used as fast RAM (1 KiB)

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
            const uint32_t wrappedAddr = (addr & 0x1FFFFF);
            ASSERT_LOG(wrappedAddr + sizeof(T) <= 0x200000, "Address pointed to spills past the 2 MiB of PSX RAM!");
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
            const uint32_t wrappedAddr = (addr & 0x1FFFFF);
            return reinterpret_cast<void*>(PsxVm::gpRam + wrappedAddr);
        }
    } else {
        return nullptr;
    }
}
