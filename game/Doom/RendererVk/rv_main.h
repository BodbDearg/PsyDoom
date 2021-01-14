#pragma once

#include "Macros.h"
#include "Doom/doomdef.h"

#if PSYDOOM_VULKAN_RENDERER

extern float        gViewXf;
extern float        gViewYf;
extern float        gViewZf;
extern float        gViewAnglef;
extern float        gViewCosf;
extern float        gViewSinf;
extern uint16_t     gClutX;
extern uint16_t     gClutY;

void RV_RenderPlayerView() noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER
