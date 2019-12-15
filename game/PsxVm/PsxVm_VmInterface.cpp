//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface functions.
// Functionality which helps us emulate the original hardware environment of the program.
//------------------------------------------------------------------------------------------------------------------------------------------

#define PSX_VM_NO_REGISTER_MACROS 1     // Because they cause conflicts with Avocado
#include "PsxVm.h"

#include <cstdlib>

// Disabling certain Avocado warnings for MSVC
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4201)
    #pragma warning(disable: 4244)
#endif

#include <cpu/instructions.h>
#include <system.h>

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

using namespace PsxVm;

namespace PsxVm {
    const uint32_t* gpReg_zero;
    uint32_t* gpReg_at;
    uint32_t* gpReg_v0;
    uint32_t* gpReg_v1;
    uint32_t* gpReg_a0;
    uint32_t* gpReg_a1;
    uint32_t* gpReg_a2;
    uint32_t* gpReg_a3;
    uint32_t* gpReg_t0;
    uint32_t* gpReg_t1;
    uint32_t* gpReg_t2;
    uint32_t* gpReg_t3;
    uint32_t* gpReg_t4;
    uint32_t* gpReg_t5;
    uint32_t* gpReg_t6;
    uint32_t* gpReg_t7;
    uint32_t* gpReg_s0;
    uint32_t* gpReg_s1;
    uint32_t* gpReg_s2;
    uint32_t* gpReg_s3;
    uint32_t* gpReg_s4;
    uint32_t* gpReg_s5;
    uint32_t* gpReg_s6;
    uint32_t* gpReg_s7;
    uint32_t* gpReg_t8;
    uint32_t* gpReg_t9;
    uint32_t* gpReg_k0;
    uint32_t* gpReg_k1;
    uint32_t* gpReg_gp;
    uint32_t* gpReg_sp;
    uint32_t* gpReg_fp;
    uint32_t* gpReg_ra;
    uint32_t* gpReg_hi;
    uint32_t* gpReg_lo;
}

// Used to save and restore the value of all registers upon making an emulation call
static uint32_t gSavedMipsRegs[32];
static uint32_t gSavedHiReg;
static uint32_t gSavedLoReg;

static void notImplementedError() noexcept {
    // TODO: add failure message
    std::abort();
}

void tge(
    [[maybe_unused]] const uint32_t r1,
    [[maybe_unused]] const uint32_t r2,
    [[maybe_unused]] const uint16_t i
) noexcept {
    // Shouldn't ever be invoking this instruction!
    // TODO: add failure message
    std::abort();
}

uint32_t add(const uint32_t r1, const uint32_t r2) noexcept {
    // Note: not detecting/handling overflow and generating hardware exceptions.
    // Can add that later if it's required for correct behavior, but I don't think it will be.
    return r1 + r2;
}

uint32_t addi(const uint32_t r1, const int16_t i) noexcept {
    // Note: not detecting/handling overflow and generating hardware exceptions.
    // Can add that later if it's required for correct behavior, but I don't think it will be.
    const uint32_t i32 = (uint32_t)(int32_t) i;
    return r1 + i32;
}

void div(const uint32_t r1, const uint32_t r2) noexcept {
    // Note: MIPS handles division by zero slightly differently to X86.
    // This code is taken from Avocado's division routine.
    const int32_t a = (int32_t) r1;
    const int32_t b = (int32_t) r2;

    if (b == 0) {
        *gpReg_lo = (a < 0) ? 0x00000001 : 0xffffffff;
        *gpReg_hi = a;
    } else if (r1 == 0x80000000 && r2 == 0xffffffff) {
        *gpReg_lo = 0x80000000;
        *gpReg_hi = 0x00000000;
    } else {
        *gpReg_lo = (uint32_t)(a / b);
        *gpReg_hi = (uint32_t)(a % b);
    }
}

void divu(const uint32_t r1, const uint32_t r2) noexcept {
    // Note: MIPS handles division by zero slightly differently to X86
    if (r2 != 0) {
        *gpReg_lo = r1 / r2;
        *gpReg_hi = r1 % r2;
    } else {
        *gpReg_lo = 0xFFFFFFFF;
        *gpReg_hi = r1;
    }
}

void mult(const uint32_t r1, const uint32_t r2) noexcept {
    const int64_t a = (int32_t) r1;
    const int64_t b = (int32_t) r2;
    const uint64_t result = (uint64_t)(a * b);
    *gpReg_lo = result & 0xFFFFFFFF;
    *gpReg_hi = result >> 32;
}

void multu(const uint32_t r1, const uint32_t r2) noexcept {
    const uint64_t result = (uint64_t) r1 * (uint64_t) r2;
    *gpReg_lo = result & 0xFFFFFFFF;
    *gpReg_hi = result >> 32;
}

