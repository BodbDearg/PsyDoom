//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface functions.
// Functionality which helps us emulate the original hardware environment of the program.
//------------------------------------------------------------------------------------------------------------------------------------------

#define PSX_VM_NO_REGISTER_MACROS 1     // Because they cause conflicts with Avocado
#include "PsxVm.h"

#include "PcPsx/Macros.h"
#include "PcPsx/ProgArgs.h"

BEGIN_THIRD_PARTY_INCLUDES
    #include <cpu/instructions.h>
    #include <cstdlib>
    #include <sound/sound.h>
    #include <system.h>
END_THIRD_PARTY_INCLUDES

// External function required from LIBCD.
// This is a slight layering violation (this module should not know app code) but is required for correct functionality.
// This steps the cdrom drive and invokes any data callbacks that would be invoked by new sectors being available.
void stepCdromWithCallbacks() noexcept;

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

void sb(const uint32_t r1, const uint32_t addr) noexcept {
    gpCpu->sys->writeMemory8(addr, (uint8_t) r1);
}

void sh(const uint32_t r1, const uint32_t addr) noexcept {
    gpCpu->sys->writeMemory16(addr, (uint16_t) r1);
}

void sw(const uint32_t r1, const uint32_t addr) noexcept {
    gpCpu->sys->writeMemory32(addr, r1);
}

void writeGP0(const uint32_t data) noexcept {
    gpGpu->writeGP0(data);
}

void writeGP1(const uint32_t data) noexcept {
    gpGpu->writeGP1(data);
}

uint32_t getGpuStat() noexcept {
    return gpGpu->getStat();
}

void emulate_sound_if_required() noexcept {
    // Do no sound emulation in headless mode
    if (ProgArgs::gbHeadlessMode)
        return;

    // Update timer events while we are at it - that might affect the sound sequencer
    generate_timer_events();

    while (true) {
        size_t soundBufferSize;

        {
            std::unique_lock<std::mutex> lock(Sound::audioMutex);
            soundBufferSize = Sound::buffer.size();
        }

        // FIXME: temp hack - fill the sound buffers up a decent amount to prevent skipping
        if (soundBufferSize >= 1024 * 2)
            return;
        
        spu::SPU& spu = *gpSpu;
        device::cdrom::CDROM& cdrom = *gpCdrom;

        while (!spu.bufferReady) {
            // Read more CD data if we are playing music
            if (cdrom.stat.play && cdrom.audio.empty()) {
                cdrom.bForceSectorRead = true;  // Force an immediate read on the next step
                stepCdromWithCallbacks();
            }

            // Step the spu to emulate it's functionality
            spu.step(&cdrom);
        }

        if (spu.bufferReady) {
            spu.bufferReady = false;
            Sound::appendBuffer(spu.audioBuffer.begin(), spu.audioBuffer.end());
        }
    }
}

uint32_t ptrToVmAddr(const void* const ptr) noexcept {
    // Null is allowed to convert back to '0' always
    if (ptr) {
        const intptr_t offsetToRam = (const uint8_t*) ptr - gpRam;

        // Note: allow a pointer at the end of the 2 MiB RAM region, but no more
        if (offsetToRam >= 0 && offsetToRam <= 0x200000) {
            return 0x80000000 + (uint32_t) offsetToRam;
        } else {
            // Check for a scratchpad memory address
            const intptr_t offsetToScratchpad = (const uint8_t*) ptr - gpScratchpad;

            if (offsetToScratchpad >= 0 && offsetToScratchpad <= 0x400) {
                return 0x1F800000 + (uint32_t) offsetToScratchpad;
            } else {
                FATAL_ERROR("ptrToVmAddr: pointer does not point to an area inside PSX RAM or the scratchpad!");
            }
        }
    } else {
        return 0;
    }
}
