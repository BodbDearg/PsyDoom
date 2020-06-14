//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface functions.
// Functionality which helps us emulate the original hardware environment of the program.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "PsxVm.h"

#include "PcPsx/ProgArgs.h"

BEGIN_DISABLE_HEADER_WARNINGS
    #include <sound/sound.h>
    #include <system.h>
END_DISABLE_HEADER_WARNINGS

// External function required from LIBCD.
// This is a slight layering violation (this module should not know app code) but is required for correct functionality.
// This steps the cdrom drive and invokes any data callbacks that would be invoked by new sectors being available.
void stepCdromWithCallbacks() noexcept;

using namespace PsxVm;

void emulate_sound_if_required() noexcept {
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
