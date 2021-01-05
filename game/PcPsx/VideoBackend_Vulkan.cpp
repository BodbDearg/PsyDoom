#if PSYDOOM_VULKAN_RENDERER

#include "VideoBackend_Vulkan.h"

#include "Asserts.h"
#include "PhysicalDeviceSelection.h"
#include "VkFuncs.h"
#include "VulkanInstance.h"
#include "WindowSurface.h"

#include <SDL.h>
#include <SDL_vulkan.h>

BEGIN_NAMESPACE(Video)

//------------------------------------------------------------------------------------------------------------------------------------------
// Checks whether the Vulkan video backend is supported
//------------------------------------------------------------------------------------------------------------------------------------------
bool VideoBackend_Vulkan::isBackendSupported() noexcept {
    // Must be able to load the Vulkan library or have it embedded in the application
    if (SDL_Vulkan_LoadLibrary(nullptr) != 0)
        return false;
        
    // Must be able to retrieve Vulkan functions
    if (!SDL_Vulkan_GetVkGetInstanceProcAddr())
        return false;

    // Create a temporary (headless) API instance and make sure we can pick a suitable device from it that supports graphics.
    // It's possible that a Vulkan driver might be installed on the system but no valid devices are present.
    vgl::VkFuncs vkFuncs = {};
    vgl::VulkanInstance vulkanInstance(vkFuncs);

    if (!vulkanInstance.init(nullptr))
        return false;

    return (vgl::PhysicalDeviceSelection::selectBestHeadlessDevice(vulkanInstance.getPhysicalDevices()) != nullptr);
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates the backend with stuff for the Vulkan and classic renderers uninitialized
//------------------------------------------------------------------------------------------------------------------------------------------
VideoBackend_Vulkan::VideoBackend_Vulkan() noexcept 
    : mpSdlWindow(nullptr)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Ensures everything is cleaned up
//------------------------------------------------------------------------------------------------------------------------------------------
VideoBackend_Vulkan::~VideoBackend_Vulkan() noexcept {
    destroyRenderers();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Get the window create flags required for an SDL video backend window
//------------------------------------------------------------------------------------------------------------------------------------------
uint32_t VideoBackend_Vulkan::getSdlWindowCreateFlags() noexcept {
    // Want a Vulkan window only
    return SDL_WINDOW_VULKAN;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the SDL renderer used by this backend
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_Vulkan::initRenderers(SDL_Window* const pSdlWindow) noexcept {
    ASSERT(pSdlWindow);
    mpSdlWindow = pSdlWindow;

    // TODO...
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Cleans up and destroys the SDL renderer used by this video backend
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_Vulkan::destroyRenderers() noexcept {
    mpSdlWindow = nullptr;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Copies the output from the classic renderer (PSX framebuffer) to an SDL texture and then blits that to the screen.
//------------------------------------------------------------------------------------------------------------------------------------------
void VideoBackend_Vulkan::displayFramebuffer() noexcept {
    // TODO...
}

END_NAMESPACE(Video)

#endif  // #if PSYDOOM_VULKAN_RENDERER
