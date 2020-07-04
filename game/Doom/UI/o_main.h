#pragma once

#include "Doom/doomdef.h"

struct texture_t;

extern texture_t    gTex_OptionsBg;
extern int32_t      gOptionsSndVol;
extern int32_t      gOptionsMusVol;

void O_Init() noexcept;
void O_Shutdown(const gameaction_t exitAction) noexcept;
gameaction_t O_Control() noexcept;
void O_Drawer() noexcept;
