#pragma once

#include "PcPsx/Types.h"
#include "PsxVm/VmPtr.h"

extern const VmPtr<bool32_t>    gbTryMove2;

void P_TryMove2() noexcept;
