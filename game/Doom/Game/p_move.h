#pragma once

#include "Doom/doomdef.h"

struct mobj_t;

extern const VmPtr<bool32_t>        gbTryMove2;
extern const VmPtr<VmPtr<mobj_t>>   gpMoveThing;
extern const VmPtr<fixed_t>         gTmFloorZ;
extern const VmPtr<fixed_t>         gTmCeilingZ;
extern const VmPtr<fixed_t>         gTmDropoffZ;
extern const VmPtr<int32_t>         gNumCrossCheckLines;
extern const VmPtr<bool32_t>        gbFloatOk;

void P_TryMove2() noexcept;
