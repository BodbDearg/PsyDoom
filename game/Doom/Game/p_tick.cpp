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
#include "PcPsx/Config.h"
#include "PcPsx/DemoResult.h"
#include "PcPsx/ProgArgs.h"
#include "PcPsx/PsxPadButtons.h"
#include "PcPsx/Utils.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/psxcd.h"
#include "Wess/psxspu.h"
#include "Wess/wessapi.h"

// The maximum level for the warp cheat.
// PC-PSX: For this version of the game I'm allowing the user to warp to the secret levels!
// If you're cheating you can more or less do anything anyway, so not much point in hiding these.
#if PC_PSX_DOOM_MODS
    static constexpr int32_t MAX_CHEAT_WARP_LEVEL = NUM_MAPS;
#else
    static constexpr int32_t MAX_CHEAT_WARP_LEVEL = NUM_REGULAR_MAPS;
#endif

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
uint32_t    gTicButtons[MAXPLAYERS];                // Currently pressed buttons by all players
uint32_t    gOldTicButtons[MAXPLAYERS];             // Previously pressed buttons by all players
thinker_t   gThinkerCap;                            // Dummy thinker which serves as both the head and tail of the thinkers list.
mobj_t      gMObjHead;                              // Dummy map object which serves as both the head and tail of the map objects linked list.

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
    // Check for pause or options menu actions by any player.
    // Note that one player doing the action causes the action to happen for other players too.
    for (int32_t playerIdx = MAXPLAYERS - 1; playerIdx >= 0; --playerIdx) {
        // Skip this player if not in the game, otherwise grab inputs for the player
        if (!gbPlayerInGame[playerIdx])
            continue;

        const uint32_t padBtns = gTicButtons[playerIdx];
        const uint32_t oldPadBtns = gOldTicButtons[playerIdx];
        
        // Toggling pause?
        if (Utils::padBtnJustPressed(PAD_START, padBtns, oldPadBtns)) {
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
        if ((!Utils::padBtnJustPressed(PAD_SELECT, padBtns, oldPadBtns)) || (!gbGamePaused))
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
    const uint32_t padBtns = gTicButtons[0];
    const uint32_t oldPadBtns = gOldTicButtons[0];

    // If there is no current input then you can move immediately next frame
    if (padBtns == 0) {
        gVBlanksUntilMenuMove[0] = 0;
    }
    
    // Are we showing the cheat warp menu?
    // If so then do the controls for that and exit.
    player_t& player = gPlayers[0];
    
    if (player.cheats & CF_WARPMENU) {
        gVBlanksUntilMenuMove[0] -= gPlayersElapsedVBlanks[0];

        if (gVBlanksUntilMenuMove[0] <= 0) {
            if ((padBtns & PAD_LEFT) != 0) {
                gMapNumToCheatWarpTo--;

                if (gMapNumToCheatWarpTo <= 0) {
                    // PC-PSX: wraparound for convenience: provides a fast way to access opposite ends of the list
                    #if PC_PSX_DOOM_MODS
                        gMapNumToCheatWarpTo = MAX_CHEAT_WARP_LEVEL;
                    #else
                        gMapNumToCheatWarpTo = 1;
                    #endif
                }
                
                gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;
            }
            else if ((padBtns & PAD_RIGHT) != 0) {
                gMapNumToCheatWarpTo++;

                if (gMapNumToCheatWarpTo > MAX_CHEAT_WARP_LEVEL) {
                    // PC-PSX: wraparound for convenience: provides a fast way to access opposite ends of the list
                    #if PC_PSX_DOOM_MODS
                        gMapNumToCheatWarpTo = 1;
                    #else
                        gMapNumToCheatWarpTo = MAX_CHEAT_WARP_LEVEL;
                    #endif
                }

                gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;
            }
        }

        // Are we initiating the the actual warp?
        if (padBtns != oldPadBtns) {
            if (padBtns & PAD_ACTION_BTNS) {
                // Button pressed to initiate the level warp - kick it off!
                gGameAction = ga_warped;
                player.cheats &= (~CF_WARPMENU);
                gStartMapOrEpisode = gMapNumToCheatWarpTo;
                gGameMap = gMapNumToCheatWarpTo;
            }
        }

        return;
    }

    // Are we showing the VRAM viewer?
    // If so then do the controls for that and exit.
    if (player.cheats & CF_VRAMVIEWER) {
        if (padBtns != oldPadBtns) {
            if (padBtns & PAD_LEFT) {
                gVramViewerTexPage--;

                if (gVramViewerTexPage < 0) {
                    gVramViewerTexPage = 0;
                }
            }
            else if (padBtns & PAD_RIGHT) {
                gVramViewerTexPage++;

                if (gVramViewerTexPage > 10) {
                    gVramViewerTexPage = 10;
                }
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
                    
                    if (gGameMap > MAX_CHEAT_WARP_LEVEL) {
                        gMapNumToCheatWarpTo = MAX_CHEAT_WARP_LEVEL;
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
        // PC PSX: Don't do any updates if no vblanks have elapsed and it's not the first tick.
        // This is required now because of the potentially uncapped framerate.
        if ((!gbIsFirstTick) && (gElapsedVBlanks <= 0))
            return gGameAction;

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
            gTotalVBlanks += 4;
            gLastTotalVBlanks = gTotalVBlanks;
            gElapsedVBlanks = 4;
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

    // PC-PSX: don't interpolate the first draw frame if doing uncapped framerates
    #if PC_PSX_DOOM_MODS
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
