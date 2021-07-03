#include "g_game.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_texcache.h"
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
#include "Endian.h"
#include "info.h"
#include "p_inter.h"
#include "p_local.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_setup.h"
#include "p_tick.h"
#include "PsyDoom/Config.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/MapInfo.h"
#include "PsyDoom/Utils.h"
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

// PsyDoom: external camera for cutscenes showing doors opening etc.
// How many tics it has left, the camera position and angle.
#if PSYDOOM_MODS
    uint32_t    gExtCameraTicsLeft;
    fixed_t     gExtCameraX;
    fixed_t     gExtCameraY;
    fixed_t     gExtCameraZ;
    angle_t     gExtCameraAngle;
#endif

// An empty map object initially assigned to players during network game setup, for net consistency checks.
// This is all zeroed out initially.
static mobj_t gEmptyMobj;

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays a loading message then loads the current map
//------------------------------------------------------------------------------------------------------------------------------------------
void G_DoLoadLevel() noexcept {
    // Draw the loading plaque
    I_DrawLoadingPlaque(gTex_LOADING, 95, 109, Game::getTexPalette_LOADING());

    // Wait for the pistol and barrel explode menu sounds to stop playing.
    // PsyDoom: this can now be optionally skipped if fast loading is enabled.
    #if PSYDOOM_MODS
        const bool bWaitForSoundsToEnd = (!Config::gbUseFastLoading);
    #else
        const bool bWaitForSoundsToEnd = true;
    #endif

    if (bWaitForSoundsToEnd) {
        Utils::waitUntilSeqExitedStatus(sfx_barexp, SequenceStatus::SEQUENCE_PLAYING);
        Utils::waitUntilSeqExitedStatus(sfx_pistol, SequenceStatus::SEQUENCE_PLAYING);
    }

    // PsyDoom: no startup warning initially and ensure all playing stuff is stopped
    #if PSYDOOM_MODS
        wess_seq_stopall();
        std::memset(gLevelStartupWarning, 0, sizeof(gLevelStartupWarning));
    #endif

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

    // PsyDoom: init the new external camera fields
    #if PSYDOOM_MODS
        gExtCameraTicsLeft = 0;
        gExtCameraX = 0;
        gExtCameraY = 0;
        gExtCameraZ = 0;
        gExtCameraAngle = 0;
    #endif

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

    #if PSYDOOM_MODS
        const uint32_t cheatFlags = player.cheats;      // PsyDoom: preserve cheats on level warping
    #endif

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

    #if PSYDOOM_MODS
        player.cheats = cheatFlags;     // PsyDoom: preserve cheats on level warping
    #endif
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
            if (P_CheckPosition(playerMobj, d_int_to_fixed(spawnPt.x), d_int_to_fixed(spawnPt.y))) {
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
        if (!P_CheckPosition(playerMobj, d_int_to_fixed(prefSpawnPt.x), d_int_to_fixed(prefSpawnPt.y))) {
            for (int32_t spawnPtIdx = 0; spawnPtIdx < MAXPLAYERS; ++spawnPtIdx) {
                mapthing_t& otherSpawnPt = gPlayerStarts[spawnPtIdx];

                // If we found a good position we can stop.
                // Make sure the map object type is correct for this player index also.
                if (P_CheckPosition(playerMobj, d_int_to_fixed(otherSpawnPt.x), d_int_to_fixed(otherSpawnPt.y))) {
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
    const fixed_t spawnX = d_int_to_fixed(pChosenSpawnPt->x);
    const fixed_t spawnY = d_int_to_fixed(pChosenSpawnPt->y);
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

    // Texture cache: unlock everything except UI assets and other reserved areas of VRAM.
    // In limit removing mode also ensure we are using tight packing of VRAM.
    #if PSYDOOM_MODS
        #if PSYDOOM_LIMIT_REMOVING
            I_LockAllWallAndFloorTextures(false);
            I_TexCacheUseLoosePacking(false);
        #else
            I_UnlockAllTexCachePages();
            I_LockTexCachePage(0);
        #endif
    #else
        gLockedTexPagesMask &= 1;
    #endif

    I_PurgeTexCache();

    Z_FreeTags(*gpMainMemZone, PU_CACHE | PU_ANIMATION | PU_LEVSPEC | PU_LEVEL);
    M_ClearRandom();

    // Save game params: note that in multiplayer these might be overriden later
    gGameMap = mapNum;
    gGameSkill = skill;
    gNetGame = gameType;

    // PsyDoom: determine the game settings for single player games that are not demos.
    // In a multiplayer game these settings will have already been determined before this point, hence no determination here.
    // For demo playback this will also be the case since correct settings must be forced for demo compatibility.
    #if PSYDOOM_MODS
        if ((gNetGame == gt_single) && (!gpDemo_p)) {
            Game::getUserGameSettings(Game::gSettings);
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

    // PsyDoom: clear all player cheat flags here on starting a new game because we now preserve cheat flags between levels in 'G_PlayerReborn'
    #if PSYDOOM_MODS
        for (player_t& player : gPlayers) {
            player.cheats = 0;
        }
    #endif

    // Clear the empty map object and assign to both players initially.
    // This is used for network consistency checks:
    D_memset(&gEmptyMobj, std::byte(0), sizeof(mobj_t));
    gPlayers[0].mo = &gEmptyMobj;
    gPlayers[1].mo = &gEmptyMobj;

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
        gMobjInfo[MT_SERGEANT].speed = 15;
        gMobjInfo[MT_BRUISERSHOT].speed = 40 * FRACUNIT;
        gMobjInfo[MT_HEADSHOT].speed = 40 * FRACUNIT;
        gMobjInfo[MT_TROOPSHOT].speed = 40 * FRACUNIT;
    } else {
        gStates[S_SARG_ATK1].tics = 4;
        gStates[S_SARG_ATK2].tics = 4;
        gStates[S_SARG_ATK3].tics = 4;
        gMobjInfo[MT_SERGEANT].speed = 10;
        gMobjInfo[MT_BRUISERSHOT].speed = 30 * FRACUNIT;
        gMobjInfo[MT_HEADSHOT].speed = 20 * FRACUNIT;
        gMobjInfo[MT_TROOPSHOT].speed = 20 * FRACUNIT;
    }

    // PsyDoom: apply the Super Shotgun delay tweak if enabled, or otherwise undo it
    #if PSYDOOM_MODS
        if (Game::gSettings.bUseSuperShotgunDelayTweak) {
            gStates[S_DSGUN1].tics = 1;
            gStates[S_DSGUN5].tics = 4;
        } else {
            gStates[S_DSGUN1].tics = 2;
            gStates[S_DSGUN5].tics = 3;
        }
    #endif
}

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// Run the actual in-game (3d) portions of the game.
// Also do intermission and finale screens following a level.
// This is used to run the game for non-demo gameplay.
// PsyDoom: this function has been rewritten. For the original version see the 'Old' folder.
//------------------------------------------------------------------------------------------------------------------------------------------
void G_RunGame() noexcept {
    while (true) {
        // Load the level and run the game
        G_DoLoadLevel();
        MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);

        // PsyDoom: if app quit was requested then exit immediately
        if (Input::isQuitRequested())
            break;

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

        // Cleanup after the level is done.
        // Texture cache: unlock everything except UI assets and other reserved areas of VRAM.
        // In limit removing mode also ensure we are using tight packing of VRAM.
        #if PSYDOOM_LIMIT_REMOVING
            I_LockAllWallAndFloorTextures(false);
            I_TexCacheUseLoosePacking(false);
        #else
            I_UnlockAllTexCachePages();
            I_LockTexCachePage(0);
        #endif

        Z_FreeTags(*gpMainMemZone, PU_ANIMATION);

        if (gGameAction == ga_exitdemo)
            break;

        // Do the intermission
        MiniLoop(IN_Start, IN_Stop, IN_Ticker, IN_Drawer);

        // PsyDoom: if app quit was requested then exit immediately
        if (Input::isQuitRequested())
            break;

        // Should we do the Ultimate DOOM style (text only, no cast sequence) finale?
        const MapInfo::Map* const pMap = MapInfo::getMap(gGameMap);
        const MapInfo::Map* const pNextMap = MapInfo::getMap(gNextMap);

        const int32_t curClusterNum = (pMap) ? pMap->cluster : 0;
        const int32_t nextClusterNum = (pNextMap) ? pNextMap->cluster : curClusterNum;
        const int32_t nextEpisodeNum = Game::getMapEpisode(gNextMap);

        if ((gNetGame == gt_single) && (curClusterNum != nextClusterNum)) {
            MiniLoop(F1_Start, F1_Stop, F1_Ticker, F1_Drawer);

            // PsyDoom: if app quit was requested then exit immediately
            if (Input::isQuitRequested())
                break;

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

        MiniLoop(F2_Start, F2_Stop, F2_Ticker, F2_Drawer);

        // PsyDoom: if app quit was requested then exit immediately
        if (Input::isQuitRequested())
            break;

        if ((gGameAction != ga_warped) && (gGameAction != ga_restart))
            break;
    }
}
#endif  // #if PSYDOOM_MODS

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

    // PsyDoom: determine the game settings to play back this classic demo correctly, depending on what game is being used.
    // Save the previous game settings also, so they can be restored later.
    // N.B: this *MUST* be done before loading the level, as some of the settings (e.g nomonsters) affect level loading.
    #if PSYDOOM_MODS
        const GameSettings prevGameSettings = Game::gSettings;
        Game::getClassicDemoGameSettings(Game::gSettings);
    #endif

    // Do basic game initialization
    G_InitNew(skill, mapNum, gt_single);

    // Load the map used by the demo
    G_DoLoadLevel();

    // Run the demo
    gbDemoPlayback = true;
    const gameaction_t exitAction = MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);
    gbDemoPlayback = false;

    // Restore the previous control bindings, mouse sensitivity and cleanup
    D_memcpy(gCtrlBindings, prevCtrlBindings, sizeof(prevCtrlBindings));
    gPsxMouseSensitivity = oldPsxMouseSensitivity;

    // Texture cache: unlock everything except UI assets and other reserved areas of VRAM.
    // In limit removing mode also ensure we are using tight packing of VRAM.
    #if PSYDOOM_MODS
        #if PSYDOOM_LIMIT_REMOVING
            I_LockAllWallAndFloorTextures(false);
            I_TexCacheUseLoosePacking(false);
        #else
            I_UnlockAllTexCachePages();
            I_LockTexCachePage(0);
        #endif
    #else
        gLockedTexPagesMask &= 1;
    #endif

    Z_FreeTags(*gpMainMemZone, PU_LEVEL | PU_LEVSPEC | PU_ANIMATION | PU_CACHE);

    // PsyDoom: cleanup the demo pointer when we're done and restore the previous game settings.
    // Also purge the texture cache to cleanup any textures.
    #if PSYDOOM_MODS
        gpDemo_p = nullptr;
        Game::gSettings = prevGameSettings;
        I_PurgeTexCache();
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
