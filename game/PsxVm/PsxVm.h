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

// These are optional, since they can conflict with Avocado and other things due to short names
#if (!PSX_VM_NO_REGISTER_MACROS)
    #define zero    (*PsxVm::gpReg_zero)
    #define at      (*PsxVm::gpReg_at)
    #define v0      (*PsxVm::gpReg_v0)
    #define v1      (*PsxVm::gpReg_v1)
    #define a0      (*PsxVm::gpReg_a0)
    #define a1      (*PsxVm::gpReg_a1)
    #define a2      (*PsxVm::gpReg_a2)
    #define a3      (*PsxVm::gpReg_a3)
    #define t0      (*PsxVm::gpReg_t0)
    #define t1      (*PsxVm::gpReg_t1)
    #define t2      (*PsxVm::gpReg_t2)
    #define t3      (*PsxVm::gpReg_t3)
    #define t4      (*PsxVm::gpReg_t4)
    #define t5      (*PsxVm::gpReg_t5)
    #define t6      (*PsxVm::gpReg_t6)
    #define t7      (*PsxVm::gpReg_t7)
    #define s0      (*PsxVm::gpReg_s0)
    #define s1      (*PsxVm::gpReg_s1)
    #define s2      (*PsxVm::gpReg_s2)
    #define s3      (*PsxVm::gpReg_s3)
    #define s4      (*PsxVm::gpReg_s4)
    #define s5      (*PsxVm::gpReg_s5)
    #define s6      (*PsxVm::gpReg_s6)
    #define s7      (*PsxVm::gpReg_s7)
    #define t8      (*PsxVm::gpReg_t8)
    #define t9      (*PsxVm::gpReg_t9)
    #define k0      (*PsxVm::gpReg_k0)
    #define k1      (*PsxVm::gpReg_k1)
    #define gp      (*PsxVm::gpReg_gp)
    #define sp      (*PsxVm::gpReg_sp)
    #define fp      (*PsxVm::gpReg_fp)
    #define ra      (*PsxVm::gpReg_ra)
    #define hi      (*PsxVm::gpReg_hi)
    #define lo      (*PsxVm::gpReg_lo)
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface: mips instructions.
// These instructions were not so easy to convert directly to C++ so they are handled via functions.
//------------------------------------------------------------------------------------------------------------------------------------------

// Trap instructions
void tge(const uint32_t r1, const uint32_t r2, const uint16_t i) noexcept;

// Arithmetic
uint32_t add(const uint32_t r1, const uint32_t r2) noexcept;
uint32_t addi(const uint32_t r1, const int16_t i) noexcept;
void div(const uint32_t r1, const uint32_t r2) noexcept;
void divu(const uint32_t r1, const uint32_t r2) noexcept;
void mult(const uint32_t r1, const uint32_t r2) noexcept;
void multu(const uint32_t r1, const uint32_t r2) noexcept;
uint32_t sub(const uint32_t r1, const uint32_t r2) noexcept;

// RAM to CPU loads
uint32_t lb(const uint32_t addr) noexcept;
uint32_t lbu(const uint32_t addr) noexcept;
uint32_t lh(const uint32_t addr) noexcept;
uint32_t lhu(const uint32_t addr) noexcept;
uint32_t lw(const uint32_t addr) noexcept;
uint32_t lwl(const uint32_t r1, const uint32_t addr) noexcept;
uint32_t lwr(const uint32_t r1, const uint32_t addr) noexcept;

// CPU to RAM stores
void sb(const uint32_t r1, const uint32_t addr) noexcept;
void sh(const uint32_t r1, const uint32_t addr) noexcept;
void sw(const uint32_t r1, const uint32_t addr) noexcept;
void swl(const uint32_t r1, const uint32_t addr) noexcept;
void swr(const uint32_t r1, const uint32_t addr) noexcept;

// Coprocessor 0 instructions
uint32_t mfc0(const uint8_t s) noexcept;
void mtc0(const uint32_t r1, const uint8_t d) noexcept;

// Coprocessor 2 (GTE) instructions
void cop2(const uint32_t i) noexcept;
uint32_t cfc2(const uint8_t s) noexcept;
void ctc2(const uint32_t r1, const uint8_t d) noexcept;
uint32_t mfc2(const uint8_t s) noexcept;
void mtc2(const uint32_t r1, const uint8_t d) noexcept;
void lwc2(const uint8_t d, const uint32_t addr) noexcept;
void swc2(const uint8_t s, const uint32_t addr) noexcept;

// Misc
void _break(const uint32_t i) noexcept;
void syscall(const uint32_t i) noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface: function calls and utilities
// Call a function pointer or make a bios call.
//------------------------------------------------------------------------------------------------------------------------------------------

// Call a pointer to a function at the given address.
// The function passes args and return values through the VM interface.
// The call *MAY* be emulated via the PSX emulator.
void ptr_call(const uint32_t addr) noexcept;

// Call function via the emulator.
// This will transfer control over to the emulator and not return until the called function is exited.
void emu_call(const uint32_t func) noexcept;

// Called when the code is trying to jump to an unexpected location for a jump table.
// If this happens then something has seriously gone wrong.
void jump_table_err() noexcept;

// Emulate a frame in the PSX emulator.
// This causes time to pass, interupts to be generated etc.
void emulate_frame() noexcept;

// Do a little bit of emulation in the PSX emulator (not a whole frame)
void emulate_a_little() noexcept;

// Emulate the CD-ROM, interrupts generated by the CD-ROM and then a little system emulation.
// Intended to help move along the process of reading from CD.
// The CD-ROM is advanced in its emulation a lot by this call.
void emulate_cdrom() noexcept;

// Emulate the GPU (only!) by a specified number of cycles.
// Useful to speed up generating a new vblank.
void emulate_gpu(const int numCycles) noexcept;

// Emulate timers only by a specified number of cycles
void emulate_timers(const int numCycles) noexcept;

// Emulate sound until we have enough samples to handle an upcoming buffer request
void emulate_sound_if_required() noexcept;

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

    // Initialize the VM with the given bios, original Playstation DOOM .EXE and CD file path (.cue format)
    bool init(
        const char* const biosFilePath,
        const char* const doomExePath,
        const char* const doomCdCuePath
    ) noexcept;

    void shutdown() noexcept;

    // Updates inputs to the emulator from real inputs on the host machine
    void updateInput() noexcept;

    // Lookup the function pointer to call for a given address
    VmFunc getVmFuncForAddr(const uint32_t addr) noexcept;

    // Is the emulator at the point in the program where it returns control to C++?
    bool isEmulatorAtExitPoint() noexcept;

    // Tells if the emulator can return control back to the native C++ code
    bool canExitEmulator() noexcept;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert a VM address to a pointer
//------------------------------------------------------------------------------------------------------------------------------------------
template <class T>
inline T* vmAddrToPtr(const uint32_t addr) noexcept {
    if (addr != 0) {
        const uint32_t wrappedAddr = (addr & 0x1FFFFF);
        ASSERT_LOG(wrappedAddr + sizeof(T) <= 0x200000, "Address pointed to spills past the 2MB of PSX RAM!");
        return reinterpret_cast<T*>(PsxVm::gpRam + wrappedAddr);
    } else {
        return nullptr;
    }
}

template <>
inline void* vmAddrToPtr(const uint32_t addr) noexcept {
    if (addr != 0) {
        const uint32_t wrappedAddr = (addr & 0x1FFFFF);
        return reinterpret_cast<void*>(PsxVm::gpRam + wrappedAddr);
    } else {
        return nullptr;
    }
}
