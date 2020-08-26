#pragma once

#include "PcPsx/Macros.h"

#include <cstdint>

struct DiscInfo;
struct IsoFileSys;

namespace Spu {
    struct Core;
}

// Forward declaring avocado types
struct System;
namespace gpu { class GPU; }

BEGIN_NAMESPACE(PsxVm)

// Information for the game disc and the filesystem
extern DiscInfo     gDiscInfo;
extern IsoFileSys   gIsoFileSys;

// Access to emulated Avocado devices
extern System*      gpSystem;
extern gpu::GPU*    gpGpu;
extern Spu::Core    gSpu;

bool init(const char* const doomCdCuePath) noexcept;
void shutdown() noexcept;
void submitGpuPrimitive(const void* const pPrim) noexcept;

// Fire timer (root counter) related events if appropriate.
// Note: this is implemented in LIBAPI, where timers are handled.
void generateTimerEvents() noexcept;

// Lock and unlock the SPU and a helper to do it via the RAII pattern.
// The SPU should be locked before reading from or writing to any of it's properties.
void lockSpu() noexcept;
void unlockSpu() noexcept;

struct LockSpu {
    LockSpu() noexcept { lockSpu(); }
    ~LockSpu() noexcept { unlockSpu(); }
};

END_NAMESPACE(PsxVm)
