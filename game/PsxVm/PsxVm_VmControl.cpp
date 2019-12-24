//------------------------------------------------------------------------------------------------------------------------------------------
// VM controller functions.
// Setup and control the Avocado emulator and our emulation layers.
//------------------------------------------------------------------------------------------------------------------------------------------

#define PSX_VM_NO_REGISTER_MACROS 1     // Because they cause conflicts with Avocado
#include "PsxVm.h"

#include "PcPsx/FatalErrors.h"
#include <map>

// Disabling certain Avocado warnings for MSVC
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable: 4201)
    #pragma warning(disable: 4244)
#endif

#include "PcPsx/Macros.h"
#include <disc/format/cue.h>
#include <disc/load.h>
#include <input/input_manager.h>
#include <memory>
#include <SDL.h>
#include <sound/sound.h>
#include <system.h>

#ifdef _MSC_VER
    #pragma warning(pop)
#endif

using namespace PsxVm;

static std::unique_ptr<System>          gSystem;
static std::unique_ptr<InputManager>    gInputMgr;

System*                 PsxVm::gpSystem;
mips::CPU*              PsxVm::gpCpu;
gpu::GPU*               PsxVm::gpGpu;
spu::SPU*               PsxVm::gpSpu;
device::cdrom::CDROM*   PsxVm::gpCdrom;
Interrupt*              PsxVm::gpInterrupt;
uint8_t*                PsxVm::gpRam;

namespace PsxVm {
    // Address to function lookup for the VM
    extern std::map<uint32_t, VmFunc> gFuncTable;
}

// Keys for input handling
const std::string gPadBtnKey_Up = "controller/1/dpad_up";
const std::string gPadBtnKey_Down = "controller/1/dpad_down";
const std::string gPadBtnKey_Left = "controller/1/dpad_left";
const std::string gPadBtnKey_Right = "controller/1/dpad_right";
const std::string gPadBtnKey_Triangle = "controller/1/triangle";
const std::string gPadBtnKey_Circle = "controller/1/circle";
const std::string gPadBtnKey_Cross = "controller/1/cross";
const std::string gPadBtnKey_Square = "controller/1/square";
const std::string gPadBtnKey_Start = "controller/1/start";
const std::string gPadBtnKey_Select = "controller/1/select";
const std::string gPadBtnKey_L2 = "controller/1/l2";
const std::string gPadBtnKey_R2 = "controller/1/r2";
const std::string gPadBtnKey_L1 = "controller/1/l1";
const std::string gPadBtnKey_R1 = "controller/1/r1";

