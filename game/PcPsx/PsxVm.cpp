//------------------------------------------------------------------------------------------------------------------------------------------
// VM controller functions.
// Setup and control the Avocado emulator and our emulation layers.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "PsxVm.h"

#include "PcPsx/Assert.h"
#include "ProgArgs.h"

#include <SDL.h>

BEGIN_DISABLE_HEADER_WARNINGS
    #include <disc/format/cue.h>
    #include <disc/load.h>
    #include <input/input_manager.h>
    #include <sound/sound.h>
    #include <system.h>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(PsxVm)

System*                 gpSystem;
gpu::GPU*               gpGpu;
spu::SPU*               gpSpu;
device::cdrom::CDROM*   gpCdrom;

// TODO: this is a temp hack for gamepad support
static SDL_GameController*  gpGameController;
static SDL_Joystick*        gpJoystick;
static SDL_JoystickID       gJoystickId;

static std::unique_ptr<System>          gSystem;
static std::unique_ptr<InputManager>    gInputMgr;

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
// TODO: temp function to check for game controller connection/disconnection.
// Eventually should live elsewhere and be broken up into separate close / rescan functions.
//------------------------------------------------------------------------------------------------------------------------------------------
static void rescanGameControllers() noexcept {
    if (gpGameController) {
        if (!SDL_GameControllerGetAttached(gpGameController)) {
            // Gamepad disconnected
            SDL_GameControllerClose(gpGameController);
            gpGameController = nullptr;
            gpJoystick = nullptr;
            gJoystickId = {};
        }
    } else {
        // No gamepad: see if there are any joysticks connected.
        // Note: a return of < 0 means an error, which we will ignore:
        const int numJoysticks = SDL_NumJoysticks();

        for (int joyIdx = 0; joyIdx < numJoysticks; ++joyIdx) {
            // If we find a valid game controller then try to open it.
            // If we succeed then our work is done!
            if (SDL_IsGameController(joyIdx)) {
                gpGameController = SDL_GameControllerOpen(joyIdx);

                if (gpGameController) {
                    gpJoystick = SDL_GameControllerGetJoystick(gpGameController);
                    gJoystickId = SDL_JoystickInstanceID(gpJoystick);
                    break;
                }
            }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize emulated PlayStation system components and use the given .cue file for the game disc
//------------------------------------------------------------------------------------------------------------------------------------------
bool init(const char* const doomCdCuePath) noexcept {
    // Create a new system and setup vm pointers
    gSystem.reset(new System());

    System& system = *gSystem;
    gpSystem = gSystem.get();
    gpGpu = gpSystem->gpu.get();
    gpSpu = gpSystem->spu.get();
    gpCdrom = gpSystem->cdrom.get();

    // GPU: disable logging - this eats up TONS of memory!
    gpGpu->gpuLogEnabled = false;
    
    // Open the DOOM cdrom
    system.cdrom->disc = disc::load(doomCdCuePath);
    system.cdrom->setShell(false);

    if (!system.cdrom->disc) {
        FatalErrors::raiseF(
            "Couldn't open or failed to parse the .cue disc descriptor '%s'!\n"
            "Is it present at the specified path and is it valid? This *MUST* be the .cue file for PlayStation Doom (PAL or NTSC).",
            doomCdCuePath
        );

        shutdown();
        return false;
    }

    // Setup input manager
    gInputMgr.reset(new InputManager());
    InputManager::setInstance(gInputMgr.get());

    // Setup sound.
    // TODO: also setting up game controllers here - move elsewhere.
    SDL_InitSubSystem(SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);
    Sound::init();
    Sound::play();
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tear down emulated PlayStation components
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    Sound::close();
    InputManager::setInstance(nullptr);
    gInputMgr.reset();
    
    gpCdrom = nullptr;
    gpSpu = nullptr;
    gpGpu = nullptr;
    gpSystem = nullptr;
    gSystem.reset();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Updates inputs to the emulator from real inputs on the host machine
//------------------------------------------------------------------------------------------------------------------------------------------
void updateInput() noexcept {
    // TODO: support configurable input
    // TODO: support game controller
    const uint8_t* const pKbState = SDL_GetKeyboardState(nullptr);

    InputManager::AnalogValue btnUp = (pKbState[SDL_SCANCODE_UP] != 0);
    InputManager::AnalogValue btnDown = (pKbState[SDL_SCANCODE_DOWN] != 0);
    InputManager::AnalogValue btnLeft = (pKbState[SDL_SCANCODE_LEFT] != 0);
    InputManager::AnalogValue btnRight = (pKbState[SDL_SCANCODE_RIGHT] != 0);
    InputManager::AnalogValue btnL1 = (pKbState[SDL_SCANCODE_A] != 0);
    InputManager::AnalogValue btnR1 = (pKbState[SDL_SCANCODE_D] != 0);
    InputManager::AnalogValue btnL2 = ((pKbState[SDL_SCANCODE_PAGEDOWN] != 0) || (pKbState[SDL_SCANCODE_Q] != 0));
    InputManager::AnalogValue btnR2 = ((pKbState[SDL_SCANCODE_PAGEUP] != 0) || (pKbState[SDL_SCANCODE_E] != 0));
    InputManager::AnalogValue btnCircle = (pKbState[SDL_SCANCODE_SPACE] != 0);
    InputManager::AnalogValue btnSquare = ((pKbState[SDL_SCANCODE_LSHIFT] != 0) || (pKbState[SDL_SCANCODE_RSHIFT] != 0));
    InputManager::AnalogValue btnTriangle = ((pKbState[SDL_SCANCODE_LCTRL] != 0) || (pKbState[SDL_SCANCODE_RCTRL] != 0) || (pKbState[SDL_SCANCODE_F] != 0));
    InputManager::AnalogValue btnCross = ((pKbState[SDL_SCANCODE_LALT] != 0) || (pKbState[SDL_SCANCODE_RALT] != 0));
    InputManager::AnalogValue btnStart = (pKbState[SDL_SCANCODE_RETURN] != 0);
    InputManager::AnalogValue btnSelect = (pKbState[SDL_SCANCODE_TAB] != 0);

    // TODO: temp hack for gamepad support
    rescanGameControllers();

    if (gpGameController) {
        auto setValueIfPressed = [&](InputManager::AnalogValue& value, const SDL_GameControllerButton controllerBtn) noexcept {
            if (SDL_GameControllerGetButton(gpGameController, controllerBtn)) {
                value.value = UINT8_MAX;
            }
        };

        // TODO: use or remove
        #if 0
        auto setValueIfTrigPressed = [&](InputManager::AnalogValue& value, const SDL_GameControllerAxis axis) noexcept {
            if (SDL_GameControllerGetAxis(gpGameController, axis) >= 0.5f) {
                value.value = UINT8_MAX;
            }
        };
        #endif

        setValueIfPressed(btnUp, SDL_CONTROLLER_BUTTON_DPAD_UP);
        setValueIfPressed(btnDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        setValueIfPressed(btnLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        setValueIfPressed(btnRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        setValueIfPressed(btnL2, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
        setValueIfPressed(btnR2, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
        setValueIfPressed(btnTriangle, SDL_CONTROLLER_BUTTON_Y);
        setValueIfPressed(btnSquare, SDL_CONTROLLER_BUTTON_X);
        setValueIfPressed(btnCircle, SDL_CONTROLLER_BUTTON_B);
        setValueIfPressed(btnCross, SDL_CONTROLLER_BUTTON_A);
        setValueIfPressed(btnStart, SDL_CONTROLLER_BUTTON_START);
        setValueIfPressed(btnSelect, SDL_CONTROLLER_BUTTON_BACK);

        const int16_t DIGITAL_THRESHOLD = INT16_MAX / 2;

        if (SDL_GameControllerGetAxis(gpGameController, SDL_CONTROLLER_AXIS_LEFTX) >= DIGITAL_THRESHOLD) {
            btnR1.value = UINT8_MAX;
        } else if (SDL_GameControllerGetAxis(gpGameController, SDL_CONTROLLER_AXIS_LEFTX) <= -DIGITAL_THRESHOLD) {
            btnL1.value = UINT8_MAX;
        }

        if (SDL_GameControllerGetAxis(gpGameController, SDL_CONTROLLER_AXIS_LEFTY) >= DIGITAL_THRESHOLD) {
            btnDown.value = UINT8_MAX;
        } else if (SDL_GameControllerGetAxis(gpGameController, SDL_CONTROLLER_AXIS_LEFTY) <= -DIGITAL_THRESHOLD) {
            btnUp.value = UINT8_MAX;
        }

        if (SDL_GameControllerGetAxis(gpGameController, SDL_CONTROLLER_AXIS_RIGHTX) >= DIGITAL_THRESHOLD) {
            btnRight.value = UINT8_MAX;
        } else if (SDL_GameControllerGetAxis(gpGameController, SDL_CONTROLLER_AXIS_RIGHTX) <= -DIGITAL_THRESHOLD) {
            btnLeft.value = UINT8_MAX;
        }

        if (SDL_GameControllerGetAxis(gpGameController, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) >= DIGITAL_THRESHOLD) {
            btnTriangle.value = UINT8_MAX;
        }

        if (SDL_GameControllerGetAxis(gpGameController, SDL_CONTROLLER_AXIS_TRIGGERLEFT) >= DIGITAL_THRESHOLD) {
            btnSquare.value = UINT8_MAX;
        }
    }

    // Update the emulator controller
    ASSERT(InputManager::getInstance());
    InputManager& inputMgr = *InputManager::getInstance();

    inputMgr.setState(gPadBtnKey_Up, btnUp);
    inputMgr.setState(gPadBtnKey_Down, btnDown);
    inputMgr.setState(gPadBtnKey_Left, btnLeft);
    inputMgr.setState(gPadBtnKey_Right, btnRight);
    inputMgr.setState(gPadBtnKey_L1, btnL1);
    inputMgr.setState(gPadBtnKey_R1, btnR1);
    inputMgr.setState(gPadBtnKey_L2, btnL2);
    inputMgr.setState(gPadBtnKey_R2, btnR2);
    inputMgr.setState(gPadBtnKey_Circle, btnCircle);
    inputMgr.setState(gPadBtnKey_Square, btnSquare);
    inputMgr.setState(gPadBtnKey_Triangle, btnTriangle);
    inputMgr.setState(gPadBtnKey_Cross, btnCross);
    inputMgr.setState(gPadBtnKey_Start, btnStart);
    inputMgr.setState(gPadBtnKey_Select, btnSelect);

    // TODO: remove use of avocado controller
    gpSystem->controller->update();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the state of an emulated PlayStation digital controller, telling which buttons are pressed in the returned bit-mask.
// TODO: move getting PSX controller buttons to a controls module and make the PSX button bindings configurable.
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t getControllerButtonBits() noexcept {
    uint16_t buttonBits = 0;
    InputManager& inputMgr = *InputManager::getInstance();
    
    // TODO: don't use hardcoded constants for the button bits
    if (inputMgr.getDigital(gPadBtnKey_Up))         { buttonBits |= 0x1000; }
    if (inputMgr.getDigital(gPadBtnKey_Down))       { buttonBits |= 0x4000; }
    if (inputMgr.getDigital(gPadBtnKey_Left))       { buttonBits |= 0x8000; }
    if (inputMgr.getDigital(gPadBtnKey_Right))      { buttonBits |= 0x2000; }
    if (inputMgr.getDigital(gPadBtnKey_L1))         { buttonBits |= 0x4;    }
    if (inputMgr.getDigital(gPadBtnKey_R1))         { buttonBits |= 0x8;    }
    if (inputMgr.getDigital(gPadBtnKey_L2))         { buttonBits |= 0x1;    }
    if (inputMgr.getDigital(gPadBtnKey_R2))         { buttonBits |= 0x2;    }
    if (inputMgr.getDigital(gPadBtnKey_Circle))     { buttonBits |= 0x20;   }
    if (inputMgr.getDigital(gPadBtnKey_Square))     { buttonBits |= 0x80;   }
    if (inputMgr.getDigital(gPadBtnKey_Triangle))   { buttonBits |= 0x10;   }
    if (inputMgr.getDigital(gPadBtnKey_Cross))      { buttonBits |= 0x40;   }
    if (inputMgr.getDigital(gPadBtnKey_Start))      { buttonBits |= 0x800;  }
    if (inputMgr.getDigital(gPadBtnKey_Select))     { buttonBits |= 0x100;  }

    return buttonBits;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Submit a drawing primitive to the GPU
// TODO: replace with a more readable solution; don't write to GP0 and setup the GPU and submit primitives directly
//------------------------------------------------------------------------------------------------------------------------------------------
void submitGpuPrimitive(const void* const pPrim) noexcept {
    ASSERT(pPrim);
    
    // Get the primitive tag and consequently how many data words there are in the primitive
    const uint32_t* pCurWord = (const uint32_t*) pPrim;
    const uint32_t tag = pCurWord[0];
    const uint32_t numDataWords = tag >> 24;

    ++pCurWord;

    // Submit the primitive's data words to the GPU
    uint32_t dataWordsLeft = numDataWords;

    while (dataWordsLeft > 0) {
        const uint32_t dataWord = *pCurWord;
        ++pCurWord;
        --dataWordsLeft;
        gpGpu->writeGP0(dataWord);
    }
}

void emulateSoundIfRequired() noexcept {
    // Do no sound emulation in headless mode
    if (ProgArgs::gbHeadlessMode)
        return;

    while (true) {
        size_t soundBufferSize;

        {
            std::unique_lock<std::mutex> lock(Sound::audioMutex);
            soundBufferSize = Sound::buffer.size();
        }

        // TODO: (FIXME) temp hack - fill the sound buffers up a decent amount to prevent skipping
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

END_NAMESPACE(PsxVm)
