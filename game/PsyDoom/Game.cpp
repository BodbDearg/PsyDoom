//------------------------------------------------------------------------------------------------------------------------------------------
// Various game logic, mostly related to handling different game types and variants
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Game.h"

#include "Config/Config.h"
#include "Doom/d_main.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/UI/in_main.h"
#include "Endian.h"
#include "FatalErrors.h"
#include "IsoFileSys.h"
#include "MapInfo/MapInfo.h"
#include "ProgArgs.h"
#include "PsxVm.h"
#include "Utils.h"

#include <algorithm>
#include <chrono>

BEGIN_NAMESPACE(Game)

// The Lost Soul spawn limit for Doom and Final Doom (-1 means no limit)
static constexpr uint32_t SOUL_LIMIT_DOOM = UINT32_MAX;
static constexpr uint32_t SOUL_LIMIT_FINAL_DOOM = 16;

// Unknown map/episode name placeholders
static constexpr String32 UNKNOWN_EPISODE_NAME = "Unknown Episode";
static constexpr String32 UNKNOWN_MAP_NAME = "Unknown Map";

GameType        gGameType;              // The high level game type (Doom, Final Doom etc.)
GameVariant     gGameVariant;           // Which region specific version of the game this is
GameSettings    gSettings;              // Game rules to play with (unless overriden by demo playback or multiplayer)
GameConstants   gConstants;             // Constants that vary by game type
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
// Determine which game type we are playing and from what region.
// Also sets constants for the game based on the game type.
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
    else if (discFileExists("SLUS_666.01", 0xD012570E0D1C31ED, 0xC51B8585D187CD59)) {
        // [GEC] Master Edition PSX Doom for the PlayStation (Beta 3)
        gGameType = GameType::GEC_ME_Beta3;
        gGameVariant = GameVariant::NTSC_U;
    }
    else if (discFileExists("SLUS_666.02", 0x97BB6F2D22E807E0, 0x6D818DA45FF98708)) {
        // [GEC] Master Edition PSX Doom for the PlayStation (Beta 4)
        gGameType = GameType::GEC_ME_Beta4;
        gGameVariant = GameVariant::NTSC_U;
    }
    else if (discFileExists("PSXDOOM/ABIN/PSXDOOM.EXE") && discFileExists("PSXDOOM/MAPDIR0/MAP01.WAD") && (!discFileExists("PSXDOOM/MAPDIR0/MAP02.WAD"))) {
        // [GEC] Master Edition tools: single map test disc ('Doom' format)
        gGameType = GameType::GEC_ME_TestMap_Doom;
        gGameVariant = GameVariant::NTSC_U;
    }
    else if (discFileExists("PSXDOOM/ABIN/PSXDOOM.EXE") && discFileExists("PSXDOOM/MAPDIR0/MAP01.ROM") && (!discFileExists("PSXDOOM/MAPDIR0/MAP02.ROM"))) {
        // [GEC] Master Edition tools: single map test disc ('Final Doom' format)
        gGameType = GameType::GEC_ME_TestMap_FinalDoom;
        gGameVariant = GameVariant::NTSC_U;
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
            "The disc must be one of the following:\n\n"
            "   - Doom: NTSC-U, NTSC-J or PAL version (original or re-release editions).\n"
            "   - Final Doom: NTSC-U, NTSC-J or PAL version.\n"
            "   - [GEC] Master Edition PSX Doom (Beta 3 or 4).\n"
            "   - [GEC] Master Edition tools: single map test disc.\n"
            "   - PSX Doom Forever (ROM hack).\n"
            "   - Doom single level PAL demo (standalone disc, or in a demo collection)."
        );
    }

    // Populate constants that vary from game to game
    gConstants.populate(gGameType, gbIsDemoVersion);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Helper: tells if the game type is one of the 'GEC Master Edition' versions
