#pragma once

#include "Doom/doomdef.h"

struct texture_t;

extern int32_t      gTitleScreenSpriteY;
extern texture_t    gTex_TITLE;

void START_Title() noexcept;
void STOP_Title(const gameaction_t exitAction) noexcept;
gameaction_t TIC_Title() noexcept;
void DRAW_Title() noexcept;
