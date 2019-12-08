#pragma once

void P_InitPicAnims() noexcept;
void getSide() noexcept;
void getSector() noexcept;
void twoSided() noexcept;
void getNextSector() noexcept;
void P_FindLowestFloorSurrounding() noexcept;
void P_FindHighestFloorSurrounding() noexcept;
void P_FindNextHighestFloor() noexcept;
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
