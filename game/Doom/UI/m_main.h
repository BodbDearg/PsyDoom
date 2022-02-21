#pragma once

#include "Doom/doomdef.h"

struct texture_t;

// UV coordinates and size of the menu cursor
static constexpr uint8_t M_SKULL_TEX_U = 132;
static constexpr uint8_t M_SKULL_TEX_V = 192;
static constexpr uint8_t M_SKULL_W = 16;
static constexpr uint8_t M_SKULL_H = 18;

extern texture_t    gTex_BACK;
extern texture_t    gTex_DOOM;
extern int32_t      gCursorPos[MAXPLAYERS];
extern int32_t      gCursorFrame;
extern int32_t      gMenuTimeoutStartTicCon;

#if PSYDOOM_MODS
    extern texture_t gTex_DATA;
#endif

gameaction_t RunMenu() noexcept;

void M_Start() noexcept;
void M_Stop(const gameaction_t exitAction) noexcept;
gameaction_t M_Ticker() noexcept;
void M_Drawer() noexcept;

#if PSYDOOM_MODS
    void M_DrawNetworkConnectDisplay() noexcept;
#endif
