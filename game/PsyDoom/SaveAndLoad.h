#pragma once

#include "Macros.h"

#include <unordered_map>

struct mobj_t;

BEGIN_NAMESPACE(SaveAndLoad)

extern std::unordered_map<mobj_t*, int32_t>     gMobjToIdx;
extern std::vector<mobj_t*>                     gMobjList;

END_NAMESPACE(SaveAndLoad)
