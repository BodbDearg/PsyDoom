#include "in_main.h"

#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_password.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Renderer/r_data.h"
#include "m_main.h"
#include "PcPsx/Game.h"
#include "PcPsx/Input.h"
#include "PcPsx/PlayerPrefs.h"
#include "PcPsx/PsxPadButtons.h"
#include "PcPsx/Utils.h"
#include "pw_main.h"
#include "st_main.h"
#include "Wess/psxcd.h"

struct pstats_t {
    int32_t     killpercent;
    int32_t     itempercent;
    int32_t     secretpercent;
    int32_t     fragcount;
};

const char gMapNames_Doom[][32] = {
    // Doom 1
    "Hangar",
    "Plant",
    "Toxin Refinery",
    "Command Control",
    "Phobos Lab",
    "Central Processing",
    "Computer Station",
    "Phobos Anomaly",
    "Deimos Anomaly",
    "Containment Area",
    "Refinery",
    "Deimos Lab",
    "Command Center",
    "Halls of the Damned",
    "Spawning Vats",
    "Hell Gate",
    "Hell Keep",
    "Pandemonium",
    "House of Pain",
    "Unholy Cathedral",
    "Mt. Erebus",
    "Limbo",
    "Tower Of Babel",
    "Hell Beneath",
    "Perfect Hatred",
    "Sever The Wicked",
    "Unruly Evil",
    "Unto The Cruel",
    "Twilight Descends",
    "Threshold of Pain",
    // Doom 2
    "Entryway",
    "Underhalls",
    "The Gantlet",
    "The Focus",
    "The Waste Tunnels",
    "The Crusher",
    "Dead Simple",
    "Tricks And Traps",
    "The Pit",
    "Refueling Base",
    "O of Destruction!",
    "The Factory",
    "The Inmost Dens",
    "Suburbs",
    "Tenements",
    "The Courtyard",
    "The Citadel",
    "Nirvana",
    "The Catacombs",
    "Barrels of Fun",
    "Bloodfalls",
    "The Abandoned Mines",
    "Monster Condo",
    "Redemption Denied",
    "Fortress of Mystery",
    "The Military Base",
    "The Marshes",
    "The Mansion",
    "Club Doom"
};

const char gMapNames_FinalDoom[][32] = {
    // Master Levels
    "Attack",
    "Virgil",
    "Canyon",
    "Combine",
    "Catwalk",
    "Fistula",
    "Geryon",
    "Minos",
    "Nessus",
    "Paradox",
    "Subspace",
    "Subterra",
    "Vesperas",
    // TNT
    "System Control",
    "Human Barbeque",
    "Wormhole",
    "Crater",
    "Nukage Processing",
    "Deepest Reaches",
    "Processing Area",
    "Lunar Mining Project",
    "Quarry",
    "Ballistyx",
    "Heck",
    // Plutonia
    "Congo",
    "Aztec",
    "Ghost Town",
    "Baron's Lair",
    "The Death Domain",
    "Onslaught"
};

static_assert(C_ARRAY_SIZE(gMapNames_Doom) == 59);
static_assert(C_ARRAY_SIZE(gMapNames_FinalDoom) == 30);

// The final stats to display for each player
static pstats_t gPStats[MAXPLAYERS];

// The currently displaying stats for each player, this is ramped up over time
static int32_t  gKillValue[MAXPLAYERS];
static int32_t  gItemValue[MAXPLAYERS];
static int32_t  gSecretValue[MAXPLAYERS];
static int32_t  gFragValue[MAXPLAYERS];

// What stage of the intermission we are at.
// 0 = ramping up score, 1 = score fully shown, 2 = begin exiting intermission.
static int32_t  gIntermissionStage;

