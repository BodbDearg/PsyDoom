#include "p_tick.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/w_wad.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/am_main.h"
#include "Doom/UI/o_main.h"
#include "Doom/UI/st_main.h"
#include "g_game.h"
#include "info.h"
#include "p_base.h"
#include "p_mobj.h"
#include "p_sight.h"
#include "p_spec.h"
#include "p_user.h"
#include "PcPsx/Assert.h"
#include "PcPsx/Config.h"
#include "PcPsx/DemoResult.h"
#include "PcPsx/Game.h"
#include "PcPsx/Input.h"
#include "PcPsx/ProgArgs.h"
#include "PcPsx/PsxPadButtons.h"
#include "PcPsx/Utils.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"
#include "Wess/psxspu.h"
#include "Wess/wessapi.h"

#include <algorithm>
#include <cmath>
#include <SDL.h>

// The number of buttons in a cheat sequence and a list of all the cheat sequences and their indices
static constexpr uint32_t CHEAT_SEQ_LEN = 8;

struct CheatSequence {
    uint16_t btns[CHEAT_SEQ_LEN];
};

// What buttons to press to activate each of the cheat sequences
static constexpr CheatSequence CHEAT_SEQUENCES[] = {
    { PAD_TRIANGLE, PAD_TRIANGLE, PAD_L2,     PAD_R2,     PAD_L2,       PAD_R2,       PAD_R1,     PAD_SQUARE },     // CHT_SEQ_SHOW_ALL_MAP_LINES
    { PAD_TRIANGLE, PAD_TRIANGLE, PAD_L2,     PAD_R2,     PAD_L2,       PAD_R2,       PAD_R1,     PAD_CIRCLE },     // CHT_SEQ_SHOW_ALL_MAP_THINGS
    { PAD_DOWN,     PAD_L2,       PAD_SQUARE, PAD_R1,     PAD_RIGHT,    PAD_L1,       PAD_LEFT,   PAD_CIRCLE },     // CHT_SEQ_GOD_MODE
    { PAD_CROSS,    PAD_TRIANGLE, PAD_L1,     PAD_UP,     PAD_DOWN,     PAD_R2,       PAD_LEFT,   PAD_LEFT   },     // CHT_SEQ_WEAPONS_AND_AMMO
    { PAD_UP,       PAD_UP,       PAD_UP,     PAD_UP,     PAD_UP,       PAD_UP,       PAD_UP,     PAD_R1     },     // PC-PSX: CHT_SEQ_NOCLIP (PSX: CHT_SEQ_UNUSED_04)
    { PAD_RIGHT,    PAD_LEFT,     PAD_R2,     PAD_R1,     PAD_TRIANGLE, PAD_L1,       PAD_CIRCLE, PAD_CROSS  },     // CHT_SEQ_LEVEL_WARP
    { PAD_LEFT,     PAD_LEFT,     PAD_LEFT,   PAD_LEFT,   PAD_LEFT,     PAD_LEFT,     PAD_LEFT,   PAD_LEFT   },     // CHT_SEQ_UNUSED_06
    { PAD_TRIANGLE, PAD_SQUARE,   PAD_UP,     PAD_LEFT,   PAD_DOWN,     PAD_RIGHT,    PAD_CROSS,  PAD_CIRCLE },     // PC-PSX: CHT_SEQ_VRAM_VIEWER (PSX: CHT_SEQ_UNUSED_07)
    { PAD_CROSS,    PAD_CROSS,    PAD_CROSS,  PAD_CROSS,  PAD_CROSS,    PAD_CROSS,    PAD_CROSS,  PAD_CROSS  },     // CHT_SEQ_UNUSED_08
    { PAD_L1,       PAD_R2,       PAD_L2,     PAD_R1,     PAD_RIGHT,    PAD_TRIANGLE, PAD_CROSS,  PAD_RIGHT  },     // CHT_SEQ_XRAY_VISION
    { PAD_CIRCLE,   PAD_CIRCLE,   PAD_CIRCLE, PAD_CIRCLE, PAD_CIRCLE,   PAD_CIRCLE,   PAD_CIRCLE, PAD_CIRCLE },     // CHT_SEQ_UNUSED_10
    { PAD_SQUARE,   PAD_SQUARE,   PAD_SQUARE, PAD_SQUARE, PAD_SQUARE,   PAD_SQUARE,   PAD_SQUARE, PAD_SQUARE }      // CHT_SEQ_UNUSED_11
};

static_assert(NUM_CHEAT_SEQ == C_ARRAY_SIZE(CHEAT_SEQUENCES));

int32_t     gVBlanksUntilMenuMove[MAXPLAYERS];      // How many 60 Hz ticks until we can move the cursor on the menu (one slot for each player)
bool        gbGamePaused;                           // Whether the game is currently paused by either player
int32_t     gPlayerNum;                             // Current player number being updated/processed
int32_t     gMapNumToCheatWarpTo;                   // What map the player currently has selected for cheat warp
int32_t     gVramViewerTexPage;                     // What page of texture memory to display in the VRAM viewer
thinker_t   gThinkerCap;                            // Dummy thinker which serves as both the head and tail of the thinkers list.
mobj_t      gMObjHead;                              // Dummy map object which serves as both the head and tail of the map objects linked list.

// PC-PSX: PSX gamepad button presses are now confined to just this player, and are just used for entering the original cheat sequences.
// For a networked game we use the tick inputs sent across with each packet.
#if PC_PSX_DOOM_MODS
    TickInputs  gTickInputs[MAXPLAYERS];        // Current tick inputs for the current 30 Hz tick
    TickInputs  gOldTickInputs[MAXPLAYERS];     // Previous tick inputs for the last 30 Hz tick
    TickInputs  gNextTickInputs;                // Network games only: what inputs we told the other player we will use next; sent ahead of time to reduce lag
    uint32_t    gTicButtons;                    // Currently PSX pad buttons for this player
    uint32_t    gOldTicButtons;                 // Previously pressed PSX buttons for this player    
#else
    uint32_t    gTicButtons[MAXPLAYERS];        // Currently pressed buttons by all players
    uint32_t    gOldTicButtons[MAXPLAYERS];     // Previously pressed buttons by all players
#endif

static int32_t      gCurCheatBtnSequenceIdx;                // What button press in the cheat sequence we are currently on
static uint16_t     gCheatSequenceBtns[CHEAT_SEQ_LEN];      // Cheat sequence buttons inputted by the player
static int32_t      gTicConOnPause;                         // What 60Hz tick we paused on, used to discount paused time on unpause
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
    for (mobj_t* pMObj = gMObjHead.next; pMObj != &gMObjHead; pMObj = pMObj->next) {
        if (pMObj->latecall) {
            pMObj->latecall(*pMObj);
        }
    }
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
    // PC-PSX: For this version of the game I'm allowing the user to warp to the secret levels!
    // If you're cheating you can more or less do anything anyway, so not much point in hiding these.
    #if PC_PSX_DOOM_MODS
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

        #if PC_PSX_DOOM_MODS
            const TickInputs& inputs = gTickInputs[playerIdx];
            const TickInputs& oldInputs = gOldTickInputs[playerIdx];
        #else
            const uint32_t padBtns = gTicButtons[playerIdx];
            const uint32_t oldPadBtns = gOldTicButtons[playerIdx];
        #endif
        
        // Toggling pause?
        #if PC_PSX_DOOM_MODS
            const bool bPauseJustPressed = (inputs.bTogglePause && (!oldInputs.bTogglePause));
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

                // Remember the tick we paused on and reset cheat button sequences
                gCurCheatBtnSequenceIdx = 0;
                gTicConOnPause = gTicCon;
                return;
            }

            // Otherwise restart cd handling and fade out cd audio
            psxcd_restart(0);

            while (psxcd_seeking_for_play()) {
                // Wait until the cdrom has stopped seeking to the current audio location.
                // Note: should NEVER be in here in this emulated environment: seek happens instantly!
            }

            psxspu_start_cd_fade(500, gCdMusicVol);
            S_Resume();

            // When the pause menu is opened the warp menu and vram viewer are initially disabled
            gPlayers[0].cheats &= ~(CF_VRAMVIEWER|CF_WARPMENU);

            // Restore previous tick counters on unpause
            gTicCon = gTicConOnPause;
            gLastTgtGameTicCount = gTicConOnPause >> VBLANK_TO_TIC_SHIFT;
        }

        // Showing the options menu if the game is paused and the options button has just been pressed.
        // Otherwise do not do any of the logic below...
        #if PC_PSX_DOOM_MODS
            const bool bMenuBackJustPressed = (inputs.bMenuBack && (!oldInputs.bMenuBack));
        #else
            const bool bMenuBackJustPressed = Utils::padBtnJustPressed(PAD_SELECT, padBtns, oldPadBtns);
        #endif

        if ((!bMenuBackJustPressed) || (!gbGamePaused))
            continue;
        
        // About to open up the options menu, disable these player cheats and present what we have to the screen
        player_t& player = gPlayers[playerIdx];
        player.cheats &= ~(CF_VRAMVIEWER|CF_WARPMENU);
        I_DrawPresent();

        // Run the options menu and possibly do one final draw, depending on the exit code.
        // TODO: what is the final draw for - screen fade?
        const gameaction_t optionsAction = MiniLoop(O_Init, O_Shutdown, O_Control, O_Drawer);
        
        if (optionsAction != ga_exit) {
            gGameAction = optionsAction;

            if (optionsAction == ga_restart || optionsAction == ga_exitdemo) {
                O_Drawer();
            }
        }
        
        return;
    }

    // Cheats are disallowed in a multiplayer game
    if (gNetGame != gt_single)
        return;

    // Grab inputs for the 1st player.
    // The rest of the cheat logic is for singleplayer mode only!
    #if PC_PSX_DOOM_MODS
        const uint32_t padBtns = gTicButtons;
        const uint32_t oldPadBtns = gOldTicButtons;

        const bool bMenuLeft = gTickInputs[0].bMenuLeft;
        const bool bMenuRight = gTickInputs[0].bMenuRight;
        const bool bJustPressedMenuLeft = (gTickInputs[0].bMenuLeft && (!gOldTickInputs[0].bMenuLeft));
        const bool bJustPressedMenuRight = (gTickInputs[0].bMenuRight && (!gOldTickInputs[0].bMenuRight));
        const bool bJustPressedMenuOk = (gTickInputs[0].bMenuOk && (!gOldTickInputs[0].bMenuOk));
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
    #if PC_PSX_DOOM_MODS
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
                    // PC-PSX: wraparound for convenience: provides a fast way to access opposite ends of the list
                    #if PC_PSX_DOOM_MODS
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
                    // PC-PSX: wraparound for convenience: provides a fast way to access opposite ends of the list
                    #if PC_PSX_DOOM_MODS
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

            if (gVramViewerTexPage > 10) {
                gVramViewerTexPage = 10;
            }
        }

        return;
    }

    // Only check for cheat sequences if the game is paused
    if (!gbGamePaused)
        return;

    // PC-PSX: allow cheats to be easily input using keyboard keys in dev builds
    #if PC_PSX_DOOM_MODS
        static cheatseq_t prevDevCheatSeq = (cheatseq_t) UINT32_MAX;
        cheatseq_t devCheatSeq = Utils::getDevCheatSequenceToExec();

        // Cheat key must be released in order to be used again.
        // This prevents us from rapidly cycling between on/off states for some cheats.
        if (devCheatSeq == prevDevCheatSeq) {
            devCheatSeq = (cheatseq_t) UINT32_MAX;
        } else {
            prevDevCheatSeq = devCheatSeq;
        }
    #endif

    // Only check for cheat sequences if some new buttons were pressed.
    // PC-PSX: also check for cheats if any dev cheats are input.
    if ((!padBtns) || (padBtns == oldPadBtns)) {
        #if PC_PSX_DOOM_MODS
            if (devCheatSeq >= NUM_CHEAT_SEQ)
                return;
        #else
            return;
        #endif
    }

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

        // PC-PSX: allow cheats to be easily input using keyboard keys in dev builds
        #if PC_PSX_DOOM_MODS
            if (devCheatSeq < NUM_CHEAT_SEQ && cheatSeqIdx == devCheatSeq) {
                // Force a match if dev cheat keys specify this cheat must be used!
                numMatchingBtns = CHEAT_SEQ_LEN;
            }
        #endif

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
                    for (mobj_t* pMObj = gMObjHead.next; pMObj != &gMObjHead; pMObj = pMObj->next) {
                        switch (pMObj->type) {
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
                    player.cheats |= CF_WARPMENU;
                    
                    if (gGameMap > maxCheatWarpLevel) {
                        gMapNumToCheatWarpTo = maxCheatWarpLevel;
                    } else {
                        gMapNumToCheatWarpTo = gGameMap;
                    }
                }   break;

                // Enable/disable 'xray vision' cheat
                case CHT_SEQ_XRAY_VISION:
                    player.cheats ^= CF_XRAYVISION;
                    break;

            #if PC_PSX_DOOM_MODS
                // Re-add in the VRAM viewer that was not available in the retail build
                case CHT_SEQ_VRAM_VIEWER: {
                    player.cheats ^= CF_VRAMVIEWER;
                }   break;

                // No-clip cheat
                case CHT_SEQ_NOCLIP: {
                    player.mo->flags ^= MF_NOCLIP;
                    gStatusBar.messageTicsLeft = 1;

                    if (player.mo->flags & MF_NOCLIP) {
                        gStatusBar.message = "Incorporeal Mode ON.";
                    } else {
                        gStatusBar.message = "Incorporeal Mode OFF.";
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

    #if PC_PSX_DOOM_MODS
        // PC-PSX: do framerate uncapped turning for the current player
        P_PlayerDoTurning();

        // PC-PSX: Don't do any updates if no vblanks have elapsed and it's not the first tick.
        // This is required now because of the potentially uncapped framerate.
        // Hold onto any input events until when we actually process a tick however...
        if ((!gbIsFirstTick) && (gElapsedVBlanks <= 0)) {
            gbKeepInputEvents = true;
            return gGameAction;
        }

        // PC-PSX: update the old values used for interpolation before simulating a new frame (if doing uncapped framerates)
        if (Config::gbUncapFramerate) {
            R_NextInterpolation();
        }
    #endif

    // Check for pause and cheats
    P_CheckCheats();

    // Run map entities and do status bar logic, if it's time
    if ((!gbGamePaused) && (gGameTic > gPrevGameTic)) {
        P_RunThinkers();
        P_CheckSights();
        P_RunMobjBase();
        P_RunMobjLate();
        P_UpdateSpecials();
        P_RespawnSpecials();
        ST_Ticker();
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

    return gGameAction;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does all drawing for main gameplay
//------------------------------------------------------------------------------------------------------------------------------------------
void P_Drawer() noexcept {
    // PC-PSX: no drawing in headless mode, but do advance the elapsed time.
    // Keep the framerate at 15-Hz for consistent demo playback (4 60Hz vblanks).
    #if PC_PSX_DOOM_MODS
        if (ProgArgs::gbHeadlessMode) {
            const int32_t demoTickVBlanks = (!gbPlayingPalDemo) ? VBLANKS_PER_TIC : 3;

            gTotalVBlanks += demoTickVBlanks;
            gLastTotalVBlanks = gTotalVBlanks;
            gElapsedVBlanks = demoTickVBlanks;
            return;
        }
    #endif

    I_IncDrawnFrameCount();

    // Draw either the automap or 3d view, depending on whether the automap is active or not
    if (gPlayers[gCurPlayerIndex].automapflags & AF_ACTIVE) {
        AM_Drawer();
    } else {
        R_RenderPlayerView();
    }

    ST_Drawer();
    I_SubmitGpuCmds();
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

    #if PC_PSX_DOOM_MODS
        // PC-PSX: initialize the new framerate uncapped turning system
        P_PlayerInitTurning();

        // PC-PSX: don't interpolate the first draw frame if doing uncapped framerates
        if (Config::gbUncapFramerate) {
            R_NextInterpolation();
        }
    #endif

    // Shouldn't be loading anything off the CDROM during gameplay after this point
    gbIsLevelDataCached = true;

    // Play music: for demos play the credits music cd track.
    // Otherwise play some sequencer music for the level.
    if (!gbDemoPlayback) {
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
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shuts down main gameplay
//------------------------------------------------------------------------------------------------------------------------------------------
void P_Stop([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // Finish up any GPU related work
    LIBGPU_DrawSync(0);

    // PC-PSX: save/check demo result if requested
    #if PC_PSX_DOOM_MODS
        if (gbDemoPlayback || gbDemoRecording) {
            if (ProgArgs::gSaveDemoResultFilePath[0]) {
                DemoResult::saveToJsonFile(ProgArgs::gSaveDemoResultFilePath);
            }
        }

        if (gbDemoPlayback && ProgArgs::gCheckDemoResultFilePath[0]) {
            if (!DemoResult::verifyMatchesJsonFileResult(ProgArgs::gCheckDemoResultFilePath)) {
                // If checking the demo result fails, return code '1' to indicate a failure
                std::exit(1);
            }
        }
    #endif
    
    // Stop all sounds and music
    S_StopAll();
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
}

#if PC_PSX_DOOM_MODS

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the inputs to use for the next tick
//------------------------------------------------------------------------------------------------------------------------------------------
void P_GatherTickInputs(TickInputs& inputs) noexcept {
    inputs = {};
    inputs.directSwitchToWeapon = wp_nochange;

    // TODO: don't hardcode the analog to digital threshold
    const float DIGITAL_THRESHOLD = 0.5f;

    // TODO: don't hardcode these bindings like this.
    // TODO: translate inputs bound to original PSX 'buttons' based on the control bindings.
    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_UP) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_W) ||
        Input::isControllerInputPressed(ControllerInput::BTN_DPAD_UP)
    ) {
        inputs.bMenuUp = true;
        inputs.bMoveForward = true;
        inputs.bAutomapMoveUp = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_DOWN) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_S) ||
        Input::isControllerInputPressed(ControllerInput::BTN_DPAD_DOWN)
    ) {
        inputs.bMenuDown = true;
        inputs.bMoveBackward = true;
        inputs.bAutomapMoveDown = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_LEFT) ||
        Input::isControllerInputPressed(ControllerInput::BTN_DPAD_LEFT)
    ) {
        inputs.bMenuLeft = true;
        inputs.bTurnLeft = true;
        inputs.bAutomapMoveLeft = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_RIGHT) ||
        Input::isControllerInputPressed(ControllerInput::BTN_DPAD_RIGHT)
    ) {
        inputs.bMenuRight = true;
        inputs.bTurnRight = true;
        inputs.bAutomapMoveRight = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_A)) {
        inputs.bStrafeLeft = true;
        inputs.bAutomapZoomIn = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_D)) {
        inputs.bStrafeRight = true;
        inputs.bAutomapZoomOut = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_MINUS) ||
        Input::isControllerInputPressed(ControllerInput::AXIS_TRIG_LEFT)
    ) {
        inputs.bAutomapZoomOut = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_EQUALS) ||
        Input::isControllerInputPressed(ControllerInput::AXIS_TRIG_RIGHT)
    ) {
        inputs.bAutomapZoomIn = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_PAGEDOWN) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_Q) ||
        Input::isControllerInputPressed(ControllerInput::BTN_LEFT_SHOULDER)
    ) {
        inputs.bPrevWeapon = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_PAGEUP) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_E) ||
        Input::isControllerInputPressed(ControllerInput::BTN_RIGHT_SHOULDER)
    ) {
        inputs.bNextWeapon = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_SPACE) ||
        Input::isControllerInputPressed(ControllerInput::BTN_B) ||
        Input::isMouseButtonPressed(MouseButton::RIGHT)
    ) {
        inputs.bUse = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_LSHIFT) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_RSHIFT) ||
        Input::isControllerInputPressed(ControllerInput::BTN_X) ||
        (Input::getControllerInputValue(ControllerInput::AXIS_TRIG_LEFT) >= DIGITAL_THRESHOLD)
    ) {
        inputs.bRun = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_LCTRL) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_RCTRL) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_F) ||
        Input::isControllerInputPressed(ControllerInput::BTN_Y) ||
        Input::isMouseButtonPressed(MouseButton::LEFT) ||
        (Input::getControllerInputValue(ControllerInput::AXIS_TRIG_RIGHT) >= DIGITAL_THRESHOLD)
    ) {
        inputs.bAttack = true;
        inputs.bRespawn = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_LALT) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_RALT) ||
        Input::isControllerInputPressed(ControllerInput::BTN_A)
    ) {
        inputs.bAutomapPan = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_PAUSE) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_P) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_RETURN) ||
        Input::isControllerInputPressed(ControllerInput::BTN_START)
    ) {
        inputs.bTogglePause = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_RETURN) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_LCTRL) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_RCTRL) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_SPACE) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_F) ||
        Input::isMouseButtonPressed(MouseButton::LEFT) ||
        Input::isControllerInputPressed(ControllerInput::BTN_A) ||
        (Input::getControllerInputValue(ControllerInput::AXIS_TRIG_RIGHT) >= DIGITAL_THRESHOLD)
    ) {
        inputs.bMenuOk = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_ESCAPE) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_TAB) ||
        Input::isMouseButtonPressed(MouseButton::RIGHT) ||
        Input::isControllerInputPressed(ControllerInput::BTN_B) ||
        Input::isControllerInputPressed(ControllerInput::BTN_BACK)
    ) {
        inputs.bMenuBack = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_TAB) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_M) ||
        Input::isControllerInputPressed(ControllerInput::BTN_BACK)
    ) {
        inputs.bToggleMap = true;
    }

    if (Input::isControllerInputPressed(ControllerInput::BTN_START)) {
        inputs.bMenuStart = true;
    }

    if (Input::getControllerInputValue(ControllerInput::AXIS_LEFT_Y) <= -DIGITAL_THRESHOLD) {
        inputs.bMenuUp = true;
    } else if (Input::getControllerInputValue(ControllerInput::AXIS_LEFT_Y) >= DIGITAL_THRESHOLD) { 
        inputs.bMenuDown = true;
    }

    if (Input::getControllerInputValue(ControllerInput::AXIS_RIGHT_Y) <= -DIGITAL_THRESHOLD) {
        inputs.bMenuUp = true;
    }  else if (Input::getControllerInputValue(ControllerInput::AXIS_RIGHT_Y) >= DIGITAL_THRESHOLD) { 
        inputs.bMenuDown = true;
    }

    if (Input::getControllerInputValue(ControllerInput::AXIS_LEFT_X) >= DIGITAL_THRESHOLD) {
        inputs.bMenuRight = true;
    } else if (Input::getControllerInputValue(ControllerInput::AXIS_LEFT_X) <= -DIGITAL_THRESHOLD) { 
        inputs.bMenuLeft = true;
    }

    if (Input::getControllerInputValue(ControllerInput::AXIS_RIGHT_X) >= DIGITAL_THRESHOLD) {
        inputs.bMenuRight = true;
    } else if (Input::getControllerInputValue(ControllerInput::AXIS_RIGHT_X) <= -DIGITAL_THRESHOLD) { 
        inputs.bMenuLeft = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_RETURN) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_SPACE) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_LCTRL) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_RCTRL) ||
        Input::isMouseButtonPressed(MouseButton::LEFT) ||
        Input::isControllerInputPressed(ControllerInput::BTN_A) ||
        (Input::getControllerInputValue(ControllerInput::AXIS_TRIG_RIGHT) >= DIGITAL_THRESHOLD)
    ) {
        inputs.bEnterPasswordChar = true;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_DELETE) ||
        Input::isKeyboardKeyPressed(SDL_SCANCODE_BACKSPACE) ||
        Input::isMouseButtonPressed(MouseButton::RIGHT) ||
        Input::isControllerInputPressed(ControllerInput::BTN_X) ||
        (Input::getControllerInputValue(ControllerInput::AXIS_TRIG_LEFT) >= DIGITAL_THRESHOLD)
    ) {
        inputs.bDeletePasswordChar = true;
    }

    inputs.analogForwardMove = (fixed_t)(Input::getAdjustedControllerInputValue(ControllerInput::AXIS_LEFT_Y, Config::gGamepadDeadZone) * FRACUNIT);
    inputs.analogSideMove = (fixed_t)(Input::getAdjustedControllerInputValue(ControllerInput::AXIS_LEFT_X, Config::gGamepadDeadZone) * FRACUNIT);

    // Direct weapon switching with the keyboard
    player_t& player = gPlayers[gCurPlayerIndex];

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_1)) {
        const weapontype_t nextWeapon = ((player.readyweapon == wp_chainsaw) || (!player.weaponowned[wp_chainsaw])) ? wp_fist : wp_chainsaw;
        inputs.directSwitchToWeapon = (uint8_t) nextWeapon;
    }

    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_2)) { inputs.directSwitchToWeapon = wp_pistol;         }
    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_3)) { inputs.directSwitchToWeapon = wp_shotgun;        }
    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_4)) { inputs.directSwitchToWeapon = wp_supershotgun;   }
    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_5)) { inputs.directSwitchToWeapon = wp_chaingun;       }
    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_6)) { inputs.directSwitchToWeapon = wp_missile;        }
    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_7)) { inputs.directSwitchToWeapon = wp_plasma;         }
    if (Input::isKeyboardKeyPressed(SDL_SCANCODE_8)) { inputs.directSwitchToWeapon = wp_bfg;            }

    // Direct weapon switching with the mouse wheel
    const int32_t mouseWheelMovement = (int32_t) Input::getMouseWheelAxisMovement(1);

    if (mouseWheelMovement != 0) {
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
            const int32_t nextWeaponIdx = std::clamp(selectedWeaponIdx + mouseWheelMovement, 0, numOwnedWeapons - 1);
            inputs.directSwitchToWeapon = (uint8_t) ownedWeapons[nextWeaponIdx];
        }
    }

    // Do one more update of the player's turning before gathering the input
    P_PlayerDoTurning();

    // Apply uncommited turning outside of the 30 Hz update loop from analog controllers, mouse and keyboard.
    // This all gets committed under the 'analogTurn' field now:
    inputs.analogTurn += gPlayerUncommittedAxisTurning;
    inputs.analogTurn += gPlayerUncommittedMouseTurning;

    gPlayerUncommittedAxisTurning = 0;
    gPlayerUncommittedMouseTurning = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Convert PlayStation gamepad buttons to tick inputs.
