#include "p_tick.h"

#include "Asserts.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_texcache.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/psx_main.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/RendererVk/rv_data.h"
#include "Doom/RendererVk/rv_main.h"
#include "Doom/UI/am_main.h"
#include "Doom/UI/loadsave_main.h"
#include "Doom/UI/o_main.h"
#include "Doom/UI/st_main.h"
#include "g_game.h"
#include "info.h"
#include "p_base.h"
#include "p_local.h"
#include "p_mobj.h"
#include "p_sight.h"
#include "p_spec.h"
#include "p_user.h"
#include "PsyDoom/Cheats.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Controls.h"
#include "PsyDoom/DemoPlayer.h"
#include "PsyDoom/DemoResult.h"
#include "PsyDoom/DevMapAutoReloader.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/MapInfo.h"
#include "PsyDoom/PlayerPrefs.h"
#include "PsyDoom/ProgArgs.h"
#include "PsyDoom/PsxPadButtons.h"
#include "PsyDoom/SaveAndLoad.h"
#include "PsyDoom/ScriptingEngine.h"
#include "PsyDoom/Video.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"
#include "Wess/psxspu.h"
#include "Wess/wessapi.h"

#include <algorithm>
#include <cmath>

// The number of buttons in a cheat sequence and a list of all the cheat sequences and their indices
static constexpr int32_t CHEAT_SEQ_LEN = 8;

struct CheatSequence {
    uint16_t btns[CHEAT_SEQ_LEN];
};

// What buttons to press to activate each of the cheat sequences
static constexpr CheatSequence CHEAT_SEQUENCES[] = {
    { PAD_TRIANGLE, PAD_TRIANGLE, PAD_L2,     PAD_R2,     PAD_L2,       PAD_R2,       PAD_R1,     PAD_SQUARE },     // CHT_SEQ_SHOW_ALL_MAP_LINES
    { PAD_TRIANGLE, PAD_TRIANGLE, PAD_L2,     PAD_R2,     PAD_L2,       PAD_R2,       PAD_R1,     PAD_CIRCLE },     // CHT_SEQ_SHOW_ALL_MAP_THINGS
    { PAD_DOWN,     PAD_L2,       PAD_SQUARE, PAD_R1,     PAD_RIGHT,    PAD_L1,       PAD_LEFT,   PAD_CIRCLE },     // CHT_SEQ_GOD_MODE
    { PAD_CROSS,    PAD_TRIANGLE, PAD_L1,     PAD_UP,     PAD_DOWN,     PAD_R2,       PAD_LEFT,   PAD_LEFT   },     // CHT_SEQ_WEAPONS_AND_AMMO
    { PAD_UP,       PAD_UP,       PAD_UP,     PAD_UP,     PAD_UP,       PAD_UP,       PAD_UP,     PAD_R1     },     // PsyDoom: CHT_SEQ_NOCLIP (PSX: CHT_SEQ_UNUSED_04)
    { PAD_RIGHT,    PAD_LEFT,     PAD_R2,     PAD_R1,     PAD_TRIANGLE, PAD_L1,       PAD_CIRCLE, PAD_CROSS  },     // CHT_SEQ_LEVEL_WARP
    { PAD_LEFT,     PAD_LEFT,     PAD_LEFT,   PAD_LEFT,   PAD_LEFT,     PAD_LEFT,     PAD_LEFT,   PAD_LEFT   },     // CHT_SEQ_UNUSED_06
    { PAD_TRIANGLE, PAD_SQUARE,   PAD_UP,     PAD_LEFT,   PAD_DOWN,     PAD_RIGHT,    PAD_CROSS,  PAD_CIRCLE },     // PsyDoom: CHT_SEQ_VRAM_VIEWER (PSX: CHT_SEQ_UNUSED_07)
#if PSYDOOM_MODS
    { PAD_CROSS,    PAD_UP,       PAD_CROSS,  PAD_UP,     PAD_SQUARE,    PAD_SQUARE,  PAD_CROSS,  PAD_SQUARE },     // CHT_SEQ_NOTARGET
#else
    { PAD_CROSS,    PAD_CROSS,    PAD_CROSS,  PAD_CROSS,  PAD_CROSS,    PAD_CROSS,    PAD_CROSS,  PAD_CROSS  },     // CHT_SEQ_UNUSED_08
#endif
    { PAD_L1,       PAD_R2,       PAD_L2,     PAD_R1,     PAD_RIGHT,    PAD_TRIANGLE, PAD_CROSS,  PAD_RIGHT  },     // CHT_SEQ_XRAY_VISION
    { PAD_CIRCLE,   PAD_CIRCLE,   PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE,   PAD_CIRCLE,   PAD_CIRCLE, PAD_CIRCLE },     // CHT_SEQ_UNUSED_10
    { PAD_SQUARE,   PAD_SQUARE,   PAD_SQUARE, PAD_SQUARE, PAD_SQUARE,   PAD_SQUARE,   PAD_SQUARE, PAD_SQUARE }      // CHT_SEQ_UNUSED_11
};

static_assert(NUM_CHEAT_SEQ == C_ARRAY_SIZE(CHEAT_SEQUENCES));

int32_t     gVBlanksUntilMenuMove[MAXPLAYERS];      // How many 1 vblank ticks until we can move the cursor on the menu (one slot for each player)
bool        gbGamePaused;                           // Whether the game is currently paused by either player
int32_t     gPlayerNum;                             // Current player number being updated/processed
int32_t     gMapNumToCheatWarpTo;                   // What map the player currently has selected for cheat warp
int32_t     gVramViewerTexPage;                     // What page of texture memory to display in the VRAM viewer
thinker_t   gThinkerCap;                            // Dummy thinker which serves as both the head and tail of the thinkers list.
mobj_t      gMobjHead;                              // Dummy map object which serves as both the head and tail of the map objects linked list.
int32_t     gCurCheatBtnSequenceIdx;                // What button press in the cheat sequence we are currently on
int32_t     gTicConOnPause;                         // What 1 vblank tick we paused on, used to discount paused time on unpause

// PsyDoom: PSX gamepad button presses are now confined to just this player, and are just used for entering the original cheat sequences.
// For a networked game we use the tick inputs sent across with each packet.
#if PSYDOOM_MODS
    TickInputs  gTickInputs[MAXPLAYERS];        // Current tick inputs for the current 30 Hz tick
    TickInputs  gOldTickInputs[MAXPLAYERS];     // Previous tick inputs for the last 30 Hz tick
    TickInputs  gNextTickInputs;                // Network games only: what inputs we told the other player we will use next; sent ahead of time to reduce lag
    uint32_t    gTicButtons;                    // Currently PSX pad buttons for this player
    uint32_t    gOldTicButtons;                 // Previously pressed PSX buttons for this player
    bool        gbIgnoreCurrentAttack;          // A flag set to prevent accidental firing on returning to the game - causes attack to be ignored until the key is released
    bool        gbDoQuicksave;                  // A flag set to perform a quicksave at the next available opportunity (15 Hz tick)
    bool        gbDoQuickload;                  // A flag set to perform a quicksave at the next available opportunity (15 Hz tick)
