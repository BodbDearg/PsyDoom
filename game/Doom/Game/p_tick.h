#pragma once

#include "Doom/doomdef.h"

typedef uint32_t padbuttons_t;

// How many 1 vblank ticks between menu movements - allow roughly 4 a second. This figure can be reset however
// if there is no input, so that rapidly pressing/releasing buttons can move things faster if required.
static constexpr int32_t MENU_MOVE_VBLANK_DELAY = 15;

// Identifier for one of the cheat sequences
enum cheatseq_t : int32_t {
    CHT_SEQ_SHOW_ALL_MAP_LINES,
    CHT_SEQ_SHOW_ALL_MAP_THINGS,
    CHT_SEQ_GOD_MODE,
    CHT_SEQ_WEAPONS_AND_AMMO,
// PsyDoom: turning this into a noclip cheat
#if PSYDOOM_MODS
    CHT_SEQ_NOCLIP,
#else
    CHT_SEQ_UNUSED_04,
#endif
    CHT_SEQ_LEVEL_WARP,
    CHT_SEQ_UNUSED_06,
// PsyDoom: reactivating the VRAM viewer - provide a cheat for it!
// This is the same cheat code for the VRAM viewer used by the "GEC PlayStation Doom: Master Edition" project also.
// PsyDoom: also adding a new 'notarget' cheat that causes enemies not to see the player.
#if PSYDOOM_MODS
    CHT_SEQ_VRAM_VIEWER,
    CHT_SEQ_NOTARGET,
#else
    CHT_SEQ_UNUSED_07,
    CHT_SEQ_UNUSED_08,
#endif
    CHT_SEQ_XRAY_VISION,
    CHT_SEQ_UNUSED_10,
    CHT_SEQ_UNUSED_11,
    NUM_CHEAT_SEQ
};

extern int32_t      gVBlanksUntilMenuMove[MAXPLAYERS];
extern bool         gbGamePaused;
extern int32_t      gPlayerNum;
extern int32_t      gMapNumToCheatWarpTo;
extern int32_t      gVramViewerTexPage;
extern thinker_t    gThinkerCap;
extern mobj_t       gMobjHead;
extern int32_t      gCurCheatBtnSequenceIdx;
extern int32_t      gTicConOnPause;

#if PSYDOOM_MODS
    extern TickInputs   gTickInputs[MAXPLAYERS];
    extern TickInputs   gOldTickInputs[MAXPLAYERS];
    extern TickInputs   gNextTickInputs;
    extern uint32_t     gTicButtons;
    extern uint32_t     gOldTicButtons;
    extern bool         gbIgnoreCurrentAttack;
    extern bool         gbDoQuicksave;
    extern bool         gbDoQuickload;
    extern int64_t      gLevelTimerStartElapsedUsecs;
#else
    extern uint32_t     gTicButtons[MAXPLAYERS];
    extern uint32_t     gOldTicButtons[MAXPLAYERS];
#endif

void P_AddThinker(thinker_t& thinker) noexcept;
void P_RemoveThinker(thinker_t& thinker) noexcept;
void P_RunThinkers() noexcept;
void P_RunMobjLate() noexcept;
void P_CheckCheats() noexcept;
gameaction_t P_Ticker() noexcept;
void P_Drawer() noexcept;
void P_Start() noexcept;
void P_Stop(const gameaction_t exitAction) noexcept;
fixed_t P_GetGravity() noexcept;

#if PSYDOOM_MODS
    void P_GatherTickInputs(TickInputs& inputs) noexcept;
    void P_PsxButtonsToTickInputs(const padbuttons_t buttons, const padbuttons_t* const pControlBindings, TickInputs& inputs) noexcept;
#endif
