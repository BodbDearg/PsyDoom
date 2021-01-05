//------------------------------------------------------------------------------------------------------------------------------------------
// Manages the low level details of interacting with Vulkan for the new Vulkan renderer
//------------------------------------------------------------------------------------------------------------------------------------------
#include "Vulkan.h"

#if PSYDOOM_VULKAN_RENDERER

#include "FatalErrors.h"
#include "PhysicalDeviceSelection.h"
#include "Video.h"
#include "VkFuncs.h"
#include "VulkanInstance.h"
#include "WindowSurface.h"
#include "LogicalDevice.h"

BEGIN_NAMESPACE(Vulkan)

static vgl::VkFuncs                 gVkFuncs;                       // Pointers to all Vulkan functions used
static vgl::VulkanInstance          gVulkanInstance(gVkFuncs);      // Vulkan API instance object
static vgl::WindowSurface           gWindowSurface;                 // Window surface to draw on
static const vgl::PhysicalDevice*   gpPhysicalDevice;               // Physical device chosen for rendering
static vgl::LogicalDevice           gLogicalDevice(gVkFuncs);       // The logical device used for Vulkan

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes Vulkan for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
void init() noexcept {
    // Initialize the Vulkan API, the window surface, and choose a device to use
    if (!gVulkanInstance.init(Video::gpSdlWindow)) {
        FatalErrors::raise("Failed to initialize a Vulkan API instance!");
        return;
    }

    if (!gWindowSurface.init(Video::gpSdlWindow, gVulkanInstance)) {
        FatalErrors::raise("Failed to initialize a Vulkan window surface!");
        return;
    }

    gpPhysicalDevice = vgl::PhysicalDeviceSelection::selectBestDevice(gVulkanInstance.getPhysicalDevices(), gWindowSurface);

    if (!gpPhysicalDevice) {
        FatalErrors::raise("Failed to find a suitable rendering device for use with Vulkan!");
        return;
    }

    if (!gLogicalDevice.init(*gpPhysicalDevice, &gWindowSurface)) {
        FatalErrors::raise("Failed to initialize a Vulkan logical device!");
        return;
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Tears down Vulkan for PsyDoom
//------------------------------------------------------------------------------------------------------------------------------------------
void destroy() noexcept {
    gLogicalDevice.destroy();
    gpPhysicalDevice = nullptr;
    gWindowSurface.destroy();
    gVulkanInstance.destroy();
    gVkFuncs = {};
}

END_NAMESPACE(Vulkan)

#endif  // #if PSYDOOM_VULKAN_RENDERER