#else
    uint32_t    gTicButtons[MAXPLAYERS];        // Currently pressed buttons by all players
    uint32_t    gOldTicButtons[MAXPLAYERS];     // Previously pressed buttons by all players
#endif

static uint16_t     gCheatSequenceBtns[CHEAT_SEQ_LEN];      // Cheat sequence buttons inputted by the player
static int32_t      gNumActiveThinkers;                     // Stat tracking count, no use other than that

//------------------------------------------------------------------------------------------------------------------------------------------
// Add a thinker to the linked list of thinkers
//------------------------------------------------------------------------------------------------------------------------------------------
void P_AddThinker(thinker_t& thinker) noexcept {
    gThinkerCap.prev->next = &thinker;
    thinker.next = &gThinkerCap;
    thinker.prev = gThinkerCap.prev;
    gThinkerCap.prev = &thinker;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Mark a thinker for removal from the list of thinkers.
// The removal happens later, during updates.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_RemoveThinker(thinker_t& thinker) noexcept {
    thinker.function = (think_t)(intptr_t) -1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Execute think logic for all thinkers
//------------------------------------------------------------------------------------------------------------------------------------------
void P_RunThinkers() noexcept {
    gNumActiveThinkers = 0;

    for (thinker_t* pThinker = gThinkerCap.next; pThinker != &gThinkerCap; pThinker = pThinker->next) {
        if ((intptr_t) pThinker->function == (intptr_t) -1) {
            // Time to remove this thinker, it's function has been zapped
            pThinker->next->prev = pThinker->prev;
            pThinker->prev->next = pThinker->next;
            Z_Free2(*gpMainMemZone, pThinker);
        } else {
            // Run the thinker if it has a think function and increment the active count stat
            if (pThinker->function) {
                pThinker->function(*pThinker);
            }

            gNumActiveThinkers++;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Execute the 'late call' update function for all map objects
//------------------------------------------------------------------------------------------------------------------------------------------
void P_RunMobjLate() noexcept {
    for (mobj_t* pMobj = gMobjHead.next; pMobj != &gMobjHead; pMobj = pMobj->next) {
        if (pMobj->latecall) {
            pMobj->latecall(*pMobj);
        }
    }
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom addition: returns the number of cheat buttons matched for the classic cheat key sequence currently being entered
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t P_GetNumMatchingCheatSeqBtns() noexcept {
    // Scan through all the cheats and see if the current input so far partially matches any of them.
    // Find the maximum number of cheat buttons matched from the available sequences.
    int32_t longestMatchingSeq = 0;

    for (int32_t cheatSeqIdx = 0; cheatSeqIdx < NUM_CHEAT_SEQ; ++cheatSeqIdx) {
        const CheatSequence& cheatSeq = CHEAT_SEQUENCES[cheatSeqIdx];
        int32_t matchLen = 0;

        while (matchLen < gCurCheatBtnSequenceIdx) {
            if (gCheatSequenceBtns[matchLen] != cheatSeq.btns[matchLen]) {
                matchLen = 0;
                break;
            }

            ++matchLen;
        }

        longestMatchingSeq = std::max(longestMatchingSeq, matchLen);
    }

    return longestMatchingSeq;
}
#endif  // #if PSYDOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Logic executed when unpausing the game.
// This was originally inline logic in 'P_CheckCheats' but I extracted it out for PsyDoom so it can be called in multiple places.
//------------------------------------------------------------------------------------------------------------------------------------------
static void P_OnGameUnpause() noexcept {
    // Restart cd handling and fade out cd audio
    psxcd_restart(0);

    while (psxcd_seeking_for_play()) {
        // Wait until the cdrom has stopped seeking to the current audio location.
        // Note: should NEVER be in here in this emulated environment: seek happens instantly!
    }

    psxspu_start_cd_fade(500, gCdMusicVol);
    S_Resume();

    // When the pause menu is opened the warp menu and vram viewer are initially disabled
    gPlayers[0].cheats &= ~(CF_VRAMVIEWER | CF_WARPMENU);

    // Restore previous tick counters on unpause
    gTicCon = gTicConOnPause;
    gLastTgtGameTicCount = d_rshift<VBLANK_TO_TIC_SHIFT>(gTicConOnPause);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Handles the following:
//  (1) Pausing/unpausing the game.
//  (2) Opening the options menu when the game is paused.
//  (3) Checking for cheat sequences in singleplayer.
//  (4) Controls for cheats that require them (VRAM viewer, level warp).
//------------------------------------------------------------------------------------------------------------------------------------------
void P_CheckCheats() noexcept {
    // The maximum level for the warp cheat.
    // PsyDoom: For this version of the game I'm allowing the user to warp to the secret levels!
    // If you're cheating you can more or less do anything anyway, so not much point in hiding these.
    #if PSYDOOM_MODS
        const int32_t maxCheatWarpLevel = Game::getNumMaps();
    #else
        const int32_t maxCheatWarpLevel = Game::getNumRegularMaps();
    #endif

    // Check for pause or options menu actions by any player.
    // Note that one player doing the action causes the action to happen for other players too.
    for (int32_t playerIdx = MAXPLAYERS - 1; playerIdx >= 0; --playerIdx) {
        // Skip this player if not in the game, otherwise grab inputs for the player
        if (!gbPlayerInGame[playerIdx])
            continue;

        #if PSYDOOM_MODS
            const TickInputs& inputs = gTickInputs[playerIdx];
            const TickInputs& oldInputs = gOldTickInputs[playerIdx];
        #else
            const uint32_t padBtns = gTicButtons[playerIdx];
            const uint32_t oldPadBtns = gOldTicButtons[playerIdx];
        #endif

        // Toggling pause?
        #if PSYDOOM_MODS
            const bool bPauseJustPressed = (inputs.fTogglePause() && (!oldInputs.fTogglePause()));
        #else
            const bool bPauseJustPressed = Utils::padBtnJustPressed(PAD_START, padBtns, oldPadBtns);
        #endif

        if (bPauseJustPressed) {
            gbGamePaused = (!gbGamePaused);

            // Handle the game being paused, if just pausing
            if (gbGamePaused) {
                // Pause all audio and also stop the chainsaw sounds.
                //
                // Note: Erick informed me that stopping chainsaw sounds was added in the 'Greatest Hits' (v1.1) re-release of DOOM (and also Final DOOM).
                // Stopping such sounds did NOT happen in the original release of PSX DOOM.
                psxcd_pause();
                wess_seq_stop(sfx_sawful);
                wess_seq_stop(sfx_sawhit);
                S_Pause();

                // PsyDoom: if there is any turning contained in tick inputs which hasn't been rolled into the player object then uncommit that turning now.
                // The tick inputs from this point on will essentially be ignored/discarded, but we don't want the player to lose turning made in between 30 Hz frames.
                #if PSYDOOM_MODS
                    P_UncommitTurningTickInputs();
                #endif

                // Remember the tick we paused on and reset cheat button sequences
                gCurCheatBtnSequenceIdx = 0;
                gTicConOnPause = gTicCon;

                // PsyDoom: allow 'pause' and 'menu back' to be executed with the same key stroke (typically the 'Esc' key)
                #if !PSYDOOM_MODS
                    return;
                #endif
            } else {
                // Note: this was originally inline logic, but PsyDoom needs to now call this in multiple places
                P_OnGameUnpause();
            }
        }

        // Showing the options menu if the game is paused and the options button has just been pressed.
        // Otherwise do not do any of the logic below...
        #if PSYDOOM_MODS
            const bool bMenuBackJustPressed = (
                (inputs.fMenuBack() && (!oldInputs.fMenuBack())) &&
                // PsyDoom: hack: ignore the 'back' operation if a cheat sequence is halfway entered and cheat buttons are being pressed.
                // This fixes an input conflict where the normal menu back button is mapped to a PSX button for cheat inputting.
                // This assumes the conflict will occur in the later part of the cheat sequence.
                ((Controls::getPSXCheatButtonBits() == 0) || (P_GetNumMatchingCheatSeqBtns() < 4))
            );
        #else
            const bool bMenuBackJustPressed = Utils::padBtnJustPressed(PAD_SELECT, padBtns, oldPadBtns);
        #endif

        if ((!bMenuBackJustPressed) || (!gbGamePaused))
            continue;

        // About to open up the options menu, disable these player cheats and present what we have to the screen
        player_t& player = gPlayers[playerIdx];
        player.cheats &= ~(CF_VRAMVIEWER|CF_WARPMENU);

        #if !PSYDOOM_MODS
            // PsyDoom: disable this as it causes a 1 frame black screen during transitions.
            // Not sure why this was here?
            I_DrawPresent();
        #endif

        // Run the options menu
        const gameaction_t optionsAction = MiniLoop(O_Init, O_Shutdown, O_Control, O_Drawer);

        #if PSYDOOM_MODS
            // PsyDoom: don't quit the game entirely if just exiting menus is requested
            const bool bExitGame = ((optionsAction != ga_exit) && (optionsAction != ga_exitmenus));
        #else
            const bool bExitGame = (optionsAction != ga_exit);
        #endif

        if (bExitGame) {
            gGameAction = optionsAction;

            // Do one final draw in some situations for screen fading
            if ((optionsAction == ga_restart) || (optionsAction == ga_exitdemo)) {
                O_Drawer();
            }
        } else {
            // PsyDoom: allow the game to be unpaused at the same time when exiting the options menu.
            // This allows the 'Esc' key (in the default bindings) to toggle between the options screen and gameplay (normally a 2 step operation).
            #if PSYDOOM_MODS
                if (gbUnpauseAfterOptionsMenu) {
                    gbGamePaused = false;
                    P_OnGameUnpause();
                }
            #endif
        }

        return;
    }

    // Cheats are disallowed in a multiplayer game
    if (gNetGame != gt_single)
        return;

    // PsyDoom: disallow cheats during demo playback or recording
    #if PSYDOOM_MODS
        if (gbDemoPlayback || gbDemoRecording)
            return;
    #endif

    // Grab inputs for the 1st player.
    // The rest of the cheat logic is for singleplayer mode only!
    #if PSYDOOM_MODS
        const uint32_t padBtns = gTicButtons;
        const uint32_t oldPadBtns = gOldTicButtons;

        const bool bMenuLeft = gTickInputs[0].fMenuLeft();
        const bool bMenuRight = gTickInputs[0].fMenuRight();
        const bool bJustPressedMenuLeft = (gTickInputs[0].fMenuLeft() && (!gOldTickInputs[0].fMenuLeft()));
        const bool bJustPressedMenuRight = (gTickInputs[0].fMenuRight() && (!gOldTickInputs[0].fMenuRight()));
        const bool bJustPressedMenuOk = (gTickInputs[0].fMenuOk() && (!gOldTickInputs[0].fMenuOk()));
    #else
        const uint32_t padBtns = gTicButtons[0];
        const uint32_t oldPadBtns = gOldTicButtons[0];

        const bool bMenuLeft = (padBtns & PAD_LEFT);
        const bool bMenuRight = (padBtns & PAD_RIGHT);
        const bool bJustPressedMenuLeft = (bMenuLeft && (padBtns != oldPadBtns));
        const bool bJustPressedMenuRight = (bMenuRight && (padBtns != oldPadBtns));
        const bool bJustPressedMenuOk = ((padBtns != oldPadBtns) && (padBtns & PAD_ACTION_BTNS));
    #endif

    // If there is no current input then you can move immediately on the next frame
    #if PSYDOOM_MODS
        if ((!bMenuLeft) && (!bMenuRight)) {
            gVBlanksUntilMenuMove[0] = 0;
        }
    #else
        if (padBtns == 0) {
            gVBlanksUntilMenuMove[0] = 0;
        }
    #endif

    // Are we showing the cheat warp menu?
    // If so then do the controls for that and exit.
    player_t& player = gPlayers[0];

    if (player.cheats & CF_WARPMENU) {
        gVBlanksUntilMenuMove[0] -= gPlayersElapsedVBlanks[0];

        if (gVBlanksUntilMenuMove[0] <= 0) {
            if (bMenuLeft) {
                gMapNumToCheatWarpTo--;

                if (gMapNumToCheatWarpTo <= 0) {
                    // PsyDoom: wraparound for convenience: provides a fast way to access opposite ends of the list
                    #if PSYDOOM_MODS
                        gMapNumToCheatWarpTo = maxCheatWarpLevel;
                    #else
                        gMapNumToCheatWarpTo = 1;
                    #endif
                }

                gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;
            }
            else if (bMenuRight) {
                gMapNumToCheatWarpTo++;

                if (gMapNumToCheatWarpTo > maxCheatWarpLevel) {
                    // PsyDoom: wraparound for convenience: provides a fast way to access opposite ends of the list
                    #if PSYDOOM_MODS
                        gMapNumToCheatWarpTo = 1;
                    #else
                        gMapNumToCheatWarpTo = maxCheatWarpLevel;
                    #endif
                }

                gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;
            }
        }

        // Are we initiating the the actual warp?
        if (bJustPressedMenuOk) {
            // Button pressed to initiate the level warp - kick it off!
            gGameAction = ga_warped;
            player.cheats &= (~CF_WARPMENU);
            gStartMapOrEpisode = gMapNumToCheatWarpTo;
            gGameMap = gMapNumToCheatWarpTo;
        }

        return;
    }

    // Are we showing the VRAM viewer?
    // If so then do the controls for that and exit.
    if (player.cheats & CF_VRAMVIEWER) {
        if (bJustPressedMenuLeft) {
            gVramViewerTexPage--;

            if (gVramViewerTexPage < 0) {
                gVramViewerTexPage = 0;
            }
        }
        else if (bJustPressedMenuRight) {
            gVramViewerTexPage++;

            #if PSYDOOM_MODS
                const int32_t numTexPages = I_GetNumTexCachePages();
            #else
                const int32_t numTexPages = 11;
            #endif

            if (gVramViewerTexPage >= numTexPages) {
                gVramViewerTexPage = numTexPages - 1;
            }
        }

        return;
    }

    // Only check for cheat sequences if the game is paused
    if (!gbGamePaused)
        return;

    // Only check for cheat sequences if some new buttons were pressed
    if ((!padBtns) || (padBtns == oldPadBtns))
        return;

    // Add the currently pressed buttons to the input
    gCheatSequenceBtns[gCurCheatBtnSequenceIdx] = (uint16_t) padBtns;
    gCurCheatBtnSequenceIdx++;

    // Scan through all the cheats and see if the current input matches any of them
    for (int32_t cheatSeqIdx = 0; cheatSeqIdx < NUM_CHEAT_SEQ; ++cheatSeqIdx) {
        // Try to match this cheat sequence against the current input
        const CheatSequence& curCheatSeq = CHEAT_SEQUENCES[cheatSeqIdx];
        int32_t numMatchingBtns = 0;

        while (numMatchingBtns < gCurCheatBtnSequenceIdx) {
            if (gCheatSequenceBtns[numMatchingBtns] != curCheatSeq.btns[numMatchingBtns])
                break;

            ++numMatchingBtns;
        }

        // Did all of the buttons match an entire cheat sequence?
        if (numMatchingBtns >= CHEAT_SEQ_LEN) {
            switch (cheatSeqIdx) {
                // Toggle show all map lines cheat
                case CHT_SEQ_SHOW_ALL_MAP_LINES: {
                    player.cheats ^= CF_ALLLINES;
                    gStatusBar.messageTicsLeft = 1;

                    if (player.cheats & CF_ALLLINES) {
                        gStatusBar.message = "Map All Lines ON.";
                    } else {
                        gStatusBar.message = "Map All Lines OFF.";
                    }
                }   break;

                // Toggle show all map things cheat
                case CHT_SEQ_SHOW_ALL_MAP_THINGS: {
                    player.cheats ^= CF_ALLMOBJ;
                    gStatusBar.messageTicsLeft = 1;

                    if (player.cheats & CF_ALLMOBJ) {
                        gStatusBar.message = "Map All Things ON.";
                    } else {
                        gStatusBar.message = "Map All Things OFF.";
                    }
                }   break;

                // Toggle god mode cheat
                case CHT_SEQ_GOD_MODE: {
                    player.cheats ^= CF_GODMODE;
                    gStatusBar.messageTicsLeft = 1;

                    if (player.cheats & CF_GODMODE) {
                        player.health = 100;
                        player.mo->health = 100;
                        gStatusBar.message = "All Powerful Mode ON.";
                    } else {
                        gStatusBar.message = "All Powerful Mode OFF.";
                    }
                }   break;

                // Weapons ammo and keys cheat
                case CHT_SEQ_WEAPONS_AND_AMMO: {
                    // Grant any keys that are present in the level.
                    // Run through the list of keys that are sitting around and give to the player...
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

                    gStatusBar.messageTicsLeft = 1;
                    gStatusBar.message = "Lots Of Goodies!";
                }   break;

                // Level warp cheat, bring up the warp menu
                case CHT_SEQ_LEVEL_WARP: {
                    // Level warp not possible on the demo version!
                    #if PSYDOOM_MODS
                        if (Game::gbIsDemoVersion)
                            return;
                    #endif

                    player.cheats |= CF_WARPMENU;

                    if (gGameMap > maxCheatWarpLevel) {
                        gMapNumToCheatWarpTo = maxCheatWarpLevel;
                    } else {
                        gMapNumToCheatWarpTo = gGameMap;
                    }
                }   break;

                // Enable/disable 'xray vision' cheat
                case CHT_SEQ_XRAY_VISION: {
                    player.cheats ^= CF_XRAYVISION;

                    // PsyDoom addition: show a message when this cheat is enabled and disabled to avoid confusion.
                    // Making this non-optional, as it doesn't detract from the original game experience.
                    #if PSYDOOM_MODS
                        gStatusBar.messageTicsLeft = 1;

                        if (player.cheats & CF_XRAYVISION) {
                            gStatusBar.message = "X-Ray Vision ON.";
                        } else {
                            gStatusBar.message = "X-Ray Vision OFF.";
                        }
                    #endif
                }   break;

            #if PSYDOOM_MODS
                // Re-add in the VRAM viewer that was not available in the retail build
                case CHT_SEQ_VRAM_VIEWER: {
                    player.cheats ^= CF_VRAMVIEWER;
                }   break;

                // No-clip cheat
                case CHT_SEQ_NOCLIP: {
                    player.cheats ^= CF_NOCLIP;
                    gStatusBar.messageTicsLeft = 1;

                    if (player.cheats & CF_NOCLIP) {
                        player.mo->flags |= MF_NOCLIP;
                        gStatusBar.message = "Incorporeal Mode ON.";
                    } else {
                        player.mo->flags &= ~MF_NOCLIP;
                        gStatusBar.message = "Incorporeal Mode OFF.";
                    }
                }   break;

                // No-target cheat
                case CHT_SEQ_NOTARGET: {
                    player.cheats ^= CF_NOTARGET;
                    gStatusBar.messageTicsLeft = 1;

                    if (player.cheats & CF_NOTARGET) {
                        gStatusBar.message = "Unperceivable Mode ON.";
                    } else {
                        gStatusBar.message = "Unperceivable Mode OFF.";
                    }
                }   break;
            #endif
            }

            // A full cheat sequence (8 buttons) was entered - we are done checking for cheats
            break;
        }
    }

    // Wraparound this if we need to!
    gCurCheatBtnSequenceIdx %= CHEAT_SEQ_LEN;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// High level tick/update logic for main gameplay
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t P_Ticker() noexcept {
    gGameAction = ga_nothing;

    #if PSYDOOM_MODS
        // PsyDoom: do framerate uncapped turning for the current player
        P_PlayerDoTurning();

        // PsyDoom: Don't do any updates if no vblanks have elapsed and it's not the first tick.
        // This is required now because of the potentially uncapped framerate.
        // Hold onto any input events until when we actually process a tick however...
        if ((!gbIsFirstTick) && (gElapsedVBlanks <= 0)) {
            gbKeepInputEvents = true;
            return gGameAction;
        }

        // PsyDoom: update the frame start times for interpolation and snap the player's position
        if (Config::gbUncapFramerate) {
            R_SnapPlayerInterpolation();
            R_InterpBeginPlayerFrame();

            if (gGameTic > gPrevGameTic) {
                R_InterpBeginWorldFrame();
            }
        }

        // PsyDoom: fix certain sequencer music tracks in 'Final Doom' not looping correctly: if the sequencer track has ended then restart it.
        // Only do this check when the game is not paused however, since that stops the music.
        if (!gbGamePaused) {
            // Make sure we're not playing a CD audio track like the 'Club Doom' music (that takes over from the map music)
            if (psxcd_get_playing_track() <= 0) {
                const MapInfo::Map* const pMap = MapInfo::getMap(gGameMap);
                const MapInfo::MusicTrack* const pSequencerTrack = (pMap && (!pMap->bPlayCdMusic)) ? MapInfo::getMusicTrack(pMap->music) : nullptr;

                // Has the current sequencer track stopped? If so then restart it...
                if (pSequencerTrack && (wess_seq_status(pSequencerTrack->sequenceNum) == SEQUENCE_INACTIVE)) {
                    wess_seq_trigger(pSequencerTrack->sequenceNum);
                }
            }
        }
    #endif

    // Check for pause and cheats
    #if PSYDOOM_MODS
        Cheats::update();
    #endif

    P_CheckCheats();

    #if PSYDOOM_MODS
        if (gGameTic > gPrevGameTic) {
            // PsyDoom: the player's view has not been pushed by the world (yet) for this 15 Hz tick.
            // The 'old' view z value also does not (yet) incorporate any pushing that may be done by the world this tick.
            // Note: deliberately run this logic even when paused, since interpolation is still active for a short time while paused.
            // This is because we are always a frame behind when interpolating...
            gViewPushedZ = 0;
            gbOldViewZIsPushed = false;
        }
    #endif

    // Run map entities and do status bar logic, if it's time
    if ((!gbGamePaused) && (gGameTic > gPrevGameTic)) {
        #if PSYDOOM_MODS
            // PsyDoom: execute any scheduled script actions and tick the external camera (if it's active)
            ScriptingEngine::runScheduledActions();

            if (gExtCameraTicsLeft > 0) {
                gExtCameraTicsLeft--;
            }
        #endif

        P_RunThinkers();
        P_CheckSights();
        P_RunMobjBase();
        P_RunMobjLate();
        P_UpdateSpecials();
        P_RespawnSpecials();
        ST_Ticker();

        // PsyDoom: allow the developer map auto-reloader to do it's thing and trigger a map reload if required
        #if PSYDOOM_MODS
            DevMapAutoReloader::update();
        #endif
    }

    // Run player logic
    for (gPlayerNum = 0; gPlayerNum < MAXPLAYERS; gPlayerNum += 1) {
        // Only if this player is in the game!
        if (!gbPlayerInGame[gPlayerNum])
            continue;

        // Respawn if we need to
        player_t& player = gPlayers[gPlayerNum];

        if (player.playerstate == PST_REBORN) {
            G_DoReborn(gPlayerNum);
        }

        // Do automap and player updates (controls, movement etc.)
        AM_Control(player);
        P_PlayerThink(player);
    }

    // PsyDoom: do quick save and load if requested in singleplayer (even if paused).
    // Only do them on 15 Hz (full game tick) boundaries however. Also this functionality is not available in the demo version.
    #if PSYDOOM_MODS
        if ((gNetGame == gt_single) && (gGameTic > gPrevGameTic) && (!Game::gbIsDemoVersion)) {
            if (gbDoQuicksave) {
                DoQuicksave();
            } else if (gbDoQuickload) {
                // Try to do a quick load and request warp to another map if that save is on another map.
                // Otherwise (if requested via 'exit all menus') unpause the game.
                const gameaction_t action = DoQuickload();

                if (action == ga_warped) {
                    gGameAction = ga_warped;
                } else if (action == ga_exitmenus) {
                    gbGamePaused = false;
                    P_OnGameUnpause();
                } else if (action == ga_restart) {
                    // Triggered if loading fails midway through the map setup process.
                    // Restart the current map to try and recover...
                    gGameAction = ga_restart;
                }
            }

            gbDoQuicksave = false;
            gbDoQuickload = false;
        }
    #endif

    return gGameAction;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does all drawing for main gameplay
//------------------------------------------------------------------------------------------------------------------------------------------
void P_Drawer() noexcept {
    // PsyDoom: no drawing in headless mode, but do advance the elapsed time.
    // Keep the framerate at the appropriate amount (for PAL or NTSC mode) for consistent demo playback.
    #if PSYDOOM_MODS
        if (ProgArgs::gbHeadlessMode) {
            const int32_t demoTickVBlanks = (Game::gSettings.bUsePalTimings) ? 3 : VBLANKS_PER_TIC;

            gTotalVBlanks += demoTickVBlanks;
            gLastTotalVBlanks = gTotalVBlanks;
            gElapsedVBlanks = demoTickVBlanks;
            return;
        }
    #endif

    I_IncDrawnFrameCount();

    // Draw either the automap or 3d view, depending on whether the automap is active or not.
    // PsyDoom: force the automap off if doing a camera.
    #if PSYDOOM_MODS
        const bool bShowAutomap = ((gPlayers[gCurPlayerIndex].automapflags & AF_ACTIVE) && (gExtCameraTicsLeft <= 0));
    #else
        const bool bShowAutomap = (gPlayers[gCurPlayerIndex].automapflags & AF_ACTIVE);
    #endif

    if (bShowAutomap) {
        AM_Drawer();
    } else {
        #if PSYDOOM_VULKAN_RENDERER
            // PsyDoom: use the new Vulkan renderer if enabled and bypass the classic one
            if (Video::isUsingVulkanRenderPath()) {
                RV_RenderPlayerView();
            } else {
                R_RenderPlayerView();
            }
        #else 
            R_RenderPlayerView();
        #endif
    }

    ST_Drawer();

    // PsyDoom: draw any enabled performance counters
    #if PSYDOOM_MODS
        I_DrawEnabledPerfCounters();
    #endif

    I_SubmitGpuCmds();

    // PsyDoom: moved presentation of rendering here to work better with the new Vulkan renderer.
    // Was previously done at the start of 'R_RenderPlayerView', before any world drawing was done.
    #if PSYDOOM_MODS
        I_DrawPresent();
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Starts up main gameplay
//------------------------------------------------------------------------------------------------------------------------------------------
void P_Start() noexcept {
    // Initialize some basic fields and the automap
    gbGamePaused = false;
    gValidCount = 1;

    AM_Start();
    M_ClearRandom();

    #if PSYDOOM_MODS
        // PsyDoom: initialize data-structures for the Vulkan renderer and the new framerate uncapped turning system
        #if PSYDOOM_VULKAN_RENDERER
            RV_InitLevelData();
        #endif

        P_PlayerInitTurning();
        gbIgnoreCurrentAttack = true;   // If fire is held while the level is loading then ignore the current attack

        // PsyDoom: don't interpolate the first draw frame if doing uncapped framerates
        if (Config::gbUncapFramerate) {
            R_InterpBeginPlayerFrame();
            R_InterpBeginWorldFrame();
            R_SnapPlayerInterpolation();
        }
    #endif

    // Shouldn't be loading anything off the CDROM during gameplay after this point
    gbIsLevelDataCached = true;

    // Play music: for classic demos (only!) play the credits music cd track.
    // Otherwise play some sequencer music for the level.
    #if PSYDOOM_MODS
        const bool bPlayNormalMapMusic = (!DemoPlayer::isPlayingAClassicDemo());
    #else
        const bool bPlayNormalMapMusic = (!gbDemoPlayback);
    #endif

    if (bPlayNormalMapMusic) {
        S_StartMusic();
    } else {
        psxcd_play_at_andloop(
            gCDTrackNum[cdmusic_credits_demo],
            gCdMusicVol,
            0,
            0,
            gCDTrackNum[cdmusic_credits_demo],
            gCdMusicVol,
            0,
            0
        );
    }

    // PsyDoom: start the level timer and auto save if requested
    #if PSYDOOM_MODS
        Game::startLevelTimer();

        if (gbAutoSaveOnLevelStart) {
            gbAutoSaveOnLevelStart = false;
            SaveGameForSlot(SaveFileSlot::AUTOSAVE, true);
        }
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down main gameplay
//------------------------------------------------------------------------------------------------------------------------------------------
void P_Stop([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // PsyDoom: end the level timer
    #if PSYDOOM_MODS
        Game::stopLevelTimer();
    #endif

    // Finish up any GPU related work
    LIBGPU_DrawSync(0);

    // PsyDoom: save/check demo result if requested
    #if PSYDOOM_MODS
        if (gbDemoPlayback || gbDemoRecording) {
            if (ProgArgs::gSaveDemoResultFilePath[0]) {
                DemoResult::saveToJsonFile(ProgArgs::gSaveDemoResultFilePath);
            }
        }

        if (gbDemoPlayback && ProgArgs::gCheckDemoResultFilePath[0]) {
            if (!DemoResult::verifyMatchesJsonFileResult(ProgArgs::gCheckDemoResultFilePath)) {
                // If demo produces an unexpected/wrong result set this flag to indicate a failure.
                // It will cause error code '1' to be returned upon the program exiting.
                gbCheckDemoResultFailed = true;
            }
        }
    #endif

    // Stop all sounds and music.
    // PsyDoom: don't stop all sounds, let them fade out naturally - otherwise the pistol sound on closing the main menu gets cut off.
    // We stop all sounds anyway when pausing, which is the only route out of the game.
    #if !PSYDOOM_MODS
        S_StopAll();
    #endif

    psxcd_stop();
    S_StopMusic();

    // Game is no longer paused and level data no longer cached
    gbGamePaused = false;
    gbIsLevelDataCached = false;

    // Finish up the level for each player
    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        if (gbPlayerInGame[playerIdx]) {
            G_PlayerFinishLevel(playerIdx);
        }
    }

    // PsyDoom: free data-structures for the Vulkan renderer, shutdown the scripting engine and developer auto map reloader
    #if PSYDOOM_VULKAN_RENDERER
        RV_FreeLevelData();
        ScriptingEngine::shutdown();
        DevMapAutoReloader::shutdown();
    #endif

    // PsyDoom: shut down the map object weak referencing system.
    // Also cleanup all map objects to remove all usage of weak reference counts before we shutdown the system.
    #if PSYDOOM_MODS
        if (gMobjHead.next) {
            while (gMobjHead.next != &gMobjHead) {
                P_RemoveMobj(*gMobjHead.next);
            }
        }

        P_ShutdownWeakRefs();
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper added for PsyDoom: returns the current gravity strength, adjusted for the game's current framerate (if that fix is enabled)
//------------------------------------------------------------------------------------------------------------------------------------------
fixed_t P_GetGravity() noexcept {
    fixed_t gravity = GRAVITY;

    #if PSYDOOM_MODS
        // Fix gravity to be consistent irrespective of framerate if this setting is enabled.
        // Makes 30 FPS gravity behave the same as 15 FPS gravity:
        if (Game::gSettings.bFixGravityStrength) {
            // Player 1's vblank count is what determines the game speed, even in a networked game
            const int32_t elapsedVblanks = gPlayersElapsedVBlanks[0];
            gravity = (gravity * elapsedVblanks) / 4;
        }
    #endif

    return gravity;
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: get the inputs to use for the next tick in the new 'TickInputs' format
//------------------------------------------------------------------------------------------------------------------------------------------
void P_GatherTickInputs(TickInputs& inputs) noexcept {
    // No inputs initially
    inputs.reset();

    // Gather basic inputs
    const float analogForwardMove = (
        Controls::getFloat(Controls::Binding::Analog_MoveForward) -
        Controls::getFloat(Controls::Binding::Analog_MoveBackward)
    );

    const float analogSideMove = (
        Controls::getFloat(Controls::Binding::Analog_StrafeRight) -
        Controls::getFloat(Controls::Binding::Analog_StrafeLeft)
    );

    inputs.setAnalogForwardMove((fixed_t)(analogForwardMove * (float) FRACUNIT));
    inputs.setAnalogSideMove((fixed_t)(analogSideMove * (float) FRACUNIT));
    inputs.fTurnLeft() = Controls::getBool(Controls::Binding::Digital_TurnLeft);
    inputs.fTurnRight() = Controls::getBool(Controls::Binding::Digital_TurnRight);
    inputs.fMoveForward() = Controls::getBool(Controls::Binding::Digital_MoveForward);
    inputs.fMoveBackward() = Controls::getBool(Controls::Binding::Digital_MoveBackward);
    inputs.fStrafeLeft() = Controls::getBool(Controls::Binding::Digital_StrafeLeft);
    inputs.fStrafeRight() = Controls::getBool(Controls::Binding::Digital_StrafeRight);
    inputs.fUse() = Controls::getBool(Controls::Binding::Action_Use);
    inputs.fAttack() = Controls::getBool(Controls::Binding::Action_Attack);
    inputs.fRun() = Controls::getBool(Controls::Binding::Modifier_Run);
    inputs.fStrafe() = Controls::getBool(Controls::Binding::Modifier_Strafe);
    inputs.fPrevWeapon() = Controls::getBool(Controls::Binding::Weapon_Previous);
    inputs.fNextWeapon() = Controls::getBool(Controls::Binding::Weapon_Next);
    inputs.fTogglePause() = Controls::isJustPressed(Controls::Binding::Toggle_Pause);
    inputs.fToggleMap() = Controls::isJustPressed(Controls::Binding::Toggle_Map);
    inputs.fAutomapZoomIn() = Controls::getBool(Controls::Binding::Automap_ZoomIn);
    inputs.fAutomapZoomOut() = Controls::getBool(Controls::Binding::Automap_ZoomOut);
    inputs.fAutomapMoveLeft() = Controls::getBool(Controls::Binding::Automap_MoveLeft);
    inputs.fAutomapMoveRight() = Controls::getBool(Controls::Binding::Automap_MoveRight);
    inputs.fAutomapMoveUp() = Controls::getBool(Controls::Binding::Automap_MoveUp);
    inputs.fAutomapMoveDown() = Controls::getBool(Controls::Binding::Automap_MoveDown);
    inputs.fAutomapPan() = Controls::getBool(Controls::Binding::Automap_Pan);
    inputs.fRespawn() = Controls::getBool(Controls::Binding::Action_Respawn);
    inputs.fMenuUp() = Controls::getBool(Controls::Binding::Menu_Up);
    inputs.fMenuDown() = Controls::getBool(Controls::Binding::Menu_Down);
    inputs.fMenuLeft() = Controls::getBool(Controls::Binding::Menu_Left);
    inputs.fMenuRight() = Controls::getBool(Controls::Binding::Menu_Right);
    inputs.fMenuOk() = Controls::getBool(Controls::Binding::Menu_Ok);
    inputs.fMenuStart() = Controls::getBool(Controls::Binding::Menu_Start);
    inputs.fMenuBack() = Controls::getBool(Controls::Binding::Menu_Back);
    inputs.fEnterPasswordChar() = Controls::getBool(Controls::Binding::Menu_EnterPasswordChar);
    inputs.fDeletePasswordChar() = Controls::getBool(Controls::Binding::Menu_DeletePasswordChar);
    inputs.fQuicksave() = Controls::getBool(Controls::Binding::Quicksave);
    inputs.fQuickload() = Controls::getBool(Controls::Binding::Quickload);

    // Allow toggle of autorun if the right button is pressed
    if (Controls::isJustPressed(Controls::Binding::Toggle_Autorun)) {
        PlayerPrefs::gbAlwaysRun = (!PlayerPrefs::gbAlwaysRun);
        gStatusBar.messageTicsLeft = 15;
        gStatusBar.message = (PlayerPrefs::gbAlwaysRun) ? "Autorun On" : "Autorun Off";
    }

    // If we're always running then this gets inverted
    if (PlayerPrefs::gbAlwaysRun) {
        inputs.fRun() = (!inputs.fRun());
    }

    // Should the currently held attack be ignored immediately after loading or saving a game and returning to gameplay?
    // If this is the case then once the user releases attack everything goes back to normal.
    if (gbIgnoreCurrentAttack) {
        if (Controls::getBool(Controls::Binding::Action_Attack)) {
            inputs.fAttack() = false;
        } else {
            gbIgnoreCurrentAttack = false;
        }
    }

    // Direct weapon switching
    player_t& player = gPlayers[gCurPlayerIndex];

    if (Controls::getBool(Controls::Binding::Weapon_FistChainsaw)) {
        const weapontype_t nextWeapon = ((player.readyweapon == wp_chainsaw) || (!player.weaponowned[wp_chainsaw])) ? wp_fist : wp_chainsaw;
        inputs.directSwitchToWeapon = (uint8_t) nextWeapon;
    }

    if (Controls::getBool(Controls::Binding::Weapon_Pistol))            { inputs.directSwitchToWeapon = wp_pistol;         }
    if (Controls::getBool(Controls::Binding::Weapon_Shotgun))           { inputs.directSwitchToWeapon = wp_shotgun;        }
    if (Controls::getBool(Controls::Binding::Weapon_SuperShotgun))      { inputs.directSwitchToWeapon = wp_supershotgun;   }
    if (Controls::getBool(Controls::Binding::Weapon_Chaingun))          { inputs.directSwitchToWeapon = wp_chaingun;       }
    if (Controls::getBool(Controls::Binding::Weapon_RocketLauncher))    { inputs.directSwitchToWeapon = wp_missile;        }
    if (Controls::getBool(Controls::Binding::Weapon_PlasmaRifle))       { inputs.directSwitchToWeapon = wp_plasma;         }
    if (Controls::getBool(Controls::Binding::Weapon_BFG))               { inputs.directSwitchToWeapon = wp_bfg;            }

    // Direct weapon switching via weapon scrolling - normally done with a mouse wheel
    const int32_t weaponScroll = (int32_t)(
        Controls::getFloat(Controls::Binding::Weapon_ScrollUp) -
        Controls::getFloat(Controls::Binding::Weapon_ScrollDown)
    );

    if (weaponScroll != 0) {
        // Get all of the owned weapons in a flat list in order of switching priority: this makes scrolling logic easier
        weapontype_t ownedWeapons[NUMWEAPONS] = { wp_nochange };
        int32_t numOwnedWeapons = 0;

        if (player.weaponowned[wp_fist])            { ownedWeapons[numOwnedWeapons++] = wp_fist;            }
        if (player.weaponowned[wp_chainsaw])        { ownedWeapons[numOwnedWeapons++] = wp_chainsaw;        }
        if (player.weaponowned[wp_pistol])          { ownedWeapons[numOwnedWeapons++] = wp_pistol;          }
        if (player.weaponowned[wp_shotgun])         { ownedWeapons[numOwnedWeapons++] = wp_shotgun;         }
        if (player.weaponowned[wp_supershotgun])    { ownedWeapons[numOwnedWeapons++] = wp_supershotgun;    }
        if (player.weaponowned[wp_chaingun])        { ownedWeapons[numOwnedWeapons++] = wp_chaingun;        }
        if (player.weaponowned[wp_missile])         { ownedWeapons[numOwnedWeapons++] = wp_missile;         }
        if (player.weaponowned[wp_plasma])          { ownedWeapons[numOwnedWeapons++] = wp_plasma;          }
        if (player.weaponowned[wp_bfg])             { ownedWeapons[numOwnedWeapons++] = wp_bfg;             }

        // Figure out the index of the currently selected weapon in this list
        const weapontype_t selectedWeapon = (player.pendingweapon != wp_nochange) ? player.pendingweapon : player.readyweapon;
        int32_t selectedWeaponIdx = 0;

        for (int32_t i = 0; i < numOwnedWeapons; ++i) {
            if (ownedWeapons[i] == selectedWeapon) {
                selectedWeaponIdx = i;
                break;
            }
        }

        // Get the index of the next weapon to select and schedule it to be selected
        if (numOwnedWeapons > 0) {
            const int32_t nextWeaponIdx = std::clamp(selectedWeaponIdx + weaponScroll, 0, numOwnedWeapons - 1);
            inputs.directSwitchToWeapon = (uint8_t) ownedWeapons[nextWeaponIdx];
        }
    }

    // Do one more update of the player's turning before gathering the input
    P_PlayerDoTurning();

    // Apply uncommited turning outside of the 30 Hz update loop from analog controllers, mouse and keyboard etc.
    // Don't do this if the game is paused however, hold on to the uncommitted turning in that case till the next tick.
    if (!gbGamePaused) {
        inputs.setAnalogTurn(inputs.getAnalogTurn() + gPlayerUncommittedTurning);
        gPlayerUncommittedTurning = 0;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom: Convert PlayStation gamepad buttons to tick inputs.
// Used to convert the original demo format inputs to the new 'TickInputs' format.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_PsxButtonsToTickInputs(const padbuttons_t buttons, const padbuttons_t* const pControlBindings, TickInputs& inputs) noexcept {
    ASSERT(pControlBindings);
    inputs.reset();

    inputs.psxMouseDx = -(int8_t)(buttons >> 16);
    inputs.psxMouseDy = -(int8_t)(buttons >> 24);

    if (buttons & PAD_UP) {
        inputs.fMoveForward() = true;
        inputs.fMenuUp() = true;
        inputs.fAutomapMoveUp() = true;
    }

    if (buttons & PAD_DOWN) {
        inputs.fMoveBackward() = true;
        inputs.fMenuDown() = true;
        inputs.fAutomapMoveDown() = true;
    }

    if (buttons & PAD_LEFT) {
        inputs.fTurnLeft() = true;
        inputs.fMenuLeft() = true;
        inputs.fAutomapMoveLeft() = true;
    }

    if (buttons & PAD_RIGHT) {
        inputs.fTurnRight() = true;
        inputs.fMenuRight() = true;
        inputs.fAutomapMoveRight() = true;
    }

    if (buttons & PAD_TRIANGLE) {
        inputs.fDeletePasswordChar() = true;
        inputs.fMenuOk() = true;
        inputs.fRespawn() = true;
    }

    if (buttons & PAD_CROSS) {
        inputs.fEnterPasswordChar() = true;
        inputs.fMenuOk() = true;
        inputs.fAutomapPan() = true;
        inputs.fRespawn() = true;
    }

    if (buttons & PAD_SQUARE) {
        inputs.fEnterPasswordChar() = true;
        inputs.fMenuOk() = true;
        inputs.fRespawn() = true;
    }

    if (buttons & PAD_CIRCLE) {
        inputs.fEnterPasswordChar() = true;
        inputs.fMenuOk() = true;
        inputs.fRespawn() = true;
    }

    if (buttons & PAD_L1) {
        inputs.fAutomapZoomIn() = true;
        inputs.fRespawn() = true;
    }

    if (buttons & PAD_R1) {
        inputs.fAutomapZoomOut() = true;
        inputs.fRespawn() = true;
    }

    if (buttons & PAD_L2) {
        inputs.fRespawn() = true;
    }

    if (buttons & PAD_R2) {
        inputs.fRespawn() = true;
    }

    if (buttons & PAD_START) {
        inputs.fMenuStart() = true;
        inputs.fTogglePause() = true;
    }

    if (buttons & PAD_SELECT) {
        inputs.fMenuBack() = true;
        inputs.fToggleMap() = true;
    }

    if (buttons & PSX_MOUSE_ANY_BTNS) {
        inputs.fPsxMouseUse() = true;
    }

    if (buttons & pControlBindings[cbind_attack]) {
        inputs.fAttack() = true;
    }

    if (buttons & pControlBindings[cbind_strafe]) {
        inputs.fStrafe() = true;
    }

    if (buttons & pControlBindings[cbind_run]) {
        inputs.fRun() = true;
    }

    if (buttons & pControlBindings[cbind_use]) {
        inputs.fUse() = true;
    }

    if (buttons & pControlBindings[cbind_strafe_left]) {
        inputs.fStrafeLeft() = true;
    }

    if (buttons & pControlBindings[cbind_strafe_right]) {
        inputs.fStrafeRight() = true;
    }

    if (buttons & pControlBindings[cbind_prev_weapon]) {
        inputs.fPrevWeapon() = true;
    }

    if (buttons & pControlBindings[cbind_next_weapon]) {
        inputs.fNextWeapon() = true;
    }

    if (buttons & pControlBindings[cbind_move_forward]) {
        inputs.fMoveForward() = true;
        inputs.fAutomapMoveUp() = true;
    }

    if (buttons & pControlBindings[cbind_move_backward]) {
        inputs.fMoveBackward() = true;
        inputs.fAutomapMoveDown() = true;
    }
}
#endif  // #if PSYDOOM_MODS
