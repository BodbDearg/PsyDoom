#pragma once

#include <cstdint>

void* LIBETC_getScratchAddr(const uint32_t word32Index);
void LIBETC_ResetCallback() noexcept;
int32_t LIBETC_VSync(const int32_t mode) noexcept;
uint32_t LIBETC_PadRead(uint32_t unusedControllerId) noexcept;
