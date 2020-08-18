#include "g_game.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/Base/z_zone.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/Renderer/r_local.h"
#include "Doom/Renderer/r_main.h"
#include "Doom/UI/f_finale.h"
#include "Doom/UI/in_main.h"
#include "doomdata.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_setup.h"
#include "p_tick.h"
#include "PcPsx/Endian.h"
#include "PcPsx/Game.h"
#include "PcPsx/Input.h"
#include "PcPsx/Utils.h"
#include "Wess/wessapi.h"

// Helper global holding the result of executing a gameloop via 'MiniLoop'.
// Sometimes this is used in preference to the return action, and sometimes it is used temporarily to hold the return action.
gameaction_t gGameAction;

// The current skill level. game type, game map, and the next/upcoming map
skill_t     gGameSkill;
gametype_t  gNetGame;
int32_t     gGameMap;
int32_t     gNextMap;

// State for each player and whether they are in the game
player_t gPlayers[MAXPLAYERS];
bool gbPlayerInGame[MAXPLAYERS];

// Current and previous game tick count (15 Hz ticks)
int32_t gGameTic;
int32_t gPrevGameTic;

// The last tick count we wanted to be at (15 Hz ticks).
// On the PSX if the game was running slow, then we might not have reached this amount.
int32_t gLastTgtGameTicCount;

// Player stats
int32_t gTotalKills;
int32_t gTotalItems;
int32_t gTotalSecret;

// Are we playing back or recording a demo?
bool gbDemoPlayback;
bool gbDemoRecording;

// Is the level being restarted?
bool gbIsLevelBeingRestarted;

