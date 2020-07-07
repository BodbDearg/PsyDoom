#pragma once

#include "Macros.h"

#include <cstdint>

// What type of game disc is loaded?
enum class GameType : int32_t {
    Doom,
    FinalDoom
};

// What variant of the game is being run?
enum class GameVariant : int32_t {
    NTSC_U,     // North America/US version (NTSC)
    NTSC_J,     // Japanese version (NTSC)
    PAL         // European version (PAL)
};

BEGIN_NAMESPACE(Game)

extern GameType     gGameType;
extern GameVariant  gGameVariant;

void determineGameTypeAndVariant() noexcept;
bool isFinalDoom() noexcept;
int32_t getNumMaps() noexcept;
int32_t getNumRegularMaps() noexcept;
const char* getMapName(const int32_t mapNum) noexcept;
int32_t getNumEpisodes() noexcept;
const char* getEpisodeName(const int32_t episodeNum) noexcept;
int32_t getEpisodeStartMap(const int32_t episodeNum) noexcept;
int32_t getMapEpisode(const int32_t mapNum) noexcept;
uint16_t getTexPalette_BACK() noexcept;
uint16_t getTexPalette_LOADING() noexcept;
uint16_t getTexPalette_PAUSE() noexcept;
uint16_t getTexPalette_OptionsBg() noexcept;
const char* getTexLumpName_OptionsBg() noexcept;
uint16_t getTexPalette_NETERR() noexcept;
uint16_t getTexPalette_DOOM() noexcept;
uint16_t getTexPalette_BUTTONS() noexcept;
uint16_t getTexPalette_CONNECT() noexcept;
uint16_t getTexPalette_DebugFontSmall() noexcept;

END_NAMESPACE(Game)
