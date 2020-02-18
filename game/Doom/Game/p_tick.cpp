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
#include "PcPsx/Utils.h"
#include "PsxVm/PsxVm.h"
#include "PsyQ/LIBETC.h"
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

const VmPtr<int32_t[MAXPLAYERS]>    gVBlanksUntilMenuMove(0x80077EF8);      // How many 60 Hz ticks until we can move the cursor on the menu (one slot for each player)
const VmPtr<bool32_t>               gbGamePaused(0x80077EC0);               // Whether the game is currently paused by either player
const VmPtr<int32_t>                gPlayerNum(0x800782EC);                 // Current player number being updated/processed
const VmPtr<int32_t>                gMapNumToCheatWarpTo(0x80078270);       // What map the player currently has selected for cheat warp
const VmPtr<int32_t>                gVramViewerTexPage(0x80077ED4);         // What page of texture memory to display in the VRAM viewer
const VmPtr<uint32_t[MAXPLAYERS]>   gTicButtons(0x80077F44);                // Currently pressed buttons by all players
const VmPtr<uint32_t[MAXPLAYERS]>   gOldTicButtons(0x80078214);             // Previously pressed buttons by all players
const VmPtr<thinker_t>              gThinkerCap(0x80096550);                // Dummy thinker which serves as both the head and tail of the thinkers list.
const VmPtr<mobj_t>                 gMObjHead(0x800A8E90);                  // Dummy map object which serves as both the head and tail of the map objects linked list.

static const VmPtr<int32_t>                     gCurCheatBtnSequenceIdx(0x80077FE4);    // What button press in the cheat sequence we are currently on
static const VmPtr<uint16_t[CHEAT_SEQ_LEN]>     gCheatSequenceBtns(0x800A91A4);         // Cheat sequence buttons inputted by the player
static const VmPtr<int32_t>                     gTicConOnPause(0x800782D8);             // What 60Hz tick we paused on, used to discount paused time on unpause

// Cheat activated message strings.
//
// TODO: eventually make these be actual C++ string constants.
// Can't to do that at the moment since these pointers need to be referenced by a 'VmPtr<T>', hence must be inside the executable itself.
static const VmPtr<const char> STR_Cheat_AllMapLines_On(0x80011060);
static const VmPtr<const char> STR_Cheat_AllMapLines_Off(0x80011074);
static const VmPtr<const char> STR_Cheat_AllMapThings_On(0x80011088);
static const VmPtr<const char> STR_Cheat_AllMapThings_Off(0x8001109C);
static const VmPtr<const char> STR_Cheat_GodMode_On(0x800110B0);
static const VmPtr<const char> STR_Cheat_GodMode_Off(0x800110C8);
static const VmPtr<const char> STR_Cheat_LotsOfGoodies(0x800110E0);
static const VmPtr<const char> STR_Cheat_On(0x8001106E);                    // Temp 'On' message for 'noclip' cheat.
static const VmPtr<const char> STR_Cheat_Off(0x80011082);                   // Temp 'Off' message for 'noclip' cheat.

void P_AddThinker() noexcept {
loc_80028C38:
    v0 = 0x80090000;                                    // Result = 80090000
    v0 = lw(v0 + 0x6550);                               // Load from: gThinkerCap[0] (80096550)
    sw(a0, v0 + 0x4);
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    sw(v0, a0 + 0x4);
    v1 = lw(v0);                                        // Load from: gThinkerCap[0] (80096550)
    sw(v1, a0);
    sw(a0, v0);                                         // Store to: gThinkerCap[0] (80096550)
    return;
}

void P_RemoveThinker() noexcept {
loc_80028C68:
    v0 = -1;                                            // Result = FFFFFFFF
    sw(v0, a0 + 0x8);
    return;
}

