#pragma once

#include "Doom/doomdef.h"

// How many 60Hz ticks between menu movements - allow roughly 4 a second. This figure can be reset however
// if there is no input, so that rapidly pressing/releasing buttons can move things faster if required.
static constexpr int32_t MENU_MOVE_VBLANK_DELAY = 15;

// Identifier for one of the cheat sequences
enum cheatseq_t : uint32_t {
    CHT_SEQ_SHOW_ALL_MAP_LINES,
    CHT_SEQ_SHOW_ALL_MAP_THINGS,
    CHT_SEQ_GOD_MODE,
    CHT_SEQ_WEAPONS_AND_AMMO,
// PC-PSX: turning this into a noclip cheat
#if PC_PSX_DOOM_MODS
    CHT_SEQ_NOCLIP,
#else
    CHT_SEQ_UNUSED_04,
#endif
    CHT_SEQ_LEVEL_WARP,
    CHT_SEQ_UNUSED_06,
// PC-PSX: reactivating the VRAM viewer - provide a cheat for it!
// This is the same cheat code for the VRAM viewer used by the "GEC PlayStation Doom: Master Edition" project also.
#if PC_PSX_DOOM_MODS
    CHT_SEQ_VRAM_VIEWER,
#else
    CHT_SEQ_UNUSED_07,
#endif
    CHT_SEQ_UNUSED_08,
    CHT_SEQ_XRAY_VISION,
    CHT_SEQ_UNUSED_10,
    CHT_SEQ_UNUSED_11,
    NUM_CHEAT_SEQ
};

extern const VmPtr<int32_t[MAXPLAYERS]>     gVBlanksUntilMenuMove;
extern const VmPtr<bool32_t>                gbGamePaused;
extern const VmPtr<int32_t>                 gPlayerNum;
extern const VmPtr<int32_t>                 gMapNumToCheatWarpTo;
extern const VmPtr<int32_t>                 gVramViewerTexPage;
extern const VmPtr<uint32_t[MAXPLAYERS]>    gTicButtons;
extern const VmPtr<uint32_t[MAXPLAYERS]>    gOldTicButtons;
extern const VmPtr<thinker_t>               gThinkerCap;
extern const VmPtr<mobj_t>                  gMObjHead;
extern const VmPtr<mobj_t>                  gMObjHead;

void P_AddThinker() noexcept;
void P_RemoveThinker() noexcept;
void P_RunThinkers() noexcept;
void P_RunMobjLate() noexcept;
void P_CheckCheats() noexcept;

gameaction_t P_Ticker() noexcept;
void _thunk_P_Ticker() noexcept;

void P_Drawer() noexcept;
void P_Start() noexcept;

void P_Stop(const gameaction_t exitAction) noexcept;
void _thunk_P_Stop() noexcept;
