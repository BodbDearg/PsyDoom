#pragma once

#include "PsxVm/VmPtr.h"

struct texture_t;

extern const VmPtr<texture_t>   gTex_MARB01;
extern const VmPtr<int32_t>     gOptionsSndVol;
extern const VmPtr<int32_t>     gOptionsMusVol;

void O_Init() noexcept;
void _thunk_O_Shutdown() noexcept;
void O_Control() noexcept;
void O_Drawer() noexcept;
