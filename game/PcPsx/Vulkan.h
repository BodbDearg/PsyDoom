#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Macros.h"

BEGIN_NAMESPACE(Vulkan)

void init() noexcept;
void destroy() noexcept;

END_NAMESPACE(Vulkan)

#endif  // #if PSYDOOM_VULKAN_RENDERER
