#pragma once

#include "Doom/doomdef.h"

//------------------------------------------------------------------------------------------------------------------------------------------
// Enum representing a style/mode for the credits screen.
// This can vary depending on what game is loaded.
//------------------------------------------------------------------------------------------------------------------------------------------
enum class CreditsScreenStyle : uint8_t {
    Doom,           // Doom style credits screen: two pages, ID and Williams credits.
    FinalDoom,      // Final Doom credits screen: level credits, Williams and ID credits.
    GEC_ME,         // GEC Master Edition: credits for the project, then Final Doom credits.
};

// How many types of credits screen are supported
static constexpr uint8_t NUM_CREDITS_SCREEN_STYLES = 3;

void START_Credits() noexcept;
void STOP_Credits(const gameaction_t exitAction) noexcept;
gameaction_t TIC_Credits() noexcept;
void DRAW_Credits() noexcept;
