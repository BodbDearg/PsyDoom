//------------------------------------------------------------------------------------------------------------------------------------------
// VM controller functions.
// Setup and control the Avocado emulator and our emulation layers.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "PsxVm.h"

#include "Assert.h"
#include "DiscInfo.h"
#include "DiscReader.h"
#include "Input.h"
#include "IsoFileSys.h"
#include "ProgArgs.h"
#include "PsxPadButtons.h"

#include <SDL.h>

BEGIN_DISABLE_HEADER_WARNINGS
    #include <disc/format/cue.h>
    #include <disc/load.h>
    #include <sound/sound.h>
    #include <system.h>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(PsxVm)

DiscInfo                gDiscInfo;
IsoFileSys              gIsoFileSys;
System*                 gpSystem;
gpu::GPU*               gpGpu;
spu::SPU*               gpSpu;
device::cdrom::CDROM*   gpCdrom;

static std::unique_ptr<System> gSystem;

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

    // Parse the .cue info for the game disc
    {
        std::string parseErrorMsg;

        if (!gDiscInfo.parseFromCueFile(doomCdCuePath, parseErrorMsg)) {
            FatalErrors::raiseF(
                "Couldn't open or failed to parse the game disc .cue file '%s'!\nError message: %s",
                doomCdCuePath,
                parseErrorMsg.c_str()
            );
        }
    }

    // Build up the ISO file system from the game disc
    {
        DiscReader discReader(gDiscInfo);

        if (!gIsoFileSys.build(discReader)) {
            FatalErrors::raise(
                "Failed to extract the ISO 9960 filesystem records from the game's disc! "
                "Is the disc in a strange format, or is the image corrupt?"
            );
        }
    }
    
    // Open the DOOM cdrom with Avocado
    system.cdrom->disc = disc::load(doomCdCuePath);
    system.cdrom->setShell(false);

    if (!system.cdrom->disc) {
        FatalErrors::raiseF("Couldn't open or failed to parse the game disc .cue file '%s'!\n", doomCdCuePath);
    }

    // Setup sound.
    // TODO: move sound stuff elsewhere.
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
    
    gpCdrom = nullptr;
    gpSpu = nullptr;
    gpGpu = nullptr;
    gpSystem = nullptr;
    gSystem.reset();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the state of an emulated PlayStation digital controller, telling which buttons are pressed in the returned bit-mask.
// TODO: move getting PSX controller buttons to a controls module and make the PSX button bindings configurable.
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t getControllerButtonBits() noexcept {
    uint16_t pressedButtonBits = 0;

    // TODO: don't hardcode these bindings to PSX controller buttons
    auto bindControllerInput = [&](const ControllerInput input, const uint16_t toButtonBits) noexcept {
        if (Input::isControllerInputPressed(input)) {
            pressedButtonBits |= toButtonBits;
        }
    };

    bindControllerInput(ControllerInput::BTN_DPAD_UP, PAD_UP);
    bindControllerInput(ControllerInput::BTN_DPAD_DOWN, PAD_DOWN);
    bindControllerInput(ControllerInput::BTN_DPAD_LEFT, PAD_LEFT);
    bindControllerInput(ControllerInput::BTN_DPAD_RIGHT, PAD_RIGHT);
    bindControllerInput(ControllerInput::BTN_Y, PAD_TRIANGLE);
    bindControllerInput(ControllerInput::BTN_X, PAD_SQUARE);
    bindControllerInput(ControllerInput::BTN_B, PAD_CIRCLE);
    bindControllerInput(ControllerInput::BTN_A, PAD_CROSS);
    bindControllerInput(ControllerInput::BTN_LEFT_SHOULDER, PAD_L1);
    bindControllerInput(ControllerInput::BTN_RIGHT_SHOULDER, PAD_R1);
    bindControllerInput(ControllerInput::AXIS_TRIG_LEFT, PAD_L2);
    bindControllerInput(ControllerInput::AXIS_TRIG_RIGHT, PAD_R2);
    bindControllerInput(ControllerInput::BTN_START, PAD_START);
    bindControllerInput(ControllerInput::BTN_BACK, PAD_SELECT);

    return pressedButtonBits;
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
