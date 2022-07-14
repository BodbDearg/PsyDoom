//------------------------------------------------------------------------------------------------------------------------------------------
// Various game logic, mostly related to handling different game types and variants
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Game.h"

#include "Config.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/UI/in_main.h"
#include "Endian.h"
#include "FatalErrors.h"
#include "IsoFileSys.h"
#include "MapInfo.h"
#include "ProgArgs.h"
#include "PsxVm.h"
#include "Utils.h"

#include <algorithm>
#include <chrono>

//------------------------------------------------------------------------------------------------------------------------------------------
// Byte swaps all fields in the 'GameSettings' structure
//------------------------------------------------------------------------------------------------------------------------------------------
void GameSettings::byteSwap() noexcept {
    Endian::byteSwapInPlace(bUsePalTimings);
    Endian::byteSwapInPlace(bUseDemoTimings);
    Endian::byteSwapInPlace(bUseExtendedPlayerShootRange);
    Endian::byteSwapInPlace(bUsePlayerRocketBlastFix);
    Endian::byteSwapInPlace(bUseSuperShotgunDelayTweak);
    Endian::byteSwapInPlace(bUseMoveInputLatencyTweak);
    Endian::byteSwapInPlace(bUseItemPickupFix);
    Endian::byteSwapInPlace(bUseFinalDoomPlayerMovement);
    Endian::byteSwapInPlace(bAllowMovementCancellation);
    Endian::byteSwapInPlace(bAllowTurningCancellation);
    Endian::byteSwapInPlace(bFixViewBobStrength);
    Endian::byteSwapInPlace(bFixGravityStrength);
    Endian::byteSwapInPlace(bNoMonsters);
    Endian::byteSwapInPlace(bPistolStart);
    Endian::byteSwapInPlace(bTurboMode);
    Endian::byteSwapInPlace(bUseLostSoulSpawnFix);
    Endian::byteSwapInPlace(bUseLineOfSightOverflowFix);
    Endian::byteSwapInPlace(bRemoveMaxCrossLinesLimit);
    Endian::byteSwapInPlace(bFixOutdoorBulletPuffs);
    Endian::byteSwapInPlace(bFixBlockingGibsBug);
    Endian::byteSwapInPlace(bFixSoundPropagation);
    Endian::byteSwapInPlace(lostSoulSpawnLimit);
    Endian::byteSwapInPlace(viewBobbingStrengthFixed);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Bytes swaps from little to big endian or big endian to little if the host architecture is big-endian
//------------------------------------------------------------------------------------------------------------------------------------------
void GameSettings::endianCorrect() noexcept {
    if constexpr (Endian::isBig()) {
        byteSwap();
    }
}

BEGIN_NAMESPACE(Game)

// The Lost Soul spawn limit for Doom and Final Doom (-1 means no limit)
static constexpr uint32_t SOUL_LIMIT_DOOM = -1;
static constexpr uint32_t SOUL_LIMIT_FINAL_DOOM = 16;

// Unknown map/episode name placeholders
static constexpr String32 UNKNOWN_EPISODE_NAME = "Unknown Episode";
static constexpr String32 UNKNOWN_MAP_NAME = "Unknown Map";

GameType        gGameType;              // The high level game type (Doom, Final Doom etc.)
GameVariant     gGameVariant;           // Which region specific version of the game this is
GameSettings    gSettings;              // Game rules to play with (unless overriden by demo playback or multiplayer)
bool            gbIsDemoVersion;        // If the game type is 'Doom' this is set to 'true' if the one level demo disc is being played
bool            gbIsPsxDoomForever;     // If the game type is 'Final Doom' this is set to 'true' if the 'PSX Doom Forever' ROM hack is being played

// Level timer: start time
static std::chrono::high_resolution_clock::time_point gLevelStartTime;

// Level timer: how many centiseconds (1/100 second units) were elapsed when the level was finished.
// This value is only set once the end time is recorded.
static int64_t gLevelFinishTimeCentisecs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: checks if a file on the game disc exists
//------------------------------------------------------------------------------------------------------------------------------------------
static bool discFileExists(const char* const filePath) noexcept {
    return (PsxVm::gIsoFileSys.getEntry(filePath) != nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: checks if a file on the game disc exists with the specified MD5 hash (encoded in visual order)
//------------------------------------------------------------------------------------------------------------------------------------------
static bool discFileExists(const char* const filePath, const uint64_t hashWord1, const uint64_t hashWord2) noexcept {
    return Utils::checkDiscFileMD5Hash(PsxVm::gDiscInfo, PsxVm::gIsoFileSys, filePath, hashWord1, hashWord2);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Determine which game type we are playing and from what region
//------------------------------------------------------------------------------------------------------------------------------------------
void determineGameTypeAndVariant() noexcept {
    gbIsDemoVersion = false;
    gbIsPsxDoomForever = false;

    if (discFileExists("SLUS_000.77")) {
        gGameType = GameType::Doom;
        gGameVariant = GameVariant::NTSC_U;
    } else if (discFileExists("SLPS_003.08")) {
        gGameType = GameType::Doom;
        gGameVariant = GameVariant::NTSC_J;
    } else if (discFileExists("SLES_001.32")) {
        gGameType = GameType::Doom;
        gGameVariant = GameVariant::PAL;
    } else if (discFileExists("SLUS_003.31")) {
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::NTSC_U;
    } else if (discFileExists("SLPS_007.27")) {
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::NTSC_J;
    } else if (discFileExists("SLES_004.87")) {
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::PAL;
    } else if (
        (discFileExists("SLES_001.57")) ||                                                          // Standalone Doom demo disc
        (discFileExists("PSXDOOM/ABIN/PALDEMO.EXE", 0x6AAD82C7FF2D773A, 0x33E19B2483B90A36)) ||     // Essential PlayStation CD Three/3
        (discFileExists("PSXDOOM/ABIN/PALDEMO.EXE", 0xBC82CC0AD31992FF, 0xFBB47455A3FCD677))        // Euro Demo (Future) 103 (Official PlayStation Magazine 103)
    ) {
        // Doom one level demo, standalone or in a demo collection; other demo discs may also match, but I haven't tested them all!
        gGameType = GameType::Doom;
        gGameVariant = GameVariant::PAL;
        gbIsDemoVersion = true;
    }
    else if (discFileExists("ZONE3D/ABIN/ZONE3D.WAD")) {
        // This appears to be the 'PSX Doom Forever' ROM hack: pretend it's Final Doom, because it's basically just a re-skin of it
        gGameType = GameType::FinalDoom;
        gGameVariant = GameVariant::NTSC_U;
        gbIsPsxDoomForever = true;
    } 
    else {
        FatalErrors::raise(
            "Unknown/unrecognized PSX Doom game disc provided!\n"
            "The disc given must be either 'Doom' or 'Final Doom' (NTSC-U, NTSC-J or PAL version).\n"
            "Certain demo discs of 'Doom' are also supported as well as 'PSX Doom Forever'."
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
    const bool bFinalDoomDefaultRules = MapInfo::getGameInfo().bFinalDoomGameRules;

    if (Config::gUsePalTimings < 0) {
        settings.bUsePalTimings = (gGameVariant == GameVariant::PAL) ? true : false;        // Auto set timings based on the game disc
    } else {
        settings.bUsePalTimings = (Config::gUsePalTimings != 0);
    }

    settings.bUseDemoTimings                = Config::gbUseDemoTimings;
    settings.bUseExtendedPlayerShootRange   = Config::gbUseExtendedPlayerShootRange;
    settings.bUsePlayerRocketBlastFix       = Config::gbUsePlayerRocketBlastFix;
    settings.bUseSuperShotgunDelayTweak     = Config::gbUseSuperShotgunDelayTweak;
    settings.bUseMoveInputLatencyTweak      = Config::gbUseMoveInputLatencyTweak;
    settings.bUseItemPickupFix              = Config::gbUseItemPickupFix;

    if (Config::gUseFinalDoomPlayerMovement < 0) {
        settings.bUseFinalDoomPlayerMovement = (bFinalDoomDefaultRules) ? true : false;     // Auto decide based on the game
    } else {
        settings.bUseFinalDoomPlayerMovement = (Config::gUseFinalDoomPlayerMovement != 0);
    }

    if (Config::gAllowMovementCancellation < 0) {
        settings.bAllowMovementCancellation = (bFinalDoomDefaultRules) ? true : false;       // Auto decide based on the game
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
    settings.bFixBlockingGibsBug = Config::gbFixBlockingGibsBug;
    settings.bFixSoundPropagation = Config::gbFixSoundPropagation;

    if (Config::gLostSoulSpawnLimit == 0) {
        settings.lostSoulSpawnLimit = (bFinalDoomDefaultRules) ? SOUL_LIMIT_FINAL_DOOM : SOUL_LIMIT_DOOM;   // Auto set the spawn limit based on the game
    } else {
        settings.lostSoulSpawnLimit = Config::gLostSoulSpawnLimit;
    }

    settings.viewBobbingStrengthFixed = (int32_t)(std::clamp(Config::gViewBobbingStrength, 0.0f, 64.0f) * (float) FRACUNIT);    // Cap this to a reasonable number, won't make much difference going above this!
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the settings to use to play back a classic (original game) demo for the current game
//------------------------------------------------------------------------------------------------------------------------------------------
void getClassicDemoGameSettings(GameSettings& settings) noexcept {
    // Note: ignore MAPINFO defaults and user settings for game rules when it comes to classic demos.
    // Use the game rules associated with the current base game, because that is what the demos would be recorded with.
    const bool bFinalDoomRules = Game::isFinalDoom();

    settings = {};
    settings.bUsePalTimings                 = (gGameVariant == GameVariant::PAL);
    settings.bUseDemoTimings                = true;
    settings.bUseExtendedPlayerShootRange   = false;
    settings.bUsePlayerRocketBlastFix       = false;
    settings.bUseSuperShotgunDelayTweak     = false;
    settings.bUseMoveInputLatencyTweak      = false;
    settings.bUseItemPickupFix              = false;
    settings.bUseFinalDoomPlayerMovement    = bFinalDoomRules;
    settings.bAllowMovementCancellation     = bFinalDoomRules;
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
    settings.bFixSoundPropagation           = false;
    settings.bFixBlockingGibsBug            = false;
    settings.lostSoulSpawnLimit             = (bFinalDoomRules) ? SOUL_LIMIT_FINAL_DOOM : SOUL_LIMIT_DOOM;
    settings.viewBobbingStrengthFixed       = FRACUNIT;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the total number of maps for this game
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getNumMaps() noexcept {
    return MapInfo::getGameInfo().numMaps;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the total number of maps excluding secret levels for this game
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getNumRegularMaps() noexcept {
    return MapInfo::getGameInfo().numRegularMaps;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the name of the specified map number
//------------------------------------------------------------------------------------------------------------------------------------------
const String32& getMapName(const int32_t mapNum) noexcept {
    const MapInfo::Map* const pMap = MapInfo::getMap(mapNum);
    return (pMap) ? pMap->name : UNKNOWN_MAP_NAME;
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

//------------------------------------------------------------------------------------------------------------------------------------------
// Returns how much time has elapsed in the level (in microseconds)
//------------------------------------------------------------------------------------------------------------------------------------------
int64_t getLevelElapsedTimeMicrosecs() noexcept {
    const auto now = std::chrono::high_resolution_clock::now();
    const auto elapsed = now - gLevelStartTime;
    return std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Set how much time has elapsed in the level (in microseconds)
//------------------------------------------------------------------------------------------------------------------------------------------
void setLevelElapsedTimeMicrosecs(const int64_t elapsedUsec) noexcept {
    const auto now = std::chrono::high_resolution_clock::now();
    std::chrono::microseconds elapsed(elapsedUsec);
    gLevelStartTime = now - elapsed;
}

END_NAMESPACE(Game)
