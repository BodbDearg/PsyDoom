#pragma once

#include "Doom/doomdef.h"

struct mobj_t;

extern const VmPtr<bool32_t>        gbTryMove2;
extern const VmPtr<VmPtr<mobj_t>>   gpMoveThing;
extern const VmPtr<fixed_t>         gTmFloorZ;
extern const VmPtr<fixed_t>         gTmCeilingZ;
extern const VmPtr<fixed_t>         gTmDropoffZ;

void P_TryMove2() noexcept;
