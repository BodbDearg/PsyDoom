#pragma once

#include "Doom/doomdef.h"

struct line_t;
struct mobj_t;

extern bool         gbTryMove2;
extern mobj_t*      gpMoveThing;
extern line_t*      gpBlockLine;
extern fixed_t      gTmFloorZ;
extern fixed_t      gTmCeilingZ;
extern fixed_t      gTmDropoffZ;
extern int32_t      gNumCrossCheckLines;
extern bool         gbFloatOk;

void P_TryMove2() noexcept;
