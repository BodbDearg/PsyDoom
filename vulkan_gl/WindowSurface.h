#pragma once

#include "Macros.h"

#include <vulkan/vulkan.h>

struct SDL_Window;

BEGIN_NAMESPACE(vgl)

class VulkanInstance;

//------------------------------------------------------------------------------------------------------------------------------------------
// Handles creation of a Vulkan presentation surface for the native windowing manager
//------------------------------------------------------------------------------------------------------------------------------------------
class WindowSurface {
public:
    WindowSurface() noexcept;
    ~WindowSurface() noexcept;

    bool init(SDL_Window* const pSdlWindow, VulkanInstance& vulkanInstance) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline SDL_Window* getSdlWindow() const noexcept { return mpSdlWindow; }
    inline VulkanInstance* getVulkanInstance() const noexcept { return mpVulkanInstance; }
    inline VkSurfaceKHR getVkSurface() const noexcept { return mVkSurface; }

private:
    // Copy and assign disallowed
    WindowSurface(const WindowSurface& other) = delete;
    WindowSurface(WindowSurface&& other) = delete;
    WindowSurface& operator = (const WindowSurface& other) = delete;
    WindowSurface& operator = (WindowSurface&& other) = delete;

    bool                mbIsValid;          // True if this object was created & initialized successfully
    SDL_Window*         mpSdlWindow;        // The window that the surface belongs to
    VulkanInstance*     mpVulkanInstance;   // The vulkan instance the surface was created with
    VkSurfaceKHR        mVkSurface;         // The Vulkan surface being presented to
};

END_NAMESPACE(vgl)
