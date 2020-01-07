#pragma once

#include "Doom/doomdef.h"

// How many 60Hz ticks between menu movements - allow roughly 4 a second. This figure can be reset however
// if there is no input, so that rapidly pressing/releasing buttons can move things faster if required.
static constexpr int32_t MENU_MOVE_VBLANK_DELAY = 15;

extern const VmPtr<int32_t>                 gVBlanksUntilMenuMove;
extern const VmPtr<bool32_t>                gbGamePaused;
extern const VmPtr<int32_t>                 gMapNumToCheatWarpTo;
extern const VmPtr<int32_t>                 gVramViewerTexPage;
extern const VmPtr<uint32_t[MAXPLAYERS]>    gTicButtons;
extern const VmPtr<uint32_t[MAXPLAYERS]>    gOldTicButtons;

void P_AddThinker() noexcept;
void P_RemoveThinker() noexcept;
void P_RunThinkers() noexcept;
void P_RunMobjLate() noexcept;
void P_CheckCheats() noexcept;
void P_Ticker() noexcept;
void P_Drawer() noexcept;
void P_Start() noexcept;
void P_Stop() noexcept;