//------------------------------------------------------------------------------------------------------------------------------------------
// Initialization/setup logic for the intermission screen
//------------------------------------------------------------------------------------------------------------------------------------------
void IN_Start() noexcept {
    // Clear out purgable textures and cache the background for the intermission
    I_PurgeTexCache();
    I_LoadAndCacheTexLump(gTex_BACK, "BACK", 0);

    // Initialize final and displayed stats for all players
    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        const player_t& player = gPlayers[playerIdx];
        pstats_t& pstats = gPStats[playerIdx];

        gKillValue[playerIdx] = 0;
        gItemValue[playerIdx] = 0;
        gSecretValue[playerIdx] = 0;
        gFragValue[playerIdx] = 0;

        if (gTotalKills != 0) {
            pstats.killpercent = (player.killcount * 100) / gTotalKills;
        } else {
            pstats.killpercent = 100;
        }

        if (gTotalItems != 0) {
            pstats.itempercent = (player.itemcount * 100) / gTotalItems;
        } else {
            pstats.itempercent = 100;
        }

        if (gTotalSecret != 0) {
            pstats.secretpercent = (player.secretcount * 100) / gTotalSecret;
        } else {
            pstats.secretpercent = 100;
        }

        if (gNetGame == gt_deathmatch) {
            pstats.fragcount = player.frags;
        }
    }

    // Init menu timeout and intermission stage
    gIntermissionStage = 0;
    gMenuTimeoutStartTicCon = gTicCon;

    // Compute the password for the next map and mark it as entered (so we don't do a pistol start)
    if (gNextMap <= Game::getNumMaps()) {
        P_ComputePassword(gPasswordCharBuffer);
        gNumPasswordCharsEntered = 10;

        // PsyDoom: remember this password in player prefs, so it's restored on relaunch
        #if PSYDOOM_MODS
            PlayerPrefs::pullLastPassword();
        #endif
    }
    
    // Play the intermission cd track
    psxcd_play_at_andloop(
        gCDTrackNum[cdmusic_intermission],
        gCdMusicVol,
        0,
        0,
        gCDTrackNum[cdmusic_intermission],
        gCdMusicVol,
        0,
        0
    );

    // Wait until some cd audio has been read
    Utils::waitForCdAudioPlaybackStart();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Shutdown logic for the intermission screen
