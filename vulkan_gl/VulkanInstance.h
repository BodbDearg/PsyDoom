#pragma once

#include "ApiLayers.h"
#include "InstanceExtensions.h"

struct SDL_Window;

BEGIN_NAMESPACE(vgl)

class PhysicalDevice;
struct VkFuncs;

//------------------------------------------------------------------------------------------------------------------------------------------
// Represents an instance of the Vulkan API.
// Physical devices can be queried and logical devices created/destroyed through an instance.
//------------------------------------------------------------------------------------------------------------------------------------------
class VulkanInstance {
public:
    VulkanInstance(VkFuncs& funcs) noexcept;
    ~VulkanInstance() noexcept;

    bool init(SDL_Window* const pSdlWindow) noexcept;
    void destroy(const bool bForceIfInvalid = false) noexcept;

    inline bool isValid() const noexcept { return mbIsValid; }
    inline SDL_Window* getSdlWindow() const noexcept { return mpSdlWindow; }
    inline VkFuncs& getVkFuncs() noexcept { return mVkFuncs; }
    inline const VkFuncs& getVkFuncs() const noexcept { return mVkFuncs; }
    inline VkInstance getVkInstance() const noexcept { return mVkInstance; }
    inline const InstanceExtensions& getExtensions() const noexcept { return mExtensions; }
    inline const std::vector<PhysicalDevice>& getPhysicalDevices() const noexcept { return mPhysicalDevices; }
    inline bool areValidationLayersEnabled() const noexcept { return (mValidationLayerCallback != VK_NULL_HANDLE); }

private:
    // Disallow copy and move
    VulkanInstance(const VulkanInstance& other) = delete;
    VulkanInstance(VulkanInstance&& other) = delete;
    VulkanInstance& operator = (const VulkanInstance& other) = delete;
    VulkanInstance& operator = (VulkanInstance&& other) = delete;

    bool createVkInstance(const bool bEnableValidationLayers) noexcept;
    void queryPhysicalDevices() noexcept;
    void enableValidationLayerReporting() noexcept;
    void disableValidationLayerReporting() noexcept;

    bool                            mbIsValid;                  // Tells if the vulkan instance is valid and initialized successfully
    SDL_Window*                     mpSdlWindow;                // SDL window the instance is being created for
    VkFuncs&                        mVkFuncs;                   // Handle to all of the loaded Vulkan API functions
    VkInstance                      mVkInstance;                // The handle to the Vulkan instance object
    ApiLayers                       mApiLayers;                 // Available Vulkan API layers, including validation layers
    InstanceExtensions              mExtensions;                // Information and function pointers for instance-level extensions
    std::vector<PhysicalDevice>     mPhysicalDevices;           // A list of physical devices available on the instance
    VkDebugReportCallbackEXT        mValidationLayerCallback;   // Handle to the callback object for validation layers
};

END_NAMESPACE(vgl)