uint32_t sub(const uint32_t r1, const uint32_t r2) noexcept {
    // Note: not detecting/handling overflow and generating hardware exceptions.
    // Can add that later if it's required for correct behavior, but I don't think it will be.
    return r1 - r2;
}

uint32_t lb(const uint32_t addr) noexcept {
    return (uint32_t)(int32_t)(int8_t) gpCpu->sys->readMemory8(addr);
}

uint32_t lbu(const uint32_t addr) noexcept {
    return gpCpu->sys->readMemory8(addr);
}

uint32_t lh(const uint32_t addr) noexcept {
    return (uint32_t)(int32_t)(int16_t) gpCpu->sys->readMemory16(addr);
}

uint32_t lhu(const uint32_t addr) noexcept {
    return gpCpu->sys->readMemory16(addr);
}

uint32_t lw(const uint32_t addr) noexcept {
    return gpCpu->sys->readMemory32(addr);
}

uint32_t lwl(const uint32_t r1, const uint32_t addr) noexcept {
    // This code is largely based on Avocado's implementation of the same instruction
    const uint32_t mem = gpCpu->sys->readMemory32(addr & 0xFFFFFFFC);
    uint32_t result;

    switch (addr % 4) {
        case 0:     result = (r1 & 0x00FFFFFF) | (mem << 24);   break;
        case 1:     result = (r1 & 0x0000FFFF) | (mem << 16);   break;
        case 2:     result = (r1 & 0x000000FF) | (mem << 8);    break;
        default:    result = (r1 & 0x00000000) | (mem);         break;
    }

    return result;
}

uint32_t lwr(const uint32_t r1, const uint32_t addr) noexcept {
    // This code is largely based on Avocado's implementation of the same instruction
    const uint32_t mem = gpCpu->sys->readMemory32(addr & 0xFFFFFFFC);
    uint32_t result;

    switch (addr % 4) {
        case 0:     result = (r1 & 0x00000000) | (mem);         break;
        case 1:     result = (r1 & 0xFF000000) | (mem >> 8);    break;
        case 2:     result = (r1 & 0xFFFF0000) | (mem >> 16);   break;
        default:    result = (r1 & 0xFFFFFF00) | (mem >> 24);   break;
    }
    
    return result;
}

void sb(const uint32_t r1, const uint32_t addr) noexcept {
    gpCpu->sys->writeMemory8(addr, (uint8_t) r1);
}

void sh(const uint32_t r1, const uint32_t addr) noexcept {
    gpCpu->sys->writeMemory16(addr, (uint16_t) r1);
}

void sw(const uint32_t r1, const uint32_t addr) noexcept {
    gpCpu->sys->writeMemory32(addr, r1);
}

void swl(const uint32_t r1, const uint32_t addr) noexcept {
    // This code is largely based on Avocado's implementation of the same instruction
    System& sys = *gpCpu->sys;
    const uint32_t alignedAddr = addr & 0xFFFFFFFC;
    const uint32_t mem = sys.readMemory32(alignedAddr);
    uint32_t result;

    switch (addr % 4) {
        case 0:     result = (mem & 0xFFFFFF00) | (r1 >> 24);   break;
        case 1:     result = (mem & 0xFFFF0000) | (r1 >> 16);   break;
        case 2:     result = (mem & 0xFF000000) | (r1 >> 8);    break;
        default:    result = (mem & 0x00000000) | (r1);         break;
    }

    sys.writeMemory32(alignedAddr, result);
}

void swr(const uint32_t r1, const uint32_t addr) noexcept {
    // This code is largely based on Avocado's implementation of the same instruction
    System& sys = *gpCpu->sys;
    const uint32_t alignedAddr = addr & 0xFFFFFFFC;    
    const uint32_t mem = sys.readMemory32(alignedAddr);
    uint32_t result;

    switch (addr % 4) {
        case 0:     result = (r1      ) | (mem & 0x00000000);   break;
        case 1:     result = (r1 << 8 ) | (mem & 0x000000FF);   break;
        case 2:     result = (r1 << 16) | (mem & 0x0000FFFF);   break;
        default:    result = (r1 << 24) | (mem & 0x00FFFFFF);   break;
    }

    sys.writeMemory32(alignedAddr, result);
}

uint32_t mfc0(const uint8_t s) noexcept {
    return gpCpu->cop0.read(s).first;
}

void mtc0(const uint32_t r1, const uint8_t d) noexcept {
    gpCpu->cop0.write(d, r1);
}

void cop2(const uint32_t i) noexcept {
    notImplementedError();
}

