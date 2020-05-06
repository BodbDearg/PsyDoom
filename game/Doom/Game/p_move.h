#pragma once

#include "PcPsx/Types.h"
#include "PsxVm/VmPtr.h"

struct mobj_t;

extern const VmPtr<bool32_t>        gbTryMove2;
extern const VmPtr<VmPtr<mobj_t>>   gpMoveThing;

void P_TryMove2() noexcept;
