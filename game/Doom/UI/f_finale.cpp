//------------------------------------------------------------------------------------------------------------------------------------------
// Logic for the various finale screens, including the end of game cast sequence.
// Note that I have adopted and mixed some of the code from Final Doom here because it is more flexible to support multiple games.
//------------------------------------------------------------------------------------------------------------------------------------------
#include "f_finale.h"

#include "Doom/Base/i_drawcmds.h"
#include "Doom/Base/i_main.h"
#include "Doom/Base/i_misc.h"
#include "Doom/Base/i_texcache.h"
#include "Doom/Base/s_sound.h"
#include "Doom/Base/sounds.h"
#include "Doom/d_main.h"
#include "Doom/Game/g_game.h"
#include "Doom/Game/p_setup.h"
#include "Doom/Game/p_tick.h"
#include "Doom/Game/sprinfo.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/RendererVk/rv_utils.h"
#include "m_main.h"
#include "PsyDoom/Config/Config.h"
#include "PsyDoom/Game.h"
#include "PsyDoom/Input.h"
#include "PsyDoom/MapInfo/MapInfo.h"
#include "PsyDoom/PsxPadButtons.h"
#include "PsyDoom/Utils.h"
#include "PsyDoom/Video.h"
#include "PsyDoom/Vulkan/VDrawing.h"
#include "PsyDoom/Vulkan/VRenderer.h"
#include "PsyDoom/Vulkan/VTypes.h"
#include "PsyQ/LIBGPU.h"
#include "Wess/wessapi.h"

// PsyDoom: this is all now defined in the 'MapInfo' module
#if !PSYDOOM_MODS
    // Win text for Doom 1 and 2, and Final Doom.
    // Note that this is all in the Final Doom format, which is an unbounded C-String.
    static const char* const gDoom1WinText[] = {
        "you have won!",
        "your victory enabled",
        "humankind to evacuate",
        "earth and escape the",
        "nightmare.",
        "but then earth control",
        "pinpoints the source",
        "of the alien invasion.",
        "you are their only hope.",
        "you painfully get up",
        "and return to the fray.",
        nullptr
    };

    static const char* const gDoom2WinText[] = {
        "you did it!",
        "by turning the evil of",
        "the horrors of hell in",
        "upon itself you have",
        "destroyed the power of",
        "the demons.",
        "their dreadful invasion",
        "has been stopped cold!",
        "now you can retire to",
        "a lifetime of frivolity.",
        "congratulations!",
        nullptr
    };

    static const char* const gFinalDoomWinText_MasterLevels[] = {
        "you have assaulted and",
        "triumphed over the most",
        "vicious realms that the",
        "demented minds of our",
        "designers could devise.",
        "the havoc you left",
        "behind you as you",
        "smashed your way",
        "through the master",
        "levels is mute tribute",
        "to your prowess.",
        "you have earned the",
        "title of",
        "Master of Destruction.",
        nullptr
    };

    static const char* const gFinalDoomWinText_Tnt[] = {
        "suddenly all is silent",
        "from one horizon to the",
        "other.",
        "the agonizing echo of",
        "hell fades away.",
        "the nightmare sky",
        "turns blue.",
        "the heaps of monster",
        "corpses begin to dissolve",
        "along with the evil stench",
        "that filled the air.",
        "maybe you_have done it.",
        "Have you really won...",
        nullptr
    };

    static const char* const gFinalDoomWinText_Plutonia[] = {
        "you_gloat_over_the",
        "carcass_of_the_guardian.",
        "with_its_death_you_have",
        "wrested_the_accelerator",
        "from_the_stinking_claws",
        "of_hell._you_are_done.",
        "hell_has_returned_to",
        "pounding_dead_folks",
        "instead_of_good_live_ones.",
        "remember_to_tell_your",
        "grandkids_to_put_a_rocket",
        "launcher_in_your_coffin.",
        "If_you_go_to_hell_when",
        "you_die_you_will_need_it",
        "for_some_cleaning_up.",
        nullptr
    };

    // The top y position of the win text for Doom 1 and 2, and Final Doom
    static constexpr int32_t DOOM1_TEXT_YPOS = 45;
    static constexpr int32_t DOOM2_TEXT_YPOS = 45;
    static constexpr int32_t MASTERLEV_TEXT_YPOS = 22;
    static constexpr int32_t TNT_TEXT_YPOS = 29;
    static constexpr int32_t PLUTONIA_TEXT_YPOS = 15;
#endif

// The cast of characters to display
struct castinfo_t {
    const char*     name;
    mobjtype_t      type;
};

static const castinfo_t gCastOrder[] = {
    { "Zombieman",              MT_POSSESSED    },
    { "Shotgun Guy",            MT_SHOTGUY      },
    { "Heavy Weapon Dude",      MT_CHAINGUY     },
    { "Imp",                    MT_TROOP        },
    { "Demon",                  MT_SERGEANT     },
    { "Lost Soul",              MT_SKULL        },
    { "Cacodemon",              MT_HEAD         },
    { "Hell Knight",            MT_KNIGHT       },
    { "Baron Of Hell",          MT_BRUISER      },
    { "Arachnotron",            MT_BABY         },
    { "Pain Elemental",         MT_PAIN         },
    { "Revenant",               MT_UNDEAD       },
    { "Mancubus",               MT_FATSO        },
    { "The Spider Mastermind",  MT_SPIDER       },
    { "The Cyberdemon",         MT_CYBORG       },
    { "Our Hero",               MT_PLAYER       },
    { nullptr,                  MT_PLAYER       }       // Null marker
};

