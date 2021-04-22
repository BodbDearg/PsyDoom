#pragma once

#include "Doom/doomdef.h"

#include <vector>

struct line_t;
struct mobj_t;

extern bool         gbTryMove2;
extern mobj_t*      gpMoveThing;
extern line_t*      gpBlockLine;
extern fixed_t      gTmFloorZ;
extern fixed_t      gTmCeilingZ;
extern fixed_t      gTmDropoffZ;
extern bool         gbFloatOk;

#if PSYDOOM_MODS
    extern std::vector<line_t*> gpCrossCheckLines;
#else
    extern int32_t gNumCrossCheckLines;
#endif

void P_TryMove2() noexcept;
