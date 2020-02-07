#pragma once

#include "PsxVm/VmPtr.h"

extern const VmPtr<int32_t> gTitleScreenSpriteY;

void START_Title() noexcept;
void STOP_Title() noexcept;
void TIC_Title() noexcept;
void DRAW_Title() noexcept;
