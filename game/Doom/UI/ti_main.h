#pragma once

#include "Doom/doomdef.h"

extern int32_t gTitleScreenSpriteY;

void START_Title() noexcept;
void STOP_Title(const gameaction_t exitAction) noexcept;
gameaction_t TIC_Title() noexcept;
void DRAW_Title() noexcept;
