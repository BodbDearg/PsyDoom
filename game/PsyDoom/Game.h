#pragma once

#include "GameConstants.h"
#include "GameSettings.h"
#include "SmallString.h"

#include <cstdint>

//------------------------------------------------------------------------------------------------------------------------------------------
// What type of game disc is loaded?
//------------------------------------------------------------------------------------------------------------------------------------------
enum class GameType : int32_t {
    Doom,
    FinalDoom,
    GEC_ME_Beta3,               // [GEC] Master Edition PSX Doom for the PlayStation (Beta 3)
    GEC_ME_TestMap_Doom,        // [GEC] Master Edition tools: single map test disc ('Doom' format)
    GEC_ME_TestMap_FinalDoom,   // [GEC] Master Edition tools: single map test disc ('Final Doom' format)
    GEC_ME_Beta4,               // [GEC] Master Edition PSX Doom for the PlayStation (Beta 4)
};

//------------------------------------------------------------------------------------------------------------------------------------------
// What variant of the game is being run?
//------------------------------------------------------------------------------------------------------------------------------------------
enum class GameVariant : int32_t {
    NTSC_U,     // North America/US version (NTSC)
    NTSC_J,     // Japanese version (NTSC)
    PAL         // European version (PAL)
};

struct String32;

BEGIN_NAMESPACE(Game)

extern GameType         gGameType;
extern GameVariant      gGameVariant;
extern GameSettings     gSettings;
extern GameConstants    gConstants;
extern bool             gbIsDemoVersion;
extern bool             gbIsPsxDoomForever;

void determineGameTypeAndVariant() noexcept;
void getUserGameSettings(GameSettings& settings) noexcept;
void getClassicDemoGameSettings(GameSettings& settings) noexcept;
int32_t getNumMaps() noexcept;
int32_t getNumRegularMaps() noexcept;
const String32& getMapName(const int32_t mapNum) noexcept;
int32_t getNumEpisodes() noexcept;
const String32& getEpisodeName(const int32_t episodeNum) noexcept;
int32_t getEpisodeStartMap(const int32_t episodeNum) noexcept;
int32_t getMapEpisode(const int32_t mapNum) noexcept;
// TODO: rename these palette functions to include clut id (name might cause confusion and indicate a palette index is being returned)
uint16_t getTexPalette_STATUS() noexcept;
uint16_t getTexPalette_TITLE() noexcept;
uint16_t getTexPalette_BACK() noexcept;
String8 getTexLumpName_BACK() noexcept;
uint16_t getTexPalette_Inter_BACK() noexcept;
String8 getTexLumpName_Inter_BACK() noexcept;
uint16_t getTexPalette_LOADING() noexcept;
uint16_t getTexPalette_PAUSE() noexcept;
uint16_t getTexPalette_NETERR() noexcept;
uint16_t getTexPalette_DOOM() noexcept;
uint16_t getTexPalette_CONNECT() noexcept;
uint16_t getTexPalette_BUTTONS() noexcept;
uint16_t getTexPalette_IDCRED1() noexcept;
uint16_t getTexPalette_IDCRED2() noexcept;
uint16_t getTexPalette_WMSCRED1() noexcept;
uint16_t getTexPalette_WMSCRED2() noexcept;
uint16_t getTexPalette_LEVCRED2() noexcept;
uint16_t getTexPalette_GEC() noexcept;
uint16_t getTexPalette_GECCRED() noexcept;
uint16_t getTexPalette_DWOLRD() noexcept;
uint16_t getTexPalette_DWCRED() noexcept;
uint16_t getTexPalette_DATA() noexcept;
uint16_t getTexPalette_FINAL() noexcept;
uint16_t getTexPalette_OptionsBg() noexcept;
String8 getTexLumpName_OptionsBg() noexcept;
void startLevelTimer() noexcept;
void stopLevelTimer() noexcept;
int64_t getLevelFinishTimeCentisecs() noexcept;
int64_t getLevelElapsedTimeMicrosecs() noexcept;
void setLevelElapsedTimeMicrosecs(const int64_t elapsedUsec) noexcept;

END_NAMESPACE(Game)
