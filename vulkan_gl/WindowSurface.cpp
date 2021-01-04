#include "WindowSurface.h"

#include "Asserts.h"
#include "Finally.h"
#include "InstanceExtensions.h"
#include "VkFuncs.h"
#include "VulkanInstance.h"

#include <SDL_vulkan.h>

BEGIN_NAMESPACE(vgl)

//------------------------------------------------------------------------------------------------------------------------------------------
// Creates a default uninitialized surface
//------------------------------------------------------------------------------------------------------------------------------------------
WindowSurface::WindowSurface() noexcept
    : mbIsValid(false)
    , mpSdlWindow(nullptr)
    , mpVulkanInstance(nullptr)
    , mVkSurface(VK_NULL_HANDLE)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Automatically cleans up the surface if not already done
//------------------------------------------------------------------------------------------------------------------------------------------
WindowSurface::~WindowSurface() noexcept {
    destroy();
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Initializes the surface for the given SDL window
//------------------------------------------------------------------------------------------------------------------------------------------
bool WindowSurface::init(SDL_Window* const pSdlWindow, VulkanInstance& vulkanInstance) noexcept {
    // Preconditions
    ASSERT_LOG((!mbIsValid), "Must call destroy() before re-initializing!");
    ASSERT(pSdlWindow);
    ASSERT(vulkanInstance.isValid());

    // If anything goes wrong, cleanup on exit - don't half initialize!
    auto cleanupOnError = finally([&]{
        if (!mbIsValid) {
            destroy(true);
        }
    });

    // Save these fields and attempt to create the window surface
    mpSdlWindow = pSdlWindow;
    mpVulkanInstance = &vulkanInstance;

    if (!SDL_Vulkan_CreateSurface(pSdlWindow, vulkanInstance.getVkInstance(), &mVkSurface)) {
        ASSERT_FAIL("Failed to create the window surface!");
        return false;
    }
    
    mbIsValid = true;
    return true;
}

//------------------------------------------------------------------------------------------------------------------------------------------
// Destroys the window surface
//------------------------------------------------------------------------------------------------------------------------------------------
void WindowSurface::destroy(const bool bForceIfInvalid) noexcept {
    // Only destroy if we need to
    if ((!mbIsValid) && (!bForceIfInvalid))
        return;
    
    // Destroy the surface
    mbIsValid = false;

    if (mVkSurface) {
        ASSERT(mpVulkanInstance);
        const VkFuncs& vkFuncs = mpVulkanInstance->getVkFuncs();
        vkFuncs.vkDestroySurfaceKHR(mpVulkanInstance->getVkInstance(), mVkSurface, nullptr);
        mVkSurface = VK_NULL_HANDLE;
    }

    mpVulkanInstance = nullptr;
    mpSdlWindow = nullptr;
}

END_NAMESPACE(vgl)