// An empty map object initially assigned to players during network game setup, for net consistency checks.
// This is all zeroed out initially.
static mobj_t gEmptyMObj;

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays a loading message then loads the current map
//------------------------------------------------------------------------------------------------------------------------------------------
void G_DoLoadLevel() noexcept {
    // Draw the loading plaque
    I_DrawLoadingPlaque(gTex_LOADING, 95, 109, Game::getTexPalette_LOADING());

    // Wait for the pistol and barrel explode menu sounds to stop playing
    while ((wess_seq_status(sfx_barexp) == SEQUENCE_PLAYING) || (wess_seq_status(sfx_pistol) == SEQUENCE_PLAYING)) {
        // PsyDoom: need to update sound to escape this loop, also ensure the window stays responsive etc.
        #if PSYDOOM_MODS
            Utils::doPlatformUpdates();
            Utils::threadYield();
        #endif
    }

    // Loading sound and music
    S_LoadMapSoundAndMusic(gGameMap);

    // Initialize the state of each player if required
    {
        const gameaction_t gameAction = gGameAction;

        for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
            if (gbPlayerInGame[playerIdx]) {
                player_t& player = gPlayers[playerIdx];

                if ((gameAction == ga_restart) || (gameAction == ga_warped) || (player.playerstate == PST_DEAD)) {
                    player.playerstate = PST_REBORN;
                }
            }
        }
    }

    // And and setup the level, then verify the heap after all that is done
    P_SetupLevel(gGameMap, gGameSkill);
    Z_CheckHeap(*gpMainMemZone);

    // No action set upon starting a level
    gGameAction = ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Some end of level logic for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void G_PlayerFinishLevel(int32_t playerIdx) noexcept {
    // Remove cards and powerups
    D_memset(gPlayers[playerIdx].powers, std::byte(0), sizeof(player_t::powers));
    D_memset(gPlayers[playerIdx].cards, std::byte(0), sizeof(player_t::cards));

    // Clear blending flags on the player.
    // PsyDoom: preserve the noclip cheat also, if active.
    mobj_t& mobj = *gPlayers[playerIdx].mo;
    mobj.flags &= (~MF_ALL_BLEND_FLAGS);

    // Clearing out a few other fields
    gPlayers[playerIdx].extralight = 0;
    gPlayers[playerIdx].fixedcolormap = 0;
    gPlayers[playerIdx].damagecount = 0;
    gPlayers[playerIdx].bonuscount = 0;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Re-initializes player state after respawning
//------------------------------------------------------------------------------------------------------------------------------------------
void G_PlayerReborn(const int32_t playerIdx) noexcept {
    // Save the old player stats and reset player state completely
    player_t& player = gPlayers[playerIdx];

    const uint32_t frags = player.frags;
    const uint32_t killcount = player.killcount;
    const uint32_t itemcount = player.itemcount;
    const uint32_t secretcount = player.secretcount;

    D_memset(&player, std::byte(0), sizeof(player_t));

    // Initialize player state and restore the previously saved stats
    player.health = MAXHEALTH;
    player.attackdown = 1;
    player.usedown = true;
    player.playerstate = PST_LIVE;
    player.pendingweapon = wp_pistol;
    player.readyweapon = wp_pistol;
    player.weaponowned[wp_fist] = true;
    player.weaponowned[wp_pistol] = true;
    player.ammo[am_clip] = 50;
    player.frags = frags;
    player.killcount = killcount;
    player.itemcount = itemcount;
    player.secretcount = secretcount;

    for (int32_t ammoIdx = 0; ammoIdx < NUMAMMO; ++ammoIdx) {
        player.maxammo[ammoIdx] = gMaxAmmo[ammoIdx];
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Logic for respawning the player in co-op or deathmatch games
//------------------------------------------------------------------------------------------------------------------------------------------
void G_DoReborn(const int32_t playerIdx) noexcept {
    // No respawning in single player
    if (gNetGame == gt_single) {
        gGameAction = ga_died;
        return;
    }

    // Disassociate the player's corpse from the player
    if (gPlayers[playerIdx].mo->player) {
        gPlayers[playerIdx].mo->player = nullptr;
    }

    // Try and pick a point to spawn at
    mobj_t& playerMobj = *gPlayers[playerIdx].mo;
    mapthing_t* pChosenSpawnPt = nullptr;

    if (gNetGame == gt_deathmatch) {
        // Deathmatch game: try to choose a random spawn point that's good
        const int32_t numSpawnPts = (int32_t)(gpDeathmatchP - gDeathmatchStarts);

        for (int32_t attemptNum = 0; attemptNum < 16; ++attemptNum) {
            mapthing_t& spawnPt = gDeathmatchStarts[P_Random() % numSpawnPts];
            
            // If we found a good position we can stop.
            // Make sure the map object type is correct for this player index also.
            if (P_CheckPosition(playerMobj, (fixed_t) spawnPt.x << FRACBITS, (fixed_t) spawnPt.y << FRACBITS)) {
                pChosenSpawnPt = &spawnPt;
                pChosenSpawnPt->type = 1 + (int16_t) playerIdx;
                break;
            }
        }
    }
    else {
        // Cooperative game: try to spawn at this player's preferred/assigned spawn point first
        mapthing_t& prefSpawnPt = gPlayerStarts[playerIdx];

        // If that didn't work out try another player's spawn point
        if (!P_CheckPosition(playerMobj, (fixed_t) prefSpawnPt.x << FRACBITS, (fixed_t) prefSpawnPt.y << FRACBITS)) {
            for (int32_t spawnPtIdx = 0; spawnPtIdx < MAXPLAYERS; ++spawnPtIdx) {
                mapthing_t& otherSpawnPt = gPlayerStarts[spawnPtIdx];

                // If we found a good position we can stop.
                // Make sure the map object type is correct for this player index also.
                if (P_CheckPosition(playerMobj, (fixed_t) otherSpawnPt.x << FRACBITS, (fixed_t) otherSpawnPt.y << FRACBITS)) {
                    pChosenSpawnPt = &otherSpawnPt;
                    pChosenSpawnPt->type = 1 + (int16_t) playerIdx;
                    break;
                }
            }
        }
    }

    // If we didn't find any good spawn point then just use the default start for this player
    if (!pChosenSpawnPt) {
        pChosenSpawnPt = &gPlayerStarts[playerIdx];
    }

    // Spawn the player
    P_SpawnPlayer(*pChosenSpawnPt);

    // Restore all cooperative starts back to having their previous type, if we modified them.
    // The co-op spawn logic assumes the type is correct for the corresponding player index.
    for (int32_t spawnPtIdx = 0; spawnPtIdx < MAXPLAYERS; ++spawnPtIdx) {
        gPlayerStarts[spawnPtIdx].type = 1 + (int16_t) spawnPtIdx;
    }
    
    // Figure out what subsector the player will spawn in
    const fixed_t spawnX = (fixed_t) pChosenSpawnPt->x << FRACBITS;
    const fixed_t spawnY = (fixed_t) pChosenSpawnPt->y << FRACBITS;
    subsector_t* const pSubsec = R_PointInSubsector(spawnX, spawnY);

    // This mask wraps the fine angle for the map thing and restricts it to the 8 diagonal directions
    constexpr uint32_t FINE_ANGLE_MASK = FINEANGLES - (FINEANGLES / 8);

    // Compute the fine angle for the map thing and wrap + restrict to 8 directions.
    // The angle in the wad is from 0-360, so we must scale and adjust accordingly.
    const uint32_t fineAngle = (((uint32_t) pChosenSpawnPt->angle * FINEANGLES) / 360) & FINE_ANGLE_MASK;
    
    // Spawn teleport fog a bit away from the player in the direction the player is facing (clamped to 8 directions)
    mobj_t* const pSpawnedThing = P_SpawnMobj(
        spawnX + gFineCosine[fineAngle] * 20,
        spawnY + gFineSine[fineAngle] * 20,
        pSubsec->sector->floorheight,
        MT_TFOG
    );
    
    // Play the teleport sound
    S_StartSound(pSpawnedThing, sfx_telept);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Mark the level as done
//------------------------------------------------------------------------------------------------------------------------------------------
void G_CompleteLevel() noexcept {
    gGameAction = ga_completed;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Common game setup logic for both demos and regular gameplay
//------------------------------------------------------------------------------------------------------------------------------------------
void G_InitNew(const skill_t skill, const int32_t mapNum, const gametype_t gameType) noexcept {
    // Resetting memory management related stuff and RNGs
    gbIsLevelBeingRestarted = false;
    gLockedTexPagesMask &= 1;
    I_PurgeTexCache();

    Z_FreeTags(*gpMainMemZone, PU_CACHE | PU_ANIMATION | PU_LEVSPEC | PU_LEVEL);
    M_ClearRandom();

    // Save game params: note that in multiplayer these might be overriden later
    gGameMap = mapNum;
    gGameSkill = skill;
    gNetGame = gameType;

    // PsyDoom: determine the game settings for single player games.
    // Note: in a multiplayer game these will have already been determined before this point, hence no determination here.
    // These settings may also be overwritten by demo playback, if a demo will be played.
    #if PSYDOOM_MODS
        if (gNetGame == gt_single) {
            Game::getConfigGameSettings(Game::gSettings);
        }
    #endif

    // Mark all players as reborn
    {
        player_t* pPlayer = &gPlayers[MAXPLAYERS - 1];
        player_t* const pFirstPlayer = &gPlayers[0];

        while (pPlayer >= pFirstPlayer) {
            pPlayer->playerstate = PST_REBORN;
            --pPlayer;
        }
    }

    // Clear the empty map object and assign to both players initially.
    // This is used for network consistency checks:
    D_memset(&gEmptyMObj, std::byte(0), sizeof(mobj_t));
    gPlayers[0].mo = &gEmptyMObj;
    gPlayers[1].mo = &gEmptyMObj;

    // Set some player status flags and controls stuff
    gbPlayerInGame[0] = true;

    if (gameType == gt_single) {
        // PsyDoom: we don't need sets of pointers to the bindings for each player anymore.
        // We only store PSX control bindings for the current player in any game type (networked or single player).
        #if !PSYDOOM_MODS
            gpPlayerCtrlBindings[0] = gCtrlBindings;
        #endif

        gbPlayerInGame[1] = false;
    }
    else if ((gameType == gt_deathmatch) || (gameType == gt_coop)) {
        gbPlayerInGame[1] = true;
    }

    // Not recording or playing back a demo (yet)
    gbDemoRecording = false;
    gbDemoPlayback = false;

    // Patching some monster states depending on difficulty
    if (skill == sk_nightmare) {
        gStates[S_SARG_ATK1].tics = 2;
        gStates[S_SARG_ATK2].tics = 2;
        gStates[S_SARG_ATK3].tics = 2;
        gMObjInfo[MT_SERGEANT].speed = 15;
        gMObjInfo[MT_BRUISERSHOT].speed = 40 * FRACUNIT;
        gMObjInfo[MT_HEADSHOT].speed = 40 * FRACUNIT;
        gMObjInfo[MT_TROOPSHOT].speed = 40 * FRACUNIT;
    } else {
        gStates[S_SARG_ATK1].tics = 4;
        gStates[S_SARG_ATK2].tics = 4;
        gStates[S_SARG_ATK3].tics = 4;
        gMObjInfo[MT_SERGEANT].speed = 10;
        gMObjInfo[MT_BRUISERSHOT].speed = 30 * FRACUNIT;
        gMObjInfo[MT_HEADSHOT].speed = 20 * FRACUNIT;
        gMObjInfo[MT_TROOPSHOT].speed = 20 * FRACUNIT;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Run the actual in-game (3d) portions of the game.
// Also do intermission and finale screens following a level.
// This is used to run the game for non-demo gameplay.
//------------------------------------------------------------------------------------------------------------------------------------------
void G_RunGame() noexcept {
    while (true) {
        // Load the level and run the game
        G_DoLoadLevel();
        MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);

        // PsyDoom: if app quit was requested then exit immediately
        #if PSYDOOM_MODS
            if (Input::isQuitRequested())
                break;
        #endif

        // Assume we are not restarting the current level at first
        gbIsLevelBeingRestarted = false;
        
        // End demo recording actions
        if (gGameAction == ga_recorddemo) {
            G_EndDemoRecording();
        }
        
        if (gGameAction == ga_warped)
            continue;
        
        // Can restart the level if died or explicitly restarting
        if ((gGameAction == ga_died) || (gGameAction == ga_restart)) {
            gbIsLevelBeingRestarted = true;
            continue;
        }
        
        // Cleanup after the level is done
        gLockedTexPagesMask &= 1;
        Z_FreeTags(*gpMainMemZone, PU_ANIMATION);
        
        if (gGameAction == ga_exitdemo)
            break;
        
        // Do the intermission
        MiniLoop(IN_Start, IN_Stop, IN_Ticker, IN_Drawer);

        // PsyDoom: if app quit was requested then exit immediately
        #if PSYDOOM_MODS
            if (Input::isQuitRequested())
                break;
        #endif

        // Should we do the Ultimate DOOM style (text only, no cast sequence) finale?
        //
        // Notes:
        //  (1) For Final Doom this finale type will show when finishing the first 2 out of 3 episodes.
        //  (2) Showing finales is restricted to the case where we are not warping to a secret level, since the
        //      Ultimate Doom secret levels will not be within the map number range for the Ultimate Doom episode.
        //      Any warp to a level other than the next one is considered a secret level warp.
        //  (3) PsyDoom: I'm restricting endings to the Doom and Final Doom games specifically.
        //      If some other game type is playing then we simply won't do them.
        //
        const bool bDoFinales = ((Game::gGameType == GameType::Doom) || (Game::gGameType == GameType::FinalDoom));
        const bool bGoingToSecretLevel = (gNextMap != gGameMap + 1);
        const int32_t curEpisodeNum = Game::getMapEpisode(gGameMap);
        const int32_t nextEpisodeNum = Game::getMapEpisode(gNextMap);

        if ((gNetGame == gt_single) && (!bGoingToSecretLevel) && (curEpisodeNum != nextEpisodeNum)) {
            if (bDoFinales) {
                MiniLoop(F1_Start, F1_Stop, F1_Ticker, F1_Drawer);

                // PsyDoom: if app quit was requested then exit immediately
                #if PSYDOOM_MODS
                    if (Input::isQuitRequested())
                        break;
                #endif
            } else {
                gGameAction = ga_nothing;
                break;
            }

            if (gGameAction == ga_warped || gGameAction == ga_restart)
                continue;
    
            if (gGameAction == ga_exitdemo)
                break;

            gStartMapOrEpisode = -nextEpisodeNum;   // The '-' instructs the main menu to select this episode automatically
            break;
        }

        // If there is a next map go onto it, otherwise show the DOOM II style finale (text, followed by cast)
        if (gNextMap <= Game::getNumMaps()) {
            gGameMap = gNextMap;
            continue;
        }

        if (bDoFinales) {
            MiniLoop(F2_Start, F2_Stop, F2_Ticker, F2_Drawer);

            // PsyDoom: if app quit was requested then exit immediately
            #if PSYDOOM_MODS
                if (Input::isQuitRequested())
                    break;
            #endif
        } else {
            gGameAction = ga_nothing;
            break;
        }

        if ((gGameAction != ga_warped) && (gGameAction != ga_restart))
            break;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Plays back the current demo in the demo buffer
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t G_PlayDemoPtr() noexcept {
    // Read the demo skill and map number
    gpDemo_p = gpDemoBuffer;

    const skill_t skill = (skill_t) Endian::littleToHost(gpDemo_p[0]);
    const int32_t mapNum = (int32_t) Endian::littleToHost(gpDemo_p[1]);

    gpDemo_p += 2;

    // Read the control bindings for the demo and save the previous ones before that.
    //
    // Note: for original PSX Doom there are 8 bindings, for Final Doom there are 10.
    // Need to adjust demo reading accordingly depending on which game version we are dealing with.
    padbuttons_t prevCtrlBindings[NUM_BINDABLE_BTNS];
    D_memcpy(prevCtrlBindings, gCtrlBindings, sizeof(prevCtrlBindings));

    if (Game::isFinalDoom()) {
        D_memcpy(gCtrlBindings, gpDemo_p, sizeof(padbuttons_t) * NUM_BINDABLE_BTNS);
    } else {
        // Note: original Doom did not have the move forward/backward bindings (due to no mouse support) - hence they are zeroed here:
        D_memcpy(gCtrlBindings, gpDemo_p, sizeof(padbuttons_t) * 8);
        gCtrlBindings[8] = 0;
        gCtrlBindings[9] = 0;
    }

    #if PSYDOOM_MODS
        // PsyDoom: endian correct the controls read from the demo
        if constexpr (!Endian::isLittle()) {
            for (uint32_t i = 0; i < NUM_BINDABLE_BTNS; ++i) {
                gCtrlBindings[i] = Endian::littleToHost(gCtrlBindings[i]);
            }
        }
    #endif

    static_assert(sizeof(prevCtrlBindings) == 40);

    if (Game::isFinalDoom()) {
        gpDemo_p += NUM_BINDABLE_BTNS;
    } else {
        gpDemo_p += 8;
    }

    // For Final Doom read the mouse sensitivity and save the old value to restore later:
    const int32_t oldPsxMouseSensitivity = gPsxMouseSensitivity;

    if (Game::isFinalDoom()) {
        gPsxMouseSensitivity = Endian::littleToHost((int32_t) *gpDemo_p);
        gpDemo_p++;
    }
    
    // Initialize the demo pointer, game and load the level
    G_InitNew(skill, mapNum, gt_single);
    G_DoLoadLevel();

    // PsyDoom: determine the game settings to play back this classic demo correctly, depending on what game is being used.
    // Save the previous game settings also, so they can be restored later.
    #if PSYDOOM_MODS
        const GameSettings prevGameSettings = Game::gSettings;
        Game::getClassicDemoGameSettings(Game::gSettings);
    #endif

    // Run the demo
    gbDemoPlayback = true;
    const gameaction_t exitAction = MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);
    gbDemoPlayback = false;

    // Restore the previous control bindings, mouse sensitivity and cleanup
    D_memcpy(gCtrlBindings, prevCtrlBindings, sizeof(prevCtrlBindings));
    gPsxMouseSensitivity = oldPsxMouseSensitivity;

    gLockedTexPagesMask &= 1;
    Z_FreeTags(*gpMainMemZone, PU_LEVEL | PU_LEVSPEC | PU_ANIMATION | PU_CACHE);

    // PsyDoom: cleanup the demo pointer when we're done and restore the previous game settings
    #if PSYDOOM_MODS
        gpDemo_p = nullptr;
        Game::gSettings = prevGameSettings;
    #endif

    return exitAction;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// An empty function called when a level is ended while a demo is being recorded.
// Does nothing in the retail version of the game, but likely did stuff in debug builds - perhaps saving the demo file somewhere.
//------------------------------------------------------------------------------------------------------------------------------------------
void G_EndDemoRecording() noexcept {
    // Who knows what this did...
}
