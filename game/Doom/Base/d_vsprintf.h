#pragma once

#include "PsxVm/VmPtr.h"

int32_t D_vsprintf(char* dstStr, const char* fmtStr, VmPtr<uint32_t> argPtr) noexcept;
