#pragma once

#if PSYDOOM_VULKAN_RENDERER

#include "Macros.h"

namespace vgl {
    class LogicalDevice;
    class PhysicalDevice;
    class VulkanInstance;
    class WindowSurface;
    struct VkFuncs;
}

BEGIN_NAMESPACE(VRenderer)

extern vgl::VkFuncs                     gVkFuncs;
extern vgl::VulkanInstance              gVulkanInstance;
extern vgl::WindowSurface               gWindowSurface;
extern const vgl::PhysicalDevice*       gpPhysicalDevice;
extern vgl::LogicalDevice               gDevice;

void init() noexcept;
void destroy() noexcept;
void beginFrame() noexcept;
void endFrame() noexcept;

END_NAMESPACE(VRenderer)

#endif  // #if PSYDOOM_VULKAN_RENDERER
