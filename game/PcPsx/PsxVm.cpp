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
#include "Spu.h"

#include <SDL.h>
#include <mutex>

BEGIN_DISABLE_HEADER_WARNINGS
    #include <disc/format/cue.h>
    #include <disc/load.h>
    #include <system.h>
END_DISABLE_HEADER_WARNINGS

BEGIN_NAMESPACE(PsxVm)

DiscInfo                gDiscInfo;
IsoFileSys              gIsoFileSys;
System*                 gpSystem;
gpu::GPU*               gpGpu;
Spu::Core               gSpu;
device::cdrom::CDROM*   gpCdrom;

static std::unique_ptr<System>  gSystem;
static SDL_AudioDeviceID        gSdlAudioDeviceId;
static std::recursive_mutex     gSpuMutex;

//------------------------------------------------------------------------------------------------------------------------------------------
// A callback invoked by SDL to ask for audio from PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
static void SdlAudioCallback([[maybe_unused]] void* userData, Uint8* pOutput, int outputSize) noexcept {
    // Ignore invalid requests
    if (outputSize <= 0)
        return;

    // Lock the SPU and generate the requested number of samples
    PsxVm::LockSpu spuLock;
    const uint32_t numSamples = (uint32_t) outputSize / sizeof(Spu::StereoSample);
    
    for (uint32_t sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
        const Spu::StereoSample sample = Spu::stepCore(gSpu);
        std::memcpy(pOutput, &sample, sizeof(sample));
        pOutput += sizeof(sample);
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
    Spu::initPS1Core(gSpu);
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

    // Setup sound
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    {
        // Firstly try to open an audio device sampling at 44,100 Hz, stereo in signed 16-bit mode.
        // Note that if initialization succeeds then we've got our requested format, since we ask SDL not to allow any deviation.
        SDL_AudioSpec wantFmt = {};
        wantFmt.freq = 44100;
        wantFmt.format = AUDIO_S16SYS;
        wantFmt.channels = 2;
        wantFmt.samples = 512;  // Update approximately every 12 MS or at 83 Hz - this should be pretty responsive
        wantFmt.callback = SdlAudioCallback;

        SDL_AudioSpec gotFmt = {};
        gSdlAudioDeviceId = SDL_OpenAudioDevice(nullptr, false, &wantFmt, &gotFmt, false);

        if (gSdlAudioDeviceId != 0) {
            SDL_PauseAudioDevice(gSdlAudioDeviceId, false);
        }
    }

    // Initialize game controller support
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tear down emulated PlayStation components
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {    
    if (gSdlAudioDeviceId != 0) {
        SDL_PauseAudioDevice(gSdlAudioDeviceId, true);
        SDL_CloseAudioDevice(gSdlAudioDeviceId);
        gSdlAudioDeviceId = 0;
    }

    gpCdrom = nullptr;
    Spu::destroyCore(gSpu);     // Note: no locking of the SPU here because all threads should be done with it at this point
    gpGpu = nullptr;
    gpSystem = nullptr;
    gSystem.reset();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Submit a drawing primitive to the GPU
// TODO: submitGpuPrimitive: replace with a more readable solution; don't write to GP0 and setup the GPU and submit primitives directly
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

void lockSpu() noexcept {
    gSpuMutex.lock();
}

void unlockSpu() noexcept {
    gSpuMutex.unlock();
}

END_NAMESPACE(PsxVm)
