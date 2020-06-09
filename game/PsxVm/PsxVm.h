#pragma once

#include "PcPsx/Macros.h"

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface: shorthand type aliases
//------------------------------------------------------------------------------------------------------------------------------------------
typedef void (*VmFunc)();

//------------------------------------------------------------------------------------------------------------------------------------------
// VM interface: function calls and utilities
//------------------------------------------------------------------------------------------------------------------------------------------

// Emulate sound until we have enough samples to handle an upcoming buffer request
void emulate_sound_if_required() noexcept;

// Fire timer (root counter) related events if appropriate.
// Note: this is implemented in LIBAPI, where timers are handled.
void generate_timer_events() noexcept;

//------------------------------------------------------------------------------------------------------------------------------------------
// Avocado types
//------------------------------------------------------------------------------------------------------------------------------------------
struct System;

namespace gpu               { class GPU;    }
namespace spu               { struct SPU;   }
namespace device::cdrom     { class CDROM;  }

//------------------------------------------------------------------------------------------------------------------------------------------
// VM control: setup and control the VM
//------------------------------------------------------------------------------------------------------------------------------------------
namespace PsxVm {
    // Access to the emulated PSX SPU & GPU
    extern System*                  gpSystem;
    extern gpu::GPU*                gpGpu;
    extern spu::SPU*                gpSpu;
    extern device::cdrom::CDROM*    gpCdrom;
    extern uint8_t*                 gpScratchpad;       // Cache used as fast RAM (1 KiB) (TODO: make this a native app buffer)

    bool init(const char* const doomCdCuePath) noexcept;
    void shutdown() noexcept;

    // Updates inputs to the emulator from real inputs on the host machine
    void updateInput() noexcept;

    // Get the button bits for the controller directly, bypassing emulation
    uint16_t getControllerButtonBits() noexcept;

    // Submit a drawing primitive to the GPU
    void submitGpuPrimitive(const void* const pPrim) noexcept;
}
