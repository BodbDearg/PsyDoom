//------------------------------------------------------------------------------------------------------------------------------------------
// Basic setup and management for some of the emulated elements of a PlayStation 1, including the CD-ROM, SPU and GPU.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "PsxVm.h"

#include "Asserts.h"
#include "AudioCompressor.h"
#include "Config.h"
#include "DiscInfo.h"
#include "DiscReader.h"
#include "Gpu.h"
#include "Input.h"
#include "IsoFileSys.h"
#include "ProgArgs.h"
#include "Spu.h"

#include <SDL.h>
#include <mutex>

BEGIN_NAMESPACE(PsxVm)

DiscInfo    gDiscInfo;
IsoFileSys  gIsoFileSys;
Gpu::Core   gGpu;
Spu::Core   gSpu;

static SDL_AudioDeviceID        gSdlAudioDeviceId;
static std::recursive_mutex     gSpuMutex;

// The audio compressor is only needed if we have a floating point SPU
#if SIMPLE_SPU_FLOAT_SPU
    static AudioCompressor::State gAudioCompState;
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// A callback invoked by SDL to ask for audio from PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
static void SdlAudioCallback([[maybe_unused]] void* userData, Uint8* pOutput, int outputSize) noexcept {
    // Ignore invalid requests
    if (outputSize <= 0)
        return;

    // How many samples are to be output?
    const uint32_t numSamples = (uint32_t) outputSize / (sizeof(float) * 2);

    // Lock the SPU and generate the requested number of samples
    float* pOutputF = reinterpret_cast<float*>(pOutput);
    PsxVm::LockSpu spuLock;

    for (uint32_t sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx) {
        // Get this sample in floating point format
        const Spu::StereoSample sample = Spu::stepCore(gSpu);

        #if SIMPLE_SPU_FLOAT_SPU
            float sampleL = sample.left;
            float sampleR = sample.right;
        #else
            float sampleL = Spu::toFloatSample(sample.left);
            float sampleR = Spu::toFloatSample(sample.right);
        #endif

        // If using the floating point SPU apply audio compression.
        // When using floating point sound the audio can get EXTREMELY loud (and painful to listen to) if not capped.
        // When using the original 16-bit SPU the sound will also clip/distort if too loud, so no point in using compression in that case.
        #if SIMPLE_SPU_FLOAT_SPU
            AudioCompressor::compress(gAudioCompState, sampleL, sampleR);
        #endif

        pOutputF[0] = sampleL;
        pOutputF[1] = sampleR;
        pOutputF += 2;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determine the VRAM size to use from user config
//------------------------------------------------------------------------------------------------------------------------------------------
static void getVramSize(uint16_t& vramW, uint16_t& vramH) noexcept {
    const int32_t vramSize = Config::gVramSizeInMegabytes;

    #if PSYDOOM_LIMIT_REMOVING
        if (vramSize > 64) {
            vramW = 8192; vramH = 8192;     // 128 MiB
        } else if (vramSize > 32) {
            vramW = 4096; vramH = 8192;     // 64 MiB
        } else if (vramSize > 16) {
            vramW = 4096; vramH = 4096;     // 32 MiB
        } else if (vramSize > 8) {
            vramW = 2048; vramH = 4096;     // 16 MiB
        } else if (vramSize > 4) {
            vramW = 2048; vramH = 2048;     // 8 MiB
        } else if (vramSize > 2) {
            vramW = 1024; vramH = 2048;     // 4 MiB
        } else if (vramSize == 2) {
            vramW = 1024; vramH = 1024;     // 2 MiB
        } else if (vramSize == 1) {
            vramW = 1024; vramH = 512;      // 1 MiB
        } else {
            vramW = 8192; vramH = 8192;     // Default: 128 MiB
        }
    #else
         vramW = 1024; vramH = 512;         // 1 MiB
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialize emulated PlayStation system components and use the given .cue file for the game disc
//------------------------------------------------------------------------------------------------------------------------------------------
bool init(const char* const doomCdCuePath) noexcept {
    // Init the GPU core
    {
        uint16_t vramW = {};
        uint16_t vramH = {};
        getVramSize(vramW, vramH);
        Gpu::initCore(gGpu, vramW, vramH);
    }

    // Init the SPU core and use extended hardware voice counts (64 max) and an expanded RAM size (defaulted to 16 MiB) if the build is limit removing.
    // Note: don't allow  SPU RAM to be smaller than the original 512 KiB for compatibility reasons.
    constexpr uint32_t PSX_SPU_RAM_SIZE = 512 * 1024;

    #if PSYDOOM_LIMIT_REMOVING
        constexpr uint32_t DEFAULT_SPU_RAM_SIZE = 16 * 1024 * 1024;
        constexpr uint32_t SPU_VOICE_COUNT = 64;
        const uint32_t spuRamSize = std::max<uint32_t>((Config::gSpuRamSize <= 0) ? DEFAULT_SPU_RAM_SIZE : Config::gSpuRamSize, PSX_SPU_RAM_SIZE);
    #else
        constexpr uint32_t SPU_VOICE_COUNT = 24;
        constexpr uint32_t spuRamSize = PSX_SPU_RAM_SIZE;
    #endif

    Spu::initCore(gSpu, spuRamSize, SPU_VOICE_COUNT);

    // Init the audio compressor if using the float SPU (don't need it for the 16-bit SPU)
    #if SIMPLE_SPU_FLOAT_SPU
        AudioCompressor::init(
            gAudioCompState,
            -12.0f,             // thresholdDB
            10.0f,              // kneeWidthDB
            2.0f,               // compressionRatio
            8.0f,               // postGainDB
            0.150f,             // lpfResponseTime
            0.002f,             // attackTime
            0.005f              // releaseTime
        );
    #endif

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

    // Setup sound
    SDL_InitSubSystem(SDL_INIT_AUDIO);

    {
        // Firstly try to open an audio device sampling at 44,100 Hz stereo in floating point mode.
        // Note that if initialization succeeds then we've got our requested format, since we ask SDL not to allow any deviation.
        SDL_AudioSpec wantFmt = {};
        wantFmt.freq = 44100;
        wantFmt.format = AUDIO_F32;
        wantFmt.channels = 2;

        if (Config::gAudioBufferSize > 0) {
            wantFmt.samples = (uint16_t) std::min<int32_t>(Config::gAudioBufferSize, UINT16_MAX);
        } else {
            wantFmt.samples = 128;  // Use a default of '128' samples (~2.9 MS latency) when using 'auto' configure mode
        }

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

    Spu::destroyCore(gSpu);     // Note: no locking of the SPU here because all threads should be done with it at this point
    Gpu::destroyCore(gGpu);
}

void lockSpu() noexcept {
    gSpuMutex.lock();
}

void unlockSpu() noexcept {
    gSpuMutex.unlock();
}

END_NAMESPACE(PsxVm)
