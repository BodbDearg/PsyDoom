#pragma once

#include <cstdarg>
#include <cstdint>

int32_t D_vsprintf(char* dstStr, const char* fmtStr, va_list args) noexcept;
