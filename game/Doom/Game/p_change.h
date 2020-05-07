#pragma once

#include "PcPsx/Types.h"
#include "PsxVm/VmPtr.h"

struct mobj_t;
struct sector_t;

extern const VmPtr<bool32_t>    gbNofit;
extern const VmPtr<bool32_t>    gbCrushChange;

bool P_ThingHeightClip(mobj_t& mobj) noexcept;
bool PIT_ChangeSector(mobj_t& mobj) noexcept;
bool P_ChangeSector(sector_t& sector, const bool bCrunch) noexcept;
