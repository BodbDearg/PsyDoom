#pragma once

#include "PsxVm/VmPtr.h"

struct texture_t;

extern const VmPtr<texture_t> gTex_BACK;

void RunMenu() noexcept;
void M_Start() noexcept;
void M_Stop() noexcept;
void M_Ticker() noexcept;
void M_Drawer() noexcept;
