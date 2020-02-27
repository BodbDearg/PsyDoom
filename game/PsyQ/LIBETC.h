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
void LIBETC_DMACallback() noexcept;
void LIBETC_CheckCallback() noexcept;
void LIBETC_SetIntrMask() noexcept;
void LIBETC_INTR_startIntr() noexcept;
void LIBETC_INTR_trapIntr() noexcept;
void LIBETC_INTR_setIntr() noexcept;
void LIBETC_INTR_stopIntr() noexcept;
void LIBETC_INTR_memclr() noexcept;
void LIBETC_startIntrVSync() noexcept;
void LIBETC_INTR_stopIntr_UNKNOWN_Helper2() noexcept;
void LIBETC_INTR_VB_trapIntrVSync() noexcept;
void LIBETC_INTR_VB_setIntrVSync() noexcept;
void LIBETC_INTR_VB_memclr() noexcept;
void LIBETC_startIntrDMA() noexcept;
void LIBETC_INTR_stopIntr_UNKNOWN_Helper1() noexcept;
void LIBETC_INTR_DMA_trapIntrDMA() noexcept;
void LIBETC_INTR_DMA_setIntrDMA() noexcept;
void LIBETC_INTR_DMA_memclr() noexcept;

int32_t LIBETC_VSync(const int32_t mode) noexcept;
void _thunk_LIBETC_VSync() noexcept;

void LIBETC_v_wait(const int32_t targetVCount, const uint16_t timeout) noexcept;
void _thunk_LIBETC_v_wait() noexcept;