void P_RunThinkers() noexcept {
    sp -= 0x20;
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6554;                                       // Result = gThinkerCap[1] (80096554)
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    s0 = lw(v0);                                        // Load from: gThinkerCap[1] (80096554)
    v0 -= 4;                                            // Result = gThinkerCap[0] (80096550)
    sw(0, gp + 0xD14);                                  // Store to: gNumActiveThinkers (800782F4)
    s2 = -1;                                            // Result = FFFFFFFF
    if (s0 == v0) goto loc_80028D14;
    s1 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_80028CA8:
    v0 = lw(s0 + 0x8);
    a1 = s0;
    if (v0 != s2) goto loc_80028CE4;
    v1 = lw(s0 + 0x4);
    v0 = lw(s0);
    a0 = *gpMainMemZone;
    sw(v0, v1);
    v1 = lw(s0);
    v0 = lw(s0 + 0x4);
    sw(v0, v1 + 0x4);
    _thunk_Z_Free2();
    goto loc_80028D04;
loc_80028CE4:
    if (v0 == 0) goto loc_80028CF4;
    a0 = s0;
    ptr_call(v0);
loc_80028CF4:
    v0 = lw(gp + 0xD14);                                // Load from: gNumActiveThinkers (800782F4)
    v0++;
    sw(v0, gp + 0xD14);                                 // Store to: gNumActiveThinkers (800782F4)
loc_80028D04:
    s0 = lw(s0 + 0x4);
    if (s0 != s1) goto loc_80028CA8;
loc_80028D14:
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_RunMobjLate() noexcept {
    sp -= 0x20;
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x715C;                                       // Result = gMObjHead[5] (800A8EA4)
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    a0 = lw(v0);                                        // Load from: gMObjHead[5] (800A8EA4)
    v0 -= 0x14;                                         // Result = gMObjHead[0] (800A8E90)
    s1 = v0;                                            // Result = gMObjHead[0] (800A8E90)
    if (a0 == v0) goto loc_80028D7C;
loc_80028D58:
    v0 = lw(a0 + 0x18);
    s0 = lw(a0 + 0x14);
    if (v0 == 0) goto loc_80028D70;
    ptr_call(v0);
loc_80028D70:
    a0 = s0;
    if (a0 != s1) goto loc_80028D58;
loc_80028D7C:
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
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
        if (padBtnJustPressed(PAD_START, padBtns, oldPadBtns)) {
            *gbGamePaused = (!*gbGamePaused);

            // Handle the game being paused, if just pausing
            if (*gbGamePaused) {
                // Pause all audio and also stop the chainsaw sounds.
                //
                // Note: Erick also informed me that stopping the chainsaw sounds was actually only added in the 'Greatest Hits'
                // re-release of DOOM and also in Final DOOM; stopping the chainsaw sound was NOT done in the original DOOM release.
                // I guess the team took the opportunity with the re-releases and subsequent versions to fix/tweak this code?
                psxcd_pause();
                
                a0 = sfx_sawful;
                wess_seq_stop();
                
                a0 = sfx_sawhit;
                wess_seq_stop();

                S_Pause();

                // Remember the tick we paused on and reset cheat button sequences
                *gCurCheatBtnSequenceIdx = 0;
                *gTicConOnPause = *gTicCon;
                return;
            }

            // Otherwise restart audio
            a0 = 0;
            psxcd_restart();

            do {
                psxcd_seeking_for_play();
            } while (v0 != 0);

            a0 = 500;
            a1 = *gCdMusicVol;
            psxspu_start_cd_fade();

            S_Resume();

            // When the pause menu is opened the warp menu and vram viewer are initially disabled
            gPlayers[0].cheats &= ~(CF_VRAMVIEWER|CF_WARPMENU);

            // Restore previous tick counters on unpause
            *gTicCon = *gTicConOnPause;
            *gLastTgtGameTicCount = *gTicConOnPause / (REFRESHRATE / TICRATE);
        }

        // Showing the options menu if the game is paused and the options button has just been pressed.
        // Otherwise do not do any of the logic below...
        if ((!padBtnJustPressed(PAD_SELECT, padBtns, oldPadBtns)) || (!*gbGamePaused))
            continue;
        
        // About to open up the options menu, disable these player cheats and present what we have to the screen
        player_t& player = gPlayers[playerIdx];
        player.cheats &= ~(CF_VRAMVIEWER|CF_WARPMENU);
        I_DrawPresent();

        // Run the options menu and possibly do one final draw, depending on the exit code.
        // TODO: what is the final draw for - screen fade?
        const gameaction_t optionsAction = MiniLoop(O_Init, _thunk_O_Shutdown, _thunk_O_Control, O_Drawer);
        
        if (optionsAction != ga_exit) {
            *gGameAction = optionsAction;

            if (optionsAction == ga_restart || optionsAction == ga_exitdemo) {
                O_Drawer();
            }
        }
        
        return;
    }

    // Cheats are disallowed in a multiplayer game
    if (*gNetGame != gt_single)
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
                *gMapNumToCheatWarpTo -= 1;

                if (*gMapNumToCheatWarpTo <= 0) {
                    *gMapNumToCheatWarpTo = 1;
                }
                
                gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;
            }
            else if ((padBtns & PAD_RIGHT) != 0) {
                *gMapNumToCheatWarpTo += 1;

                if (*gMapNumToCheatWarpTo > MAX_CHEAT_WARP_LEVEL) {
                    *gMapNumToCheatWarpTo = MAX_CHEAT_WARP_LEVEL;
                }

                gVBlanksUntilMenuMove[0] = MENU_MOVE_VBLANK_DELAY;
            }
        }

        // Are we initiating the the actual warp?
        if (padBtns != oldPadBtns) {
            if (padBtns & PAD_ACTION_BTNS) {
                // Button pressed to initiate the level warp - kick it off!
                *gGameAction = ga_warped;
                player.cheats &= (~CF_WARPMENU);
                *gStartMapOrEpisode = *gMapNumToCheatWarpTo;
                *gGameMap = *gMapNumToCheatWarpTo;
            }
        }

        return;
    }

    // Are we showing the VRAM viewer?
    // If so then do the controls for that and exit.
    if (player.cheats & CF_VRAMVIEWER) {
        if (padBtns != oldPadBtns) {
            if (padBtns & PAD_LEFT) {
                --*gVramViewerTexPage;

                if (*gVramViewerTexPage < 0) {
                    *gVramViewerTexPage = 0;
                }
            }
            else if (padBtns & PAD_RIGHT) {
                ++*gVramViewerTexPage;

                if (*gVramViewerTexPage > 10) {
                    *gVramViewerTexPage = 10;
                }
            }
        }

        return;
    }

    // Only check for cheat sequences if the game is paused
    if (!*gbGamePaused)
        return;

    // PC-PSX: allow cheats to be easily input using keyboard keys in dev builds
    #if PC_PSX_DOOM_MODS
        static cheatseq_t prevDevCheatSeq = (cheatseq_t) UINT32_MAX;
        cheatseq_t devCheatSeq = getDevCheatSequenceToExec();

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
    gCheatSequenceBtns[*gCurCheatBtnSequenceIdx] = (uint16_t) padBtns;
    ++*gCurCheatBtnSequenceIdx;

    // Scan through all the cheats and see if the current input matches any of them
    for (uint32_t cheatSeqIdx = 0; cheatSeqIdx < NUM_CHEAT_SEQ; ++cheatSeqIdx) {
        // Try to match this cheat sequence against the current input
        const CheatSequence& curCheatSeq = CHEAT_SEQUENCES[cheatSeqIdx];
        int32_t numMatchingBtns = 0;

        while (numMatchingBtns < *gCurCheatBtnSequenceIdx) {
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
                    gStatusBar->messageTicsLeft = 1;

                    if (player.cheats & CF_ALLLINES) {
                        gStatusBar->message = STR_Cheat_AllMapLines_On;
                    } else {
                        gStatusBar->message = STR_Cheat_AllMapLines_Off;
                    }
                }   break;

                // Toggle show all map things cheat
                case CHT_SEQ_SHOW_ALL_MAP_THINGS: {
                    player.cheats ^= CF_ALLMOBJ;
                    gStatusBar->messageTicsLeft = 1;

                    if (player.cheats & CF_ALLMOBJ) {
                        gStatusBar->message = STR_Cheat_AllMapThings_On;
                    } else {
                        gStatusBar->message = STR_Cheat_AllMapThings_Off;
                    }
                }   break;

                // Toggle god mode cheat
                case CHT_SEQ_GOD_MODE: {
                    player.cheats ^= CF_GODMODE;
                    gStatusBar->messageTicsLeft = 1;

                    if (player.cheats & CF_GODMODE) {
                        player.health = 100;
                        player.mo->health = 100;
                        gStatusBar->message = STR_Cheat_GodMode_On;
                    } else {
                        gStatusBar->message = STR_Cheat_GodMode_Off;
                    }
                }   break;
                
                // Weapons ammo and keys cheat
                case CHT_SEQ_WEAPONS_AND_AMMO: {
                    // Grant any keys that are present in the level.
                    // Run through the list of keys that are sitting around and give to the player...
                    for (mobj_t* pMObj = gMObjHead->next.get(); pMObj != gMObjHead.get(); pMObj = pMObj->next.get()) {
                        switch (pMObj->type) {
                            case MT_MISC4: player.cards[it_bluecard]    = true; break;
                            case MT_MISC5: player.cards[it_redcard]     = true; break;
                            case MT_MISC6: player.cards[it_yellowcard]  = true; break;
                            case MT_MISC7: player.cards[it_yellowskull] = true; break;
                            case MT_MISC8: player.cards[it_redskull]    = true; break;
                            case MT_MISC9: player.cards[it_blueskull]   = true; break;
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

                    gStatusBar->messageTicsLeft = 1;
                    gStatusBar->message = STR_Cheat_LotsOfGoodies;
                }   break;

                // Level warp cheat, bring up the warp menu
                case CHT_SEQ_LEVEL_WARP: {
                    player.cheats |= CF_WARPMENU;
                    
                    if (*gGameMap > MAX_CHEAT_WARP_LEVEL) {
                        *gMapNumToCheatWarpTo = MAX_CHEAT_WARP_LEVEL;
                    } else {
                        *gMapNumToCheatWarpTo = *gGameMap;
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
                    gStatusBar->messageTicsLeft = 1;

                    if (player.mo->flags & MF_NOCLIP) {
                        gStatusBar->message = STR_Cheat_On;
                    } else {
                        gStatusBar->message = STR_Cheat_Off;
                    }
                }   break;
            #endif
            }
            
            // A full cheat sequence (8 buttons) was entered - we are done checking for cheats
            break;
        }
    }

    // Wraparound this if we need to!
    *gCurCheatBtnSequenceIdx %= CHEAT_SEQ_LEN;
}

