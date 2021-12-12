#pragma once

#if PSYDOOM_VULKAN_RENDERER

struct subsector_t;

#include <cstdint>

void RV_BuildSpriteFragLists() noexcept;
void RV_DrawSubsecSpriteFrags(const int32_t drawSubsecIdx) noexcept;
void RV_DrawWeapon() noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER
