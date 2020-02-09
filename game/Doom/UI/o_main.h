#pragma once

#include "PsxVm/VmPtr.h"

struct texture_t;

extern const VmPtr<texture_t> gTex_MARB01;

void O_Init() noexcept;
void O_Shutdown() noexcept;
void O_Control() noexcept;
void O_Drawer() noexcept;
