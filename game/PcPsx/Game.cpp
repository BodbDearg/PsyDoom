//------------------------------------------------------------------------------------------------------------------------------------------
// Various game logic, mostly related to handling different game types and variants
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Game.h"

#include "Config.h"
#include "Doom/Renderer/r_data.h"
#include "Doom/UI/in_main.h"
#include "FatalErrors.h"
#include "IsoFileSys.h"
#include "ProgArgs.h"
#include "PsxVm.h"

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

// Episode names for Doom and Final Doom
static const char* const gEpisodeNames_Doom[] = {
    "Ultimate Doom",
    "Doom II"
};

static const char* const gEpisodeNames_FinalDoom[] = {
    "Master Levels",
    "TNT",
    "Plutonia"
};

GameType        gGameType;
GameVariant     gGameVariant;
GameSettings    gSettings;
bool            gbIsPsxDoomForever;

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
    settings.bUseMoveInputLatencyTweak  = Config::gbUseMoveInputLatencyTweak;

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
    settings.bNoMonsters = ProgArgs::gbNoMonsters;

    if (Config::gLostSoulSpawnLimit == 0) {
        settings.lostSoulSpawnLimit = (isFinalDoom()) ? SOUL_LIMIT_FINAL_DOOM : SOUL_LIMIT_DOOM;    // Auto set the spawn limit based on the game
    } else {
        settings.lostSoulSpawnLimit = Config::gLostSoulSpawnLimit;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the settings to use to play back a classic (original game) demo for the current game
//------------------------------------------------------------------------------------------------------------------------------------------
void getClassicDemoGameSettings(GameSettings& settings) noexcept {
    settings = {};
    settings.bUsePalTimings                 = (gGameVariant == GameVariant::PAL);
    settings.bUseDemoTimings                = true;
    settings.bUsePlayerRocketBlastFix       = false;
    settings.bUseMoveInputLatencyTweak      = false;
    settings.bUseFinalDoomPlayerMovement    = isFinalDoom();
    settings.bAllowMovementCancellation     = isFinalDoom();
    settings.bAllowTurningCancellation      = false;
    settings.bFixViewBobStrength            = false;
    settings.bNoMonsters                    = false;
    settings.lostSoulSpawnLimit             = (isFinalDoom()) ? SOUL_LIMIT_FINAL_DOOM : SOUL_LIMIT_DOOM;
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
    return (gGameType == GameType::FinalDoom) ? 3 : 2;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Gets the name of the specified episode
//------------------------------------------------------------------------------------------------------------------------------------------
const char* getEpisodeName(const int32_t episodeNum) noexcept {
    if (gGameType == GameType::Doom) {
        if ((episodeNum >= 1) && (episodeNum <= 2)) {
            return gEpisodeNames_Doom[episodeNum - 1];
        }
    } else if (gGameType == GameType::FinalDoom) {
        if ((episodeNum >= 1) && (episodeNum <= 3)) {
            return gEpisodeNames_FinalDoom[episodeNum - 1];
        }
    }

    return "Unknown Episode";
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the starting map for the specified episode
//------------------------------------------------------------------------------------------------------------------------------------------
int32_t getEpisodeStartMap(const int32_t episodeNum) noexcept {
    if (gGameType == GameType::Doom) {
        switch (episodeNum) {
            case 1: return 1;
            case 2: return 31;
        }
    } else if (gGameType == GameType::FinalDoom) {
        switch (episodeNum) {
            case 1: return 1;
            case 2: return 14;
            case 3: return 25;
        }
    }
    
    return 1;
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

END_NAMESPACE(Game)
