#pragma once

#include "Doom/doomdef.h"

struct texture_t;

//------------------------------------------------------------------------------------------------------------------------------------------
// Enum representing a style/mode for the title screen.
// This can vary depending on what game is loaded.
//------------------------------------------------------------------------------------------------------------------------------------------
enum class TitleScreenStyle : uint8_t {
    Doom,           // Doom style title screen where the 'DOOM' logo floats up over the fire
    FinalDoom,      // Final Doom style title screen where the main graphic displays over the fire and fades in from black
    GEC_ME,         // GEC Master Edition: shows both the 'DOOM' and 'Final DOOM' logos as well as the 'Master Edition' logo
};

// How many types of title screen are supported
static constexpr uint8_t NUM_TITLE_SCREEN_STYLES = 3;

extern int32_t      gTitleScreenSpriteY;
extern texture_t    gTex_TITLE;

void START_Title() noexcept;
void STOP_Title(const gameaction_t exitAction) noexcept;
gameaction_t TIC_Title() noexcept;
void DRAW_Title() noexcept;
