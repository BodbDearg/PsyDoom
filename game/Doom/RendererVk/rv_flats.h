#pragma once

#if PSYDOOM_VULKAN_RENDERER

struct subsector_t;

#include <cstdint>

void RV_InitNextDrawFlats() noexcept;
void RV_DrawSubsecFloors(const int32_t fromDrawSubsecIdx) noexcept;
void RV_DrawSubsecCeilings(const int32_t fromDrawSubsecIdx) noexcept;

#endif  // #if PSYDOOM_VULKAN_RENDERER
