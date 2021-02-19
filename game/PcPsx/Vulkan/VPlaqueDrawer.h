#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Defines.h"

struct texture_t;

namespace vgl {
    class LogicalDevice;
}

BEGIN_NAMESPACE(VPlaqueDrawer)

void init(vgl::LogicalDevice& device) noexcept;
void destroy() noexcept;
void drawPlaque(texture_t& plaqueTex, const int16_t plaqueX, const int16_t plaqueY, const int16_t clutId) noexcept;

END_NAMESPACE(VPlaqueDrawer)

#endif  // #if PSYDOOM_VULKAN_RENDERER
