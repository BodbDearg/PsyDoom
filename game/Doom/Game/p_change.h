#pragma once

struct mobj_t;
struct sector_t;

extern bool gbNofit;
extern bool gbCrushChange;

bool P_ThingHeightClip(mobj_t& mobj) noexcept;
bool PIT_ChangeSector(mobj_t& mobj) noexcept;
bool P_ChangeSector(sector_t& sector, const bool bCrunch) noexcept;
