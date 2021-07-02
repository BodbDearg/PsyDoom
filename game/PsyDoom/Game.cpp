//------------------------------------------------------------------------------------------------------------------------------------------
// Various game logic, mostly related to handling different game types and variants
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Game.h"

#include "Config.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/UI/in_main.h"
#include "FatalErrors.h"
#include "IsoFileSys.h"
#include "MapInfo.h"
#include "ProgArgs.h"
#include "PsxVm.h"

#include <algorithm>
#include <chrono>

BEGIN_NAMESPACE(Game)

// Total number of maps in the game for Doom and Final Doom
static constexpr uint32_t NUM_MAPS_DOOM = 59;
static constexpr uint32_t NUM_MAPS_FINAL_DOOM = 30;

// Number of regular (non secret) maps in the game for Doom and Final Doom
static constexpr uint32_t NUM_REGULAR_MAPS_DOOM = 54;
static constexpr uint32_t NUM_REGULAR_MAPS_FINAL_DOOM = 30;

// The Lost Soul spawn limit for Doom and Final Doom (-1 means no limit)
static constexpr uint32_t SOUL_LIMIT_DOOM = -1;
static constexpr uint32_t SOUL_LIMIT_FINAL_DOOM = 16;

// The an unknown episode name
static constexpr String32 UNKNOWN_EPISODE_NAME = "Unknown Episode";

GameType        gGameType;
GameVariant     gGameVariant;
GameSettings    gSettings;
bool            gbIsPsxDoomForever;

// Level timer: start time
static std::chrono::high_resolution_clock::time_point gLevelStartTime;

