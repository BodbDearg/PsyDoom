#include "g_game.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/m_random.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
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
#include "PcPsx/Utils.h"
#include "Wess/wessapi.h"

// Helper global holding the result of executing a gameloop via 'MiniLoop'.
// Sometimes this is used in preference to the return action, and sometimes it is used temporarily to hold the return action.
const VmPtr<gameaction_t> gGameAction(0x80077EB4);

// The current skill level. game type, game map, and the next/upcoming map
const VmPtr<skill_t>        gGameSkill(0x80078258);
const VmPtr<gametype_t>     gNetGame(0x8007805C);
const VmPtr<int32_t>        gGameMap(0x80078048);
const VmPtr<int32_t>        gNextMap(0x80078098);

// State for each player and whether they are in the game
const VmPtr<player_t[MAXPLAYERS]> gPlayers(0x800A87EC);
const VmPtr<bool32_t[MAXPLAYERS]> gbPlayerInGame(0x800780AC);

// Current and previous game tick count (15 Hz ticks)
const VmPtr<int32_t> gGameTic(0x8007804C);
const VmPtr<int32_t> gPrevGameTic(0x80077FA4);

// The last tick count we wanted to be at (15 Hz ticks).
// On the PSX if the game was running slow, then we might not have reached this amount.
const VmPtr<int32_t> gLastTgtGameTicCount(0x8007829C);

// Player stats
const VmPtr<int32_t> gTotalKills(0x80077F20);
const VmPtr<int32_t> gTotalItems(0x80077F2C);
const VmPtr<int32_t> gTotalSecret(0x80077FEC);

// Are we playing back or recording a demo?
const VmPtr<bool32_t> gbDemoPlayback(0x80078080);
const VmPtr<bool32_t> gbDemoRecording(0x800781AC);

// Is the level being restarted?
const VmPtr<bool32_t> gbIsLevelBeingRestarted(0x80077FF4);

// An empty map object initially assigned to players during network game setup, for net consistency checks.
// This is all zeroed out initially.
const VmPtr<mobj_t> gEmptyMObj(0x800A9E30);

