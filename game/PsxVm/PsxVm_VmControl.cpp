//------------------------------------------------------------------------------------------------------------------------------------------
// VM controller functions.
// Setup and control the Avocado emulator and our emulation layers.
//------------------------------------------------------------------------------------------------------------------------------------------

#define PSX_VM_NO_REGISTER_MACROS 1     // Because they cause conflicts with Avocado
#include "PsxVm.h"

// Disabling certain Avocado warnings for MSVC
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4201)
    #pragma warning(disable: 4244)
#endif

#include <disc/format/cue.h>
#include <memory>
#include <system.h>

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

static std::unique_ptr<System> gSystem;

System*                  PsxVm::gpSystem;
mips::CPU*               PsxVm::gpCpu;
gpu::GPU*                PsxVm::gpGpu;
spu::SPU*                PsxVm::gpSpu;
device::cdrom::CDROM*    PsxVm::gpCdrom;

namespace PsxVm {
    //--------------------------------------------------------------------------------------------------------------------------------------
    // Setup and clear pointers for the VM environment
    //--------------------------------------------------------------------------------------------------------------------------------------
    static void setupVmPointers() noexcept {
        // System devices
        gpSystem = gSystem.get();
        gpCpu = gpSystem->cpu.get();
        gpGpu = gpSystem->gpu.get();
        gpSpu = gpSystem->spu.get();
        gpCdrom = gpSystem->cdrom.get();

        // Mips registers
        gpReg_zero = &gpCpu->reg[0];
        gpReg_at = &gpCpu->reg[1];
        gpReg_v0 = &gpCpu->reg[2];
        gpReg_v1 = &gpCpu->reg[3];
        gpReg_a0 = &gpCpu->reg[4];
        gpReg_a1 = &gpCpu->reg[5];
        gpReg_a2 = &gpCpu->reg[6];
        gpReg_a3 = &gpCpu->reg[7];
        gpReg_t0 = &gpCpu->reg[8];
        gpReg_t1 = &gpCpu->reg[9];
        gpReg_t2 = &gpCpu->reg[10];
        gpReg_t3 = &gpCpu->reg[11];
        gpReg_t4 = &gpCpu->reg[12];
        gpReg_t5 = &gpCpu->reg[13];
        gpReg_t6 = &gpCpu->reg[14];
        gpReg_t7 = &gpCpu->reg[15];
        gpReg_s0 = &gpCpu->reg[16];
        gpReg_s1 = &gpCpu->reg[17];
        gpReg_s2 = &gpCpu->reg[18];
        gpReg_s3 = &gpCpu->reg[19];
        gpReg_s4 = &gpCpu->reg[20];
        gpReg_s5 = &gpCpu->reg[21];
        gpReg_s6 = &gpCpu->reg[22];
        gpReg_s7 = &gpCpu->reg[23];
        gpReg_t8 = &gpCpu->reg[24];
        gpReg_t9 = &gpCpu->reg[25];
        gpReg_k0 = &gpCpu->reg[26];
        gpReg_k1 = &gpCpu->reg[27];
        gpReg_gp = &gpCpu->reg[28];
        gpReg_sp = &gpCpu->reg[29];
        gpReg_fp = &gpCpu->reg[30];
        gpReg_ra = &gpCpu->reg[31];
        gpReg_hi = &gpCpu->hi;
        gpReg_lo = &gpCpu->lo;
    }

    static void clearVmPointers() noexcept {
        gpReg_lo = nullptr;
        gpReg_hi = nullptr;
        gpReg_ra = nullptr;
        gpReg_fp = nullptr;
        gpReg_sp = nullptr;
        gpReg_gp = nullptr;
        gpReg_k1 = nullptr;
        gpReg_k0 = nullptr;
        gpReg_t9 = nullptr;
        gpReg_t8 = nullptr;
        gpReg_s7 = nullptr;
        gpReg_s6 = nullptr;
        gpReg_s5 = nullptr;
        gpReg_s4 = nullptr;
        gpReg_s3 = nullptr;
        gpReg_s2 = nullptr;
        gpReg_s1 = nullptr;
        gpReg_s0 = nullptr;
        gpReg_t7 = nullptr;
        gpReg_t6 = nullptr;
        gpReg_t5 = nullptr;
        gpReg_t4 = nullptr;
        gpReg_t3 = nullptr;
        gpReg_t2 = nullptr;
        gpReg_t1 = nullptr;
        gpReg_t0 = nullptr;
        gpReg_a3 = nullptr;
        gpReg_a2 = nullptr;
        gpReg_a1 = nullptr;
        gpReg_a0 = nullptr;
        gpReg_v1 = nullptr;
        gpReg_v0 = nullptr;
        gpReg_at = nullptr;
        gpReg_zero = nullptr;

        gpCdrom = nullptr;
        gpSpu = nullptr;
        gpGpu = nullptr;
        gpCpu = nullptr;
        gpSystem = nullptr;
    }

    //--------------------------------------------------------------------------------------------------------------------------------------
    // Run the bios bootstrap code until it reaches the point where it's able to display something.
    // This needs to be done in order to correctly configure the Avocado VM.
    //--------------------------------------------------------------------------------------------------------------------------------------
    static void emulateBiosUntilShell() noexcept {
        // Set a breakpoint on the BIOS executing up until the shell and emulate until then
        gpCpu->breakpoints.emplace(0x80030000, mips::CPU::Breakpoint(true));
        gpSystem->debugOutput = false;

        while (gpSystem->state == System::State::run) {
            gpSystem->emulateFrame();
        }

        gpCpu->breakpoints.clear();
        gpSystem->debugOutput = true;
        gpSystem->state = System::State::run;
    }
}

bool PsxVm::init(
    const char* const biosFilePath,
    const char* const doomExePath,
    const char* const doomCdCuePath
) noexcept {
    // Create a new system and load the bios file
    gSystem.reset(new System());
    gSystem->cdrom->disc = disc::format::Cue::fromBin(doomCdCuePath);

    if (!gSystem->cdrom->disc) {
        shutdown();
        return false;
    }
    
    if (!gSystem->loadBios(biosFilePath)) {
        shutdown();
        return false;
    }

    // Setup pointers and emulate the bios until the shell
    setupVmPointers();
    emulateBiosUntilShell();

    // Load the DOOM exe
    disc::Data data = getFileContents(doomExePath);

    if (!gSystem->loadExeFile(data)) {
        shutdown();
        return false;
    }

    return true;
}

void PsxVm::shutdown() noexcept {
    clearVmPointers();
    gSystem.reset();
}
