#pragma once

#include "Doom/doomdef.h"

struct line_t;
struct side_t;

// Holds state for an animated texture or flat.
// Note that only one of these exists per texture - individual walls & floors do not have unique anims.
struct anim_t {
    uint32_t    istexture;
    int32_t     picnum;
    int32_t     basepic;
    int32_t     numpics;
    int32_t     current;
    uint32_t    ticmask;        // New field for PSX: controls which game tics the animation will advance on
};

static_assert(sizeof(anim_t) == 24);

// The number of animated floor/texture types in the game
static constexpr int32_t MAXANIMS = 16;

extern const VmPtr<anim_t[MAXANIMS]>    gAnims;
extern const VmPtr<VmPtr<anim_t>>       gpLastAnim;

void P_InitPicAnims() noexcept;
side_t* getSide(const int32_t sectorIdx, const int32_t lineIdx, const int32_t sideIdx) noexcept;
sector_t* getSector(const int32_t sectorIdx, const int32_t lineIdx, const int32_t sideIdx) noexcept;
bool twoSided(const int32_t sectorIdx, const int32_t lineIdx) noexcept;
sector_t* getNextSector(line_t& line, sector_t& sector) noexcept;
fixed_t P_FindLowestFloorSurrounding(sector_t& sector) noexcept;
fixed_t P_FindHighestFloorSurrounding(sector_t& sector) noexcept;
fixed_t P_FindNextHighestFloor(sector_t& sector, const fixed_t baseHeight) noexcept;
void P_FindLowestCeilingSurrounding() noexcept;
void P_FindHighestCeilingSurrounding() noexcept;
void P_FindSectorFromLineTag() noexcept;
void P_FindMinSurroundingLight() noexcept;
void P_CrossSpecialLine() noexcept;
void P_ShootSpecialLine() noexcept;
void P_PlayerInSpecialSector() noexcept;
void P_UpdateSpecials() noexcept;
void EV_DoDonut() noexcept;
void G_ScheduleExitLevel() noexcept;
void G_BeginExitLevel() noexcept;
void G_ExitLevel() noexcept;
void G_SecretExitLevel() noexcept;
void P_SpawnSpecials() noexcept;