//------------------------------------------------------------------------------------------------------------------------------------------
bool isGameTypeGecMe() noexcept {
    return (
        (gGameType == GameType::GEC_ME_Beta3) ||
        (gGameType == GameType::GEC_ME_Beta4)
    );
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
    settings.bFixKillCount                  = Config::gbFixKillCount;
    settings.bFixLineActivation             = Config::gbFixLineActivation;
    settings.bUseExtendedPlayerShootRange   = Config::gbUseExtendedPlayerShootRange;
    settings.bFixMultiLineSpecialCrossing   = Config::gbFixMultiLineSpecialCrossing;
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

    settings.bAllowTurningCancellation      = Config::gbAllowTurningCancellation;
    settings.bFixViewBobStrength            = Config::gbFixViewBobStrength;
    settings.bFixGravityStrength            = Config::gbFixGravityStrength;
    settings.bNoMonsters                    = ProgArgs::gbNoMonsters;
    settings.bNoMonstersBossFixup           = ProgArgs::gbNoMonstersBossFixup;
    settings.bPistolStart                   = ProgArgs::gbPistolStart;
    settings.bTurboMode                     = ProgArgs::gbTurboMode;
    settings.bUseLostSoulSpawnFix           = Config::gbUseLostSoulSpawnFix;
    settings.bUseLineOfSightOverflowFix     = Config::gbUseLineOfSightOverflowFix;

    // Note: not worth making this one a config option, just bake the choice into the binary based on whether limit removing is enabled or not.
    // In a multiplayer game or for demos however this setting will still be synchronized.
    #if PSYDOOM_LIMIT_REMOVING
        settings.bRemoveMaxCrossLinesLimit = true;
    #else
        settings.bRemoveMaxCrossLinesLimit = false;
    #endif

    settings.bFixOutdoorBulletPuffs             = Config::gbFixOutdoorBulletPuffs;
    settings.bFixBlockingGibsBug                = Config::gbFixBlockingGibsBug;
    settings.bFixSoundPropagation               = Config::gbFixSoundPropagation;
    settings.bFixSpriteVerticalWarp             = Config::gbFixSpriteVerticalWarp;
    settings.bAllowMultiMapPickup               = Config::gbAllowMultiMapPickup;
    settings.bEnableMapPatches_GamePlay         = Config::gbEnableMapPatches_GamePlay;
    settings.bCoopNoFriendlyFire                = Config::gbCoopNoFriendlyFire;
    settings.bCoopForceSpawnDeathmatchThings    = Config::gbCoopForceSpawnDeathmatchThings;
    settings.bDmExitDisabled                    = Config::gbDmExitDisabled;
    settings.bCoopPreserveKeys                  = Config::gbCoopPreserveKeys;
    settings.bDmActivateBossSpecialSectors      = Config::gbDmActivateBossSpecialSectors;

    if (Config::gLostSoulSpawnLimit == 0) {
        settings.lostSoulSpawnLimit = (bFinalDoomDefaultRules) ? SOUL_LIMIT_FINAL_DOOM : SOUL_LIMIT_DOOM;   // Auto set the spawn limit based on the game
    } else {
        settings.lostSoulSpawnLimit = Config::gLostSoulSpawnLimit;
    }

    settings.viewBobbingStrengthFixed           = (int32_t)(std::clamp(Config::gViewBobbingStrength, 0.0f, 64.0f) * (float) FRACUNIT);    // Cap this to a reasonable number, won't make much difference going above this!
    settings.dmFragLimit                        = Config::gDmFragLimit;
    settings.coopPreserveAmmoFactor             = Config::gCoopPreserveAmmoFactor;
    settings.bSinglePlayerForceSpawnDmThings    = Config::gbSinglePlayerForceSpawnDmThings;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the settings to use to play back a classic (original game) demo for the current game
//------------------------------------------------------------------------------------------------------------------------------------------
void getClassicDemoGameSettings(GameSettings& settings) noexcept {
    // Note: ignore MAPINFO defaults and user settings for game rules when it comes to classic demos.
    // Use the rules associated with the base game that the demo was recorded against in order to reproduce the same behavior and avoid desync...
    const bool bFinalDoomRules = gCurBuiltInDemo.bFinalDoomDemo;

    settings = {};
    settings.bUsePalTimings                     = gCurBuiltInDemo.bPalDemo;
    settings.bUseDemoTimings                    = true;
    settings.bFixKillCount                      = false;
    settings.bFixLineActivation                 = false;
    settings.bUseExtendedPlayerShootRange       = false;
    settings.bFixMultiLineSpecialCrossing       = false;
    settings.bUsePlayerRocketBlastFix           = false;
    settings.bUseSuperShotgunDelayTweak         = false;
    settings.bUseMoveInputLatencyTweak          = false;
    settings.bUseItemPickupFix                  = false;
    settings.bUseFinalDoomPlayerMovement        = bFinalDoomRules;
    settings.bAllowMovementCancellation         = bFinalDoomRules;
    settings.bAllowTurningCancellation          = false;
    settings.bFixViewBobStrength                = false;
    settings.bFixGravityStrength                = false;
    settings.bNoMonsters                        = false;
    settings.bNoMonstersBossFixup               = false;
    settings.bPistolStart                       = false;
    settings.bTurboMode                         = false;
    settings.bUseLostSoulSpawnFix               = false;
    settings.bUseLineOfSightOverflowFix         = false;
    settings.bRemoveMaxCrossLinesLimit          = false;
    settings.bFixOutdoorBulletPuffs             = false;
    settings.bFixBlockingGibsBug                = false;
    settings.bFixSoundPropagation               = false;
    settings.bFixSpriteVerticalWarp             = false;
    settings.bAllowMultiMapPickup               = false;
    settings.bEnableMapPatches_GamePlay         = false;
    settings.bCoopNoFriendlyFire                = false;
    settings.bCoopForceSpawnDeathmatchThings    = false;
    settings.bDmExitDisabled                    = false;
    settings.bCoopPreserveKeys                  = false;
    settings.bDmActivateBossSpecialSectors      = false;
    settings.lostSoulSpawnLimit                 = (bFinalDoomRules) ? SOUL_LIMIT_FINAL_DOOM : SOUL_LIMIT_DOOM;
    settings.viewBobbingStrengthFixed           = FRACUNIT;
    settings.dmFragLimit                        = 0;
    settings.coopPreserveAmmoFactor             = 0;
    settings.bSinglePlayerForceSpawnDmThings    = false;
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
// Get the palette (PSX Clut ID) or lump name to use for various textures
//------------------------------------------------------------------------------------------------------------------------------------------
uint16_t getTexClut_TitleScreenFire() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_titleScreenFire);
}

