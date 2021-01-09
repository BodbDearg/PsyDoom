#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Macros.h"

#include <cstdint>

BEGIN_NAMESPACE(VRenderer)

extern bool gbUsePsxRenderer;

void init() noexcept;
void destroy() noexcept;
void beginFrame() noexcept;
void endFrame() noexcept;
void pushPsxVramUpdates(const uint16_t rectLx, const uint16_t rectRx, const uint16_t rectTy, const uint16_t rectBy) noexcept;

END_NAMESPACE(VRenderer)

#endif  // #if PSYDOOM_VULKAN_RENDERER
