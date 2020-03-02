#pragma once

#include <cstdint>
#include <cstddef>

namespace PsxVm {
    extern uint8_t* gpScratchpad;
}

// Return a pointer to the specified word in the PSX scratchpad memory/cache.
// There is 1 KiB of memory in total in the scratchpad.
static inline void* LIBETC_getScratchAddr(const uint32_t word) {
    return PsxVm::gpScratchpad + word * sizeof(uint32_t);
}

void LIBETC_ResetCallback() noexcept;
void LIBETC_InterruptCallback() noexcept;
void LIBETC_SetIntrMask() noexcept;
void LIBETC_INTR_VB_setIntrVSync() noexcept;

int32_t LIBETC_VSync(const int32_t mode) noexcept;
void _thunk_LIBETC_VSync() noexcept;

void LIBETC_v_wait(const int32_t targetVCount, const uint16_t timeout) noexcept;
void _thunk_LIBETC_v_wait() noexcept;
