#pragma once

#include <cstdint>

void LIBCOMB_AddCOMB() noexcept;
void LIBCOMB_DelCOMB() noexcept;

// Note: these were originally convenience macros for using 'LIBCOMB__comb_control'.
// I've made that function private to the module, since it is more awkward to use.
bool LIBCOMB_CombCTS() noexcept;
int32_t LIBCOMB_CombResetError() noexcept;
int32_t LIBCOMB_CombCancelRead() noexcept;
int32_t LIBCOMB_CombSetBPS(const int32_t commsBps) noexcept;