// Used to convert the original demo format inputs to the new 'TickInputs' format.
//------------------------------------------------------------------------------------------------------------------------------------------
void P_PsxButtonsToTickInputs(const padbuttons_t buttons, const padbuttons_t* const pControlBindings, TickInputs& inputs) noexcept {
    ASSERT(pControlBindings);
    inputs = {};
    inputs.directSwitchToWeapon = wp_nochange;

    inputs.psxMouseDx = -(int8_t)(buttons >> 16);
    inputs.psxMouseDy = -(int8_t)(buttons >> 24);

    if (buttons & PAD_UP) {
        inputs.bMoveForward = true;
        inputs.bMenuUp = true;
        inputs.bAutomapMoveUp = true;
    }

    if (buttons & PAD_DOWN) {
        inputs.bMoveBackward = true;
        inputs.bMenuDown = true;
        inputs.bAutomapMoveDown = true;
    }

    if (buttons & PAD_LEFT) {
        inputs.bTurnLeft = true;
        inputs.bMenuLeft = true;
        inputs.bAutomapMoveLeft = true;
    }

    if (buttons & PAD_RIGHT) {
        inputs.bTurnRight = true;
        inputs.bMenuRight = true;
        inputs.bAutomapMoveRight = true;
    }

    if (buttons & PAD_TRIANGLE) {
        inputs.bDeletePasswordChar = true;
        inputs.bMenuOk = true;
        inputs.bRespawn = true;
    }

    if (buttons & PAD_CROSS) {
        inputs.bEnterPasswordChar = true;
        inputs.bMenuOk = true;
        inputs.bAutomapPan = true;
        inputs.bRespawn = true;
    }

    if (buttons & PAD_SQUARE) {
        inputs.bEnterPasswordChar = true;
        inputs.bMenuOk = true;
        inputs.bRespawn = true;
    }

    if (buttons & PAD_CIRCLE) {
        inputs.bEnterPasswordChar = true;
        inputs.bMenuOk = true;
        inputs.bRespawn = true;
    }

    if (buttons & PAD_L1) {
        inputs.bAutomapZoomIn = true;
        inputs.bRespawn = true;
    }

    if (buttons & PAD_R1) {
        inputs.bAutomapZoomOut = true;
        inputs.bRespawn = true;
    }

    if (buttons & PAD_L2) {
        inputs.bRespawn = true;
    }

    if (buttons & PAD_R2) {
        inputs.bRespawn = true;
    }

    if (buttons & PAD_START) {
        inputs.bMenuStart = true;
        inputs.bTogglePause = true;
    }

    if (buttons & PAD_SELECT) {
        inputs.bMenuBack = true;
        inputs.bToggleMap = true;
    }

    if (buttons & PSX_MOUSE_ANY_BTNS) {
        inputs.bPsxMouseUse = true;
    }

    if (buttons & pControlBindings[cbind_attack]) {
        inputs.bAttack = true;
    }

    if (buttons & pControlBindings[cbind_strafe]) {
        inputs.bStrafe = true;
    }

    if (buttons & pControlBindings[cbind_run]) {
        inputs.bRun = true;
    }

    if (buttons & pControlBindings[cbind_use]) {
        inputs.bUse = true;
    }

    if (buttons & pControlBindings[cbind_strafe_left]) {
        inputs.bStrafeLeft = true;
    }

    if (buttons & pControlBindings[cbind_strafe_right]) {
        inputs.bStrafeRight = true;
    }

    if (buttons & pControlBindings[cbind_prev_weapon]) {
        inputs.bPrevWeapon = true;
    }

    if (buttons & pControlBindings[cbind_next_weapon]) {
        inputs.bNextWeapon = true;
    }

    if (buttons & pControlBindings[cbind_move_forward]) {
        inputs.bMoveForward = true;
        inputs.bAutomapMoveUp = true;
    }

    if (buttons & pControlBindings[cbind_move_backward]) {
        inputs.bMoveBackward = true;
        inputs.bAutomapMoveDown = true;
    }
}

#endif