void P_Ticker() noexcept {
    sp -= 0x20;
    sw(ra, sp + 0x1C);
    sw(s2, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    *gGameAction = ga_nothing;
    P_CheckCheats();
    v0 = *gbGamePaused;
    if (v0 != 0) goto loc_8002955C;
    v1 = *gGameTic;
    v0 = *gPrevGameTic;
    v0 = (i32(v0) < i32(v1));
    if (v0 == 0) goto loc_8002955C;
    s0 = 0x80090000;                                    // Result = 80090000
    s0 = lw(s0 + 0x6554);                               // Load from: gThinkerCap[1] (80096554)
    v0 = 0x80090000;                                    // Result = 80090000
    v0 += 0x6550;                                       // Result = gThinkerCap[0] (80096550)
    sw(0, gp + 0xD14);                                  // Store to: gNumActiveThinkers (800782F4)
    if (s0 == v0) goto loc_800294F8;
    s2 = -1;                                            // Result = FFFFFFFF
    s1 = v0;                                            // Result = gThinkerCap[0] (80096550)
loc_8002948C:
    v0 = lw(s0 + 0x8);
    a1 = s0;
    if (v0 != s2) goto loc_800294C8;
    v1 = lw(s0 + 0x4);
    v0 = lw(s0);
    a0 = *gpMainMemZone;
    sw(v0, v1);
    v1 = lw(s0);
    v0 = lw(s0 + 0x4);
    sw(v0, v1 + 0x4);
    _thunk_Z_Free2();
    goto loc_800294E8;
loc_800294C8:
    if (v0 == 0) goto loc_800294D8;
    a0 = s0;
    ptr_call(v0);
loc_800294D8:
    v0 = lw(gp + 0xD14);                                // Load from: gNumActiveThinkers (800782F4)
    v0++;
    sw(v0, gp + 0xD14);                                 // Store to: gNumActiveThinkers (800782F4)
loc_800294E8:
    s0 = lw(s0 + 0x4);
    if (s0 != s1) goto loc_8002948C;
loc_800294F8:
    P_CheckSights();
    P_RunMobjBase();
    a0 = 0x800B0000;                                    // Result = 800B0000
    a0 = lw(a0 - 0x715C);                               // Load from: gMObjHead[5] (800A8EA4)
    v0 = 0x800B0000;                                    // Result = 800B0000
    v0 -= 0x7170;                                       // Result = gMObjHead[0] (800A8E90)
    s1 = v0;                                            // Result = gMObjHead[0] (800A8E90)
    if (a0 == v0) goto loc_80029544;
loc_80029520:
    v0 = lw(a0 + 0x18);
    s0 = lw(a0 + 0x14);
    if (v0 == 0) goto loc_80029538;
    ptr_call(v0);
loc_80029538:
    a0 = s0;
    if (a0 != s1) goto loc_80029520;
loc_80029544:
    P_UpdateSpecials();
    P_RespawnSpecials();
    ST_Ticker();
loc_8002955C:
    *gPlayerNum = 0;
    s0 = 0x800B0000;                                    // Result = 800B0000
    s0 -= 0x7814;                                       // Result = gPlayer1[0] (800A87EC)
    s2 = 0x80080000;                                    // Result = 80080000
    s2 -= 0x7F54;                                       // Result = gbPlayerInGame[0] (800780AC)
    s1 = 2;                                             // Result = 00000002
loc_80029574:
    a0 = *gPlayerNum;
    v0 = a0 << 2;
    v0 += s2;
    v0 = lw(v0);
    if (v0 == 0) goto loc_800295BC;
    v0 = lw(s0 + 0x4);
    if (v0 != s1) goto loc_800295AC;
    G_DoReborn();
loc_800295AC:
    a0 = s0;
    AM_Control(*vmAddrToPtr<player_t>(a0));
    a0 = s0;
    P_PlayerThink();
loc_800295BC:
    v0 = *gPlayerNum;
    v0++;
    *gPlayerNum = v0;
    v0 = (i32(v0) < 2);
    s0 += 0x12C;
    if (v0 != 0) goto loc_80029574;
    v0 = *gGameAction;
    ra = lw(sp + 0x1C);
    s2 = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}

void P_Drawer() noexcept {
    sp -= 0x18;
    sw(ra, sp + 0x10);
    I_IncDrawnFrameCount();
    v0 = *gCurPlayerIndex;
    v1 = v0 << 2;
    v1 += v0;
    v0 = v1 << 4;
    v0 -= v1;
    v0 <<= 2;
    at = 0x800B0000;                                    // Result = 800B0000
    at -= 0x76F0;                                       // Result = gPlayer1[49] (800A8910)
    at += v0;
    v0 = lw(at);
    v0 &= 1;
    if (v0 == 0) goto loc_8002965C;
    AM_Drawer();
    goto loc_80029664;
loc_8002965C:
    R_RenderPlayerView();
loc_80029664:
    ST_Drawer();
    I_SubmitGpuCmds();
    ra = lw(sp + 0x10);
    sp += 0x18;
    return;
}

void P_Start() noexcept {
    sp -= 0x28;
    sw(s0, sp + 0x20);
    s0 = 1;                                             // Result = 00000001
    sw(ra, sp + 0x24);
    *gbGamePaused = false;
    at = 0x80070000;                                    // Result = 80070000
    sw(s0, at + 0x7BC4);                                // Store to: gValidCount (80077BC4)
    AM_Start();
    M_ClearRandom();
    v0 = 0x80080000;                                    // Result = 80080000
    v0 = *gbDemoPlayback;
    *gbIsLevelDataCached = (s0 != 0);
    a2 = 0;                                             // Result = 00000000
    if (v0 == 0) goto loc_80029700;
    v1 = 0x80070000;                                    // Result = 80070000
    v1 += 0x3E54;                                       // Result = CDTrackNum_Credits_Demo (80073E54)
    a1 = *gCdMusicVol;
    v0 = lw(v1);                                        // Load from: CDTrackNum_Credits_Demo (80073E54)
    sw(0, sp + 0x18);
    sw(0, sp + 0x1C);
    sw(v0, sp + 0x10);
    sw(a1, sp + 0x14);
    a0 = lw(v1);                                        // Load from: CDTrackNum_Credits_Demo (80073E54)
    a3 = 0;                                             // Result = 00000000
    psxcd_play_at_andloop();
    goto loc_80029708;
loc_80029700:
    S_StartMusicSequence();
loc_80029708:
    ra = lw(sp + 0x24);
    s0 = lw(sp + 0x20);
    sp += 0x28;
    return;
}

void P_Stop() noexcept {
    sp -= 0x20;
    a0 = 0;                                             // Result = 00000000
    sw(ra, sp + 0x18);
    sw(s1, sp + 0x14);
    sw(s0, sp + 0x10);
    _thunk_LIBGPU_DrawSync();
    s0 = 0;                                             // Result = 00000000
    S_Clear();
    psxcd_stop();
    S_StopMusicSequence();
    s1 = 0x80080000;                                    // Result = 80080000
    s1 -= 0x7F54;                                       // Result = gbPlayerInGame[0] (800780AC)
    *gbGamePaused = false;
    *gbIsLevelDataCached = false;
loc_80029760:
    v0 = lw(s1);
    s1 += 4;
    if (v0 == 0) goto loc_80029778;
    a0 = s0;
    G_PlayerFinishLevel();
loc_80029778:
    s0++;
    v0 = (i32(s0) < 2);
    if (v0 != 0) goto loc_80029760;
    ra = lw(sp + 0x18);
    s1 = lw(sp + 0x14);
    s0 = lw(sp + 0x10);
    sp += 0x20;
    return;
}