uint32_t cfc2(const uint8_t s) noexcept {
    return gpCpu->gte.read((int) s + 32);
}

void ctc2(const uint32_t r1, const uint8_t d) noexcept {
    gpCpu->gte.write((int) d + 32, r1);
}

uint32_t mfc2(const uint8_t s) noexcept {
    notImplementedError();
    return 0;
}

void mtc2(const uint32_t r1, const uint8_t d) noexcept {
    notImplementedError();
}

void lwc2(const uint8_t d, const uint32_t addr) noexcept {
    notImplementedError();
}

void swc2(const uint8_t s, const uint32_t addr) noexcept {
    notImplementedError();
}

void _break(const uint32_t i) noexcept {
    notImplementedError();
}

static void saveMipsRegisters() noexcept {
    mips::CPU& cpu = *gpCpu;
    std::memcpy(gSavedMipsRegs, cpu.reg, sizeof(uint32_t) * 32);
    gSavedHiReg = cpu.hi;
    gSavedLoReg = cpu.lo;
}

static void restoreMipsRegisters() noexcept {
    mips::CPU& cpu = *gpCpu;
    cpu.hi = gSavedHiReg;
    cpu.lo = gSavedLoReg;
    std::memcpy(cpu.reg, gSavedMipsRegs, sizeof(uint32_t) * 32);

    // Make sure no load delay stuff and that zero is zero
    cpu.reg[0] = 0;
    cpu.slots[0].reg = DUMMY_REG;
    cpu.slots[1].reg = DUMMY_REG;
}

static void restoreMipsRegistersExceptReturnRegs() noexcept {
    mips::CPU& cpu = *gpCpu;
    const uint32_t retReg1 = cpu.reg[2];
    const uint32_t retReg2 = cpu.reg[3];
    
    restoreMipsRegisters();

    cpu.reg[2] = retReg1;
    cpu.reg[3] = retReg2;    
}

void syscall(const uint32_t i) noexcept {
    saveMipsRegisters();
    mips::CPU& cpu = *gpCpu;
    cpu.setReg(4, i);

    assert(cpu.PC == 0x80050714 || cpu.PC == 0x80050718);
    instructions::op_syscall(&cpu, {});
    
    while (!canExitEmulator()) {
        gpSystem->emulateFrame();
    }

    restoreMipsRegistersExceptReturnRegs();
}

void ptr_call(const uint32_t addr) noexcept {
    // Check for a bios function call
    const uint32_t maskedAdr = addr & 0x1FFFFFFF;
    
    if (maskedAdr == 0xa0 || maskedAdr == 0xb0 || maskedAdr == 0xc0) {
        emu_call(addr);
    } else {
        // Regular function pointer call
        const VmFunc func = PsxVm::getVmFuncForAddr(addr);

        if (func) {
            func();
        } else {
            abort();    // TODO: add failure message
        }
    }
}

static void setupForEmulatorCall() {
    // Set the 'RA' register to return to our 'emulator exit' point.
    // I've modified Avocado to stop emulation and hand back control to the C++ code when this address is reached.
    // This is the entrypoint for PSXDOOM.EXE.
    gpCpu->setReg(31, 0x80050714);

    // Clear load delay and branch related stuff
    gpCpu->slots[0].reg = DUMMY_REG;
    gpCpu->slots[1].reg = DUMMY_REG;
    gpCpu->branchTaken = true;
    gpCpu->inBranchDelay = false;
}

void emu_call(const uint32_t func) noexcept {
    mips::CPU& cpu = *gpCpu;
    System& system = *gpSystem;

    saveMipsRegisters();
    setupForEmulatorCall();    
    cpu.setPC(func);

    while (true) {
        system.emulateFrame();

        if (canExitEmulator())
            break;
    }

    restoreMipsRegistersExceptReturnRegs();
}

void jump_table_err() noexcept {
    std::abort();
}

void emulate_frame() noexcept {
    saveMipsRegisters();
    gpSystem->vblankCounter = 0;

    while (gpSystem->vblankCounter < 1) {
        gpSystem->emulateFrame();
    }

    restoreMipsRegisters();
}

void emulate_a_little() noexcept {
    saveMipsRegisters();
    gpSystem->emulateFrame();
    restoreMipsRegisters();
}

void emulate_gpu(const int numCycles) noexcept {
    if (gpSystem->gpu->emulateGpuCycles((int) numCycles)) {
        gpSystem->interrupt->trigger(interrupt::VBLANK);
    }
}

void emulate_cdrom() noexcept {
    System& system = *gpSystem;

    // This speeds up things a bit
    for (uint32_t i = 0; i < 16; ++i) {
        system.cdrom->step();        
    }

    system.interrupt->step();
    emulate_a_little();
}
