#pragma once

#include "Doom/doomdef.h"

typedef uint32_t padbuttons_t;

// How many 60Hz ticks between menu movements - allow roughly 4 a second. This figure can be reset however
// if there is no input, so that rapidly pressing/releasing buttons can move things faster if required.
static constexpr int32_t MENU_MOVE_VBLANK_DELAY = 15;

// Identifier for one of the cheat sequences
enum cheatseq_t : int32_t {
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

#if PC_PSX_DOOM_MODS
    //--------------------------------------------------------------------------------------------------------------------------------------
    // A data structure used to hold all of the inputs and actions for a player for a particular frame.
    // This is now used in networking and as well as single player to drive all game inputs, instead of raw button codes.
    // It's advantages are that it supports analog motions and doesn't care about control sources or bindings at all - simplifying usage.
    // Note: most fields are only used depending on certain contexts, like when you are in a menu or in-game.
    //--------------------------------------------------------------------------------------------------------------------------------------
    struct TickInputs {
        // In-game: movement (0-1 range) and turning delta from analog sources and mouse input
        fixed_t analogForwardMove;
        fixed_t analogSideMove;
        angle_t analogTurn;

        // In-game: which weapon the player wants to try and directly switch to ('wp_nochange' if not switching).
        // This is used to allow direct switching to weapons for PC.
        uint8_t directSwitchToWeapon;

        // In-game: whether various action buttons are pressed
        uint8_t bTurnLeft : 1;
        uint8_t bTurnRight : 1;
        uint8_t bMoveForward : 1;
        uint8_t bMoveBackward : 1;
        uint8_t bStrafeLeft : 1;
        uint8_t bStrafeRight : 1;
        uint8_t bUse : 1;
        uint8_t bAttack : 1;

        uint8_t bRun : 1;
        uint8_t bStrafe : 1;
        uint8_t bPrevWeapon : 1;
        uint8_t bNextWeapon : 1;
        uint8_t bTogglePause : 1;
        uint8_t bToggleMap : 1;             // Toggle the automap on/off
        uint8_t bAutomapZoomIn : 1;
        uint8_t bAutomapZoomOut : 1;

        uint8_t bAutomapMoveLeft : 1;
        uint8_t bAutomapMoveRight : 1;
        uint8_t bAutomapMoveUp : 1;
        uint8_t bAutomapMoveDown : 1;
        uint8_t bAutomapPan : 1;            // Manually pan the automap
        uint8_t bRespawn : 1;               // Respawn in deathmatch
        uint8_t _unused1 : 2;

        // UI: whether various action buttons are pressed
        uint8_t bMenuUp : 1;
        uint8_t bMenuDown : 1;
        uint8_t bMenuLeft : 1;
        uint8_t bMenuRight : 1;
        uint8_t bMenuOk : 1;
        uint8_t bMenuStart : 1;
        uint8_t bMenuBack : 1;
        uint8_t bEnterPasswordChar : 1;

        uint8_t bDeletePasswordChar : 1;
        uint8_t _unused2 : 7;

        // Unused bytes to pad the struct out to 24 bytes: can be repurposed later if need be
        uint8_t _unused3;
        uint8_t _unused4;
    };
#endif

extern int32_t      gVBlanksUntilMenuMove[MAXPLAYERS];
extern bool         gbGamePaused;
extern int32_t      gPlayerNum;
extern int32_t      gMapNumToCheatWarpTo;
extern int32_t      gVramViewerTexPage;
extern thinker_t    gThinkerCap;
extern mobj_t       gMObjHead;

#if PC_PSX_DOOM_MODS
    extern TickInputs   gTickInputs[MAXPLAYERS];
    extern TickInputs   gOldTickInputs[MAXPLAYERS];
    extern TickInputs   gNextTickInputs;
    extern uint32_t     gTicButtons;
    extern uint32_t     gOldTicButtons;
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

#if PC_PSX_DOOM_MODS
    void P_GatherTickInputs(TickInputs& inputs) noexcept;
    void P_PsxButtonsToTickInputs(const padbuttons_t buttons, const padbuttons_t* const pControlBindings, TickInputs& inputs) noexcept;
#endif
