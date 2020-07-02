#pragma once

#include "PcPsx/Macros.h"

#include <cstdint>

struct DiscInfo;
struct IsoFileSys;

// Forward declaring avocado types
struct System;

namespace gpu               { class GPU;    }
namespace spu               { struct SPU;   }
namespace device::cdrom     { class CDROM;  }

BEGIN_NAMESPACE(PsxVm)

// Information for the game disc and the filesystem
extern DiscInfo     gDiscInfo;
extern IsoFileSys   gIsoFileSys;

// Access to emulated Avocado devices
extern System*                  gpSystem;
extern gpu::GPU*                gpGpu;
extern spu::SPU*                gpSpu;
extern device::cdrom::CDROM*    gpCdrom;

bool init(const char* const doomCdCuePath) noexcept;
void shutdown() noexcept;
uint16_t getControllerButtonBits() noexcept;
void submitGpuPrimitive(const void* const pPrim) noexcept;
void emulateSoundIfRequired() noexcept;

// Fire timer (root counter) related events if appropriate.
// Note: this is implemented in LIBAPI, where timers are handled.
void generateTimerEvents() noexcept;

// External function required from LIBCD.
// This is a slight layering violation (this module should not know app code) but is required for correct functionality.
// This steps the cdrom drive and invokes any data callbacks that would be invoked by new sectors being available.
void stepCdromWithCallbacks() noexcept;

END_NAMESPACE(PsxVm)
