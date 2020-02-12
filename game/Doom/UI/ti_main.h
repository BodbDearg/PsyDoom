#pragma once

#include "Doom/doomdef.h"

extern const VmPtr<int32_t> gTitleScreenSpriteY;

void START_Title() noexcept;

void STOP_Title([[maybe_unused]] const gameaction_t exitAction) noexcept;
void _thunk_STOP_Title() noexcept;

gameaction_t TIC_Title() noexcept;
void _thunk_TIC_Title() noexcept;

void DRAW_Title() noexcept;