//------------------------------------------------------------------------------------------------------------------------------------------
void IN_Stop([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // PsyDoom: if quitting the app then exit immediately
    #if PSYDOOM_MODS
        if (Input::isQuitRequested())
            return;
    #endif

    IN_Drawer();
    psxcd_stop();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the intermission screen.
// Adavnces the counters and checks for user input to skip to the next intermission stage.
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t IN_Ticker() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0)
            return ga_nothing;
    #endif

    // Intermission pauses for 1 second (60 vblanks) initially to stop accidental button presses
    if (gTicCon - gMenuTimeoutStartTicCon <= 60)
        return ga_nothing;

    // Checking for inputs from all players to speed up the intermission
    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        #if PSYDOOM_MODS
            const TickInputs& inputs = gTickInputs[playerIdx];
            const TickInputs& oldInputs = gOldTickInputs[playerIdx];
            const bool bMenuOk = (inputs.bMenuOk && (!oldInputs.bMenuOk));
        #else
            const padbuttons_t ticButtons = gTicButtons[playerIdx];
            const padbuttons_t oldTicButtons = gOldTicButtons[playerIdx];
            const bool bMenuOk = ((ticButtons != oldTicButtons) && (ticButtons & PAD_ACTION_BTNS));
        #endif
        
        // Handle the player trying to goto to the next stage of the intermission when action buttons are pressed
        if (bMenuOk) {
            // Advance to the next stage of the intermission
            gIntermissionStage++;

            // If we are skipping to the stats being fully shown then fully display them now
            if (gIntermissionStage == 1) {
                for (int32_t i = 0; i < MAXPLAYERS; ++i) {
                    const pstats_t& stats = gPStats[i];

                    gKillValue[i] = stats.killpercent;
                    gItemValue[i] = stats.itempercent;
                    gSecretValue[i] = stats.secretpercent;
                    gFragValue[i] = stats.fragcount;
                }

                S_StartSound(nullptr, sfx_barexp);
            }

            // If we are at the stage where the intermission can be exited do that now
            if (gIntermissionStage >= 2) {
                S_StartSound(nullptr, sfx_barexp);
                return ga_died;
            }
        }

        // If a single player game only check the first player for skip inputs
        if (gNetGame == gt_single)
            break;
    }

    // If a full 15Hz tick has not yet elapsed then do not advance the count
    if (gGameTic <= gPrevGameTic)
        return ga_nothing;
    
    // Count up the applicable stats for the current game mode.
    // The step is increments of '2' and if we go past our target then we clamp in each case.
    bool bStillCounting = false;

    for (int32_t playerIdx = 0; playerIdx < MAXPLAYERS; ++playerIdx) {
        const pstats_t& stats = gPStats[playerIdx];

        if (gNetGame == gt_deathmatch) {
            // Deathmatch game: note that frag count can count both downwards and upwards
            if (stats.fragcount < 0) {
                if (gFragValue[playerIdx] > stats.fragcount) {
                    gFragValue[playerIdx] -= 2;
                    bStillCounting = true;

                    if (gFragValue[playerIdx] < stats.fragcount) {
                        gFragValue[playerIdx] = stats.fragcount;
                    }
                }
            } else {
                if (gFragValue[playerIdx] < stats.fragcount) {
                    gFragValue[playerIdx] += 2;
                    bStillCounting = true;

                    if (gFragValue[playerIdx] > stats.fragcount) {
                        gFragValue[playerIdx] = stats.fragcount;
                    }
                }
            }
        }
        else {
            // Single player or co-op game
            if (gKillValue[playerIdx] < stats.killpercent) {
                gKillValue[playerIdx] += 2;
                bStillCounting = true;

                if (gKillValue[playerIdx] > stats.killpercent) {
                    gKillValue[playerIdx] = stats.killpercent;
                }
            }

            if (gItemValue[playerIdx] < stats.itempercent) {
                gItemValue[playerIdx] += 2;
                bStillCounting = true;

                if (gItemValue[playerIdx] > stats.itempercent) {
                    gItemValue[playerIdx] = stats.itempercent;
                }
            }

            if (gSecretValue[playerIdx] < stats.secretpercent) {
                gSecretValue[playerIdx] += 2;
                bStillCounting = true;

                if (gSecretValue[playerIdx] > stats.secretpercent) {
                    gSecretValue[playerIdx] = stats.secretpercent;
                }
            }
        }
    }

    // If the ramp up is done and we are on the initial stage then advance to the next and do the explode sfx
    if ((!bStillCounting) && (gIntermissionStage == 0)) {
        gIntermissionStage = 1;
        S_StartSound(nullptr, sfx_barexp);
    }

    // Do periodic gun shot sounds (every 2nd tick) while the count up is happening
    if (bStillCounting && ((gGameTic & 1) == 0)) {
        S_StartSound(nullptr, sfx_pistol);
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the intermission screen
//------------------------------------------------------------------------------------------------------------------------------------------
void IN_Drawer() noexcept {
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    if (gNetGame == gt_coop) {
        IN_CoopDrawer();
    } else if (gNetGame == gt_deathmatch) {
        IN_DeathmatchDrawer();
    } else {
        IN_SingleDrawer();
    }

    I_SubmitGpuCmds();
    I_DrawPresent();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the single player intermission screen
//------------------------------------------------------------------------------------------------------------------------------------------
void IN_SingleDrawer() noexcept {
    I_CacheAndDrawSprite(gTex_BACK, 0, 0, Game::getTexPalette_BACK());

    I_DrawString(-1, 20, Game::getMapName(gGameMap));
    I_DrawString(-1, 36, "Finished");

    I_DrawString(57, 65, "Kills");
    I_DrawString(182, 65, "%");
    I_DrawNumber(170, 65, gKillValue[0]);
    
    I_DrawString(53, 91, "Items");
    I_DrawString(182, 91, "%");
    I_DrawNumber(170, 91, gItemValue[0]);

    I_DrawString(26, 117, "Secrets");
    I_DrawString(182, 117, "%");
    I_DrawNumber(170, 117, gSecretValue[0]);
    
    // Only draw the next map and password if there is a next map
    if (gNextMap <= Game::getNumMaps()) {
        I_DrawString(-1, 145, "Entering");
        I_DrawString(-1, 161, Game::getMapName(gNextMap));
        I_DrawString(-1, 187, "Password");

        char passwordStr[PW_SEQ_LEN + 1];

        for (int32_t i = 0; i < PW_SEQ_LEN; ++i) {
            passwordStr[i] = gPasswordChars[gPasswordCharBuffer[i]];
        }

        passwordStr[PW_SEQ_LEN] = 0;
        I_DrawString(-1, 203, passwordStr);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the cooperative mode intermission screen
//------------------------------------------------------------------------------------------------------------------------------------------
void IN_CoopDrawer() noexcept {
    I_CacheAndDrawSprite(gTex_BACK, 0, 0, Game::getTexPalette_BACK());

    I_DrawSprite(
        gTex_STATUS.texPageId,
        gPaletteClutIds[UIPAL],
        139,
        20,
        gFaceSprites[0].texU,
        gFaceSprites[0].texV,
        gFaceSprites[0].w,
        gFaceSprites[0].h
    );

    I_DrawString(130, 52, "you");

    I_DrawSprite(
        gTex_STATUS.texPageId,
        gPaletteClutIds[UIPAL],
        213,
        20,
        gFaceSprites[0].texU,
        gFaceSprites[0].texV,
        gFaceSprites[0].w,
        gFaceSprites[0].h
    );

    I_DrawString(208, 52, "him");
    
    I_DrawString(57, 79, "Kills");
    I_DrawString(155, 79, "%");
    I_DrawString(228, 79, "%");
    I_DrawNumber(143, 79, gKillValue[gCurPlayerIndex]);
    I_DrawNumber(216, 79, gKillValue[(gCurPlayerIndex == 0) ? 1 : 0]);

    I_DrawString(53, 101, "Items");
    I_DrawString(155, 101, "%");
    I_DrawString(228, 101, "%");
    I_DrawNumber(143, 101, gItemValue[gCurPlayerIndex]);
    I_DrawNumber(216, 101, gItemValue[(gCurPlayerIndex == 0) ? 1 : 0]);

    I_DrawString(26, 123, "Secrets");
    I_DrawString(155, 123, "%");
    I_DrawString(228, 123, "%");
    I_DrawNumber(143, 123, gSecretValue[gCurPlayerIndex]);
    I_DrawNumber(216, 123, gSecretValue[(gCurPlayerIndex == 0) ? 1 : 0]);

    // Only draw the next map and password if there is a next map
    if (gNextMap <= Game::getNumMaps()) {
        I_DrawString(-1, 149, "Entering");
        I_DrawString(-1, 165, Game::getMapName(gNextMap));

        // Well this is mean! The current player only gets to see a password if not dead :(
        if (gPlayers[gCurPlayerIndex].health > 0) {
            I_DrawString(-1, 191, "Password");

            char passwordStr[PW_SEQ_LEN + 1];

            for (int32_t i = 0; i < PW_SEQ_LEN; ++i) {
                passwordStr[i] = gPasswordChars[gPasswordCharBuffer[i]];
            }

            passwordStr[PW_SEQ_LEN] = 0;
            I_DrawString(-1, 207, passwordStr);
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Draws the deathmatch mode intermission screen
//------------------------------------------------------------------------------------------------------------------------------------------
void IN_DeathmatchDrawer() noexcept {
    I_CacheAndDrawSprite(gTex_BACK, 0, 0, Game::getTexPalette_BACK());

    I_DrawString(-1, 20, Game::getMapName(gGameMap));
    I_DrawString(-1, 36, "Finished");

    const facesprite_t* pFaceSpriteP1;
    const facesprite_t* pFaceSpriteP2;
    
    if (gFragValue[0] > gFragValue[1]) {
        if (gCurPlayerIndex == 0) {
            pFaceSpriteP1 = &gFaceSprites[EVILFACE];
            pFaceSpriteP2 = &gFaceSprites[DEADFACE];
        } else {
            pFaceSpriteP1 = &gFaceSprites[DEADFACE];
            pFaceSpriteP2 = &gFaceSprites[EVILFACE];
        }
    }
    else if (gFragValue[0] < gFragValue[1]) {
        if (gCurPlayerIndex == 0) {
            pFaceSpriteP1 = &gFaceSprites[DEADFACE];
            pFaceSpriteP2 = &gFaceSprites[EVILFACE];
        } else {
            pFaceSpriteP1 = &gFaceSprites[EVILFACE];
            pFaceSpriteP2 = &gFaceSprites[DEADFACE];
        }
    }
// PsyDoom: This check will always evaluate to 'true' so help compilers which might warn we are using unitialized vars...
#if PSYDOOM_MODS
    else {
#else
    else if (gFragValue[0] == gFragValue[1]) {
#endif
        pFaceSpriteP1 = &gFaceSprites[0];
        pFaceSpriteP2 = &gFaceSprites[0];
    }
        
    I_DrawSprite(
        gTex_STATUS.texPageId,
        gPaletteClutIds[UIPAL],
        127,
        70,
        pFaceSpriteP1->texU,
        pFaceSpriteP1->texV,
        pFaceSpriteP1->w,
        pFaceSpriteP1->h
    );

    I_DrawString(118, 102, "you");

    I_DrawSprite(
        gTex_STATUS.texPageId,
        gPaletteClutIds[UIPAL],
        200,
        70,
        pFaceSpriteP2->texU,
        pFaceSpriteP2->texV,
        pFaceSpriteP2->w,
        pFaceSpriteP2->h
    );

    I_DrawString(195, 102, "him");

    I_DrawString(35, 138, "Frags");
    I_DrawNumber(133, 138, gFragValue[gCurPlayerIndex]);
    I_DrawNumber(206, 138, gFragValue[(gCurPlayerIndex == 0) ? 1 : 0]);

    // Only draw the next map if there is one
    if (gNextMap <= Game::getNumMaps()) {
        I_DrawString(-1, 190, "Entering");
        I_DrawString(-1, 206, Game::getMapName(gNextMap));
    }
}