// Level timer: how many centiseconds (1/100 second units) were elapsed when the level was finished.
// This value is only set once the end time is recorded.
static int64_t gLevelFinishTimeCentisecs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Determine which game type we are playing and from what region
//------------------------------------------------------------------------------------------------------------------------------------------
void determineGameTypeAndVariant() noexcept {
    gbIsPsxDoomForever = false;

    if (PsxVm::gIsoFileSys.getEntry("SLUS_000.77")) {
        gGameType = GameType::Doom;
        gGameVariant = GameVariant::NTSC_U;
    } else if (PsxVm::gIsoFileSys.getEntry("SLPS_003.08")) {
        gGameType = GameType::Doom;
        gGameVariant = GameVariant::NTSC_J;
    } else if (PsxVm::gIsoFileSys.getEntry("SLES_001.32")) {
        gGameType = GameType::Doom;
        gGameVariant = GameVariant::PAL;
    } else if (PsxVm::gIsoFileSys.getEntry("SLUS_003.31")) {
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::NTSC_U;
    } else if (PsxVm::gIsoFileSys.getEntry("SLPS_007.27")) {
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::NTSC_J;
    } else if (PsxVm::gIsoFileSys.getEntry("SLES_004.87")) {
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::PAL;
    } else if (PsxVm::gIsoFileSys.getEntry("ZONE3D/ABIN/ZONE3D.WAD")) {
        // This appears to be the 'PSX Doom Forever' ROM hack: pretend it's Final Doom, because it's basically just a re-skin of it
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::NTSC_U;
        gbIsPsxDoomForever = true;
    } else {
        FatalErrors::raise(
            "Unknown/unrecognized PSX Doom game disc provided!\n"
            "The disc given must be either 'Doom' or 'Final Doom' (NTSC-U, NTSC-J or PAL version)."
        );
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tells if we're running Final Doom
//------------------------------------------------------------------------------------------------------------------------------------------
bool isFinalDoom() noexcept {
    return (gGameType == GameType::FinalDoom);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the game settings as specified by the user's configuration files and command line arguments
//------------------------------------------------------------------------------------------------------------------------------------------
void getUserGameSettings(GameSettings& settings) noexcept {
    settings = {};

    if (Config::gUsePalTimings < 0) {
        settings.bUsePalTimings = (gGameVariant == GameVariant::PAL) ? true : false;    // Auto set timings based on the game disc
    } else {
        settings.bUsePalTimings = (Config::gUsePalTimings != 0);
    }

    settings.bUseDemoTimings            = Config::gbUseDemoTimings;
    settings.bUsePlayerRocketBlastFix   = Config::gbUsePlayerRocketBlastFix;
    settings.bUseSuperShotgunDelayTweak = Config::gbUseSuperShotgunDelayTweak;
    settings.bUseMoveInputLatencyTweak  = Config::gbUseMoveInputLatencyTweak;
    settings.bUseItemPickupFix          = Config::gbUseItemPickupFix;

    if (Config::gUseFinalDoomPlayerMovement < 0) {
        settings.bUseFinalDoomPlayerMovement = (isFinalDoom()) ? true : false;  // Auto decide based on the game
    } else {
        settings.bUseFinalDoomPlayerMovement = (Config::gUseFinalDoomPlayerMovement != 0);
    }

    if (Config::gAllowMovementCancellation < 0) {
        settings.bAllowMovementCancellation = (isFinalDoom()) ? true : false;   // Auto decide based on the game
    } else {
        settings.bAllowMovementCancellation = (Config::gAllowMovementCancellation != 0);
    }

    settings.bAllowTurningCancellation = Config::gbAllowTurningCancellation;
    settings.bFixViewBobStrength = Config::gbFixViewBobStrength;
    settings.bFixGravityStrength = Config::gbFixGravityStrength;
    settings.bNoMonsters = ProgArgs::gbNoMonsters;
    settings.bPistolStart = ProgArgs::gbPistolStart;
    settings.bTurboMode = ProgArgs::gbTurboMode;
    settings.bUseLostSoulSpawnFix = Config::gbUseLostSoulSpawnFix;
    settings.bUseLineOfSightOverflowFix = Config::gbUseLineOfSightOverflowFix;

    // Note: not worth making this one a config option, just bake the choice into the binary based on whether limit removing is enabled or not.
    // In a multiplayer game or for demos however this setting will still be synchronized.
    #if PSYDOOM_LIMIT_REMOVING
        settings.bRemoveMaxCrossLinesLimit = true;
    #else
        settings.bRemoveMaxCrossLinesLimit = false;
    #endif

    settings.bFixOutdoorBulletPuffs = Config::gbFixOutdoorBulletPuffs;

    if (Config::gLostSoulSpawnLimit == 0) {
        settings.lostSoulSpawnLimit = (isFinalDoom()) ? SOUL_LIMIT_FINAL_DOOM : SOUL_LIMIT_DOOM;    // Auto set the spawn limit based on the game
    } else {
        settings.lostSoulSpawnLimit = Config::gLostSoulSpawnLimit;
    }

    settings.viewBobbingStrengthFixed = (int32_t)(std::clamp(Config::gViewBobbingStrength, 0.0f, 64.0f) * (float) FRACUNIT);    // Cap this to a reasonable number, won't make much difference going above this!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the settings to use to play back a classic (original game) demo for the current game
//------------------------------------------------------------------------------------------------------------------------------------------
void getClassicDemoGameSettings(GameSettings& settings) noexcept {
    settings = {};
    settings.bUsePalTimings                 = (gGameVariant == GameVariant::PAL);
    settings.bUseDemoTimings                = true;
    settings.bUsePlayerRocketBlastFix       = false;
    settings.bUseSuperShotgunDelayTweak     = false;
    settings.bUseMoveInputLatencyTweak      = false;
    settings.bUseItemPickupFix              = false;
    settings.bUseFinalDoomPlayerMovement    = isFinalDoom();
    settings.bAllowMovementCancellation     = isFinalDoom();
    settings.bAllowTurningCancellation      = false;
    settings.bFixViewBobStrength            = false;
    settings.bFixGravityStrength            = false;
    settings.bNoMonsters                    = false;
    settings.bPistolStart                   = false;
    settings.bTurboMode                     = false;
    settings.bUseLostSoulSpawnFix           = false;
    settings.bUseLineOfSightOverflowFix     = false;
    settings.bRemoveMaxCrossLinesLimit      = false;
    settings.bFixOutdoorBulletPuffs         = false;
    settings.lostSoulSpawnLimit             = (isFinalDoom()) ? SOUL_LIMIT_FINAL_DOOM : SOUL_LIMIT_DOOM;
    settings.viewBobbingStrengthFixed       = FRACUNIT;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the total number of maps for this game
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getNumMaps() noexcept {
    return (gGameType == GameType::FinalDoom) ? NUM_MAPS_FINAL_DOOM : NUM_MAPS_DOOM;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the total number of maps excluding secret levels for this game
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getNumRegularMaps() noexcept {
    return (gGameType == GameType::FinalDoom) ? NUM_REGULAR_MAPS_FINAL_DOOM : NUM_REGULAR_MAPS_DOOM;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the name of the specified map number
//------------------------------------------------------------------------------------------------------------------------------------------
const char* getMapName(const int32_t mapNum) noexcept {
    if (gGameType == GameType::Doom) {
        if ((mapNum >= 1) && (mapNum <= 59)) {
            return gMapNames_Doom[mapNum - 1];
        }
    } else if (gGameType == GameType::FinalDoom) {
        if ((mapNum >= 1) && (mapNum <= 30)) {
            return gMapNames_FinalDoom[mapNum - 1];
        }
    }

    return "Unknown Mapname";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the number of episodes in the game
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getNumEpisodes() noexcept {
    return MapInfo::getNumEpisodes();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the name of the specified episode
//------------------------------------------------------------------------------------------------------------------------------------------
const String32& getEpisodeName(const int32_t episodeNum) noexcept {
    const MapInfo::Episode* pEpisode = MapInfo::getEpisode(episodeNum);
    return (pEpisode) ? pEpisode->name : UNKNOWN_EPISODE_NAME;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the starting map for the specified episode
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getEpisodeStartMap(const int32_t episodeNum) noexcept {
    const MapInfo::Episode* pEpisode = MapInfo::getEpisode(episodeNum);
    return (pEpisode) ? pEpisode->startMap : 1;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the episode for a specified map number.
// Note: the episode number returned is always 1-NumEpisodes, even if the map number is out of range.
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getMapEpisode(const int32_t mapNum) noexcept {
    const int32_t numEpisodes = getNumEpisodes();
    int32_t episodeNum = 1;

    while ((episodeNum + 1 <= numEpisodes) && (mapNum >= getEpisodeStartMap(episodeNum + 1))) {
        ++episodeNum;
    }

    return episodeNum;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the palette or lump name to use for various textures
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t getTexPalette_BACK() noexcept {
    return gPaletteClutIds[(gGameType == GameType::FinalDoom) ? TITLEPAL : MAINPAL];
}

uint16_t getTexPalette_LOADING() noexcept {
    return gPaletteClutIds[(gGameType == GameType::FinalDoom) ? UIPAL2 : UIPAL];
}

uint16_t getTexPalette_PAUSE() noexcept {
    return gPaletteClutIds[(gGameType == GameType::FinalDoom) ? UIPAL2 : MAINPAL];
}

uint16_t getTexPalette_OptionsBg() noexcept {
    return gPaletteClutIds[(gGameType == GameType::FinalDoom) ? UIPAL2 : MAINPAL];
}

const char* getTexLumpName_OptionsBg() noexcept {
    return (gGameType == GameType::FinalDoom) ? "TILE" : "MARB01";
}

uint16_t getTexPalette_NETERR() noexcept {
    return gPaletteClutIds[(gGameType == GameType::FinalDoom) ? UIPAL2 : UIPAL];
}

uint16_t getTexPalette_DOOM() noexcept {
    return gPaletteClutIds[(gGameType == GameType::FinalDoom) ? UIPAL : TITLEPAL];
}

uint16_t getTexPalette_BUTTONS() noexcept {
    return gPaletteClutIds[(gGameType == GameType::FinalDoom) ? UIPAL2 : MAINPAL];
}

uint16_t getTexPalette_CONNECT() noexcept {
    return gPaletteClutIds[(gGameType == GameType::FinalDoom) ? UIPAL2 : MAINPAL];
}

uint16_t getTexPalette_DebugFontSmall() noexcept {
    return gPaletteClutIds[(gGameType == GameType::FinalDoom) ? UIPAL : MAINPAL];
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Starts the timer for recording how long a playthrough of a level takes
//------------------------------------------------------------------------------------------------------------------------------------------
void startLevelTimer() noexcept {
    // Record the start time
    gLevelStartTime = std::chrono::high_resolution_clock::now();
    gLevelFinishTimeCentisecs = {};
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ends the timer for recording how long a playthrough of a level takes and records the number of 1/100 second units elapsed (centiseconds).
//------------------------------------------------------------------------------------------------------------------------------------------
void stopLevelTimer() noexcept {
    // Compute how much time has elapsed and save
    const std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
    const std::chrono::high_resolution_clock::duration elapsedNs = endTime - gLevelStartTime;
    typedef std::chrono::duration<int64_t, std::ratio<1,100>> centiseconds;

    gLevelFinishTimeCentisecs = std::chrono::duration_cast<centiseconds>(elapsedNs).count();
    gLevelStartTime = endTime;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns the recorded duration of the level playthrough, after the timer has been stopped
//------------------------------------------------------------------------------------------------------------------------------------------
int64_t getLevelFinishTimeCentisecs() noexcept {
    return gLevelFinishTimeCentisecs;
}

END_NAMESPACE(Game)
