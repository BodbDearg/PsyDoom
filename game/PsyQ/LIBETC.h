#pragma once

#include <cstdint>

namespace PsxVm {
    extern uint8_t* gpScratchpad;
}

// Return a pointer to the specified word in the PSX scratchpad memory/cache.
// There is 1 KiB of memory in total in the scratchpad.
static inline void* LIBETC_getScratchAddr(const uint32_t word) {
    return PsxVm::gpScratchpad + word * sizeof(uint32_t);
}

void LIBETC_ResetCallback() noexcept;
int32_t LIBETC_VSync(const int32_t mode) noexcept;
uint32_t LIBETC_PadRead(uint32_t unusedControllerId) noexcept;
