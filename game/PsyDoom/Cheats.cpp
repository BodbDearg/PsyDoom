//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom cheats module: checks for cheat keyboard key sequences and developer cheat function keys (if enabled) to execute various cheats.
// There is a lot of duplication here with the cheat logic found in 'P_CheckCheats' since it was too messy to preserve the original logic
// there while integrating new keyboard key sequences which can be input without pausing the game.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Cheats.h"

#include "Asserts.h"
#include "Config/Config.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/info.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Game/p_user.h"
#include "Doom/UI/st_main.h"
#include "Game.h"
#include "Input.h"
#include "Wess/psxcd.h"
#include "Wess/wessapi.h"

#include <SDL.h>
#include <vector>

BEGIN_NAMESPACE(Cheats)

// Type for a function which executes a cheat
typedef void (*CheatAction)();

// Holds details for a cheat, what to execute, keys to press and how much of it has been input
struct Cheat {
    const Config::CheatKeySequence*     pSequence;
    CheatAction                         pAction;
    uint32_t                            curSequenceIdx;
    uint32_t                            sequenceLength;
};

// All of the registered cheats
static std::vector<Cheat> gCheats;

//------------------------------------------------------------------------------------------------------------------------------------------
// Registers a cheat key sequence
//------------------------------------------------------------------------------------------------------------------------------------------
static void addCheat(const Config::CheatKeySequence& sequence, const CheatAction action) noexcept {
    // Register the cheat
    Cheat& cheat = gCheats.emplace_back();
    cheat.pSequence = &sequence;
    cheat.pAction = action;
    cheat.curSequenceIdx = 0;
    cheat.sequenceLength = 0;

    for (uint32_t i = 0; i < Config::CheatKeySequence::MAX_KEYS; ++i) {
        if (sequence.keys[i] != 0) {
            ++cheat.sequenceLength;
        } else {
            break;
        }
    }

    // If there are no keys to type for the cheat then remove it entirely since it can never be triggered
    if (cheat.sequenceLength == 0) {
        gCheats.pop_back();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handles a keyboard key being pressed and updates all of the cheat sequences accordingly.
// Returns 'true' if a cheat was executed.
//------------------------------------------------------------------------------------------------------------------------------------------
static bool updateCheatKeySequencesForKeyPress(const uint16_t key) noexcept {
    bool bExecutedCheats = false;

    for (Cheat& cheat : gCheats) {
        // Did we press the right key?
        if (key == cheat.pSequence->keys[cheat.curSequenceIdx]) {
            ++cheat.curSequenceIdx;

            if (cheat.curSequenceIdx >= cheat.sequenceLength) {
                cheat.curSequenceIdx = 0;
                cheat.pAction();
                bExecutedCheats = true;
            }
        } else {
            cheat.curSequenceIdx = 0;
        }
    }

    return bExecutedCheats;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper that ensures the game is paused if it isn't already
//------------------------------------------------------------------------------------------------------------------------------------------
static void ensureGamePaused() noexcept {
    // Already paused?
    if (gbGamePaused)
        return;

    gbGamePaused = true;

    // Pause all audio and also stop the chainsaw sounds.
    //
    // Note: Erick informed me that stopping chainsaw sounds was added in the 'Greatest Hits' (v1.1) re-release of DOOM (and also Final DOOM).
    // Stopping such sounds did NOT happen in the original release of PSX DOOM.
    psxcd_pause();
    wess_seq_stop(sfx_sawful);
    wess_seq_stop(sfx_sawhit);
    S_Pause();

    // Remember the tick we paused on and reset cheat button sequences
    gCurCheatBtnSequenceIdx = 0;
    gTicConOnPause = gTicCon;

    // If there is any turning contained in tick inputs which hasn't been rolled into the player object then uncommit that turning now.
    // The tick inputs from this point on will essentially be ignored/discarded, but we don't want the player to lose turning made in between 30 Hz frames.
    P_UncommitTurningTickInputs();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Grants the specified player all weapons, max ammo and full armor
//------------------------------------------------------------------------------------------------------------------------------------------
static void grantAllWeaponsAmmoAndArmor(player_t& player) noexcept {
    // Grant mega armor
    player.armorpoints = 200;
    player.armortype = 2;

    // Grant all weapons and max ammo
    for (uint32_t weaponIdx = 0; weaponIdx < NUMWEAPONS; ++weaponIdx) {
        player.weaponowned[weaponIdx] = true;
    }

    for (uint32_t ammoIdx = 0; ammoIdx < NUMAMMO; ++ammoIdx) {
        player.ammo[ammoIdx] = player.maxammo[ammoIdx];
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Grants the specified player all keys currently in the level
//------------------------------------------------------------------------------------------------------------------------------------------
static void grantAllKeys(player_t& player) noexcept {
    // Run through the list of keys that are sitting around and give any found to the player:
    for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead; pMobj = pMobj->next) {
        switch (pMobj->type) {
            case MT_MISC4: player.cards[it_bluecard]    = true; break;
            case MT_MISC5: player.cards[it_redcard]     = true; break;
            case MT_MISC6: player.cards[it_yellowcard]  = true; break;
            case MT_MISC7: player.cards[it_yellowskull] = true; break;
            case MT_MISC8: player.cards[it_redskull]    = true; break;
            case MT_MISC9: player.cards[it_blueskull]   = true; break;

            default: break;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the toggle god mode cheat
//------------------------------------------------------------------------------------------------------------------------------------------
static void doToggleGodModeCheat() noexcept {
    player_t& player = gPlayers[gCurPlayerIndex];
    player.cheats ^= CF_GODMODE;
    gStatusBar.messageTicsLeft = 30;

    if (player.cheats & CF_GODMODE) {
        player.health = 100;
        player.mo->health = 100;
        gStatusBar.message = "All Powerful Mode ON.";
    } else {
        gStatusBar.message = "All Powerful Mode OFF.";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the toggle no-clip cheat
//------------------------------------------------------------------------------------------------------------------------------------------
static void doToggleNoClipCheat() noexcept {
    player_t& player = gPlayers[gCurPlayerIndex];
    player.cheats ^= CF_NOCLIP;
    gStatusBar.messageTicsLeft = 30;

    if (player.cheats & CF_NOCLIP) {
        player.mo->flags |= MF_NOCLIP;
        gStatusBar.message = "Incorporeal Mode ON.";
    } else {
        player.mo->flags &= ~MF_NOCLIP;
        gStatusBar.message = "Incorporeal Mode OFF.";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the cheat to open the level warp menu
//------------------------------------------------------------------------------------------------------------------------------------------
static void doOpenLevelWarpCheat() noexcept {
    // Level warp not possible on the demo version!
    if (Game::gbIsDemoVersion)
        return;

    ensureGamePaused();

    player_t& player = gPlayers[gCurPlayerIndex];
    player.cheats |= CF_WARPMENU;
    const int32_t maxCheatWarpLevel = Game::getNumMaps();

    if (gGameMap > maxCheatWarpLevel) {
        gMapNumToCheatWarpTo = maxCheatWarpLevel;
    } else {
        gMapNumToCheatWarpTo = gGameMap;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the cheat to give all weapons, max ammo, and max armor
//------------------------------------------------------------------------------------------------------------------------------------------
static void doWeaponsAndArmorCheat() noexcept {
    grantAllWeaponsAmmoAndArmor(gPlayers[gCurPlayerIndex]);

    gStatusBar.messageTicsLeft = 30;
    gStatusBar.message = "Lots Of Goodies! (No Keys)";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the cheat to give all weapons, max ammo, all keys and max armor
//------------------------------------------------------------------------------------------------------------------------------------------
static void doWeaponsKeysAndArmorCheat() noexcept {
    player_t& player = gPlayers[gCurPlayerIndex];
    grantAllWeaponsAmmoAndArmor(player);
    grantAllKeys(player);

    gStatusBar.messageTicsLeft = 30;
    gStatusBar.message = "Lots Of Goodies!";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the cheat to toggle the display of all map lines
//------------------------------------------------------------------------------------------------------------------------------------------
static void doToggleAllMapLinesCheat() noexcept {
    player_t& player = gPlayers[gCurPlayerIndex];
    player.cheats ^= CF_ALLLINES;
    gStatusBar.messageTicsLeft = 30;

    if (player.cheats & CF_ALLLINES) {
        gStatusBar.message = "Map All Lines ON.";
    } else {
        gStatusBar.message = "Map All Lines OFF.";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the cheat to toggle the display of all map things
//------------------------------------------------------------------------------------------------------------------------------------------
static void doToggleAllMapThingsCheat() noexcept {
    player_t& player = gPlayers[gCurPlayerIndex];
    player.cheats ^= CF_ALLMOBJ;
    gStatusBar.messageTicsLeft = 30;

    if (player.cheats & CF_ALLMOBJ) {
        gStatusBar.message = "Map All Things ON.";
    } else {
        gStatusBar.message = "Map All Things OFF.";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the cheat to toggle x-ray vision
//------------------------------------------------------------------------------------------------------------------------------------------
static void doToggleXRayVisionCheat() noexcept {
    player_t& player = gPlayers[gCurPlayerIndex];
    player.cheats ^= CF_XRAYVISION;
    gStatusBar.messageTicsLeft = 30;

    if (player.cheats & CF_XRAYVISION) {
        gStatusBar.message = "X-Ray Vision ON.";
    } else {
        gStatusBar.message = "X-Ray Vision OFF.";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the to cheat to open the VRAM viewer
//------------------------------------------------------------------------------------------------------------------------------------------
static void doToggleVramViewerCheat() noexcept {
    ensureGamePaused();
    player_t& player = gPlayers[gCurPlayerIndex];
    player.cheats ^= CF_VRAMVIEWER;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Executes the to cheat to toggle no-target
//------------------------------------------------------------------------------------------------------------------------------------------
static void doToggleNoTargetCheat() noexcept {
    player_t& player = gPlayers[gCurPlayerIndex];
    player.cheats ^= CF_NOTARGET;
    gStatusBar.messageTicsLeft = 30;

    if (player.cheats & CF_NOTARGET) {
        gStatusBar.message = "Unperceivable Mode ON.";
    } else {
        gStatusBar.message = "Unperceivable Mode OFF.";
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the cheats module and registers all cheats
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    addCheat(Config::gCheatKeys_GodMode,                doToggleGodModeCheat);
    addCheat(Config::gCheatKeys_NoClip,                 doToggleNoClipCheat);
    addCheat(Config::gCheatKeys_LevelWarp,              doOpenLevelWarpCheat);
    addCheat(Config::gCheatKeys_WeaponsKeysAndArmor,    doWeaponsKeysAndArmorCheat);
    addCheat(Config::gCheatKeys_WeaponsAndArmor,        doWeaponsAndArmorCheat);
    addCheat(Config::gCheatKeys_AllMapLinesOn,          doToggleAllMapLinesCheat);
    addCheat(Config::gCheatKeys_AllMapThingsOn,         doToggleAllMapThingsCheat);
    addCheat(Config::gCheatKeys_XRayVision,             doToggleXRayVisionCheat);
    addCheat(Config::gCheatKeys_VramViewer,             doToggleVramViewerCheat);
    addCheat(Config::gCheatKeys_NoTarget,               doToggleNoTargetCheat);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// De-initialize the cheat module
//------------------------------------------------------------------------------------------------------------------------------------------
void shutdown() noexcept {
    gCheats.clear();
    gCheats.shrink_to_fit();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Should be called once per actual game tick to check for newly input cheats due to key presses
//------------------------------------------------------------------------------------------------------------------------------------------
void update() noexcept {
    // Only allowed in singleplayer games!
    // Also disallowed during demo playback and recording.
    if (gNetGame != gt_single)
        return;

    if (gbDemoPlayback || gbDemoRecording)
        return;

    // Check for cheat key sequences just input
    bool bEnteredCheatKeySequence = false;

    for (const uint16_t key : Input::getKeyboardKeysJustPressed()) {
        if (updateCheatKeySequencesForKeyPress(key)) {
            bEnteredCheatKeySequence = true;
        }
    }

    // Check for developer mode cheat shortcuts
    if (Config::gbEnableDevCheatShortcuts) {
        const auto checkDevCheat = [&](const uint16_t keyboardKey, const CheatAction pAction) noexcept {
            if (Input::isKeyboardKeyJustPressed(keyboardKey)) {
                pAction();
            }
        };

        checkDevCheat(SDL_SCANCODE_F1, doToggleGodModeCheat);
        checkDevCheat(SDL_SCANCODE_F2, doToggleNoClipCheat);
        checkDevCheat(SDL_SCANCODE_F3, doWeaponsKeysAndArmorCheat);
        checkDevCheat(SDL_SCANCODE_F4, doOpenLevelWarpCheat);
        checkDevCheat(SDL_SCANCODE_F6, doToggleXRayVisionCheat);
        checkDevCheat(SDL_SCANCODE_F7, doToggleVramViewerCheat);
        checkDevCheat(SDL_SCANCODE_F8, doToggleNoTargetCheat);

        if (Config::gbEnableDevInPlaceReloadFunctionKey) {
            checkDevCheat(SDL_SCANCODE_F11, doInPlaceReloadCheat);
        }
    }

    // If we entered a cheat key sequence then kill all inputs to try and prevent conflicts.
    if (bEnteredCheatKeySequence) {
        Input::consumeEvents();
        gTickInputs[gCurPlayerIndex].reset();
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the 'in-place' level reload cheat which reloads the map but preserves the player's position and angle.
// This cheat is useful for quickly previewing map changes and fast iteration.
//------------------------------------------------------------------------------------------------------------------------------------------
void doInPlaceReloadCheat() noexcept {
    // Request the level be reloaded
    gGameAction = ga_warped;
    gStartMapOrEpisode = gGameMap;

    // Close the warp menu and VRAM viewer if open
    player_t& player = gPlayers[gCurPlayerIndex];
    player.cheats &= ~(CF_WARPMENU | CF_VRAMVIEWER);

    // Request an 'in-place' level reload and remember the player position and angle
    gbDoInPlaceLevelReload = true;

    const mobj_t& playerMobj = *player.mo;
    gInPlaceReloadPlayerX = playerMobj.x;
    gInPlaceReloadPlayerY = playerMobj.y;
    gInPlaceReloadPlayerZ = playerMobj.z;
    gInPlaceReloadPlayerAng = playerMobj.angle;
}

END_NAMESPACE(Cheats)
