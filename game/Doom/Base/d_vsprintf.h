#pragma once

#include "PsxVm/VmPtr.h"
#include <cstdarg>

int32_t D_vsprintf(char* dstStr, const char* fmtStr, va_list args) noexcept;