//------------------------------------------------------------------------------------------------------------------------------------------
// Displays a loading message then loads the current map
//------------------------------------------------------------------------------------------------------------------------------------------
void G_DoLoadLevel() noexcept {
    // Draw the loading plaque
    I_DrawLoadingPlaque(*gTex_LOADING, 95, 109, gPaletteClutIds[UIPAL]);

    // Wait for the pistol and barrel explode menu sounds to stop playing
    while ((wess_seq_status(sfx_barexp) == SEQUENCE_PLAYING) || (wess_seq_status(sfx_pistol) == SEQUENCE_PLAYING)) {
        // PC-PSX: need to update sound to escape this loop, also ensure the window stays responsive etc.
        #if PC_PSX_DOOM_MODS
            Utils::doPlatformUpdates();
            Utils::threadYield();
        #endif
    }

    // Loading sound and music
    S_LoadMapSoundAndMusic(*gGameMap);

    // Initialize the state of each player if required
    {
        const gameaction_t gameAction = *gGameAction;

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
    P_SetupLevel(*gGameMap, *gGameSkill);
    Z_CheckHeap(*gpMainMemZone);

    // No action set upon starting a level
    *gGameAction = ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Some end of level logic for the player
//------------------------------------------------------------------------------------------------------------------------------------------
void G_PlayerFinishLevel(int32_t playerIdx) noexcept {
    // Remove cards and powerups
    D_memset(gPlayers[playerIdx].powers, std::byte(0), sizeof(player_t::powers));
    D_memset(gPlayers[playerIdx].cards, std::byte(0), sizeof(player_t::cards));

    // Clear blending flags on the player.
    // PC-PSX: preserve the noclip cheat also, if active.
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
    if (*gNetGame == gt_single) {
        *gGameAction = ga_died;
        return;
    }

    // Disassociate the player's corpse from the player
    if (gPlayers[playerIdx].mo->player) {
        gPlayers[playerIdx].mo->player = nullptr;
    }

    // Try and pick a point to spawn at
    mobj_t& playerMobj = *gPlayers[playerIdx].mo;
    mapthing_t* pChosenSpawnPt = nullptr;

    if (*gNetGame == gt_deathmatch) {
        // Deathmatch game: try to choose a random spawn point that's good
        const int32_t numSpawnPts = (int32_t)(gpDeathmatchP->get() - gDeathmatchStarts.get());

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
    *gGameAction = ga_completed;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Common game setup logic for both demos and regular gameplay
//------------------------------------------------------------------------------------------------------------------------------------------
void G_InitNew(const skill_t skill, const int32_t mapNum, const gametype_t gameType) noexcept {
    // Resetting memory management related stuff and RNGs
    *gbIsLevelBeingRestarted = false;
    gLockedTexPagesMask &= 1;
    I_PurgeTexCache();

    Z_FreeTags(*gpMainMemZone, PU_CACHE|PU_ANIMATION|PU_LEVSPEC|PU_LEVEL);
    M_ClearRandom();

    // Save game params: note that in multiplayer these might be overriden later
    *gGameMap = mapNum;
    *gGameSkill = skill;
    *gNetGame = gameType;

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
    D_memset(gEmptyMObj.get(), std::byte(0), sizeof(mobj_t));
    gPlayers[0].mo = gEmptyMObj;
    gPlayers[1].mo = gEmptyMObj;

    // Set some player status flags and controls stuff
    gbPlayerInGame[0] = true;

    if (gameType == gt_single) {
        gpPlayerCtrlBindings[0] = gCtrlBindings;
        gbPlayerInGame[1] = false;
    }
    else if ((gameType == gt_deathmatch) || (gameType == gt_coop)) {
        gbPlayerInGame[1] = true;
    }

    // Not recording or playing back a demo (yet)
    *gbDemoRecording = false;
    *gbDemoPlayback = false;

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
    
        *gbIsLevelBeingRestarted = false;
        
        if (*gGameAction == ga_recorddemo) {
            G_EndDemoRecording();
        }
        
        if (*gGameAction == ga_warped)
            continue;
        
        if ((*gGameAction == ga_died) || (*gGameAction == ga_restart)) {
            *gbIsLevelBeingRestarted = true;
            continue;
        }
        
        // Cleanup after the level is done
        gLockedTexPagesMask &= 1;
        Z_FreeTags(*gpMainMemZone, PU_ANIMATION);
        
        if (*gGameAction == ga_exitdemo)
            break;
        
        // Do the intermission
        MiniLoop(IN_Start, IN_Stop, IN_Ticker, IN_Drawer);

        // Should we do the Ultimate DOOM finale?
        if ((*gNetGame == gt_single) && (*gGameMap == 30) && (*gNextMap == 31)) {
            MiniLoop(F1_Start, F1_Stop, F1_Ticker, F1_Drawer);

            if (*gGameAction == ga_warped || *gGameAction == ga_restart)
                continue;
    
            if (*gGameAction == ga_exitdemo)
                break;

            gStartMapOrEpisode = -2;
            break;
        }

        // If there is a next map go onto it, otherwise show the DOOM II finale
        if (*gNextMap < 60) {
            *gGameMap = *gNextMap;
            continue;
        }

        MiniLoop(F2_Start, F2_Stop, F2_Ticker, F2_Drawer);

        if ((*gGameAction != ga_warped) && (*gGameAction != ga_restart))
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

    // Read the control bindings for the demo and save the previous ones before that
    padbuttons_t prevCtrlBindings[NUM_BINDABLE_BTNS];
    
    D_memcpy(prevCtrlBindings, gCtrlBindings, sizeof(prevCtrlBindings));
    D_memcpy(gCtrlBindings, gpDemo_p, sizeof(prevCtrlBindings));

    #if PC_PSX_DOOM_MODS
        // PC-PSX: endian correct the controls written to the demo also
        if constexpr (!Endian::isLittle()) {
            for (uint32_t i = 0; i < NUM_BINDABLE_BTNS; ++i) {
                gCtrlBindings[i] = Endian::littleToHost(gCtrlBindings[i]);
            }
        }
    #endif

    static_assert(sizeof(prevCtrlBindings) == 32);
    gpDemo_p += 8;
    
    // Initialize the demo pointer, game and load the level
    G_InitNew(skill, mapNum, gt_single);
    G_DoLoadLevel();

    // Run the demo
    *gbDemoPlayback = true;
    const gameaction_t exitAction = MiniLoop(P_Start, P_Stop, P_Ticker, P_Drawer);
    *gbDemoPlayback = false;

    // Restore the previous control bindings and cleanup
    D_memcpy(gCtrlBindings, prevCtrlBindings, sizeof(prevCtrlBindings));
    gLockedTexPagesMask &= 1;
    Z_FreeTags(*gpMainMemZone, PU_LEVEL | PU_LEVSPEC | PU_ANIMATION | PU_CACHE);

    // PC-PSX: cleanup the demo pointer when we're done
    #if PC_PSX_DOOM_MODS
        gpDemo_p = nullptr;
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
