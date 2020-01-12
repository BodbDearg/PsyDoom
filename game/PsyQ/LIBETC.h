#pragma once

#include <cstdint>
#include <cstddef>

namespace PsxVm {
    extern uint8_t* gpScratchpad;
}

// Bit flags for controller buttons
static constexpr uint16_t PADLup        = 0x1000;       // Up
static constexpr uint16_t PADLdown      = 0x4000;       // Down
static constexpr uint16_t PADLleft      = 0x8000;       // Left
static constexpr uint16_t PADLright     = 0x2000;       // Right
static constexpr uint16_t PADRup        = 0x10;         // Triangle
static constexpr uint16_t PADRdown      = 0x40;         // Cross
static constexpr uint16_t PADRleft      = 0x80;         // Square
static constexpr uint16_t PADRright     = 0x20;         // Circle
static constexpr uint16_t PADL1         = 0x4;          // L1
static constexpr uint16_t PADL2         = 0x1;          // L2
static constexpr uint16_t PADR1         = 0x8;          // R1
static constexpr uint16_t PADR2         = 0x2;          // R2
static constexpr uint16_t PADstart      = 0x800;        // Start
static constexpr uint16_t PADselect     = 0x100;        // Select

// A combination of all the available pad buttons.
// There was no such constant in the PsyQ SDK but I'm making it since the value is used in some places in DOOM:
static constexpr uint16_t PAD_ANY = (
    PADLup|PADLdown|PADLleft|PADLright|PADRup|PADRdown|PADRleft|PADRright|PADL1|PADL2|PADR1|PADR2|PADstart|PADselect
);

// Return a pointer to the specified word in the PSX scratchpad memory/cache.
// There is 1 KiB of memory in total in the scratchpad.
static constexpr void* getScratchAddr(const uint32_t word) {
    return PsxVm::gpScratchpad + word * sizeof(uint32_t);
}

void LIBETC_ResetCallback() noexcept;
void LIBETC_InterruptCallback() noexcept;
void LIBETC_DMACallback() noexcept;
void LIBETC_VSyncCallbacks() noexcept;
void LIBETC_StopCallback() noexcept;
void LIBETC_CheckCallback() noexcept;
void LIBETC_GetIntrMask() noexcept;
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

void LIBETC_SetVideoMode() noexcept;
void LIBETC_GetVideoMode() noexcept;
