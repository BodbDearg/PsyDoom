//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface functions.
// Functionality which helps us emulate the original hardware environment of the program.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "PsxVm.h"

#include "PcPsx/Macros.h"
#include "PcPsx/ProgArgs.h"
#include <cstdlib>

BEGIN_DISABLE_HEADER_WARNINGS
    #include <cpu/instructions.h>
    #include <sound/sound.h>
    #include <system.h>
END_DISABLE_HEADER_WARNINGS

// External function required from LIBCD.
// This is a slight layering violation (this module should not know app code) but is required for correct functionality.
// This steps the cdrom drive and invokes any data callbacks that would be invoked by new sectors being available.
void stepCdromWithCallbacks() noexcept;

using namespace PsxVm;

namespace PsxVm {
    uint32_t* gpReg_sp;
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