// Used for the cast finale - what stage we are at
enum finalestage_t : int32_t {
    F_STAGE_TEXT,
    F_STAGE_SCROLLTEXT,
    F_STAGE_CAST,
};

#if PSYDOOM_MODS
    static const MapInfo::Cluster*  gpCluster;      // PsyDoom: which cluster the finale is being shown for
    static texture_t                gFinaleBgTex;   // PsyDoom: finale background texture (it can be anything)
#endif

static finalestage_t        gFinaleStage;           // What stage of the finale we are on
static int32_t              gFinTextYPos;           // Current y position of the top finale line
static char                 gFinIncomingLine[64];   // Text for the incoming line
static int32_t              gFinIncomingLineLen;    // How many characters are being displayed for the incomming text line

// PsyDoom: not needed anymore after changes to make the finale use data defined in MAPINFO
#if !PSYDOOM_MODS
    static const char* const*   gFinTextLines;      // Which text lines we are using for this finale text?
#endif

static int32_t              gFinLinesDone;          // How many full lines we are displaying
static int32_t              gCastNum;               // Which of the cast characters (specified by index) we are showing
static int32_t              gCastTics;              // Tracks current time in the current cast state
static int32_t              gCastFrames;            // Tracks how many frames a cast member has done - used to decide when to attack
static int32_t              gCastOnMelee;           // If non zero then the cast member should do a melee attack
static bool                 gbCastDeath;            // Are we killing the current cast member?
static const state_t*       gpCastState;            // Current state being displayed for the cast character

// PsyDoom: the background for the finale can now be anything as it is sourced from MAPINFO
#if !PSYDOOM_MODS
    static texture_t gTex_DEMON;    // The demon (icon of sin) background for the DOOM II finale
#endif

