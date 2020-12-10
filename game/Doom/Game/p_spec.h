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

extern card_t   gMapBlueKeyType;
extern card_t   gMapRedKeyType;
extern card_t   gMapYellowKeyType;
extern int32_t  gMapBossSpecialFlags;

#if PSYDOOM_MODS
    void P_SetAnimsToBasePic() noexcept;
#endif

void P_InitPicAnims() noexcept;
side_t* getSide(const int32_t sectorIdx, const int32_t lineIdx, const int32_t sideIdx) noexcept;
sector_t* getSector(const int32_t sectorIdx, const int32_t lineIdx, const int32_t sideIdx) noexcept;
bool twoSided(const int32_t sectorIdx, const int32_t lineIdx) noexcept;
sector_t* getNextSector(line_t& line, sector_t& sector) noexcept;
fixed_t P_FindLowestFloorSurrounding(sector_t& sector) noexcept;
fixed_t P_FindHighestFloorSurrounding(sector_t& sector) noexcept;
fixed_t P_FindNextHighestFloor(sector_t& sector, const fixed_t baseHeight) noexcept;
fixed_t P_FindLowestCeilingSurrounding(sector_t& sector) noexcept;
fixed_t P_FindHighestCeilingSurrounding(sector_t& sector) noexcept;
int32_t P_FindSectorFromLineTag(line_t& line, const int32_t searchStart) noexcept;
int32_t P_FindMinSurroundingLight(sector_t& sector, const int32_t maxLightLevel) noexcept;
void P_CrossSpecialLine(line_t& line, mobj_t& mobj) noexcept;
void P_ShootSpecialLine(mobj_t& mobj, line_t& line) noexcept;
void P_PlayerInSpecialSector(player_t& player) noexcept;
void P_UpdateSpecials() noexcept;
bool EV_DoDonut(line_t& line) noexcept;
void G_ExitLevel() noexcept;
void G_SecretExitLevel(const int32_t nextMap) noexcept;
void P_SpawnSpecials() noexcept;
