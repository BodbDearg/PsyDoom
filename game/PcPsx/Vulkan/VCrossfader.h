#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Defines.h"

namespace vgl {
    class LogicalDevice;
}

BEGIN_NAMESPACE(VCrossfader)

void init(vgl::LogicalDevice& device) noexcept;
void destroy() noexcept;
void doPreCrossfadeSetup() noexcept;
void doCrossfade(const int32_t vblanksDuration) noexcept;

END_NAMESPACE(VCrossfader)

#endif  // #if PSYDOOM_VULKAN_RENDERER
