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

BEGIN_NAMESPACE(GameUtils)

extern GameType     gGameType;
extern GameVariant  gGameVariant;

void determineGameTypeAndVariant() noexcept;

END_NAMESPACE(GameUtils)