uint16_t getTexClut_STATUS() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_STATUS);
}

uint16_t getTexClut_TITLE() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_TITLE);
}

uint16_t getTexClut_TITLE2() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_TITLE2);
}

uint16_t getTexClut_BACK() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_BACK);
}

uint16_t getTexClut_Inter_BACK() noexcept {
    // Only use the intermission specific version of 'BACK' if available
    const MapInfo::GameInfo& gameInfo = MapInfo::getGameInfo();
    const uint8_t palIdx = (gameInfo.texLumpName_Inter_BACK.chars[0]) ? gameInfo.texPalette_Inter_BACK : gameInfo.texPalette_BACK;
    return R_GetPaletteClutId(palIdx);
}

uint16_t getTexClut_LOADING() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_LOADING);
}

uint16_t getTexClut_PAUSE() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_PAUSE);
}

uint16_t getTexClut_NETERR() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_NETERR);
}

uint16_t getTexClut_DOOM() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_DOOM);
}

uint16_t getTexClut_CONNECT() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_CONNECT);
}

uint16_t getTexClut_BUTTONS() noexcept {
    // Note: don't bother making this configurable via MAPINFO since it's not used anymore
    return R_GetPaletteClutId(Game::gConstants.texPalette_BUTTONS);
}

uint16_t getTexClut_DATA() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_DATA);
}

uint16_t getTexClut_FINAL() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_FINAL);
}

uint16_t getTexClut_OptionsBg() noexcept {
    return R_GetPaletteClutId(MapInfo::getGameInfo().texPalette_OptionsBG);
}

String8 getTexLumpName_STATUS() noexcept {
    return MapInfo::getGameInfo().texLumpName_STATUS;
}

String8 getTexLumpName_TITLE() noexcept {
    return MapInfo::getGameInfo().texLumpName_TITLE;
}

String8 getTexLumpName_TITLE2() noexcept {
    return MapInfo::getGameInfo().texLumpName_TITLE2;
}

String8 getTexLumpName_BACK() noexcept {
    return MapInfo::getGameInfo().texLumpName_BACK;
}

String8 getTexLumpName_Inter_BACK() noexcept {
    // Only use the intermission specific version of 'BACK' if available
    const MapInfo::GameInfo& gameInfo = MapInfo::getGameInfo();
    return (gameInfo.texLumpName_Inter_BACK.chars[0]) ? gameInfo.texLumpName_Inter_BACK : gameInfo.texLumpName_BACK;
}

String8 getTexLumpName_OptionsBg() noexcept {
    return MapInfo::getGameInfo().texLumpName_OptionsBG;
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
