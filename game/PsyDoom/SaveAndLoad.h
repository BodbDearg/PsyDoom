#pragma once

#include "Macros.h"

#include <unordered_map>

class InputStream;
class OutputStream;
struct mobj_t;

BEGIN_NAMESPACE(SaveAndLoad)

extern std::unordered_map<mobj_t*, int32_t>     gMobjToIdx;
extern std::vector<mobj_t*>                     gMobjList;

bool save(OutputStream& out) noexcept;

END_NAMESPACE(SaveAndLoad)
