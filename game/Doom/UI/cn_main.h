#pragma once

#include "PsxVm/VmPtr.h"

struct texture_t;

extern const VmPtr<texture_t> gTex_BUTTONS;

void START_ControlsScreen() noexcept;
void STOP_ControlsScreen() noexcept;
void TIC_ControlsScreen() noexcept;
void DRAW_ControlsScreen() noexcept;