//------------------------------------------------------------------------------------------------------------------------------------------
// Setup and clear pointers for the VM environment
//------------------------------------------------------------------------------------------------------------------------------------------
static void setupVmPointers() noexcept {
    // System devices
    gpSystem = gSystem.get();
    gpCpu = gpSystem->cpu.get();
    gpGpu = gpSystem->gpu.get();
    gpSpu = gpSystem->spu.get();
    gpCdrom = gpSystem->cdrom.get();
    gpInterrupt = gpSystem->interrupt.get();
    gpRam = gpSystem->ram.data();

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

    gpRam = nullptr;
    gpInterrupt = nullptr;
    gpCdrom = nullptr;
    gpSpu = nullptr;
    gpGpu = nullptr;
    gpCpu = nullptr;
    gpSystem = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Run the bios bootstrap code until it reaches the point where it's able to display something.
// This needs to be done in order to correctly configure the emulated PlayStation.
//------------------------------------------------------------------------------------------------------------------------------------------
static void emulateBiosUntilShell() noexcept {
    // Set a breakpoint on the BIOS executing up until the shell and emulate until then
    mips::CPU& cpu = *gpCpu;
    System& system = *gpSystem;

    cpu.breakpoints.emplace(0x80030000, mips::CPU::Breakpoint(true));
    system.debugOutput = false;

    while (system.state == System::State::run) {
        system.emulateFrame();
    }

    cpu.breakpoints.clear();
    system.debugOutput = true;
    system.state = System::State::run;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Patch an 'exit' point for the PlayStation emulator in RAM.
// This exit point consists of a jump instruction that jumps to itself, followed by a NOP.
// We patch this at the entry point for NTSC DOOM.
//------------------------------------------------------------------------------------------------------------------------------------------
static void createEmulatorExitPointPatch() noexcept {
    const uint32_t patchAddr1 = 0x00050714;     // 80050714h mapped to the 2MB of RAM that the PSX has
    const uint32_t patchAddr2 = 0x00050718;     // 80050718h mapped to the 2MB of RAM that the PSX has
    const uint32_t inst1 = 0x080141C5;          // j 80050714
    const uint32_t inst2 = 0x00000000;          // sll $zero $zero 0 (a.k.a NOP)

    uint8_t* const pRam = gpSystem->ram.data();
    std::memcpy(pRam + patchAddr1, &inst1, sizeof(uint32_t));
    std::memcpy(pRam + patchAddr2, &inst2, sizeof(uint32_t));
}

bool PsxVm::init(
    const char* const biosFilePath,
    const char* const doomExePath,
    const char* const doomCdCuePath
) noexcept {
    // Create a new system and setup vm pointers 
    gSystem.reset(new System());
    System& system = *gSystem;
    setupVmPointers();
    
    // Open the DOOM cdrom
    system.cdrom->disc = disc::load(doomCdCuePath);
    system.cdrom->setShell(false);

    if (!system.cdrom->disc) {
        FATAL_ERROR_F(
            "Couldn't open the .cue disc descriptor '%s'!\n"
            "Is it present in the current working directory?\n"
            "This *MUST* be the .cue file for PSX Doom NTSC-U (SLUS-00077).",
            doomCdCuePath
        );

        shutdown();
        return false;
    }

    // Load the bios file and emulate the bios until the shell
    if (!system.loadBios(biosFilePath)) {
        FATAL_ERROR_F(
            "Couldn't load the NTSC-U PSX bios file '%s'!\n"
            "Is it present in the current working directory?",
            biosFilePath
        );

        shutdown();
        return false;
    }

    emulateBiosUntilShell();

    // Load the DOOM exe and patch memory to create an emulator 'exit point' where we can return control to native code from
    disc::Data data = getFileContents(doomExePath);

    if (!system.loadExeFile(data)) {
        FATAL_ERROR_F(
            "Couldn't load the PSX Doom .exe file '%s'!\n"
            "Is it present in the current working directory?\n"
            "This *MUST* be the .exe file for PSX Doom NTSC-U (SLUS-00077).",
            doomExePath
        );

        shutdown();
        return false;
    }

    createEmulatorExitPointPatch();

    // Point the emulator at the exit point
    system.cpu->setReg(31, 0x80050714);
    system.cpu->setPC(0x80050714);

    // Expect emulator to be in a state where we can exit it
    ASSERT(canExitEmulator());

    // Setup input manager
    gInputMgr.reset(new InputManager());
    InputManager::setInstance(gInputMgr.get());

    // Setup sound
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    Sound::init();
    Sound::play();
    return true;
}

void PsxVm::shutdown() noexcept {
    Sound::close();
    InputManager::setInstance(nullptr);
    gInputMgr.reset();
    clearVmPointers();
    gSystem.reset();
}

void PsxVm::updateInput() noexcept {
    // TODO: support configurable input
    // TODO: support game controller
    ASSERT(InputManager::getInstance());
    InputManager& inputMgr = *InputManager::getInstance();
    const uint8_t* const pKbState = SDL_GetKeyboardState(nullptr);

    inputMgr.setState(gPadBtnKey_Up, InputManager::AnalogValue(pKbState[SDL_SCANCODE_UP] != 0));
    inputMgr.setState(gPadBtnKey_Down, InputManager::AnalogValue(pKbState[SDL_SCANCODE_DOWN] != 0));
    inputMgr.setState(gPadBtnKey_Left, InputManager::AnalogValue(pKbState[SDL_SCANCODE_LEFT] != 0));
    inputMgr.setState(gPadBtnKey_Right, InputManager::AnalogValue(pKbState[SDL_SCANCODE_RIGHT] != 0));

    inputMgr.setState(gPadBtnKey_L1, InputManager::AnalogValue(pKbState[SDL_SCANCODE_A] != 0));
    inputMgr.setState(gPadBtnKey_R1, InputManager::AnalogValue(pKbState[SDL_SCANCODE_D] != 0));
    inputMgr.setState(gPadBtnKey_L2, InputManager::AnalogValue(pKbState[SDL_SCANCODE_PAGEUP] != 0 || pKbState[SDL_SCANCODE_Q] != 0));
    inputMgr.setState(gPadBtnKey_R2, InputManager::AnalogValue(pKbState[SDL_SCANCODE_PAGEDOWN] != 0|| pKbState[SDL_SCANCODE_E] != 0));

    inputMgr.setState(gPadBtnKey_Circle, InputManager::AnalogValue(pKbState[SDL_SCANCODE_SPACE] != 0));
    inputMgr.setState(gPadBtnKey_Square, InputManager::AnalogValue(pKbState[SDL_SCANCODE_LSHIFT] != 0 || pKbState[SDL_SCANCODE_RSHIFT]));
    inputMgr.setState(gPadBtnKey_Triangle, InputManager::AnalogValue(pKbState[SDL_SCANCODE_LCTRL] != 0 || pKbState[SDL_SCANCODE_RCTRL]));
    inputMgr.setState(gPadBtnKey_Cross, InputManager::AnalogValue(pKbState[SDL_SCANCODE_LALT] != 0 || pKbState[SDL_SCANCODE_RALT]));

    inputMgr.setState(gPadBtnKey_Start, InputManager::AnalogValue(pKbState[SDL_SCANCODE_RETURN] != 0));
    inputMgr.setState(gPadBtnKey_Select, InputManager::AnalogValue(pKbState[SDL_SCANCODE_TAB] != 0));

    gpSystem->controller->update();
}

VmFunc PsxVm::getVmFuncForAddr(const uint32_t addr) noexcept {
    auto iter = gFuncTable.find(addr);
    return (iter != gFuncTable.end()) ? iter->second : nullptr;
}

bool PsxVm::isEmulatorAtExitPoint() noexcept {
    // The instructions at '0x80050714' and '0x80050718' are the exit point instructions
    mips::CPU& cpu = *gpCpu;
    const uint32_t curPC = cpu.PC;
    const uint32_t nextPC = cpu.nextPC;
    return (
        (curPC == 0x80050714 || curPC == 0x80050718) &&
        (nextPC == 0x80050714 || nextPC == 0x80050718)
    );
}

bool PsxVm::canExitEmulator() noexcept {
    // Only allow exit if we are the emulator exit point
    if (!isEmulatorAtExitPoint())
        return false;
    
    // There must be no interrupts pending also in order to exit
    mips::CPU& cpu = *gpCpu;

    if (cpu.cop0.status.interruptEnable) {
        if ((cpu.cop0.cause.interruptPending & cpu.cop0.status.interruptMask) != 0) {
            return false;
        }
    }

    if (gpSystem->interrupt->interruptPending())
        return false;

    // If we get to here then we can exit the emulator back to native code
    return true;
}