#if PSYDOOM_MODS
//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom helper: fetches the cluster to use for the finale and issues a fatal error if not found
//------------------------------------------------------------------------------------------------------------------------------------------
static void F_LookupCluster() noexcept {
    const MapInfo::Map* const pMap = MapInfo::getMap(gGameMap);

    if (!pMap) {
        I_Error("F_LookupCluster: no map info available for map '%d'!", gGameMap);
    }

    gpCluster = MapInfo::getCluster(pMap->cluster);

    if (!gpCluster) {
        I_Error("F_LookupCluster: cluster '%d' does not exist in MAPINFO!", pMap->cluster);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom helper: gets the specified finale line as a null terminated c-string.
// If the line number is invalid then an empty line is returned.
//------------------------------------------------------------------------------------------------------------------------------------------
static std::array<char, 33> F_GetLine(const int32_t lineNum) noexcept {
    constexpr int32_t MAX_LINES = C_ARRAY_SIZE(MapInfo::Cluster::text);
    return ((lineNum >= 0) && (lineNum < MAX_LINES)) ? gpCluster->text[lineNum].c_str() : std::array<char, 33>{};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// PsyDoom helper: determine the x position to render a C-string at given the cluster finale settings
//------------------------------------------------------------------------------------------------------------------------------------------
static int32_t F_GetLineXPos(const char* const str) noexcept {
    // If not centering the text then the 'textX' field is an explicit position
    if (gpCluster->bNoCenterText)
        return gpCluster->textX;

    // If not using the small font 'I_DrawString' will automatically center for us when specifying '-1'
    if (!gpCluster->bSmallFont)
        return -1;

    // When using the small 8x8 font we have to manually center the string
    const int32_t textLen = (int32_t) std::strlen(str);
    return (SCREEN_W - textLen * 8) / 2;
}
#endif

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the Ultimate DOOM style finale (text only, no cast sequence) screen
//------------------------------------------------------------------------------------------------------------------------------------------
void F1_Start() noexcept {
    // PsyDoom: lookup which cluster to show the finale for
    #if PSYDOOM_MODS
        F_LookupCluster();
    #endif

    // Draw the loading plaque, purge the texture cache and load up the background needed
    I_DrawLoadingPlaque(gTex_LOADING, 95, 109, Game::getTexPalette_LOADING());
    I_PurgeTexCache();

    // PsyDoom: the background texture for the finale can now be anything
    #if PSYDOOM_MODS
        I_LoadAndCacheTexLump(gFinaleBgTex, gpCluster->pic);
    #else
        I_CacheTex(gTex_BACK);
    #endif

    // Init finale
    gFinLinesDone = 0;
    gFinIncomingLineLen = 0;
    gFinIncomingLine[0] = 0;

    // PsyDoom: finale data is now defined in the 'MapInfo' module
    #if PSYDOOM_MODS
        gFinTextYPos = gpCluster->textY;
    #else
        if (Game::isFinalDoom()) {
            if (Game::getMapEpisode(gGameMap) == 1) {
                gFinTextLines = gFinalDoomWinText_MasterLevels;
                gFinTextYPos = MASTERLEV_TEXT_YPOS;
            } else {
                gFinTextLines = gFinalDoomWinText_Tnt;
                gFinTextYPos = TNT_TEXT_YPOS;
            }
        } else {
            gFinTextLines = gDoom1WinText;
            gFinTextYPos = DOOM1_TEXT_YPOS;
        }
    #endif

    // Play the finale cd track.
    // PsyDoom: finale data is now defined in the 'MapInfo' module.
    #if PSYDOOM_MODS
        const int32_t startCdTrack = gpCluster->cdMusicA;
        const int32_t loopCdTrack = gpCluster->cdMusicB;
    #else
        const int32_t startCdTrack = gCDTrackNum[cdmusic_finale_doom1_final_doom];
        const int32_t loopCdTrack = gCDTrackNum[cdmusic_credits_demo];
    #endif

    psxcd_play_at_andloop(startCdTrack, gCdMusicVol, 0, 0, loopCdTrack, gCdMusicVol, 0, 0);

    // Wait until some cd audio has been read
    Utils::waitForCdAudioPlaybackStart();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called to shut down the Ultimate DOOM style finale (text only, no cast sequence) screen
//------------------------------------------------------------------------------------------------------------------------------------------
void F1_Stop([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // PsyDoom: if quitting the app then exit immediately
    #if PSYDOOM_MODS
        if (Input::isQuitRequested())
            return;
    #endif

    gbGamePaused = false;
    psxcd_stop();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the Ultimate DOOM style finale (text only, no cast sequence) screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t F1_Ticker() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0) {
            gbKeepInputEvents = true;   // Don't consume 'key pressed' etc. events yet, not ticking...
            return ga_nothing;
        }
    #endif

    // Grab inputs and set global game action
    gGameAction = ga_nothing;

    #if PSYDOOM_MODS
        const TickInputs& inputs = gTickInputs[gCurPlayerIndex];
        const TickInputs& oldInputs = gOldTickInputs[gCurPlayerIndex];
        const bool bMenuOk = (inputs.fMenuOk() && (!oldInputs.fMenuOk()));
    #else
        const padbuttons_t ticButtons = gTicButtons[gCurPlayerIndex];
        const padbuttons_t oldTicButtons = gOldTicButtons[gCurPlayerIndex];
        const bool bMenuOk = ((ticButtons != oldTicButtons) && (ticButtons & PAD_ACTION_BTNS));
    #endif

    // Check for pause: this allows the user a path to exit back to the main menu if desired
    P_CheckCheats();

    if (gbGamePaused)
        return gGameAction;

    // Check to see if the text needs to advance more, or if we can exit.
    // PsyDoom: added updates here to source finale data from MAPINFO instead.
    #if PSYDOOM_MODS
        const std::array<char, 33> textLineChars = F_GetLine(gFinLinesDone);
        const char* const textLine = (textLineChars.data()[0]) ? textLineChars.data() : nullptr;
    #else
        const char* const textLine = gFinTextLines[gFinLinesDone];
    #endif

    if (textLine) {
        // Text is not yet done popping up: only advance if time has elapsed and on every 2nd tick
        if ((gGameTic > gPrevGameTic) && ((gGameTic & 1) == 0)) {
            // Get the current incoming text line text and see if we need to move onto another
            if (textLine[gFinIncomingLineLen] == 0) {
                // We've reached the end of this line, move onto a new one
                gFinIncomingLineLen = 0;
                gFinLinesDone++;
            } else {
                D_strncpy(gFinIncomingLine, textLine, gFinIncomingLineLen);
            }

            // Null terminate the incomming text line and include this character in the length
            gFinIncomingLine[gFinIncomingLineLen] = 0;
            gFinIncomingLineLen++;
        }
    }
    else if (bMenuOk) {
        // If all the lines are done and an action button is just pressed then exit the screen
        return ga_exit;
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the rendering for the Ultimate Doom style finale screen (text popping up gradually)
//------------------------------------------------------------------------------------------------------------------------------------------
void F1_Drawer() noexcept {
    // Increment the frame count (for the texture cache) and draw the background
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    // PsyDoom: the background for the finale can now be anything as it is sourced from MAPINFO.
    #if PSYDOOM_MODS
        I_CacheAndDrawBackgroundSprite(gFinaleBgTex, R_GetPaletteClutId(gpCluster->picPal));
    #else
        I_CacheAndDrawBackgroundSprite(gTex_BACK, Game::getTexPalette_BACK());
    #endif

    // Show both the incoming and fully displayed text lines.
    // PsyDoom: update this to source the line data from MAPINFO and to allow for small and non-centered text.
    int32_t ypos = gFinTextYPos;

    #if PSYDOOM_MODS
        // PsyDoom: need to set the texture window before drawing (if we are using the small font)
        {
            DR_MODE drawModePrim = {};
            const SRECT texWindow = { (int16_t) gTex_STATUS.texPageCoordX, (int16_t) gTex_STATUS.texPageCoordY, 256, 256 };
            LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS.texPageId, &texWindow);
            I_AddPrim(drawModePrim);
        }

        // Draw each fully visible line of text
        const int32_t ystep = (gpCluster->bSmallFont) ? 8 : 14;

        for (int32_t lineIdx = 0; lineIdx < gFinLinesDone; ++lineIdx) {
            const std::array<char, 33> textLineChars = F_GetLine(lineIdx);
            const int32_t xpos = F_GetLineXPos(textLineChars.data());

            if (gpCluster->bSmallFont) {
                I_DrawStringSmall(xpos, ypos, textLineChars.data(), Game::getTexPalette_STATUS(), 128, 128, 128, false, true);
            } else {
                I_DrawString(xpos, ypos, textLineChars.data());
            }

            ypos += ystep;
        }

        // Draw the incoming line of text which is partially visible
        const int32_t lastLineXPos = F_GetLineXPos(gFinIncomingLine);

        if (gpCluster->bSmallFont) {
            I_DrawStringSmall(lastLineXPos, ypos, gFinIncomingLine, Game::getTexPalette_STATUS(), 128, 128, 128, false, true);
        } else {
            I_DrawString(lastLineXPos, ypos, gFinIncomingLine);
        }
    #else
        // Draw each fully presented line of text
        for (int32_t lineIdx = 0; lineIdx < gFinLinesDone; ++lineIdx) {
            I_DrawString(-1, ypos, gFinTextLines[lineIdx]);
            ypos += 14;
        }

        // Draw the incoming line of text which is partially visible
        I_DrawString(-1, ypos, gFinIncomingLine);
    #endif

    // Finale can be paused too, a step towards exiting back to the main menu...
    if (gbGamePaused) {
        I_DrawPausedOverlay();
    }

    // PsyDoom: draw any enabled performance counters
    #if PSYDOOM_MODS
        I_DrawEnabledPerfCounters();
    #endif

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the DOOM II style finale (text, followed by cast) screen
//------------------------------------------------------------------------------------------------------------------------------------------
void F2_Start() noexcept {
    // PsyDoom: lookup which cluster to show the finale for
    #if PSYDOOM_MODS
        F_LookupCluster();
    #endif

    // Show the loading plaque and purge the texture cache
    I_DrawLoadingPlaque(gTex_LOADING, 95, 109, Game::getTexPalette_LOADING());
    I_PurgeTexCache();

    // Load the background and sprites needed.
    // PsyDoom: the background texture for the finale can now be anything.
    #if PSYDOOM_MODS
        I_LoadAndCacheTexLump(gFinaleBgTex, gpCluster->pic);
    #else
        I_LoadAndCacheTexLump(gTex_DEMON, "DEMON", 0);
    #endif

    #if !PSYDOOM_MODS
        // PsyDoom: don't bother loading the sprites yet, let the code grab them from the main IWAD as required.
        // This means we don't need to worry about loading the correct blocks file for various different game types.
        P_LoadBlocks(CdFileId::MAPSPR60_IMG);
    #endif

    // Initialize the finale text
    const mobjinfo_t& mobjInfo = gMobjInfo[gCastOrder[0].type];
    const state_t& state = gStates[mobjInfo.seestate];

    gFinaleStage = F_STAGE_TEXT;
    gFinLinesDone = 0;
    gFinIncomingLineLen = 0;
    gFinIncomingLine[0] = 0;

    // PsyDoom: finale data is now defined in the 'MapInfo' module
    #if PSYDOOM_MODS
        gFinTextYPos = gpCluster->textY;
    #else
        if (Game::isFinalDoom()) {
            gFinTextLines = gFinalDoomWinText_Plutonia;
            gFinTextYPos = PLUTONIA_TEXT_YPOS;
        } else {
            gFinTextLines = gDoom2WinText;
            gFinTextYPos = DOOM2_TEXT_YPOS;
        }
    #endif

    // Initialize the cast display
    gCastNum = 0;
    gpCastState = &state;
    gCastTics = state.tics;
    gbCastDeath = false;
    gCastFrames = 0;
    gCastOnMelee = 0;

    // PsyDoom: wait for the pistol and barrel explode menu sounds to stop playing, then kill all sounds.
    // Fixes an odd bug where a Revenant sound can sometimes be heard when skipping the intermission quickly to get to this screen.
    // The bug is caused by loading in new sounds while waveform data is still in use.
    // Note: the wait for sounds to stop is skipped if 'fast loading' is enabled.
    #if PSYDOOM_MODS
        if (!Config::gbUseFastLoading) {
            Utils::waitUntilSeqExitedStatus(sfx_barexp, SequenceStatus::SEQUENCE_PLAYING);
            Utils::waitUntilSeqExitedStatus(sfx_pistol, SequenceStatus::SEQUENCE_PLAYING);
        }

        S_StopAll();
    #endif

    // Load sound for the finale
    S_LoadMapSoundAndMusic(Game::getNumMaps() + 1);

    // Play the finale cd track.
    // PsyDoom: finale data is now defined in the 'MapInfo' module.
    #if PSYDOOM_MODS
        const int32_t startCdTrack = gpCluster->cdMusicA;
        const int32_t loopCdTrack = gpCluster->cdMusicB;
    #else
        const int32_t startCdTrack = gCDTrackNum[(Game::isFinalDoom()) ? cdmusic_finale_doom1_final_doom : cdmusic_finale_doom2];
        const int32_t loopCdTrack = gCDTrackNum[cdmusic_credits_demo];
    #endif

    psxcd_play_at_andloop(startCdTrack, gCdMusicVol, 0, 0, loopCdTrack, gCdMusicVol, 0, 0);

    // Wait until some cd audio has been read
    Utils::waitForCdAudioPlaybackStart();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Called to shut down the DOOM II style finale (text, followed by cast) screen
//------------------------------------------------------------------------------------------------------------------------------------------
void F2_Stop([[maybe_unused]] const gameaction_t exitAction) noexcept {
    // PsyDoom: if quitting the app then exit immediately
    #if PSYDOOM_MODS
        if (Input::isQuitRequested())
            return;
    #endif

    gbGamePaused = false;
    psxcd_stop();

    // PsyDoom: wait for barrel and pistol sounds to stop in addition to cd music and draw a loading plaque while we wait.
    // Also kill all sounds before exiting back to the main menu - fixes a bug where a strange sound plays on returning to the main menu.
    #if PSYDOOM_MODS
        I_DrawLoadingPlaque(gTex_LOADING, 95, 109, Game::getTexPalette_LOADING());
        Utils::waitUntilSeqExitedStatus(sfx_barexp, SequenceStatus::SEQUENCE_PLAYING);
        Utils::waitUntilSeqExitedStatus(sfx_pistol, SequenceStatus::SEQUENCE_PLAYING);
        S_StopAll();
    #endif
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Update logic for the DOOM II style finale (text, followed by cast) screen
//------------------------------------------------------------------------------------------------------------------------------------------
gameaction_t F2_Ticker() noexcept {
    // PsyDoom: tick only if vblanks are registered as elapsed; this restricts the code to ticking at 30 Hz for NTSC
    #if PSYDOOM_MODS
        if (gPlayersElapsedVBlanks[0] <= 0) {
            gbKeepInputEvents = true;   // Don't consume 'key pressed' etc. events yet, not ticking...
            return ga_nothing;
        }
    #endif

    // Grab inputs and set global game action
    gGameAction = ga_nothing;

    #if PSYDOOM_MODS
        const TickInputs& inputs = gTickInputs[gCurPlayerIndex];
        const TickInputs& oldInputs = gOldTickInputs[gCurPlayerIndex];
        const bool bMenuOk = (inputs.fMenuOk() && (!oldInputs.fMenuOk()));
    #else
        const padbuttons_t ticButtons = gTicButtons[gCurPlayerIndex];
        const padbuttons_t oldTicButtons = gOldTicButtons[gCurPlayerIndex];
        const bool bMenuOk = ((ticButtons != oldTicButtons) && (ticButtons & PAD_ACTION_BTNS));
    #endif

    // Check for pause: this allows the user a path to exit back to the main menu if desired
    P_CheckCheats();

    if (gbGamePaused)
        return gGameAction;

    // Handle whatever finale stage we are on
    if (gFinaleStage == F_STAGE_TEXT) {
        // Currently popping up text: only advance if time has elapsed and on every 2nd tick
        if ((gGameTic > gPrevGameTic) && ((gGameTic & 1) == 0)) {
            // Get the current incoming text line text and see if we need to move onto another.
            // PsyDoom: added updates here to source finale data from MAPINFO instead.
            #if PSYDOOM_MODS
                const std::array<char, 33> textLineChars = F_GetLine(gFinLinesDone);
                const char* const textLine = (textLineChars.data()[0]) ? textLineChars.data() : nullptr;
            #else
                const char* const textLine = gFinTextLines[gFinLinesDone];
            #endif

            if (textLine) {
                if (textLine[gFinIncomingLineLen] == 0) {
                    // We've reached the end of this line, move onto a new one
                    gFinLinesDone++;
                    gFinIncomingLineLen = 0;
                } else {
                    D_strncpy(gFinIncomingLine, textLine, gFinIncomingLineLen);
                }
            } else {
                // If we're done all the lines then begin scrolling the text up
                gFinaleStage = F_STAGE_SCROLLTEXT;
            }

            // Null terminate the incomming text line and include this character in the length
            gFinIncomingLine[gFinIncomingLineLen] = 0;
            gFinIncomingLineLen++;
        }
    }
    else if (gFinaleStage == F_STAGE_SCROLLTEXT) {
        // Scrolling the finale text upwards for a bit before the cast call
        gFinTextYPos -= 1;

        if (gFinTextYPos < -200) {
            gFinaleStage = F_STAGE_CAST;
        }
    }
    else if (gFinaleStage == F_STAGE_CAST)  {
        // Doing the cast call: see first if the player is shooting the current character
        if ((!gbCastDeath) && bMenuOk) {
            // Shooting this character! Play the shotgun sound:
            S_StartSound(nullptr, sfx_shotgn);

            // Play enemy death sound if it has it
            const mobjinfo_t& mobjinfo = gMobjInfo[gCastOrder[gCastNum].type];

            if (mobjinfo.deathsound != 0) {
                S_StartSound(nullptr, mobjinfo.deathsound);
            }

            // Begin playing the death animation
            const state_t& deathState = gStates[mobjinfo.deathstate];

            gbCastDeath = true;
            gCastFrames = 0;
            gpCastState = &deathState;
            gCastTics = deathState.tics;
        }

        // Only advance character animation if time has passed
        if (gGameTic > gPrevGameTic) {
            if (gbCastDeath && (gpCastState->nextstate == S_NULL)) {
                // Character is dead and there is no state which follows (death anim is finished): switch to the next character
                gCastNum++;
                gbCastDeath = false;

                // Loop back around to the first character when we reach the end
                if (gCastOrder[gCastNum].name  == nullptr) {
                    gCastNum = 0;
                }

                // Initialize frame count, set state to the 'see' state and make a noise if there is a 'see' sound
                const castinfo_t& castinfo = gCastOrder[gCastNum];
                const mobjinfo_t& mobjinfo = gMobjInfo[castinfo.type];

                gCastFrames = 0;
                gpCastState = &gStates[mobjinfo.seestate];

                if (mobjinfo.seesound != 0) {
                    S_StartSound(nullptr, mobjinfo.seesound);
                }
            } else {
                // Character is not dead, advance the time until the next state
                gCastTics -= 1;

                // If we still have time until the next state then there is nothing more to do
                if (gCastTics > 0)
                    return ga_nothing;

                // Hacked in logic to play sounds on certain frames of animation
                sfxenum_t soundId;

                switch (gpCastState->nextstate) {
                    case S_PLAY_ATK2:   soundId = sfx_dshtgn;   break;
                    case S_POSS_ATK2:   soundId = sfx_pistol;   break;
                    case S_SPOS_ATK2:   soundId = sfx_shotgn;   break;
                    case S_SKEL_FIST2:  soundId = sfx_skeswg;   break;
                    case S_SKEL_FIST4:  soundId = sfx_skepch;   break;
                    case S_SKEL_MISS2:  soundId = sfx_skeatk;   break;
                    case S_FATT_ATK2:
                    case S_FATT_ATK5:
                    case S_FATT_ATK8:   soundId = sfx_firsht;   break;
                    case S_CPOS_ATK2:
                    case S_CPOS_ATK3:
                    case S_CPOS_ATK4:   soundId = sfx_pistol;   break;
                    case S_TROO_ATK3:   soundId = sfx_claw;     break;
                    case S_SARG_ATK2:   soundId = sfx_sgtatk;   break;
                    case S_BOSS_ATK2:
                    case S_BOS2_ATK2:
                    case S_HEAD_ATK2:   soundId = sfx_firsht;   break;
                    case S_SKULL_ATK2:  soundId = sfx_sklatk;   break;
                    case S_SPID_ATK2:
                    case S_SPID_ATK3:   soundId = sfx_pistol;   break;
                    case S_BSPI_ATK2:   soundId = sfx_plasma;   break;
                    case S_CYBER_ATK2:
                    case S_CYBER_ATK4:
                    case S_CYBER_ATK6:  soundId = sfx_rlaunc;   break;
                    case S_PAIN_ATK3:   soundId = sfx_sklatk;   break;

                    default: soundId = sfx_None;
                }

                if (soundId != sfx_None) {
                    S_StartSound(nullptr, soundId);
                }
            }

            // Advance onto the next state
            gpCastState = &gStates[gpCastState->nextstate];
            gCastFrames++;

            const castinfo_t& castinfo = gCastOrder[gCastNum];
            const mobjinfo_t& mobjinfo = gMobjInfo[castinfo.type];

            // Every 12 frames make the enemy attack.
            // Alternate between melee and ranged attacks where possible.
            if (gCastFrames == 12) {
                if (gCastOnMelee) {
                    gpCastState = &gStates[mobjinfo.meleestate];
                } else {
                    gpCastState = &gStates[mobjinfo.missilestate];
                }

                // If there wasn't a ranged or melee attack then try using the opposite attack type
                gCastOnMelee ^= 1;

                if (gpCastState == &gStates[S_NULL]) {
                    if (gCastOnMelee) {
                        gpCastState = &gStates[mobjinfo.meleestate];
                    } else {
                        gpCastState = &gStates[mobjinfo.missilestate];
                    }
                }
            }

            // If it is the player then every so often put it into the 'see' state
            if ((gCastFrames == 24) || (gpCastState == &gStates[S_PLAY])) {
                gpCastState = &gStates[mobjinfo.seestate];
                gCastFrames = 0;
            }

            // Update the number of tics to stay in this state.
            // If the state defines no time limit then make up one:
            gCastTics = gpCastState->tics;

            if (gCastTics == -1) {
                gCastTics = TICRATE;
            }
        }
    }

    return ga_nothing;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Does the rendering for the Doom II style finale screen (text, then cast of characters)
//------------------------------------------------------------------------------------------------------------------------------------------
void F2_Drawer() noexcept {
    // Draw the icon of sin background and increment frame count for the texture cache
    I_IncDrawnFrameCount();

    #if PSYDOOM_MODS
        Utils::onBeginUIDrawing();  // PsyDoom: UI drawing setup for the new Vulkan renderer
    #endif

    // PsyDoom: the background for the finale can now be anything as it is sourced from MAPINFO
    #if PSYDOOM_MODS
        I_CacheAndDrawBackgroundSprite(gFinaleBgTex, R_GetPaletteClutId(gpCluster->picPal));
    #else
        I_CacheAndDrawBackgroundSprite(gTex_DEMON, gPaletteClutIds[MAINPAL]);
    #endif

    // See whether we are drawing the text or the cast of characters
    if ((gFinaleStage >= F_STAGE_TEXT) && (gFinaleStage <= F_STAGE_SCROLLTEXT)) {
        // Still showing the text, show both the incoming and fully displayed text lines.
        // PsyDoom: update this to source the line data from MAPINFO and to allow for small and non-centered text.
        int32_t ypos = gFinTextYPos;

        #if PSYDOOM_MODS
            // PsyDoom: need to set the texture window before drawing (if we are using the small font)
            {
                DR_MODE drawModePrim = {};
                const SRECT texWindow = { (int16_t) gTex_STATUS.texPageCoordX, (int16_t) gTex_STATUS.texPageCoordY, 256, 256 };
                LIBGPU_SetDrawMode(drawModePrim, false, false, gTex_STATUS.texPageId, &texWindow);
                I_AddPrim(drawModePrim);
            }

            // Draw each fully visible line of text
            const int32_t ystep = (gpCluster->bSmallFont) ? 8 : 14;

            for (int32_t lineIdx = 0; lineIdx < gFinLinesDone; ++lineIdx) {
                const std::array<char, 33> textLineChars = F_GetLine(lineIdx);
                const int32_t xpos = F_GetLineXPos(textLineChars.data());

                if (gpCluster->bSmallFont) {
                    I_DrawStringSmall(xpos, ypos, textLineChars.data(), Game::getTexPalette_STATUS(), 128, 128, 128, false, true);
                } else {
                    I_DrawString(xpos, ypos, textLineChars.data());
                }

                ypos += ystep;
            }

            // Draw the incoming line of text which is partially visible
            const int32_t lastLineXPos = F_GetLineXPos(gFinIncomingLine);

            if (gpCluster->bSmallFont) {
                I_DrawStringSmall(lastLineXPos, ypos, gFinIncomingLine, Game::getTexPalette_STATUS(), 128, 128, 128, false, true);
            } else {
                I_DrawString(lastLineXPos, ypos, gFinIncomingLine);
            }
        #else
            // Draw each fully visible line of text
            for (int32_t lineIdx = 0; lineIdx < gFinLinesDone; ++lineIdx) {
                I_DrawString(-1, ypos, gFinTextLines[lineIdx]);
                ypos += 14;
            }

            // Draw the incoming line of text which is partially visible
            I_DrawString(-1, ypos, gFinIncomingLine);
        #endif
    }
    else if (gFinaleStage == F_STAGE_CAST) {
        // Showing the cast of character, get the texture for the current sprite to show and cache it
        const state_t& state = *gpCastState;
        const spritedef_t& spriteDef = gSprites[state.sprite];
        const spriteframe_t& spriteFrame = spriteDef.spriteframes[state.frame & FF_FRAMEMASK];

        // PsyDoom: updates for changes to how textures are managed (changes that help support user modding)
        #if PSYDOOM_MODS
            texture_t& spriteTex = R_GetTexForLump(spriteFrame.lump[0]);
        #else
            texture_t& spriteTex = gpSpriteTextures[spriteFrame.lump[0] - gFirstSpriteLumpNum];
        #endif

        I_CacheTex(spriteTex);

        // Setup and draw the sprite for the cast character
        #if PSYDOOM_MODS
            // PsyDoom: use local instead of scratchpad draw primitives; compiler can optimize better, and removes reliance on global state
            POLY_FT4 polyPrim = {};
        #else
            POLY_FT4& polyPrim = *(POLY_FT4*) LIBETC_getScratchAddr(128);
        #endif

        do {
            // PsyDoom: new drawing path for when the Vulkan renderer is in use.
            // This is needed to avoid artifacts when MSAA is enabled.
            #if PSYDOOM_VULKAN_RENDERER
                if (Video::isUsingVulkanRenderPath()) {
                    // Only issue draw commands if rendering is allowed!
                    if (VRenderer::isRendering()) {
                        // Set the correct draw pipeline
                        VDrawing::setDrawPipeline(VPipelineType::UI_8bpp);

                        // Decide on sprite position and texture coordinates
                        float ul, ur;
                        const int16_t ypos = 180 - spriteTex.offsetY;
                        int16_t xpos;

                        if (!spriteFrame.flip[0]) {
                            ul = 0.0f;
                            ur = (float) spriteTex.width;
                            xpos = HALF_SCREEN_W - spriteTex.offsetX;
                        } else {
                            ul = (float) spriteTex.width;
                            ur = (float) 0.0f;
                            xpos = HALF_SCREEN_W + spriteTex.offsetX - spriteTex.width;
                        }

                        const float vt = 0.0f;
                        const float vb = (float) spriteTex.height;
                        const float xl = (float) xpos;
                        const float xr = (float) xpos + (float) spriteTex.width;
                        const float yt = (float) ypos;
                        const float yb = (float) ypos + (float) spriteTex.height;

                        // Get the sprite texture window and clut location.
                        // Make sprite texture coordinates relative to a clamping texture window.
                        uint16_t texWinX, texWinY, texWinW, texWinH;
                        RV_GetTexWinXyWh(spriteTex, texWinX, texWinY, texWinW, texWinH);

                        uint16_t clutX, clutY;
                        RV_ClutIdToClutXy(gPaletteClutIds[MAINPAL], clutX, clutY);

                        // Draw the sprite and skip the classic draw code
                        VDrawing::addWorldQuad(
                            { xl, yt, 0.0f, ul, vt, 128, 128, 128 },
                            { xr, yt, 0.0f, ur, vt, 128, 128, 128 },
                            { xr, yb, 0.0f, ur, vb, 128, 128, 128 },
                            { xl, yb, 0.0f, ul, vb, 128, 128, 128 },
                            clutX, clutY,
                            texWinX, texWinY, texWinW, texWinH,
                            VLightDimMode::None, 128, 128, 128, 128
                        );
                    }

                    break;
                }
            #endif

            // Classic drawing code: use a PolyFT4 to draw the character
            LIBGPU_SetPolyFT4(polyPrim);
            LIBGPU_SetShadeTex(polyPrim, true);
            polyPrim.clut = gPaletteClutIds[MAINPAL];
            polyPrim.tpage = spriteTex.texPageId;

            const int16_t ypos = 180 - spriteTex.offsetY;
            int16_t xpos;

            if (!spriteFrame.flip[0]) {
                // PsyDoom: corrected UV coordinates to avoid pixel doubling and use 16-bit coords in case of overflow
                #if PSYDOOM_LIMIT_REMOVING
                    polyPrim.u0 = (LibGpuUV)(spriteTex.texPageCoordX);
                    polyPrim.u1 = (LibGpuUV)(spriteTex.texPageCoordX + spriteTex.width);
                    polyPrim.u2 = (LibGpuUV)(spriteTex.texPageCoordX);
                    polyPrim.u3 = (LibGpuUV)(spriteTex.texPageCoordX + spriteTex.width);
                #else
                    polyPrim.u0 = spriteTex.texPageCoordX;
                    polyPrim.u1 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;
                    polyPrim.u2 = spriteTex.texPageCoordX;
                    polyPrim.u3 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;
                #endif

                xpos = HALF_SCREEN_W - spriteTex.offsetX;
            } else {
                // PsyDoom: corrected UV coordinates to avoid pixel doubling and use 16-bit coords in case of overflow
                #if PSYDOOM_LIMIT_REMOVING
                    polyPrim.u0 = (LibGpuUV)(spriteTex.texPageCoordX + spriteTex.width);
                    polyPrim.u1 = (LibGpuUV)(spriteTex.texPageCoordX);
                    polyPrim.u2 = (LibGpuUV)(spriteTex.texPageCoordX + spriteTex.width);
                    polyPrim.u3 = (LibGpuUV)(spriteTex.texPageCoordX);
                #else
                    polyPrim.u0 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;
                    polyPrim.u1 = spriteTex.texPageCoordX;
                    polyPrim.u2 = spriteTex.texPageCoordX + (uint8_t) spriteTex.width - 1;
                    polyPrim.u3 = spriteTex.texPageCoordX;
                #endif

                xpos = HALF_SCREEN_W + spriteTex.offsetX - spriteTex.width;
            }

            LIBGPU_setXY4(polyPrim,
                xpos,                       ypos,
                spriteTex.width + xpos,     ypos,
                xpos,                       spriteTex.height + ypos,
                spriteTex.width + xpos,     spriteTex.height + ypos
            );

            // PsyDoom: corrected UV coordinates to avoid pixel doubling and use 16-bit coords in case of overflow
            #if PSYDOOM_LIMIT_REMOVING
                polyPrim.v0 = (LibGpuUV) spriteTex.texPageCoordY;
                polyPrim.v1 = (LibGpuUV) spriteTex.texPageCoordY;
                polyPrim.v2 = (LibGpuUV) spriteTex.texPageCoordY + spriteTex.height;
                polyPrim.v3 = (LibGpuUV) spriteTex.texPageCoordY + spriteTex.height;
            #else
                polyPrim.v0 = spriteTex.texPageCoordY;
                polyPrim.v1 = spriteTex.texPageCoordY;
                polyPrim.v2 = spriteTex.texPageCoordY + (uint8_t) spriteTex.height - 1;
                polyPrim.v3 = spriteTex.texPageCoordY + (uint8_t) spriteTex.height - 1;
            #endif

            I_AddPrim(polyPrim);
        } while (0);

        // Draw screen title and current character name
        I_DrawString(-1, 20, "Cast Of Characters");
        I_DrawString(-1, 208, gCastOrder[gCastNum].name);
    }

    // Finale can be paused too, a step towards exiting back to the main menu...
    if (gbGamePaused) {
        I_DrawPausedOverlay();
    }

    // PsyDoom: draw any enabled performance counters
    #if PSYDOOM_MODS
        I_DrawEnabledPerfCounters();
    #endif

    // Finish up the frame
    I_SubmitGpuCmds();
    I_DrawPresent();
}
