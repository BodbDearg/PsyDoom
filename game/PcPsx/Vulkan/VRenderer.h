#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(VRenderer)

// Min/max supported depth ranges for the renderer
static constexpr float MIN_DEPTH = 1.0f;
static constexpr float MAX_DEPTH = 32768.0f;

extern bool gbUsePsxRenderer;

void init() noexcept;
void destroy() noexcept;
bool beginFrame() noexcept;
bool canSubmitDrawCmds() noexcept;
void endFrame() noexcept;
void pushPsxVramUpdates(const uint16_t rectLx, const uint16_t rectRx, const uint16_t rectTy, const uint16_t rectBy) noexcept;
uint32_t getVkRendererFbWidth() noexcept;
uint32_t getVkRendererFbHeight() noexcept;

END_NAMESPACE(VRenderer)

#endif  // #if PSYDOOM_VULKAN_RENDERER
