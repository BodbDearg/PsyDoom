#pragma once

#include <cstdint>

void LIBCOMB_AddCOMB() noexcept;
void LIBCOMB_DelCOMB() noexcept;
int32_t LIBCOMB__comb_control(const int32_t cmd, const int32_t subcmd, const int32_t param) noexcept;
